/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Largely rewritten by Ivan Avramovic <ivan@avramovic.com>

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
#include <math.h>
#include "csgeom/box.h"
#include "csgeom/transfrm.h"
#include "csgeom/poly2d.h"
#include "csgeom/math3d.h"

//---------------------------------------------------------------------------
csBox2::bEdge csBox2:: edges[8] =
{
  { CS_BOX_CORNER_xy, CS_BOX_CORNER_Xy },
  { CS_BOX_CORNER_Xy, CS_BOX_CORNER_xy },
  { CS_BOX_CORNER_Xy, CS_BOX_CORNER_XY },
  { CS_BOX_CORNER_XY, CS_BOX_CORNER_Xy },
  { CS_BOX_CORNER_XY, CS_BOX_CORNER_xY },
  { CS_BOX_CORNER_xY, CS_BOX_CORNER_XY },
  { CS_BOX_CORNER_xY, CS_BOX_CORNER_xy },
  { CS_BOX_CORNER_xy, CS_BOX_CORNER_xY }
};

csVector2 csBox2::GetCorner (int corner) const
{
  switch (corner)
  {
    case CS_BOX_CORNER_xy:  return Min ();
    case CS_BOX_CORNER_xY:  return csVector2 (MinX (), MaxY ());
    case CS_BOX_CORNER_Xy:  return csVector2 (MaxX (), MinY ());
    case CS_BOX_CORNER_XY:  return Max ();
    case CS_BOX_CENTER2:    return GetCenter ();
  }

  return csVector2 (0, 0);
}

void csBox2::SetCenter (const csVector2 &c)
{
  csVector2 move = c - GetCenter ();
  minbox += move;
  maxbox += move;
}

void csBox2::SetSize (const csVector2 &s)
{
  csVector2 center = GetCenter ();
  minbox = center - s * .5;
  maxbox = center + s * .5;
}

csBox2 &csBox2::operator+= (const csBox2 &box)
{
  if (box.minbox.x < minbox.x) minbox.x = box.minbox.x;
  if (box.minbox.y < minbox.y) minbox.y = box.minbox.y;
  if (box.maxbox.x > maxbox.x) maxbox.x = box.maxbox.x;
  if (box.maxbox.y > maxbox.y) maxbox.y = box.maxbox.y;
  return *this;
}

csBox2 &csBox2::operator+= (const csVector2 &point)
{
  if (point.x < minbox.x) minbox.x = point.x;
  if (point.x > maxbox.x) maxbox.x = point.x;
  if (point.y < minbox.y) minbox.y = point.y;
  if (point.y > maxbox.y) maxbox.y = point.y;
  return *this;
}

csBox2 &csBox2::operator*= (const csBox2 &box)
{
  if (box.minbox.x > minbox.x) minbox.x = box.minbox.x;
  if (box.minbox.y > minbox.y) minbox.y = box.minbox.y;
  if (box.maxbox.x < maxbox.x) maxbox.x = box.maxbox.x;
  if (box.maxbox.y < maxbox.y) maxbox.y = box.maxbox.y;
  return *this;
}

bool csBox2::TestIntersect (const csBox2& box) const
{
  if (box.minbox.x >= maxbox.x) return false;
  if (box.minbox.y >= maxbox.y) return false;
  if (box.maxbox.x <= minbox.x) return false;
  if (box.maxbox.y <= minbox.y) return false;
  return true;
}

csBox2 operator+ (const csBox2 &box1, const csBox2 &box2)
{
  return csBox2 (
      MIN (box1.minbox.x, box2.minbox.x),
      MIN (box1.minbox.y, box2.minbox.y),
      MAX (box1.maxbox.x, box2.maxbox.x),
      MAX (box1.maxbox.y, box2.maxbox.y));
}

csBox2 operator+ (const csBox2 &box, const csVector2 &point)
{
  return csBox2 (
      MIN (box.minbox.x, point.x),
      MIN (box.minbox.y, point.y),
      MAX (box.maxbox.x, point.x),
      MAX (box.maxbox.y, point.y));
}

csBox2 operator * (const csBox2 &box1, const csBox2 &box2)
{
  return csBox2 (
      MAX (box1.minbox.x, box2.minbox.x),
      MAX (box1.minbox.y, box2.minbox.y),
      MIN (box1.maxbox.x, box2.maxbox.x),
      MIN (box1.maxbox.y, box2.maxbox.y));
}

bool operator== (const csBox2 &box1, const csBox2 &box2)
{
  return (box1.minbox.x == box2.minbox.x) &&
    (box1.minbox.y == box2.minbox.y) &&
    (box1.maxbox.x == box2.maxbox.x) &&
    (box1.maxbox.y == box2.maxbox.y);
}

bool operator!= (const csBox2 &box1, const csBox2 &box2)
{
  return (box1.minbox.x != box2.minbox.x) ||
    (box1.minbox.y != box2.minbox.y) ||
    (box1.maxbox.x != box2.maxbox.x) ||
    (box1.maxbox.y != box2.maxbox.y);
}

bool operator< (const csBox2 &box1, const csBox2 &box2)
{
  return (box1.minbox.x >= box2.minbox.x) &&
    (box1.minbox.y >= box2.minbox.y) &&
    (box1.maxbox.x <= box2.maxbox.x) &&
    (box1.maxbox.y <= box2.maxbox.y);
}

bool operator> (const csBox2 &box1, const csBox2 &box2)
{
  return (box2.minbox.x >= box1.minbox.x) &&
    (box2.minbox.y >= box1.minbox.y) &&
    (box2.maxbox.x <= box1.maxbox.x) &&
    (box2.maxbox.y <= box1.maxbox.y);
}

bool operator< (const csVector2 &point, const csBox2 &box)
{
  return (point.x >= box.minbox.x) && (point.x <= box.maxbox.x)
  	&& (point.y >= box.minbox.y) && (point.y <= box.maxbox.y);
}

bool csBox2::Intersect (
  float minx,
  float miny,
  float maxx,
  float maxy,
  csVector2 *poly,
  int num_poly)
{
  int i, i1;
  for (i = 0; i < num_poly; i++)
    if (
      poly[i].x <= maxx &&
      poly[i].y <= maxy &&
      poly[i].x >= minx &&
      poly[i].y >= miny)
      return true;

  float r, x, y;
  i1 = num_poly - 1;
  for (i = 0; i < num_poly; i++)
  {
    bool do_hor_test1 = (poly[i].x < minx && poly[i1].x > minx);
    bool do_hor_test2 = (poly[i].x < maxx && poly[i1].x > maxx);
    if (do_hor_test1 || do_hor_test2)
    {
      r = (poly[i1].y - poly[i].y) / (poly[i1].x - poly[i].x);
      if (do_hor_test1)
      {
        y = r * (minx - poly[i].x) + poly[i].y;
        if (y >= miny && y <= maxy) return true;
      }

      if (do_hor_test2)
      {
        y = r * (maxx - poly[i].x) + poly[i].y;
        if (y >= miny && y <= maxy) return true;
      }
    }

    bool do_ver_test1 = (poly[i].y < miny && poly[i1].y > miny);
    bool do_ver_test2 = (poly[i].y < maxy && poly[i1].y > maxy);
    if (do_ver_test1 || do_ver_test2)
    {
      r = (poly[i1].x - poly[i].x) / (poly[i1].y - poly[i].y);
      if (do_ver_test1)
      {
        x = r * (miny - poly[i].y) + poly[i].x;
        if (x >= minx && x <= maxx) return true;
      }

      if (do_ver_test2)
      {
        x = r * (maxy - poly[i].y) + poly[i].x;
        if (x >= minx && x <= maxx) return true;
      }
    }

    i1 = i;
  }

  return false;
}

float csBox2::SquaredOriginDist () const
{
  // Thanks to Ivan Avramovic for the original.
  // Adapted by Norman Kramer, Jorrit Tyberghein and Wouter Wijngaards.
  float res = 0;
  if (minbox.x > 0)
    res = minbox.x * minbox.x;
  else if (maxbox.x < 0)
    res = maxbox.x * maxbox.x;
  if (minbox.y > 0)
    res += minbox.y * minbox.y;
  else if (maxbox.y < 0)
    res += maxbox.y * maxbox.y;
  return res;
}

float csBox2::SquaredOriginMaxDist () const
{
  // Thanks to Ivan Avramovic for the original.
  // Adapted by Norman Kramer, Jorrit Tyberghein and Wouter Wijngaards.
  float res;
  if (minbox.x > 0)
    res = maxbox.x * maxbox.x;
  else if (maxbox.x < 0)
    res = minbox.x * minbox.x;
  else
    res = MAX (maxbox.x * maxbox.x, minbox.x * minbox.x);
  if (minbox.y > 0)
    res += maxbox.y * maxbox.y;
  else if (maxbox.y < 0)
    res += minbox.y * minbox.y;
  else
    res += MAX (maxbox.y * maxbox.y, minbox.y * minbox.y);
  return res;
}

//---------------------------------------------------------------------------

/*
 * We have a coordinate system around our box which is
 * divided into 27 regions. The center region at coordinate (1,1,1)
 * is the node itself. Every one of the 26 remaining regions
 * defines an number of vertices which are the convex outline
 * as seen from a camera view point in that region.
 * The numbers inside the outlines table are indices from 0 to
 * 7 which describe the 8 vertices outlining the node:
 *	0: left/down/front vertex
 *	1: left/down/back
 *	2: left/up/front
 *	3: left/up/back
 *	4: right/down/front
 *	5: right/down/back
 *	6: right/up/front
 *	7: right/up/back
 * Note that every entry in this table contains 8 vertices.
 * The first are for the outline but the remaining non-outline
 * indexes are also included.
 * Note that even though 'num' may be equal to 7 there are always
 * at most 6 vertices valid for the real outline. If there is a
 * seventh element then that's one additional vertex that is visible
 * but not part of the outline (i.e. the center point).
 */

// This table also contains an array of sides visible from that region.
struct Outline
{
  int num;
  int vertices[8];
  int num_sides;
  int sides[3];
};

/// Outline lookup table.
static Outline outlines[27] =
{
  { 7, { 3, 2, 6, 4, 5, 1, 0, /**/ 7 },
    3, { CS_BOX_SIDE_x, CS_BOX_SIDE_y, CS_BOX_SIDE_z } }, //000
  { 6, { 3, 2, 0, 4, 5, 1, /**/ 6, 7 },
    2, { CS_BOX_SIDE_x, CS_BOX_SIDE_y, -1 } }, //001
  { 7, { 7, 3, 2, 0, 4, 5, 1, /**/ 6 },
    3, { CS_BOX_SIDE_x, CS_BOX_SIDE_y, CS_BOX_SIDE_Z } }, //002
  { 6, { 3, 2, 6, 4, 0, 1, /**/ 5, 7 },
    2, { CS_BOX_SIDE_x, CS_BOX_SIDE_z, -1 } }, //010
  { 4, { 3, 2, 0, 1, /**/ 4, 5, 6, 7 },
    1, { CS_BOX_SIDE_x, -1, -1 } }, //011
  { 6, { 7, 3, 2, 0, 1, 5, /**/ 4, 6 },
    2, { CS_BOX_SIDE_x, CS_BOX_SIDE_Z, -1 } }, //012
  { 7, { 3, 7, 6, 4, 0, 1, 2, /**/ 5 },
    3, { CS_BOX_SIDE_x, CS_BOX_SIDE_Y, CS_BOX_SIDE_z } }, //020
  { 6, { 3, 7, 6, 2, 0, 1, /**/ 4, 5 },
    2, { CS_BOX_SIDE_x, CS_BOX_SIDE_Y, -1 } }, //021
  { 7, { 7, 6, 2, 0, 1, 5, 3, /**/ 4 },
    3, { CS_BOX_SIDE_x, CS_BOX_SIDE_Y, CS_BOX_SIDE_Z } }, //022
  { 6, { 2, 6, 4, 5, 1, 0, /**/ 3, 7 },
    2, { CS_BOX_SIDE_y, CS_BOX_SIDE_z, -1 } }, //100
  { 4, { 0, 4, 5, 1, /**/ 2, 3, 6, 7 },
    1, { CS_BOX_SIDE_y, -1, -1 } }, //101
  { 6, { 3, 1, 0, 4, 5, 7, /**/ 2, 6 },
    2, { CS_BOX_SIDE_y, CS_BOX_SIDE_Z, -1 } }, //102
  { 4, { 2, 6, 4, 0, /**/ 1, 3, 5, 7 },
    1, { CS_BOX_SIDE_z, -1, -1 } }, //110
  { 0, { /**/ 0, 1, 2, 3, 4, 5, 6, 7 },
    0, { -1, -1, -1 } }, //111
  { 4, { 7, 3, 1, 5, /**/ 0, 2, 4, 6 },
    1, { CS_BOX_SIDE_Z, -1, -1 } }, //112
  { 6, { 3, 7, 6, 4, 0, 2, /**/ 1, 5 },
    2, { CS_BOX_SIDE_Y, CS_BOX_SIDE_z, -1 } }, //120
  { 4, { 3, 7, 6, 2, /**/ 0, 1, 4, 5 },
    1, { CS_BOX_SIDE_Y, -1, -1 } }, //121
  { 6, { 2, 3, 1, 5, 7, 6, /**/ 0, 4 },
    2, { CS_BOX_SIDE_Y, CS_BOX_SIDE_Z, -1 } }, //122
  { 7, { 2, 6, 7, 5, 1, 0, 4, /**/ 3 },
    3, { CS_BOX_SIDE_X, CS_BOX_SIDE_y, CS_BOX_SIDE_z } }, //200
  { 6, { 6, 7, 5, 1, 0, 4, /**/ 2, 3 },
    2, { CS_BOX_SIDE_X, CS_BOX_SIDE_y, -1 } }, //201
  { 7, { 6, 7, 3, 1, 0, 4, 5, /**/ 2 },
    3, { CS_BOX_SIDE_X, CS_BOX_SIDE_y, CS_BOX_SIDE_Z } }, //202
  { 6, { 2, 6, 7, 5, 4, 0, /**/ 1, 3 },
    2, { CS_BOX_SIDE_X, CS_BOX_SIDE_z, -1 } }, //210
  { 4, { 6, 7, 5, 4, /**/ 0, 1, 2, 3 },
    1, { CS_BOX_SIDE_X, -1, -1 } }, //211
  { 6, { 6, 7, 3, 1, 5, 4, /**/ 0, 2 },
    2, { CS_BOX_SIDE_X, CS_BOX_SIDE_Z, -1 } }, //212
  { 7, { 2, 3, 7, 5, 4, 0, 6, /**/ 1 },
    3, { CS_BOX_SIDE_X, CS_BOX_SIDE_Y, CS_BOX_SIDE_z } }, //220
  { 6, { 2, 3, 7, 5, 4, 6, /**/ 0, 1 },
    2, { CS_BOX_SIDE_X, CS_BOX_SIDE_Y, -1 } }, //221
  { 7, { 6, 2, 3, 1, 5, 4, 7, /**/ 0 },
    3, { CS_BOX_SIDE_X, CS_BOX_SIDE_Y, CS_BOX_SIDE_Z } } //222
};

csBox3::bEdge csBox3:: edges[24] =
{
  { CS_BOX_CORNER_Xyz, CS_BOX_CORNER_xyz, CS_BOX_SIDE_y, CS_BOX_SIDE_z },
  { CS_BOX_CORNER_xyz, CS_BOX_CORNER_Xyz, CS_BOX_SIDE_z, CS_BOX_SIDE_y },
  { CS_BOX_CORNER_xyz, CS_BOX_CORNER_xYz, CS_BOX_SIDE_x, CS_BOX_SIDE_z },
  { CS_BOX_CORNER_xYz, CS_BOX_CORNER_xyz, CS_BOX_SIDE_z, CS_BOX_SIDE_x },
  { CS_BOX_CORNER_xYz, CS_BOX_CORNER_XYz, CS_BOX_SIDE_Y, CS_BOX_SIDE_z },
  { CS_BOX_CORNER_XYz, CS_BOX_CORNER_xYz, CS_BOX_SIDE_z, CS_BOX_SIDE_Y },
  { CS_BOX_CORNER_XYz, CS_BOX_CORNER_Xyz, CS_BOX_SIDE_X, CS_BOX_SIDE_z },
  { CS_BOX_CORNER_Xyz, CS_BOX_CORNER_XYz, CS_BOX_SIDE_z, CS_BOX_SIDE_X },
  { CS_BOX_CORNER_Xyz, CS_BOX_CORNER_XyZ, CS_BOX_SIDE_X, CS_BOX_SIDE_y },
  { CS_BOX_CORNER_XyZ, CS_BOX_CORNER_Xyz, CS_BOX_SIDE_y, CS_BOX_SIDE_X },
  { CS_BOX_CORNER_XyZ, CS_BOX_CORNER_XYZ, CS_BOX_SIDE_X, CS_BOX_SIDE_Z },
  { CS_BOX_CORNER_XYZ, CS_BOX_CORNER_XyZ, CS_BOX_SIDE_Z, CS_BOX_SIDE_X },
  { CS_BOX_CORNER_XYZ, CS_BOX_CORNER_XYz, CS_BOX_SIDE_X, CS_BOX_SIDE_Y },
  { CS_BOX_CORNER_XYz, CS_BOX_CORNER_XYZ, CS_BOX_SIDE_Y, CS_BOX_SIDE_X },
  { CS_BOX_CORNER_XYZ, CS_BOX_CORNER_xYZ, CS_BOX_SIDE_Y, CS_BOX_SIDE_Z },
  { CS_BOX_CORNER_xYZ, CS_BOX_CORNER_XYZ, CS_BOX_SIDE_Z, CS_BOX_SIDE_Y },
  { CS_BOX_CORNER_xYZ, CS_BOX_CORNER_xYz, CS_BOX_SIDE_Y, CS_BOX_SIDE_x },
  { CS_BOX_CORNER_xYz, CS_BOX_CORNER_xYZ, CS_BOX_SIDE_x, CS_BOX_SIDE_Y },
  { CS_BOX_CORNER_xYZ, CS_BOX_CORNER_xyZ, CS_BOX_SIDE_x, CS_BOX_SIDE_Z },
  { CS_BOX_CORNER_xyZ, CS_BOX_CORNER_xYZ, CS_BOX_SIDE_Z, CS_BOX_SIDE_x },
  { CS_BOX_CORNER_xyZ, CS_BOX_CORNER_xyz, CS_BOX_SIDE_x, CS_BOX_SIDE_y },
  { CS_BOX_CORNER_xyz, CS_BOX_CORNER_xyZ, CS_BOX_SIDE_y, CS_BOX_SIDE_x },
  { CS_BOX_CORNER_xyZ, CS_BOX_CORNER_XyZ, CS_BOX_SIDE_y, CS_BOX_SIDE_Z },
  { CS_BOX_CORNER_XyZ, CS_BOX_CORNER_xyZ, CS_BOX_SIDE_Z, CS_BOX_SIDE_y }
};

// Index by CS_BOX_SIDE_? number.
csBox3::bFace csBox3:: faces[6] =
{
  { CS_BOX_EDGE_xyz_xyZ, CS_BOX_EDGE_xyZ_xYZ, CS_BOX_EDGE_xYZ_xYz,
      CS_BOX_EDGE_xYz_xyz },
  { CS_BOX_EDGE_XYz_XYZ, CS_BOX_EDGE_XYZ_XyZ, CS_BOX_EDGE_XyZ_Xyz,
      CS_BOX_EDGE_Xyz_XYz },
  { CS_BOX_EDGE_xyz_Xyz, CS_BOX_EDGE_Xyz_XyZ, CS_BOX_EDGE_XyZ_xyZ,
      CS_BOX_EDGE_xyZ_xyz },
  { CS_BOX_EDGE_xYZ_XYZ, CS_BOX_EDGE_XYZ_XYz, CS_BOX_EDGE_XYz_xYz,
      CS_BOX_EDGE_xYz_xYZ },
  { CS_BOX_EDGE_xYz_XYz, CS_BOX_EDGE_XYz_Xyz, CS_BOX_EDGE_Xyz_xyz,
      CS_BOX_EDGE_xyz_xYz },
  { CS_BOX_EDGE_XYZ_xYZ, CS_BOX_EDGE_xYZ_xyZ, CS_BOX_EDGE_xyZ_XyZ,
      CS_BOX_EDGE_XyZ_XYZ }
};

csVector3 csBox3::GetCorner (int corner) const
{
  switch (corner)
  {
    case CS_BOX_CORNER_xyz: return Min ();
    case CS_BOX_CORNER_xyZ: return csVector3 (MinX (), MinY (), MaxZ ());
    case CS_BOX_CORNER_xYz: return csVector3 (MinX (), MaxY (), MinZ ());
    case CS_BOX_CORNER_xYZ: return csVector3 (MinX (), MaxY (), MaxZ ());
    case CS_BOX_CORNER_Xyz: return csVector3 (MaxX (), MinY (), MinZ ());
    case CS_BOX_CORNER_XyZ: return csVector3 (MaxX (), MinY (), MaxZ ());
    case CS_BOX_CORNER_XYz: return csVector3 (MaxX (), MaxY (), MinZ ());
    case CS_BOX_CORNER_XYZ: return Max ();
    case CS_BOX_CENTER3:    return GetCenter ();
  }

  return csVector3 (0, 0, 0);
}

void csBox3::SetCenter (const csVector3 &c)
{
  csVector3 move = c - GetCenter ();
  minbox += move;
  maxbox += move;
}

void csBox3::SetSize (const csVector3 &s)
{
  csVector3 center = GetCenter ();
  minbox = center - s * .5;
  maxbox = center + s * .5;
}

csBox2 csBox3::GetSide (int side) const
{
  switch (side)
  {
    case CS_BOX_SIDE_x:
    case CS_BOX_SIDE_X:
      return csBox2 (MinY (), MinZ (), MaxY (), MaxZ ());
    case CS_BOX_SIDE_y:
    case CS_BOX_SIDE_Y:
      return csBox2 (MinX (), MinZ (), MaxX (), MaxZ ());
    case CS_BOX_SIDE_z:
    case CS_BOX_SIDE_Z:
      return csBox2 (MinX (), MinY (), MaxX (), MaxY ());
  }

  return csBox2 ();
}

bool csBox3::AdjacentX (const csBox3 &other) const
{
  if (
    ABS (other.MinX () - MaxX ()) < SMALL_EPSILON ||
    ABS (other.MaxX () - MinX ()) < SMALL_EPSILON)
  {
    if (MaxY () < other.MinY () || MinY () > other.MaxY ()) return false;
    if (MaxZ () < other.MinZ () || MinZ () > other.MaxZ ()) return false;
    return true;
  }

  return false;
}

bool csBox3::AdjacentY (const csBox3 &other) const
{
  if (
    ABS (other.MinY () - MaxY ()) < SMALL_EPSILON ||
    ABS (other.MaxY () - MinY ()) < SMALL_EPSILON)
  {
    if (MaxX () < other.MinX () || MinX () > other.MaxX ()) return false;
    if (MaxZ () < other.MinZ () || MinZ () > other.MaxZ ()) return false;
    return true;
  }

  return false;
}

bool csBox3::AdjacentZ (const csBox3 &other) const
{
  if (
    ABS (other.MinZ () - MaxZ ()) < SMALL_EPSILON ||
    ABS (other.MaxZ () - MinZ ()) < SMALL_EPSILON)
  {
    if (MaxX () < other.MinX () || MinX () > other.MaxX ()) return false;
    if (MaxY () < other.MinY () || MinY () > other.MaxY ()) return false;
    return true;
  }

  return false;
}

int csBox3::Adjacent (const csBox3 &other) const
{
  if (AdjacentX (other))
  {
    if (other.MaxX () > MaxX ())
      return CS_BOX_SIDE_X;
    else
      return CS_BOX_SIDE_x;
  }

  if (AdjacentY (other))
  {
    if (other.MaxY () > MaxY ())
      return CS_BOX_SIDE_Y;
    else
      return CS_BOX_SIDE_y;
  }

  if (AdjacentZ (other))
  {
    if (other.MaxZ () > MaxZ ())
      return CS_BOX_SIDE_Z;
    else
      return CS_BOX_SIDE_z;
  }

  return -1;
}

int csBox3::CalculatePointSegment (const csVector3& pos) const
{
  const csVector3 &bmin = Min ();
  const csVector3 &bmax = Max ();
  int idx;

  // First select x part of coordinate.
  if (pos.x < bmin.x)
    idx = 0 * 9;
  else if (pos.x > bmax.x)
    idx = 2 * 9;
  else
    idx = 1 * 9;

  // Then y part.
  if (pos.y < bmin.y)
    idx += 0 * 3;
  else if (pos.y > bmax.y)
    idx += 2 * 3;
  else
    idx += 1 * 3;

  // Then z part.
  if (pos.z < bmin.z)
    idx += 0;
  else if (pos.z > bmax.z)
    idx += 2;
  else
    idx += 1;

  return idx;
}

int csBox3::GetVisibleSides (const csVector3 &pos, int *visible_sides) const
{
  int idx = CalculatePointSegment (pos);
  const Outline &ol = outlines[idx];
  int num_array = ol.num_sides;
  int i;
  for (i = 0; i < num_array; i++) visible_sides[i] = ol.sides[i];
  return num_array;
}

void csBox3::GetConvexOutline (
  const csVector3 &pos,
  csVector3 *ar,
  int &num_array,
  bool bVisible) const
{
  const csVector3 &bmin = Min ();
  const csVector3 &bmax = Max ();
  int idx = CalculatePointSegment (pos);

  const Outline &ol = outlines[idx];
  num_array = (bVisible ? ol.num : MIN (ol.num, 6));

  int i;
  for (i = 0; i < num_array; i++)
  {
    switch (ol.vertices[i])
    {
      case 0: ar[i].x = bmin.x; ar[i].y = bmin.y; ar[i].z = bmin.z; break;
      case 1: ar[i].x = bmin.x; ar[i].y = bmin.y; ar[i].z = bmax.z; break;
      case 2: ar[i].x = bmin.x; ar[i].y = bmax.y; ar[i].z = bmin.z; break;
      case 3: ar[i].x = bmin.x; ar[i].y = bmax.y; ar[i].z = bmax.z; break;
      case 4: ar[i].x = bmax.x; ar[i].y = bmin.y; ar[i].z = bmin.z; break;
      case 5: ar[i].x = bmax.x; ar[i].y = bmin.y; ar[i].z = bmax.z; break;
      case 6: ar[i].x = bmax.x; ar[i].y = bmax.y; ar[i].z = bmin.z; break;
      case 7: ar[i].x = bmax.x; ar[i].y = bmax.y; ar[i].z = bmax.z; break;
    }
  }
}

bool csBox3::Between (const csBox3 &box1, const csBox3 &box2) const
{
  // First the trival test to see if the coordinates are
  // at least within the right intervals.
  if (
    (
      (maxbox.x >= box1.minbox.x && minbox.x <= box2.maxbox.x) ||
      (maxbox.x >= box2.minbox.x && minbox.x <= box1.maxbox.x)
    ) &&
    (
      (maxbox.y >= box1.minbox.y && minbox.y <= box2.maxbox.y) ||
      (maxbox.y >= box2.minbox.y && minbox.y <= box1.maxbox.y)
    ) &&
    (
      (maxbox.z >= box1.minbox.z && minbox.z <= box2.maxbox.z) ||
      (maxbox.z >= box2.minbox.z && minbox.z <= box1.maxbox.z)
    ))
  {
    // @@@ Ok, let's just return true here. Maybe this test is already
    // enough? We could have used the planes as well.
    return true;
  }

  return false;
}

void csBox3::ManhattanDistance (const csBox3 &other, csVector3 &dist) const
{
  if (other.MinX () >= MaxX ())
    dist.x = other.MinX () - MaxX ();
  else if (MinX () >= other.MaxX ())
    dist.x = MinX () - other.MaxX ();
  else
    dist.x = 0;
  if (other.MinY () >= MaxY ())
    dist.y = other.MinY () - MaxY ();
  else if (MinY () >= other.MaxY ())
    dist.y = MinY () - other.MaxY ();
  else
    dist.y = 0;
  if (other.MinZ () >= MaxZ ())
    dist.z = other.MinZ () - MaxZ ();
  else if (MinZ () >= other.MaxZ ())
    dist.z = MinZ () - other.MaxZ ();
  else
    dist.z = 0;
}

float csBox3::SquaredOriginDist () const
{
  // Thanks to Ivan Avramovic for the original.
  // Adapted by Norman Kramer, Jorrit Tyberghein and Wouter Wijngaards.
  float res = 0;
  if (minbox.x > 0)
    res = minbox.x * minbox.x;
  else if (maxbox.x < 0)
    res = maxbox.x * maxbox.x;
  if (minbox.y > 0)
    res += minbox.y * minbox.y;
  else if (maxbox.y < 0)
    res += maxbox.y * maxbox.y;
  if (minbox.z > 0)
    res += minbox.z * minbox.z;
  else if (maxbox.z < 0)
    res += maxbox.z * maxbox.z;
  return res;
}

float csBox3::SquaredOriginMaxDist () const
{
  // Thanks to Ivan Avramovic for the original.
  // Adapted by Norman Kramer, Jorrit Tyberghein and Wouter Wijngaards.
  float res;
  if (minbox.x > 0)
    res = maxbox.x * maxbox.x;
  else if (maxbox.x < 0)
    res = minbox.x * minbox.x;
  else
    res = MAX (maxbox.x * maxbox.x, minbox.x * minbox.x);
  if (minbox.y > 0)
    res += maxbox.y * maxbox.y;
  else if (maxbox.y < 0)
    res += minbox.y * minbox.y;
  else
    res += MAX (maxbox.y * maxbox.y, minbox.y * minbox.y);
  if (minbox.z > 0)
    res += maxbox.z * maxbox.z;
  else if (maxbox.z < 0)
    res += minbox.z * minbox.z;
  else
    res += MAX (maxbox.z * maxbox.z, minbox.z * minbox.z);
  return res;
}

// Version to cope with z <= 0. This is wrong but it in the places where
// it is used below the result is acceptable because it generates a
// conservative result (i.e. a box or outline that is bigger then reality).
static void PerspectiveWrong (const csVector3& v, csVector2& p, float fov,
    	float sx, float sy)
{
  float iz = fov * 10;
  p.x = v.x * iz + sx;
  p.y = v.y * iz + sy;
}

static void Perspective (const csVector3& v, csVector2& p, float fov,
    	float sx, float sy)
{
  float iz = fov / v.z;
  p.x = v.x * iz + sx;
  p.y = v.y * iz + sy;
}

bool csBox3::ProjectBox (const csTransform& trans, float fov,
	float sx, float sy, csBox2& sbox, float& min_z, float& max_z) const
{
  const csVector3& origin = trans.GetOrigin ();
  int idx = CalculatePointSegment (origin);
  const Outline &ol = outlines[idx];
  int num_array = MIN (ol.num, 6);

  csBox3 cbox (trans * GetCorner (ol.vertices[0]));
  int i;
  // We go to 8 so that we can calculate the correct min_z/max_z.
  // If we only go to num_array we will only calculate min_z/max_z
  // for the outine vertices.
  for (i = 1; i < 8; i++)
  {
    csVector3 v = trans * GetCorner (ol.vertices[i]);
    if (i < num_array)
    {
      cbox.AddBoundingVertexSmart (v);
      min_z = cbox.MinZ ();
      max_z = cbox.MaxZ ();
    }
    else
    {
      if (v.z < min_z) min_z = v.z;
      if (v.z > max_z) max_z = v.z;
    }
  }

  if (max_z < 0.01) return false;

// @@@ In theory we can optimize here again by calling CalculatePointSegment
// again for the new box and the 0,0,0 point. By doing that we could
// avoid doing four perspective projections.

  // If z < .1 we do conservative clipping. Not correct but it will generate
  // a box that is bigger then the real one which is ok for testing culling.
  csVector2 oneCorner;
  if (cbox.Max ().z < .1)
    PerspectiveWrong (cbox.Max (), oneCorner, fov, sx, sy);
  else
    Perspective (cbox.Max (), oneCorner, fov, sx, sy);
  sbox.StartBoundingBox (oneCorner);

  csVector3 v (cbox.MinX (), cbox.MinY (), cbox.MaxZ ());
  if (v.z < .1)
    PerspectiveWrong (v, oneCorner, fov, sx, sy);
  else
    Perspective (v, oneCorner, fov, sx, sy);
  sbox.AddBoundingVertexSmart (oneCorner);

  if (cbox.Min ().z < .1)
    PerspectiveWrong (cbox.Min (), oneCorner, fov, sx, sy);
  else
    Perspective (cbox.Min (), oneCorner, fov, sx, sy);
  sbox.AddBoundingVertexSmart (oneCorner);

  v.Set (cbox.MaxX (), cbox.MaxY (), cbox.MinZ ());
  if (v.z < .1)
    PerspectiveWrong (v, oneCorner, fov, sx, sy);
  else
    Perspective (v, oneCorner, fov, sx, sy);
  sbox.AddBoundingVertexSmart (oneCorner);

  return true;
}

bool csBox3::ProjectOutline (const csVector3& origin,
	int axis, float where,
  	csArray<csVector2>& poly) const
{
  int idx = CalculatePointSegment (origin);
  const Outline &ol = outlines[idx];
  int num_array = MIN (ol.num, 6);

  int i;
  for (i = 0 ; i < num_array ; i++)
  {
    csVector3 isect;
    if (!csIntersect3::AxisPlane (origin, ol.vertices[i], axis, where,
    	isect))
      return false;
    csVector2 v;
    switch (axis)
    {
      case 0: v.x = isect.y; v.y = isect.z; break;
      case 1: v.x = isect.x; v.y = isect.z; break;
      case 2: v.x = isect.x; v.y = isect.y; break;
    }
    poly.Push (v);
  }
  return true;
}

bool csBox3::ProjectOutline (const csVector3& origin,
	int axis, float where,
  	csPoly2D& poly) const
{
  int idx = CalculatePointSegment (origin);
  const Outline &ol = outlines[idx];
  int num_array = MIN (ol.num, 6);
  poly.SetVertexCount (num_array);

  int i;
  for (i = 0 ; i < num_array ; i++)
  {
    csVector3 isect;
    if (!csIntersect3::AxisPlane (origin, ol.vertices[i], axis, where,
    	isect))
      return false;
    switch (axis)
    {
      case 0: poly[i].x = isect.y; poly[i].y = isect.z; break;
      case 1: poly[i].x = isect.x; poly[i].y = isect.z; break;
      case 2: poly[i].x = isect.x; poly[i].y = isect.y; break;
    }
  }
  return true;
}

bool csBox3::ProjectOutline (const csTransform& trans, float fov,
	float sx, float sy,
  	csPoly2D& poly, float& min_z, float& max_z) const
{
  const csVector3& origin = trans.GetOrigin ();
  int idx = CalculatePointSegment (origin);
  const Outline &ol = outlines[idx];
  int num_array = MIN (ol.num, 6);
  poly.SetVertexCount (num_array);

  min_z = 100000000.0;
  max_z = 0;
  int i;
  // We go to 8 so that we can calculate the correct min_z/max_z.
  // If we only go to num_array we will only calculate min_z/max_z
  // for the outine vertices.
  for (i = 0 ; i < 8 ; i++)
  {
    csVector3 v = trans * GetCorner (ol.vertices[i]);
    if (v.z > max_z) max_z = v.z;
    if (v.z < min_z) min_z = v.z;
    if (i < num_array)
    {
      if (v.z < .1)
        PerspectiveWrong (v, poly[i], fov, sx, sy);
      else
        Perspective (v, poly[i], fov, sx, sy);
    }
  }
  return max_z >= .1;
}

bool csBox3::ProjectBoxAndOutline (const csTransform& trans, float fov,
	float sx, float sy, csBox2& sbox,
  	csPoly2D& poly, float& min_z, float& max_z) const
{
  const csVector3& origin = trans.GetOrigin ();
  int idx = CalculatePointSegment (origin);
  const Outline &ol = outlines[idx];
  int num_array = MIN (ol.num, 6);
  poly.SetVertexCount (num_array);

  min_z = 100000000.0;
  max_z = 0;
  sbox.StartBoundingBox ();
  int i;
  // We go to 8 so that we can calculate the correct min_z/max_z.
  // If we only go to num_array we will only calculate min_z/max_z
  // for the outine vertices.
  for (i = 0 ; i < 8 ; i++)
  {
    csVector3 v = trans * GetCorner (ol.vertices[i]);
    if (v.z > max_z) max_z = v.z;
    if (v.z < min_z) min_z = v.z;
    if (i < num_array)
    {
      if (v.z < .1)
        PerspectiveWrong (v, poly[i], fov, sx, sy);
      else
        Perspective (v, poly[i], fov, sx, sy);
      sbox.AddBoundingVertex (poly[i]);
    }
  }
  return max_z >= .1;
}

csBox3 &csBox3::operator+= (const csBox3 &box)
{
  if (box.minbox.x < minbox.x) minbox.x = box.minbox.x;
  if (box.minbox.y < minbox.y) minbox.y = box.minbox.y;
  if (box.minbox.z < minbox.z) minbox.z = box.minbox.z;
  if (box.maxbox.x > maxbox.x) maxbox.x = box.maxbox.x;
  if (box.maxbox.y > maxbox.y) maxbox.y = box.maxbox.y;
  if (box.maxbox.z > maxbox.z) maxbox.z = box.maxbox.z;
  return *this;
}

csBox3 &csBox3::operator+= (const csVector3 &point)
{
  if (point.x < minbox.x) minbox.x = point.x;
  if (point.x > maxbox.x) maxbox.x = point.x;
  if (point.y < minbox.y) minbox.y = point.y;
  if (point.y > maxbox.y) maxbox.y = point.y;
  if (point.z < minbox.z) minbox.z = point.z;
  if (point.z > maxbox.z) maxbox.z = point.z;
  return *this;
}

csBox3 &csBox3::operator*= (const csBox3 &box)
{
  if (box.minbox.x > minbox.x) minbox.x = box.minbox.x;
  if (box.minbox.y > minbox.y) minbox.y = box.minbox.y;
  if (box.minbox.z > minbox.z) minbox.z = box.minbox.z;
  if (box.maxbox.x < maxbox.x) maxbox.x = box.maxbox.x;
  if (box.maxbox.y < maxbox.y) maxbox.y = box.maxbox.y;
  if (box.maxbox.z < maxbox.z) maxbox.z = box.maxbox.z;
  return *this;
}

bool csBox3::TestIntersect (const csBox3& box) const
{
  if (box.minbox.x >= maxbox.x) return false;
  if (box.minbox.y >= maxbox.y) return false;
  if (box.minbox.z >= maxbox.z) return false;
  if (box.maxbox.x <= minbox.x) return false;
  if (box.maxbox.y <= minbox.y) return false;
  if (box.maxbox.z <= minbox.z) return false;
  return true;
}

csBox3 operator+ (const csBox3 &box1, const csBox3 &box2)
{
  return csBox3 (
      MIN (box1.minbox.x, box2.minbox.x),
      MIN (box1.minbox.y, box2.minbox.y),
      MIN (box1.minbox.z, box2.minbox.z),
      MAX (box1.maxbox.x, box2.maxbox.x),
      MAX (box1.maxbox.y, box2.maxbox.y),
      MAX (box1.maxbox.z, box2.maxbox.z));
}

csBox3 operator+ (const csBox3 &box, const csVector3 &point)
{
  return csBox3 (
      MIN (box.minbox.x, point.x),
      MIN (box.minbox.y, point.y),
      MIN (box.minbox.z, point.z),
      MAX (box.maxbox.x, point.x),
      MAX (box.maxbox.y, point.y),
      MAX (box.maxbox.z, point.z));
}

csBox3 operator * (const csBox3 &box1, const csBox3 &box2)
{
  return csBox3 (
      MAX (box1.minbox.x, box2.minbox.x),
      MAX (box1.minbox.y, box2.minbox.y),
      MAX (box1.minbox.z, box2.minbox.z),
      MIN (box1.maxbox.x, box2.maxbox.x),
      MIN (box1.maxbox.y, box2.maxbox.y),
      MIN (box1.maxbox.z, box2.maxbox.z));
}

bool operator== (const csBox3 &box1, const csBox3 &box2)
{
  return (box1.minbox.x == box2.minbox.x) &&
    (box1.minbox.y == box2.minbox.y) &&
    (box1.minbox.z == box2.minbox.z) &&
    (box1.maxbox.x == box2.maxbox.x) &&
    (box1.maxbox.y == box2.maxbox.y) &&
    (box1.maxbox.z == box2.maxbox.z);
}

bool operator!= (const csBox3 &box1, const csBox3 &box2)
{
  return (box1.minbox.x != box2.minbox.x) ||
    (box1.minbox.y != box2.minbox.y) ||
    (box1.minbox.z != box2.minbox.z) ||
    (box1.maxbox.x != box2.maxbox.x) ||
    (box1.maxbox.y != box2.maxbox.y) ||
    (box1.maxbox.z != box2.maxbox.z);
}

bool operator < (const csBox3 &box1, const csBox3 &box2)
{
  return (box1.minbox.x >= box2.minbox.x) &&
    (box1.minbox.y >= box2.minbox.y) &&
    (box1.minbox.z >= box2.minbox.z) &&
    (box1.maxbox.x <= box2.maxbox.x) &&
    (box1.maxbox.y <= box2.maxbox.y) &&
    (box1.maxbox.z <= box2.maxbox.z);
}
bool operator> (const csBox3 &box1, const csBox3 &box2)
{
  return (box2.minbox.x >= box1.minbox.x) &&
    (box2.minbox.y >= box1.minbox.y) &&
    (box2.minbox.z >= box1.minbox.z) &&
    (box2.maxbox.x <= box1.maxbox.x) &&
    (box2.maxbox.y <= box1.maxbox.y) &&
    (box2.maxbox.z <= box1.maxbox.z);
}

bool operator < (const csVector3 &point, const csBox3 &box)
{
  return (point.x >= box.minbox.x) && (point.x <= box.maxbox.x) &&
    (point.y >= box.minbox.y) && (point.y <= box.maxbox.y) &&
      (point.z >= box.minbox.z) && (point.z <= box.maxbox.z);
}

//---------------------------------------------------------------------------
