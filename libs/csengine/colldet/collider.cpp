/*
    Copyright (C) 2000 by Jorrit Tyberghein
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
#include "csengine/collider.h"
#include "iengine/collider.h"
#include "csengine/engine.h"

//----------------------------------------------------------------------

IMPLEMENT_IBASE_EXT (csCollider)
  IMPLEMENTS_INTERFACE (csCollider)
IMPLEMENT_IBASE_EXT_END

csCollider::csCollider (csObject& parent,
	iCollideSystem* collide_system,
	iPolygonMesh* mesh)
{
  parent.ObjAdd (this);
  csCollider::collide_system = collide_system;
  collide_system->IncRef ();
  collider = collide_system->CreateCollider (mesh);
}

csCollider::csCollider (iObject* parent,
	iCollideSystem* collide_system,
	iPolygonMesh* mesh)
{
  iObject* ithis = QUERY_INTERFACE (this, iObject);
  parent->ObjAdd (ithis);
  ithis->DecRef ();
  csCollider::collide_system = collide_system;
  collide_system->IncRef ();
  collider = collide_system->CreateCollider (mesh);
}

csCollider::~csCollider ()
{
  collide_system->DecRef ();
  collider->DecRef ();
}

bool csCollider::Collide (csObject& otherObject,
                          csTransform* pThisTransform,
                          csTransform* pOtherTransform) 
{
  csCollider *pOtherCollider = GetCollider (otherObject);
  if (pOtherCollider)
    return Collide (*pOtherCollider, pThisTransform, pOtherTransform);
  else
    return false;
}

bool csCollider::Collide (iObject* otherObject,
                          csTransform* pThisTransform,
                          csTransform* pOtherTransform) 
{
  csCollider *pOtherCollider = GetCollider (otherObject);
  if (pOtherCollider)
    return Collide (*pOtherCollider, pThisTransform, pOtherTransform);
  else
    return false;
}

bool csCollider::Collide (csCollider& otherCollider, 
                          csTransform* pTransform1, 
                          csTransform* pTransform2)
{
  csCollider *pCollider2 = &otherCollider;
  if (pCollider2 == this) return false;

  return collide_system->Collide (collider, pTransform1,
  	pCollider2->collider, pTransform2);
}

csCollider* csCollider::GetCollider (csObject &object) 
{
  return GET_CHILD_OBJECT_FAST (&object, csCollider);
}

csCollider* csCollider::GetCollider (iObject* object) 
{
  return GET_CHILD_OBJECT_FAST (object, csCollider);
}

