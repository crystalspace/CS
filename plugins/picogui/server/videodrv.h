/*
    Crystal Space Video Interface for PicoGUI
    (C) 2003 Mat Sutcliffe <oktal@gmx.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_PICOGUI_SERVER_VIDEODRV_H__
#define __CS_PICOGUI_SERVER_VIDEODRV_H__

#include "hbitmap.h"

struct vidlib;
struct sprite;
class csPicoGUIServer;
struct iImageIO;

/**
 * The static methods of this class make up a PicoGUI graphics driver.
 * PicoGUI will call them, and they access the CS graphics canvas.
 * They may be used as C-style function pointers, for passing to PicoGUI.
 */
class csPGVideoDriver
{
 private:
  static csRef<iGraphics3D> Gfx3D;
  static csRef<iImageIO> ImageIO;

 protected:
  static bool Construct (iGraphics3D *, iImageIO *);
  friend class csPicoGUIServer;

 public:
  static g_error RegFunc (vidlib *);
  static g_error Init ();

  // Unsigned long for flags due to MSVC/posix typedef differences
  // Works in gcc too, right? Otherwise someone on posix fix it :)
  // - Anders Stenberg
  static g_error SetMode (int16 x, int16 y, int16 bpp, __u32 flags);
  static int BeginDraw (struct divnode **div, struct gropnode ***listp,
    struct groprender *rend);
  static void Close ();
  static void CoordLogicalize (int *x, int *y);
  static void Update (hwrbitmap, int16 x, int16 y, int16 w, int16 h);
  static int IsRootless () { return 0; }
  static hwrcolor ColorPG2CS (pgcolor);
  static pgcolor ColorCS2PG (hwrcolor);
  static void Pixel (hwrbitmap, int16 x, int16 y, hwrcolor color, int16);
  static hwrcolor GetPixel (hwrbitmap, int16 x, int16 y);
  static void Slab (hwrbitmap, int16 x, int16 y, int16 w,
    hwrcolor color, int16);
  static void Bar (hwrbitmap, int16 x, int16 y, int16 h,
    hwrcolor color, int16);
  static void Line (hwrbitmap, int16 x1, int16 y1, int16 x2, int16 y2,
    hwrcolor color, int16);
  static void Rect (hwrbitmap, int16 x1, int16 y1, int16 x2, int16 y2,
    hwrcolor color, int16);
  static void Blit (hwrbitmap, int16 x, int16 y, int16 w, int16 h,
    stdbitmap *pic, int16 px, int16 py, int16);
  static g_error Load (hwrbitmap*, const uint8 *data, __u32 len);
  static g_error New (hwrbitmap*, int16 w, int16 h, uint16 bpp);
  static void Free (hwrbitmap hb);
  static g_error GetSize (hwrbitmap, int16 *w, int16 *h);
  static g_error GetGropRender (hwrbitmap, groprender **);
  static g_error GetShareMem (hwrbitmap, __u32 uid, pgshmbitmap *info);
  static void SpriteShow (sprite *);
  static void SpriteHide (sprite *);
  static void SpriteRedraw (sprite *);
  static void SpriteProtect (pgquad *, sprite *);
  static void CharBlit (hwrbitmap, uint8 *data,
    int16 x, int16 y, int16 w, int16 h, int16 lines, int16 angle,
    hwrcolor color, pgquad *clip, int16, int pitch);
  static void CharBlitAlpha (hwrbitmap, uint8 *data,
    int16 x, int16 y, int16 w, int16 h, int pitch, uint8 *gamma, int16 angle,
    hwrcolor color, pgquad *clip, int16);
};

#endif
