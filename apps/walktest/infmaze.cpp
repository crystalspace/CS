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

#include "cssysdef.h"
#include "cssys/system.h"
#include "walktest/infmaze.h"
#include "walktest/walktest.h"
#include "csengine/sector.h"
#include "csengine/portal.h"
#include "csengine/polygon.h"
#include "csengine/engine.h"
#include "csengine/texture.h"
#include "csengine/material.h"
#include "csengine/light.h"
#include "csengine/lghtmap.h"
#include "csengine/collider.h"
#include "csobject/dataobj.h"
#include "csutil/sparse3d.h"

InfiniteMaze::InfiniteMaze ()
{
  infinite_world = new WideSparse3D ();
}

InfiniteMaze::~InfiniteMaze ()
{
  if (infinite_world) delete infinite_world;
}

void InfiniteMaze::create_one_side (csSector* room, char* pname,
	csMaterialWrapper* tm, csMaterialWrapper* tm2,
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

  csPolygon3D* p;
  p = room->NewPolygon (tm);
  p->SetName (pname);
  p->AddVertex (cx+sx*x1, cy+sy*y1, cz+sz*z1);
  p->AddVertex (cx+sx*x2, cy+sy*y2, cz+sz*z2);
  p->AddVertex (cx+sx*x3, cy+sy*y3, cz+sz*z3);
  p->AddVertex (cx+sx*x4, cy+sy*y4, cz+sz*z4);
  p->SetTextureSpace (p->Vobj (0), p->Vobj (1), 1);

  float nx1 = x1+dx, ny1 = y1+dy, nz1 = z1+dz;
  float nx2 = x2+dx, ny2 = y2+dy, nz2 = z2+dz;
  float nx3 = x3+dx, ny3 = y3+dy, nz3 = z3+dz;
  float nx4 = x4+dx, ny4 = y4+dy, nz4 = z4+dz;
  float ncx = cx+dx, ncy = cy+dy, ncz = cz+dz;

  p = room->NewPolygon (tm2);
  p->AddVertex (cx+sx*x2, cy+sy*y2, cz+sz*z2);
  p->AddVertex (cx+sx*x1, cy+sy*y1, cz+sz*z1);
  p->AddVertex (ncx+sx*nx1, ncy+sy*ny1, ncz+sz*nz1);
  p->AddVertex (ncx+sx*nx2, ncy+sy*ny2, ncz+sz*nz2);
  p->SetTextureSpace (p->Vobj (0), p->Vobj (1), 1);

  p = room->NewPolygon (tm2);
  p->AddVertex (cx+sx*x3, cy+sy*y3, cz+sz*z3);
  p->AddVertex (cx+sx*x2, cy+sy*y2, cz+sz*z2);
  p->AddVertex (ncx+sx*nx2, ncy+sy*ny2, ncz+sz*nz2);
  p->AddVertex (ncx+sx*nx3, ncy+sy*ny3, ncz+sz*nz3);
  p->SetTextureSpace (p->Vobj (0), p->Vobj (1), 1);

  p = room->NewPolygon (tm2);
  p->AddVertex (cx+sx*x4, cy+sy*y4, cz+sz*z4);
  p->AddVertex (cx+sx*x3, cy+sy*y3, cz+sz*z3);
  p->AddVertex (ncx+sx*nx3, ncy+sy*ny3, ncz+sz*nz3);
  p->AddVertex (ncx+sx*nx4, ncy+sy*ny4, ncz+sz*nz4);
  p->SetTextureSpace (p->Vobj (0), p->Vobj (1), 1);

  p = room->NewPolygon (tm2);
  p->AddVertex (cx+sx*x1, cy+sy*y1, cz+sz*z1);
  p->AddVertex (cx+sx*x4, cy+sy*y4, cz+sz*z4);
  p->AddVertex (ncx+sx*nx4, ncy+sy*ny4, ncz+sz*nz4);
  p->AddVertex (ncx+sx*nx1, ncy+sy*ny1, ncz+sz*nz1);
  p->SetTextureSpace (p->Vobj (0), p->Vobj (1), 1);
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

InfRoomData* InfiniteMaze::create_six_room (csEngine* engine, int x, int y, int z)
{
  char buf[50];
  sprintf (buf, "r%d_%d_%d", x, y, z);
  csSector* room = engine->CreateCsSector (buf);
  float dx, dy, dz;
  dx = 2.0*(float)x;
  dy = 2.0*(float)y;
  dz = 2.0*(float)z;
  csMaterialWrapper* t = engine->GetMaterials ()->FindByName ("txt");
  csMaterialWrapper* t2 = engine->GetMaterials ()->FindByName ("txt2");
  float s = 1;

  create_one_side (room, "n", t, t2, dx-s,dy+s,dz+s,  dx+s,dy+s,dz+s,  dx+s,dy-s,dz+s,  dx-s,dy-s,dz+s, 0,0,-.1);
  create_one_side (room, "e", t, t2, dx+s,dy+s,dz+s,  dx+s,dy+s,dz-s,  dx+s,dy-s,dz-s,  dx+s,dy-s,dz+s, -.1,0,0);
  create_one_side (room, "w", t, t2, dx-s,dy+s,dz+s,  dx-s,dy-s,dz+s,  dx-s,dy-s,dz-s,  dx-s,dy+s,dz-s, .1,0,0);
  create_one_side (room, "s", t, t2, dx+s,dy+s,dz-s,  dx-s,dy+s,dz-s,  dx-s,dy-s,dz-s,  dx+s,dy-s,dz-s, 0,0,.1);
  create_one_side (room, "f", t, t2, dx-s,dy-s,dz+s,  dx+s,dy-s,dz+s,  dx+s,dy-s,dz-s,  dx-s,dy-s,dz-s, 0,.1,0);
  create_one_side (room, "c", t, t2, dx-s,dy+s,dz-s,  dx+s,dy+s,dz-s,  dx+s,dy+s,dz+s,  dx-s,dy+s,dz+s, 0,-.1,0);

  csStatLight* light = new csStatLight (dx+rand2 (.9*s), dy+rand2 (.9*s), dz+rand2 (.9*s), 1+rand1 (3),
  	rand1 (1), rand1 (1), rand1 (1), false);
  room->AddLight (light);

  InfRoomData* ird = new InfRoomData ();
  ird->x = x;
  ird->y = y;
  ird->z = z;
  ird->sector = room;
  infinite_world->set (x, y, z, (void*)ird);
  csDataObject* irddata = new csDataObject(ird);
  room->ObjAdd(irddata);
  return ird;
}

void InfiniteMaze::connect_infinite (int x1, int y1, int z1, int x2, int y2, int z2, bool create_portal1)
{
  InfRoomData* s1 = (InfRoomData*)(infinite_world->get (x1, y1, z1));
  InfRoomData* s2 = (InfRoomData*)(infinite_world->get (x2, y2, z2));
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
  csPolygon3D* po1 = s1->sector->GetPolygon3D (p1);
  csPolygon3D* po2 = s2->sector->GetPolygon3D (p2);
  if (create_portal1) po1->SetCSPortal (s2->sector);
  po2->SetCSPortal (s1->sector);
}

void InfiniteMaze::create_loose_portal (int x1, int y1, int z1, int x2, int y2, int z2)
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
  InfRoomData* s = (InfRoomData*)(infinite_world->get (x1, y1, z1));
  csPolygon3D* po = s->sector->GetPolygon3D (p1);
  InfPortalCS* prt = new InfPortalCS ();
  po->SetPortal (prt);
  prt->SetSector (NULL);
  prt->x1 = x1; prt->y1 = y1; prt->z1 = z1;
  prt->x2 = x2; prt->y2 = y2; prt->z2 = z2;
  infinite_world->set (x2, y2, z2, (void*)1);
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
        if (!infinite_world->get (x1+1, y1, z1)) { create_loose_portal (x1, y1, z1, x1+1, y1, z1); cnt--; }
	break;
      case 1:
        if (!infinite_world->get (x1-1, y1, z1)) { create_loose_portal (x1, y1, z1, x1-1, y1, z1); cnt--; }
	break;
      case 2:
        if (!infinite_world->get (x1, y1+1, z1)) { create_loose_portal (x1, y1, z1, x1, y1+1, z1); cnt--; }
	break;
      case 3:
        if (!infinite_world->get (x1, y1-1, z1)) { create_loose_portal (x1, y1, z1, x1, y1-1, z1); cnt--; }
	break;
      case 4:
        if (!infinite_world->get (x1, y1, z1+1)) { create_loose_portal (x1, y1, z1, x1, y1, z1+1); cnt--; }
	break;
      case 5:
        if (!infinite_world->get (x1, y1, z1-1)) { create_loose_portal (x1, y1, z1, x1, y1, z1-1); cnt--; }
	break;
    }
    max--;
  }
}

void InfPortalCS::CompleteSector ()
{
  extern WalkTest* Sys;
  InfiniteMaze* infinite_maze = Sys->infinite_maze;
  csSector* s = infinite_maze->create_six_room (Sys->engine, x2, y2, z2)->sector;
  infinite_maze->connect_infinite (x1, y1, z1, x2, y2, z2, false);
  SetSector (s);
  infinite_maze->random_loose_portals (x2, y2, z2);
  s->Prepare (s);
  s->InitLightMaps (false);
  s->ShineLights ();
  s->CreateLightMaps (System->G3D);
  while (lviews)
  {
    int old_draw_busy = s->draw_busy;
    s->draw_busy = 0;
    CheckFrustum (lviews->lv, 0);
    s->draw_busy = old_draw_busy;

    LV* n = lviews->next;
    delete lviews;
    lviews = n;
  }
  iPolygonMesh* mesh = QUERY_INTERFACE (s, iPolygonMesh);
  (void)new csCollider (*s, Sys->collide_system, mesh);
  mesh->DecRef ();
}

void InfPortalCS::CheckFrustum (csFrustumView& lview, int alpha)
{
  if (!GetSector ())
  {
    if (!lview.dynamic)
    {
      // If we want to shine light through this portal but it doesn't
      // really exist yet then we remember the csFrustumView for later.
      LV* lv = new LV ();
      lv->next = lviews;
      lviews = lv;
      lv->lv = lview;
      if (lview.light_frustum)
        lv->lv.light_frustum = new csFrustum (*lview.light_frustum);
    }
  }
  else
  {
    csPortal::CheckFrustum (lview, alpha);
  }
}

