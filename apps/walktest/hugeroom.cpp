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

#include "cssysdef.h"
#include "walktest/hugeroom.h"
#include "walktest/walktest.h"
#include "csengine/sector.h"
#include "csengine/polygon.h"
#include "csengine/engine.h"
#include "csengine/texture.h"
#include "csengine/light.h"
#include "csengine/meshobj.h"

extern WalkTest* Sys;

extern float rand1 (float max);
extern float rand2 (float max);

static int pol_nr = 0;

HugeRoom::HugeRoom ()
{
  wall_dim = 100;
  wall_num_tris = 10;
  wall_min_red = .78;
  wall_min_green = .78;
  wall_min_blue = .78;
  wall_max_red = .82;
  wall_max_green = .82;
  wall_max_blue = .82;
  thing_max_x = 30;
  thing_max_y = 30;
  thing_max_z = 30;
  thing_min_poly = 10;
  thing_max_poly = 20;
  thing_cityblock_dim = 4;
  sector_min_thing = 5;
  sector_max_thing = 12;
  sector_min_thing_x = 30;
  sector_min_thing_y = 30;
  sector_min_thing_z = 30;
  sector_max_thing_x = 90;
  sector_max_thing_y = 90;
  sector_max_thing_z = 90;
  sector_min_lights = 0;
  sector_max_lights = 0;
  sector_light_max_pos = 90;
  sector_light_min_radius = 100;
  sector_light_max_radius = 400;
  sector_light_min_red = .6;
  sector_light_min_green = .6;
  sector_light_min_blue = .6;
  sector_light_max_red = 1;
  sector_light_max_green = 1;
  sector_light_max_blue = 1;
  seed = 1654594509;
}

void HugeRoom::create_wall (iThingState* thing_state,
	const csVector3& p1, const csVector3& p2, const csVector3& p3,
	const csVector3& p4, int hor_res, int ver_res, int txt)
{
  int i, j;
  iPolygon3D* p;
  for (i = 0 ; i < hor_res ; i++)
    for (j = 0 ; j < ver_res ; j++)
    {
      csVector3 v12a = p1 + ((float)i/(float)hor_res) * (p2-p1);
      csVector3 v43a = p4 + ((float)i/(float)hor_res) * (p3-p4);
      csVector3 v12b = p1 + ((float)(i+1)/(float)hor_res) * (p2-p1);
      csVector3 v43b = p4 + ((float)(i+1)/(float)hor_res) * (p3-p4);
      csVector3 v1 = v12a + ((float)j/(float)ver_res) * (v43a-v12a);
      csVector3 v2 = v12b + ((float)j/(float)ver_res) * (v43b-v12b);
      csVector3 v3 = v12b + ((float)(j+1)/(float)ver_res) * (v43b-v12b);
      csVector3 v4 = v12a + ((float)(j+1)/(float)ver_res) * (v43a-v12a);
      p = create_polygon (thing_state, v1, v2, v3, txt);
      create_polygon (thing_state, v1, v3, v4, txt);
    }
}

iPolygon3D* HugeRoom::create_polygon (iThingState* thing_state,
	const csVector3& p1, const csVector3& p2, const csVector3& p3,
	int txt)
{
  csMatrix3 t_m;
  csVector3 t_v (0, 0, 0);

  iMaterialWrapper* tm = NULL;
  switch (txt)
  {
    case 0: tm = NULL; break;
    case 1: tm = engine->FindMaterial ("txt"); break;
    case 2: tm = engine->FindMaterial ("txt2"); break;
  }

  char polname[10];
  sprintf (polname, "p%d", pol_nr);
  iPolygon3D* p = thing_state->CreatePolygon (polname);
  p->SetMaterial (tm);
  pol_nr++;

  p->CreateVertex (p1);
  p->CreateVertex (p2);
  p->CreateVertex (p3);

  p->SetTextureType (POLYTXT_GOURAUD);
  iPolyTexType* ptt = p->GetPolyTexType ();
  iPolyTexGouraud* gs = QUERY_INTERFACE (ptt, iPolyTexGouraud);
  iPolyTexFlat* fs = QUERY_INTERFACE (ptt, iPolyTexFlat);
  gs->Setup (p);
  fs->SetUV (0, 0, 0);
  fs->SetUV (1, 1, 0);
  fs->SetUV (2, 0, 1);
  fs->DecRef ();
  gs->DecRef ();
  // this is not needed anyway?
  p->SetTextureSpace (t_m, t_v);

  return p;
}

//#define ROOM_PURE_RANDOM
//#define ROOM_RANDOM_WALLS
//#define ROOM_CITYBLOCKS
//#define ROOM_SMALL
#define ROOM_CITY

static csMeshWrapper* CreateMeshWrapper (const char* name)
{
  iMeshObjectFactory* thing_fact = Sys->engine->GetThingType ()->NewFactory ();
  iMeshObject* mesh_obj = QUERY_INTERFACE (thing_fact, iMeshObject);
  thing_fact->DecRef ();
  csMeshWrapper* mesh_wrap = new csMeshWrapper (Sys->engine, mesh_obj);
  mesh_obj->DecRef ();
  mesh_wrap->SetName (name);
  Sys->engine->meshes.Push (mesh_wrap);
  return mesh_wrap;
}

csMeshWrapper* HugeRoom::create_thing (csSector* sector, const csVector3& pos)
{
  csMeshWrapper* thing = CreateMeshWrapper ("t");
  iThingState* thing_state = QUERY_INTERFACE (thing->GetMeshObject (),
  	iThingState);

#ifdef ROOM_SMALL
  int txt = (rand () & 0x8) ? 1 : 2;
  csVector3 p1 (rand2 (thing_max_x), rand2 (thing_max_y), rand2 (thing_max_z));
  csVector3 p2 (rand2 (thing_max_x), rand2 (thing_max_y), rand2 (thing_max_z));
  csVector3 p3 (rand2 (thing_max_x), rand2 (thing_max_y), rand2 (thing_max_z));
  create_polygon (thing_state, p1, p2, p3, txt);
#endif
#ifdef ROOM_CITYBLOCKS
  float y_low = -wall_dim;
  float y_high = y_low + rand1 (wall_dim) + 5;
  int txt = (rand () & 0x8) ? 1 : 2;
  csVector3 p1 (-thing_cityblock_dim/2,y_low,thing_cityblock_dim/2);
  csVector3 p2 (thing_cityblock_dim/2,y_low,thing_cityblock_dim/2);
  csVector3 p3 (thing_cityblock_dim/2,y_low,-thing_cityblock_dim/2);
  csVector3 p4 (-thing_cityblock_dim/2,y_low,-thing_cityblock_dim/2);
  csVector3 p5 (-thing_cityblock_dim/2,y_high,thing_cityblock_dim/2);
  csVector3 p6 (thing_cityblock_dim/2,y_high,thing_cityblock_dim/2);
  csVector3 p7 (thing_cityblock_dim/2,y_high,-thing_cityblock_dim/2);
  csVector3 p8 (-thing_cityblock_dim/2,y_high,-thing_cityblock_dim/2);
  create_wall (thing_state, p5, p6, p7, p8, 3, 3, txt);	// Top
  create_wall (thing_state, p8, p7, p3, p4, 3, 3, txt);	// Front
  create_wall (thing_state, p7, p6, p2, p3, 3, 3, txt);	// Right
  create_wall (thing_state, p5, p8, p4, p1, 3, 3, txt);	// Left
  create_wall (thing_state, p6, p5, p1, p2, 3, 3, txt);	// Back
#endif
#ifdef ROOM_RANDOM_WALLS
  thing_min_poly = 3;
  thing_min_poly = 8;
  int num = ((rand () >> 3) % (thing_max_poly-thing_min_poly+1)) + thing_min_poly;
  int i;
  for (i = 0 ; i < num ; i++)
  {
    int txt = (rand () & 0x8) ? 1 : 2;
    csVector3 p1 (rand2 (thing_max_x), rand2 (thing_max_y), rand2 (thing_max_z));
    csVector3 p2 (rand2 (thing_max_x), rand2 (thing_max_y), rand2 (thing_max_z));
    csVector3 p3 (rand2 (thing_max_x), rand2 (thing_max_y), rand2 (thing_max_z));
    csVector3 p4 = p2 + (p1-p2) + (p3-p2);
    create_wall (thing_state, p1, p2, p3, p4, 4, 4, txt);
  }
#endif
#ifdef ROOM_PURE_RANDOM
  int num = ((rand () >> 3) % (thing_max_poly-thing_min_poly+1)) + thing_min_poly;
  int i;
  csVector3 p1 (rand2 (thing_max_x), rand2 (thing_max_y), rand2 (thing_max_z));
  csVector3 p2 (rand2 (thing_max_x), rand2 (thing_max_y), rand2 (thing_max_z));
  csVector3 p3 (rand2 (thing_max_x), rand2 (thing_max_y), rand2 (thing_max_z));
  for (i = 0 ; i < num ; i++)
  {
    int txt = (rand () & 0x8) ? 1 : 2;
    create_polygon (thing_state, p1, p2, p3, txt);
    create_polygon (thing_state, p3, p2, p1, txt);
    p1 = p2;
    p2 = p3;
    p3 = csVector3 (rand2 (thing_max_x), rand2 (thing_max_y), rand2 (thing_max_z));
  }
#endif

  csMovable& move = thing->GetMovable ();
  move.SetSector (sector);
  csReversibleTransform obj;
  obj.SetT2O (csMatrix3 ());
  obj.SetOrigin (pos);
  move.SetTransform (obj);
  move.UpdateMove ();
  thing_state->DecRef ();

  return thing;
}

csMeshWrapper* HugeRoom::create_building (csSector* sector,
	const csVector3& pos,
	float xdim, float ydim, float zdim, float angle_y)
{
  csMeshWrapper* thing = CreateMeshWrapper ("t");
  iThingState* thing_state = QUERY_INTERFACE (thing->GetMeshObject (),
  	iThingState);

  float y_low = -wall_dim+1;
  float y_high = y_low + ydim;
  int txt = (rand () & 0x8) ? 1 : 2;
  csVector3 p1 (-xdim/2,y_low,zdim/2);
  csVector3 p2 (xdim/2,y_low,zdim/2);
  csVector3 p3 (xdim/2,y_low,-zdim/2);
  csVector3 p4 (-xdim/2,y_low,-zdim/2);
  csVector3 p5 (-xdim/2,y_high,zdim/2);
  csVector3 p6 (xdim/2,y_high,zdim/2);
  csVector3 p7 (xdim/2,y_high,-zdim/2);
  csVector3 p8 (-xdim/2,y_high,-zdim/2);
  int hor_div = 3;//7	(10)
  int ver_div = 4;//14	(20)
  create_wall (thing_state, p5, p6, p7, p8, hor_div, hor_div, txt);	// Top
  create_wall (thing_state, p8, p7, p3, p4, hor_div, ver_div, txt);	// Front
  create_wall (thing_state, p7, p6, p2, p3, hor_div, ver_div, txt);	// Right
  create_wall (thing_state, p5, p8, p4, p1, hor_div, ver_div, txt);	// Left
  create_wall (thing_state, p6, p5, p1, p2, hor_div, ver_div, txt);	// Back

  csMovable& move = thing->GetMovable ();
  move.SetSector (sector);
  csReversibleTransform obj;
  obj.SetT2O (csYRotMatrix3 (angle_y));
  obj.SetOrigin (pos);
  move.SetTransform (obj);
  move.UpdateMove ();
  thing_state->DecRef ();

  return thing;
}


csSector* HugeRoom::create_huge_world (csEngine* engine)
{
  this->engine = engine;
  csSector* room = engine->CreateCsSector ("sector");

  if (seed == 0) seed = rand ();
  srand (seed);
  Sys->Printf (MSG_INITIALIZATION, "Used seed %u.\n", seed);

  int i, num;

#ifdef ROOM_CITY
  create_building (room, csVector3 (-5, 0, 15), 10, 50, 10, 0);
  create_building (room, csVector3 (-2.5, 0, 5), 5, 30, 10, 0);
  create_building (room, csVector3 (-5, 0, -2.5), 5, 30, 10, -.78539816);
  create_building (room, csVector3 (-12.5, 0, -10), 5, 20, 10, -.78539816);
  create_building (room, csVector3 (13, 0, 17.5), 14, 40, 5, 0);
  create_building (room, csVector3 (10, 0, 7.5), 8, 20, 15, 0);
  create_building (room, csVector3 (-12.5, 0, 17.5), 5, 25, 5, 0);
  create_building (room, csVector3 (-17.5, 0, 17.5), 5, 25, 5, 0);
  create_building (room, csVector3 (-22.5, 0, 17.5), 5, 25, 5, 0);
  create_building (room, csVector3 (-27.5, 0, 17.5), 5, 30, 5, 0);
  create_building (room, csVector3 (-27.5, 0, 12.5), 5, 25, 5, 0);
  create_building (room, csVector3 (-25, 0, 0), 10, 30, 20, 0);
  create_building (room, csVector3 (12, 0, -10.5), 18, 30, 9, 0);
  create_building (room, csVector3 (-2.5, 0, -15), 5, 30, 15, -.78539816);
  create_building (room, csVector3 (19.5, 0, 2.5), 11, 40, 5, 0);
  create_building (room, csVector3 (22.5, 0, 2.5), 5, 20, 5, 0);
  create_building (room, csVector3 (22.5, 0, 7.5), 5, 25, 5, 0);
  create_building (room, csVector3 (25, 0, 15), 10, 45, 10, 0);
  create_building (room, csVector3 (-27.5, 0, -15), 5, 30, 10, 0);
  create_building (room, csVector3 (-27.5, 0, -22.5), 5, 20, 5, 0);
  create_building (room, csVector3 (25, 0, -7.5), 10, 30, 5, 0);
  create_building (room, csVector3 (33, 0, -12.5), 6, 30, 15, 0);
  create_building (room, csVector3 (-40, 0, 15), 10, 35, 10, 0);
  create_building (room, csVector3 (-37.5, 0, 5), 5, 20, 10, 0);
  create_building (room, csVector3 (-40, 0, -2.5), 10, 30, 5, 0);
  create_building (room, csVector3 (-40, 0, -7.5), 10, 40, 5, 0);
  create_building (room, csVector3 (-40, 0, -12.5), 10, 30, 5, 0);
  create_building (room, csVector3 (-37.5, 0, -17.5), 5, 25, 5, 0);
  create_building (room, csVector3 (-37.5, 0, -22.5), 5, 20, 5, 0);
  create_building (room, csVector3 (-37.5, 0, -27.5), 5, 30, 5, 0);
  create_building (room, csVector3 (-32.5, 0, 27.5), 5, 20, 5, 0);
  create_building (room, csVector3 (-27.5, 0, 27.5), 5, 15, 5, 0);
  create_building (room, csVector3 (-22.5, 0, 27.5), 5, 25, 5, 0);
  create_building (room, csVector3 (-12.5, 0, 30), 15, 20, 10, 0);
  create_building (room, csVector3 (-2.5, 0, 32.5), 5, 20, 15, 0);
  create_building (room, csVector3 (8, 0, 27.5), 4, 20, 5, 0);
  create_building (room, csVector3 (8, 0, 32.5), 4, 30, 5, 0);
  create_building (room, csVector3 (12.5, 0, 27.5), 5, 15, 5, 0);
  create_building (room, csVector3 (17.5, 0, 27.5), 5, 20, 5, 0);
  create_building (room, csVector3 (27.5, 0, 30), 15, 40, 10, 0);
  create_building (room, csVector3 (37.5, 0, 25), 5, 10, 10, 0);
  create_building (room, csVector3 (37.5, 0, 15), 5, 15, 10, 0);
  create_building (room, csVector3 (37.5, 0, 5), 5, 20, 10, 0);
  create_building (room, csVector3 (5, 0, -22.5), 10, 30, 5, 0);
  create_building (room, csVector3 (12.5, 0, -22.5), 5, 50, 5, 0);
  create_building (room, csVector3 (17.5, 0, -22.5), 5, 20, 5, 0);
  create_building (room, csVector3 (27.5, 0, -22.5), 15, 25, 5, 0);
  create_building (room, csVector3 (-10, 0, -35), 20, 15, 10, 0);
  create_building (room, csVector3 (2.5, 0, -35), 5, 35, 10, 0);
  create_building (room, csVector3 (7.5, 0, -35), 5, 30, 10, 0);
  create_building (room, csVector3 (15, 0, -32.5), 10, 20, 5, 0);
  create_building (room, csVector3 (25, 0, -32.5), 10, 30, 5, 0);
  create_building (room, csVector3 (35, 0, -35), 10, 35, 10, 0);
#endif
#ifdef ROOM_SMALL
  create_thing (room, csVector3 (-wall_dim/2, -wall_dim/2, -wall_dim/2));

  create_thing (room, csVector3 (-wall_dim/2, -wall_dim/2,  wall_dim/2));
  create_thing (room, csVector3 (-wall_dim/2, -wall_dim/2,  wall_dim/2));

  create_thing (room, csVector3 (-wall_dim/2,  wall_dim/2, -wall_dim/2));
  create_thing (room, csVector3 (-wall_dim/2,  wall_dim/2, -wall_dim/2));
  create_thing (room, csVector3 (-wall_dim/2,  wall_dim/2, -wall_dim/2));

  create_thing (room, csVector3 (-wall_dim/2,  wall_dim/2,  wall_dim/2));
  create_thing (room, csVector3 (-wall_dim/2,  wall_dim/2,  wall_dim/2));
  create_thing (room, csVector3 (-wall_dim/2,  wall_dim/2,  wall_dim/2));
  create_thing (room, csVector3 (-wall_dim/2,  wall_dim/2,  wall_dim/2));

  create_thing (room, csVector3 ( wall_dim/2, -wall_dim/2, -wall_dim/2));
  create_thing (room, csVector3 ( wall_dim/2, -wall_dim/2, -wall_dim/2));
  create_thing (room, csVector3 ( wall_dim/2, -wall_dim/2, -wall_dim/2));
  create_thing (room, csVector3 ( wall_dim/2, -wall_dim/2, -wall_dim/2));
  create_thing (room, csVector3 ( wall_dim/2, -wall_dim/2, -wall_dim/2));

  create_thing (room, csVector3 ( wall_dim/2, -wall_dim/2,  wall_dim/2));
  create_thing (room, csVector3 ( wall_dim/2, -wall_dim/2,  wall_dim/2));
  create_thing (room, csVector3 ( wall_dim/2, -wall_dim/2,  wall_dim/2));
  create_thing (room, csVector3 ( wall_dim/2, -wall_dim/2,  wall_dim/2));
  create_thing (room, csVector3 ( wall_dim/2, -wall_dim/2,  wall_dim/2));
  create_thing (room, csVector3 ( wall_dim/2, -wall_dim/2,  wall_dim/2));

  create_thing (room, csVector3 ( wall_dim/2,  wall_dim/2, -wall_dim/2));
  create_thing (room, csVector3 ( wall_dim/2,  wall_dim/2, -wall_dim/2));
  create_thing (room, csVector3 ( wall_dim/2,  wall_dim/2, -wall_dim/2));
  create_thing (room, csVector3 ( wall_dim/2,  wall_dim/2, -wall_dim/2));
  create_thing (room, csVector3 ( wall_dim/2,  wall_dim/2, -wall_dim/2));
  create_thing (room, csVector3 ( wall_dim/2,  wall_dim/2, -wall_dim/2));
  create_thing (room, csVector3 ( wall_dim/2,  wall_dim/2, -wall_dim/2));

  create_thing (room, csVector3 ( wall_dim/2,  wall_dim/2,  wall_dim/2));
  create_thing (room, csVector3 ( wall_dim/2,  wall_dim/2,  wall_dim/2));
  create_thing (room, csVector3 ( wall_dim/2,  wall_dim/2,  wall_dim/2));
  create_thing (room, csVector3 ( wall_dim/2,  wall_dim/2,  wall_dim/2));
  create_thing (room, csVector3 ( wall_dim/2,  wall_dim/2,  wall_dim/2));
  create_thing (room, csVector3 ( wall_dim/2,  wall_dim/2,  wall_dim/2));
  create_thing (room, csVector3 ( wall_dim/2,  wall_dim/2,  wall_dim/2));
  create_thing (room, csVector3 ( wall_dim/2,  wall_dim/2,  wall_dim/2));
#else
#ifdef ROOM_CITYBLOCKS
  float x, y;
  float cnt = wall_dim/thing_cityblock_dim;
  for (x = -cnt/2 ; x < cnt/2 ; x++)
    for (y = -cnt/2 ; y < cnt/2 ; y++)
    {
      if ((rand () & 0xc) == 0)
        create_thing (room, csVector3 (x*thing_cityblock_dim, 0, y*thing_cityblock_dim));
    }
#else
  num = ((rand () >> 3) % (sector_max_thing-sector_min_thing+1)) + sector_min_thing;
  for (i = 0 ; i < num ; i++)
  {
    float x = rand1 (sector_max_thing_x-sector_min_thing_x+1)+sector_min_thing_x;
    if (rand () & 0x8) x = -x;
    float y = rand1 (sector_max_thing_y-sector_min_thing_y+1)+sector_min_thing_y;
    if (rand () & 0x8) y = -y;
    float z = rand1 (sector_max_thing_z-sector_min_thing_z+1)+sector_min_thing_z;
    if (rand () & 0x8) z = -z;
    create_thing (room, csVector3 (x, y, z));
  }
#endif
#endif

  num = ((rand () >> 3) % (sector_max_lights-sector_min_lights+1)) + sector_min_lights;
  for (i = 0 ; i < num ; i++)
  {
    csStatLight* light = new csStatLight (
    	rand2 (sector_light_max_pos), rand2 (sector_light_max_pos), rand2 (sector_light_max_pos),
    	sector_light_min_radius+rand1 (sector_light_max_radius-sector_light_min_radius+1),
    	rand1 (sector_light_max_red-sector_light_min_red)+sector_light_min_red,
    	rand1 (sector_light_max_green-sector_light_min_green)+sector_light_min_green,
    	rand1 (sector_light_max_blue-sector_light_min_blue)+sector_light_min_blue, false);
    room->AddLight (light);
  }

#if defined(ROOM_CITY)
  csMeshWrapper* floorthing = CreateMeshWrapper ("floor");
  iThingState* thing_state = QUERY_INTERFACE (floorthing->GetMeshObject (),
  	iThingState);
  create_wall (thing_state,
  	csVector3 (-wall_dim, -wall_dim+1, wall_dim),
  	csVector3 (wall_dim, -wall_dim+1, wall_dim),
  	csVector3 (wall_dim, -wall_dim+1, -wall_dim),
	csVector3 (-wall_dim, -wall_dim+1, -wall_dim), 40, 40, 0);
  thing_state->DecRef ();
  floorthing->GetMovable ().SetSector (room);
  floorthing->GetMovable ().UpdateMove ();
#elif !defined(ROOM_SMALL)
  csMeshWrapper* floorthing = CreateMeshWrapper ("floor");
  iThingState* thing_state = QUERY_INTERFACE (floorthing->GetMeshObject (),
  	iThingState);
  create_wall (thing_state, csVector3 (-3, -1, 3), csVector3 (3, -1, 3),
  	csVector3 (3, -1, -3), csVector3 (-3, -1, -3), 4, 4, 0);
  create_wall (thing_state, csVector3 (-3, -1, -3), csVector3 (3, -1, -3),
  	csVector3 (3, -1, 3), csVector3 (-3, -1, 3), 4, 4, 0);
  thing_state->DecRef ();
  floorthing->GetMovable ().SetSector (room);
  floorthing->GetMovable ().UpdateMove ();
#endif

  iMeshWrapper* walls = engine->CreateSectorWallsMesh (room, "walls");
  thing_state = QUERY_INTERFACE (walls->GetMeshObject (),
  	iThingState);
  create_wall (thing_state,
  	csVector3 (-wall_dim,wall_dim,wall_dim), csVector3 (wall_dim,wall_dim,wall_dim),
	csVector3 (wall_dim,-wall_dim,wall_dim), csVector3 (-wall_dim,-wall_dim,wall_dim),
	wall_num_tris, wall_num_tris, 0);
  create_wall (thing_state,
  	csVector3 (wall_dim,wall_dim,-wall_dim), csVector3 (-wall_dim,wall_dim,-wall_dim),
	csVector3 (-wall_dim,-wall_dim,-wall_dim), csVector3 (wall_dim,-wall_dim,-wall_dim),
	wall_num_tris, wall_num_tris, 0);
  create_wall (thing_state,
  	csVector3 (-wall_dim,wall_dim,-wall_dim), csVector3 (-wall_dim,wall_dim,wall_dim),
	csVector3 (-wall_dim,-wall_dim,wall_dim), csVector3 (-wall_dim,-wall_dim,-wall_dim),
	wall_num_tris, wall_num_tris, 0);
  create_wall (thing_state,
  	csVector3 (wall_dim,wall_dim,wall_dim), csVector3 (wall_dim,wall_dim,-wall_dim),
	csVector3 (wall_dim,-wall_dim,-wall_dim), csVector3 (wall_dim,-wall_dim,wall_dim),
	wall_num_tris, wall_num_tris, 0);
  create_wall (thing_state,
  	csVector3 (-wall_dim,-wall_dim,wall_dim), csVector3 (wall_dim,-wall_dim,wall_dim),
	csVector3 (wall_dim,-wall_dim,-wall_dim), csVector3 (-wall_dim,-wall_dim,-wall_dim),
	wall_num_tris, wall_num_tris, 0);
  create_wall (thing_state,
  	csVector3 (-wall_dim,wall_dim,-wall_dim), csVector3 (wall_dim,wall_dim,-wall_dim),
	csVector3 (wall_dim,wall_dim,wall_dim), csVector3 (-wall_dim,wall_dim,wall_dim),
	wall_num_tris, wall_num_tris, 0);
  thing_state->DecRef ();

  Sys->Printf (MSG_INITIALIZATION, "Number of polygons: %d\n", pol_nr);
  room->UseCuller ("@@@ (NOT WORKING!!!)");
  return room;
}

