/*
  Copyright (C) 1998 by Jorrit Tyberghein

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
#include "csgeom/volume.h"

void csVolume::Clear ()
{
  CHK (delete [] planes);
  planes = NULL;
  num_planes = max_planes = 0;
}

csVolume::csVolume ()
{
  planes = NULL;
  num_planes = 0;
  max_planes = 0;
}

csVolume::csVolume (const csVolume &copy)
{
  num_planes = copy.num_planes;
  max_planes = copy.max_planes;
  if (copy.planes)
  {
    CHK (planes = new csPlane [max_planes]);
    memcpy (planes, copy.planes, sizeof (csPlane) * num_planes);
  }
  else planes = NULL;
}

void csVolume::AddPlane (csPlane& plane)
{
  AddPlane (plane.A (), plane.B (), plane.C (), plane.D ());
}

void csVolume::AddPlane (float A, float B, float C, float D)
{
  if (num_planes >= max_planes)
  {
    csPlane* new_planes = new csPlane [max_planes+10];
    if (planes)
    {
      memcpy (new_planes, planes, sizeof (csPlane)*max_planes);
      delete [] planes;
    }
    planes = new_planes;
    max_planes += 10;
  }
  planes[num_planes].Set (A, B, C, D);
  num_planes++;
}

csVolume csVolume::Intersect (const csVolume& volume)
{
  (void)volume;
  csVolume v;
  return v;
}

csVector3* csVolume::Intersect (csVector3* poly, int num_in, int& num_out)
{
  (void)poly; (void)num_in; (void)num_out;
  return NULL;
}

bool csVolume::Intersects (csVolume& target)
{
  (void)target;
  return false;
}

int csVolume::ClassifyToPlane (const csPlane& cutter)
{
  (void)cutter;
  return 0;
}

void csVolume::CutByPlane (const csPlane& cutter)
{
  (void)cutter;
}

bool csVolume::Contains (csVector3& p)
{
  (void)p;
  return false;
}

bool csVolume::IsEmpty ()
{
  return true;
}

