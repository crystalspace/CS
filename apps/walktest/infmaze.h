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

#ifndef __INFMAZE_H__
#define __INFMAZE_H__

#include "csgeom/math3d.h"
#include "iengine/portal.h"

class csSparse3D;
class csWideSparse3D;
class csFrustumContext;
struct iEngine;
struct iSector;
struct iMeshWrapper;
struct iThingState;
struct iThingFactoryState;
struct iMaterialWrapper;
struct iMeshWrapper;
struct iFrustumView;

/**
 * Data for every room in the infinite maze.
 */
struct InfRoomData
{
  ~InfRoomData ();
  int x, y, z;
  iSector* sector;
  iMeshWrapper* walls;
  iThingFactoryState* walls_fact_state;
};

/**
 * The Infinite Maze demo.
 */
class InfiniteMaze
{
private:
  csWideSparse3D* infinite_world;

public:
  ///
  InfiniteMaze ();

  ///
  ~InfiniteMaze ();


  ///
  void create_one_side (iThingFactoryState* walls_state, char* pname,
	iMaterialWrapper* tm, iMaterialWrapper* tm2,
	float x1, float y1, float z1,
	float x2, float y2, float z2,
	float x3, float y3, float z3,
	float x4, float y4, float z4,
	float dx, float dy, float dz);
  /**
   * Create a six-sided room. This is used by the infinite maze.
   * x, y, z are the coordinates in integer units.
   */
  InfRoomData* create_six_room (iEngine* engine, int x, int y, int z);

  ///
  void connect_infinite (int x1, int y1, int z1, int x2, int y2, int z2, bool create_portal1 = true);

  ///
  void create_loose_portal (int x1, int y1, int z1, int x2, int y2, int z2);

  ///
  void random_loose_portals (int x1, int y1, int z1);
};

class LV
{
public:
  LV () : next (0), lv (0), ctxt (0) { }

  LV* next;
  iFrustumView* lv;
  csFrustumContext* ctxt;
};

/**
 * Structure that is kept with a portal to remember lighting information
 * that still has to be computed.
 */
struct InfPortalCS : public iPortalCallback
{
  LV* lviews;
  int x1, y1, z1;
  int x2, y2, z2;

  SCF_DECLARE_IBASE;
  InfPortalCS ();
  virtual ~InfPortalCS ();
  virtual bool Traverse (iPortal* portal, iBase* context);
};

#endif // __INFMAZE_H__

