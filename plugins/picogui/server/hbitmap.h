/*
    Crystal Space Canvas and Bitmap Interface for PicoGUI
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

#ifndef __CS_PICOGUI_SERVER_HBITMAP_H__
#define __CS_PICOGUI_SERVER_HBITMAP_H__

#include "ivideo/graph2d.h"
#include "igraphic/image.h"

extern "C"
{
  #include <picogui.h>
  #include <pgserver/types.h>
  #include <pgserver/render.h>
}

/**
 * This class describes a rendering canvas or bitmap.
 * PicoGUI will work with pointers to instances of this class.
 * It will pass those pointers to the static methods in csPGVideoDriver
 * and csPGFontEngine when it wants to perform operations on the
 * canvas or bitmap.
 */
class csHwrBitmap
{
 private:
  csRef<iGraphics2D> g2d;
  csRef<iImage> image;
  groprender grop;

 public:
  /// Construct a canvas.
  inline csHwrBitmap (iGraphics2D *g2d0) : g2d (g2d0) {}

  /// Construct a bitmap.
  inline csHwrBitmap (iImage *image0) : image (image0) {}

  /// Get the CS 2d graphics class stored in this canvas.
  inline csRef<iGraphics2D> G2D ()
    { return g2d; }

  /// Get the CS image class stored in this bitmap,
  /// or take a screenshot if it is a canvas.
  inline csRef<iImage> Image ()
    { if (image) return image; else return g2d->ScreenShot (); }

  /// Get the "grop render" structure associated with this bitmap.
  inline groprender* Grop ()
    { return & grop; }
};

/// Cast a PicoGUI hwrbitmap (like a void*) to a csHwrBitmap pointer
#define GETBMP(PG) ((csHwrBitmap *) (PG))

/// Set a pointer-to-a-pointer to a hwrbitmap to a csHwrBitmap
#define SETBMP(PG, CS) (* (PG) = (stdbitmap *) (CS))

#endif
