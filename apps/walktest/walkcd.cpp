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
#include "walktest/walktest.h"
#include "walktest/infmaze.h"
#include "qint.h"
#include "cssys/system.h"
#include "csgeom/frustum.h"
#include "csengine/dumper.h"
#include "csengine/campos.h"
#include "csengine/csview.h"
#include "csengine/stats.h"
#include "csengine/light.h"
#include "csengine/texture.h"
#include "csengine/thing.h"
#include "csengine/wirefrm.h"
#include "csengine/polytext.h"
#include "csengine/polygon.h"
#include "csengine/pol2d.h"
#include "csengine/sector.h"
#include "csengine/engine.h"
#include "csengine/keyval.h"
#include "csengine/collider.h"
#include "csengine/meshobj.h"
#include "csengine/terrobj.h"
#include "csparser/impexp.h"
#include "csgeom/csrect.h"
#include "csobject/dataobj.h"
#include "csfx/cspixmap.h"
#include "iengine/collider.h"

extern WalkTest *Sys;

int FindIntersection(csCollisionPair& cd,csVector3 line[2])
{
  csVector3 tri1[3]; tri1[0]=cd.a1; tri1[1]=cd.b1; tri1[2]=cd.c1;
  csVector3 tri2[3]; tri2[0]=cd.a2; tri2[1]=cd.b2; tri2[2]=cd.c2;

  return csMath3::FindIntersection(tri1,tri2,line);
}

// Define the player bounding box.
// The camera's lens or person's eye is assumed to be
// at 0,0,0.  The height (DY), width (DX) and depth (DZ).
// Is the size of the camera/person and the origin
// coordinates (OX,OY,OZ) locate the bbox with respect to the eye.
// This player is 1.8 metres tall (assuming 1cs unit = 1m) (6feet)
#define DX    cfg_body_width
#define DY    cfg_body_height
#define DZ    cfg_body_depth
#define OY    Sys->cfg_eye_offset

#define DX_L  cfg_legs_width
#define DZ_L  cfg_legs_depth

#define DX_2  (DX/2)
#define DZ_2  (DZ/2)

#define DX_2L (DX_L/2)
#define DZ_2L (DZ_L/2)

#define OYL  Sys->cfg_legs_offset
#define DYL  (OY-OYL)

void WalkTest::CreateColliders ()
{
  iPolygon3D *p;
  iPolygonMesh* mesh;
  iMeshObjectFactory* thing_fact = engine->GetThingType ()->NewFactory ();
  iMeshObject* mesh_obj = QUERY_INTERFACE (thing_fact, iMeshObject);
  thing_fact->DecRef ();
  plbody = new csMeshWrapper (engine, mesh_obj);
  mesh_obj->DecRef ();
  plbody->SetName ("Player's Body");
  iThingState* thing_state = QUERY_INTERFACE (mesh_obj, iThingState);

  thing_state->CreateVertex (csVector3 (-DX_2, OY,    -DZ_2));
  thing_state->CreateVertex (csVector3 (-DX_2, OY,    DZ_2));
  thing_state->CreateVertex (csVector3 (-DX_2, OY+DY, DZ_2));
  thing_state->CreateVertex (csVector3 (-DX_2, OY+DY, -DZ_2));
  thing_state->CreateVertex (csVector3 (DX_2,  OY,    -DZ_2));
  thing_state->CreateVertex (csVector3 (DX_2,  OY,    DZ_2));
  thing_state->CreateVertex (csVector3 (DX_2,  OY+DY, DZ_2));
  thing_state->CreateVertex (csVector3 (DX_2,  OY+DY, -DZ_2));

  // Left
  p = thing_state->CreatePolygon ();
  p->CreateVertex (0); p->CreateVertex (1);
  p->CreateVertex (2); p->CreateVertex (3);

  // Right
  p = thing_state->CreatePolygon ();
  p->CreateVertex (4); p->CreateVertex (5);
  p->CreateVertex (6); p->CreateVertex (7);

  // Bottom
  p = thing_state->CreatePolygon ();
  p->CreateVertex (0); p->CreateVertex (1);
  p->CreateVertex (5); p->CreateVertex (4);

  // Top
  p = thing_state->CreatePolygon ();
  p->CreateVertex (3); p->CreateVertex (2);
  p->CreateVertex (6); p->CreateVertex (7);

  // Front
  p = thing_state->CreatePolygon ();
  p->CreateVertex (1); p->CreateVertex (5);
  p->CreateVertex (6); p->CreateVertex (2);

  // Back
  p = thing_state->CreatePolygon ();
  p->CreateVertex (0); p->CreateVertex (4);
  p->CreateVertex (7); p->CreateVertex (3);

  mesh = QUERY_INTERFACE (mesh_obj, iPolygonMesh);
  body = new csCollider (*plbody, collide_system, mesh);
  body_radius = plbody->GetRadius ();
  mesh->DecRef ();
  thing_state->DecRef ();

  thing_fact = engine->GetThingType ()-> NewFactory ();
  mesh_obj = QUERY_INTERFACE (thing_fact, iMeshObject);
  thing_fact->DecRef ();
  pllegs = new csMeshWrapper (engine, mesh_obj);
  mesh_obj->DecRef ();
  pllegs->SetName ("Player's Legs");
  thing_state = QUERY_INTERFACE (mesh_obj, iThingState);

  thing_state->CreateVertex (csVector3 (-DX_2L, OYL,     -DZ_2L));
  thing_state->CreateVertex (csVector3 (-DX_2L, OYL,     DZ_2L));
  thing_state->CreateVertex (csVector3 (-DX_2L, OYL+DYL, DZ_2L));
  thing_state->CreateVertex (csVector3 (-DX_2L, OYL+DYL, -DZ_2L));
  thing_state->CreateVertex (csVector3 (DX_2L,  OYL,     -DZ_2L));
  thing_state->CreateVertex (csVector3 (DX_2L,  OYL,     DZ_2L));
  thing_state->CreateVertex (csVector3 (DX_2L,  OYL+DYL, DZ_2L));
  thing_state->CreateVertex (csVector3 (DX_2L,  OYL+DYL, -DZ_2L));

  // Left
  p = thing_state->CreatePolygon ();
  p->CreateVertex (0); p->CreateVertex (1);
  p->CreateVertex (2); p->CreateVertex (3);

  // Right
  p = thing_state->CreatePolygon ();
  p->CreateVertex (4); p->CreateVertex (5);
  p->CreateVertex (6); p->CreateVertex (7);

  // Bottom
  p = thing_state->CreatePolygon ();
  p->CreateVertex (0); p->CreateVertex (1);
  p->CreateVertex (5); p->CreateVertex (4);

  // Top
  p = thing_state->CreatePolygon ();
  p->CreateVertex (3); p->CreateVertex (2);
  p->CreateVertex (6); p->CreateVertex (7);

  // Front
  p = thing_state->CreatePolygon ();
  p->CreateVertex (1); p->CreateVertex (5);
  p->CreateVertex (6); p->CreateVertex (2);

  // Back
  p = thing_state->CreatePolygon ();
  p->CreateVertex (0); p->CreateVertex (4);
  p->CreateVertex (7); p->CreateVertex (3);

  mesh = QUERY_INTERFACE (mesh_obj, iPolygonMesh);
  legs = new csCollider (*pllegs, collide_system, mesh);
  legs_radius = pllegs->GetRadius ();
  mesh->DecRef ();
  thing_state->DecRef ();

  if (!body || !legs)
    do_cd = false;
}

#define MAXSECTORSOCCUPIED  20

// No more than 1000 collisions ;)
csCollisionPair our_cd_contact[1000];
int num_our_cd;

int FindSectors (csVector3 v, csVector3 d, csSector *s, csSector **sa)
{
  int c = 0;
  // @@@ Avoid this sqrt somehow? i.e. by having it in the objects.
  float size = sqrt (d.x * d.x + d.y * d.y + d.z * d.z);
  csSectorIt* it = Sys->engine->GetNearbySectors (s, v, size);
  csSector* sector;
  while ((sector = it->Fetch ()) != NULL)
  {
    sa[c++] = sector;
    if (c >= MAXSECTORSOCCUPIED) break;
  }
  delete it;
  return c;
}

int CollisionDetect (csCollider *c, csSector* sp, csTransform *cdt)
{
  int hit = 0;
  int i;

  // Check collision with this sector.
  Sys->collide_system->ResetCollisionPairs ();
  if (c->Collide (*sp, cdt)) hit++;
  csCollisionPair* CD_contact = Sys->collide_system->GetCollisionPairs ();

  for (i=0 ; i<Sys->collide_system->GetNumCollisionPairs () ; i++)
    our_cd_contact[num_our_cd++] = CD_contact[i];

  if (Sys->collide_system->GetOneHitOnly () && hit)
    return 1;

  // Check collision with the meshes in this sector.
  for (i = 0 ; i < sp->GetNumberMeshes () ; i++)
  {
    csMeshWrapper* tp = sp->GetMesh (i);
    Sys->collide_system->ResetCollisionPairs ();
    if (c->Collide (*tp, cdt, &tp->GetMovable ().GetTransform ())) hit++;

    CD_contact = Sys->collide_system->GetCollisionPairs ();
    for (int j=0 ; j<Sys->collide_system->GetNumCollisionPairs () ; j++)
      our_cd_contact[num_our_cd++] = CD_contact[j];

    if (Sys->collide_system->GetOneHitOnly () && hit)
      return 1;
    // TODO, should test which one is the closest.
  }

  return hit;
}

void DoGravity (csVector3& pos, csVector3& vel)
{
  pos=Sys->view->GetCamera ()->GetOrigin ();

  csVector3 new_pos = pos+vel;
  csMatrix3 m;
  csOrthoTransform test (m, new_pos);

  csSector *n[MAXSECTORSOCCUPIED];
  int num_sectors = FindSectors (new_pos, 4.0f*Sys->body_radius,
    Sys->view->GetCamera()->GetSector(), n);

  num_our_cd = 0;
  Sys->collide_system->SetOneHitOnly (false);
  int hits = 0;

  // Check to see if there are any terrains, if so test against those.
  // This routine will automatically adjust the transform to the highest
  // terrain at this point.
  int k;
  for ( k = 0; k < num_sectors ; k++)
  {
    if (n[k]->GetNumberTerrains () > 0)
    {
      int i;
      for (i = 0 ; i < n[k]->GetNumberTerrains () ; i++)
      {
	      csTerrainWrapper* terrain = n[k]->GetTerrain( i );
	      hits += terrain->CollisionDetect( &test );
      }
    }
  }

  // If there were hits with the terrain we update our new position
  // here. Side note: this could moved outside the loop above because
  // a compiler bug with gcc 2.7.2 prevented it from working when inside
  // the loop.
  if (hits) new_pos = test.GetOrigin ();

  if (hits == 0)
  {
    Sys->collide_system->ResetCollisionPairs ();

    for ( ; num_sectors-- ; )
      hits += CollisionDetect (Sys->body, n[num_sectors], &test);

//printf ("body: hits=%d num_our_cd=%d\n", hits, num_our_cd);
    for (int j=0 ; j<num_our_cd ; j++)
    {
      csCollisionPair& cd = our_cd_contact[j];
      csVector3 n = ((cd.c2-cd.b2)%(cd.b2-cd.a2)).Unit();
      if (n*vel<0)
        continue;
      vel = -(vel%n)%n;
    }

    // We now know our (possible) velocity. Let's try to move up or down, if possible
    new_pos = pos+vel;
    test = csOrthoTransform (csMatrix3(), new_pos);

    num_sectors = FindSectors (new_pos, 4.0f*Sys->legs_radius, 
		Sys->view->GetCamera()->GetSector(), n);

    num_our_cd = 0;
    Sys->collide_system->SetOneHitOnly (false);
    Sys->collide_system->ResetCollisionPairs ();
    int hit = 0;

    for ( ; num_sectors-- ; )
      hit += CollisionDetect (Sys->legs, n[num_sectors], &test);
 
    if (!hit)
    {
      Sys->on_ground = false;
      if (Sys->do_gravity && !Sys->move_3d)
	vel.y -= 0.002;
    }
    else
    {
      float max_y=-1e10;
      
      for (int j=0 ; j<num_our_cd ; j++)
      {
	csCollisionPair cd = our_cd_contact[j];
        csVector3 n = ((cd.c2-cd.b2)%(cd.b2-cd.a2)).Unit();

	if (n*csVector3(0,-1,0)<0.7) continue;

	csVector3 line[2];

	cd.a1 += new_pos;
	cd.b1 += new_pos;
	cd.c1 += new_pos;

	if (FindIntersection (cd,line))
	{
	  if (line[0].y>max_y)
	    max_y=line[0].y;
	  if (line[1].y>max_y)
	    max_y=line[1].y;
	}
      }

      float p = new_pos.y-max_y+OYL+0.01;
      if (ABS(p)<DYL-0.01)
      {
	if (max_y != -1e10)
	  new_pos.y = max_y-OYL-0.01;

	if (vel.y<0)
	  vel.y = 0;
      }
      Sys->on_ground = true;
    }
  }
  new_pos -= Sys->view->GetCamera ()->GetOrigin ();
  Sys->view->GetCamera ()->MoveWorld (new_pos);
  Sys->velocity = Sys->view->GetCamera ()->GetO2T ()*vel;

  if(!Sys->do_gravity)
    Sys->velocity.y -= SIGN (Sys->velocity.y) * MIN (0.017, ABS (Sys->velocity.y));
}

