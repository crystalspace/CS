/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
    Written by Andrew Zabolotny <bit@eltech.ru>

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

#include <stdarg.h>
#include <stdlib.h>
#include "cssysdef.h"
#include "graph2d.h"
#include "csqint.h"
#include "scrshot.h"
#include "iutil/plugin.h"
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"
#include "iengine/texture.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "ivaria/reporter.h"

#include "softfontcache.h"

SCF_IMPLEMENT_IBASE(csGraphics2D)
  SCF_IMPLEMENTS_INTERFACE(iGraphics2D)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iConfig)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iNativeWindowManager)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iNativeWindow)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iDebugHelper)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGraphics2D::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGraphics2D::CanvasConfig)
  SCF_IMPLEMENTS_INTERFACE (iConfig)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGraphics2D::NativeWindow)
  SCF_IMPLEMENTS_INTERFACE (iNativeWindow)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGraphics2D::eiDebugHelper)
  SCF_IMPLEMENTS_INTERFACE (iDebugHelper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGraphics2D::NativeWindowManager)
  SCF_IMPLEMENTS_INTERFACE (iNativeWindowManager)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csGraphics2D::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

csGraphics2D::csGraphics2D (iBase* parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiConfig);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiNativeWindow);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiNativeWindowManager);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiDebugHelper);
  scfiEventHandler = 0;
  Memory = 0;
  LineAddress = 0;
  Palette = 0;
  Width = 640;
  Height = 480;
  Depth = 16;
  DisplayNumber = 0;
  FullScreen = false;
  is_open = false;
  win_title = csStrNew ("Crystal Space Application");
  object_reg = 0;
  AllowResizing = false;
  refreshRate = 0;
  vsync = false;
  fontCache = 0;
}

bool csGraphics2D::Initialize (iObjectRegistry* r)
{
  CS_ASSERT (r != 0);
  object_reg = r;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  // Get the system parameters
  config.AddConfig (object_reg, "/config/video.cfg");
  Width = config->GetInt ("Video.ScreenWidth", Width);
  Height = config->GetInt ("Video.ScreenHeight", Height);
  Depth = config->GetInt ("Video.ScreenDepth", Depth);
  FullScreen = config->GetBool ("Video.FullScreen", FullScreen);
  DisplayNumber = config->GetInt ("Video.DisplayNumber", DisplayNumber);
  refreshRate = config->GetInt ("Video.DisplayFrequency", 0);
  vsync = config->GetBool ("Video.VSync", false);

  // Get the font server: A missing font server is NOT an error
  if (!FontServer)
  {
    FontServer = CS_QUERY_REGISTRY (object_reg, iFontServer);
  }
#ifdef CS_DEBUG
  if (!FontServer)
  {
    csReport (r, CS_REPORTER_SEVERITY_WARNING,
      "crystalspace.graphics2d.common",
      "Canvas driver couldn't find a font server plugin!  "
      "This is normal if you don't want one (warning displays only in "
      "debug mode)");
  }
#endif

  Palette = new csRGBpixel [256];
  pfmt.PalEntries = 256;
  pfmt.PixelBytes = 1;
  // Initialize pointers to default drawing methods
  _DrawPixel = DrawPixel8;
  _GetPixelAt = GetPixelAt8;
  // Mark all slots in palette as free
  int i;
  for (i = 0; i < 256; i++)
  {
    PaletteAlloc [i] = false;
    Palette [i].red = 0;
    Palette [i].green = 0;
    Palette [i].blue = 0;
  }

  if (!scfiEventHandler)
    scfiEventHandler = new EventHandler (this);
  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (q != 0)
    q->RegisterListener (scfiEventHandler, CSMASK_Broadcast);

  return true;
}

bool csGraphics2D::Initialize (iObjectRegistry* r, int width, int height,
    int depth, void* memory, iOffscreenCanvasCallback* ofscb)
{
  CS_ASSERT (r != 0);
  object_reg = r;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  // Get the system parameters
  config.AddConfig (object_reg, "/config/video.cfg");
  Width = width;
  Height = height;
  Depth = depth;
  FullScreen = false;
  Memory = (unsigned char*)memory;

  // Get the font server: A missing font server is NOT an error
  if (!FontServer)
  {
    FontServer = CS_QUERY_REGISTRY (object_reg, iFontServer);
  }

  // Initialize pointers to default drawing methods
  _DrawPixel = DrawPixel8;
  _GetPixelAt = GetPixelAt8;

  Palette = new csRGBpixel [256];
  if (Depth == 8)
  {
    pfmt.PalEntries = 256;
    pfmt.PixelBytes = 1;
  }
  else if (Depth == 16)
  {
    _DrawPixel = DrawPixel16;
    _GetPixelAt = GetPixelAt16;

    // Set pixel format
    pfmt.RedMask   = 0x1f << 11;
    pfmt.GreenMask = 0x3f << 5;
    pfmt.BlueMask  = 0x1f;
    pfmt.PixelBytes = 2;
    pfmt.PalEntries = 0;
  }
  else if (Depth == 32)
  {
    _DrawPixel = DrawPixel32;
    _GetPixelAt = GetPixelAt32;

    // calculate CS's pixel format structure.
    pfmt.RedMask = 0xff << 16;
    pfmt.GreenMask = 0xff << 8;
    pfmt.BlueMask = 0xff;
    pfmt.PixelBytes = 4;
    pfmt.PalEntries = 0;
  }
  pfmt.complete ();
  // Mark all slots in palette as free
  int i;
  for (i = 0; i < 256; i++)
  {
    PaletteAlloc [i] = false;
    Palette [i].red = 0;
    Palette [i].green = 0;
    Palette [i].blue = 0;
  }

  scfiEventHandler = 0;

  csGraphics2D::ofscb = ofscb;

  return true;
}

void csGraphics2D::ChangeDepth (int d)
{
  if (Depth == d) return;
  Depth = d;
}

csGraphics2D::~csGraphics2D ()
{
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
    if (q != 0)
      q->RemoveListener (scfiEventHandler);
    scfiEventHandler->DecRef ();
  }
  Close ();
  delete [] Palette;
  delete [] win_title;

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiDebugHelper);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiNativeWindowManager);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiNativeWindow);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiConfig);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csGraphics2D::HandleEvent (iEvent& Event)
{
  if (Event.Type == csevBroadcast)
    switch (Event.Command.Code)
    {
      case cscmdSystemOpen:
      {
        Open ();
        return true;
      }
      case cscmdSystemClose:
      {
        Close ();
        return true;
      }
    }
  return false;
}

bool csGraphics2D::Open ()
{
  if (is_open) return true;
  is_open = true;

  FrameBufferLocked = 0;

  // Allocate buffer for address of each scan line to avoid multuplication
  LineAddress = new int [Height];
  if (LineAddress == 0) return false;

  // Initialize scanline address array
  int i,addr,bpl = Width * pfmt.PixelBytes;
  for (i = 0, addr = 0; i < Height; i++, addr += bpl)
    LineAddress[i] = addr;

  if (!fontCache)
  {
    if (pfmt.PixelBytes == 1)
    {
      fontCache = new csSoftFontCache8 (this);
    }
    else if (pfmt.PixelBytes == 2)
    {
      if (pfmt.GreenMask == 0x07e0)
      {
        fontCache = new csSoftFontCache16_565 (this);
      }
      else if (pfmt.GreenMask == 0x03e0)
      {
        fontCache = new csSoftFontCache16_555 (this);
      }
      else
      {
        fontCache = new csSoftFontCache16_NoAA (this);
      }
    }
    else if (pfmt.PixelBytes == 4)
    {
      fontCache = new csSoftFontCache32 (this);
    }
  }

  SetClipRect (0, 0, Width, Height);

  return true;
}

void csGraphics2D::Close ()
{
  if (!is_open) return;
  is_open = false;
  delete [] LineAddress;
  LineAddress = 0;
  delete fontCache;
  fontCache = 0;
}

bool csGraphics2D::BeginDraw ()
{
  FrameBufferLocked++;
  return true;
}

void csGraphics2D::FinishDraw ()
{
  if (FrameBufferLocked)
    FrameBufferLocked--;
  if (ofscb) ofscb->FinishDraw (this);
}

void csGraphics2D::Clear(int color)
{
  DrawBox (0, 0, Width, Height, color);
}

bool csGraphics2D::DoubleBuffer (bool Enable)
{
  return !Enable;
}

bool csGraphics2D::GetDoubleBufferState ()
{
  return false;
}

int csGraphics2D::GetPage ()
{
  return 0;
}

void csGraphics2D::ClearAll (int color)
{
  int CurPage = GetPage ();
  do
  {
    if (!BeginDraw ())
      break;
    Clear (color);
    FinishDraw ();
    Print ();
  } while (GetPage () != CurPage);
}

void csGraphics2D::DrawPixel8 (csGraphics2D *This, int x, int y, int color)
{
  if ((x >= This->ClipX1) && (x < This->ClipX2)
   && (y >= This->ClipY1) && (y < This->ClipY2))
    *(This->GetPixelAt (x, y)) = color;
}

void csGraphics2D::DrawPixel16 (csGraphics2D *This, int x, int y, int color)
{
  if ((x >= This->ClipX1) && (x < This->ClipX2)
   && (y >= This->ClipY1) && (y < This->ClipY2))
    *(short *)(This->GetPixelAt (x, y)) = color;
}

void csGraphics2D::DrawPixel32 (csGraphics2D *This, int x, int y, int color)
{
  if ((x >= This->ClipX1) && (x < This->ClipX2)
   && (y >= This->ClipY1) && (y < This->ClipY2))
    *(long *)(This->GetPixelAt (x, y)) = color;
}

void csGraphics2D::DrawPixels (
  csPixelCoord const* pixels, int num_pixels, int color)
{
  while (num_pixels > 0)
  {
    DrawPixel (pixels->x, pixels->y, color);
    pixels++;
    num_pixels--;
  }
}

void csGraphics2D::Blit (int x, int y, int w, int h,
    unsigned char const* data)
{
  bool hor_clip_needed = false;
  bool ver_clip_needed = false;
  int orig_x = x;
  int orig_y = y;
  int orig_w = w;
  if ((x > ClipX2) || (y > ClipY2))
    return;
  if (x < ClipX1)
    { w -= (ClipX1 - x), x = ClipX1; hor_clip_needed = true; }
  if (y < ClipY1)
    { h -= (ClipY1 - y), y = ClipY1; ver_clip_needed = true; }
  if (x + w > ClipX2)
    { w = ClipX2 - x; hor_clip_needed = true; }
  if (y + h > ClipY2)
    { h = ClipY2 - y; }
  if ((w <= 0) || (h <= 0))
    return;

  // If vertical clipping is needed we skip the initial part.
  if (ver_clip_needed)
    data += 4*w*(y-orig_y);
  // Same for horizontal clipping.
  if (hor_clip_needed)
    data += 4*(x-orig_x);

  int r, g, b, a;
  unsigned char const* d;
  switch (pfmt.PixelBytes)
  {
    case 1:
      while (h)
      {
	register uint8 *vram = GetPixelAt (x, y);
	int w2 = w;
	d = data;
	while (w2 > 0)
	{
	  r = *d++; g = *d++; b = *d++; d++;
	  *vram++ = FindRGB (r, g, b);
	  w2--;
	}
        data += 4*orig_w;
        y++; h--;
      }
      break;
    case 2:
      while (h)
      {
        register uint16 *vram = (uint16 *)GetPixelAt (x, y);
	int w2 = w;
	d = data;
	while (w2 > 0)
	{
	  r = *d++; g = *d++; b = *d++; d++;
	  *vram++ = FindRGB (r, g, b);
	  w2--;
	}
        data += 4*orig_w;
        y++; h--;
      } /* endwhile */
      break;
    case 4:
      while (h)
      {
        register uint32 *vram = (uint32 *)GetPixelAt (x, y);
	int w2 = w;
	d = data;
	while (w2 > 0)
	{
	  r = *d++; g = *d++; b = *d++; a = *d++;
	  *vram++ = FindRGB (r, g, b) | (a<<24);
	  w2--;
	}
        data += 4*orig_w;
        y++; h--;
      } /* endwhile */
      break;
  } /* endswitch */
}

#ifndef NO_DRAWLINE
void csGraphics2D::DrawLine (float x1, float y1, float x2, float y2, int color)
{
  if (ClipLine (x1, y1, x2, y2, ClipX1, ClipY1, ClipX2, ClipY2))
    return;

  int fx1 = csQint (x1), fx2 = csQint (x2),
      fy1 = csQint (y1), fy2 = csQint (y2);


  // Adjust the farthest margin
  //if (fx1 < fx2) { if (float (fx2) == x2) fx2--; }
  //else if (fx1 > fx2) { if (float (fx1) == x1) fx1--; }
  //if (fy1 < fy2) { if (float (fy2) == y2) fy2--; }
  //else if (fy1 > fy2) { if (float (fy1) == y1) fy1--; }


  if (fy1 == fy2)
  {
    if (fx2 - fx1)
    {
      if (fx1 > fx2) { int tmp = fx1; fx1 = fx2; fx2 = tmp; }
      int count = fx2 - fx1 + 1;
      switch (pfmt.PixelBytes)
      {
        case 1:
          memset (GetPixelAt (fx1, fy1), color, count);
          break;
        case 2:
        {
          uint16 *dest = (uint16 *)GetPixelAt (fx1, fy1);
          while (count--) *dest++ = color;
          break;
        }
        case 4:
        {
          uint32 *dest = (uint32 *)GetPixelAt (fx1, fy1);
          while (count--) *dest++ = color;
          break;
        }
      } /* endswitch */
    }
    else
      DrawPixel (fx1, fy1, color);
  }
  else if (abs (fx2 - fx1) > abs (fy2 - fy1))
  {
    // Transform floating-point format to 16.16 fixed-point
    fy1 = csQint16 (y1); fy2 = csQint16 (y2);

    if (fx1 > fx2)
    {
      int tmp = fx1; fx1 = fx2; fx2 = tmp;
      tmp = fy1; fy1 = fy2; fy2 = tmp;
    }

    // delta Y can be negative
    int deltay = (fy2 - fy1) / (fx2 - fx1 + 1);

#define H_LINE(pixtype)                     \
  {                             \
    int x, y;  \
    for (x = fx1, y = fy1 + deltay / 2; x <= fx2; x++)  \
    {                               \
      pixtype *p = (pixtype *)(Memory +             \
        (x * sizeof (pixtype) + LineAddress [y >> 16]));    \
      *p = color; y += deltay;                  \
    }                               \
  }

/*#define H_LINE(pixtype)                       \
  {                             \
    int x, y;  \
    for (x = fx1, y = fy1 + deltay / 2; x <= fx2; x++)  \
    {                               \
      DrawPixel(x, (y>>16), color); y += deltay;                    \
    }                               \ */

    switch (pfmt.PixelBytes)
    {
      case 1: H_LINE (uint8); break;
      case 2: H_LINE (uint16); break;
      case 4: H_LINE (uint32); break;
    } /* endswitch */

#undef H_LINE
  }
  else
  {
    // Transform floating-point format to 16.16 fixed-point
    fx1 = csQint16 (x1); fx2 = csQint16 (x2);

    if (fy1 > fy2)
    {
      int tmp = fy1; fy1 = fy2; fy2 = tmp;
      tmp = fx1; fx1 = fx2; fx2 = tmp;
    }

    // delta X can be negative
    int deltax = (fx2 - fx1) / (fy2 - fy1 + 1);

#define V_LINE(pixtype)                     \
  {                             \
    int x, y; \
    for (x = fx1 + deltax / 2, y = fy1; y <= fy2; y++)  \
    {                               \
      pixtype *p = (pixtype *)(Memory +             \
        ((x >> 16) * sizeof (pixtype) + LineAddress [y]));  \
      *p = color; x += deltax;                  \
    }                               \
  }

/*#define V_LINE(pixtype)                       \
  {                             \
    int x, y; \
    for (x = fx1 + deltax / 2, y = fy1; y <= fy2; y++)  \
    {                               \
      DrawPixel((x>>16), y, color); \
      x += deltax; \
    }                               \
  }*/

    switch (pfmt.PixelBytes)
    {
      case 1: V_LINE (uint8); break;
      case 2: V_LINE (uint16); break;
      case 4: V_LINE (uint32); break;
    } /* endswitch */

#undef V_LINE
  }
}
#endif

void csGraphics2D::DrawBox (int x, int y, int w, int h, int color)
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
  switch (pfmt.PixelBytes)
  {
    case 1:
      while (h)
      {
        memset (GetPixelAt (x, y), color, w);
        y++; h--;
      } /* endwhile */
      break;
    case 2:
      while (h)
      {
        register uint16 *dest = (uint16 *)GetPixelAt (x, y);
        register int count = w;
        while (count--) *dest++ = color;
        y++; h--;
      } /* endwhile */
      break;
    case 4:
      while (h)
      {
        register uint32 *dest = (uint32 *)GetPixelAt (x, y);
        register int count = w;
        while (count--) *dest++ = color;
        y++; h--;
      } /* endwhile */
      break;
  } /* endswitch */
}

void csGraphics2D::SetClipRect (int xmin, int ymin, int xmax, int ymax)
{
  if (xmin < 0) xmin = 0;
  else if (xmin > Width) xmin = Width;
  if (xmax < 0) xmax = 0;
  else if (xmax > Width) xmax = Width;
  if (ymin < 0) ymin = 0;
  else if (ymin > Height) ymin = Height;
  if (ymax < 0) ymax = 0;
  else if (ymax > Height) ymax = Height;
  ClipX1 = xmin; ClipX2 = xmax;
  ClipY1 = ymin; ClipY2 = ymax;
  
  fontCache->SetClipRect (ClipX1, ClipY1, ClipX2, ClipY2);
}

void csGraphics2D::GetClipRect (int &xmin, int &ymin, int &xmax, int &ymax)
{
  xmin = ClipX1; xmax = ClipX2;
  ymin = ClipY1; ymax = ClipY2;
}

/* helper function for ClipLine below */
bool csGraphics2D::CLIPt(float denom, float num, float& tE, float& tL)
{
    float t;

    if(denom > 0) {
        t = num / denom;
        if(t > tL) return false;
        else if(t > tE) tE = t;
    } else if(denom < 0) {
        t = num / denom;
        if(t < tE) return false;
        else if(t < tL) tL = t; // note: there is a mistake on this line in the C edition of the book!
    } else if(num > 0) return false;
    return true;
}

/* This function and the next one were taken
   from _Computer Graphics: Principals and Practice_ (2nd ed)
   by Foley et al
   This implements the Liang-Barsky efficient parametric
   line-clipping algorithm
*/
bool csGraphics2D::ClipLine (float &x0, float &y0, float &x1, float &y1,
                             int xmin, int ymin, int xmax, int ymax)
{
    // exclude the left/bottom edges (the Liang-Barsky algorithm will
    // clip to those edges exactly, whereas the documentation for
    // ClipLine specifies that the lower/bottom edges are excluded)
    xmax--;
    ymax--;

    float dx = x1 - x0;
    float dy = y1 - y0;
    bool visible = false;

    if(dx == 0 && dy == 0 && x0 >= xmin && y0 >= ymin && x0 < xmax && y0 < ymax) {
        visible = true;
    } else {
        float tE = 0.0;
        float tL = 1.0;
        if(CLIPt(dx, xmin - x0, tE, tL))
            if(CLIPt(-dx, x0 - xmax, tE, tL))
                if(CLIPt(dy, ymin - y0, tE, tL))
                    if(CLIPt(-dy, y0 - ymax, tE, tL))
                    {
                        visible = true;
                        if(tL < 1.0) {
                            x1 = x0 + tL * dx;
                            y1 = y0 + tL * dy;
                        }
                        if(tE > 0) {
                            x0 += tE * dx;
                            y0 += tE * dy;
                        }
                    }
    }
    return !visible;
}


csImageArea *csGraphics2D::SaveArea (int x, int y, int w, int h)
{
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
  w *= pfmt.PixelBytes;
  char *dest = Area->data = new char [w * h];
  if (!dest)
  {
    delete Area;
    return 0;
  }
  for (; h > 0; y++, h--)
  {
    unsigned char *VRAM = GetPixelAt (x, y);
    memcpy (dest, VRAM, w);
    dest += w;
  } /* endfor */
  return Area;
}

void csGraphics2D::RestoreArea (csImageArea *Area, bool Free)
{
  if (Area)
  {
    char *dest = Area->data;
    int x = Area->x, y = Area->y, w = Area->w, h = Area->h;
    w *= pfmt.PixelBytes;
    for (; h; y++, h--)
    {
      unsigned char *VRAM = GetPixelAt (x, y);
      memcpy (VRAM, dest, w);
      dest += w;
    } /* endfor */
    if (Free)
      FreeArea (Area);
  } /* endif */
}

void csGraphics2D::FreeArea (csImageArea *Area)
{
  if (Area)
  {
    if (Area->data)
      delete [] Area->data;
    delete Area;
  } /* endif */
}

void csGraphics2D::SetRGB (int i, int r, int g, int b)
{
  Palette[i].red = r;
  Palette[i].green = g;
  Palette[i].blue = b;
  PaletteAlloc[i] = true;
  if (ofscb) ofscb->SetRGB (this, i, r, g, b);
}

void csGraphics2D::Write (iFont *font, int x, int y, int fg, int bg, 
			  const char *text, uint flags) 
{ 
  fontCache->WriteString (font, x, y, fg, bg, (utf8_char*)text, flags);
}

void csGraphics2D::WriteBaseline (iFont *font, int x, int y, int fg, int bg, 
				  const char *text) 
{ 
  Write (font, x, y, fg, bg, text, CS_WRITE_BASELINE);
}

unsigned char *csGraphics2D::GetPixelAt8 (csGraphics2D *This, int x, int y)
{
  return (This->Memory + (x + This->LineAddress[y]));
}

unsigned char *csGraphics2D::GetPixelAt16 (csGraphics2D *This, int x, int y)
{
  return (This->Memory + (x + x + This->LineAddress[y]));
}

unsigned char *csGraphics2D::GetPixelAt32 (csGraphics2D *This, int x, int y)
{
  return (This->Memory + ((x<<2) + This->LineAddress[y]));
}

bool csGraphics2D::PerformExtensionV (char const*, va_list)
{
  return false;
}

bool csGraphics2D::PerformExtension (char const* command, ...)
{
  va_list args;
  va_start (args, command);
  bool rc = PerformExtensionV(command, args);
  va_end (args);
  return rc;
}

void csGraphics2D::GetPixel (int x, int y, uint8 &oR, uint8 &oG, uint8 &oB)
{
  oR = oG = oB = 0;

  if (x < 0 || y < 0 || x >= Width || y >= Height)
    return;

  uint8 *vram = GetPixelAt (x, y);
  if (!vram)
    return;

  if (pfmt.PalEntries)
  {
    uint8 pix = *vram;
    oR = Palette [pix].red;
    oG = Palette [pix].green;
    oB = Palette [pix].blue;
  }
  else
  {
    uint32 pix = 0;
    switch (pfmt.PixelBytes)
    {
      case 1: pix = *vram; break;
      case 2: pix = *(uint16 *)vram; break;
      case 4: pix = *(uint32 *)vram; break;
    }
    oR = ((pix & pfmt.RedMask)   >> pfmt.RedShift)   << (8 - pfmt.RedBits);
    oG = ((pix & pfmt.GreenMask) >> pfmt.GreenShift) << (8 - pfmt.GreenBits);
    oB = ((pix & pfmt.BlueMask)  >> pfmt.BlueShift)  << (8 - pfmt.BlueBits);
  }
}

csPtr<iImage> csGraphics2D::ScreenShot ()
{
  BeginDraw ();
  csScreenShot *ss = new csScreenShot (this);
  FinishDraw ();
  return ss;
}

void csGraphics2D::AlertV (int type, const char* title, const char* okMsg,
    const char* msg, va_list arg)
{
  (void)type; (void)title; (void)okMsg;
  printf ("ALERT: ");
  vprintf (msg, arg);
  fflush (stdout);
}

void csGraphics2D::NativeWindowManager::Alert (int type,
    const char* title, const char* okMsg, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  scfParent->AlertV (type, title, okMsg, msg, arg);
  va_end (arg);
}

iNativeWindow* csGraphics2D::GetNativeWindow ()
{
  return &scfiNativeWindow;
}

void csGraphics2D::SetTitle (const char* title)
{
  if (title == win_title) return;
  delete[] win_title;
  win_title = csStrNew (title);
}

bool csGraphics2D::Resize (int w, int h)
{
  if (!LineAddress)
  {
    // Still in Initialization phase, configuring size of canvas
    Width = w;
    Height = h;
    return true;
  }

  if (!AllowResizing)
    return false;

  if (Width != w || Height != h)
  {
    Width = w;
    Height = h;

    delete [] LineAddress;
    LineAddress = 0;

    // Allocate buffer for address of each scan line to avoid multuplication
    LineAddress = new int [Height];
    CS_ASSERT (LineAddress != 0);

    // Initialize scanline address array
    int i,addr,bpl = Width * pfmt.PixelBytes;
    for (i = 0, addr = 0; i < Height; i++, addr += bpl)
      LineAddress[i] = addr;

    SetClipRect (0, 0, Width, Height);
  }
  return true;
}

void csGraphics2D::SetFullScreen (bool b)
{
  if (FullScreen == b) return;
  FullScreen = b;
}

bool csGraphics2D::SetMousePosition (int x, int y)
{
  (void)x; (void)y;
  return false;
}

bool csGraphics2D::SetMouseCursor (csMouseCursorID iShape)
{
  return (iShape == csmcArrow);
}

bool csGraphics2D::SetMouseCursor (iImage *, const csRGBcolor*, int, int, 
                                   csRGBcolor, csRGBcolor)
{
  return false;
}


/**
 * A nice observation about the properties of the human eye:
 * Let's call the largest R or G or B component of a color "main".
 * If some other color component is much smaller than the main component,
 * we can change it in a large range without noting any change in
 * the color itself. Examples:
 * (128, 128, 128) - we note a change in color if we change any component
 * by 4 or more.
 * (192, 128, 128) - we should change of G or B components by 8 to note any
 * change in color.
 * (255, 128, 128) - we should change of G or B components by 16 to note any
 * change in color.
 * (255, 0, 0) - we can change any of G or B components by 32 and we
 * won't note any change.
 * Thus, we use this observation to create a palette that contains more
 * useful colors. We implement here a function to evaluate the "distance"
 * between two colors. tR,tG,tB define the color we are looking for (target);
 * sR, sG, sB define the color we're examining (source).
 */
static inline int rgb_dist (int tR, int tG, int tB, int sR, int sG, int sB)
{
  register int max = MAX (tR, tG);
  max = MAX (max, tB);

  sR -= tR; sG -= tG; sB -= tB;

  return R_COEF_SQ * sR * sR * (32 - ((max - tR) >> 3)) +
         G_COEF_SQ * sG * sG * (32 - ((max - tG) >> 3)) +
         B_COEF_SQ * sB * sB * (32 - ((max - tB) >> 3));
}

int csGraphics2D::FindRGBPalette (int r, int g, int b)
{
  // @@@ Not a very fast routine!
  int i;
  int best_dist = 1000000;
  int best_idx = -1;
  for (i = 0 ; i < 256 ; i++)
    if (PaletteAlloc[i])
    {
      int dist = rgb_dist (r, g, b, Palette[i].red, Palette[i].green,
        Palette[i].blue);
      if (dist == 0) return i;
      if (dist < best_dist)
      {
        best_dist = dist;
    best_idx = i;
      }
    }
  return best_idx;
}

csPtr<iGraphics2D> csGraphics2D::CreateOffscreenCanvas (
    void* memory, int width, int height, int depth,
    iOffscreenCanvasCallback* ofscb)
{
  csGraphics2D* g2d = new csGraphics2D (0);
  if (g2d->Initialize (object_reg, width, height, depth, memory,
    ofscb) && g2d->Open ())
  {
    return csPtr<iGraphics2D> (g2d);
  }
  else
  {
    delete g2d;
    return 0;
  }
}

bool csGraphics2D::DebugCommand (const char* cmd)
{
  return false;
}

//---------------------------------------------------------------------------

#define NUM_OPTIONS 3

static const csOptionDescription config_options [NUM_OPTIONS] =
{
  { 0, "depth", "Display depth", CSVAR_LONG },
  { 1, "fs", "Fullscreen if available", CSVAR_BOOL },
  { 2, "mode", "Window size or resolution", CSVAR_STRING },
};

bool csGraphics2D::CanvasConfig::SetOption (int id, csVariant* value)
{
  if (value->GetType () != config_options[id].type)
    return false;
  switch (id)
  {
    case 0: scfParent->ChangeDepth (value->GetLong ()); break;
    case 1: scfParent->SetFullScreen (value->GetBool ()); break;
    case 2:
    {
      const char* buf = value->GetString ();
      int wres, hres;
      if (sscanf (buf, "%dx%d", &wres, &hres) == 2)
        scfParent->Resize (wres, hres);
      break;
    }
    default: return false;
  }
  return true;
}

bool csGraphics2D::CanvasConfig::GetOption (int id, csVariant* value)
{
  switch (id)
  {
    case 0: value->SetLong (scfParent->Depth); break;
    case 1: value->SetBool (scfParent->FullScreen); break;
    case 2:
    {
      char buf[100];
      sprintf (buf, "%dx%d", scfParent->GetWidth (), scfParent->GetHeight ());
      value->SetString (buf);
      break;
    }
    default: return false;
  }
  return true;
}

bool csGraphics2D::CanvasConfig::GetOptionDescription
  (int idx, csOptionDescription* option)
{
  if (idx < 0 || idx >= NUM_OPTIONS)
    return false;
  *option = config_options[idx];
  return true;
}

