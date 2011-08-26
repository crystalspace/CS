/*
  Copyright (C) 2011 by Liu Lu

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "iengine/movable.h"
#include "collisionobject2.h"

CS_PLUGIN_NAMESPACE_BEGIN (Opcode2)
{
csOpcodeCollisionObject::csOpcodeCollisionObject (csOpcodeCollisionSystem* sys)
  :scfImplementationType (this), system (sys), collCb (NULL), sector (NULL),
  type (CS::Collision2::COLLISION_OBJECT_BASE), collider (NULL), isTerrain (false)
{
  transform.Identity ();
}

csOpcodeCollisionObject::~csOpcodeCollisionObject ()
{

}

void csOpcodeCollisionObject::SetObjectType (CS::Collision2::CollisionObjectType type, bool forceRebuild)
{
}

void csOpcodeCollisionObject::SetTransform (const csOrthoTransform& trans)
{
  transform = trans;
  if (movable)
    movable->SetFullTransform (transform);
}

csOrthoTransform csOpcodeCollisionObject::GetTransform ()
{
  if (movable)
    return movable->GetFullTransform ();
  return transform;
}

void csOpcodeCollisionObject::AddCollider (CS::Collision2::iCollider* collider, const csOrthoTransform& relaTrans)
{
  this->collider = collider;
  if (collider->GetGeometryType () == CS::Collision2::COLLIDER_TERRAIN)
    isTerrain = true;
}

void csOpcodeCollisionObject::RemoveCollider (CS::Collision2::iCollider* collider)
{
  //Please delete the collision object or set another collider. Don't use this function.
}

void csOpcodeCollisionObject::RemoveCollider (size_t index)
{
  //Please delete the collision object or set another collider. Don't use this function.
}

CS::Collision2::iCollider* csOpcodeCollisionObject::GetCollider (size_t index)
{
  return collider;
}

size_t csOpcodeCollisionObject::GetColliderCount ()
{
  if (collider == NULL)
    return 0;
  else
    return 1;
}

void csOpcodeCollisionObject::RebuildObject ()
{
  if (collider == NULL)
  {  
    csFPrintf (stderr, "csBulletCollisionObject: Haven't add any collider to the object.\nRebuild failed.\n");
    return;
  }
}

void csOpcodeCollisionObject::SetCollisionGroup (const char* name)
{
  if (!sector)
    return;

  CS::Collision2::CollisionGroup& group = sector->FindCollisionGroup (name);
  this->collGroup = group;
}

bool csOpcodeCollisionObject::Collide (CS::Collision2::iCollisionObject* otherObject)
{
  csOpcodeCollisionObject* obj = dynamic_cast<csOpcodeCollisionObject*> (otherObject);
  bool collides = (obj->collGroup.value & collGroup.mask) != 0;
  collides = collides && (collGroup.value & obj->collGroup.mask);

  if(!collides)
    return false;

  csArray<CS::Collision2::CollisionData> collisions;

  if (obj->isTerrain && isTerrain)
    return false;
  else if (obj->isTerrain || isTerrain)
  {
    bool contact = sector->CollideTerrain (this, obj, collisions);
    if (contact)
      if (collCb)
        if (obj->isTerrain)
          collCb->OnCollision (this,obj,collisions);
        else
          collCb->OnCollision (obj,this,collisions);
    return contact;
  }
  else
  {
    bool contact = sector->CollideObject (this, obj, collisions);
    if (contact)
      if (collCb)
        collCb->OnCollision (this, obj, collisions);
    return contact;
  }
}

CS::Collision2::HitBeamResult csOpcodeCollisionObject::HitBeam (const csVector3& start, const csVector3& end)
{
  CS::Collision2::iCollider* col = collider;
  float dep;
  CS::Collision2::HitBeamResult result;
  if (isTerrain)
    result = sector->HitBeamTerrain (this, start, end, dep);
  else
    result = sector->HitBeamObject (this, start, end, dep);

  return result;
}

//There's no step function now. Maybe use ContactTest is better.
size_t csOpcodeCollisionObject::GetContactObjectsCount (){return 0;}

CS::Collision2::iCollisionObject* csOpcodeCollisionObject::GetContactObject (size_t index)
{
  return NULL;
}
}
CS_PLUGIN_NAMESPACE_END (Opcode2)