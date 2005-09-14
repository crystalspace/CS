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

#include "cstool/collider.h"
#include "csutil/flags.h"
#include "csutil/sparse3d.h"
#include "iengine/light.h"
#include "iengine/material.h"
#include "iengine/movable.h"
#include "iengine/sector.h"
#include "igeom/objmodel.h"
#include "igeom/polymesh.h"
#include "imesh/lighting.h"
#include "imesh/object.h"
#include "imesh/thing.h"

#include "infmaze.h"
#include "walktest.h"

InfiniteMaze::InfiniteMaze ()
{
  infinite_world = new csWideSparse3D ();
}

InfiniteMaze::~InfiniteMaze ()
{
  delete infinite_world;
}

void InfiniteMaze::create_one_side (iThingFactoryState* walls_state,
	char* pname,
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
  if (dx) { sy = sz = 0.9f; sx = 0; }
  else if (dy) { sx = sz = 0.9f; sy = 0; }
  else { sx = sy = 0.9f; sz = 0; }

  walls_state->AddQuad (
    csVector3 (cx+sx*x1, cy+sy*y1, cz+sz*z1),
    csVector3 (cx+sx*x2, cy+sy*y2, cz+sz*z2),
    csVector3 (cx+sx*x3, cy+sy*y3, cz+sz*z3),
    csVector3 (cx+sx*x4, cy+sy*y4, cz+sz*z4));
  walls_state->SetPolygonMaterial (CS_POLYRANGE_LAST, tm);
  walls_state->SetPolygonName (CS_POLYRANGE_LAST, pname);

  float nx1 = x1+dx, ny1 = y1+dy, nz1 = z1+dz;
  float nx2 = x2+dx, ny2 = y2+dy, nz2 = z2+dz;
  float nx3 = x3+dx, ny3 = y3+dy, nz3 = z3+dz;
  float nx4 = x4+dx, ny4 = y4+dy, nz4 = z4+dz;
  float ncx = cx+dx, ncy = cy+dy, ncz = cz+dz;

  int first = walls_state->AddQuad (
    csVector3 (cx+sx*x2, cy+sy*y2, cz+sz*z2),
    csVector3 (cx+sx*x1, cy+sy*y1, cz+sz*z1),
    csVector3 (ncx+sx*nx1, ncy+sy*ny1, ncz+sz*nz1),
    csVector3 (ncx+sx*nx2, ncy+sy*ny2, ncz+sz*nz2));

  walls_state->AddQuad (
    csVector3 (cx+sx*x3, cy+sy*y3, cz+sz*z3),
    csVector3 (cx+sx*x2, cy+sy*y2, cz+sz*z2),
    csVector3 (ncx+sx*nx2, ncy+sy*ny2, ncz+sz*nz2),
    csVector3 (ncx+sx*nx3, ncy+sy*ny3, ncz+sz*nz3));

  walls_state->AddQuad (
    csVector3 (cx+sx*x4, cy+sy*y4, cz+sz*z4),
    csVector3 (cx+sx*x3, cy+sy*y3, cz+sz*z3),
    csVector3 (ncx+sx*nx3, ncy+sy*ny3, ncz+sz*nz3),
    csVector3 (ncx+sx*nx4, ncy+sy*ny4, ncz+sz*nz4));

  int last = walls_state->AddQuad (
    csVector3 (cx+sx*x1, cy+sy*y1, cz+sz*z1),
    csVector3 (cx+sx*x4, cy+sy*y4, cz+sz*z4),
    csVector3 (ncx+sx*nx4, ncy+sy*ny4, ncz+sz*nz4),
    csVector3 (ncx+sx*nx1, ncy+sy*ny1, ncz+sz*nz1));

  walls_state->SetPolygonMaterial (CS_POLYRANGE (first, last), tm2);
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
  csString buf;
  buf.Format ("r%d_%d_%d", x, y, z);
  iSector* room = engine->CreateSector (buf);
  csRef<iMeshWrapper> walls (engine->CreateSectorWallsMesh (room, "walls"));
  walls->SetZBufMode (CS_ZBUF_USE);
  walls->SetRenderPriority (engine->GetObjectRenderPriority ());
  csRef<iThingState> walls_state (SCF_QUERY_INTERFACE (walls->GetMeshObject (),
  	iThingState));
  csRef<iThingFactoryState> walls_fact_state = walls_state->GetFactory ();
  float dx, dy, dz;
  dx = 2.0*(float)x;
  dy = 2.0*(float)y;
  dz = 2.0*(float)z;
  iMaterialWrapper* t = engine->GetMaterialList ()->FindByName ("txt");
  iMaterialWrapper* t2 = engine->GetMaterialList ()->FindByName ("txt2");
  float s = 1;

  create_one_side (walls_fact_state, "n", t, t2, dx-s,dy+s,dz+s,
  		dx+s,dy+s,dz+s, dx+s,dy-s,dz+s,  dx-s,dy-s,dz+s, 0,0,-0.1f);
  create_one_side (walls_fact_state, "e", t, t2, dx+s,dy+s,dz+s,
  		dx+s,dy+s,dz-s, dx+s,dy-s,dz-s,  dx+s,dy-s,dz+s, -0.1f,0,0);
  create_one_side (walls_fact_state, "w", t, t2, dx-s,dy+s,dz+s,
  		dx-s,dy-s,dz+s, dx-s,dy-s,dz-s,  dx-s,dy+s,dz-s, 0.1f,0,0);
  create_one_side (walls_fact_state, "s", t, t2, dx+s,dy+s,dz-s,
  		dx-s,dy+s,dz-s, dx-s,dy-s,dz-s,  dx+s,dy-s,dz-s, 0,0,0.1f);
  create_one_side (walls_fact_state, "f", t, t2, dx-s,dy-s,dz+s,
  		dx+s,dy-s,dz+s, dx+s,dy-s,dz-s,  dx-s,dy-s,dz-s, 0,0.1f,0);
  create_one_side (walls_fact_state, "c", t, t2, dx-s,dy+s,dz-s,
  		dx+s,dy+s,dz-s, dx+s,dy+s,dz+s,  dx-s,dy+s,dz+s, 0,-0.1f,0);

  csRef<iLight> light (engine->CreateLight ("",
  	csVector3 (dx+rand2 (.9*s), dy+rand2 (.9*s), dz+rand2 (.9*s)),
	1+rand1 (3),
  	csColor (rand1 (1), rand1 (1), rand1 (1))));
  room->GetLights ()->Add (light);

  InfRoomData* ird = new InfRoomData ();
  ird->x = x;
  ird->y = y;
  ird->z = z;
  ird->sector = room;
  ird->walls = walls;
  ird->walls_fact_state = walls_fact_state;
  infinite_world->Set (x, y, z, (void*)ird);
  WalkDataObject* irddata = new WalkDataObject (ird);
  room->QueryObject ()->ObjAdd (irddata);
  irddata->DecRef ();

  return ird;
}

void InfiniteMaze::connect_infinite (int x1, int y1, int z1, int x2, int y2,
	int z2, bool create_portal1)
{
  int a;
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

  int po1 = s1->walls_fact_state->FindPolygonByName (p1);
  int tmpCount = s1->walls_fact_state->GetPolygonVertexCount(po1);
  csVector3 *vertices = new csVector3[tmpCount];
  for(a=0;a<tmpCount;a++)
  {
    vertices[a] = s1->walls_fact_state->GetPolygonVertex(po1,a);
  }
  iPortal* portalBack;
  csRef<iMeshWrapper> portalMeshBack = Sys->Engine->CreatePortal (
    "new_portal_back", s1->sector, csVector3 (0),
    s2->sector, vertices, tmpCount, portalBack);
  delete[] vertices;

  int po2 = s2->walls_fact_state->FindPolygonByName (p2);
  tmpCount = s2->walls_fact_state->GetPolygonVertexCount(po2);
  vertices = new csVector3[tmpCount];
  for(a=0;a<tmpCount;a++)
  {
    vertices[a] = s2->walls_fact_state->GetPolygonVertex(po2,a);
  }
  iPortal* portalBack2;
  csRef<iMeshWrapper> portalMeshBack2 = Sys->Engine->CreatePortal (
    "new_portal_back2", s2->sector, csVector3 (0),
    s1->sector, vertices, tmpCount, portalBack2);
  portalBack->GetFlags ().Set (CS_PORTAL_ZFILL);
  portalBack->GetFlags ().Set (CS_PORTAL_CLIPDEST);
  portalBack2->GetFlags ().Set (CS_PORTAL_ZFILL);
  portalBack2->GetFlags ().Set (CS_PORTAL_CLIPDEST);
  s1->walls_fact_state->RemovePolygon(po1);
  s2->walls_fact_state->RemovePolygon(po2);
  delete[] vertices;
}

SCF_IMPLEMENT_IBASE (InfPortalCS)
  SCF_IMPLEMENTS_INTERFACE (iPortalCallback)
SCF_IMPLEMENT_IBASE_END

InfPortalCS::InfPortalCS ()
{
  SCF_CONSTRUCT_IBASE (0);
  lviews = 0;
}

InfPortalCS::~InfPortalCS ()
{
  SCF_DESTRUCT_IBASE ();
}

bool InfPortalCS::Traverse (iPortal* portal, iBase* context)
{
  csRef<iFrustumView> fv;
  if (context) fv = SCF_QUERY_INTERFACE (context, iFrustumView);
  if (fv)
  {
    iFrustumViewUserdata* ud = fv->GetUserdata ();
    csRef<iLightingProcessInfo> linfo (SCF_QUERY_INTERFACE (ud,
    	iLightingProcessInfo));
    if (linfo)
    {
      if (false && !linfo->IsDynamic ())
      {
        // If we want to shine light through this portal but it doesn't
        // really exist yet then we remember the csFrustumView for later.
        LV* lv = new LV ();
        lv->next = lviews;
        lviews = lv;
        lv->lv = fv;
        // Make a copy of the current context and remember it.
        lv->ctxt = fv->CopyFrustumContext ();
      }
    }
  }
  else
  {
printf ("Trav!\n"); fflush (stdout);
    extern WalkTest* Sys;
    InfiniteMaze* infinite_maze = Sys->infinite_maze;
    InfRoomData* ird = infinite_maze->create_six_room (Sys->Engine,
    	x2, y2, z2);
    iSector* is = ird->sector;
    //csSector* s = is->GetPrivateObject (); //@@@
    infinite_maze->connect_infinite (x1, y1, z1,
    	x2, y2, z2, false);
    portal->SetSector (is);
    infinite_maze->random_loose_portals (x2, y2, z2);

    int i;
    iMeshList* ml = is->GetMeshes ();
    for (i = 0 ; i < ml->GetCount () ; i++)
    {
      iMeshWrapper* mesh = ml->Get (i);
      csRef<iLightingInfo> linfo (SCF_QUERY_INTERFACE (mesh->GetMeshObject (),
      	iLightingInfo));
      if (linfo)
        linfo->InitializeDefault (true);
    }
    is->ShineLights ();
    for (i = 0 ; i < ml->GetCount () ; i++)
    {
      iMeshWrapper* mesh = ml->Get (i);
      csRef<iLightingInfo> linfo (SCF_QUERY_INTERFACE (mesh->GetMeshObject (),
      	iLightingInfo));
      if (linfo)
        linfo->PrepareLighting ();
    }

    while (lviews)
    {
      //int old_draw_busy = s->draw_busy;
      //s->draw_busy = 0;
      iFrustumView* fv = lviews->lv;
      csFrustumContext* orig_ctxt = fv->GetFrustumContext ();
      fv->SetFrustumContext (lviews->ctxt);
      portal->CheckFrustum (fv, ird->walls->GetMovable ()->GetTransform (), 0);
      fv->RestoreFrustumContext (orig_ctxt);
      //s->draw_busy = old_draw_busy;

      LV* n = lviews->next;
      delete lviews;
      lviews = n;
    }
    csRef<iPolygonMesh> mesh = 
      ird->walls->GetMeshObject ()->GetObjectModel()->GetPolygonMeshColldet();
    csColliderHelper::InitializeCollisionWrapper (Sys->collide_system, ird->walls);
    return true;
  }
  return false;
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
  //@@@@@@@@@@@@
  //InfRoomData* s = (InfRoomData*)(infinite_world->Get (x1, y1, z1));
  //iPolygon3DStatic* po = s->walls_fact_state->GetPolygon (p1);
  //iPortal* portal = po->CreateNullPortal ();
  InfPortalCS* prt = new InfPortalCS ();
  prt->x1 = x1; prt->y1 = y1; prt->z1 = z1;
  prt->x2 = x2; prt->y2 = y2; prt->z2 = z2;
  infinite_world->Set (x2, y2, z2, (void*)1);
  //portal->SetMissingSectorCallback (prt);
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

InfRoomData::~InfRoomData ()
{
}
