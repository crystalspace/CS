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
#include "iutil/dbghelp.h"

struct iGraphics2D;
struct iGraphics3D;
class csString;

/**
 * A 2D bounding box with integer coordinates.
 */
class csBox2Int
{
public:
  int minx, miny;
  int maxx, maxy;
};

/**
 * The XOR Buffer.
 * This is a black-and-white bitmap represented by 32-bit ints
 * arranged in rows. For example, a 128x128 bitmap is represented
 * by 4 rows of 128 ints. Every int represents a column of 32 pixels.
 */
class csXORBuffer : public iBase
{
private:
  int width, height;
  int width_po2;	// Width after correcting for power of two.
  int w_shift;		// Horizontal shift for width_po2.
  int numrows;
  int bufsize;
  uint32* scr_buffer;	// The total screen buffer.
  uint32* buffer;	// The buffer for a single polygon.
  int* partcols;	// For every row the number of partial columns.

  bool Debug_ExtensiveTest (int num_iterations, csVector2* verts,
  	int& num_verts);
  bool Debug_TestOneIteration (csString& str);

  /**
   * Initialize the XOR polygon buffer to empty.
   */
  void InitializePolygonBuffer ();

  /**
   * Initialize the XOR polygon buffer to empty.
   */
  void InitializePolygonBuffer (const csBox2Int& bbox);

  /**
   * Draw a left-side line on the XOR buffer.
   * Normally a line is rendered upto but NOT including x2,y2 (i.e. the
   * scanline at y2 is ignored). With yfurther==1 you can go to y2. This
   * is useful for the lowest lines.
   */
  void DrawLeftLine (int x1, int y1, int x2, int y2, int yfurther = 0);

  /**
   * Draw a right-side line on the XOR buffer.
   * Normally a line is rendered upto but NOT including x2,y2 (i.e. the
   * scanline at y2 is ignored). With yfurther==1 you can go to y2. This
   * is useful for the lowest lines.
   */
  void DrawRightLine (int x1, int y1, int x2, int y2, int yfurther = 0);

  /**
   * Draw a polygon on the XOR buffer.
   * This function will not do any backface culling and it will work
   * perfectly in all orientations. Polygon has to be convex.
   * The optional 'shift' parameter indicates how many pixels the
   * polygon will be extended horizontally.
   * Returns false if polygon is outside screen.
   */
  bool DrawPolygon (csVector2* verts, int num_verts, csBox2Int& bbox,
  	int shift = 0);

  /**
   * Do a XOR sweep on the entire buffer.
   */
  void XORSweep ();

public:
  /// Create a new XOR buffer with the given dimensions.
  csXORBuffer (int w, int h);
  /// Destroy the XOR buffer.
  virtual ~csXORBuffer ();

  /// Setup XOR buffer for given size.
  void Setup (int w, int h);

  SCF_DECLARE_IBASE;

  /// Initialize the XOR buffer to empty.
  void Initialize ();

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
   * Return true if entire screen buffer is full.
   */
  bool IsFull ();

  // Debugging functions.
  iString* Debug_UnitTest ();
  csTicks Debug_Benchmark (int num_iterations);
  void Debug_Dump (iGraphics3D* g3d, int zoom = 1);

  struct DebugHelper : public iDebugHelper
  {
    SCF_DECLARE_EMBEDDED_IBASE (csXORBuffer);
    virtual int GetSupportedTests () const
    {
      return CS_DBGHELP_UNITTEST |
      	     CS_DBGHELP_BENCHMARK |
	     CS_DBGHELP_GFXDUMP;
    }
    virtual iString* UnitTest ()
    {
      return scfParent->Debug_UnitTest ();
    }
    virtual iString* StateTest ()
    {
      return NULL;
    }
    virtual csTicks Benchmark (int num_iterations)
    {
      return scfParent->Debug_Benchmark (num_iterations);
    }
    virtual iString* Dump ()
    {
      return NULL;
    }
    virtual void Dump (iGraphics3D* g3d)
    {
      scfParent->Debug_Dump (g3d, 1);
    }
    virtual bool DebugCommand (const char*)
    {
      return false;
    }
  } scfiDebugHelper;
};

#endif // __CS_XORBUF_H__

