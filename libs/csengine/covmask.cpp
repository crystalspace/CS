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

#include "sysdef.h"
#include "qint.h"
#include "csgeom/math2d.h"
#include "csengine/covmask.h"
#include "csengine/sysitf.h"

csCovMaskLUT::csCovMaskLUT (int dimension)
{
  csCovMaskLUT::dimension = dimension;
  num_edge_points = dimension*4;
  CHK (triage_masks = new csCovMaskTriage[num_edge_points*num_edge_points]);
  CHK (masks = new csCovMask[num_edge_points*num_edge_points]);
  switch (dimension)
  {
    case 1: dim_shift = 0+2; break;
    case 2: dim_shift = 1+2; break;
    case 4: dim_shift = 2+2; break;
    case 8: dim_shift = 3+2; break;
    case 16: dim_shift = 4+2; break;
    case 32: dim_shift = 5+2; break;
    case 64: dim_shift = 6+2; break;
    case 128: dim_shift = 7+2; break;
    case 256: dim_shift = 8+2; break;
  }
  // build lookup table here.
  BuildTables ();
}

csCovMaskLUT::~csCovMaskLUT ()
{
  CHK (delete [] triage_masks);
  CHK (delete [] masks);
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
  CHK (bool* left = new bool[(CS_CM_HOR+1)*(CS_CM_VER+1)]);
  CHK (bool* right = new bool[(CS_CM_HOR+1)*(CS_CM_VER+1)]);
  
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
      int mask_idx = (from << dim_shift) + to;

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

      // We traverse all (CS_CM_HOR+1)*(CS_CM_VER+1) pixels in the box
      // and mark if they are left of L1 and/or right of L2.
      int dimhor1 = CS_CM_HOR+1;
      float fdim = (float)dimension;
// @@@ BUG! What if from1 and to1 are equal? (and from2 and to2).
      for (ix = 0 ; ix <= CS_CM_HOR ; ix++)
        for (iy = 0 ; iy <= CS_CM_VER ; iy++)
	{
	  csVector2 p (
	  	(float)ix * fdim / (float)CS_CM_HOR,
	  	(float)iy * fdim / (float)CS_CM_VER);
	  left[iy*dimhor1+ix] = csMath2::WhichSide2D (p, from1, to1) < 0;
	  right[iy*dimhor1+ix] = csMath2::WhichSide2D (p, from2, to2) > 0;
if (mask_idx == 540)
{
printf ("ix=%d iy=%d p=(%.3f,%.3f) from1=(%.3f,%.3f) to1=(%.3f,%.3f) from2=(%.3f,%.3f) to2=(%.3f,%.3f) -> left=%d right=%d\n",
	ix, iy, p.x, p.y, from1.x, from1.y, to1.x, to1.y,
	from2.x, from2.y, to2.x, to2.y, left[iy*dimhor1+ix], right[iy*dimhor1+ix]);
}
	}

      // Now we are going to calculate the normal and single coverage
      // masks based on the 'left' and 'right' tables.
      triage_masks[mask_idx].Clear ();
      masks[mask_idx].Clear ();
      // For every bit in the mask.
      for (bit = 0 ; bit < CS_CM_BITS ; bit++)
      {
	ix = bit % CS_CM_HOR;
	iy = bit / CS_CM_HOR;
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
	int s = right[iy*dimhor1+ix] || right[iy*dimhor1+ix+1] ||
		right[(iy+1)*dimhor1+ix] || right[(iy+1)*dimhor1+ix+1];
if (mask_idx == 540)
{
  printf ("bit=%d ix=%d iy=%d so=%d si=%d s=%d l00=%d l01=%d l10=%d l11=%d\n", bit, ix, iy, so, si, s,
	left[iy*dimhor1+ix], left[iy*dimhor1+ix+1],
	left[(iy+1)*dimhor1+ix], left[(iy+1)*dimhor1+ix+1]);
}
	triage_masks[mask_idx].SetState (bit, so, si);
	masks[mask_idx].SetState (bit, s);
      }
    }
  }

  CHK (delete [] left);
  CHK (delete [] right);
}

int csCovMaskLUT::GetIndex (const csVector2& start,
	const csVector2& stop,
	float dxdy, float dydx, int box) const
{
//@@@ This routine is not optimal. It should use the
// fact that 'box' is power of two and not try to
// convert to float or something.
  float fbox = (float)box;	// Optimal?@@@
  float fdim = (float)dimension; // Optimal?@@@
  float x_top, x_bot;
  float y_left, y_right;
  int from, to;

  // We're going to create a bitmask with four bits to represent
  // all intersections. The bits are tblr (top, bottom, left,
  // right). For example, if the mask is 1001 then there is an
  // intersection through the top and right edges of the box.
  int mask = 0;

  // Top horizontal side.
  x_top = dxdy * (0-start.y) + start.x;
  if (x_top >= 0 && x_top < fbox) mask |= 0x8;
  // Bottom horizontal side.
  x_bot = dxdy * (fbox-start.y) + start.x;
  if (x_bot >= 0 && x_bot < fbox) mask |= 0x4;
  // Left vertical side.
  y_left = dydx * (0-start.x) + start.y;
  if (y_left >= 0 && y_left < fbox) mask |= 0x2;
  // Right vertical side.
  y_right = dydx * (fbox-start.x) + start.y;
  if (y_right >= 0 && y_right < fbox) mask |= 0x1;

  // Try all combinations. Also make sure we have
  // reasonable actions for pathological cases.
  switch (mask)
  {
    // No intersections with edges. In this case we need to test
    // if the box is left or right of the edge.
    case 0x0:
      break;
    // Impossible cases (only one intersection).
    case 0x1:
    case 0x2:
    case 0x4:
    case 0x8:
      CsPrintf (MSG_INTERNAL_ERROR,
      	"ERROR: Only one intersection in csCovMaskLUT::GetIndex()!\n");
      break;
    // Top, left, and right side.
    case 0xb:
    // Top, left, and bottom side.
    case 0xe:
    // Top, right, and bottom side.
    case 0xd:
    // Bottom, left, and right side.
    case 0x7:
      CsPrintf (MSG_INTERNAL_ERROR,
      	"ERROR: Tree intersections in csCovMaskLUT::GetIndex()!\n");
      break;
    // All sides.
    case 0xf:
      CsPrintf (MSG_INTERNAL_ERROR,
      	"ERROR: Four intersections in csCovMaskLUT::GetIndex()!\n");
      break;
    // Left and right side.
    case 0x3:
      if (start.x < stop.x)
      {
        from = QInt (fdim * y_left / fbox) + 2*dimension;
        to = QInt (fdim * y_right / fbox) + 3*dimension;
      }
      else
      {
        from = QInt (fdim * y_right / fbox) + 3*dimension;
        to = QInt (fdim * y_left / fbox) + 2*dimension;
      }
      break;
    // Top and bottom side.
    case 0xc:
      if (start.y < stop.y)
      {
        from = QInt (fdim * x_top / fbox);
        to = QInt (fdim * x_bot / fbox) + dimension;
      }
      else
      {
        from = QInt (fdim * x_bot / fbox) + dimension;
        to = QInt (fdim * x_top / fbox);
      }
      break;
    // Left and top side.
    case 0xa:
      if (start.x < stop.x)
      {
        from = QInt (fdim * y_left / fbox) + 2*dimension;
        to = QInt (fdim * x_top / fbox);
      }
      else
      {
        from = QInt (fdim * x_top / fbox);
        to = QInt (fdim * y_left / fbox) + 2*dimension;
      }
      break;
    // Left and bottom side.
    case 0x6:
      if (start.x < stop.x)
      {
        from = QInt (fdim * y_left / fbox) + 2*dimension;
        to = QInt (fdim * x_bot / fbox) + dimension;
      }
      else
      {
        from = QInt (fdim * x_bot / fbox) + dimension;
        to = QInt (fdim * y_left / fbox) + 2*dimension;
      }
      break;
    // Top and right side.
    case 0x9:
      if (start.x < stop.x)
      {
        from = QInt (fdim * x_top / fbox);
        to = QInt (fdim * y_right / fbox) + 3*dimension;
      }
      else
      {
        from = QInt (fdim * y_right / fbox) + 3*dimension;
        to = QInt (fdim * x_top / fbox);
      }
      break;
    // Bottom and right side.
    case 0x5:
      if (start.x < stop.x)
      {
        from = QInt (fdim * x_bot / fbox) + dimension;
        to = QInt (fdim * y_right / fbox) + 3*dimension;
      }
      else
      {
        from = QInt (fdim * y_right / fbox) + 3*dimension;
        to = QInt (fdim * x_bot / fbox) + dimension;
      }
      break;
  }

  printf ("mask=%x from=%d to=%d index=%d\n", mask, from, to, (from<<dim_shift) + to);
  return (from<<dim_shift) + to;
}


