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
#include "cssys/csendian.h"
#include "video/canvas/common/scrshot.h"
#include "csgeom/csrect.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "qint.h"

// This header should be moved
#include "video/renderer/common/pixfmt.h"


SCF_IMPLEMENT_IBASE_EXT (csGraphics2DGLCommon)
  SCF_IMPLEMENTS_INTERFACE (iEventPlug)
SCF_IMPLEMENT_IBASE_EXT_END

csGraphics2DGLCommon::csGraphics2DGLCommon (iBase *iParent) :
  csGraphics2D (iParent), FontCache (NULL)
{
  EventOutlet = NULL;
}

bool csGraphics2DGLCommon::Initialize (iObjectRegistry *object_reg)
{
  if (!csGraphics2D::Initialize (object_reg))
    return false;

  // We don't really care about pixel format, except for ScreenShot()
  // and OpenGL software proctexes
#if (CS_24BIT_PIXEL_LAYOUT == CS_24BIT_PIXEL_ABGR)
    pfmt.RedMask = 0x000000FF;
    pfmt.GreenMask = 0x0000FF00;
    pfmt.BlueMask = 0x00FF0000;
#else 
    pfmt.RedMask = 0x00FF0000;
    pfmt.GreenMask = 0x0000FF00;
    pfmt.BlueMask = 0x000000FF;
#endif
  pfmt.PixelBytes = 4;
  pfmt.PalEntries = 0;
  pfmt.complete ();

  return true;
}

csGraphics2DGLCommon::~csGraphics2DGLCommon ()
{
  Close ();
  if (EventOutlet)
    EventOutlet->DecRef ();
}

bool csGraphics2DGLCommon::Open ()
{
  if (is_open) return true;
  // initialize font cache object
  if (!FontCache)
    FontCache = new GLFontCache (FontServer);

  if (!csGraphics2D::Open ())
    return false;

  const char *renderer = (const char *)glGetString (GL_RENDERER);
  const char *vendor = (const char *)glGetString (GL_VENDOR);
  const char *version = (const char *)glGetString (GL_VERSION);
  iReporter* reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (renderer || version || vendor)
    if (reporter)
      reporter->Report (CS_REPORTER_SEVERITY_NOTIFY,
        "crystalspace.canvas.openglcommon",
      	"OpenGL renderer: %s (vendor: %s) version %s",
        renderer ? renderer : "unknown", vendor ? vendor: "unknown", version ? version : "unknown");

  if (reporter)
    reporter->Report (CS_REPORTER_SEVERITY_NOTIFY,
        "crystalspace.canvas.openglcommon",
    	"Using %s mode at resolution %dx%d.",
	FullScreen ? "full screen" : "windowed", Width, Height);

  glClearColor (0., 0., 0., 0.);
  glClearDepth (-1.0);

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();

  glViewport (0, 0, Width, Height);
  Clear (0);
  if (reporter) reporter->DecRef ();

  return true;
}

void csGraphics2DGLCommon::Close ()
{
  if (!is_open) return;
  delete FontCache;
  FontCache = NULL;
  csGraphics2D::Close ();
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

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  glColor3f (1., 0., 0.);
  glClearColor (0., 0., 0., 0.);

  return true;
}

void csGraphics2DGLCommon::SetClipRect (int xmin, int ymin, int xmax, int ymax)
{
  csGraphics2D::SetClipRect (xmin, ymin, xmax, ymax);
  if (FontCache)
    FontCache->SetClipRect (xmin, Height - ymax, xmax, Height - ymin);
}

void csGraphics2DGLCommon::DecomposeColor (int iColor,
  GLubyte &oR, GLubyte &oG, GLubyte &oB)
{
  switch (pfmt.PixelBytes)
  {
    case 1: // paletted colors
      oR = Palette [iColor].red;
      oG = Palette [iColor].green;
      oB = Palette [iColor].blue;
      break;
    case 2: // 16bit color
    case 4: // truecolor
      oR = ((iColor & pfmt.RedMask  ) >> pfmt.RedShift  );
      oG = ((iColor & pfmt.GreenMask) >> pfmt.GreenShift);
      oB = ((iColor & pfmt.BlueMask ) >> pfmt.BlueShift );
      oR = oR << (8-pfmt.RedBits);
      oG = oG << (8-pfmt.GreenBits);
      oB = oB << (8-pfmt.BlueBits);
      break;
  }
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
  GLubyte r, g, b;
  DecomposeColor (color, r, g, b);
  glColor3ub (r, g, b);
}

void csGraphics2DGLCommon::Clear (int color)
{
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
  if (!ClipLine (x1, y1, x2, y2, ClipX1, ClipY1, ClipX2, ClipY2))
  {
    // prepare for 2D drawing--so we need no fancy GL effects!
    bool gl_texture2d = glIsEnabled(GL_TEXTURE_2D);
    bool gl_alphaTest = glIsEnabled(GL_ALPHA_TEST);
    if (gl_texture2d) glDisable (GL_TEXTURE_2D);
    if (gl_alphaTest) glDisable (GL_ALPHA_TEST);
    setGLColorfromint (color);

    // This is a workaround for a hard-to-really fix problem with OpenGL:
    // whole Y coordinates are "rounded" up, this leads to one-pixel-shift
    // compared to software line drawing. This is not exactly a bug (because
    // this is an on-the-edge case) but it's different, thus we'll slightly
    // shift whole coordinates down.
    if (QInt (y1) == y1) { y1 += 0.05; }
    if (QInt (y2) == y2) { y2 += 0.05; }

    glBegin (GL_LINES);
    glVertex2f (x1, Height - y1);
    glVertex2f (x2, Height - y2);
    glEnd ();

    if (gl_texture2d) glEnable (GL_TEXTURE_2D);
    if (gl_alphaTest) glEnable (GL_ALPHA_TEST);
  }
}

void csGraphics2DGLCommon::DrawBox (int x, int y, int w, int h, int color)
{
  if ((x > ClipX2) || (y > ClipY2))
    return;
  if (x < ClipX1)
    w -= (ClipX1 - x), x = ClipX1;
  if (y < ClipY1)
    h -= (ClipY1 - y), y = ClipY1;
  if (x + w > ClipX2)
    w = ClipX2 - x;
  if (y + h > ClipY2)
    h = ClipY2 - y;
  if ((w <= 0) || (h <= 0))
    return;

  y = Height - y;
  // prepare for 2D drawing--so we need no fancy GL effects!
  bool gl_texture2d = glIsEnabled(GL_TEXTURE_2D);
  if (gl_texture2d) glDisable (GL_TEXTURE_2D);
  setGLColorfromint (color);

  glBegin (GL_QUADS);
  glVertex2i (x, y);
  glVertex2i (x + w, y);
  glVertex2i (x + w, y - h);
  glVertex2i (x, y - h);
  glEnd ();

  if (gl_texture2d) glEnable (GL_TEXTURE_2D);
}

void csGraphics2DGLCommon::DrawPixel (int x, int y, int color)
{
  if ((x >= ClipX1) && (y < ClipX2) && (y >= ClipY1) && (y < ClipY2))
  {
    // prepare for 2D drawing--so we need no fancy GL effects!
    bool gl_texture2d = glIsEnabled(GL_TEXTURE_2D);
    if (gl_texture2d) glDisable (GL_TEXTURE_2D);
    setGLColorfromint(color);

    glBegin (GL_POINTS);
    glVertex2i (x, Height - y);
    glEnd ();

    if (gl_texture2d) glEnable (GL_TEXTURE_2D);
  }
}

void csGraphics2DGLCommon::Write (iFont *font, int x, int y, int fg, int bg,
  const char *text)
{
  bool gl_texture2d = glIsEnabled(GL_TEXTURE_2D);
  if (gl_texture2d) glDisable (GL_TEXTURE_2D);

  if (bg >= 0)
  {
    int fw, fh;
    font->GetDimensions (text, fw, fh);
    DrawBox (x, y, fw, fh, bg);
  }

  if (gl_texture2d) glEnable (GL_TEXTURE_2D);

  setGLColorfromint (fg);
  FontCache->Write (font, x, Height - y, text);
}

// This variable is usually NULL except when doing a screen shot:
// in this case it is a temporarily allocated buffer for glReadPixels ()
static uint8 *screen_shot = NULL;

unsigned char* csGraphics2DGLCommon::GetPixelAt (int x, int y)
{
  return screen_shot ?
    (screen_shot + pfmt.PixelBytes * ((Height - y) * Width + x)) : NULL;
}

csImageArea *csGraphics2DGLCommon::SaveArea (int x, int y, int w, int h)
{
  // For the time being copy data into system memory.
#ifndef GL_VERSION_1_2
  if (pfmt.PixelBytes != 1 && pfmt.PixelBytes != 4)
    return NULL;
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
    return NULL;

  csImageArea *Area = new csImageArea (x, y, w, h);
  if (!Area)
    return NULL;
  int actual_width = pfmt.PixelBytes * w;
  GLubyte* dest = new GLubyte [actual_width * h];
  Area->data = (char *)dest;
  if (!dest)
  {
    delete Area;
    return NULL;
  }
  bool gl_texture2d = glIsEnabled(GL_TEXTURE_2D);
  bool gl_alphaTest = glIsEnabled(GL_ALPHA_TEST);
  if (gl_texture2d) glDisable (GL_TEXTURE_2D);
  if (gl_alphaTest) glDisable (GL_ALPHA_TEST);
  //glDisable (GL_DITHER);
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
      return NULL; // invalid format
  }
  glReadPixels (x, y, w, h, format, type, dest);

  if (gl_texture2d) glEnable (GL_TEXTURE_2D);
  if (gl_alphaTest) glEnable (GL_ALPHA_TEST);
  return Area;
}

void csGraphics2DGLCommon::RestoreArea (csImageArea *Area, bool Free)
{
  bool gl_texture2d = glIsEnabled(GL_TEXTURE_2D);
  bool gl_alphaTest = glIsEnabled(GL_ALPHA_TEST);
  if (gl_texture2d) glDisable (GL_TEXTURE_2D);
  if (gl_alphaTest) glDisable (GL_ALPHA_TEST);
  //glDisable (GL_DITHER);
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

  if (gl_texture2d) glEnable (GL_TEXTURE_2D);
  if (gl_alphaTest) glEnable (GL_ALPHA_TEST);
}

iImage *csGraphics2DGLCommon::ScreenShot ()
{
#ifndef GL_VERSION_1_2
  if (pfmt.PixelBytes != 1 && pfmt.PixelBytes != 4)
    return NULL;
#endif

  // Need to resolve pixel alignment issues
  int screen_width = Width * pfmt.PixelBytes;
  screen_shot = new uint8 [screen_width * Height];
  if (!screen_shot) return NULL;

  // glPixelStore ()?
  switch (pfmt.PixelBytes)
  {
    case 1:
      glReadPixels (0, 0, Width, Height, GL_COLOR_INDEX,
		    GL_UNSIGNED_BYTE, screen_shot);
      break;
#ifdef GL_VERSION_1_2
    case 2:
      // experimental
      glReadPixels (0, 0, Width, Height, GL_RGB,
		    GL_UNSIGNED_SHORT_5_6_5, screen_shot);
      break;
#endif
    default:
      glReadPixels (0, 0, Width, Height, GL_RGBA,
		    GL_UNSIGNED_BYTE, screen_shot);
      break;
  }

// Pixel formats expect AXXX not XXXA so swap
// On ABGR machines, we also need to swap B/R bytes
  if (pfmt.PixelBytes == 4)
  {
    uint32* s = (uint32*)screen_shot;
    int i;
    for (i = 0 ; i < Width*Height ; i++)
    {
#if (CS_24BIT_PIXEL_LAYOUT == CS_24BIT_PIXEL_ABGR)
        *s = ((*s & 0x000000FF) << 24) | ((*s & 0x0000FF00) << 8) |
                ((*s & 0x00FF0000) >> 8) | ((*s & 0xFF000000) >> 24);
#else 
        *s = ((*s & 0xFF) << 24) | ((*s & 0xFFFFFF00) >> 8);
#endif
	s++;
    }
  }

  csScreenShot *ss = new csScreenShot (this);

  delete [] screen_shot;
  screen_shot = NULL;

  return ss;
}

bool csGraphics2DGLCommon::PerformExtensionV (char const* command, va_list)
{
  if (!strcasecmp (command, "flush"))
  {
    glFlush ();
    glFinish ();
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
    SetClipRect (0, 0, Width - 1, Height - 1);
    return true;
  }
  if (!AllowResizing)
    return false;
  Width = width;
  Height = height;
  SetClipRect (0, 0, Width - 1, Height - 1);
  EventOutlet->Broadcast (cscmdContextResize, (iGraphics2D *)this);
  return true;
}
