/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS_XORBUF_H__
#define __CS_XORBUF_H__

#include "csgeom/math2d.h"

struct iGraphics2D;
struct iGraphics3D;

/**
 * The XOR Buffer.
 * This is a black-and-white bitmap represented by 32-bit ints
 * arranged in rows. For example, a 128x128 bitmap is represented
 * by 4 rows of 128 ints. Every int represents a column of 32 pixels.
 */
class csXORBuffer
{
private:
  int width, height;
  int width_po2;	// Width after correcting for power of two.
  int w_shift;		// Horizontal shift for width_po2.
  int numrows;
  int bufsize;
  uint32* buffer;

public:
  /// Create a new XOR buffer with the given dimensions.
  csXORBuffer (int w, int h);
  /// Destroy the XOR buffer.
  ~csXORBuffer ();

  /// Initialize the XOR buffer to empty.
  void Initialize ();

  /**
   * Draw a left-side line on the XOR buffer.
   */
  void DrawLeftLine (int x1, int y1, int x2, int y2);

  /**
   * Do a graphical dump of the XOR buffer contents on screen.
   */
  void GfxDump (iGraphics2D* ig2d, iGraphics3D* ig3d);
};

#endif // __CS_XORBUF_H__

