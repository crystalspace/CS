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
#include "imesh/object.h"
#include "igeom/polymesh.h"
#include "igeom/objmodel.h"

//----------------------------------------------------------------------

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
	float stepsize,
	csVector3& newpos,
	int num_colliders,
	iCollider** colliders,
	csReversibleTransform** transforms)
{
  csReversibleTransform test = *trans;
  csVector3 start = test.GetOrigin ();
  csVector3 end = newpos;
  csVector3 testpos;
  float step = 1. / stepsize;
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

