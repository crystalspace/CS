/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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
#include "version.h"
#include "qint.h"
#include "cssys/system.h"
#include "csgeom/frustum.h"
#include "csengine/dumper.h"
#include "csengine/campos.h"
#include "csengine/csview.h"
#include "csengine/stats.h"
#include "csengine/light.h"
#include "csengine/dynlight.h"
#include "csengine/texture.h"
#include "csengine/thing.h"
#include "csengine/wirefrm.h"
#include "csengine/polytext.h"
#include "csengine/polyset.h"
#include "csengine/polygon.h"
#include "csengine/pol2d.h"
#include "csengine/sector.h"
#include "csengine/world.h"
#include "csengine/keyval.h"
#include "csengine/collider.h"
#include "csengine/cspixmap.h"
#include "csengine/cssprite.h"
#include "csengine/rapid.h"
#include "csengine/terrain.h"
#include "csparser/impexp.h"
#include "csutil/inifile.h"
#include "csutil/csrect.h"
#include "csobject/dataobj.h"

extern WalkTest *Sys;

int FindIntersection(csCdTriangle *t1,csCdTriangle *t2,csVector3 line[2])
{
  csVector3 tri1[3]; tri1[0]=t1->p1; tri1[1]=t1->p2; tri1[2]=t1->p3;
  csVector3 tri2[3]; tri2[0]=t2->p1; tri2[1]=t2->p2; tri2[2]=t2->p3;

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
  csPolygon3D *p;
  plbody = new csPolygonSet (world);
  plbody->SetName ("Player's Body");

  plbody->AddVertex(-DX_2, OY,    -DZ_2);
  plbody->AddVertex(-DX_2, OY,    DZ_2);
  plbody->AddVertex(-DX_2, OY+DY, DZ_2);
  plbody->AddVertex(-DX_2, OY+DY, -DZ_2);
  plbody->AddVertex(DX_2,  OY,    -DZ_2);
  plbody->AddVertex(DX_2,  OY,    DZ_2);
  plbody->AddVertex(DX_2,  OY+DY, DZ_2);
  plbody->AddVertex(DX_2,  OY+DY, -DZ_2);

  // Left
  p = plbody->NewPolygon (0);

  p->AddVertex (0); p->AddVertex (1);
  p->AddVertex (2); p->AddVertex (3);

  // Right
  p = plbody->NewPolygon (0);
  p->AddVertex (4); p->AddVertex (5);
  p->AddVertex (6); p->AddVertex (7);

  // Bottom
  p = plbody->NewPolygon (0);
  p->AddVertex (0); p->AddVertex (1);
  p->AddVertex (5); p->AddVertex (4);

  // Top
  p = plbody->NewPolygon (0);
  p->AddVertex (3); p->AddVertex (2);
  p->AddVertex (6); p->AddVertex (7);

  // Front
  p = plbody->NewPolygon (0);
  p->AddVertex (1); p->AddVertex (5);
  p->AddVertex (6); p->AddVertex (2);

  // Back
  p = plbody->NewPolygon (0);
  p->AddVertex (0); p->AddVertex (4);
  p->AddVertex (7); p->AddVertex (3);

  body = new csRAPIDCollider (plbody);

  pllegs = new csPolygonSet (world);

  pllegs->AddVertex(-DX_2L, OYL,     -DZ_2L);
  pllegs->AddVertex(-DX_2L, OYL,     DZ_2L);
  pllegs->AddVertex(-DX_2L, OYL+DYL, DZ_2L);
  pllegs->AddVertex(-DX_2L, OYL+DYL, -DZ_2L);
  pllegs->AddVertex(DX_2L,  OYL,     -DZ_2L);
  pllegs->AddVertex(DX_2L,  OYL,     DZ_2L);
  pllegs->AddVertex(DX_2L,  OYL+DYL, DZ_2L);
  pllegs->AddVertex(DX_2L,  OYL+DYL, -DZ_2L);

  // Left
  p = pllegs->NewPolygon (0);

  p->AddVertex (0); p->AddVertex (1);
  p->AddVertex (2); p->AddVertex (3);

  // Right
  p = pllegs->NewPolygon (0);
  p->AddVertex (4); p->AddVertex (5);
  p->AddVertex (6); p->AddVertex (7);

  // Bottom
  p = pllegs->NewPolygon (0);
  p->AddVertex (0); p->AddVertex (1);
  p->AddVertex (5); p->AddVertex (4);

  // Top
  p = pllegs->NewPolygon (0);
  p->AddVertex (3); p->AddVertex (2);
  p->AddVertex (6); p->AddVertex (7);

  // Front
  p = pllegs->NewPolygon (0);
  p->AddVertex (1); p->AddVertex (5);
  p->AddVertex (6); p->AddVertex (2);

  // Back
  p = pllegs->NewPolygon (0);
  p->AddVertex (0); p->AddVertex (4);
  p->AddVertex (7); p->AddVertex (3);

  legs = new csRAPIDCollider(pllegs);

  if (!body || !legs)
    do_cd = false;
}

#define MAXSECTORSOCCUPIED  20

// No more than 1000 collisions ;)
collision_pair our_cd_contact[1000];
int num_our_cd;

int FindSectors (csVector3 v, csVector3 d, csSector *s, csSector **sa)
{
  sa[0] = s;
  int i, c = 1;
  float size = d.x * d.x + d.y * d.y + d.z * d.z;
  for(i = 0;i < s->GetNumPolygons() && c < MAXSECTORSOCCUPIED; i++)
  {
    // Get the polygon of for this sector.
    csPolygon3D* p = s->GetPolygon3D (i);
    csPortal* portal = p->GetPortal ();
    // Handle only portals.
    if (portal != NULL)
    {
      if (p->GetPlane ()->SquaredDistance (v) < size)
      {
        if (Sys->do_infinite && !portal->GetSector ())
	{
	  ((InfPortalCS*)portal)->CompleteSector ();
	}
        sa[c] = portal->GetSector ();
        c++;
      }
    }
  }
  return c;
}

int CollisionDetect (csRAPIDCollider *c, csSector* sp, csTransform *cdt)
{
  int hit = 0;

  // Check collision with this sector.
  csRAPIDCollider::CollideReset();
  if (c->Collide(*sp, cdt)) hit++;
  collision_pair *CD_contact = csRAPIDCollider::GetCollisions ();

  for (int i=0 ; i<csRAPIDCollider::numHits ; i++)
    our_cd_contact[num_our_cd++] = CD_contact[i];

  if (csRAPIDCollider::GetFirstHit() && hit)
    return 1;

  // Check collision with the things in this sector.
  csThing *tp = sp->GetFirstThing ();
  while (tp)
  {
    // TODO, if and when Things can move, their transform must be passed in.
    csRAPIDCollider::numHits = 0;
    if (c->Collide(*tp, cdt)) hit++;

    CD_contact = csRAPIDCollider::GetCollisions ();
    for (int i=0 ; i<csRAPIDCollider::numHits ; i++)
      our_cd_contact[num_our_cd++] = CD_contact[i];

    if (csRAPIDCollider::GetFirstHit() && hit)
      return 1;
    tp = (csThing*)(tp->GetNext ());
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
  int num_sectors = FindSectors (new_pos, 4.0f*Sys->body->GetRadius(),
    Sys->view->GetCamera()->GetSector(), n);

  num_our_cd = 0;
  csRAPIDCollider::SetFirstHit (false);
  int hits = 0;

  // Check to see if there are any terrains, if so test against those.
  // This routine will automatically adjust the transform to the highest
  // terrain at this point.
  int k;
  for ( k = 0; k < num_sectors ; k++)
  {
    if (n[k]->terrains.Length () > 0)
    {
      int i;
      for (i = 0 ; i < n[k]->terrains.Length () ; i++)
      {
	csTerrain* terrain = (csTerrain*)n[k]->terrains[i];
	hits += terrain->CollisionDetect (&test);
      }
    }
    if (hits)
    {
      new_pos = test.GetOrigin ();
    }
  }
  if (hits == 0)
  {
    csRAPIDCollider::CollideReset ();

    for ( ; num_sectors-- ; )
      hits += CollisionDetect (Sys->body, n[num_sectors], &test);

//printf ("body: hits=%d num_our_cd=%d\n", hits, num_our_cd);
    for (int j=0 ; j<num_our_cd ; j++)
    {
      csCdTriangle *wall = our_cd_contact[j].tr2;
      csVector3 n = ((wall->p3-wall->p2)%(wall->p2-wall->p1)).Unit();
      if (n*vel<0)
        continue;
      vel = -(vel%n)%n;
    }

    // We now know our (possible) velocity. Let's try to move up or down, if possible
    new_pos = pos+vel;
    test = csOrthoTransform (csMatrix3(), new_pos);

    num_sectors = FindSectors (new_pos, 4.0f*Sys->legs->GetRadius(), 
		Sys->view->GetCamera()->GetSector(), n);

    num_our_cd = 0;
    csRAPIDCollider::SetFirstHit (false);
    csRAPIDCollider::numHits = 0;
    int hit = 0;

    csRAPIDCollider::CollideReset ();

    for ( ; num_sectors-- ; )
      hit += CollisionDetect (Sys->legs, n[num_sectors], &test);
 
    if (!hit)
    {
      Sys->on_ground = false;
      if (Sys->do_gravity && !Sys->move_3d)
	vel.y -= 0.004;
    }
    else
    {
      float max_y=-1e10;
      
      for (int j=0 ; j<num_our_cd ; j++)
      {
	csCdTriangle first  = *our_cd_contact[j].tr1;
	csCdTriangle second = *our_cd_contact[j].tr2;

	csVector3 n=((second.p3-second.p2)%(second.p2-second.p1)).Unit ();

	if (n*csVector3(0,-1,0)<0.7) continue;

	csVector3 line[2];

	first.p1 += new_pos;
	first.p2 += new_pos;
	first.p3 += new_pos;

	if (FindIntersection (&first,&second,line))
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

