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

#include <math.h>
#include "sysdef.h"
#include "csgeom/box.h"

//---------------------------------------------------------------------------

csVector2 csBox::GetCorner (int corner) const
{
  switch (corner)
  {
    case 0: return Min ();
    case 1: return csVector2 (MinX (), MaxY ());
    case 2: return csVector2 (MaxX (), MinY ());
    case 3: return Max ();
  }
  return csVector2 (0, 0);
}

csBox& csBox::operator+= (const csBox& box)
{
  if (box.minbox.x < minbox.x) minbox.x = box.minbox.x;
  if (box.minbox.y < minbox.y) minbox.y = box.minbox.y;
  if (box.maxbox.x > maxbox.x) maxbox.x = box.maxbox.x;
  if (box.maxbox.y > maxbox.y) maxbox.y = box.maxbox.y;
  return *this;
}

csBox& csBox::operator+= (const csVector2& point)
{
  if (point.x < minbox.x) minbox.x = point.x;
  if (point.x > maxbox.x) maxbox.x = point.x;
  if (point.y < minbox.y) minbox.y = point.y;
  if (point.y > maxbox.y) maxbox.y = point.y;
  return *this;
}

csBox& csBox::operator*= (const csBox& box)
{
  if (box.minbox.x > minbox.x) minbox.x = box.minbox.x;
  if (box.minbox.y > minbox.y) minbox.y = box.minbox.y;
  if (box.maxbox.x < maxbox.x) maxbox.x = box.maxbox.x;
  if (box.maxbox.y < maxbox.y) maxbox.y = box.maxbox.y;
  return *this;
}

csBox operator+ (const csBox& box1, const csBox& box2)
{
  return csBox( MIN(box1.minbox.x,box2.minbox.x), MIN(box1.minbox.y,box2.minbox.y),
              MAX(box1.maxbox.x,box2.maxbox.x), MAX(box1.maxbox.y,box2.maxbox.y) );
}

csBox operator+ (const csBox& box, const csVector2& point)
{
  return csBox( MIN(box.minbox.x,point.x), MIN(box.minbox.y,point.y),
              MAX(box.maxbox.x,point.x), MAX(box.maxbox.y,point.y) );
}

csBox operator* (const csBox& box1, const csBox& box2)
{
  return csBox( MAX(box1.minbox.x,box2.minbox.x), MAX(box1.minbox.y,box2.minbox.y),
              MIN(box1.maxbox.x,box2.maxbox.x), MIN(box1.maxbox.y,box2.maxbox.y) );
}

bool operator== (const csBox& box1, const csBox& box2)
{
  return ( (box1.minbox.x == box2.minbox.x) && (box1.minbox.y == box2.minbox.y) &&
           (box1.maxbox.x == box2.maxbox.x) && (box1.maxbox.y == box2.maxbox.y) );
}

bool operator!= (const csBox& box1, const csBox& box2)
{
  return ( (box1.minbox.x != box2.minbox.x) || (box1.minbox.y != box2.minbox.y) ||
           (box1.maxbox.x != box2.maxbox.x) || (box1.maxbox.y != box2.maxbox.y) );
}

bool operator< (const csBox& box1, const csBox& box2)
{
  return ( (box1.minbox.x >= box2.minbox.x) && (box1.minbox.y >= box2.minbox.y) &&
           (box1.maxbox.x <= box2.maxbox.x) && (box1.maxbox.y <= box2.maxbox.y) );
}

bool operator> (const csBox& box1, const csBox& box2)
{
  return ( (box2.minbox.x >= box1.minbox.x) && (box2.minbox.y >= box1.minbox.y) &&
           (box2.maxbox.x <= box1.maxbox.x) && (box2.maxbox.y <= box1.maxbox.y) );
}

bool operator< (const csVector2& point, const csBox& box)
{
  return ( (point.x >= box.minbox.x) && (point.x <= box.maxbox.x) &&
           (point.y >= box.minbox.y) && (point.y <= box.maxbox.y) );
}

bool csBox::Intersect (float minx, float miny, float maxx, float maxy,
    csVector2* poly, int num_poly)
{
  int i, i1;
  for (i = 0 ; i < num_poly ; i++)
    if (poly[i].x <= maxx && poly[i].y <= maxy &&
	poly[i].x >= minx && poly[i].y >= miny)
      return true;

  float r, x, y;
  i1 = num_poly-1;
  for (i = 0 ; i < num_poly ; i++)
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

//---------------------------------------------------------------------------

csVector3 csBox3::GetCorner (int corner) const
{
  switch (corner)
  {
    case 0: return Min ();
    case 1: return csVector3 (MinX (), MinY (), MaxZ ());
    case 2: return csVector3 (MinX (), MaxY (), MinZ ());
    case 3: return csVector3 (MinX (), MaxY (), MaxZ ());
    case 4: return csVector3 (MaxX (), MinY (), MinZ ());
    case 5: return csVector3 (MaxX (), MinY (), MaxZ ());
    case 6: return csVector3 (MaxX (), MaxY (), MinZ ());
    case 7: return Max ();
  }
  return csVector3 (0, 0, 0);
}

csBox3& csBox3::operator+= (const csBox3& box)
{
  if (box.minbox.x < minbox.x) minbox.x = box.minbox.x;
  if (box.minbox.y < minbox.y) minbox.y = box.minbox.y;
  if (box.minbox.z < minbox.z) minbox.z = box.minbox.z;
  if (box.maxbox.x > maxbox.x) maxbox.x = box.maxbox.x;
  if (box.maxbox.y > maxbox.y) maxbox.y = box.maxbox.y;
  if (box.maxbox.z > maxbox.z) maxbox.z = box.maxbox.z;
  return *this;
}

csBox3& csBox3::operator+= (const csVector3& point)
{
  if (point.x < minbox.x) minbox.x = point.x;
  if (point.x > maxbox.x) maxbox.x = point.x;
  if (point.y < minbox.y) minbox.y = point.y;
  if (point.y > maxbox.y) maxbox.y = point.y;
  if (point.z < minbox.z) minbox.z = point.z;
  if (point.z > maxbox.z) maxbox.z = point.z;
  return *this;
}

csBox3& csBox3::operator*= (const csBox3& box)
{
  if (box.minbox.x > minbox.x) minbox.x = box.minbox.x;
  if (box.minbox.y > minbox.y) minbox.y = box.minbox.y;
  if (box.minbox.z > minbox.z) minbox.z = box.minbox.z;
  if (box.maxbox.x < maxbox.x) maxbox.x = box.maxbox.x;
  if (box.maxbox.y < maxbox.y) maxbox.y = box.maxbox.y;
  if (box.maxbox.z < maxbox.z) maxbox.z = box.maxbox.z;
  return *this;
}

csBox3 operator+ (const csBox3& box1, const csBox3& box2)
{
  return csBox3(
  	MIN(box1.minbox.x,box2.minbox.x),
	MIN(box1.minbox.y,box2.minbox.y),
	MIN(box1.minbox.z,box2.minbox.z),
	MAX(box1.maxbox.x,box2.maxbox.x),
	MAX(box1.maxbox.y,box2.maxbox.y),
	MAX(box1.maxbox.z,box2.maxbox.z) );
}

csBox3 operator+ (const csBox3& box, const csVector3& point)
{
  return csBox3(
  	MIN(box.minbox.x,point.x),
	MIN(box.minbox.y,point.y),
	MIN(box.minbox.z,point.z),
	MAX(box.maxbox.x,point.x),
	MAX(box.maxbox.y,point.y),
	MAX(box.maxbox.z,point.z) );
}

csBox3 operator* (const csBox3& box1, const csBox3& box2)
{
  return csBox3(
  	MAX(box1.minbox.x,box2.minbox.x),
	MAX(box1.minbox.y,box2.minbox.y),
	MAX(box1.minbox.z,box2.minbox.z),
	MIN(box1.maxbox.x,box2.maxbox.x),
	MIN(box1.maxbox.y,box2.maxbox.y),
	MIN(box1.maxbox.z,box2.maxbox.z));
}

bool operator== (const csBox3& box1, const csBox3& box2)
{
  return ( (box1.minbox.x == box2.minbox.x)
  	&& (box1.minbox.y == box2.minbox.y)
  	&& (box1.minbox.z == box2.minbox.z)
	&& (box1.maxbox.x == box2.maxbox.x)
	&& (box1.maxbox.y == box2.maxbox.y)
	&& (box1.maxbox.z == box2.maxbox.z) );
}

bool operator!= (const csBox3& box1, const csBox3& box2)
{
  return ( (box1.minbox.x != box2.minbox.x)
  	|| (box1.minbox.y != box2.minbox.y)
  	|| (box1.minbox.z != box2.minbox.z)
	|| (box1.maxbox.x != box2.maxbox.x)
	|| (box1.maxbox.y != box2.maxbox.y)
	|| (box1.maxbox.z != box2.maxbox.z) );
}

bool operator< (const csBox3& box1, const csBox3& box2)
{
  return ( (box1.minbox.x >= box2.minbox.x)
  	&& (box1.minbox.y >= box2.minbox.y)
  	&& (box1.minbox.z >= box2.minbox.z)
	&& (box1.maxbox.x <= box2.maxbox.x)
	&& (box1.maxbox.y <= box2.maxbox.y)
	&& (box1.maxbox.z <= box2.maxbox.z) );
}

bool operator> (const csBox3& box1, const csBox3& box2)
{
  return ( (box2.minbox.x >= box1.minbox.x)
  	&& (box2.minbox.y >= box1.minbox.y)
  	&& (box2.minbox.z >= box1.minbox.z)
	&& (box2.maxbox.x <= box1.maxbox.x)
	&& (box2.maxbox.y <= box1.maxbox.y)
	&& (box2.maxbox.z <= box1.maxbox.z) );
}

bool operator< (const csVector3& point, const csBox3& box)
{
  return ( (point.x >= box.minbox.x)
  	&& (point.x <= box.maxbox.x)
	&& (point.y >= box.minbox.y)
	&& (point.y <= box.maxbox.y)
	&& (point.z >= box.minbox.z)
	&& (point.z <= box.maxbox.z) );
}

//---------------------------------------------------------------------------
