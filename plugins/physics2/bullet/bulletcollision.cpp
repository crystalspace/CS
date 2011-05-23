#include "cssysdef.h"
#include "csgeom/sphere.h"
#include "csgeom/tri.h"
#include "imesh/genmesh.h"
#include "imesh/object.h"
#include "csutil/sysfunc.h"
#include "iutil/objreg.h"
#include "ivaria/view.h"
#include "ivaria/collision2.h"

#include "csutil/custom_new_disable.h"

// Bullet includes.
#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"
#include "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

#include "csutil/custom_new_enable.h"

#include "common.h"
#include "bulletcolliders.h"
#include "bulletcollision.h"

const float COLLISION_THRESHOLD = 0.01f;

CS_PLUGIN_NAMESPACE_BEGIN(Bullet2)
{

csBulletCollisionObject::csBulletCollisionObject (CollisionObjectType type, csBulletCollisionSystem* sys)
  : scfImplementationType (this), collSys (sys)
{
  collCb = NULL;
  btObject = NULL;
  sector = NULL;
  compoundShape = NULL;
  movable = NULL;
  insideWorld = false;
  compoundChanged = false;
  btTransform identity;
  identity.setIdentity ();
  motionState = new csBulletMotionState (this, identity, identity);
}

csBulletCollisionObject::~csBulletCollisionObject ()
{
  if (insideWorld)
    sector->bulletWorld->removeCollisionObject (btObject);
  if (btObject)
    delete btObject;
  delete compoundShape;
  delete motionState;
}

void csBulletCollisionObject::SetObjectType (CollisionObjectType type)
{
  if (type == COLLISION_OBJECT_ACTOR || type == COLLISION_OBJECT_TERRAIN)
  {
    csFPrintf  (stderr, "csBulletCollisionObject: Can't change from BASE/GHOST to ACTOR/TERRAIN\n");
    return;
  }
  else
    this->type = type;
}

void csBulletCollisionObject::SetTransform (const csOrthoTransform& trans)
{

  //Lulu: I don't understand why remove the body from the world then set MotionState,
  //      then add it back to the world. I didn't add this part to my code.
  //      So this code may be incorrect. I have to mark this.

  //TODO: Think about this and RebuildObject...
  transform = CSToBullet (trans, sector->internalScale);

  if (type == COLLISION_OBJECT_BASE)
  {
    btTransform principalAxis = motionState->inversePrincipalAxis.inverse ();

    delete motionState;
    motionState = new csBulletMotionState (this, transform * principalAxis, principalAxis);

    if (btObject)
      dynamic_cast<btRigidBody*>(btObject)->setMotionState (motionState);
  }
  else if (type == COLLISION_OBJECT_GHOST)
    if (btObject)
      btObject->setWorldTransform(transform);
}

csOrthoTransform csBulletCollisionObject::GetTransform ()
{
  float inverseScale;
  if (sector)
    inverseScale = sector->inverseInternalScale;
  else
    inverseScale = 1.0f;
  if (type == COLLISION_OBJECT_BASE)
  {
    btTransform trans;
    motionState->getWorldTransform (trans);
    return BulletToCS (trans * motionState->inversePrincipalAxis,
      inverseScale);
  }
  else if (type == COLLISION_OBJECT_GHOST)
    return BulletToCS (btObject->getWorldTransform(), sector->inverseInternalScale);
}

void csBulletCollisionObject::AddCollider (CS::Collision::iCollider* collider,
                                           const csOrthoTransform& relaTrans)
{
  csRef<csBulletCollider> coll (dynamic_cast<csBulletCollider*>(collider));

  colliders.Push (coll);
  relaTransforms.Push (relaTrans);
  //User must call RebuildObject() after this.
}

void csBulletCollisionObject::RemoveCollider (CS::Collision::iCollider* collider)
{
  for (size_t i =0; i < colliders.GetSize(); i++)
  {
    if (colliders[i] == collider)
    {
      colliders.DeleteIndex (i);
      relaTransforms.DeleteIndex (i);
      return;
    }
  }
  //User must call RebuildObject() after this.
}

void csBulletCollisionObject::RemoveCollider (size_t index)
{
  if (index < colliders.GetSize ())
  {  
    colliders.DeleteIndex (index);
    relaTransforms.DeleteIndex (index);
  }
  //User must call RebuildObject() after this.
}

CS::Collision::iCollider* csBulletCollisionObject::GetCollider (size_t index)
{
  if (index < colliders.GetSize ())
    return colliders[index];
  return NULL;
}

void csBulletCollisionObject::RebuildObject ()
{
  if (!sector)
  {
    csFPrintf  (stderr, "csBulletCollisionObject: Haven't add the object to a sector.\nRebuild failed.\n");
    return;
  }
  size_t colliderCount = colliders.GetSize ();
  if (colliderCount == 0)
  {  
    csFPrintf  (stderr, "csBulletCollisionObject: Haven't add any collider to the object.\nRebuild failed.\n");
    return;
  }

  if(compoundShape)
    delete compoundShape;
  //set the sector to collider. Then create btCollisionShape for collider.
  if(colliderCount == 1 && colliders[0]->shape == NULL)
  {
    colliders[0]->collSector = sector;
    colliders[0]->GenerateShape ();
  }
  else if(colliderCount >= 2)
  {  
    compoundShape = new btCompoundShape();
    for (size_t i = 0; i < colliderCount; i++)
    {
      colliders[i]->collSector = sector;
      if(colliders[i]->shape == NULL)
        colliders[i]->GenerateShape();

      btTransform relaTrans = CSToBullet (relaTransforms[i], sector->internalScale);
      compoundShape->addChildShape (relaTrans, colliders[i]->shape);
    }
    //Shift children shape?
  }

  if (btObject)
  {
    if (insideWorld)
      sector->bulletWorld->removeCollisionObject (btObject);

    delete btObject;
    btObject = NULL;
  }

  btVector3 localInertia (0.0f, 0.0f, 0.0f);
  btCollisionShape* shape;
  if (compoundShape == NULL)
  {
    //only one collider.
    shape = colliders[0]->shape;
  }
  else if (compoundChanged)
  {
    //use compound shape.
    shape = compoundShape;
  }

  if (type == COLLISION_OBJECT_BASE)
  {
    btRigidBody::btRigidBodyConstructionInfo infos (0.0, motionState,
      shape, localInertia);
    btObject = new btRigidBody (infos);
    btObject->setUserPointer (static_cast<iCollisionObject*> (this));
    sector->bulletWorld->addRigidBody (dynamic_cast<btRigidBody*>(btObject));
    insideWorld = true;
  }
  else if (type == COLLISION_OBJECT_GHOST)
  {
    btObject = new btPairCachingGhostObject ();
    btObject->setUserPointer (static_cast<iCollisionObject*> (this));
    sector->bulletWorld->addCollisionObject (btObject, 
      short(btBroadphaseProxy::DefaultFilter), 
      short(btBroadphaseProxy::AllFilter));
    btObject->setTransform (transform);
    insideWorld = true;
  }
}

void csBulletCollisionObject::SetCollisionGroup (const char* name)
{
  //TODO
  CollisionGroup& group = collSys->FindCollisionGroup (name);
}

//The actural collision detection should be moved to a function like Collider (btRigidBody*)....
//TODO: I need to think about this.
bool csBulletCollisionObject::Collide (iCollisionObject* otherObject)
{
  //Terrain VS others.
  if (otherObject->GetObjectType() == COLLISION_OBJECT_TERRAIN
    || otherObject->GetObjectType() == COLLISION_OBJECT_GHOST)
    return otherObject->Collide(this);

  //Ghost VS others except terrain.
  if (type == COLLISION_OBJECT_GHOST)
  {
    btGhostObject* ghost = btGhostObject::upcast(btObject);
    btAlignedObjectArray<btCollisionObject*>& overObjects = ghost->getOverlappingPairs();
    int index = overObjects.findLinearSearch(otherObject);
    if (index < overObjects.size ())
      return true;
    else
      return false;
  }
  else
  {
    //base VS base...Really need collision detection? These are static objects.
    return sector->RigidCollide(this, otherObject);
  }
  return false;
}

HitBeamResult csBulletCollisionObject::HitBeam (const csVector3& start,
                                                const csVector3& end)
{
  //TODO Call RayTestSingle or use callback.process ? 
  //The latter calls the former, but before that it will check the collision filter.
  //Collision system will not call this hitBeam function in it's own function.
  //btCollisionWorld::rayCast() is faster.
  return sector->RigidHitBeam(this, start, end);
}

csBulletTerrainObject::csBulletTerrainObject (csBulletCollisionSystem* sys)
:scfImplementationType (this), collSys (sys)
{
  collCb = NULL;
  sector = NULL;
  movable = NULL;
  terrainCollider = NULL;
  insideWorld = false;
  totalTransform.Identity();
}

csBulletTerrainObject::~csBulletTerrainObject ()
{
  if (insideWorld)
  {
    for (size_t i = 0; i < bodies.GetSize(); i++)
    {
      sector->bulletWorld->removeRigidBody (bodies[i]);
      delete bodies[i];
    }
  }
}

void csBulletTerrainObject::SetTransform (const csOrthoTransform& trans)
{
  //Allow user to set transform of terrain?
  totalTransform = trans;

  for(size_t i =0; i < transforms.GetSize(); i++)
  {
    csOrthoTransform cellTransform (totalTransform);
    csVector3 position = collider->GetCellPosition (i);
    cellTransform.SetOrigin (totalTransform.GetOrigin()
      + totalTransform.This2OtherRelative (position));
    transforms[i] = CSToBullet (cellTransform, sector->internalScale);
    bodies[i]->setTransform (tr);
  }
}

csOrthoTransform csBulletTerrainObject::GetTransform ()
{
  return totalTransform;
}

void csBulletTerrainObject::AddCollider (CS::Collision::iCollider* collider,
                                         const csOrthoTransform& relaTrans)
{
  if (collider->GetGeometryType() != COLLIDER_TERRAIN)
    return;
  else
    this->terrainCollider = dynamic_cast<csBulletColliderTerrain*> (collider);
}

void csBulletTerrainObject::RemoveCollider (CS::Collision::iCollider* collider)
{
  csRef<csBulletColliderTerrain> coll = dynamic_cast<csBulletColliderTerrain*>(collider);
  if (this->terrainCollider == coll)
    this->terrainCollider = NULL;
}

void csBulletTerrainObject::RemoveCollider (size_t index)
{
  if (index == 0)
    this->terrainCollider = NULL;
}

CS::Collision::iCollider* csBulletTerrainObject::GetCollider (size_t index)
{
  if (index == 1)
  {
    return dynamic_cast<iColliderTerrain*>((csBulletColliderTerrain*)terrainCollider);
  }
}

void csBulletTerrainObject::RebuildObject ()
{
  //TODO
  if (!sector)
  {
    csFPrintf  (stderr, "csBulletTerrainObject: Haven't add the object to a sector.\nRebuild failed.\n");
    return;
  }
  if (!terrainCollider)
  {  
    csFPrintf  (stderr, "csBulletTerrainObject: Haven't add any collider to the object.\nRebuild failed.\n");
    return;
  }

  //set the sector to collider. Then create btCollisionShape for collider.
  terrainCollider->collSector = sector;
  terrainCollider->GenerateShape ();

  if (insideWorld)
  {
    for (size_t i = 0; i < bodies.GetSize(); i++)
    {
      sector->bulletWorld->removeRigidBody (bodies[i]);
      delete bodies[i];
    }
    bodies.Empty ();
  }

  btVector3 localInertia (0.0f, 0.0f, 0.0f);

  for (size_t i = 0; i < terrainCollider->colliders.GetSize(); i++ )
  {
    btCollisionShape* shape = terrainCollider->colliders[i];
    btRigidBody body = new btRigidBody (0, 0, shape, localInertia);	
    body->setWorldTransform (transforms[i]);
    body->setUserPointer ((iColliderTerrain*)this);
    sector->bulletWorld->addRigidBody (body);
    bodies.Push (body);
  }
  insideWorld = true;
}

void csBulletTerrainObject::SetCollisionGroup (CollisionGroup group)
{
  //TODO
}

bool csBulletTerrainObject::Collide (iCollisionObject* otherObject)
{
  //TODO
  return false;
}

HitBeamResult csBulletTerrainObject::HitBeam (const csVector3& start, const csVector3& end)
{
  //TODO
  return HitBeamResult();
}

//SCF_IMPLEMENT_FACTORY(csBulletCollisionSystem)

}
CS_PLUGIN_NAMESPACE_END(Bullet2)