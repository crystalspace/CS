/*
    Copyright (C) 1998 by Jorrit Tyberghein

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "csutil/sysfunc.h"
#include "plugins/video/canvas/openglx/glx2d.h"
#include "csutil/scf.h"
#include "csutil/csinput.h"
#include "csgeom/csrect.h"
#include "csutil/cfgacc.h"
#include "iutil/plugin.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "iutil/cfgfile.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"

#include <dlfcn.h>

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csGraphics2DGLX)

#define DEF_OGLDISP "crystalspace.graphics2d.glx.disp.empty"
#define XWIN_SCF_ID "crystalspace.window.x"

// csGraphics2DGLX function
csGraphics2DGLX::csGraphics2DGLX (iBase *iParent) :
  scfImplementationType (this, iParent), cmap (0), hardwareaccelerated(false), libGL (0)
{
}

void csGraphics2DGLX::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (object_reg, iReporter));
  if (rep)
    rep->ReportV (severity, "crystalspace.canvas.glx2d", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

bool csGraphics2DGLX::Initialize (iObjectRegistry *object_reg)
{
  const char *strDriver;
  dispdriver = 0;
  xvis = 0;
  hardwareaccelerated = false;
  
#ifdef LIBGL_NAME
  /* HACK: Work around issue where DRI drivers fail to load.
   * If RTLD_GLOBAL is not defined, DRI drivers may not find some symbols
   * exported from libGL, and hence not load. Previously, RTLD_GLOBAL was
   * set on plugin loading; however, this has a couple of wacky, undesired
   * side effects - like symbols, supposed to be from static libraries, 
   * being imported from other modules, causing side effects like memory
   * allocation not being from the same module any more.
   * To avoid using RTLD_GLOBAL for plugins, but nevertheless allow DRI 
   * drivers to work, we hack around this by manually loading libGL with
   * RTLD_GLOBAL. This is enough to satisfy the DRI drivers.
   */
  libGL = dlopen ("lib" LIBGL_NAME ".so", RTLD_LAZY | RTLD_GLOBAL);
#endif

  if (!csGraphics2DGLCommon::Initialize (object_reg))
    return false;

  /* Mesa DRI drivers don't support S3TC compressed textures entirely, only
   * upload. This behaviour is not conform to the specification for the
   * texture compression spec, so the ext is not reported by default.
   * However, it can nevertheless be activated by setting the 
   * force_s3tc_enable env var to true.
   * Do that since CS can take advantage of the upload-only support. */
  bool mesaForceS3TCEnable = 
    config->GetBool ("Video.OpenGL.MesaForceS3TCEnable", false);
  if (mesaForceS3TCEnable && !getenv ("force_s3tc_enable"))
    putenv ("force_s3tc_enable=true");

  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (object_reg, iPluginManager));
  if ((strDriver = config->GetStr ("Video.OpenGL.Display.Driver", 0)))
  {
    dispdriver = CS_LOAD_PLUGIN (plugin_mgr, strDriver, iOpenGLDisp);
    if (!dispdriver)
    {
      Report (CS_REPORTER_SEVERITY_WARNING,
        "Could not create an instance of %s ! Using 0 instead.",
        strDriver);
    }
    else if (!dispdriver->open ())
    {
      Report (CS_REPORTER_SEVERITY_ERROR,
        "open of displaydriver %s failed!", strDriver);
      return false;
    }
  }

  xwin = CS_LOAD_PLUGIN (plugin_mgr, XWIN_SCF_ID, iXWindow);
  if (!xwin)
  {
    Report (CS_REPORTER_SEVERITY_WARNING,
      "Could not create an instance of " XWIN_SCF_ID "!");
    return false;
  }

  dpy = xwin->GetDisplay ();
  screen_num = xwin->GetScreen ();

  pfmt.RedMask = 0;
  pfmt.GreenMask = 0;
  pfmt.BlueMask = 0;
  pfmt.AlphaMask = 0;
  pfmt.PalEntries = 0; // Texture mananger needs to know this.
  pfmt.PixelBytes = 0;

  // Create the event outlet
  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (q != 0)
    EventOutlet = q->CreateEventOutlet (this);

  return true;
}

csGraphics2DGLX::~csGraphics2DGLX ()
{
  // Destroy your graphic interface
  XFree ((void*)xvis);
  Close ();
  if (libGL) dlclose (libGL);
}

bool csGraphics2DGLX::Open()
{
  if (is_open) return true;

  Report (CS_REPORTER_SEVERITY_NOTIFY, "Opening GLX2D");
  // We now select the visual here as with a mesa bug it is not possible
  // to destroy double buffered contexts and then create a single buffered
  // one.

  if (!ChooseVisual ())
    return false;

  cmap = XCreateColormap (dpy, RootWindow (dpy, xvis->screen),
    xvis->visual, AllocNone);

  xwin->SetColormap (cmap);
  xwin->SetVisualInfo (xvis);
  xwin->SetCanvas (static_cast<iGraphics2D*> (this));
  if (!xwin->Open ())
  {
    Report (CS_REPORTER_SEVERITY_ERROR,
      "Failed to open the X-Window!");
    return false;
  }
  window = xwin->GetWindow ();
  active_GLContext = glXCreateContext(dpy, xvis, 0, True);

  // this makes the context we just created the current
  // context, so that all subsequent OpenGL calls will set state and
  // draw stuff on this context.  You could of couse make
  // some more contexts and switch around between several of them...
  // but we use only one here.
  glXMakeCurrent (dpy, window, active_GLContext);
  
  XSync (dpy, False);
  
  GetCurrentAttributes();
  
  // Open your graphic interface
  if (!csGraphics2DGLCommon::Open ())
    return false;

  xwin->SetTitle (win_title);

  return true;
}

void csGraphics2DGLX::Close(void)
{
  if (!is_open) return;
    
  // Close your graphic interface
  csGraphics2DGLCommon::Close ();
  if (active_GLContext != 0)
  {
    glXDestroyContext(dpy,active_GLContext);
    active_GLContext = 0;
  }

  if (dispdriver)
    dispdriver->close();

  if (xwin)
    xwin->Close ();
}

const char* csGraphics2DGLX::GetVersionString (const char* ver)
{
  if (strcmp (ver, "mesa") == 0)
  {
    static const char needle[] = "Mesa ";
    const char* glVersion = (const char*)glGetString (GL_VERSION);
    const char* p = strstr (glVersion, needle);
    if (p != 0)
      return p + sizeof (needle) - 1;
    return 0;
  }
  else
    return csGraphics2DGLCommon::GetVersionString (ver);
}

static const char *visual_class_name (int cls)
{
  switch (cls)
  {
    case StaticColor:
      return "StaticColor";
    case PseudoColor:
      return "PseudoColor";
    case StaticGray:
      return "StaticGray";
    case GrayScale:
      return "GrayScale";
    case TrueColor:
      return "TrueColor";
    case DirectColor:
      return "DirectColor";
    default:
      return "";
  }
}

bool csGraphics2DGLX::ChooseVisual ()
{
  bool do_verbose = false;
  csRef<iVerbosityManager> verbosemgr (
    CS_QUERY_REGISTRY (object_reg, iVerbosityManager));
  if (verbosemgr) 
    do_verbose = verbosemgr->Enabled ("renderer.x.visual");
    
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Creating Context");
  
  GLPixelFormat format;
  csGLPixelFormatPicker picker (this);
  
  while (picker.GetNextFormat (format))
  {
    if (do_verbose)
    {
      csString pfStr;
      GetPixelFormatString (format, pfStr);
  
      Report (CS_REPORTER_SEVERITY_NOTIFY,
	"Probing pixel format: %s", pfStr.GetData());
    }
    const int colorBits = format[glpfvColorBits];
    const int colorComponentSize = 
	((colorBits % 32) == 0) ? colorBits / 4 : colorBits / 3;
    const int accumBits = format[glpfvAccumColorBits];
    const int accumComponentSize = 
	((accumBits % 32) == 0) ? accumBits / 4 : accumBits / 3;
    int desired_attributes[] =
    {
      GLX_RGBA,
      GLX_DEPTH_SIZE, format[glpfvDepthBits],
      GLX_RED_SIZE, colorComponentSize,
      GLX_BLUE_SIZE, colorComponentSize,
      GLX_GREEN_SIZE, colorComponentSize,
      GLX_DOUBLEBUFFER,
      GLX_ALPHA_SIZE, format[glpfvAlphaBits],
      GLX_STENCIL_SIZE, format[glpfvStencilBits],
      GLX_ACCUM_RED_SIZE, accumComponentSize,
      GLX_ACCUM_BLUE_SIZE, accumComponentSize,
      GLX_ACCUM_GREEN_SIZE, accumComponentSize,
      GLX_ACCUM_ALPHA_SIZE, format[glpfvAccumAlphaBits],
      None
    };
    // find a visual that supports all the features we need
    xvis = glXChooseVisual (dpy, screen_num, desired_attributes);
    if (xvis) break;
  }

  // if a visual was found that we can use, make a graphics context which
  // will be bound to the application window.  If a visual was not
  // found, then try to figure out why it failed
  if (!xvis)
  {
    Report (CS_REPORTER_SEVERITY_WARNING,
      "Cannot use preferred GLX visual - Generic visual will be used.");
    hardwareaccelerated = false;

    // what attribute was not supplied? we know that trying to get
    // all the attributes at once doesn't work.  provide more user info by
    // trying each of the pieces and seeing if any single piece is not provided

    // try to get a visual with 12 bit color
    int generic_attributes [] = {GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 1, None};
    if (!(xvis=glXChooseVisual(dpy, screen_num, generic_attributes)) )
    {
      Report (CS_REPORTER_SEVERITY_WARNING,
        "Graphics display does not support a generic visual with double buffer and depth buffer");
		
      int doublebuffer_attributes [] = {GLX_RGBA, GLX_DOUBLEBUFFER, None};
      if (!(xvis=glXChooseVisual (dpy, screen_num, doublebuffer_attributes)))
      {
        Report (CS_REPORTER_SEVERITY_WARNING,
          "Graphics display does not provide double buffering");

        int depthbuffer_attributes [] = {GLX_RGBA, GLX_DEPTH_SIZE,1, None};
          
        if (!(xvis=glXChooseVisual (dpy, screen_num, depthbuffer_attributes)))
        {
          Report (CS_REPORTER_SEVERITY_WARNING,
            "Graphics display does not support a depth buffer");

          int color_attributes[] =
            { GLX_RGBA, GLX_RED_SIZE,4, GLX_BLUE_SIZE,4,GLX_GREEN_SIZE,4,None };

          if (!(xvis=glXChooseVisual(dpy, screen_num, color_attributes)))
          {
            Report (CS_REPORTER_SEVERITY_WARNING,
              "Graphics display does not support at least 12 bit color");
            return false;
          }
        }
      }
    }
  }
  return true;
}

void csGraphics2DGLX::GetCurrentAttributes ()
{
  hardwareaccelerated = glXIsDirect (dpy, active_GLContext);
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Video driver GL/X version %s",
    hardwareaccelerated ? "(direct renderer)" : 
    "(indirect renderer)");
  if (!hardwareaccelerated)
  {
    Report (CS_REPORTER_SEVERITY_WARNING,
      "Indirect rendering may indicate a flawed OpenGL setup if you run on "
      "a local X server.");
  }

  Depth = xvis->depth;

  if (Depth == 24 || Depth == 32)
    pfmt.PixelBytes = 4;
  else
    pfmt.PixelBytes = 2;

  Report (CS_REPORTER_SEVERITY_NOTIFY, "Visual ID: %p, %dbit %s",
    xvis->visualid, Depth, visual_class_name (xvis->c_class));

  int ctype, frame_buffer_depth, size_depth_buffer, level;
  glXGetConfig(dpy, xvis, GLX_RGBA, &ctype);
  //glXGetConfig(dpy, xvis, GLX_DOUBLEBUFFER, &double_buffer);
  glXGetConfig(dpy, xvis, GLX_BUFFER_SIZE, &frame_buffer_depth);
  glXGetConfig(dpy, xvis, GLX_DEPTH_SIZE, &size_depth_buffer);
  glXGetConfig(dpy, xvis, GLX_LEVEL, &level);

  int color_bits = 0;
  int alpha_bits = 0;
  if (ctype)
  {
    pfmt.RedMask = xvis->red_mask;
    pfmt.GreenMask = xvis->green_mask;
    pfmt.BlueMask = xvis->blue_mask;
    glXGetConfig(dpy, xvis, GLX_RED_SIZE, &pfmt.RedBits);
    color_bits += pfmt.RedBits;
    glXGetConfig(dpy, xvis, GLX_GREEN_SIZE, &pfmt.GreenBits);
    color_bits += pfmt.GreenBits;
    glXGetConfig(dpy, xvis, GLX_BLUE_SIZE, &pfmt.BlueBits);
    color_bits += pfmt.BlueBits;
    glXGetConfig(dpy, xvis, GLX_ALPHA_SIZE, &alpha_bits);
    pfmt.AlphaBits = alpha_bits;
      
    int bit;
    // Fun hack, xvis doesn't provide alpha mask
    bit=0; while (bit < alpha_bits) pfmt.AlphaMask |= (1<<(bit++));
    pfmt.AlphaMask = pfmt.AlphaMask << color_bits;

    bit=0; while (!(pfmt.RedMask & (1<<bit))) bit++; pfmt.RedShift = bit;
    bit=0; while (!(pfmt.GreenMask & (1<<bit))) bit++; pfmt.GreenShift = bit;
    bit=0; while (!(pfmt.BlueMask & (1<<bit))) bit++; pfmt.BlueShift = bit;
    if (pfmt.AlphaMask)
    {
      bit=0; while (!(pfmt.AlphaMask & (1<<bit))) bit++; pfmt.AlphaShift = bit;
    }
  }

  // Report Info
  currentFormat[glpfvColorBits] = color_bits;
  currentFormat[glpfvAlphaBits] = alpha_bits;
  currentFormat[glpfvDepthBits] = size_depth_buffer;
  int stencilSize = 0;
  glXGetConfig(dpy, xvis, GLX_STENCIL_SIZE, &stencilSize);
  currentFormat[glpfvStencilBits] = stencilSize;
  int accumBits = 0;
  int accumAlpha = 0;
  {
    int dummy;
    glXGetConfig(dpy, xvis, GLX_ACCUM_RED_SIZE, &dummy);
    accumBits += dummy;
    glXGetConfig(dpy, xvis, GLX_ACCUM_GREEN_SIZE, &dummy);
    accumBits += dummy;
    glXGetConfig(dpy, xvis, GLX_ACCUM_BLUE_SIZE, &dummy);
    accumBits += dummy;
    glXGetConfig(dpy, xvis, GLX_ACCUM_ALPHA_SIZE, &accumAlpha);
  }
  currentFormat[glpfvAccumColorBits] = accumBits;
  currentFormat[glpfvAccumAlphaBits] = accumAlpha;

  if (ctype)
  {
    if (pfmt.RedMask > pfmt.BlueMask)
    {
      Report (CS_REPORTER_SEVERITY_NOTIFY, "R%d:G%d:B%d:A%d, ",
        pfmt.RedBits, pfmt.GreenBits, pfmt.BlueBits, alpha_bits);
    }
    else
    {
      Report (CS_REPORTER_SEVERITY_NOTIFY, "B%d:G%d:R%d:A%d, ",
        pfmt.BlueBits, pfmt.GreenBits, pfmt.RedBits, alpha_bits);
    }
  }
    
  Report (CS_REPORTER_SEVERITY_NOTIFY, "level %d, double buffered", level);

  pfmt.complete ();
}

bool csGraphics2DGLX::PerformExtensionV (char const* command, va_list args)
{
  if (!strcasecmp (command, "hardware_accelerated"))
  {
    bool* hasaccel = (bool*)va_arg (args, bool*);
    *hasaccel = hardwareaccelerated;
    return true;
  }
  if (!strcasecmp (command, "fullscreen"))
  {
    xwin->SetFullScreen (!xwin->GetFullScreen ());
    return true;
  }
  if (!strcasecmp (command, "setglcontext"))
  {
    glXMakeCurrent (dpy, window, active_GLContext);
    return true;
  }
  return csGraphics2DGLCommon::PerformExtensionV (command, args);
}

void csGraphics2DGLX::Print (csRect const* /*area*/)
{
  glXSwapBuffers (dpy,window);
}


void csGraphics2DGLX::SetFullScreen (bool yesno)
{
  csGraphics2D::SetFullScreen (yesno);
  xwin->SetFullScreen (yesno);
}

void csGraphics2DGLX::AllowResize (bool iAllow)
{
  AllowResizing = iAllow;
  xwin->AllowResize (iAllow);
}

#undef XWIN_SCF_ID
