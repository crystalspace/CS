/*
    Copyright (C) 2001 by Jorrit Tyberghein

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
#include <float.h>
#include "cssysdef.h"
#include "csqint.h"
#include "cscsQsqrt.h"
#include "csgeom/sphere.h"
#include "csgeom/math3d.h"

//---------------------------------------------------------------------------
csSphere operator+ (const csSphere &s1, const csSphere &s2)
{
  csSphere s = s1;
  s += s2;
  return s;
}

void csSphere::Union (const csVector3 &ocenter, float oradius)
{
  // First calculate distance between centers of two spheres.
  float distance = csQsqrt (csSquaredDist::PointPoint (center, ocenter));

  if (radius >= oradius + distance)       // Sphere 2 is inside sphere 1
  {
  }
  else if (oradius >= radius + distance)  // Sphere 1 is inside sphere 2
  {
    center = ocenter;
    radius = oradius;
  }
  else if (ABS (distance) < SMALL_EPSILON)
  {
    // Spheres very close to each other. Because the two tests above
    // succeeded the spheres are actually equal. So nothing has to be done.
  }
  else
  {
    csVector3 direction = (center - ocenter) / distance;
    center = (center + direction * radius + ocenter + direction * oradius)
    	* 0.5f;
    radius = (radius + oradius + distance) * 0.5f;
  }
}

//---------------------------------------------------------------------------
