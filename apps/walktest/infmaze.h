/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

#ifndef INFMAZE_H
#define INFMAZE_H

#include "csgeom/math3d.h"
#include "csengine/portal.h"
#include "csengine/rview.h"

class csEngine;
class Sparse3D;
class WideSparse3D;
class csSector;
class csMaterialWrapper;
class csPolygon3D;
class csThing;

/**
 * Data for every room in the infinite maze.
 */
struct InfRoomData
{
  int x, y, z;
  csSector* sector;
  csThing* walls;
};

/**
 * The Infinite Maze demo.
 */
class InfiniteMaze
{
private:
  WideSparse3D* infinite_world;

public:
  ///
  InfiniteMaze ();

  ///
  ~InfiniteMaze ();


  ///
  void create_one_side (csThing* walls, char* pname,
	csMaterialWrapper* tm, csMaterialWrapper* tm2,
	float x1, float y1, float z1,
	float x2, float y2, float z2,
	float x3, float y3, float z3,
	float x4, float y4, float z4,
	float dx, float dy, float dz);
  /**
   * Create a six-sided room. This is used by the infinite maze.
   * x, y, z are the coordinates in integer units.
   */
  InfRoomData* create_six_room (csEngine* engine, int x, int y, int z);

  ///
  void connect_infinite (int x1, int y1, int z1, int x2, int y2, int z2, bool create_portal1 = true);

  ///
  void create_loose_portal (int x1, int y1, int z1, int x2, int y2, int z2);

  ///
  void random_loose_portals (int x1, int y1, int z1);
};

/**
 * Special subclass of PortalCS which knows how to handle
 * portal for which the destination has not been defined yet.
 */
class InfPortalCS : public csPortal
{
public:
  class LV
  {
  public:
    LV* next;
    csFrustumView lv;
  };
  LV* lviews;
  int x1, y1, z1;
  int x2, y2, z2;
  InfPortalCS () : csPortal () { lviews = NULL; }
  virtual void CheckFrustum (csFrustumView& lview, int alpha);
  virtual void CompleteSector ();
};

#endif //INFMAZE_H

