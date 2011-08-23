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

#ifndef __CS_OPCODE_COLLISIONOBJECT_H__
#define __CS_OPCODE_COLLISIONOBJECT_H__

#include "csOpcode2.h"
#include "colliders2.h"

CS_PLUGIN_NAMESPACE_BEGIN (Opcode2)
{
class csOpcodeCollisionSector;
class csOpcodeCollisionSystem;
class csOpcodeCollider;

class csOpcodeCollisionObject: public scfImplementationExt1<
  csOpcodeCollisionObject, csObject, CS::Collision2::iCollisionObject>
{
  friend class csOpcodeCollisionSector;
private:
  csOpcodeCollisionSystem* system;
  csOpcodeCollisionSector* sector;
  csWeakRef<iMovable> movable;
  csRef<CS::Collision2::iCollider> collider;
  CS::Collision2::CollisionObjectType type;
  CS::Collision2::CollisionGroup collGroup;
  csRef<CS::Collision2::iCollisionCallback> collCb;
  csRefArray<csOpcodeCollisionObject> contactObjects;
  csOrthoTransform transform;

  bool insideWorld;
  bool isTerrain;

public:
  csOpcodeCollisionObject (csOpcodeCollisionSystem* sys);
  virtual ~csOpcodeCollisionObject ();

  virtual iObject* QueryObject (void) { return (iObject*) this; }
  virtual CS::Collision2::iCollisionObject* QueryCollisionObject () {return dynamic_cast<iCollisionObject*> (this);}
  virtual CS::Physics2::iPhysicalBody* QueryPhysicalBody () {return NULL;}

  virtual void SetObjectType (CS::Collision2::CollisionObjectType type, bool forceRebuild = true);
  virtual CS::Collision2::CollisionObjectType GetObjectType () {return type;}

  virtual void SetAttachedMovable (iMovable* movable){this->movable = movable;}
  virtual iMovable* GetAttachedMovable (){return movable;}

  virtual void SetTransform (const csOrthoTransform& trans);
  virtual csOrthoTransform GetTransform ();

  virtual void AddCollider (CS::Collision2::iCollider* collider, const csOrthoTransform& relaTrans
    = csOrthoTransform (csMatrix3 (), csVector3 (0)));
  virtual void RemoveCollider (CS::Collision2::iCollider* collider);
  virtual void RemoveCollider (size_t index);

  virtual CS::Collision2::iCollider* GetCollider (size_t index);
  virtual size_t GetColliderCount ();

  virtual void RebuildObject ();

  virtual void SetCollisionGroup (const char* name);
  virtual const char* GetCollisionGroup () const {return collGroup.name.GetData ();}

  virtual void SetCollisionCallback (CS::Collision2::iCollisionCallback* cb) {collCb = cb;}
  virtual CS::Collision2::iCollisionCallback* GetCollisionCallback () {return collCb;}

  virtual bool Collide (iCollisionObject* otherObject);
  virtual CS::Collision2::HitBeamResult HitBeam (const csVector3& start, const csVector3& end);

  virtual size_t GetContactObjectsCount ();
  virtual CS::Collision2::iCollisionObject* GetContactObject (size_t index);
};
}
CS_PLUGIN_NAMESPACE_END (Opcode2)
#endif