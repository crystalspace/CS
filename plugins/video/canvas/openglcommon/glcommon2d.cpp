/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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
#include "glcommon2d.h"
#include "csutil/csendian.h"
#include "plugins/video/canvas/common/scrshot.h"
#include "csgeom/csrect.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "qint.h"
#include "igraphic/image.h"
#include "igraphic/imageio.h"
#include "plugins/video/canvas/openglcommon/glstates.h"

// This header should be moved
#include "plugins/video/renderer/common/pixfmt.h"


SCF_IMPLEMENT_IBASE_EXT (csGraphics2DGLCommon)
  SCF_IMPLEMENTS_INTERFACE (iEventPlug)
SCF_IMPLEMENT_IBASE_EXT_END
  
csGraphics2DGLCommon::csGraphics2DGLCommon (iBase *iParent) :
  csGraphics2D (iParent), statecache (0)
{
  EventOutlet = 0;
  screen_shot = 0;
  multiSamples = 0;
  multiFavorQuality = false;
  fontCache = 0;

  ssPool = 0;
}

bool csGraphics2DGLCommon::Initialize (iObjectRegistry *object_reg)
{
  if (!csGraphics2D::Initialize (object_reg))
    return false;

  // We don't really care about pixel format, except for ScreenShot()
  // and OpenGL software proctexes
#if (CS_24BIT_PIXEL_LAYOUT == CS_24BIT_PIXEL_ABGR)
    pfmt.RedMask =   0x000000FF;
    pfmt.GreenMask = 0x0000FF00;
    pfmt.BlueMask =  0x00FF0000;
#else 
    pfmt.RedMask =   0x00FF0000;
    pfmt.GreenMask = 0x0000FF00;
    pfmt.BlueMask =  0x000000FF;
#endif
  pfmt.PixelBytes = 4;
  pfmt.PalEntries = 0;
  pfmt.complete ();

  ext.Initialize (object_reg, this);

  statecache = new csGLStateCache (&ext);

  depthBits = config->GetInt ("Video.OpenGL.DepthBits", 32);
  multiSamples = config->GetInt ("Video.OpenGL.MultiSamples", 0);
  multiFavorQuality = config->GetBool ("Video.OpenGL.MultisampleFavorQuality");

  return true;
}

csGraphics2DGLCommon::~csGraphics2DGLCommon ()
{
  Close ();
  
  delete statecache;
  delete[] screen_shot;

  while (ssPool)
  {
    csGLScreenShot* next = ssPool->poolNext;
    delete ssPool;
    ssPool = next;
  }
}

bool csGraphics2DGLCommon::Open ()
{
  if (is_open) return true;

  statecache->InitCache();

  ext.Open ();
  driverdb.Open (this);

  // initialize font cache object
  fontCache = new csGLFontCache (this);

  statecache->Enable_GL_SCISSOR_TEST ();

  if (!csGraphics2D::Open ())
    return false;

  const char *renderer = (const char *)glGetString (GL_RENDERER);
  const char *vendor = (const char *)glGetString (GL_VENDOR);
  const char *version = (const char *)glGetString (GL_VERSION);
  csRef<iReporter> reporter (CS_QUERY_REGISTRY (object_reg, iReporter));
  if (renderer || version || vendor)
    if (reporter)
      reporter->Report (CS_REPORTER_SEVERITY_NOTIFY,
        "crystalspace.canvas.openglcommon",
        "OpenGL renderer: %s (vendor: %s) version %s",
        renderer ? renderer : "unknown", vendor ? vendor: "unknown", 
	version ? version : "unknown");

  if (reporter)
    reporter->Report (CS_REPORTER_SEVERITY_NOTIFY,
      "crystalspace.canvas.openglcommon",
      "Using %s mode at resolution %dx%d.",
      FullScreen ? "full screen" : "windowed", Width, Height);

  if (version)
  {
    // initialize GL version pseudo-extensions
    int n, vMajor, vMinor, vRelease;
    n = sscanf (version, "%d.%d.%d", &vMajor, &vMinor, &vRelease);
    if (n >= 2)
    {
      // Sanity check
      if ((vMajor < 1) || ((vMajor == 1) && (vMinor < 1)))
      {
	reporter->Report (CS_REPORTER_SEVERITY_ERROR,
	  "crystalspace.canvas.openglcommon",
	  "OpenGL >= 1.1 is required, but only %d.%d is present.",
	  vMajor, vMinor);
      }
      if ((vMajor >= 1) || ((vMajor == 1) && (vMinor >= 2)))
      {
	//ext.InitGL_version_1_2 ();
      }
      if ((vMajor >= 1) || ((vMajor == 1) && (vMinor >= 3)))
      {
	//ext.InitGL_version_1_3 ();
      }
    }
  }

  ext.InitGL_ARB_multitexture ();
  if (ext.CS_GL_ARB_multitexture)
  {
    GLint texUnits;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &texUnits);
    for (GLint u = texUnits - 1; u >= 0; u--)
    {
      statecache->SetActiveTU (u);
      glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
    }
  }
  
  ext.InitGL_ARB_multisample();

  if (ext.CS_GL_ARB_multisample)
  {
    GLint glmultisamp = (GLint)multiSamples;
    glGetIntegerv (GL_SAMPLES_ARB, &glmultisamp);

    if (multiSamples)
    {
      if (reporter)
	reporter->Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "crystalspace.canvas.openglcommon",
	  "Multisample: %d samples",
	  multiSamples);

      ext.InitGL_NV_multisample_filter_hint();
      if (ext.CS_GL_NV_multisample_filter_hint)
      {
	glHint (GL_MULTISAMPLE_FILTER_HINT_NV,
	  multiFavorQuality ? GL_NICEST : GL_FASTEST);
	
	GLint msHint;
	glGetIntegerv (GL_MULTISAMPLE_FILTER_HINT_NV, &msHint);
	if (reporter)
	  reporter->Report (CS_REPORTER_SEVERITY_NOTIFY,
	    "crystalspace.canvas.openglcommon",
	    "Multisample settings: %s",
	    ((msHint == GL_NICEST) ? "quality" :
	    ((msHint == GL_FASTEST) ? "performance" : "unknown")));
      }
    }
    else
    {
      if (reporter)
	reporter->Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "crystalspace.canvas.openglcommon",
	  "Multisample: disabled");
    }
  }

  glClearColor (0., 0., 0., 0.);
  glClearDepth (-1.0);

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();

  glViewport (0, 0, Width, Height);
  Clear (0);

  return true;
}

void csGraphics2DGLCommon::Close ()
{
  if (!is_open) return;
  ext.Close ();
  driverdb.Close ();
  csGraphics2D::Close ();
}

void csGraphics2DGLCommon::SetClipRect (int xmin, int ymin, int xmax, int ymax)
{
  ((csGLFontCache*)fontCache)->FlushText ();

  csGraphics2D::SetClipRect (xmin, ymin, xmax, ymax);
  glScissor (ClipX1, Height - ClipY2, ClipX2 - ClipX1, ClipY2 - ClipY1);
}

bool csGraphics2DGLCommon::BeginDraw ()
{
  if (!csGraphics2D::BeginDraw ())
    return false;
  if (FrameBufferLocked != 1)
    return true;

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  glOrtho (0, Width, 0, Height, -1.0, 10.0);
  glViewport (0, 0, Width, Height);

  // not needed really, should persist between draws
  //statecache->Enable_GL_SCISSOR_TEST ();
  //glScissor (ClipX1, Height - ClipY2, ClipX2 - ClipX1, ClipY2 - ClipY1);

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  glColor3f (1., 0., 0.);
  glClearColor (0., 0., 0., 0.);

  statecache->SetShadeModel (GL_FLAT);
  if (ext.CS_GL_ARB_multitexture)
  {
    glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
    glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
    glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PRIMARY_COLOR);
    glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
    glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
    glTexEnvf (GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1.0f);

    glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
    glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
    glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, GL_PRIMARY_COLOR);
    glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB, GL_SRC_ALPHA);
    glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_MODULATE);
    glTexEnvf (GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1.0f);
  }
  else
    glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
#ifdef CS_USE_NEW_RENDERER
  statecache->SetColorMask (true, true, true, true);
#endif

  return true;
}

void csGraphics2DGLCommon::FinishDraw ()
{
  ((csGLFontCache*)fontCache)->FlushText ();
  csGraphics2D::FinishDraw();
  if (FrameBufferLocked != 0) return;
  //statecache->Disable_GL_SCISSOR_TEST ();
}

void csGraphics2DGLCommon::DecomposeColor (int iColor,
  GLubyte &oR, GLubyte &oG, GLubyte &oB)
{
  oR = (iColor >> 16) & 0xff;
  oG = (iColor >> 8) & 0xff;
  oB = iColor & 0xff;
}

void csGraphics2DGLCommon::DecomposeColor (int iColor,
  float &oR, float &oG, float &oB)
{
  GLubyte r, g, b;
  DecomposeColor (iColor, r, g, b);
  oR = r / 255.0;
  oG = g / 255.0;
  oB = b / 255.0;
}

void csGraphics2DGLCommon::setGLColorfromint (int color)
{
  glColor3ub ((color >> 16) & 0xff, (color >> 8) & 0xff,
    color & 0xff);
}

csGLScreenShot* csGraphics2DGLCommon::GetScreenShot ()
{
  csGLScreenShot* res = 0;
  if (ssPool)
  {
    res = ssPool;
    ssPool = ssPool->poolNext;
  }
  else
  {
    res = new csGLScreenShot (this);
  }
  scfRefCount++;
  return res;
}

void csGraphics2DGLCommon::RecycleScreenShot (csGLScreenShot* shot)
{
  shot->poolNext = ssPool;
  ssPool = shot;
  if (scfRefCount == 1)
  {
    delete this;
    return;
  }
  scfRefCount--;
}

const char* csGraphics2DGLCommon::GetRendererString (const char* str)
{
  if (strcmp (str, "renderer") == 0)
  {
    return (char*)glGetString (GL_RENDERER);
  }
  else if (strcmp (str, "vendor") == 0)
  {
    return (char*)glGetString (GL_VENDOR);
  }
  else if (strcmp (str, "glversion") == 0)
  {
    return (char*)glGetString (GL_VERSION);
  }
  else if (strcmp (str, "platform") == 0)
  {
    return CS_PLATFORM_NAME;
  }
  else
    return 0;
}

const char* csGraphics2DGLCommon::GetVersionString (const char* ver)
{
  if (strcmp (ver, "gl") == 0)
  {
    return (char*)glGetString (GL_VERSION);
  }
  else
    return 0;
}

void csGraphics2DGLCommon::Clear (int color)
{
  ((csGLFontCache*)fontCache)->FlushText ();

  float r, g, b;
  DecomposeColor (color, r, g, b);
  glClearColor (r, g, b, 0.0);
  glClear (GL_COLOR_BUFFER_BIT);
}

void csGraphics2DGLCommon::SetRGB (int i, int r, int g, int b)
{
  csGraphics2D::SetRGB (i, r, g, b);
}

void csGraphics2DGLCommon::DrawLine (
  float x1, float y1, float x2, float y2, int color)
{
  ((csGLFontCache*)fontCache)->FlushText ();

  // prepare for 2D drawing--so we need no fancy GL effects!
  statecache->Disable_GL_TEXTURE_2D ();
  bool gl_alphaTest = glIsEnabled(GL_ALPHA_TEST);
  if (gl_alphaTest) statecache->Disable_GL_ALPHA_TEST ();
  setGLColorfromint (color);

  // opengl doesn't draw the last pixel of a line, but we
  // want that pixel anyway, add the pixel.
  /*if(y1==y2){ // horizontal lines
    if(x2>x1) x2++;
    else if(x1>x2) x1++;
  }
  if(x1==x2) { // vertical lines
    if(y2>y1) y2++;
    else if(y1>y2) y1++;
  }
  if(x1!=x2 && y1!=y2) // diagonal lines
  {
    if(x2>x1) x2++;
    else if(x1>x2) x1++;
  }*/

  // This extends the line enough to get the last pixel of the line on GL
  // Note! If this doesn't work in OR, just revert to old way for OR and
  // not for NR. It's tested (at least a bit :) and seems to work in NR.
  csVector2 delta (x2-x1, y2-y1);
  if (delta.SquaredNorm ()>EPSILON*EPSILON)
  {
    delta *= 1.4142135623731/delta.Norm ();
    x2 += delta.x;
    y2 += delta.y;
  }

  // This is a workaround for a hard-to-really fix problem with OpenGL:
  // whole Y coordinates are "rounded" up, this leads to one-pixel-shift
  // compared to software line drawing. This is not exactly a bug (because
  // this is an on-the-edge case) but it's different, thus we'll slightly
  // shift whole coordinates down.
  // but QInt(y1) == y1 is too coarse a test.
  if (fabs(float(int(y1))-y1) < 0.1f) { y1 += 0.05f; }
  if (fabs(float(int(y2))-y2) < 0.1f) { y2 += 0.05f; }

  // Notice: using height-y has range 1..height, but this is OK.
  //    This is because on opengl y=0.0 is off screen, as is y=height.
  //    using height-sy gives output on screen which is identical to
  //    using the software canvas.
  //    the same goes for all the other DrawX functions.
  
  glBegin (GL_LINES);
  glVertex2f (x1, Height - y1);
  glVertex2f (x2, Height - y2);
  glEnd ();

  if (gl_alphaTest) statecache->Enable_GL_ALPHA_TEST ();
}

void csGraphics2DGLCommon::DrawBox (int x, int y, int w, int h, int color)
{
  ((csGLFontCache*)fontCache)->FlushText ();

  statecache->Disable_GL_TEXTURE_2D ();
  y = Height - y;
  // prepare for 2D drawing--so we need no fancy GL effects!
  setGLColorfromint (color);

  glBegin (GL_QUADS);
  glVertex2i (x, y);
  glVertex2i (x + w, y);
  glVertex2i (x + w, y - h);
  glVertex2i (x, y - h);
  glEnd ();
}

void csGraphics2DGLCommon::DrawPixel (int x, int y, int color)
{
  ((csGLFontCache*)fontCache)->FlushText ();

  // prepare for 2D drawing--so we need no fancy GL effects!
  statecache->Disable_GL_TEXTURE_2D ();

  // using floating point pixel addresses to fix an on-the-edge case.
  // offsetting the y by a little just like for DrawLine.
  // The whole pixels get rounded up, shifting the drawpixel.
  float y1 = y;
  if (fabs(float(int(y1))-y1) < 0.1f) { y1 += 0.05f; }
  setGLColorfromint (color);
  glBegin (GL_POINTS);
  glVertex2f (x, Height - y1);
  glEnd ();
}

void csGraphics2DGLCommon::DrawPixels (
  csPixelCoord const* pixels, int num_pixels, int color)
{
  ((csGLFontCache*)fontCache)->FlushText ();

  // prepare for 2D drawing--so we need no fancy GL effects!
  statecache->Disable_GL_TEXTURE_2D ();

  setGLColorfromint (color);

  int i;
  glBegin (GL_POINTS);
  for (i = 0 ; i < num_pixels ; i++)
  {
    int x = pixels->x;
    int y = pixels->y;
    pixels++;
    glVertex2i (x, Height - y);
  }
  glEnd ();
}

void csGraphics2DGLCommon::Blit (int x, int y, int w, int h,
	unsigned char const* data)
{
  ((csGLFontCache*)fontCache)->FlushText ();

  int orig_x = x;
  int orig_y = y;

  // If vertical clipping is needed we skip the initial part.
  data += 4*w*(y-orig_y);
  // Same for horizontal clipping.
  data += 4*(x-orig_x);

  statecache->Disable_GL_TEXTURE_2D ();
  bool gl_alphaTest = glIsEnabled(GL_ALPHA_TEST);
  if (gl_alphaTest) statecache->Disable_GL_ALPHA_TEST ();

  glColor3f (0., 0., 0.);
#ifndef CS_USE_NEW_RENDERER
  //glRasterPos2i (x, Height-y-h+1);
  /*
    @@@ HACK Below is only correct when a render target was set:
    This sets up the screen so every drawing takes place in a rect in the
    upper left, but flipped. However, the raster position is transformed,
    but glDrawPixels() always takes those as the lower left dest coord (in
    window.) So it has to drawn h pixels farther down. Meaning, when NO
    render target is set, this code will simply draw at the wrong spot
    on the screen. On the other hand, only PTs use Blit() at the moment.
   */
  glRasterPos2i (x, Height-y+1);
#else
  //glRasterPos2i (x, Height-y-h);
  glRasterPos2i (x, Height-y);
#endif // CS_USE_NEW_RENDERER
  glDrawPixels (w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);

  if (gl_alphaTest) statecache->Enable_GL_ALPHA_TEST ();
}

unsigned char* csGraphics2DGLCommon::GetPixelAt (int x, int y)
{
  ((csGLFontCache*)fontCache)->FlushText ();

  /// left as Height-y-1 to keep within offscreen bitmap.
  /// but for opengl itself you'd need Height-y.
  return screen_shot ?
    (screen_shot + pfmt.PixelBytes * ((Height - y - 1) * Width + x)) : 0;
}

csImageArea *csGraphics2DGLCommon::SaveArea (int x, int y, int w, int h)
{
  ((csGLFontCache*)fontCache)->FlushText ();

  // For the time being copy data into system memory.
#ifndef GL_VERSION_1_2
  if (pfmt.PixelBytes != 1 && pfmt.PixelBytes != 4)
    return 0;
#endif
  // Convert to Opengl co-ordinate system
  y = Height - (y + h);

  if (x < 0)
  { w += x; x = 0; }
  if (x + w > Width)
    w = Width - x;
  if (y < 0)
  { h += y; y = 0; }
  if (y + h > Height)
    h = Height - y;
  if ((w <= 0) || (h <= 0))
    return 0;

  csImageArea *Area = new csImageArea (x, y, w, h);
  if (!Area)
    return 0;
  int actual_width = pfmt.PixelBytes * w;
  GLubyte* dest = new GLubyte [actual_width * h];
  Area->data = (char *)dest;
  if (!dest)
  {
    delete Area;
    return 0;
  }
  statecache->Disable_GL_TEXTURE_2D ();
  bool gl_alphaTest = glIsEnabled(GL_ALPHA_TEST);
  if (gl_alphaTest) statecache->Disable_GL_ALPHA_TEST ();
  //csGLStates::Disable_GL_DITHER ();
  GLenum format, type;
  switch (pfmt.PixelBytes)
  {
    case 1:
      format = GL_COLOR_INDEX;
      type = GL_UNSIGNED_BYTE;
      break;
#ifdef GL_VERSION_1_2
    case 2:
      format = GL_RGB;
      type = GL_UNSIGNED_SHORT_5_6_5;
      break;
#endif
    case 4:
      format = GL_RGBA;
      type = GL_UNSIGNED_BYTE;
      break;
    default:
      delete Area;
      return 0; // invalid format
  }
  glReadPixels (x, y, w, h, format, type, dest);

  if (gl_alphaTest) statecache->Enable_GL_ALPHA_TEST ();
  return Area;
}

void csGraphics2DGLCommon::RestoreArea (csImageArea *Area, bool Free)
{
  ((csGLFontCache*)fontCache)->FlushText ();

  statecache->Disable_GL_TEXTURE_2D ();
  bool gl_alphaTest = glIsEnabled(GL_ALPHA_TEST);
  if (gl_alphaTest) statecache->Disable_GL_ALPHA_TEST ();
  //csGLStates::Disable_GL_DITHER ();
  if (Area)
  {
    GLenum format, type;
    switch (pfmt.PixelBytes)
    {
      case 1:
        format = GL_COLOR_INDEX;
        type = GL_UNSIGNED_BYTE;
        break;
#ifdef GL_VERSION_1_2
      case 2:
        format = GL_RGB;
        type = GL_UNSIGNED_SHORT_5_6_5;
        break;
#endif
      case 4:
        format = GL_RGBA;
        type = GL_UNSIGNED_BYTE;
        break;
      default:
        return; // invalid format
    }
    glRasterPos2i (Area->x, Area->y);
    glDrawPixels (Area->w, Area->h, format, type, Area->data);
    glFlush ();
    if (Free)
      FreeArea (Area);
  } /* endif */

  if (gl_alphaTest) statecache->Enable_GL_ALPHA_TEST ();
}

csPtr<iImage> csGraphics2DGLCommon::ScreenShot ()
{
  ((csGLFontCache*)fontCache)->FlushText ();

/*#ifndef GL_VERSION_1_2
  if (pfmt.PixelBytes != 1 && pfmt.PixelBytes != 4)
    return 0;
#endif*/

  // Need to resolve pixel alignment issues
  int screen_width = Width * (4);
  if (!screen_shot) screen_shot = new uint8 [screen_width * Height];
  //if (!screen_shot) return 0;

  glReadPixels (0, 0, Width, Height, GL_RGBA,
    GL_UNSIGNED_BYTE, screen_shot);

  csGLScreenShot* ss = GetScreenShot ();
  ss->SetData (screen_shot);

  return ss;
}

bool csGraphics2DGLCommon::PerformExtensionV (char const* command, va_list args)
{
  if (!strcasecmp (command, "flush"))
  {
    glFlush ();
    glFinish ();
    return true;
  }
  if (!strcasecmp (command, "getstatecache"))
  {
    csGLStateCache** cache = va_arg (args, csGLStateCache**);
    *cache = statecache;
    return true;
  }
  if (!strcasecmp (command, "getextmanager"))
  {
    csGLExtensionManager** extmgr = va_arg (args, csGLExtensionManager**);
    *extmgr = &ext;
    return true;
  }
  if (!strcasecmp (command, "glflushtext"))
  {
    ((csGLFontCache*)fontCache)->FlushText ();
  }
  return false;
}

bool csGraphics2DGLCommon::DebugCommand (const char* cmdstr)
{
  CS_ALLOC_STACK_ARRAY(char, cmd, strlen (cmdstr) + 1);
  strcpy (cmd, cmdstr);
  char* param = 0;
  char* space = strchr (cmdstr, ' ');
  if (space)
  {
    param = space + 1;
    *space = 0;
  }

  if (strcasecmp (cmd, "dump_fontcache") == 0)
  {
    csRef<iImageIO> imgsaver = CS_QUERY_REGISTRY (object_reg, iImageIO);
    if (!imgsaver)
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
	"crystalspace.canvas.openglcommon",
        "Could not get image saver.");
      return false;
    }

    csRef<iVFS> vfs = CS_QUERY_REGISTRY (object_reg, iVFS);
    if (!vfs)
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
	"crystalspace.canvas.openglcommon",
	"Could not get VFS.");
      return false;
    }

    const char* dir = 
      ((param != 0) && (*param != 0)) ? param : "/tmp/fontcachedump/";
    csRefArray<iImage> images;
    ((csGLFontCache*)fontCache)->DumpFontCache (images);

    csString outfn;
    for (int i = 0; i < images.Length(); i++)
    {
      csRef<iDataBuffer> buf = imgsaver->Save (images[i], "image/png");
      if (!buf)
      {
	csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
	  "crystalspace.canvas.openglcommon",
	  "Could not save font cache page.");
      }
      else
      {
	outfn.Format ("%s%d.png", 
	  dir, i);
	if (!vfs->WriteFile (outfn, (char*)buf->GetInt8 (), buf->GetSize ()))
	{
	  csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
	    "crystalspace.canvas.openglcommon",
	    "Could not write to %s.", outfn.GetData ());
	}
	else
	{
	  csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
	    "crystalspace.canvas.openglcommon",
	    "Dumped font cache page to %s", outfn.GetData ());
	}
      }
    }

    return true;
  }

  return false;
}

bool csGraphics2DGLCommon::Resize (int width, int height)
{
  if (!is_open)
  {
    Width = width;
    Height = height;
    return true;
  }
  if (!AllowResizing)
    return false;

  ((csGLFontCache*)fontCache)->FlushText ();

  Width = width;
  Height = height;
  SetClipRect (0, 0, Width, Height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  EventOutlet->Broadcast (cscmdContextResize, (iGraphics2D *)this);
  return true;
}


