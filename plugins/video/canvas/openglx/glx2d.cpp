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
#include "csutil/csinput.h"
#include "csutil/scf.h"
#include "csutil/setenv.h"
#include "csutil/sysfunc.h"
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

#include "plugins/video/canvas/openglx/glx2d.h"

#ifdef CS_HAVE_XRENDER
#include <X11/extensions/Xrender.h>
#endif

SCF_IMPLEMENT_FACTORY (csGraphics2DGLX)

#define XWIN_SCF_ID "crystalspace.window.x"

// csGraphics2DGLX function
csGraphics2DGLX::csGraphics2DGLX (iBase *iParent) :
  scfImplementationType (this, iParent), cmap (0), hardwareaccelerated(false),
  transparencyRequested (false), transparencyAvailable (false)
{
}

void csGraphics2DGLX::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (csQueryRegistry<iReporter> (object_reg));
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
  xvis = 0;
  hardwareaccelerated = false;

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
  {
    CS::Utility::setenv ("force_s3tc_enable", "true", 1);
  }

  csRef<iPluginManager> plugin_mgr (
  	csQueryRegistry<iPluginManager> (object_reg));
  xwin = csLoadPlugin<iXWindow> (plugin_mgr, XWIN_SCF_ID);
  if (!xwin)
  {
    Report (CS_REPORTER_SEVERITY_WARNING,
      "Could not create an instance of " XWIN_SCF_ID "!");
    return false;
  }

  dpy = xwin->GetDisplay ();
  screen_num = xwin->GetScreen ();
  {
    /* According to http://standards.freedesktop.org/wm-spec/wm-spec-latest.html#id2552725
       (section "Compositing Managers") presence of a compositing window manager
       is signalled by the WM owning a selection "_NET_WM_CM_Sn", n being the screen
       number. */
    csString selname;
    selname.Format ("_NET_WM_CM_S%d", screen_num);
    compositingManagerPresenceSelection = XInternAtom (dpy, selname.GetData(), True);
  }
  
  xwin->SetHWMouseMode (hwMouse);

  // Create the event outlet
  csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (object_reg));
  if (q != 0)
    EventOutlet = q->CreateEventOutlet (this);

  return true;
}

csGraphics2DGLX::~csGraphics2DGLX ()
{
  // Destroy your graphic interface
  XFree ((void*)xvis);
  Close ();
}

bool csGraphics2DGLX::Open ()
{
  if (is_open) return true;

  Report (CS_REPORTER_SEVERITY_NOTIFY, "Opening GLX2D");
  // We now select the visual here as with a mesa bug it is not possible
  // to destroy double buffered contexts and then create a single buffered
  // one.
  
  ext.InitGLX_ARB_multisample (dpy, screen_num);

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
  active_GLContext = glXCreateContext (dpy, xvis, 0, True);

  // this makes the context we just created the current
  // context, so that all subsequent OpenGL calls will set state and
  // draw stuff on this context.  You could of couse make
  // some more contexts and switch around between several of them...
  // but we use only one here.
  glXMakeCurrent (dpy, window, active_GLContext);
  
  XSync (dpy, False);
  
  GetCurrentAttributes ();
  
  // Open your graphic interface
  if (!csGraphics2DGLCommon::Open ())
    return false;

  xwin->SetTitle (win_title);

  return true;
}

void csGraphics2DGLX::Close (void)
{
  if (!is_open) return;
    
  // Close your graphic interface
  csGraphics2DGLCommon::Close ();
  if (active_GLContext != 0)
  {
    glXDestroyContext (dpy,active_GLContext);
    active_GLContext = 0;
  }

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
    csQueryRegistry<iVerbosityManager> (object_reg));
  if (verbosemgr) 
    do_verbose = verbosemgr->Enabled ("renderer.x.visual");
    
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Creating Context");
  
  GLPixelFormat format;
  csGLPixelFormatPicker picker (this);
  csDirtyAccessArray<int> desired_attributes;
  
  bool tryMultisample = ext.CS_GLX_ARB_multisample;
  
  for (int run = (tryMultisample ? 2 : 1); (run-- > 0) && !xvis; )
  {
    while (picker.GetNextFormat (format))
    {
      if (do_verbose)
      {
        csString pfStr;
        GetPixelFormatString (format, pfStr);
    
        Report (CS_REPORTER_SEVERITY_NOTIFY,
          "Probing pixel format: %s", pfStr.GetData ());
      }
      const int colorBits = format[glpfvColorBits];
      const int colorComponentSize = 
        ((colorBits % 32) == 0) ? colorBits / 4 : colorBits / 3;
      const int accumBits = format[glpfvAccumColorBits];
      const int accumComponentSize = 
        ((accumBits % 32) == 0) ? accumBits / 4 : accumBits / 3;
      desired_attributes.DeleteAll();
      desired_attributes.Push (GLX_RENDER_TYPE);
      desired_attributes.Push (GLX_RGBA_BIT);
      desired_attributes.Push (GLX_DRAWABLE_TYPE);
      desired_attributes.Push (GLX_WINDOW_BIT);
      desired_attributes.Push (GLX_DOUBLEBUFFER);
      desired_attributes.Push (True);
      desired_attributes.Push (GLX_DEPTH_SIZE);
      desired_attributes.Push (format[glpfvDepthBits]);
      desired_attributes.Push (GLX_RED_SIZE);
      desired_attributes.Push (colorComponentSize);
      desired_attributes.Push (GLX_BLUE_SIZE);
      desired_attributes.Push (colorComponentSize);
      desired_attributes.Push (GLX_GREEN_SIZE);
      desired_attributes.Push (colorComponentSize);
      desired_attributes.Push (GLX_ALPHA_SIZE);
      desired_attributes.Push (format[glpfvAlphaBits]);
      desired_attributes.Push (GLX_STENCIL_SIZE);
      desired_attributes.Push (format[glpfvStencilBits]);
      desired_attributes.Push (GLX_ACCUM_RED_SIZE);
      desired_attributes.Push (accumComponentSize);
      desired_attributes.Push (GLX_ACCUM_BLUE_SIZE);
      desired_attributes.Push (accumComponentSize);
      desired_attributes.Push (GLX_ACCUM_GREEN_SIZE);
      desired_attributes.Push (accumComponentSize);
      desired_attributes.Push (GLX_ACCUM_ALPHA_SIZE);
      desired_attributes.Push (format[glpfvAccumAlphaBits]);
      if (run >= 1)
      {
        desired_attributes.Push (GLX_SAMPLE_BUFFERS_ARB);
        desired_attributes.Push ((format[glpfvMultiSamples] != 0) ? 1 : 0);
        desired_attributes.Push (GLX_SAMPLES_ARB);
        desired_attributes.Push (format[glpfvMultiSamples]);
      }
      desired_attributes.Push (None);
      // find an fbconfig that supports all the features we need
      int numConfigs;
      GLXFBConfig* fbconfigs = glXChooseFBConfig (dpy, screen_num,
						  desired_attributes.GetArray (),
						  &numConfigs);
      if (fbconfigs)
      {
	int fbconfig = 0; // Default to first available config
      #ifdef CS_HAVE_XRENDER
	// If transparency was requested, check if a transparent fbconfig is available
	if (transparencyRequested)
	{
	  for (int i = 0; i < numConfigs; i++)
	  {
	    XVisualInfo* vis = glXGetVisualFromFBConfig (dpy, fbconfigs[i]);
	    if (!vis) continue;
	    XRenderPictFormat* pictFormat = XRenderFindVisualFormat (dpy, vis->visual);
	    if (!pictFormat) continue;

	    if(pictFormat->direct.alphaMask > 0)
	    {
	      fbconfig = i;
	      transparencyAvailable = true;
	      break;
	    }

	    XFree (vis);
	  }
	}
      #endif
	
	xvis = glXGetVisualFromFBConfig (dpy, fbconfigs[fbconfig]);
	XFree (fbconfigs);
	break;
      }
    }
  }
  
  // if a visual was found that we can use, make a graphics context which
  // will be bound to the application window.  If a visual was not
  // found, then try to figure out why it failed
  if (!xvis)
  {
    Report (CS_REPORTER_SEVERITY_WARNING,
      "Cannot use preferred GLX visual - Generic visual will be used.");

    // try to get a basic fbconfig
    static const int generic_attributes [] =
    {
      GLX_RENDER_TYPE, 		GLX_RGBA_BIT,
      GLX_DRAWABLE_TYPE,	GLX_WINDOW_BIT,
      GLX_DOUBLEBUFFER,		True,
      GLX_DEPTH_SIZE,		1,
      GLX_RED_SIZE,		1,
      GLX_GREEN_SIZE,		1,
      GLX_BLUE_SIZE,		1,
      None
    };
    int numConfigs;
    GLXFBConfig* fbconfigs = glXChooseFBConfig (dpy, screen_num,
						generic_attributes,
						&numConfigs);
    if (!fbconfigs)
    {
      Report (CS_REPORTER_SEVERITY_ERROR,
	"Graphics display does not support basic GLX configuration");
      return false;
    }
    xvis = glXGetVisualFromFBConfig (dpy, fbconfigs[0]);
    XFree (fbconfigs);
  }
  
  return true;
}

void csGraphics2DGLX::GetCurrentAttributes ()
{
  hardwareaccelerated = glXIsDirect (dpy, active_GLContext);
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Video driver GL/X version %s",
    hardwareaccelerated ? "(direct renderer)" : "(indirect renderer)");
  if (!hardwareaccelerated)
  {
    Report (CS_REPORTER_SEVERITY_WARNING,
      "Indirect rendering may indicate a flawed OpenGL setup if you run on "
      "a local X server.");
  }

  Depth = xvis->depth;

  Report (CS_REPORTER_SEVERITY_NOTIFY, "Visual ID: %p, %dbit %s",
    xvis->visualid, Depth, visual_class_name (xvis->c_class));

  int ctype, frame_buffer_depth, size_depth_buffer, level;
  glXGetConfig (dpy, xvis, GLX_RGBA, &ctype);
  //glXGetConfig (dpy, xvis, GLX_DOUBLEBUFFER, &double_buffer);
  glXGetConfig (dpy, xvis, GLX_BUFFER_SIZE, &frame_buffer_depth);
  glXGetConfig (dpy, xvis, GLX_DEPTH_SIZE, &size_depth_buffer);
  glXGetConfig (dpy, xvis, GLX_LEVEL, &level);

  int r_bits, g_bits, b_bits, color_bits = 0;
  int alpha_bits = 0;
  if (ctype)
  {
    glXGetConfig (dpy, xvis, GLX_RED_SIZE, &r_bits);
    color_bits += r_bits;
    glXGetConfig (dpy, xvis, GLX_GREEN_SIZE, &g_bits);
    color_bits += g_bits;
    glXGetConfig (dpy, xvis, GLX_BLUE_SIZE, &b_bits);
    color_bits += b_bits;
    glXGetConfig (dpy, xvis, GLX_ALPHA_SIZE, &alpha_bits);
  }

  // Report Info
  currentFormat[glpfvColorBits] = color_bits;
  currentFormat[glpfvAlphaBits] = alpha_bits;
  currentFormat[glpfvDepthBits] = size_depth_buffer;
  int stencilSize = 0;
  glXGetConfig (dpy, xvis, GLX_STENCIL_SIZE, &stencilSize);
  currentFormat[glpfvStencilBits] = stencilSize;
  int accumBits = 0;
  int accumAlpha = 0;
  {
    int dummy;
    glXGetConfig (dpy, xvis, GLX_ACCUM_RED_SIZE, &dummy);
    accumBits += dummy;
    glXGetConfig (dpy, xvis, GLX_ACCUM_GREEN_SIZE, &dummy);
    accumBits += dummy;
    glXGetConfig (dpy, xvis, GLX_ACCUM_BLUE_SIZE, &dummy);
    accumBits += dummy;
    glXGetConfig (dpy, xvis, GLX_ACCUM_ALPHA_SIZE, &accumAlpha);
  }
  currentFormat[glpfvAccumColorBits] = accumBits;
  currentFormat[glpfvAccumAlphaBits] = accumAlpha;
  
  if (ext.CS_GLX_ARB_multisample)
  {
    int v;
    glXGetConfig (dpy, xvis, GLX_SAMPLES_ARB, &v);
    currentFormat[glpfvMultiSamples] = v;
  }

  if (ctype)
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY, "R%d:G%d:B%d:A%d, ",
      r_bits, g_bits, b_bits, alpha_bits);
  }
    
  Report (CS_REPORTER_SEVERITY_NOTIFY, "level %d, double buffered", level);
}

bool csGraphics2DGLX::GetWorkspaceDimensions (int& width, int& height)
{
  return xwin->GetWorkspaceDimensions (width, height);
}

bool csGraphics2DGLX::AddWindowFrameDimensions (int& width, int& height)
{
  return xwin->AddWindowFrameDimensions (width, height);
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

bool csGraphics2DGLX::IsWindowTransparencyAvailable()
{
#ifdef CS_HAVE_XRENDER
  if (compositingManagerPresenceSelection == None) return false;
  /* According to http://standards.freedesktop.org/wm-spec/wm-spec-latest.html#id2552725
     (section "Compositing Managers") presence of a compositing window manager
     is signalled by the WM owning a selection "_NET_WM_CM_Sn", n being the screen
     number. Check that. */
  return (XGetSelectionOwner (dpy, compositingManagerPresenceSelection) != None);
#else
  return false;
#endif
}

bool csGraphics2DGLX::SetWindowTransparent (bool transparent)
{
#ifdef CS_HAVE_XRENDER
  if (!is_open)
  {
    transparencyRequested = transparent;
    return true;
  }
  else
  {
    // This train has left the station.
    return false;
  }
#else
  return false;
#endif
}

bool csGraphics2DGLX::GetWindowTransparent ()
{
  return is_open ? transparencyAvailable : transparencyRequested;
}

bool csGraphics2DGLX::SetWindowDecoration (WindowDecoration decoration, bool flag)
{
  return xwin->SetWindowDecoration (decoration, flag);
}

bool csGraphics2DGLX::GetWindowDecoration (WindowDecoration decoration)
{
  bool flag;
  if (xwin->GetWindowDecoration (decoration, flag))
    return flag;
  return csGraphics2D::GetWindowDecoration (decoration);
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

bool csGraphics2DGLX::Resize (int w, int h)
{
  if (!csGraphics2DGLCommon::Resize (w, h)) return false;
  xwin->Resize (w, h);
  return true;
}

#undef XWIN_SCF_ID
