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
#include "cssys/system.h"
#include "walktest/infmaze.h"
#include "walktest/walktest.h"
#include "csengine/sector.h"
#include "csengine/portal.h"
#include "csengine/polygon.h"
#include "csengine/thing.h"
#include "csengine/engine.h"
#include "csengine/texture.h"
#include "csengine/material.h"
#include "csengine/light.h"
#include "csengine/lghtmap.h"
#include "cstool/collider.h"
#include "csutil/dataobj.h"
#include "csutil/sparse3d.h"

InfiniteMaze::InfiniteMaze ()
{
  infinite_world = new csWideSparse3D ();
}

InfiniteMaze::~InfiniteMaze ()
{
  delete infinite_world;
}

void InfiniteMaze::create_one_side (iThingState* walls_state, char* pname,
	iMaterialWrapper* tm, iMaterialWrapper* tm2,
	float x1, float y1, float z1,
	float x2, float y2, float z2,
	float x3, float y3, float z3,
	float x4, float y4, float z4,
	float dx, float dy, float dz)
{
  float cx, cy, cz;
  cx = (x1+x2+x3+x4) / 4;
  cy = (y1+y2+y3+y4) / 4;
  cz = (z1+z2+z3+z4) / 4;
  x1 -= cx; y1 -= cy; z1 -= cz;
  x2 -= cx; y2 -= cy; z2 -= cz;
  x3 -= cx; y3 -= cy; z3 -= cz;
  x4 -= cx; y4 -= cy; z4 -= cz;
  float sx, sy, sz;
  if (dx) { sy = sz = .9; sx = 0; }
  else if (dy) { sx = sz = .9; sy = 0; }
  else { sx = sy = .9; sz = 0; }

  iPolygon3D* p;
  p = walls_state->CreatePolygon (pname);
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (cx+sx*x1, cy+sy*y1, cz+sz*z1));
  p->CreateVertex (csVector3 (cx+sx*x2, cy+sy*y2, cz+sz*z2));
  p->CreateVertex (csVector3 (cx+sx*x3, cy+sy*y3, cz+sz*z3));
  p->CreateVertex (csVector3 (cx+sx*x4, cy+sy*y4, cz+sz*z4));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 1);

  float nx1 = x1+dx, ny1 = y1+dy, nz1 = z1+dz;
  float nx2 = x2+dx, ny2 = y2+dy, nz2 = z2+dz;
  float nx3 = x3+dx, ny3 = y3+dy, nz3 = z3+dz;
  float nx4 = x4+dx, ny4 = y4+dy, nz4 = z4+dz;
  float ncx = cx+dx, ncy = cy+dy, ncz = cz+dz;

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm2);
  p->CreateVertex (csVector3 (cx+sx*x2, cy+sy*y2, cz+sz*z2));
  p->CreateVertex (csVector3 (cx+sx*x1, cy+sy*y1, cz+sz*z1));
  p->CreateVertex (csVector3 (ncx+sx*nx1, ncy+sy*ny1, ncz+sz*nz1));
  p->CreateVertex (csVector3 (ncx+sx*nx2, ncy+sy*ny2, ncz+sz*nz2));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 1);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm2);
  p->CreateVertex (csVector3 (cx+sx*x3, cy+sy*y3, cz+sz*z3));
  p->CreateVertex (csVector3 (cx+sx*x2, cy+sy*y2, cz+sz*z2));
  p->CreateVertex (csVector3 (ncx+sx*nx2, ncy+sy*ny2, ncz+sz*nz2));
  p->CreateVertex (csVector3 (ncx+sx*nx3, ncy+sy*ny3, ncz+sz*nz3));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 1);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm2);
  p->CreateVertex (csVector3 (cx+sx*x4, cy+sy*y4, cz+sz*z4));
  p->CreateVertex (csVector3 (cx+sx*x3, cy+sy*y3, cz+sz*z3));
  p->CreateVertex (csVector3 (ncx+sx*nx3, ncy+sy*ny3, ncz+sz*nz3));
  p->CreateVertex (csVector3 (ncx+sx*nx4, ncy+sy*ny4, ncz+sz*nz4));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 1);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm2);
  p->CreateVertex (csVector3 (cx+sx*x1, cy+sy*y1, cz+sz*z1));
  p->CreateVertex (csVector3 (cx+sx*x4, cy+sy*y4, cz+sz*z4));
  p->CreateVertex (csVector3 (ncx+sx*nx4, ncy+sy*ny4, ncz+sz*nz4));
  p->CreateVertex (csVector3 (ncx+sx*nx1, ncy+sy*ny1, ncz+sz*nz1));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 1);
}

float rand1 (float max)
{
  float f = (float)rand ();
  return max*f/RAND_MAX;
}

float rand2 (float max)
{
  float f = (float)rand ();
  return max*(f/(RAND_MAX/2)-1);
}

InfRoomData* InfiniteMaze::create_six_room (iEngine* engine, int x, int y, int z)
{
  char buf[50];
  sprintf (buf, "r%d_%d_%d", x, y, z);
  iSector* room = engine->CreateSector (buf);
  iMeshWrapper* walls = engine->CreateSectorWallsMesh (room, "walls");
  iThingState* walls_state = QUERY_INTERFACE (walls->GetMeshObject (),
  	iThingState);
  float dx, dy, dz;
  dx = 2.0*(float)x;
  dy = 2.0*(float)y;
  dz = 2.0*(float)z;
  iMaterialWrapper* t = engine->FindMaterial ("txt");
  iMaterialWrapper* t2 = engine->FindMaterial ("txt2");
  float s = 1;

  create_one_side (walls_state, "n", t, t2, dx-s,dy+s,dz+s,  dx+s,dy+s,dz+s,  dx+s,dy-s,dz+s,  dx-s,dy-s,dz+s, 0,0,-.1);
  create_one_side (walls_state, "e", t, t2, dx+s,dy+s,dz+s,  dx+s,dy+s,dz-s,  dx+s,dy-s,dz-s,  dx+s,dy-s,dz+s, -.1,0,0);
  create_one_side (walls_state, "w", t, t2, dx-s,dy+s,dz+s,  dx-s,dy-s,dz+s,  dx-s,dy-s,dz-s,  dx-s,dy+s,dz-s, .1,0,0);
  create_one_side (walls_state, "s", t, t2, dx+s,dy+s,dz-s,  dx-s,dy+s,dz-s,  dx-s,dy-s,dz-s,  dx+s,dy-s,dz-s, 0,0,.1);
  create_one_side (walls_state, "f", t, t2, dx-s,dy-s,dz+s,  dx+s,dy-s,dz+s,  dx+s,dy-s,dz-s,  dx-s,dy-s,dz-s, 0,.1,0);
  create_one_side (walls_state, "c", t, t2, dx-s,dy+s,dz-s,  dx+s,dy+s,dz-s,  dx+s,dy+s,dz+s,  dx-s,dy+s,dz+s, 0,-.1,0);

  iStatLight* light = engine->CreateLight ("",
  	csVector3 (dx+rand2 (.9*s), dy+rand2 (.9*s), dz+rand2 (.9*s)),
	1+rand1 (3),
  	csColor (rand1 (1), rand1 (1), rand1 (1)),
	false);
  room->AddLight (light);

  InfRoomData* ird = new InfRoomData ();
  ird->x = x;
  ird->y = y;
  ird->z = z;
  ird->sector = room;
  ird->walls = walls;
  ird->walls_state = walls_state;
  infinite_world->Set (x, y, z, (void*)ird);
  csDataObject* irddata = new csDataObject (ird);
  room->QueryObject ()->ObjAdd (irddata);
  return ird;
}

void InfiniteMaze::connect_infinite (int x1, int y1, int z1, int x2, int y2, int z2, bool create_portal1)
{
  InfRoomData* s1 = (InfRoomData*)(infinite_world->Get (x1, y1, z1));
  InfRoomData* s2 = (InfRoomData*)(infinite_world->Get (x2, y2, z2));
  char* p1, * p2;
  if (x1 == x2)
    if (y1 == y2)
      if (z1 < z2) { p1 = "n"; p2 = "s"; }
      else { p1 = "s"; p2 = "n"; }
    else
      if (y1 < y2) { p1 = "c"; p2 = "f"; }
      else { p1 = "f"; p2 = "c"; }
  else
    if (x1 < x2) { p1 = "e"; p2 = "w"; }
    else { p1 = "w"; p2 = "e"; }
  iPolygon3D* po1 = s1->walls_state->GetPolygon (p1);
  iPolygon3D* po2 = s2->walls_state->GetPolygon (p2);
  if (create_portal1) po1->CreatePortal (s2->sector);
  po2->CreatePortal (s1->sector);
}

static bool CompleteSectorCB (iPortal* portal, iBase* context, void* data)
{
  InfPortalCS* ipc = (InfPortalCS*)data;
  iFrustumView* fv;
  if (context) fv = QUERY_INTERFACE (context, iFrustumView);
  else fv = NULL;
  if (fv)
  {
    if (!fv->IsDynamic ())
    {
      // If we want to shine light through this portal but it doesn't
      // really exist yet then we remember the csFrustumView for later.
      LV* lv = new LV ();
      lv->next = ipc->lviews;
      ipc->lviews = lv;
      lv->lv = fv;
      // Make a copy of the current context and remember it.
      lv->ctxt = fv->CopyFrustumContext ();
    }
    else
      fv->DecRef ();
    return false;
  }
  else
  {
    extern WalkTest* Sys;
    InfiniteMaze* infinite_maze = Sys->infinite_maze;
    InfRoomData* ird = infinite_maze->create_six_room (Sys->Engine,
    	ipc->x2, ipc->y2, ipc->z2);
    iSector* is = ird->sector;
    csSector* s = is->GetPrivateObject (); //@@@
    infinite_maze->connect_infinite (ipc->x1, ipc->y1, ipc->z1,
    	ipc->x2, ipc->y2, ipc->z2, false);
    portal->SetSector (is);
    infinite_maze->random_loose_portals (ipc->x2, ipc->y2, ipc->z2);
    s->InitLightMaps (false);
    s->ShineLights ();
    s->CreateLightMaps (Sys->myG3D);
    while (ipc->lviews)
    {
      int old_draw_busy = s->draw_busy;
      s->draw_busy = 0;
      iFrustumView* fv = ipc->lviews->lv;
      csFrustumContext* orig_ctxt = fv->GetFrustumContext ();
      fv->SetFrustumContext (ipc->lviews->ctxt);
      portal->CheckFrustum (fv, 0);
      fv->RestoreFrustumContext (orig_ctxt);
      s->draw_busy = old_draw_busy;

      LV* n = ipc->lviews->next;
      delete ipc->lviews;
      ipc->lviews = n;
    }
    iPolygonMesh* mesh = QUERY_INTERFACE (ird->walls->GetMeshObject (),
  	iPolygonMesh);
    iObject* io = QUERY_INTERFACE (ird->walls, iObject);
    (void)new csColliderWrapper (io, Sys->collide_system, mesh);
    io->DecRef ();
    mesh->DecRef ();
printf ("5\n"); fflush (stdout);
    return true;
  }
}

void InfiniteMaze::create_loose_portal (int x1, int y1, int z1,
	int x2, int y2, int z2)
{
  char* p1;
  if (x1 == x2)
    if (y1 == y2)
      if (z1 < z2) p1 = "n";
      else p1 = "s";
    else
      if (y1 < y2) p1 = "c";
      else p1 = "f";
  else
    if (x1 < x2) p1 = "e";
    else p1 = "w";
  InfRoomData* s = (InfRoomData*)(infinite_world->Get (x1, y1, z1));
  iPolygon3D* po = s->walls_state->GetPolygon (p1);
  iPortal* portal = po->CreateNullPortal ();
  InfPortalCS* prt = new InfPortalCS ();
  prt->x1 = x1; prt->y1 = y1; prt->z1 = z1;
  prt->x2 = x2; prt->y2 = y2; prt->z2 = z2;
  infinite_world->Set (x2, y2, z2, (void*)1);
  portal->SetPortalSectorCallback (CompleteSectorCB,
  	(void*)prt);
}

void InfiniteMaze::random_loose_portals (int x1, int y1, int z1)
{
  int cnt = (rand () >> 3) % 2 + 1;
  int max = 1000;
  while (cnt > 0 && max > 0)
  {
    switch ((rand () >> 3) % 6)
    {
      case 0:
        if (!infinite_world->Get (x1+1, y1, z1)) { create_loose_portal (x1, y1, z1, x1+1, y1, z1); cnt--; }
	break;
      case 1:
        if (!infinite_world->Get (x1-1, y1, z1)) { create_loose_portal (x1, y1, z1, x1-1, y1, z1); cnt--; }
	break;
      case 2:
        if (!infinite_world->Get (x1, y1+1, z1)) { create_loose_portal (x1, y1, z1, x1, y1+1, z1); cnt--; }
	break;
      case 3:
        if (!infinite_world->Get (x1, y1-1, z1)) { create_loose_portal (x1, y1, z1, x1, y1-1, z1); cnt--; }
	break;
      case 4:
        if (!infinite_world->Get (x1, y1, z1+1)) { create_loose_portal (x1, y1, z1, x1, y1, z1+1); cnt--; }
	break;
      case 5:
        if (!infinite_world->Get (x1, y1, z1-1)) { create_loose_portal (x1, y1, z1, x1, y1, z1-1); cnt--; }
	break;
    }
    max--;
  }
}

