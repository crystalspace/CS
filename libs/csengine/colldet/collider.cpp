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
#include "icollide.h"

csCollider::csCollider (csObject &parent)
{
  parent.ObjAdd(this);
}

IMPLEMENT_CSOBJTYPE (csCollider, csObject);

//----------------------------------------------------------------------

csPluginCollider::csPluginCollider (csObject& parent,
	iCollideSystem* collide_system,
	iPolygonMesh* mesh) : csCollider (parent)
{
  csPluginCollider::collide_system = collide_system;
  collide_system->IncRef ();
  collider = collide_system->CreateCollider (mesh);
}

csPluginCollider::~csPluginCollider ()
{
  collide_system->DecRef ();
  collider->DecRef ();
}

bool csPluginCollider::Collide (csObject &otherObject,
                               csTransform *pThisTransform,
                               csTransform *pOtherTransform) 
{
  csPluginCollider *pOtherCollider = GetPluginCollider (otherObject);
  if (pOtherCollider)
    return Collide (*pOtherCollider, pThisTransform, pOtherTransform);
  else
    return false;
}

bool csPluginCollider::Collide (csCollider &otherCollider, 
                               csTransform *pTransform1, 
                               csTransform *pTransform2)
{
  if (otherCollider.GetType() != csPluginCollider::Type) return false;
  csPluginCollider *pCollider2 = (csPluginCollider *)&otherCollider;
  if (pCollider2 == this) return false;
  // Skip inactive combinations.
  if (!m_CollisionDetectionActive || 
      !pCollider2->m_CollisionDetectionActive) return 0;

  int num_hits;
  return collide_system->Collide (collider, pTransform1,
  	pCollider2->collider, pTransform2, num_hits) != NULL;
}

csPluginCollider *csPluginCollider::GetPluginCollider (csObject &object) 
{
  csObject *o = object.GetChild (csPluginCollider::Type);
  if (o) return (csPluginCollider*) o;
  return NULL;
}

IMPLEMENT_CSOBJTYPE (csPluginCollider, csCollider);

