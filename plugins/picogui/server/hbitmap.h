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
  #include <picogui/types.h>
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
  struct groprender grop;
  int shmid;

 public:
  /// Construct a bitmap.
  inline csHwrBitmap (iGraphics2D *g2d0, int shmid0 = 0)
    : g2d (g2d0), shmid (shmid0) {}

  /// Get the CS 2d graphics class stored in this canvas.
  inline iGraphics2D * G2D () { return g2d; }

  /// Get the "grop render" structure associated with this bitmap.
  inline groprender* Grop () { return & grop; }

  /// Get the ID for a bitmap in shared memory.
  inline int ShmID () { return shmid; }
};

/// Cast a PicoGUI hwrbitmap (like a void*) to a csHwrBitmap pointer
#define GETBMP(PG) ((csHwrBitmap *) (PG))

/// Set a pointer-to-a-pointer to a hwrbitmap to a csHwrBitmap
#define SETBMP(PG, CS) (* (PG) = (stdbitmap *) (CS))

#endif
