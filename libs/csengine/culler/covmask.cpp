/*
    Copyright (C) 1999 by Jorrit Tyberghein
  
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
#include "qint.h"
#include "csgeom/math2d.h"
#include "csengine/engine.h"
#include "csengine/covmask.h"
#include "ivideo/igraph2d.h"

void csCovMask::Dump () const
{
  char st[3] = ".#";
  int i;
  for (i = 0 ; i < CS_CM_BITS ; i++)
  {
    printf ("%c", st[GetState (i)]);
    if ((i+1) % CS_CM_DIM == 0) printf ("\n");
  }
}

void csCovMask::GfxDump (iGraphics2D* ig2d, int xoffs, int yoffs) const
{
  int i, col, row, st;
  int x, y;
  for (i = 0 ; i < CS_CM_BITS ; i++)
  {
    col = xoffs + 16 * (i & CS_CM_DIMMASK);
    row = yoffs + 16 * (i >> CS_CM_DIMSHIFT);
    st = GetState (i) ? 0xf800 : 0x8410;
    for (x = 0 ; x < 15 ; x++)
      for (y = 0 ; y < 15 ; y++)
        ig2d->DrawPixel (col+x, row+y, st);
  }
}

void csCovMaskTriage::Dump () const
{
  char st[5] = "*#.?";
  int i;
  for (i = 0 ; i < CS_CM_BITS ; i++)
  {
    printf ("%c", st[GetState (i)]);
    if ((i+1) % CS_CM_DIM == 0) printf ("\n");
  }
}

void csCovMaskTriage::GfxDump (iGraphics2D* ig2d, int xoffs, int yoffs) const
{
  int i, col, row, st;
  int x, y;
  for (i = 0 ; i < CS_CM_BITS ; i++)
  {
    col = xoffs + 16 * (i & CS_CM_DIMMASK);
    row = yoffs + 16 * (i >> CS_CM_DIMSHIFT);
    st = GetState (i);
    switch (st)
    {
      case 0: st = 0x001f; break;
      case 1: st = 0xf800; break;
      case 2: st = 0x8410; break;
      case 3: st = 0x07e0; break;
    }
    for (x = 0 ; x < 15 ; x++)
      for (y = 0 ; y < 15 ; y++)
        ig2d->DrawPixel (col+x, row+y, st);
  }
}

//--------------------------------------------------------------------------

csCovMaskLUT::csCovMaskLUT (int dimension)
{
  csCovMaskLUT::dimension = dimension;
  num_edge_points = dimension*4;
  // We allocate two extra entries in the lookup tables to hold
  // the mask values for completely outside and completely inside
  // (in that order).
  triage_masks = new csCovMaskTriage[num_edge_points*num_edge_points+2];
  masks = new csCovMask[num_edge_points*num_edge_points+2];
  index_outside = num_edge_points*num_edge_points;
  index_inside = index_outside+1;
  
  switch (dimension)
  {
    case 1: dim_shift = 0; break;
    case 2: dim_shift = 1; break;
    case 4: dim_shift = 2; break;
    case 8: dim_shift = 3; break;
    case 16: dim_shift = 4; break;
    case 32: dim_shift = 5; break;
    case 64: dim_shift = 6; break;
    case 128: dim_shift = 7; break;
    case 256: dim_shift = 8; break;
  }
  edge_shift = dim_shift+2;

  // build lookup table here.
  BuildTables ();
}

csCovMaskLUT::~csCovMaskLUT ()
{
  delete [] triage_masks;
  delete [] masks;
}

void calc_from_point (int from, int dimension, int& from_ix1, int& from_iy1,
	int& from_ix2, int& from_iy2)
{
  // Calculate two integer x and y's on the edges of the box
  // corresponding with the 'from' point.
  // These two positions define the left and right starting
  // points of the double line segment intersecting the box.
  if (from < dimension*2)
  {
    from_iy1 = (from / dimension) * dimension;
    from_iy2 = from_iy1;
    if (from_iy1 == 0)
    {
      from_ix2 = from % dimension;
      from_ix1 = from_ix2+1;
    }
    else
    {
      from_ix1 = from % dimension;
      from_ix2 = from_ix1+1;
    }
  }
  else
  {
    from_ix1 = ((from-dimension*2) / dimension) * dimension;
    from_ix2 = from_ix1;
    if (from_ix1 == 0)
    {
      from_iy1 = from % dimension;
      from_iy2 = from_iy1+1;
    }
    else
    {
      from_iy2 = from % dimension;
      from_iy1 = from_iy2+1;
    }
  }
}

void calc_to_point (int to, int dimension, int& to_ix1, int& to_iy1,
	int& to_ix2, int& to_iy2)
{
  // Calculate two integer x and y's on the edges of the box
  // corresponding with the 'to' point.
  // These two positions define the left and right ending
  // points of the double line segment intersecting the box.
  if (to < dimension*2)
  {
    to_iy1 = (to / dimension) * dimension;
    to_iy2 = to_iy1;
    if (to_iy1 == 0)
    {
      to_ix1 = to % dimension;
      to_ix2 = to_ix1+1;
    }
    else
    {
      to_ix2 = to % dimension;
      to_ix1 = to_ix2+1;
    }
  }
  else
  {
    to_ix1 = ((to-dimension*2) / dimension) * dimension;
    to_ix2 = to_ix1;
    if (to_ix1 == 0)
    {
      to_iy2 = to % dimension;
      to_iy1 = to_iy2+1;
    }
    else
    {
      to_iy1 = to % dimension;
      to_iy2 = to_iy1+1;
    }
  }
}

/*
   For dimension equal to 4 the possible edge positions
   are defined as follows:

       0  1  2  3
      +--+--+--+--+
    8 |  |  |  |  | 12
      +--+--+--+--+
    9 |  |  |  |  | 13
      +--+--+--+--+
   10 |  |  |  |  | 14
      +--+--+--+--+
   11 |  |  |  |  | 15
      +--+--+--+--+
       4  5  6  7
 */
void csCovMaskLUT::BuildTables ()
{
  int from, to, ix, iy;
  int bit;

  // Allocate a left and right matrix for all boxes defined in
  // a coverage mask. See below for info how left and right are filled.
  bool* left = new bool[(CS_CM_DIM+1)*(CS_CM_DIM+1)];
  bool* right = new bool[(CS_CM_DIM+1)*(CS_CM_DIM+1)];
  
  for (from = 0 ; from < num_edge_points ; from++)
  {
    // Calculate two integer x and y's on the edges of the box
    // corresponding with the 'from' point.
    // These two positions define the left and right starting
    // points of the double line segment intersecting the box.
    int from_ix1, from_iy1;
    int from_ix2, from_iy2;
    calc_from_point (from, dimension, from_ix1, from_iy1, from_ix2, from_iy2);

    for (to = 0 ; to < num_edge_points ; to++)
    {
      // We have an edge from 'from' to 'to'.

      // If 'from' and 'to' are on the same edge then we
      // skep this case.
      if (from / dimension == to / dimension) continue;

      // The mask index for which are calculating now.
      int mask_idx = (from << edge_shift) + to;

      // Calculate two integer x and y's on the edges of the box
      // corresponding with the 'to' point.
      // These two positions define the left and right ending
      // points of the double line segment intersecting the box.
      int to_ix1, to_iy1;
      int to_ix2, to_iy2;
      calc_to_point (to, dimension, to_ix1, to_iy1, to_ix2, to_iy2);

      // Now we have a line L1 from (from_ix1,from_iy1) to (to_ix1,to_iy1)
      // and a line L2 from (from_ix2,from_iy2) to (to_ix2,to_iy2).
      csVector2 from1 ((float)from_ix1, (float)from_iy1);
      csVector2 from2 ((float)from_ix2, (float)from_iy2);
      csVector2 to1 ((float)to_ix1, (float)to_iy1);
      csVector2 to2 ((float)to_ix2, (float)to_iy2);

      // We traverse all (CS_CM_DIM+1)*(CS_CM_DIM+1) pixels in the box
      // and mark if they are left of L1 and/or right of L2.
      int dimhor1 = CS_CM_DIM+1;
      float fdim = (float)dimension;
      for (ix = 0 ; ix <= CS_CM_DIM ; ix++)
        for (iy = 0 ; iy <= CS_CM_DIM ; iy++)
	{
	  csVector2 p (
	  	((float)ix) * fdim / (float)CS_CM_DIM,
	  	((float)iy) * fdim / (float)CS_CM_DIM);
	  if (ABS (from1.x-to1.x) < EPSILON &&
	  	ABS (from1.y-to1.y) < EPSILON)
	  {
	    csVector2 t1;
	    t1.x = from1.x + (to2.x-from2.x);
	    t1.y = from1.y + (to2.y-from2.y);
	    left[iy*dimhor1+ix] = csMath2::WhichSide2D (p, from1, t1) > 0;
	  }
	  else
	    left[iy*dimhor1+ix] = csMath2::WhichSide2D (p, from1, to1) > 0;
	  if (ABS (from2.x-to2.x) < EPSILON &&
	  	ABS (from2.y-to2.y) < EPSILON)
	  {
	    csVector2 t2;
	    t2.x = from2.x + (to1.x-from1.x);
	    t2.y = from2.y + (to1.y-from1.y);
	    right[iy*dimhor1+ix] = csMath2::WhichSide2D (p, from2, t2) < 0;
	  }
	  else
	    right[iy*dimhor1+ix] = csMath2::WhichSide2D (p, from2, to2) < 0;
	}

      // Now we are going to calculate the normal and single coverage
      // masks based on the 'left' and 'right' tables.
      triage_masks[mask_idx].Clear ();
      masks[mask_idx].Clear ();
      // For every bit in the mask.
      for (bit = 0 ; bit < CS_CM_BITS ; bit++)
      {
	ix = bit % CS_CM_DIM;
	iy = bit / CS_CM_DIM;
	// 'so' will be 1 if all points of this position in mask
	// are completely left of L1.
	int so = left[iy*dimhor1+ix] && left[iy*dimhor1+ix+1] &&
		left[(iy+1)*dimhor1+ix] && left[(iy+1)*dimhor1+ix+1];
	// 'si' will be 1 if all points of this position in mask
	// are completely right of L2.
	int si = right[iy*dimhor1+ix] && right[iy*dimhor1+ix+1] &&
		right[(iy+1)*dimhor1+ix] && right[(iy+1)*dimhor1+ix+1];
	// 's' will be 1 if there are points of this position in
	// mask that are completely right of L2.
	//int s = right[iy*dimhor1+ix] || right[iy*dimhor1+ix+1] ||
		//right[(iy+1)*dimhor1+ix] || right[(iy+1)*dimhor1+ix+1];
	//int s = (!left[iy*dimhor1+ix]) || (!left[iy*dimhor1+ix+1]) ||
		//(!left[(iy+1)*dimhor1+ix]) || (!left[(iy+1)*dimhor1+ix+1]);
	triage_masks[mask_idx].SetState (bit, so, si);
	//masks[mask_idx].SetState (bit, s);
	masks[mask_idx].SetState (bit, !so);
      }
    }
  }

  delete [] left;
  delete [] right;

  masks[index_outside].Outside ();
  masks[index_inside].Inside ();
  triage_masks[index_outside].Outside ();
  triage_masks[index_inside].Inside ();
}

int csCovMaskLUT::GetIndex (const csVector2& start,
	const csVector2& stop,
	const csCovEdgeInfo& edge,
	int hor_offs, int ver_offs,
	int box_dim, int box_shift) const
{
  float fhor_offs = (float)hor_offs;	// Optimal?@@@
  float fver_offs = (float)ver_offs;	// Optimal?@@@
  float fbox_dim = (float)box_dim;	// Optimal?@@@
  int x_top = 0, x_bot = 0;
  int y_left = 0, y_right = 0;
  int from = 0, to = 0;
  float f;

  // Note: y is actually reversed in this routine to correct
  // clockwise stuff.
  
  // @@@ HARDCODED!!! This should be the max size of the cov mask tree.
  csVector2 sta (start.x-fhor_offs, (start.y-1024)+fver_offs);
  csVector2 sto (stop.x-fhor_offs, (stop.y-1024)+fver_offs);

  if (edge.horizontal)
  {
    // Special case for a horizontal edge.
    f = - sta.y;
    if (f < 0)
      if (sto.x > sta.x)
        return index_inside;
      else
        return index_outside;
    else if (f >= fbox_dim)
      if (sto.x < sta.x)
        return index_inside;
      else
        return index_outside;
    else
    {
      y_left = QInt (f);
      if (sta.x < sto.x)
      {
        from = ((y_left << dim_shift) >> box_shift) + 2*dimension;
	to   = from + dimension;
      }
      else
      {
        to   = ((y_left << dim_shift) >> box_shift) + 2*dimension;
        from = to + dimension;
      }
      return (from<<edge_shift) + to;
    }
  }
  else if (edge.vertical)
  {
    // Special case for a vertical edge.
    f = sta.x;
    if (f < 0)
      if (sto.y > sta.y)
        return index_inside;
      else
        return index_outside;
    else if (f >= fbox_dim)
      if (sto.y < sta.y)
        return index_inside;
      else
        return index_outside;
    else
    {
      x_top = QInt (f);
      if (sta.y > sto.y)
      {
        from = ((x_top << dim_shift) >> box_shift);
        to   = from + dimension;
      }
      else
      {
        to   = ((x_top << dim_shift) >> box_shift);
        from = to + dimension;
      }
      return (from<<edge_shift) + to;
    }
  }

  // General case.

  // We're going to create a bitmask with four bits to represent
  // all intersections. The bits are tblr (top, bottom, left,
  // right). For example, if the mask is 1001 then there is an
  // intersection through the top and right edges of the box.
  int mask = 0;

  // Top horizontal side.
  f = -edge.dxdy * (0+sta.y) + sta.x;
  if (f >= 0 && f < fbox_dim)
  {
    x_top = QInt (f);
    mask |= 0x8;
  }
  // Bottom horizontal side.
  f = -edge.dxdy * (fbox_dim+sta.y) + sta.x;
  if (f >= 0 && f < fbox_dim)
  {
    x_bot = QInt (f);
    mask |= 0x4;
  }

  // Left vertical side.
  f = -edge.dydx * (0-sta.x) - sta.y;
  if (f >= 0 && f < fbox_dim)
  {
    y_left = QInt (f);
    mask |= 0x2;
  }
  // Right vertical side.
  f = -edge.dydx * (fbox_dim-sta.x) - sta.y;
  if (f >= 0 && f < fbox_dim)
  {
    y_right = QInt (f);
    mask |= 0x1;
  }

  // Try all combinations. Also make sure we have
  // reasonable actions for pathological cases.
  switch (mask)
  {
    case 0x0:	// No intersection.
    case 0x1:	// Intersection on right side.
    case 0x4:	// Intersection on bottom side.
      // No or one intersections with edges. In this case we need to test
      // if the box is left or right of the edge.

      // We can use csMath2::WhichSide2D() here but since the
      // first parameter is always (0,0) we make a more optimal
      // solution here by doing the calculations ourselves.
      {
        float k  = -sta.y*(sto.x - sta.x);
        float k1 = sta.x*(-sto.y + sta.y);
        if (k < k1)
          return index_inside;
        else
          return index_outside;
      }
      break;
    case 0x2:	// Intersection on left side.
    case 0x8:	// Intersection on top side.
      // Only one intersection is also considered a case where
      // there is no intersection (near miss).
      {
        float k  = (-sta.y-fbox_dim) * (sto.x - sta.x);
        float k1 = (sta.x-fbox_dim) * (-sto.y + sta.y);
        if (k < k1)
          return index_inside;
        else
          return index_outside;
      }
      break;
    case 0xf:	// All sides.
#     if 1
      CsPrintf (MSG_INTERNAL_ERROR,
      	"ERROR: Four intersections in csCovMaskLUT::GetIndex()!\n");
      printf ("  start=(%f,%f) stop=(%f,%f)\n", start.x, start.y, stop.x, stop.y);
      printf ("  edge.dxdy=%f dydx=%f hor=%d ver=%d\n", edge.dxdy, edge.dydx,
      	edge.horizontal, edge.vertical);
      printf ("  hor_offs=%d ver_offs=%d box_dim=%d box_shift=%d\n",
      	hor_offs, ver_offs, box_dim, box_shift);
      printf ("  sta=(%f,%f) sto=(%f,%f)\n", sta.x, sta.y, sto.x, sto.y);
      printf ("  mask=0x%x x_top=%d x_bot=%d, y_left=%d y_right=%d\n",
      	mask, x_top, x_bot, y_left, y_right);
#     endif
      break;
    case 0xb:	// Top, left, and right side.
    case 0x7:	// Bottom, left, and right side.
    case 0x3:	// Left and right side.
      if (sta.x < sto.x)
      {
        from = ((y_left << dim_shift) >> box_shift) + (dimension<<1);
        to   = ((y_right << dim_shift) >> box_shift) + (dimension<<1)+dimension;
      }
      else
      {
        from = ((y_right << dim_shift) >> box_shift) + (dimension<<1)+dimension;
        to   = ((y_left << dim_shift) >> box_shift) + (dimension<<1);
      }
      break;
    case 0xe:	// Top, left, and bottom side.
    case 0xd:	// Top, right, and bottom side.
    case 0xc:	// Top and bottom side.
      if (sta.y > sto.y)
      {
        from = ((x_top << dim_shift) >> box_shift);
        to   = ((x_bot << dim_shift) >> box_shift) + dimension;
      }
      else
      {
        from = ((x_bot << dim_shift) >> box_shift) + dimension;
        to   = ((x_top << dim_shift) >> box_shift);
      }
      break;
    case 0xa:	// Left and top side.
      if (sta.x < sto.x)
      {
        from = ((y_left << dim_shift) >> box_shift) + (dimension<<1);
        to   = ((x_top << dim_shift) >> box_shift);
      }
      else
      {
        from = ((x_top << dim_shift) >> box_shift);
        to   = ((y_left << dim_shift) >> box_shift) + (dimension<<1);
      }
      break;
    case 0x6:	// Left and bottom side.
      if (sta.x < sto.x)
      {
        from = ((y_left << dim_shift) >> box_shift) + (dimension<<1);
        to   = ((x_bot << dim_shift) >> box_shift) + dimension;
      }
      else
      {
        from = ((x_bot << dim_shift) >> box_shift) + dimension;
        to   = ((y_left << dim_shift) >> box_shift) + (dimension<<1);
      }
      break;
    case 0x9:	// Top and right side.
      if (sta.x < sto.x)
      {
        from = ((x_top << dim_shift) >> box_shift);
        to   = ((y_right << dim_shift) >> box_shift) + (dimension<<1)+dimension;
      }
      else
      {
        from = ((y_right << dim_shift) >> box_shift) + (dimension<<1)+dimension;
        to   = ((x_top << dim_shift) >> box_shift);
      }
      break;
    case 0x5:	// Bottom and right side.
      if (sta.x < sto.x)
      {
        from = ((x_bot << dim_shift) >> box_shift) + dimension;
        to   = ((y_right << dim_shift) >> box_shift) + (dimension<<1)+dimension;
      }
      else
      {
        from = ((y_right << dim_shift) >> box_shift) + (dimension<<1)+dimension;
        to   = ((x_bot << dim_shift) >> box_shift) + dimension;
      }
      break;
  }

  return (from<<edge_shift) + to;
}

csCovMaskTriage csCovMaskLUT::GetTriageMask (csVector2* verts, int num_verts,
  	csCovEdgeInfo* edges, int hor_offs, int ver_offs,
	int box_dim, int box_shift) const
{
  csCovMaskTriage mask = GetTriageMask (verts[num_verts-1], verts[0],
  	edges[num_verts-1], hor_offs, ver_offs, box_dim, box_shift);
  int i;
  for (i = 0 ; i < num_verts-1 ; i++)
  {
    if (mask.IsEmpty ()) return mask;	// Stop if mask is already empty.
    mask.Intersect (GetTriageMask (verts[i], verts[i+1], edges[i],
	hor_offs, ver_offs, box_dim, box_shift));
  }
  return mask;
}

csCovMask csCovMaskLUT::GetMask (csVector2* verts, int num_verts,
  	csCovEdgeInfo* edges, int hor_offs, int ver_offs,
	int box_dim, int box_shift) const
{
  csCovMask mask = GetMask (verts[num_verts-1], verts[0],
  	edges[num_verts-1], hor_offs, ver_offs, box_dim, box_shift);
  int i;
  for (i = 0 ; i < num_verts-1 ; i++)
  {
    if (mask.IsEmpty ()) return mask;	// Stop if mask is already empty.
    mask.Intersect (GetMask (verts[i], verts[i+1], edges[i],
    	hor_offs, ver_offs, box_dim, box_shift));
  }
  return mask;
}
