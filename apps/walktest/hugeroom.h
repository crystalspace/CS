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

#ifndef HUGEROOM_H
#define HUGEROOM_H

#include "csgeom/math3d.h"

class csWorld;
class csSector;
class csPolygon3D;
class csPolygonSet;
class csThing;

/**
 * The Huge Room demo.
 */
class HugeRoom
{
private:
  ///
  csWorld* world;
  ///
  unsigned int seed;
  /// Dimensions of outer wall.
  float wall_dim;
  /// Number of horizontal and vertical triangles for every outer wall.
  int wall_num_tris;
  /// Minimum color for outer wall.
  float wall_min_red;
  float wall_min_green;
  float wall_min_blue;
  /// Maximum color for outer wall.
  float wall_max_red;
  float wall_max_green;
  float wall_max_blue;
  /// Max x,y, and z size of every polygon in thing.
  float thing_max_x;
  float thing_max_y;
  float thing_max_z;
  /// Minimum/maximum number of polygons for every thing.
  int thing_min_poly;
  int thing_max_poly;
  /// Minimum/maximum number of things in sector.
  int sector_min_thing;
  int sector_max_thing;
  /// Minimum/maximum x,y,z position of every thing.
  float sector_min_thing_x;
  float sector_min_thing_y;
  float sector_min_thing_z;
  float sector_max_thing_x;
  float sector_max_thing_y;
  float sector_max_thing_z;
  /// Minimum/maximum number of lights in sector.
  int sector_min_lights;
  int sector_max_lights;
  /// Maximum x, y, and z position where lights are created.
  float sector_light_max_pos;
  /// Minimum/maximum radius for every light.
  float sector_light_min_radius;
  float sector_light_max_radius;
  /// Minimum color for every light.
  float sector_light_min_red;
  float sector_light_min_green;
  float sector_light_min_blue;
  /// Maximum color for every light.
  float sector_light_max_red;
  float sector_light_max_green;
  float sector_light_max_blue;

private:
  ///
  csThing* create_thing (csSector* sector, const csVector3& pos);

  ///
  csPolygon3D* create_polygon (csSector* sector, csPolygonSet* thing,
  	const csVector3& p1, const csVector3& p2, const csVector3& p3,
	int txt);

  ///
  void create_wall (csSector* sector, csPolygonSet* thing,
  	const csVector3& p1, const csVector3& p2, const csVector3& p3,
  	const csVector3& p4, int hor_res, int ver_res, int txt);

public:
  ///
  HugeRoom ();

  ///
  ~HugeRoom () { }

  ///
  csSector* create_huge_world (csWorld* world);
};

#endif //HUGEROOM_H

