/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Alex Pfaffe

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

#include "sysdef.h"
#include "csengine/sysitf.h"
#include "csengine/being.h"
#include "csobject/nameobj.h"
#include "csengine/cdobj.h"

///
bool csBeing::init = false;
csBeing* csBeing::player = 0;
csWorld* csBeing::_world = 0;

csBeing::csBeing (  csPolygonSet *p, csSector* s, csTransform *cdt) : csCollider(p)
{
  sector = s;
  transform = cdt;
  _ground = 0;
  _sold = 0;
  _vold.Set (0, 0, 0);
}

csBeing::csBeing ( csSprite3D *sp, csSector* s, csTransform *cdt) : csCollider(sp)
{
  sector = s;
  transform = cdt;
  _ground = 0;
  _sold = 0;
  _vold.Set (0, 0, 0);
}

csBeing* csBeing::PlayerSpawn (char *name)
{
  // Define the player bounding box.
  // The camera's lens or person's eye is assumed to be
  // at 0,0,0.  The height (DY), width (DX) and depth (DZ).
  // Is the size of the camera/person and the origin
  // coordinates (OX,OY,OZ) locate the bbox w.r.t. the eye.
  // This player is 1.8 metres tall (assuming 1cs unit = 1m) (6feet)
  #define DX    0.5
  #define DY    1.8
  #define DZ    0.3
  #define OX   -0.25
  #define OY   -1.1
  #define OZ    0.0
  
  csPolygon3D *p;
  CHK (csPolygonSet* playerps = new csPolygonSet());
  csNameObject::AddName(*playerps, name);

  playerps->AddVertex(OX,    OY,    OZ); 
  playerps->AddVertex(OX,    OY,    OZ+DZ); 
  playerps->AddVertex(OX,    OY+DY, OZ+DZ); 
  playerps->AddVertex(OX,    OY+DY, OZ); 
  playerps->AddVertex(OX+DX, OY,    OZ); 
  playerps->AddVertex(OX+DX, OY,    OZ+DZ); 
  playerps->AddVertex(OX+DX, OY+DY, OZ+DZ); 
  playerps->AddVertex(OX+DX, OY+DY, OZ); 

  // Left
  p = playerps->NewPolygon (0);

  p->AddVertex (0); p->AddVertex (1); 
  p->AddVertex (2); p->AddVertex (3); 

  // Right
  p = playerps->NewPolygon (0);
  p->AddVertex (4); p->AddVertex (5); 
  p->AddVertex (6); p->AddVertex (7); 

  // Front
  p = playerps->NewPolygon (0);
  p->AddVertex (0); p->AddVertex (1); 
  p->AddVertex (5); p->AddVertex (4); 

  // Back
  p = playerps->NewPolygon (0);
  p->AddVertex (3); p->AddVertex (2); 
  p->AddVertex (6); p->AddVertex (7); 

  // Top
  p = playerps->NewPolygon (0);
  p->AddVertex (1); p->AddVertex (5); 
  p->AddVertex (6); p->AddVertex (2); 

  // Bottom
  p = playerps->NewPolygon (0);
  p->AddVertex (0); p->AddVertex (4); 
  p->AddVertex (7); p->AddVertex (3); 

  CHK (csBeing* b = new csBeing(playerps));

  return b;
}

int csBeing::InitWorld (csWorld* world, csCamera* /*camera*/)
{
  _world = world;
  CsPrintf (MSG_INITIALIZATION, "Computing OBBs ...\n");

  int sn = world->sectors.Length ();
  while (sn > 0)
  {
    sn--;
    csSector* sp = (csSector*)world->sectors[sn];
    // Initialize the sector itself.
    CHK(csCollider* pCollider = new csCollider(sp));
    csColliderPointerObject::SetCollider(*sp, pCollider, true);
    // Initialize the things in this sector.
    csThing* tp = sp->GetFirstThing ();
    while (tp)
    {
      CHK(csCollider* pCollider = new csCollider(tp));
      csColliderPointerObject::SetCollider(*tp, pCollider, true);
      tp = (csThing*)(tp->GetNext ());
    }
  }
  // Initialize all sprites for collision detection.
  csSprite3D* spp;
  int i;
  for (i = 0 ; i < world->sprites.Length () ; i++)
  {
    spp = (csSprite3D*)world->sprites[i];
    
    // TODO: Should create beings for these.
    CHK(csCollider* pCollider = new csCollider(spp));
    csColliderPointerObject::SetCollider(*spp, pCollider, true);
  }

  // Create a player object that follows the camera around.
  player = csBeing::PlayerSpawn("Player");

  init = true;
  CsPrintf (MSG_INITIALIZATION, "DONE\n");
  return 0; // Ok.
}

void csBeing::EndWorld(void)
{
  CHK (delete player);
}

int csBeing::_CollisionDetect (csSector* sp, csTransform *cdt)
{
  int hit = 0;

  // Check collision with this sector.
  hit += CollidePair (this,csColliderPointerObject::GetCollider(*sp),cdt);

  // Check collision of with the things in this sector.
  csThing* tp = sp->GetFirstThing ();
  while (tp)
  {
    // TODO, if and when Things can move, their transform must be passed in.
    hit += CollidePair(this,csColliderPointerObject::GetCollider(*tp),cdt);
    tp = (csThing*)(tp->GetNext ());
    // TODO, should test which one is the closest.
  }

  // Check collision of with the sprites in this sector.
  csSprite3D* sp3d = 0;
  csTransform cds;
  int i;
  for (i = 0 ; i < _world->sprites.Length () ; i++)
  {
    sp3d = (csSprite3D*)_world->sprites[i];
    cds.SetO2T (sp3d->GetW2T ());
    cds.SetOrigin (sp3d->GetW2TTranslation ());
    csCollider* pCollider = csColliderPointerObject::GetCollider(*sp3d);
    if (pCollider) hit+= CollidePair (this,pCollider,cdt,&cds);
  }

  return hit;
}

#define MAXSECTORSOCCUPIED  10      // csBeing can occupy no more than 10 sectors at one time.
// Find all sectors within distance d from point v.
// Currently only includes immediate neighbour sectors, should possibly
// recurse into 2ndary sectors as well.
int csBeing::FindSectors ( csVector3 v, csVector3 d, csSector *s, csSector **sa )
{
  sa[0] = s;
  int i, c = 1;
  float size = d.x * d.x + d.y * d.y + d.z * d.z;
  for (i = 0 ; i < s->GetNumPolygons () && c < MAXSECTORSOCCUPIED; i++)
  {
    // Get the polygon of for this sector.
    csPolygon3D* p = (csPolygon3D*) s->GetPolygon (i);
    csPortal* portal = p->GetPortal ();
    // Handle only portals.
    if (portal != NULL && portal->PortalType () == PORTAL_CS)
    {
      csPortalCS *pcs = (csPortalCS*)portal;
      if (p->GetPlane ()->SquaredDistance (v) < size)
      {
        sa[c] = pcs->GetSector ();
        c++;
      }
    }
  }
  return c;
}
// This is the main logic for collision detection of moving beings
// like players or walking monsters, for flying beings or projectiles
// a different function would need to be used, eg. one which does not
// care about gravity perhaps.
int csBeing::CollisionDetect( void )
{
  int hit = 0, cs;
  CollideReset();
        
  if (_sold == 0)
  {
    _mold = transform->GetO2T ();
    _vold = transform->GetO2TTranslation ();
    _sold = sector;
  }
        
  // See if we have entered a new sector.
  if (sector != _sold)
    _ground = 0;		// If so we do not know the ground object.
        

  // Find all sectors which our being occupies.
  csSector *all [MAXSECTORSOCCUPIED];
  int scount = 0;

  scount = FindSectors (transform->GetO2TTranslation (), GetBbox()->d, sector, all);

  // If we were to fall from our new position, which sector would our feet end up in?
  csTransform cdtnew (transform->GetO2T (), transform->GetO2TTranslation ());

  blocked = false;
  // See if we are on solid ground.
  // First test against the current ground if there is one.
  // If there isn't one, test the sector.
  cdtnew.SetOrigin (transform->GetO2TTranslation () + (0.2 * VEC_DOWN));

  // Test with the known ground object.
  hit = _ground && CollidePair (this, _ground, &cdtnew);
  cs = 0;
  while (!hit && (cs < scount))    // Test all nearby sectors.
  {
    // Test with everything in the sector.
    hit = _CollisionDetect (all [cs], &cdtnew);
    cs++;
  }

  if (hit)
  {
    falling = false;
    climbing = false;
                
    // We are on something solid, remember what the ground was.
    CDTriangle *tr1 = 0, *tr2 = 0;
    _ground = FindCollision(&tr1,&tr2);
    // TODO: Check tr2 and what its normal is w.r.t. gravity,
    // If the angle more than 45* we should slip/bounce.
                
    // printf("Ground %s\n",ground->get_name());
    // Perform forward collision of player with this sector.
    CollideReset();

    cs = 0;
    hit = 0;
    while (!hit && (cs < scount))    // Test all nearby sectors.
    {
      hit = _CollisionDetect (all[cs], transform);
      cs++;
    }
    // Hack for now assumes constant frame rate.
    if (hit) // Return to previous position.
    {
      // Get the object we hit.

      csCollider *cdhit = FindCollision();
                        
      // Try to see if we can climb up.
      csTransform cdtnew(transform->GetO2T (),transform->GetO2TTranslation ()+(0.2*VEC_UP));
                        
      // See if we still hit.  If so we are stuck, if not we may try to climb but
      // we need to retest the sector since we may still hit something else.
      cs = 0;
      hit = 0;
      while (!hit && (cs < scount))    // Test all nearby sectors.
      {
        hit = _CollisionDetect(all[cs], &cdtnew);
        cs++;
      }
      if (!CollidePair(this,cdhit, &cdtnew) && !hit)
      {
        CollideReset();
        climbing = true;
        transform->SetOrigin (cdtnew.GetO2TTranslation ());
      }
      else // Stay where we were, we definitly hit something.
      {
        climbing = false;
        blocked = true;
        transform->SetOrigin (_vold);
        transform->SetO2T (_mold);
        sector = _sold;
      }
    }
  }
  else // We did not find ground below us.
  {
    // Assume constant fall rate.

    // Patch from Ivan Avramovic to avoid the bug with falling through
    // the hole in large.zip.
    //transform->SetOrigin (transform->GetO2TTranslation () +(0.2*VEC_DOWN));
    bool mirror = false;
    csVector3 new_position = transform->GetOrigin () + (0.2*VEC_DOWN);
    csReversibleTransform t = *transform;
    sector = sector->FollowSegment (t, new_position, mirror);
    *transform = t;  transform->SetOrigin (new_position);

    //transform->SetO2T (_mold);
    _ground = 0;
    falling = true;
    climbing = false;
  }

  // Store these locations for next frame.
  _sold = sector;
  _mold = transform->GetO2T ();
  _vold = transform->GetO2TTranslation ();
  return hit;
}

