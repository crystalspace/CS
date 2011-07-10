#include "cssysdef.h"
#include "iengine/movable.h"
#include "collisionobject2.h"

CS_PLUGIN_NAMESPACE_BEGIN (Opcode2)
{
csOpcodeCollisionObject::csOpcodeCollisionObject (csOpcodeCollisionSystem* sys)
  :scfImplementationType (this), system (sys), collCb (NULL), sector (NULL),
  type (CS::Collision2::COLLISION_OBJECT_BASE)
{

}

csOpcodeCollisionObject::~csOpcodeCollisionObject ()
{

}

void csOpcodeCollisionObject::SetObjectType (CS::Collision2::CollisionObjectType type, bool forceRebuild)
{

}

void csOpcodeCollisionObject::SetTransform (const csOrthoTransform& trans)
{

}

csOrthoTransform csOpcodeCollisionObject::GetTransform ()
{
  return transform;
}

void csOpcodeCollisionObject::AddCollider (CS::Collision2::iCollider* collider, const csOrthoTransform& relaTrans)
{

}

void csOpcodeCollisionObject::RemoveCollider (CS::Collision2::iCollider* collider)
{

}

void csOpcodeCollisionObject::RemoveCollider (size_t index)
{

}

CS::Collision2::iCollider* csOpcodeCollisionObject::GetCollider (size_t index)
{
  return NULL;
}

size_t csOpcodeCollisionObject::GetColliderCount ()
{
  return 0;
}

void csOpcodeCollisionObject::RebuildObject ()
{}

void csOpcodeCollisionObject::SetCollisionGroup (const char* name)
{}

bool csOpcodeCollisionObject::Collide (iCollisionObject* otherObject){return false;}

CS::Collision2::HitBeamResult csOpcodeCollisionObject::HitBeam (const csVector3& start, const csVector3& end)
{
  CS::Collision2::HitBeamResult result;
  return result;
}

size_t csOpcodeCollisionObject::GetContactObjectsCount (){return 0;}

CS::Collision2::iCollisionObject* csOpcodeCollisionObject::GetContactObject (size_t index)
{
  return NULL;
}
}
CS_PLUGIN_NAMESPACE_END (Opcode2)