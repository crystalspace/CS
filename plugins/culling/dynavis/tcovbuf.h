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

#ifndef __CS_TCOVBUF_H__
#define __CS_TCOVBUF_H__

#include "csgeom/vector2.h"
#include "csgeom/math2d.h"
#include "iutil/dbghelp.h"
#include "bits64.h"
#include "covbuf.h"

struct iGraphics2D;
struct iGraphics3D;
class csString;
class csBox2;
class csTiledCoverageBuffer;

// Line operations in a tile.
#define OP_LINE 1	// General line.
#define OP_VLINE 2	// Vertical line.
#define OP_FULLVLINE 3	// Full vertical line (from 0 to 63).

// A definition for a line operation.
struct csLineOperation
{
  uint8 op;		// One of OP_...
  // All coordinates are with 0,0 relative to top-left of tile.
  // x coordinates are also shifted 16 to the left.
  int x1;		// Start of line.
  int y1;		// Start of line. Not used with OP_FULLVLINE.
  int x2;		// End of line. Only used with OP_LINE.
  int y2;		// End of line. Not used with OP_FULLVLINE.
  int dx;		// Slope to add to x1 (shifted 16 to left).
};

/**
 * Coverage tile.
 * One tile is 32x64 pixels. Every tile is made from 4x8 blocks (so
 * one block is 8x8 pixels).
 */
class csCoverageTile
{
  friend class csTiledCoverageBuffer;

private:
  // If true entire tile is full.
  bool tile_full;
  // If true tile is queued as empty but 'coverage' and other
  // data structures may not yet reflect this.
  bool queue_tile_empty;

  // The coverage bits.
  csBits64 coverage[32];

  // The cache on which we will write lines before or-ing that to the
  // real coverage bits.
  static csBits64 coverage_cache[32];

  // This is an array of precalculated bit-sets for vertical line
  // segments that start at 'n' and go to 63.
  static csBits64 precalc_end_lines[64];
  // This is an array of precalculated bit-sets for vertical line
  // segments that start at 0 and go to 'n'.
  static csBits64 precalc_start_lines[64];
  // If true the two arrays above are initialized.
  static bool precalc_init;

  // For every block a depth value (4 blocks on every row, ordered
  // by rows).
  float depth[32];

  // For every 8x8 block a bit indicating if full.
  uint32 blocks_full;
  // For every 8x8 block a bit indicating if partial.
  uint32 blocks_partial;

  // Line Operations that are waiting to be executed.
  int num_operations;
  int max_operations;
  csLineOperation* operations;

  // Add an operation.
  csLineOperation& AddOperation ();

  // Check if the precalc tables are precalculated. If not
  // precalculate them.
  static void MakePrecalcTables ();

public:
  csCoverageTile () :
  	tile_full (false),
	queue_tile_empty (true),
	num_operations (0),
	max_operations (16)
  {
    operations = new csLineOperation [16];
    MakePrecalcTables ();
  }

  ~csCoverageTile ()
  {
    delete[] operations;
  }

  /**
   * Mark the tile as empty but don't perform any other cleaning
   * operations. MakeEmpty() will do that.
   */
  void MarkEmpty ()
  {
    queue_tile_empty = true;
  }

  /**
   * Really make the tile empty (as opposed to just marking it as
   * empty with queue_tile_empty).
   */
  void MakeEmpty ()
  {
    tile_full = false; queue_tile_empty = false;
    memset (coverage, 0, sizeof (csBits64)*32);
    memset (depth, 0, sizeof (float)*32);
    blocks_full = 0;
    blocks_partial = 0;
    num_operations = 0;
  }

  /**
   * Return true if tile is full.
   */
  bool IsFull () const { return tile_full; }

  /**
   * Add a general line operation to the operations queue.
   */
  void PushLine (int x1, int y1, int x2, int y2, int dx);

  /**
   * Add a vertical line operation to the operations queue.
   */
  void PushVLine (int x, int y1, int y2);

  /**
   * Add a full vertical line operation to the operations queue.
   */
  void PushFullVLine (int x);

  /**
   * Flush all operations in a tile given the fvalue from the
   * previous tile and return the new fvalue (also in fvalue).
   * This is the 1-bit implementation of Flush. A 3-bit implementation
   * will come later. This function will correctly handle the case where
   * the current tile is full. In that case only the fvalue will be
   * updated.
   */
  void Flush (csBits64& fvalue, float maxdepth);

  /**
   * Test if a given rectangle with exactly the size of this tile
   * (or bigger) is visible somewhere in this tile. If the tile is
   * not full this is automatically the case. If the tile is full
   * the given depth will be used. If the given depth is smaller or
   * equal than the maximum depth in the depth buffer then rectangle is
   * visible.
   */
  bool TestFullRect (float testdepth);

  /**
   * Test if a given point is visible in this tile. Coordinates
   * are given relative to top-left coordinate of this tile.
   */
  bool TestPoint (int x, int y, float testdepth);
};

/**
 * The tiled coverage Buffer.
 * This is a black-and-white bitmap represented by 32-bit ints
 * arranged in rows. For example, a 128x128 bitmap is represented
 * by 4 rows of 128 ints. Every int represents a column of 32 pixels.
 * In addition there is also a maximum depth value for every 8x8 pixels.
 * The screen buffer is divided into tiles of 64x32 pixels.
 */
class csTiledCoverageBuffer : public iBase
{
private:
  int width, height;
  int width_po2;	// Width after correcting for power of two.
  int height_64;	// Height after making it a multiple of 64.
  int w_shift;		// Horizontal shift for width_po2 for tile multiples.
  int num_tile_rows;

  // All tiles representing the screen (ordered by rows).
  int num_tiles;
  csCoverageTile* tiles;

  // For every row the following arrays contain the left-most and
  // right-most horizontal tile number that was affected by the polygon/outline.
  // DrawLine() will update these values.
  int* dirty_left;
  int* dirty_right;

  /**
   * Draw a line on the coverage buffer.
   * Normally a line is rendered upto but NOT including x2,y2 (i.e. the
   * scanline at y2 is ignored). With yfurther==1 you can go to y2. This
   * is useful for the lowest lines.
   */
  void DrawLine (int x1, int y1, int x2, int y2, int yfurther = 0);

  /**
   * Draw a polygon on the coverage buffer.
   * This function will not do any backface culling and it will work
   * perfectly in all orientations. Polygon has to be convex.
   * The optional 'shift' parameter indicates how many pixels the
   * polygon will be extended horizontally.
   * Returns false if polygon is outside screen.
   */
  bool DrawPolygon (csVector2* verts, int num_verts, csBox2Int& bbox);

  /**
   * Draw an outline on the coverage buffer.
   * Returns false if outline is outside screen.
   * The 'used_verts' contains true for all vertices that are used.
   */
  bool DrawOutline (csVector2* verts, int num_verts,
  	bool* used_verts,
  	int* edges, int num_edges, csBox2Int& bbox);

  /**
   * Find a given tile.
   */
  csCoverageTile* GetTile (int tx, int ty)
  {
    return &tiles[(ty<<w_shift) + tx];
  }

  /**
   * Mark a tile as dirty in dirty_left and dirty_right.
   */
  void MarkTileDirty (int tx, int ty)
  {
    if (tx < dirty_left[ty]) dirty_left[ty] = tx;
    if (tx > dirty_right[ty]) dirty_right[ty] = tx;
  }

public:
  /// Create a new coverage buffer with the given dimensions.
  csTiledCoverageBuffer (int w, int h);
  /// Destroy the coverage buffer.
  virtual ~csTiledCoverageBuffer ();

  /// Setup coverage buffer for given size.
  void Setup (int w, int h);

  SCF_DECLARE_IBASE;

  /// Initialize the coverage buffer to empty.
  void Initialize ();

  /**
   * Insert a polygon in the coverage buffer.
   * This function will not do any backface culling and it will work
   * perfectly in all orientations. Polygon has to be convex.
   * It will update the screen buffer.
   * Function returns false if polygon was not visible (i.e.
   * screen buffer was not modified).
   */
  void InsertPolygon (csVector2* verts, int num_verts, float max_depth);

  /**
   * Insert an outline in the coverage buffer.
   * It will update the screen buffer.
   * Function returns false if outline was not visible (i.e.
   * screen buffer was not modified).
   * The given array of edges is an array of two integers (vertex indices)
   * per edge.
   * The 'used_verts' contains true for all vertices that are used.
   */
  void InsertOutline (csVector2* verts, int num_verts,
  	bool* used_verts,
  	int* edges, int num_edges, float max_depth);

  /**
   * Test a rectangle with the coverage buffer.
   * Function returns false if rectangle was not visible (i.e.
   * screen buffer would not have been modified).
   */
  bool TestRectangle (const csBox2& rect, float min_depth);

  /**
   * Test a point with the coverage buffer.
   * Function returns false if point was not visible (i.e.
   * screen buffer would not have been modified).
   */
  bool TestPoint (const csVector2& point, float min_depth);

  // Debugging functions.
  iString* Debug_UnitTest ();
  csTicks Debug_Benchmark (int num_iterations);
  void Debug_Dump (iGraphics3D* g3d, int zoom = 1);
  iString* Debug_Dump ();

  struct DebugHelper : public iDebugHelper
  {
    SCF_DECLARE_EMBEDDED_IBASE (csTiledCoverageBuffer);
    virtual int GetSupportedTests () const
    {
      return CS_DBGHELP_UNITTEST |
      	     CS_DBGHELP_BENCHMARK |
	     CS_DBGHELP_GFXDUMP |
	     CS_DBGHELP_TXTDUMP;
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
      return scfParent->Debug_Dump ();
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

#endif // __CS_TCOVBUF_H__

