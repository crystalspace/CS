/*
    Copyright (C) 2004 by Jorrit Tyberghein

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

#ifndef __LITPOLYGON_H__
#define __LITPOLYGON_H__

#include <stdarg.h>
#include <csutil/array.h>
#include <csgeom/vector3.h>
#include <csgeom/plane3.h>

#include "lmmapper.h"

/**
 * This is a polygon.
 */
class litPolygon
{
private:
  // Mapping for our lightmap.
  litLightMapMapper lmmapper;

  // Polygon data.
  csArray<csVector3> poly;

  // Polygon plane.
  csPlane3 poly_plane;

public:
  litPolygon ();

  /**
   * Initialize this polygon including its mapping. It will
   * also calculate the polygon plane. This assumes
   * the polygon data has already been set up.
   */
  void InitializePolygon (float density);
  
  /**
   * Get polygon data.
   */
  const csArray<csVector3>& GetPoly () const { return poly; }

  /**
   * Get polygon data.
   */
  csArray<csVector3>& GetPoly () { return poly; }
};

#endif // __LITPOLYGON_H__

