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

/**
 * The static methods of this class make up a PicoGUI graphics driver.
 * PicoGUI will call them, and they access the CS graphics canvas.
 * They may be used as C-style function pointers, for passing to PicoGUI.
 */
class csPGVideoDriver
{
 private:
  static csRef<iGraphics2D> Gfx2D;

 protected:
  static bool Construct (iGraphics2D *);
  friend class csPicoGUIServer;

 public:
  static g_error RegFunc (vidlib *);
  static g_error Init ();
  static g_error SetMode (int16 x, int16 y, int16 bpp, uint32 flags);
  static void Close ();
  static void CoordLogicalize (int *x, int *y);
  static void Update (stdbitmap *, int16 x, int16 y, int16 w, int16 h);
  static int IsRootless () { return 0; }
  static uint32 ColorPG2CS (uint32);
  static uint32 ColorCS2PG (uint32);
  static void Pixel (stdbitmap *, int16 x, int16 y, uint32 color, int16);
  static uint32 GetPixel (stdbitmap *, int16 x, int16 y);
  static void Slab (stdbitmap *, int16 x, int16 y, int16 w,
    uint32 color, int16);
  static void Bar (stdbitmap *, int16 x, int16 y, int16 h,
    uint32 color, int16);
  static void Line (stdbitmap *, int16 x1, int16 y1, int16 x2, int16 y2,
    uint32 color, int16);
  static void Rect (stdbitmap *, int16 x1, int16 y1, int16 x2, int16 y2,
    uint32 color, int16);
  static void Blit (stdbitmap *, int16 x, int16 y, int16 w, int16 h,
    stdbitmap *pic, int16 px, int16 py, int16);
  static g_error Load (stdbitmap **, const uint8 *data, uint32 len);
  static g_error New (stdbitmap **, int16 w, int16 h, uint16 bpp);
  static void Free (stdbitmap *hb) { delete GETBMP (hb); }
  static g_error GetSize (stdbitmap *, int16 *w, int16 *h);
  static g_error GetGropRender (stdbitmap *, groprender **);
  static g_error GetShareMem (stdbitmap *, uint32 uid, pgshmbitmap *info);
  static void SpriteShow (sprite *);
  static void SpriteHide (sprite *);
  static void SpriteRedraw (sprite *);
  static void CharBlit (stdbitmap *, uint8 *data,
    int16 x, int16 y, int16 w, int16 h, int16 lines, int16 angle,
    uint32 color, pgquad *clip, int16, int pitch);
  static void CharBlitAlpha (stdbitmap *, uint8 *data,
    int16 x, int16 y, int16 w, int16 h, int pitch, uint8 *gamma, int16 angle,
    uint32 color, pgquad *clip, int16);
};

#endif
