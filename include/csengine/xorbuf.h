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

#include "csgeom/vector2.h"
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
  uint32* scr_buffer;	// The total screen buffer.
  uint32* buffer;	// The buffer for a single polygon.
  bool debug_mode;

  /**
   * Draw a single debug line (used by DrawPolygonDebug).
   */
  void Debug_DrawLine (iGraphics2D* g2d, float x1, float y1, float x2, float y2,
  	int col, float l, int zoom);

public:
  /// Create a new XOR buffer with the given dimensions.
  csXORBuffer (int w, int h);
  /// Destroy the XOR buffer.
  ~csXORBuffer ();

  /**
   * Initialize the XOR polygon buffer to empty.
   * This function is normally not called by user code.
   */
  void InitializePolygonBuffer ();

  /// Initialize the XOR buffer to empty.
  void Initialize ();

  /// Set debug mode on/off.
  void SetDebugMode (bool db) { debug_mode = db; }
  /// Get debug mode.
  bool IsDebugMode () const { return debug_mode; }

  /**
   * Draw a left-side line on the XOR buffer.
   * This function is normally not called by user code.
   */
  void DrawLeftLine (int x1, int y1, int x2, int y2);

  /**
   * Draw a right-side line on the XOR buffer.
   * This function is normally not called by user code.
   */
  void DrawRightLine (int x1, int y1, int x2, int y2);

  /**
   * Draw a polygon on the XOR buffer.
   * This function will not do any backface culling and it will work
   * perfectly in all orientations. Polygon has to be convex.
   * This function is normally not called by user code.
   */
  void DrawPolygon (csVector2* verts, int num_verts);

  /**
   * Insert a polygon in the XOR buffer.
   * This function will not do any backface culling and it will work
   * perfectly in all orientations. Polygon has to be convex.
   * It will update the screen buffer.
   * Function returns false if polygon was not visible (i.e.
   * screen buffer was not modified).
   * If 'negative' is true the polygon is inserted inverted.
   */
  bool InsertPolygon (csVector2* verts, int num_verts, bool negative = false);

  /**
   * Test a polygon with the XOR buffer.
   * This function will not do any backface culling and it will work
   * perfectly in all orientations. Polygon has to be convex.
   * It will not update the screen buffer but check if the polygon would
   * have modified it.
   * Function returns false if polygon was not visible (i.e.
   * screen buffer would not have been modified).
   */
  bool TestPolygon (csVector2* verts, int num_verts);

  /**
   * Do a XOR sweep on the entire buffer.
   * This function is normally not called by user code.
   */
  void XORSweep ();

  /**
   * Do a graphical dump of the XOR buffer contents on screen.
   */
  void Debug_Dump (iGraphics2D* g2d, iGraphics3D* g3d, int zoom = 1);

  /**
   * Do a graphical dump of the XOR screen buffer contents on screen.
   */
  void Debug_DumpScr (iGraphics2D* g2d, iGraphics3D* g3d, int zoom = 1);

  /**
   * Draw a polygon on g2d for debugging purposes.
   * Usually called after GfxDump() to match the xor'ed polygon
   * with the real polygon.
   */
  void Debug_DrawPolygon (iGraphics2D* g2d, iGraphics3D* g3d,
  	csVector2* verts, int num_verts, int zoom = 1);

  /**
   * This is a special test routine that just runs a
   * specified number of iterations and tries to find illegal
   * renderings. It works by rendering random polygons on the
   * XOR buffer and testing if a fill produces pixels at the
   * last column. This should never happen. If there was an
   * error before the iterations were finished this function will
   * return false and put the offending polygon in 'verts/num_verts'.
   * 'verts' should point to an array of vectors with at least room
   * for 6 vertices.
   */
  bool Debug_ExtensiveTest (int num_iterations, csVector2* verts,
  	int& num_verts);
};

#endif // __CS_XORBUF_H__

