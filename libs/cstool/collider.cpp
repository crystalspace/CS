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


void csColliderHelper::InitializeCollisionWrappers (iCollideSystem* colsys,
  	iEngine* engine, iRegion* region)
{
  csRef<iPolygonMesh> mesh;
  // Initialize all mesh objects for collision detection.
  int i;
  iMeshList* meshes = engine->GetMeshes ();
  for (i = 0 ; i < meshes->GetCount () ; i++)
  {
    iMeshWrapper* sp = meshes->Get (i);
    if (region && !region->IsInRegion (sp->QueryObject ())) continue;
    mesh = SCF_QUERY_INTERFACE (sp->GetMeshObject (), iPolygonMesh);
    if (mesh)
    {
      csColliderWrapper *cw = new csColliderWrapper (sp->QueryObject (),
	colsys, mesh);
      cw->SetName (sp->QueryObject ()->GetName());
      cw->DecRef ();
    }
  }
}

