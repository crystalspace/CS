/*
    Copyright (C) 2002-2005 by Jorrit Tyberghein

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

#ifndef __CS_CSGEOM_TCOVBUF_H__
#define __CS_CSGEOM_TCOVBUF_H__

#include "csextern.h"
#include "csgeom/math2d.h"
#include "csgeom/transfrm.h"
#include "csgeom/vector2.h"
#include "iutil/dbghelp.h"

//#define SHIFT_TILECOL 5
//#define SHIFT_TILEROW 6

#define SHIFT_TILECOL 6
#define SHIFT_TILEROW 5

#define NUM_TILECOL (1<<SHIFT_TILECOL)
#define NUM_TILEROW (1<<SHIFT_TILEROW)
#define NUM_DEPTHROW (NUM_TILEROW/8)
#define NUM_DEPTHCOL (NUM_TILECOL/8)
#define NUM_DEPTH (NUM_DEPTHROW * NUM_DEPTHCOL)

#define TILECOL_EMPTY 0
#define TILECOL_FULL ((uint32)~0)

#define TEST_OCCLUDER_QUALITY 1

typedef uint32 csTileCol;

/**
 * A 2D bounding box with integer coordinates.
 */
class csBox2Int
{
public:
  int minx, miny;
  int maxx, maxy;
  csBox2Int& operator+= (const csBox2Int& box)
  {
    if (box.minx < minx) minx = box.minx;
    if (box.miny < miny) miny = box.miny;
    if (box.maxx > maxx) maxx = box.maxx;
    if (box.maxy > maxy) maxy = box.maxy;
    return *this;
  }
};

/**
 * A structure used by TestRectangle() and initialized by
 * PrepareTestRectangle().
 */
struct csTestRectData
{
  csBox2Int bbox;
  int startrow, endrow;
  int startcol, endcol;
  int start_x, end_x;
};

struct iGraphics2D;
struct iGraphics3D;
struct iBugPlug;
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
 * One tile is 32x64 or 64x32 pixels. Every tile is made from 4x8 or 8x4
 * blocks (so one block is 8x8 pixels).
 */
class CS_CRYSTALSPACE_EXPORT csCoverageTile
{
  friend class csTiledCoverageBuffer;

private:
  // If true entire tile is full.
  bool tile_full;
  // If true tile is queued as empty but 'coverage' and other
  // data structures may not yet reflect this.
  bool queue_tile_empty;

  // The coverage bits.
  csTileCol coverage[NUM_TILECOL];

  // The cache on which we will write lines before or-ing that to the
  // real coverage bits.
  static csTileCol coverage_cache[NUM_TILECOL];

  // This is an array of precalculated bit-sets for vertical line
  // segments that start at 'n' and go to 63.
  static csTileCol precalc_end_lines[NUM_TILEROW];
  // This is an array of precalculated bit-sets for vertical line
  // segments that start at 0 and go to 'n'.
  static csTileCol precalc_start_lines[NUM_TILEROW];
  // If true the two arrays above are initialized.
  static bool precalc_init;

  // For every block a depth value (4 blocks on every row, ordered
  // by rows).
  float depth[NUM_DEPTH];
  // Minimum depth of all blocks.
  float tile_min_depth;
  // Maximum depth of all blocks.
  float tile_max_depth;

  // Line Operations that are waiting to be executed.
  int num_operations;
  int max_operations;
  csLineOperation* operations;

  // A temporary values that are used to test if the objects in the write
  // queue can actually help cull the object.
  bool covered;
  bool fully_covered;

  // Add an operation.
  csLineOperation& AddOperation ();

  // Check if the precalc tables are precalculated. If not
  // precalculate them.
  static void MakePrecalcTables ();

  // Count how many objects were occluded away that covered this tile.
  int objects_culled;

public:
  csCoverageTile () :
  	tile_full (false),
	queue_tile_empty (true),
	num_operations (0),
	max_operations (16),
	covered (false)
  {
    operations = new csLineOperation [16];
    MakePrecalcTables ();
    MakeEmpty ();
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
    tile_full = false;
    objects_culled = 0;
  }

#define INIT_MIN_DEPTH     999999999.0f
#define INIT_MIN_DEPTH_CMP 999900000.0f

  /**
   * Really make the tile empty (as opposed to just marking it as
   * empty with queue_tile_empty).
   */
  void MakeEmpty ()
  {
    tile_full = false; queue_tile_empty = false;
    memset (coverage, 0, sizeof (csTileCol)*NUM_TILECOL);
    memset (depth, 0, sizeof (float)*NUM_DEPTH);
    tile_min_depth = INIT_MIN_DEPTH;
    tile_max_depth = 0;
    objects_culled = 0;
  }

  /**
   * Faster version of MakeEmpty() that assumes that several of the fields will
   * be correctly updated directly after calling this function. Don't call this
   * unless you know what you are doing!
   */
  void MakeEmptyQuick ()
  {
    queue_tile_empty = false;
    memset (depth, 0, sizeof (float)*NUM_DEPTH);
    tile_min_depth = INIT_MIN_DEPTH;
    tile_max_depth = 0;
    objects_culled = 0;
  }

  /**
   * Clear all operations.
   */
  void ClearOperations ()
  {
    num_operations = 0;
  }

  /**
   * Return true if tile is full.
   */
  bool IsFull () const { return tile_full; }

  /**
   * Return true if tile is surely empty.
   * This can return false even if tile is empty. So don't 100%
   * depend on this!
   */
  bool IsEmpty () const { return queue_tile_empty; }

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
   * Perform all operations in a tile and render them on the coverage_cache.
   */
  void PerformOperations ();

  /**
   * Flush all operations in a tile and render them on the coverage_cache.
   * This is the same as PerformOperations() except that the operations
   * are also cleared.
   */
  void FlushOperations ();

  /**
   * Perform all operations in a tile and only update the fvalue. This
   * version can be used if you know the tile is full.
   */
  void PerformOperationsOnlyFValue (csTileCol& fvalue);

  /**
   * Flush all operations in a tile and only update the fvalue. This
   * version can be used if you know the tile is full.
   * This is the same as PerformOperationsOnlyFValue() except that
   * the operations are also cleared.
   */
  void FlushOperationsOnlyFValue (csTileCol& fvalue);

  //-----------------------------------------------------------------

  /**
   * Flush all operations in a tile given the fvalue from the
   * previous tile and return the new fvalue (also in fvalue).
   * This is the 1-bit implementation of Flush. A 3-bit implementation
   * will come later. This function will correctly handle the case where
   * the current tile is full. In that case only the fvalue will be
   * updated.
   * Returns true if the tile was modified.
   */
  bool Flush (csTileCol& fvalue, float maxdepth);

  /**
   * Version of Flush that ignores depth.
   */
  bool FlushIgnoreDepth (csTileCol& fvalue);

  /**
   * Version of Flush that handles the case where the tile is empty.
   * Returns true if the tile was modified.
   */
  bool FlushForEmpty (csTileCol& fvalue, float maxdepth);

  /**
   * Version of Flush that handles the case where the tile is empty.
   * Returns true if the tile was modified. This version ignores depth.
   */
  bool FlushForEmptyNoDepth (csTileCol& fvalue);

  /**
   * Version of Flush that handles the case where the tile is full.
   * Returns true if the tile was modified.
   */
  bool FlushForFull (csTileCol& fvalue, float maxdepth);

  /**
   * Version of Flush that handles the case where there is no depth checking.
   * Returns true if the tile was modified.
   */
  bool FlushNoDepth (csTileCol& fvalue);

  /**
   * General Flush (slowest version).
   * Returns true if the tile was modified.
   */
  bool FlushGeneral (csTileCol& fvalue, float maxdepth);

  /**
   * Version of Flush that handles the case where the tile is empty.
   * This version is for a constant fvalue for the entire tile.
   */
  void FlushForEmptyConstFValue (csTileCol& fvalue, float maxdepth);

  /**
   * Version of Flush that handles the case where the tile is full.
   * This version is for a constant fvalue for the entire tile.
   */
  void FlushForFullConstFValue (csTileCol& fvalue, float maxdepth);

  /**
   * Version of Flush that handles the case where there is no depth checking.
   * This version is for a constant fvalue for the entire tile.
   * Returns true if the tile was modified.
   */
  bool FlushNoDepthConstFValue (csTileCol& fvalue, float maxdepth);

  /**
   * General Flush (slowest version).
   * This version is for a constant fvalue for the entire tile.
   * Returns true if the tile was modified.
   */
  bool FlushGeneralConstFValue (csTileCol& fvalue, float maxdepth);

  //-----------------------------------------------------------------

  /**
   * Perform a non-modifying flush and return true if Flush would
   * have affected the coverage buffer.
   */
  bool TestCoverageFlush (csTileCol& fvalue, float mindepth,
  	bool& do_depth_test);

  /**
   * Version of TestFlush that handles the case where the tile is full.
   */
  bool TestCoverageFlushForFull (csTileCol& fvalue, float mindepth,
  	bool& do_depth_test);

  /**
   * General TestFlush version (least efficient).
   */
  bool TestCoverageFlushGeneral (csTileCol& fvalue, float maxdepth,
  	bool& do_depth_test);

  //-----------------------------------------------------------------

  /**
   * Perform a non-modifying flush and return true if Flush would
   * have affected the coverage buffer.
   */
  bool TestDepthFlush (csTileCol& fvalue, float mindepth);

  /**
   * General TestFlush version (least efficient).
   */
  bool TestDepthFlushGeneral (csTileCol& fvalue, float maxdepth);

  //-----------------------------------------------------------------

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
   * Test if a given rectangle is visible. The rectangle is defined
   * as a set of full vertical columns from 'start' to 'end'.
   */
  bool TestDepthRect (int start, int end, float testdepth);

  /**
   * Test if a given rectangle is visible. The rectangle is defined
   * as the vertical mask from 'start' to 'end' horizontally (inclusive
   * range).
   */
  bool TestDepthRect (const csTileCol& vermask, int start, int end,
  	float testdepth);

  /**
   * Test if a given rectangle is visible. The rectangle is defined
   * as a set of full vertical columns from 'start' to 'end'.
   */
  bool TestCoverageRect (int start, int end, float testdepth,
  	bool& do_depth_test);

  /**
   * Test if a given rectangle is visible. The rectangle is defined
   * as the vertical mask from 'start' to 'end' horizontally (inclusive
   * range).
   */
  bool TestCoverageRect (const csTileCol& vermask, int start, int end,
  	float testdepth, bool& do_depth_test);

  //-----------------------------------------------------------------
  /**
   * Test if a given point is visible in this tile. Coordinates
   * are given relative to top-left coordinate of this tile.
   */
  bool TestPoint (int x, int y, float testdepth);

  /**
   * Give a textual dump of this tile.
   */
  csPtr<iString> Debug_Dump ();

  /**
   * Give a textual dump of the coverage cache.
   */
  csPtr<iString> Debug_Dump_Cache ();
};

/**
 * The tiled coverage Buffer.
 * This is a black-and-white bitmap represented by 32-bit ints
 * arranged in rows. For example, a 128x128 bitmap is represented
 * by 4 rows of 128 ints. Every int represents a column of 32 pixels.
 * In addition there is also a maximum depth value for every 8x8 pixels.
 * The screen buffer is divided into tiles of 64x32 or 32x64 pixels.
 */
class CS_CRYSTALSPACE_EXPORT csTiledCoverageBuffer : public iBase
{
public:
  iBugPlug* bugplug;	// For debugging...

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
  bool DrawPolygon (csVector2* verts, size_t num_verts, csBox2Int& bbox);

  /**
   * Draw an outline on the coverage buffer.
   * Returns false if outline is outside screen.
   * The 'used_verts' contains true for all vertices that are used.
   * If 'splat_outline' is true then outline splatting is used.
   */
  bool DrawOutline (const csReversibleTransform& trans,
  	float fov, float sx, float sy, csVector3* verts, size_t num_verts,
  	bool* used_verts,
  	int* edges, size_t num_edges, csBox2Int& bbox,
	float& max_depth, bool splat_outline);

  /**
   * Find a given tile.
   */
  csCoverageTile* GetTile (int tx, int ty)
  {
    CS_ASSERT (tx >= 0);
    CS_ASSERT (ty >= 0 && ty < num_tile_rows);
    return &tiles[(ty<<w_shift) + tx];
  }

  /**
   * Mark a tile as dirty in dirty_left and dirty_right.
   */
  void MarkTileDirty (int tx, int ty)
  {
    CS_ASSERT (ty >= 0 && ty < num_tile_rows);
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
   * Test if a polygon would modify the coverage buffer if it
   * was inserted.
   * This function will not do any backface culling and it will work
   * perfectly in all orientations. Polygon has to be convex.
   * It will NOT update the screen buffer.
   * Function returns false if polygon was not visible (i.e.
   * screen buffer was not modified).
   */
  bool TestPolygon (csVector2* verts, size_t num_verts, float min_depth);

  /**
   * Insert an inverted polygon in the coverage buffer.
   */
  void InsertPolygonInverted (csVector2* verts, size_t num_verts, float max_depth);

  /**
   * Insert an inverted polygon in the coverage buffer.
   * <p>
   * This function ignores depth in the depth buffer and should only
   * be used if you don't plan to use depth information nor depend on it.
   */
  void InsertPolygonInvertedNoDepth (csVector2* verts, size_t num_verts);

  /**
   * Insert a polygon in the coverage buffer.
   * This function will not do any backface culling and it will work
   * perfectly in all orientations. Polygon has to be convex.
   * It will update the screen buffer. 'modified_bbox' will be updated to
   * contain the union of the previous contents of 'modified_bbox' and also
   * the screen space box that was modified by this function.
   * <p>
   * If this function returns the number of tiles that were modified.
   */
  int InsertPolygon (csVector2* verts, size_t num_verts, float max_depth,
  	csBox2Int& modified_bbox);

  /**
   * Insert a polygon in the coverage buffer.
   * This function will not do any backface culling and it will work
   * perfectly in all orientations. Polygon has to be convex.
   * It will update the screen buffer.
   * <p>
   * If this function returns the number of tiles that were modified.
   * <p>
   * This function ignores depth in the depth buffer and should only
   * be used if you don't plan to use depth information nor depend on it.
   */
  int InsertPolygonNoDepth (csVector2* verts, size_t num_verts);

  /**
   * Insert an outline in the coverage buffer.
   * It will update the screen buffer. 'bbox' will contain the screen
   * space box that was modified by this function.
   * Function returns false if outline was not visible (i.e.
   * screen buffer was not modified).
   * The given array of edges is an array of two integers (vertex indices)
   * per edge.
   * The 'used_verts' contains true for all vertices that are used.
   * If 'splat_outline' is true then outline splatting is used.
   * <p>
   * If this function returns the number of tiles that were modified.
   */
  int InsertOutline (const csReversibleTransform& trans,
  	float fov, float sx, float sy, csVector3* verts, size_t num_verts,
  	bool* used_verts,
  	int* edges, size_t num_edges, bool splat_outline,
	csBox2Int& modified_bbox);

  /**
   * Prepare data for TestRectangle. If this returns false you have
   * an early exit since the rectangle cannot be visible regardless
   * of depth and coverage buffer contents.
   */
  bool PrepareTestRectangle (const csBox2& rect, csTestRectData& data);

  /**
   * Test a rectangle with the coverage buffer.
   * Function returns false if rectangle was not visible (i.e.
   * screen buffer would not have been modified).
   * Call PrepareTestRectangle() to fill in csTestRectData.
   */
  bool TestRectangle (const csTestRectData& data, float min_depth);

  /**
   * Quickly test a rectangle with the coverage buffer.
   * This is only a very rough test but it is faster then TestRectangle().
   * If this function returns false then the rectangle is not visible.
   * If this function returns true it is possible that the
   * rectangle is visible.
   */
  bool QuickTestRectangle (const csTestRectData& data, float min_depth);

  /**
   * Mark the given rectangle as being culled away. For every
   * affected tile this will increase the objects_culled field.
   */
  void MarkCulledObject (const csTestRectData& data);

  /**
   * Count the number of objects that were already culled away previously
   * for the given rectangle.
   */
  int CountNotCulledObjects (const csBox2Int& bbox);

  /**
   * Prepare a write queue test for the given rectangle. This returns the
   * number of uncovered tiles. Use AddWriteQueueTest() to add additional
   * rectangles.
   */
  int PrepareWriteQueueTest (const csTestRectData& data, float min_depth);

  /**
   * Add a rectangle for a write queue test. This returns the number of
   * tiles that were covered. If 'relevant' is set to false by this function
   * then that means the rectangle couldn't affect the result because it
   * only affects tiles that are already fully covering the object we are
   * testing.
   */
  int AddWriteQueueTest (const csTestRectData& maindata,
  	const csTestRectData& data, bool& relevant);

  /**
   * Test a point with the coverage buffer.
   * Function returns false if point was not visible (i.e.
   * screen buffer would not have been modified).
   */
  bool TestPoint (const csVector2& point, float min_depth);

  /**
   * Return status of coverage buffer (ignoring depth information).
   * If this returns 1 the buffer is full. If it returns -1 the buffer
   * is empty. If it returns 0 the buffer is partially full.
   */
  int StatusNoDepth ();

  // Debugging functions.
  csPtr<iString> Debug_UnitTest ();
  csTicks Debug_Benchmark (int num_iterations);
  void Debug_Dump (iGraphics3D* g3d, int zoom = 1);
  csPtr<iString> Debug_Dump ();

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
    virtual csPtr<iString> UnitTest ()
    {
      return scfParent->Debug_UnitTest ();
    }
    virtual csPtr<iString> StateTest ()
    {
      return 0;
    }
    virtual csTicks Benchmark (int num_iterations)
    {
      return scfParent->Debug_Benchmark (num_iterations);
    }
    virtual csPtr<iString> Dump ()
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

#endif // __CS_CSGEOM_TCOVBUF_H__

