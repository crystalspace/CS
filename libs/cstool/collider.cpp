/*
    Copyright (C) 2000-2003 by Jorrit Tyberghein
    Written by Daniel Gudbjartsson

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
#include "ivaria/collider.h"
#include "iengine/engine.h"
#include "iengine/region.h"
#include "iengine/mesh.h"
#include "iengine/sector.h"
#include "iengine/viscull.h"
#include "iengine/movable.h"
#include "iengine/portalcontainer.h"
#include "iengine/portal.h"
#include "imesh/object.h"
#include "igeom/polymesh.h"
#include "igeom/objmodel.h"
#include "csqsqrt.h"

//----------------------------------------------------------------------

CS_LEAKGUARD_IMPLEMENT (csColliderWrapper);

SCF_IMPLEMENT_IBASE_EXT (csColliderWrapper)
  SCF_IMPLEMENTS_INTERFACE (csColliderWrapper)
SCF_IMPLEMENT_IBASE_EXT_END

csColliderWrapper::csColliderWrapper (csObject& parent,
	iCollideSystem* collide_system,
	iPolygonMesh* mesh)
{
  parent.ObjAdd (this);
  csColliderWrapper::collide_system = collide_system;
  collider = collide_system->CreateCollider (mesh);
}

csColliderWrapper::csColliderWrapper (iObject* parent,
	iCollideSystem* collide_system,
	iPolygonMesh* mesh)
{
  parent->ObjAdd (this);
  csColliderWrapper::collide_system = collide_system;
  collider = collide_system->CreateCollider (mesh);
}

csColliderWrapper::csColliderWrapper (iObject* parent,
	iCollideSystem* collide_system,
	iCollider* collider)
{
  parent->ObjAdd (this);
  csColliderWrapper::collide_system = collide_system;
  csColliderWrapper::collider = collider;
}

csColliderWrapper::~csColliderWrapper ()
{
}

bool csColliderWrapper::Collide (csObject& otherObject,
                          csReversibleTransform* pThisTransform,
                          csReversibleTransform* pOtherTransform)
{
  csColliderWrapper *pOtherCollider = GetColliderWrapper (otherObject);
  if (pOtherCollider)
    return Collide (*pOtherCollider, pThisTransform, pOtherTransform);
  else
    return false;
}

bool csColliderWrapper::Collide (iObject* otherObject,
                          csReversibleTransform* pThisTransform,
                          csReversibleTransform* pOtherTransform)
{
  csColliderWrapper *pOtherCollider = GetColliderWrapper (otherObject);
  if (pOtherCollider)
    return Collide (*pOtherCollider, pThisTransform, pOtherTransform);
  else
    return false;
}

bool csColliderWrapper::Collide (csColliderWrapper& otherCollider,
                          csReversibleTransform* pTransform1,
                          csReversibleTransform* pTransform2)
{
  if (!collider) return false;
  csColliderWrapper *pCollider2 = &otherCollider;
  if (pCollider2 == this) return false;

  return collide_system->Collide (collider, pTransform1,
  	pCollider2->collider, pTransform2);
}

csColliderWrapper* csColliderWrapper::GetColliderWrapper (csObject &object)
{
  csRef<csColliderWrapper> w (CS_GET_CHILD_OBJECT (&object, csColliderWrapper));
  return w;	// This will DecRef() but that's ok in this case.
}

csColliderWrapper* csColliderWrapper::GetColliderWrapper (iObject* object)
{
  csRef<csColliderWrapper> w (CS_GET_CHILD_OBJECT (object, csColliderWrapper));
  return w;	// This will DecRef() but that's ok in this case.
}

//----------------------------------------------------------------------


void csColliderHelper::InitializeCollisionWrapper (iCollideSystem* colsys,
	iMeshWrapper* mesh)
{
  iObjectModel* obj_objmodel = mesh->GetMeshObject ()->GetObjectModel ();
  iPolygonMesh* obj_polymesh = obj_objmodel->GetPolygonMeshColldet ();

  iMeshFactoryWrapper* factory = mesh->GetFactory ();
  if (factory)
  {
    iObjectModel* fact_objmodel = factory->GetMeshObjectFactory ()
    	->GetObjectModel ();
    if (fact_objmodel)
    {
      iPolygonMesh* fact_polymesh = fact_objmodel->GetPolygonMeshColldet ();
      if (fact_polymesh && (fact_polymesh == obj_polymesh || !obj_polymesh))
      {
        // First check if the parent factory has a collider wrapper.
	iCollider* collider;
        csColliderWrapper* cw_fact = csColliderWrapper::GetColliderWrapper (
		factory->QueryObject ());
        if (cw_fact)
	{
	  collider = cw_fact->GetCollider ();
	}
	else
	{
	  csColliderWrapper *cw_fact = new csColliderWrapper (
	  	factory->QueryObject (), colsys, fact_polymesh);
	  cw_fact->SetName (factory->QueryObject ()->GetName());
	  collider = cw_fact->GetCollider ();
	  cw_fact->DecRef ();
	}

	// Now add the collider wrapper to the mesh. We need a new
	// csColliderWrapper because the csObject system is strictly
	// a tree and one csColliderWrapper cannot have multiple parents.
	csColliderWrapper *cw = new csColliderWrapper (mesh->QueryObject (),
	  colsys, collider);
	cw->SetName (mesh->QueryObject ()->GetName());
	cw->DecRef ();
        return;
      }
    }
  }

  if (obj_polymesh)
  {
    csColliderWrapper *cw = new csColliderWrapper (mesh->QueryObject (),
	colsys, obj_polymesh);
    cw->SetName (mesh->QueryObject ()->GetName());
    cw->DecRef ();
  }

  iMeshList* ml = mesh->GetChildren ();
  int i;
  for (i = 0 ; i < ml->GetCount () ; i++)
  {
    iMeshWrapper* child = ml->Get (i);
    InitializeCollisionWrapper (colsys, child);
  }
}

void csColliderHelper::InitializeCollisionWrappers (iCollideSystem* colsys,
  	iEngine* engine, iRegion* region)
{
  // Initialize all mesh objects for collision detection.
  int i;
  iMeshList* meshes = engine->GetMeshes ();
  for (i = 0 ; i < meshes->GetCount () ; i++)
  {
    iMeshWrapper* sp = meshes->Get (i);
    if (region && !region->IsInRegion (sp->QueryObject ())) continue;
    InitializeCollisionWrapper (colsys, sp);
  }
}

bool csColliderHelper::CollideArray (
	iCollideSystem* colsys,
	iCollider* collider,
  	const csReversibleTransform* trans,
  	int num_colliders,
	iCollider** colliders,
	csReversibleTransform **transforms)
{
  int i;
  for (i = 0 ; i < num_colliders ; i++)
  {
    bool rc = colsys->Collide (collider, trans,
    	colliders[i], transforms[i]);
    if (rc) return rc;
  }
  return false;
}

int csColliderHelper::CollidePath (
	iCollideSystem* colsys,
	iCollider* collider,
  	const csReversibleTransform* trans,
	float nbrsteps,
	csVector3& newpos,
	int num_colliders,
	iCollider** colliders,
	csReversibleTransform** transforms)
{
  csReversibleTransform test = *trans;
  csVector3 start = test.GetOrigin ();
  csVector3 end = newpos;
  csVector3 testpos;
  float step = 1.0f / nbrsteps;
  float curdist = 0;
  bool rc = false;
  bool firsthit = true;
  for (;;)
  {
    testpos = start+curdist * (end-start);
    test.SetOrigin (testpos);
    colsys->ResetCollisionPairs ();
    rc = CollideArray (colsys, collider, &test,
    	num_colliders, colliders, transforms);
    if (rc) break;
    firsthit = false;

    if (curdist >= 1) break;
    curdist += step;
    if (curdist > 1) curdist = 1;
  }

  if (rc)
  {
    // We had a collision.
    if (firsthit)
    {
      // The collision happened on the start point. In that case
      // we cannot move at all. Return -1.
      return -1;
    }

    // Here we try to find more exactly where the collision occured by
    // doing a binary search.
    end = testpos;
    while (csSquaredDist::PointPoint (start, end) > .05)
    {
      testpos = (start+end) / 2;
      test.SetOrigin (testpos);
      colsys->ResetCollisionPairs ();
      rc = CollideArray (colsys, collider, &test,
      	num_colliders, colliders, transforms);
      if (rc)
      {
        // Use left segment.
        end = testpos;
      }
      else
      {
        // Use right segment.
	start = testpos;
      }
    }
    // We know that the object can move to the 'start' position safely
    // because of the way we handle the binary search and the starting
    // condition that firsthit == false.
    newpos = start;

    // But first we set the collision detection array to the position
    // which resulted in collision.
    test.SetOrigin (end);
    colsys->ResetCollisionPairs ();
    CollideArray (colsys, collider, &test,
    	num_colliders, colliders, transforms);

    return 0;
  }
  else
  {
    // There was no collision.
    return 1;
  }
}

float csColliderHelper::TraceBeam (iCollideSystem* cdsys, iSector* sector,
	const csVector3& start, const csVector3& end,
	bool traverse_portals,
	csIntersectingTriangle& closest_tri,
	csVector3& closest_isect,
	iMeshWrapper** closest_mesh)
{
  if (!sector)
  {
    if (closest_mesh) *closest_mesh = 0;
    return -1.0f;
  }
  iVisibilityCuller* culler = sector->GetVisibilityCuller ();
  csRef<iVisibilityObjectIterator> it = culler->IntersectSegmentSloppy (
  	start, end);

  // We loop over all objects that intersect with the beam. For every
  // such object we will use CollideRay() to find colliding triangles. We
  // will look for the colliding triangle closest to 'start'.
  float best_squared_dist = 10000000000.0;
  csSegment3 seg (start, end);
  iMeshWrapper* best_mesh = 0;
  bool have_hit = false;
  // This will be set to the hit portal index if the best polygon we
  // hit so far is actually a portal.
  int last_portal_index = -1;
 
  while (it->HasNext ())
  {
    iVisibilityObject* visobj = it->Next ();
    iMeshWrapper* mesh = visobj->GetMeshWrapper ();
    csColliderWrapper* colwrap = csColliderWrapper::GetColliderWrapper (
    	mesh->QueryObject ());
    if (colwrap)
    {
      iMovable* movable = mesh->GetMovable ();
      csReversibleTransform trans = movable->GetFullTransform ();
      if (cdsys->CollideRay (colwrap->GetCollider (), &trans, start, end))
      {
        // This ray hits the mesh.
	const csArray<csIntersectingTriangle>& tris = cdsys->
		GetIntersectingTriangles ();
	size_t i;
	for (i = 0 ; i < tris.Length () ; i++)
	{
	  csVector3 isect;
	  csIntersectingTriangle tri;
	  if (movable->IsFullTransformIdentity ())
	  {
	    tri = tris[i];
	  }
	  else
	  {
	    tri.a = trans.This2Other (tris[i].a);
	    tri.b = trans.This2Other (tris[i].b);
	    tri.c = trans.This2Other (tris[i].c);
	  }
	  // The function below should always return true but you never know
	  // due to numerical inaccuracies.
	  if (csIntersect3::SegmentTriangle (seg, tri.a, tri.b, tri.c,
	  	isect))
	  {
	    float squared_dist = csSquaredDist::PointPoint (isect, start);
	    if (squared_dist < best_squared_dist)
	    {
	      have_hit = true;
	      best_squared_dist = squared_dist;
	      closest_tri = tri;
	      closest_isect = isect;
	      // This is not a portal we want to traverse because it
	      // has a collider which means it is solid.
	      last_portal_index = -1;
	      best_mesh = mesh;
	    }
	  }
	}
      }
    }
    if (mesh->GetPortalContainer () && traverse_portals)
    {
      // We have a portal container and we want to specifically traverse
      // portals. In that case we also have to trace a beam to whatever
      // portal we might hit.
      iMovable* movable = mesh->GetMovable ();
      csReversibleTransform trans = movable->GetFullTransform ();
      csVector3 obj_start, obj_end;
      if (movable->IsFullTransformIdentity ())
      {
        obj_start = start;
        obj_end = end;
      }
      else
      {
        obj_start = trans.Other2This (start);
        obj_end = trans.Other2This (end);
      }
      csVector3 obj_isect;
      int polygon_idx;
      if (mesh->GetMeshObject ()->HitBeamObject (obj_start, obj_end,
      		obj_isect, 0, &polygon_idx))
      {
        if (!movable->IsFullTransformIdentity ())
	  obj_isect = trans.This2Other (obj_isect);
	float squared_dist = csSquaredDist::PointPoint (obj_isect, start);
	if (squared_dist < best_squared_dist)
	{
	  have_hit = true;
	  best_squared_dist = squared_dist;
	  closest_isect = obj_isect;
	  last_portal_index = polygon_idx;
	  best_mesh = mesh;
	}
      }
    }
  }

  // If our best hit is a portal then we must traverse it here.
  if (last_portal_index != -1)
  {
    iPortal* portal = best_mesh->GetPortalContainer ()->GetPortal (
    	last_portal_index);
    // First we calculate a new beam starting in the new sector from
    // the intersection point. We make sure the new start is slightly
    // beyond the intersection point to avoid numerical inaccuracies.
    // We take a point 0.1% on the way from 'closest_isect' to 'end'.
    csVector3 new_start = closest_isect + 0.001 * (end-closest_isect);
    csVector3 new_end = end;

    // Now we have to consider space warping for the portal.
    if (portal->GetFlags ().Check (CS_PORTAL_WARP))
    {
      iMovable* movable = best_mesh->GetMovable ();
      csReversibleTransform trans = movable->GetFullTransform ();

      csReversibleTransform warp_wor;
      portal->ObjectToWorld (trans, warp_wor);
      new_start = portal->Warp (warp_wor, new_start);
      new_end = portal->Warp (warp_wor, new_end);
    }

    // Recurse with the new beam to the new sector.
    float new_squared_dist = TraceBeam (cdsys, portal->GetSector (),
	new_start, new_end, traverse_portals,
	closest_tri, closest_isect,
	closest_mesh);
    if (new_squared_dist >= 0)
    {
      // We have a hit. We have to add the distance so far to the
      // new distance.
      float new_dist = csQsqrt (best_squared_dist) + csQsqrt (new_squared_dist);
      return new_dist * new_dist;
    }
    return -1.0f;
  }

  if (closest_mesh) *closest_mesh = best_mesh;
  if (have_hit)
    return best_squared_dist;
  else
    return -1.0f;
}

