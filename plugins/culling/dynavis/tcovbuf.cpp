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

#include "cssysdef.h"
#include "cssys/sysfunc.h"
#include "csutil/scfstr.h"
#include "iutil/string.h"
#include "qint.h"
#include "qsqrt.h"
#include "csgeom/box.h"
#include "csgeom/math3d.h"
#include "tcovbuf.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"

//---------------------------------------------------------------------------

csBits64 csCoverageTile::coverage_cache[32];
csBits64 csCoverageTile::precalc_end_lines[64];
csBits64 csCoverageTile::precalc_start_lines[64];
bool csCoverageTile::precalc_init = false;

void csCoverageTile::MakePrecalcTables ()
{
  if (precalc_init) return;
  precalc_init = true;
  int i, j;
  for (i = 0 ; i < 64 ; i++)
  {
    precalc_start_lines[i].Empty ();
    for (j = 0 ; j <= i ; j++)
      precalc_start_lines[i].XorBit (j);
    precalc_end_lines[i].Empty ();
    for (j = i ; j <= 63 ; j++)
      precalc_end_lines[i].XorBit (j);
  }
}

csLineOperation& csCoverageTile::AddOperation ()
{
  if (num_operations >= max_operations)
  {
    if (max_operations < 100)
      max_operations += max_operations;
    else
      max_operations += 100;
    csLineOperation* new_op = new csLineOperation [max_operations];
    memcpy (new_op, operations, sizeof (csLineOperation)*num_operations);
    delete[] operations;
    operations = new_op;
  }
  num_operations++;
  return operations[num_operations-1];
}

void csCoverageTile::PushLine (int x1, int y1, int x2, int y2, int dx)
{
  CS_ASSERT (x1 >= 0);
  CS_ASSERT (x1 < (32<<16));
  CS_ASSERT (x2 >= 0);
  CS_ASSERT (x2 < (32<<16));
  CS_ASSERT (y1 >= 0);
  CS_ASSERT (y1 < 64);
  CS_ASSERT (y2 >= 0);
  CS_ASSERT (y2 < 64);
  CS_ASSERT (x1+ABS (y2-y1)*dx >= 0);
  CS_ASSERT (x1+ABS (y2-y1)*dx < (32<<16));
  CS_ASSERT (x2-ABS (y1-y2)*dx >= 0);
  CS_ASSERT (x2-ABS (y1-y2)*dx < (32<<16));
  csLineOperation& op = AddOperation ();
  op.op = OP_LINE;
  op.x1 = x1;
  op.y1 = y1;
  op.x2 = x2;
  op.y2 = y2;
  op.dx = dx;
//printf ("        LINE %d,%d - %d,%d (dx=%d)\n", x1>>16, y1, x2>>16, y2, dx);
}

void csCoverageTile::PushVLine (int x, int y1, int y2)
{
  CS_ASSERT (x >= 0);
  CS_ASSERT (x < (32<<16));
  CS_ASSERT (y1 >= 0);
  CS_ASSERT (y1 < 64);
  CS_ASSERT (y2 >= 0);
  CS_ASSERT (y2 < 64);
  csLineOperation& op = AddOperation ();
  op.op = OP_VLINE;
  op.x1 = x;
  op.y1 = y1;
  op.y2 = y2;
//printf ("        VLINE %d    %d - %d\n", x>>16, y1, y2);
}

void csCoverageTile::PushFullVLine (int x)
{
  CS_ASSERT (x >= 0);
  CS_ASSERT (x < (32<<16));
  csLineOperation& op = AddOperation ();
  op.op = OP_FULLVLINE;
  op.x1 = x;
//printf ("        FULLVLINE %d\n", x>>16);
}

void csCoverageTile::FlushOperations ()
{
  int i;

  memset (coverage_cache, 0, sizeof (csBits64)*32);

  // First draw all lines.
  for (i = 0 ; i < num_operations ; i++)
  {
    csLineOperation& op = operations[i];
    if (op.op == OP_FULLVLINE)
    {
      CS_ASSERT (op.x1 >= 0 && op.x1 <= (32<<16));
      coverage_cache[op.x1 >> 16].Invert ();
    }
    else if (op.op == OP_VLINE)
    {
      CS_ASSERT (op.x1 >= 0 && op.x1 <= (32<<16));
      CS_ASSERT (op.y1 >= 0);
      CS_ASSERT (op.y1 <= 63);
      CS_ASSERT (op.y2 >= 0);
      CS_ASSERT (op.y2 <= 63);
      int y1, y2;
      if (op.y1 < op.y2) { y1 = op.y1; y2 = op.y2; }
      else { y1 = op.y2; y2 = op.y1; }
      const csBits64& start = precalc_start_lines[y2];
      const csBits64& end = precalc_end_lines[y1];
      // Xor the line with the coverage cache. This happens in three stages:
      csBits64& cc = coverage_cache[op.x1 >> 16];
      cc ^= start;
      cc ^= end;
      cc.Invert ();
    }
    else // OP_LINE
    {
      CS_ASSERT (op.x1 >= 0 && op.x1 <= (32<<16));
      CS_ASSERT (op.x2 >= 0 && op.x2 <= (32<<16));
      CS_ASSERT (op.y1 >= 0);
      CS_ASSERT (op.y1 <= 63);
      CS_ASSERT (op.y2 >= 0);
      CS_ASSERT (op.y2 <= 63);
      int x1, y1, x2, y2;
      if (op.y1 < op.y2) { x1 = op.x1; y1 = op.y1; x2 = op.x2; y2 = op.y2; }
      else { x1 = op.x2; y1 = op.y2; x2 = op.x1; y2 = op.y1; }
      int dy = y2-y1;
      int x = x1;
      int y = y1;
      int dx = op.dx;
      while (dy >= 0)
      {
	CS_ASSERT ((x>>16) >= 0);
	CS_ASSERT ((x>>16) < 32);
	csBits64& cc = coverage_cache[x >> 16];
	cc.XorBit (y);
        x += dx;
        y++;
        dy--;
      }
    }
  }

  // Clear all operations.
  num_operations = 0;
}

void csCoverageTile::Flush (csBits64& fvalue, float maxdepth)
{
  int i;

  if (queue_tile_empty)
    MakeEmpty ();

  if (tile_full)
  {
    // Special case. Only update the fvalue since the tile itself
    // is full.
    for (i = 0 ; i < num_operations ; i++)
    {
      csLineOperation& op = operations[i];
      if (op.op == OP_FULLVLINE)
      {
        // We have a full line (from top to bottom). In this case
	// we simply invert the fvalue.
	fvalue.Invert ();
      }
      else
      {
        // We can ignore the x value of the line here. So VLINE and
	// LINE are equivalent in this case.
	CS_ASSERT (op.y1 >= 0);
	CS_ASSERT (op.y1 <= 63);
	CS_ASSERT (op.y2 >= 0);
	CS_ASSERT (op.y2 <= 63);
	int y1, y2;
	// @@@ DO WE REALLY HAVE TO SWAP HERE??? I DON'T THINK SO!
	if (op.y1 < op.y2) { y1 = op.y1; y2 = op.y2; }
	else { y1 = op.y2; y2 = op.y1; }
	const csBits64& start = precalc_start_lines[y2];
	const csBits64& end = precalc_end_lines[y1];
	// Xor the line with the fvalue. This happens in three stages:
	fvalue ^= start;
	fvalue ^= end;
	fvalue.Invert ();
      }
    }
    num_operations = 0;
  }
  else
  {
    FlushOperations ();

    // Now perform the XOR sweep and OR with main coverage buffer.
    // fvalue will be the modified from left to right and will be
    // OR-ed with the main buffer. In the mean time the coverage_cache
    // buffer contents will be modified to be true wherever the
    // coverage_cache actually modified the coverage buffer.
    tile_full = true;	// Assume full for now.
    csBits64* cc = coverage_cache;
    csBits64* c = coverage;
    for (i = 0 ; i < 32 ; i++)
    {
      fvalue ^= *cc;
      *cc = fvalue;
      cc->AndInverted (*c);
      *c |= fvalue;
      if (tile_full && !c->IsFull ())
        tile_full = false;
      cc++;
      c++;
    }

    // Now do the depth update. Here we will use the coverage_cache
    // to see where we need to update the depth buffer. The coverage_cache
    // will now contain true wherever the coverage buffer was modified.
    for (i = 0 ; i < 4 ; i++)
    {
      float* ldepth = &depth[i];
      int idx = i << 3;
      int j = 1;
      csBits64 mods = coverage_cache[idx];
      while (j < 8)
      {
        mods |= coverage_cache[idx++];
	j++;
      }
      if (mods.CheckByte0 ())
      {
        float& d = ldepth[0];
	if (maxdepth > d) d = maxdepth;
      }
      if (mods.CheckByte1 ())
      {
        float& d = ldepth[4];
	if (maxdepth > d) d = maxdepth;
      }
      if (mods.CheckByte2 ())
      {
        float& d = ldepth[8];
	if (maxdepth > d) d = maxdepth;
      }
      if (mods.CheckByte3 ())
      {
        float& d = ldepth[12];
	if (maxdepth > d) d = maxdepth;
      }
      if (mods.CheckByte4 ())
      {
        float& d = ldepth[16];
	if (maxdepth > d) d = maxdepth;
      }
      if (mods.CheckByte5 ())
      {
        float& d = ldepth[20];
	if (maxdepth > d) d = maxdepth;
      }
      if (mods.CheckByte6 ())
      {
        float& d = ldepth[24];
	if (maxdepth > d) d = maxdepth;
      }
      if (mods.CheckByte7 ())
      {
        float& d = ldepth[28];
	if (maxdepth > d) d = maxdepth;
      }
    }
  }
}

bool csCoverageTile::TestFullRect (float testdepth)
{
  if (tile_full)
  {
    // Tile is full so check for depth.
    int i;
    for (i = 0 ; i < 32 ; i++)
      if (testdepth <= depth[i])
        return true;
    return false;
  }
  else
  {
    // Tile is not full which means the rectangle is automatically
    // visible.
    return true;
  }
}

bool csCoverageTile::TestPoint (int x, int y, float testdepth)
{
  CS_ASSERT (x >= 0 && x < 32);
  CS_ASSERT (y >= 0 && y < 64);

  // First check for depth.
  int xd = x >> 3;	// Depth x coordinate.
  int yd = y >> 3;	// Depth y coordinate.
  float d = depth[(yd << 2) + xd];
  if (testdepth <= d)
  {
    // Visible regardless of coverage.
    return true;
  }

  if (tile_full)
  {
    // If tile is full we know we are not visible because depth
    // has already been checked.
    return false;
  }

  const csBits64& c = coverage[x];
  return !c.TestBit (y);
}

iString* csCoverageTile::Debug_Dump ()
{
  scfString* rc = new scfString ();
  csString& str = rc->GetCsString ();

  csString ss;
  ss.Format ("full=%d queue_empty=%d blocks_full=%08lx blocks_part=%08lx\n",
  	tile_full, queue_tile_empty, blocks_full, blocks_partial);
  str.Append (ss);
  ss.Format ("  d %g,%g,%g,%g\n", depth[0], depth[1], depth[2], depth[3]);
  str.Append (ss);
  ss.Format ("  d %g,%g,%g,%g\n", depth[4], depth[5], depth[6], depth[7]);
  str.Append (ss);
  ss.Format ("  d %g,%g,%g,%g\n", depth[8], depth[9], depth[10], depth[11]);
  str.Append (ss);
  ss.Format ("  d %g,%g,%g,%g\n", depth[12], depth[13], depth[14], depth[15]);
  str.Append (ss);
  ss.Format ("  d %g,%g,%g,%g\n", depth[16], depth[17], depth[18], depth[19]);
  str.Append (ss);
  ss.Format ("  d %g,%g,%g,%g\n", depth[20], depth[21], depth[22], depth[23]);
  str.Append (ss);
  ss.Format ("  d %g,%g,%g,%g\n", depth[24], depth[25], depth[26], depth[27]);
  str.Append (ss);
  ss.Format ("  d %g,%g,%g,%g\n", depth[28], depth[29], depth[30], depth[31]);
  str.Append (ss);
  int i;
  for (i = 0 ; i < num_operations ; i++)
  {
    ss.Format ("  op %d ", i);
    str.Append (ss);
    csLineOperation& op = operations[i];
    switch (op.op)
    {
      case OP_LINE: ss.Format ("LINE %d,%d - %d,%d   dx=%d\n",
      	op.x1>>16, op.y1, op.x2>>16, op.y2, op.dx);
	str.Append (ss);
	break;
      case OP_VLINE: ss.Format ("VLINE x=%d y1=%d y2=%d\n",
      	op.x1>>16, op.y1, op.y2);
	str.Append (ss);
        break;
      case OP_FULLVLINE: ss.Format ("FULLVLINE x=%d\n", op.x1>>16);
        str.Append (ss);
        break;
      default: str.Append ("???\n");
        break;
    }
  }
  str.Append ("          1    1    2    2    3  \n");
  str.Append ("0    5    0    5    0    5    0  \n");
  for (i = 0 ; i < 64 ; i++)
  {
    int j;
    for (j = 0 ; j < 32 ; j++)
    {
      const csBits64& c = coverage[j];
      str.Append (c.TestBit (i) ? "#" : ".");
    }
    ss.Format (" %d\n", i);
    str.Append (ss);
  }

  return rc;
}

iString* csCoverageTile::Debug_Dump_Cache ()
{
  scfString* rc = new scfString ();
  csString& str = rc->GetCsString ();
  csString ss;

  int i;
  str.Append ("          1    1    2    2    3  \n");
  str.Append ("0    5    0    5    0    5    0  \n");
  for (i = 0 ; i < 64 ; i++)
  {
    int j;
    for (j = 0 ; j < 32 ; j++)
    {
      const csBits64& c = coverage_cache[j];
      str.Append (c.TestBit (i) ? "#" : ".");
    }
    ss.Format (" %d\n", i);
    str.Append (ss);
  }

  return rc;
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csTiledCoverageBuffer)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iDebugHelper)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTiledCoverageBuffer::DebugHelper)
  SCF_IMPLEMENTS_INTERFACE(iDebugHelper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csTiledCoverageBuffer::csTiledCoverageBuffer (int w, int h)
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiDebugHelper);

  tiles = NULL;
  dirty_left = NULL;
  dirty_right = NULL;

  Setup (w, h);
}

csTiledCoverageBuffer::~csTiledCoverageBuffer ()
{
  delete[] tiles;
  delete[] dirty_left;
  delete[] dirty_right;
}

void csTiledCoverageBuffer::Setup (int w, int h)
{
  delete[] tiles;
  delete[] dirty_left;
  delete[] dirty_right;

  width = w;
  height = h;
  num_tile_rows = (h+63)/64;
  height_64 = num_tile_rows * 64;
  CS_ASSERT (height_64 >= height);

  width_po2 = 1;
  w_shift = 0;
  while (width_po2 < width)
  {
    width_po2 <<= 1;
    w_shift++;
  }
  w_shift -= 5;
  CS_ASSERT (w_shift >= 0);

  num_tiles = (width_po2 / 32) * num_tile_rows;

  tiles = new csCoverageTile[num_tiles];
  dirty_left = new int[num_tile_rows];
  dirty_right = new int[num_tile_rows];
}

void csTiledCoverageBuffer::Initialize ()
{
  int i;
  for (i = 0 ; i < num_tiles ; i++)
  {
    tiles[i].MarkEmpty ();
    tiles[i].ClearOperations ();
  }
}

void csTiledCoverageBuffer::DrawLine (int x1, int y1, int x2, int y2,
	int yfurther)
{
//printf ("draw line %d,%d - %d,%d (yfurther=%d)\n", x1, y1, x2, y2, yfurther);
  y2 += yfurther;

  if (y2 < 0 || y1 >= height)
  {
    //------
    // Totally outside screen vertically.
    //------
    return;
  }

  if (x1 <= 0 && x2 <= 0)
  {
//printf ("  CLAMP\n");
    //------
    // Totally on the left side. Just clamp.
    //------

    // First we need to clip vertically. This is easy to do
    // in this particular case since x=0 all the time.
    if (y1 < 0) y1 = 0;
    if (y2 >= height) y2 = height-1;

    // First calculate tile coordinates of x1,y1 and x2,y2.
    int tile_y1 = y1 >> 6;
    int tile_y2 = (y2-1) >> 6;
    csCoverageTile* tile = GetTile (0, tile_y1);

    if (tile_y1 == tile_y2)
    {
//printf ("    ONE TILE\n");
      //------
      // All is contained in one tile.
      //------
      tile->PushVLine (0, y1 & 63, (y2-1) & 63);
      MarkTileDirty (0, tile_y1);
    }
    else
    {
//printf ("    MULTIPLE TILES\n");
      //------
      // Multiple tiles. First do first tile, then intermediate tiles,
      // and finally the last tile.
      //------
      tile->PushVLine (0, y1 & 63, 63);
      MarkTileDirty (0, tile_y1);
      int t;
      for (t = tile_y1+1 ; t < tile_y2 ; t++)
      {
        tile += width_po2 >> 5;
        tile->PushFullVLine (0);
	MarkTileDirty (0, t);
      }
      tile += width_po2 >> 5;
      tile->PushVLine (0, 0, (y2-1) & 63);
      MarkTileDirty (0, tile_y2);
    }
    return;
  }
  else if (x1 >= width && x2 >= width)
  {
//printf ("  DROP\n");
    //------
    // Lines on the far right can just be dropped since they
    // will have no effect on the coverage buffer.
    //------
    return;
  }
  else if (x1 == x2)
  {
//printf ("  FULLY VERTICAL\n");
    //------
    // If line is fully vertical we also have a special case that
    // is easier to resolve.
    //------
    // First we need to clip vertically. This is easy to do
    // in this particular case since x=0 all the time.
    if (y1 < 0) y1 = 0;
    if (y2 >= height) y2 = height-1;

    // First calculate tile coordinates of x1,y1 and x2,y2.
    int tile_x = x1 >> 5;	// tile_x1 == tile_x2
    int tile_y1 = y1 >> 6;
    int tile_y2 = (y2-1) >> 6;
    x1 &= 31;
    x1 <<= 16;

    csCoverageTile* tile = GetTile (tile_x, tile_y1);
    if (tile_y1 == tile_y2)
    {
//printf ("    ONE TILE\n");
      //------
      // All is contained in one tile.
      //------
      tile->PushVLine (x1, y1 & 63, (y2-1) & 63);
      MarkTileDirty (tile_x, tile_y1);
    }
    else
    {
//printf ("    MULTIPLE TILES\n");
      //------
      // Multiple tiles. First do first tile, then intermediate tiles,
      // and finally the last tile.
      //------
      tile->PushVLine (x1, y1 & 63, 63);
      MarkTileDirty (tile_x, tile_y1);
      int t;
      for (t = tile_y1+1 ; t < tile_y2 ; t++)
      {
        tile += width_po2 >> 5;
	tile->PushFullVLine (x1);
	MarkTileDirty (tile_x, t);
      }
      tile += width_po2 >> 5;
      tile->PushVLine (x1, 0, (y2-1) & 63);
      MarkTileDirty (tile_x, tile_y2);
    }
    return;
  }

  //------
  // We don't have any of the trivial vertical cases.
  // So we must clip vertically first.
  //------
  if (y1 < 0)
  {
    x1 = x1 + ( (0-y1) * (x2-x1) ) / (y2-yfurther-y1);
    y1 = 0;
  }
  if (y2 >= height)
  {
    x2 = x1 + ( (height-1-y1) * (x2-x1) ) / (y2-yfurther-y1);
    y2 = height-1;
    yfurther = 0;
  }
//printf ("  GENERAL CASE after clip: %d,%d - %d,%d\n", x1, y1, x2, y2);
  if (y1 == y2) return;	// Return if clipping results in one pixel.

  //------
  // First calculate tile coordinates of x1,y1 and x2,y2.
  //------
  int tile_x1 = x1 >> 5;
  int tile_y1 = y1 >> 6;
  int tile_x2 = x2 >> 5;
  int tile_y2 = (y2-1) >> 6;

# define xmask ((32<<16)-1)

  if (tile_x1 == tile_x2 && tile_y1 == tile_y2)
  {
//printf ("    ONE TILE\n");
    //------
    // Easy case. The line segment is fully inside one tile.
    //------
    int dy = y2-y1;
    int dx = ((x2-x1)<<16) / (dy-yfurther);
    csCoverageTile* tile = GetTile (tile_x1, tile_y1);
    tile->PushLine ((x1 & 31) << 16, y1 & 63, ((x2 & 31) << 16)-dx,
    	(y2-1) & 63, dx);
    MarkTileDirty (tile_x1, tile_y1);
    return;
  }
  else if (tile_x1 == tile_x2)
  {
//printf ("    NEARLY VERTICAL\n");
    //------
    // Line is nearly vertical. This means we will stay in the same
    // column of tiles.
    //------
    int dy = y2-y1;
    int dx = ((x2-x1)<<16) / (dy-yfurther);
    x1 <<= 16;
    x2 <<= 16;
    int x = x1 + dx * (63 - (y1 & 63));
    csCoverageTile* tile = GetTile (tile_x1, tile_y1);
    tile->PushLine (x1 & xmask, y1 & 63, x & xmask, 63, dx);
    MarkTileDirty (tile_x1, tile_y1);
    x += dx;
    int t;
    for (t = tile_y1+1 ; t < tile_y2 ; t++)
    {
      tile += width_po2 >> 5;
      int xt = x + (dx << 6) - dx;
      tile->PushLine (x & xmask, 0, xt & xmask, 63, dx);
      MarkTileDirty (tile_x1, t);
      x = xt+dx;
    }
    tile += width_po2 >> 5;
    tile->PushLine (x & xmask, 0, (x2 & xmask) - dx, (y2-1) & 63, dx);
    MarkTileDirty (tile_x1, tile_y2);
    return;
  }

  //------
  // This is the most general case and it is extremely slow.
  // @@@ NEED A BETTER ALGO HERE!!!
  //------
//printf ("    MOST GENERIC\n");
  int dy = y2-y1;

  int x = x1<<16;
  int y = y1;
  int dx = ((x2-x1)<<16) / (dy-yfurther);

  //------
  // First we check if the first part of the line is out of
  // screen (to the right). If that's the case we first do a small loop to
  // skip that initial part. @@@ Not efficient!
  //------
//if (dy > 0 && x >= (width<<16))printf ("      SKIP CLIP-RIGHT\n");
  while (dy > 0 && x >= (width<<16))
  {
    x += dx;
    y++;
    dy--;
  }

  if (dy <= 0) return;

  int last_x = x;
  int last_y = y;
  int cur_tile_x = x >> (16+5);
  int cur_tile_y = y >> 6;
  bool need_to_finish;

  //------
  // Then we check if there is an initial line segment where x is
  // out of screen to the left. If that's the case we need to clamp.
  // @@@ Not efficient!
  //------
  if (x <= 0)
  {
    need_to_finish = false;
//if (dy > 0 && x <= 0)printf ("      SKIP CLAMP-LEFT\n");
    while (dy > 0 && x <= 0)
    {
      need_to_finish = true;
      int tile_y = y >> 6;
      if (cur_tile_y != tile_y)
      {
        csCoverageTile* tile = GetTile (0, cur_tile_y);
        tile->PushVLine (0, last_y & 63, (y-1) & 63);
        MarkTileDirty (0, cur_tile_y);
        cur_tile_y = tile_y;
        last_y = y;
      }

      x += dx;
      y++;
      dy--;
    }

    if (need_to_finish)
    {
      //int tile_y = (y-1) >> 6;
      csCoverageTile* tile = GetTile (0, cur_tile_y);
      tile->PushVLine (0, last_y & 63, (y-1) & 63);
      MarkTileDirty (0, cur_tile_y);
    }
  }

  if (dy <= 0) return;

  //------
  // At this point we know that:
  //    x,y is the first point of the line that actually is on screen.
  //    x is shifted 16 pixels to the left.
  //    dy contains the number of lines left to process.
  //    dx is the slope of the line.
  //------

  last_x = x;
  last_y = y;
  cur_tile_x = x >> (16+5);
  cur_tile_y = y >> 6;

//printf ("      REMAINDER %d,%d\n", x>>16, y);

  //------
  // Here is the remainder of the line until we go out screen again.
  //------
  need_to_finish = false;
  while (dy > 0 && x > 0 && x < (width<<16))
  {
    need_to_finish = true;
    int tile_x = x >> (16+5);
    int tile_y = y >> 6;
//printf ("        dy=%d x>>16=%d y=%d tile_x=%d tile_y=%d\n", dy, x>>16, y, tile_x, tile_y);
    if (cur_tile_x != tile_x || cur_tile_y != tile_y)
    {
//printf ("        cur_tile_x=%d cur_tile_y=%d\n", tile_x, tile_y);
      csCoverageTile* tile = GetTile (cur_tile_x, cur_tile_y);
      tile->PushLine (last_x & xmask, last_y & 63, (x-dx) & xmask,
      	(y-1) & 63, dx);
      MarkTileDirty (cur_tile_x, cur_tile_y);
      cur_tile_x = tile_x;
      cur_tile_y = tile_y;
      last_x = x;
      last_y = y;
    }

    x += dx;
    y++;
    dy--;
  }

  if (need_to_finish)
  {
//printf ("        need_to_finish!\n");
    //int tile_x = (x-dx) >> (16+5);
    //int tile_y = (y-1) >> 6;
    csCoverageTile* tile = GetTile (cur_tile_x, cur_tile_y);
    tile->PushLine (last_x & xmask, last_y & 63, (x-dx) & xmask,
    	(y-1) & 63, dx);
    MarkTileDirty (cur_tile_x, cur_tile_y);
  }

  if (dy <= 0) return;

  //------
  // Now we need to check if there is a remaining part of the line
  // that we have to clamp.
  //------
  if (dy > 0 && x <= 0)
  {
    last_x = x;
    last_y = y;
    cur_tile_y = y >> 6;

    need_to_finish = false;
    while (dy > 0 && x <= 0)
    {
      need_to_finish = true;
      int tile_y = y >> 6;
      if (cur_tile_y != tile_y)
      {
        csCoverageTile* tile = GetTile (0, cur_tile_y);
        tile->PushVLine (0, last_y & 63, (y-1) & 63);
        MarkTileDirty (0, cur_tile_y);
        cur_tile_y = tile_y;
        last_y = y;
      }

      x += dx;
      y++;
      dy--;
    }
    CS_ASSERT (x <= 0);

    if (need_to_finish)
    {
      //int tile_y = (y-1) >> 6;
      csCoverageTile* tile = GetTile (0, cur_tile_y);
      tile->PushVLine (0, last_y & 63, (y-1) & 63);
      MarkTileDirty (0, cur_tile_y);
    }
  }
}

bool csTiledCoverageBuffer::DrawPolygon (csVector2* verts, int num_verts,
	csBox2Int& bbox)
{
  int i, j;

  //---------
  // First we copy the vertices to xa/ya. In the mean time
  // we convert to integer and also search for the top vertex (lowest
  // y coordinate) and bottom vertex.
  //@@@ TODO: pre-shift x with 16
  //---------
  int xa[128], ya[128];
  int top_vt = 0;
  int bot_vt = 0;
  xa[0] = QRound (verts[0].x);
  ya[0] = QRound (verts[0].y);
  bbox.minx = bbox.maxx = xa[0];
  bbox.miny = bbox.maxy = ya[0];
  for (i = 1 ; i < num_verts ; i++)
  {
    xa[i] = QRound (verts[i].x);
    ya[i] = QRound (verts[i].y);

    if (xa[i] < bbox.minx) bbox.minx = xa[i];
    else if (xa[i] > bbox.maxx) bbox.maxx = xa[i];

    if (ya[i] < bbox.miny)
    {
      bbox.miny = ya[i];
      top_vt = i;
    }
    else if (ya[i] > bbox.maxy)
    {
      bbox.maxy = ya[i];
      bot_vt = i;
    }
  }

  if (bbox.maxx <= 0) return false;
  if (bbox.maxy <= 0) return false;
  if (bbox.minx >= width) return false;
  if (bbox.miny >= height) return false;

  //---------
  // First initialize dirty_left and dirty_right for every row.
  //---------
  for (i = 0 ; i < num_tile_rows ; i++)
  {
    dirty_left[i] = 1000;
    dirty_right[i] = -1;
  }

  //---------
  // Draw all lines.
  //---------
  j = num_verts-1;
  for (i = 0 ; i < num_verts ; i++)
  {
    if (ya[i] != ya[j])
    {
      int xa1, xa2, ya1, ya2;
      if (ya[i] < ya[j])
      {
	xa1 = xa[i];
        xa2 = xa[j];
	ya1 = ya[i];
	ya2 = ya[j];
      }
      else
      {
	xa1 = xa[j];
        xa2 = xa[i];
	ya1 = ya[j];
	ya2 = ya[i];
      }
      DrawLine (xa1, ya1, xa2, ya2, ya2 == bbox.maxy ? 1 : 0);
    }
    j = i;
  }

  return true;
}

bool csTiledCoverageBuffer::DrawOutline (csVector2* verts, int num_verts,
	bool* used_verts,
	int* edges, int num_edges,
	csBox2Int& bbox)
{
  int i;

  //---------
  // First we copy the vertices to xa/ya. In the mean time
  // we convert to integer and also search for the top vertex (lowest
  // y coordinate) and bottom vertex.
  //@@@ TODO: pre-shift x with 16
  //---------
  static int* xa = NULL, * ya = NULL;
  static int num_xa = 0;
  if (num_verts > num_xa)
  {
    delete[] xa;
    delete[] ya;
    num_xa = num_verts;
    // @@@ MEMORY LEAK!!!
    xa = new int[num_xa];
    ya = new int[num_xa];
  }
  int top_vt = -1;
  int bot_vt = -1;
  bbox.minx = 1000000;
  bbox.maxx = -1000000;
  bbox.miny = 1000000;
  bbox.maxy = -1000000;
  for (i = 0 ; i < num_verts ; i++)
  {
    if (used_verts[i])
    {
      xa[i] = QRound (verts[i].x);
      ya[i] = QRound (verts[i].y);

      if (xa[i] < bbox.minx) bbox.minx = xa[i];
      if (xa[i] > bbox.maxx) bbox.maxx = xa[i];

      if (ya[i] < bbox.miny)
      {
        bbox.miny = ya[i];
        top_vt = i;
      }
      if (ya[i] > bbox.maxy)
      {
        bbox.maxy = ya[i];
        bot_vt = i;
      }
    }
  }

  if (bbox.maxx <= 0) return false;
  if (bbox.maxy <= 0) return false;
  if (bbox.minx >= width) return false;
  if (bbox.miny >= height) return false;

  //---------
  // First initialize dirty_left and dirty_right for every row.
  //---------
  for (i = 0 ; i < num_tile_rows ; i++)
  {
    dirty_left[i] = 1000;
    dirty_right[i] = -1;
  }

  //---------
  // Draw all edges.
  //---------
  for (i = 0 ; i < num_edges ; i++)
  {
    int vt1 = *edges++;
    int vt2 = *edges++;
    int ya1 = ya[vt1];
    int ya2 = ya[vt2];
    if (ya1 != ya2)
    {
      int xa1, xa2;
      if (ya1 < ya2)
      {
	xa1 = xa[vt1];
        xa2 = xa[vt2];
      }
      else
      {
        int y = ya1;
	ya1 = ya2;
	ya2 = y;
	xa1 = xa[vt2];
        xa2 = xa[vt1];
      }
      DrawLine (xa1, ya1, xa2, ya2, 0);	// @@@ last parm 1 on last row?
    }
  }

  return true;
}

void csTiledCoverageBuffer::InsertPolygon (csVector2* verts, int num_verts,
	float max_depth)
{
  csBox2Int bbox;
  if (!DrawPolygon (verts, num_verts, bbox))
    return;

  int tx, ty;
  int startrow, endrow;
  startrow = bbox.miny >> 6;
  if (startrow < 0) startrow = 0;
  endrow = bbox.maxy >> 6;
  if (endrow >= num_tile_rows) endrow = num_tile_rows-1;

  for (ty = startrow ; ty <= endrow ; ty++)
  {
    csBits64 fvalue;
    fvalue.Empty ();
    csCoverageTile* tile = GetTile (dirty_left[ty], ty);
    for (tx = dirty_left[ty] ; tx <= dirty_right[ty] ; tx++)
    {
      tile->Flush (fvalue, max_depth);
      tile++;
    }
  }
}

void csTiledCoverageBuffer::InsertOutline (csVector2* verts, int num_verts,
	bool* used_verts,
	int* edges, int num_edges, float max_depth)
{
  csBox2Int bbox;
  if (!DrawOutline (verts, num_verts, used_verts, edges, num_edges, bbox))
    return;

  int tx, ty;
  int startrow, endrow;
  startrow = bbox.miny >> 6;
  if (startrow < 0) startrow = 0;
  endrow = bbox.maxy >> 6;
  if (endrow >= num_tile_rows) endrow = num_tile_rows-1;

  for (ty = startrow ; ty <= endrow ; ty++)
  {
    csBits64 fvalue;
    fvalue.Empty ();
    csCoverageTile* tile = GetTile (dirty_left[ty], ty);
    for (tx = dirty_left[ty] ; tx <= dirty_right[ty] ; tx++)
    {
      tile->Flush (fvalue, max_depth);
      tile++;
    }
  }
}

bool csTiledCoverageBuffer::TestRectangle (const csBox2& rect, float min_depth)
{
  csBox2Int bbox;
  if (rect.MaxX () > 10000.0) bbox.maxx = 10000;
  else
  {
    if (rect.MaxX () <= 0) return false;
    bbox.maxx = QRound (rect.MaxX ());
  }
  if (rect.MaxY () > 10000.0) bbox.maxy = 10000;
  else
  {
    if (rect.MaxY () <= 0) return false;
    bbox.maxy = QRound (rect.MaxY ());
  }

  if (rect.MinX () < -10000.0) bbox.minx = -10000;
  else
  {
    if (rect.MinX () > 10000.0) return false;
    bbox.minx = QRound (rect.MinX ());
    if (bbox.minx >= width) return false;
  }
  if (rect.MinY () < -10000.0) bbox.miny = -10000;
  else
  {
    if (rect.MinY () > 10000.0) return false;
    bbox.miny = QRound (rect.MinY ());
    if (bbox.miny >= height) return false;
  }

  int tx, ty;
  if (bbox.miny < 0) bbox.miny = 0;
  int startrow = bbox.miny >> 6;
  if (bbox.maxy >= height) bbox.maxy = height-1;
  int endrow = bbox.maxy >> 6;
  CS_ASSERT (endrow < num_tile_rows);
  if (bbox.minx < 0) bbox.minx = 0;
  int startcol = bbox.minx >> 5;
  if (bbox.maxx >= width) bbox.maxx = width-1;
  CS_ASSERT (bbox.maxx < width);
  int endcol = bbox.maxx >> 5;

  for (ty = startrow ; ty <= endrow ; ty++)
  {
    csCoverageTile* tile = GetTile (startcol, ty);
    for (tx = startcol ; tx <= endcol ; tx++)
    {
      //@@@ NOT ALWAYS USE FULLRECT!
      if (tile->TestFullRect (min_depth))
        return true;
      tile++;
    }
  }
  return false;
}

bool csTiledCoverageBuffer::TestPoint (const csVector2& point, float min_depth)
{
  int xi, yi;
  xi = QRound (point.x);
  yi = QRound (point.y);

  if (xi < 0) return false;
  if (yi < 0) return false;
  if (xi >= width) return false;
  if (yi >= height) return false;

  int ty = yi >> 6;
  int tx = xi >> 5;

  csCoverageTile* tile = GetTile (tx, ty);
  return tile->TestPoint (xi & 31, yi & 63, min_depth);
}

static void DrawZoomedPixel (iGraphics2D* g2d, int x, int y, int col, int zoom)
{
  if (zoom == 1)
    g2d->DrawPixel (x, y, col);
  else if (zoom == 2)
  {
    x <<= 1;
    y <<= 1;
    g2d->DrawPixel (x+0, y+0, col);
    g2d->DrawPixel (x+1, y+0, col);
    g2d->DrawPixel (x+0, y+1, col);
    g2d->DrawPixel (x+1, y+1, col);
  }
  else if (zoom == 3)
  {
    x *= 3;
    y *= 3;
    g2d->DrawPixel (x+0, y+0, col);
    g2d->DrawPixel (x+1, y+0, col);
    g2d->DrawPixel (x+2, y+0, col);
    g2d->DrawPixel (x+0, y+1, col);
    g2d->DrawPixel (x+1, y+1, col);
    g2d->DrawPixel (x+2, y+1, col);
    g2d->DrawPixel (x+0, y+2, col);
    g2d->DrawPixel (x+1, y+2, col);
    g2d->DrawPixel (x+2, y+2, col);
  }
}

iString* csTiledCoverageBuffer::Debug_Dump ()
{
  scfString* rc = new scfString ();
  csString& str = rc->GetCsString ();

  int x, y, tx, ty, i, j;
  for (ty = 0 ; ty < num_tile_rows ; ty++)
  {
    for (y = 0 ; y < 8 ; y++)
    {
      for (tx = 0 ; tx < (width_po2 >> 5) ; tx++)
      {
        csCoverageTile* tile = GetTile (tx, ty);
	for (x = 0 ; x < 4 ; x++)
	{
          int cnt = 0;
	  if (!tile->queue_tile_empty)
	    for (i = 0 ; i < 8 ; i++)
	      for (j = 0 ; j < 8 ; j++)
	        cnt += tile->coverage[x*8+i].TestBit (y*8+j);
	  char c;
	  if (cnt == 64) c = '#';
	  else if (cnt > 54) c = '*';
	  else if (cnt == 0) c = ' ';
	  else if (cnt < 10) c = '.';
	  else c = 'x';
	  str.Append (c);
	}
      }
      str.Append ('\n');
    }
  }

  return rc;
}

void csTiledCoverageBuffer::Debug_Dump (iGraphics3D* g3d, int /*zoom*/)
{
  iGraphics2D* g2d = g3d->GetDriver2D ();
  int colpoint = g3d->GetTextureManager ()->FindRGB (255, 0, 0);

  int x, y, tx, ty, i, j;
  for (ty = 0 ; ty < num_tile_rows ; ty++)
  {
    for (y = 0 ; y < 8 ; y++)
    {
      for (tx = 0 ; tx < (width_po2 >> 5) ; tx++)
      {
        g2d->DrawPixel (tx*32, ty*64, colpoint);

        csCoverageTile* tile = GetTile (tx, ty);
	for (x = 0 ; x < 4 ; x++)
	{
	  float depth = tile->depth[y*4+x];
	  for (i = 0 ; i < 8 ; i++)
	    for (j = 0 ; j < 8 ; j++)
	    {
	      bool val;
	      if (tile->queue_tile_empty)
	        val = false;
	      else
	        val = tile->coverage[x*8+i].TestBit (y*8+j);
	      if (val)
	      {
	        int c = 255-int (depth);
		if (c < 50) c = 50;
		int col = g3d->GetTextureManager ()->FindRGB (c, c, c);
	        g2d->DrawPixel (tx*32+x*8+i, ty*64+y*8+j, col);
	      }
	    }
	}
      }
    }
  }
}

static float rnd (int totrange, int leftpad, int rightpad)
{
  return float (((rand () >> 4) % (totrange-leftpad-rightpad)) + leftpad);
}

#define COV_ASSERT(test,msg) \
  if (!(test)) \
  { \
    str.Format ("csTiledCoverageBuffer failure (%d,%s): %s\n", int(__LINE__), \
    	#msg, #test); \
    return rc; \
  }

iString* csTiledCoverageBuffer::Debug_UnitTest ()
{
  Setup (640, 480);

  scfString* rc = new scfString ();
  csString& str = rc->GetCsString ();

  csVector2 poly[4];
  //csCoverageTile* t;
  iString* s;

  //Initialize ();
  //COV_ASSERT (TestPoint (csVector2 (100, 100), 5) == true, "tp");
  //poly[0].Set (50, 50);
  //poly[1].Set (600, 50);
  //poly[2].Set (600, 430);
  //poly[3].Set (50, 430);
  //InsertPolygon (poly, 4, 10.0);
  //COV_ASSERT (TestPoint (csVector2 (100, 100), 5) == true, "tp");
  //COV_ASSERT (TestPoint (csVector2 (100, 100), 15) == false, "tp");
  //COV_ASSERT (TestPoint (csVector2 (599, 100), 5) == true, "tp");
  //COV_ASSERT (TestPoint (csVector2 (599, 100), 15) == false, "tp");
  //COV_ASSERT (TestPoint (csVector2 (601, 100), 5) == true, "tp");
  //COV_ASSERT (TestPoint (csVector2 (601, 100), 15) == true, "tp");

  Initialize ();
  poly[0].Set (194, 315);
  poly[1].Set (358, 203);
  poly[2].Set (443, 376);
  InsertPolygon (poly, 3, 10.0);
s=Debug_Dump ();
printf ("%s\n", s->GetData ());
s->DecRef ();
    COV_ASSERT (TestPoint (csVector2 (194-5, 315), 15) == true, "tp");



  int i;
  for (i = 0 ; i < 10000 ; i++)
  {
    Initialize ();
    float x1 = rnd (640, 10, 330);
    float y1 = rnd (480, 1, 1);
    float x2 = rnd (640, 330, 10);
    float y2 = rnd (480, 1, 230);
    float x3 = rnd (640, 330, 10);
    float y3 = rnd (480, 250, 1);
    float cx = (x1+x2+x3)/3.0;
    float cy = (y1+y2+y3)/3.0;
    poly[0].Set (x1, y1);
    poly[1].Set (x2, y2);
    poly[2].Set (x3, y3);
    InsertPolygon (poly, 3, 10.0);
printf ("%g,%g  %g,%g  %g,%g  (%g,%g)\n", x1, y1, x2, y2, x3, y3, cx, cy); fflush (stdout);
s=Debug_Dump ();
printf ("%s\n", s->GetData ());
s->DecRef ();
    COV_ASSERT (TestPoint (csVector2 (x1-5, y1), 15) == true, "tp");
    COV_ASSERT (TestPoint (csVector2 (x2+5, y2), 15) == true, "tp");
    COV_ASSERT (TestPoint (csVector2 (x3+5, y3), 15) == true, "tp");
    COV_ASSERT (TestPoint (csVector2 (cx, cy), 5) == true, "tp");
    COV_ASSERT (TestPoint (csVector2 (cx, cy), 15) == false, "tp");
  }

  //Initialize ();
  //poly[0].Set (50, 50);
  //poly[1].Set (600, 400);
  //poly[2].Set (600, 430);
  //poly[3].Set (50, 70);
  //InsertPolygon (poly, 4, 10.0);
//
//s = Debug_Dump ();
//printf ("%s\n", s->GetData ());
//s->DecRef ();

  //Initialize ();
  //DrawLine (-10, 10, -40, 40, 0);
  //t = GetTile (0, 0); t->FlushOperations ();
  //s = t->Debug_Dump_Cache (); printf ("%s\n", s->GetData ()); s->DecRef ();

  rc->DecRef ();
  return NULL;
}

csTicks csTiledCoverageBuffer::Debug_Benchmark (int num_iterations)
{
  Setup (640, 480);

  csTicks start = csGetTicks ();
  csTicks end = csGetTicks ();
  return end-start;
}

