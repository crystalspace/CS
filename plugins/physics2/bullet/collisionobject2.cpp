#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

#include "cssysdef.h"
#include "iengine/movable.h"
#include "collisionobject2.h"
#include "colliders2.h"

CS_PLUGIN_NAMESPACE_BEGIN (Bullet2)
{

csBulletCollisionObject::csBulletCollisionObject (csBulletSystem* sys)
: scfImplementationType (this), system (sys), collCb (NULL), btObject (NULL),
sector (NULL), compoundShape (NULL), movable (NULL), insideWorld (false),
shapeChanged (false), isTerrain (false), type (COLLISION_OBJECT_BASE)
{
  btTransform identity;
  identity.setIdentity ();
  motionState = new csBulletMotionState (this, identity, identity);
}

csBulletCollisionObject::~csBulletCollisionObject ()
{
  RemoveBulletObject ();
  colliders.DeleteAll ();
  if (btObject)
    delete btObject;
  if (compoundShape)
    delete compoundShape;
  if (motionState)
    delete motionState;
}

void csBulletCollisionObject::SetObjectType (CollisionObjectType type)
{
  //many many constraints.
  if (type == COLLISION_OBJECT_ACTOR)
  {
    csFPrintf  (stderr, "csBulletCollisionObject: Can't change from BASE/GHOST to ACTOR\n");
    return;
  }
  else if (isTerrain && type != COLLISION_OBJECT_BASE)
  {
    csFPrintf  (stderr, "csBulletCollisionObject: Can't change terrain object from BASE to other type\n");
    return;
  }
  else
    this->type = type; // this means you can change from rigidbody/softbody to ghostobject. I dont know if the code can do this.
}

void csBulletCollisionObject::SetTransform (const csOrthoTransform& trans)
{

  //Lulu: I don't understand why remove the body from the world then set MotionState,
  //      then add it back to the world. I didn't add this part to my code.
  //      So this code may be incorrect. I have to mark this.

  //TODO: Think about this and RebuildObject...
  transform = CSToBullet (trans, system->getInternalScale ());

  if (type == COLLISION_OBJECT_BASE || type == COLLISION_OBJECT_PHYSICAL)
  {
    if (!isTerrain)
    {
      if (insideWorld)
        sector->bulletWorld->removeRigidBody (btRigidBody::upcast (btObject));

      btTransform principalAxis = motionState->inversePrincipalAxis.inverse ();

      delete motionState;
      motionState = new csBulletMotionState (this, transform * principalAxis, principalAxis);

      if (btObject)
        dynamic_cast<btRigidBody*>(btObject)->setMotionState (motionState);

      if (insideWorld)
        sector->bulletWorld->addRigidBody (btRigidBody::upcast (btObject));
    }
    else
    {
      //Do not support change transform of terrain?
      //Currently in CS it's not supported.
    }
  }
  else if (type == COLLISION_OBJECT_GHOST)
    if (btObject)
      btObject->setWorldTransform(transform);
}

csOrthoTransform csBulletCollisionObject::GetTransform ()
{
  float inverseScale = system->getInverseInternalScale ();

  if (type == COLLISION_OBJECT_BASE || type == COLLISION_OBJECT_PHYSICAL)
  {
    if (!isTerrain)
    {
      btTransform trans;
      motionState->getWorldTransform (trans);
      return BulletToCS (trans * motionState->inversePrincipalAxis,
        inverseScale);
    }
    else
    {
      csBulletColliderTerrain* terrainCollider = dynamic_cast<csBulletColliderTerrain*> (colliders[0]);
      return terrainCollider->terrainTransform;      
    }
  }
  else if (type == COLLISION_OBJECT_GHOST)
    return BulletToCS (btObject->getWorldTransform(), system->getInverseInternalScale ());
}

void csBulletCollisionObject::AddCollider (CS::Collision2::iCollider* collider,
                                           const csOrthoTransform& relaTrans)
{
  csRef<csBulletCollider> coll (dynamic_cast<csBulletCollider*>(collider));

  if(coll->GetGeometryType () == COLLIDER_TERRAIN)
  {
    colliders.Empty ();
    relaTransforms.Empty ();
    colliders.Push (coll);
    relaTransforms.Push (relaTrans);
    isTerrain = true;
  }
  else if (! isTerrain)
  {
    // If a collision object has a terrain collider. Then it is not allowed to add other colliders.
    colliders.Push (coll);
    relaTransforms.Push (relaTrans);
  }
  shapeChanged = true;
  //User must call RebuildObject() after this.
}

void csBulletCollisionObject::RemoveCollider (CS::Collision2::iCollider* collider)
{
  for (size_t i =0; i < colliders.GetSize(); i++)
  {
    if (colliders[i] == collider)
    {
      RemoveCollider (i);
      return;
    }
  }
  //User must call RebuildObject() after this.
}

void csBulletCollisionObject::RemoveCollider (size_t index)
{
  if (index < colliders.GetSize ())
  {  
    if (isTerrain && index == 0)
      isTerrain = false;
    colliders.DeleteIndex (index);
    relaTransforms.DeleteIndex (index);
  }
  //User must call RebuildObject() after this.
}

CS::Collision2::iCollider* csBulletCollisionObject::GetCollider (size_t index)
{
  if (index < colliders.GetSize ())
    return colliders[index];
  return NULL;
}

void csBulletCollisionObject::RebuildObject ()
{
  size_t colliderCount = colliders.GetSize ();
  if (colliderCount == 0)
  {  
    csFPrintf (stderr, "csBulletCollisionObject: Haven't add any collider to the object.\nRebuild failed.\n");
    return;
  }

  if (shapeChanged)
  {
    if(compoundShape)
      delete compoundShape;

    if (isTerrain)
    {
      csBulletColliderTerrain* terrainColl = dynamic_cast<csBulletColliderTerrain*> (colliders[0]);
      for (size_t i = 0; i < terrainColl->bodies.GetSize (); i++)
      {
        btRigidBody* body = terrainColl->GetBulletObject (i);
        body->setUserPointer (this);
      }
    }

    else if(colliderCount >= 2)
    {  
      compoundShape = new btCompoundShape();
      for (size_t i = 0; i < colliderCount; i++)
      {
        btTransform relaTrans = CSToBullet (relaTransforms[i], system->getInternalScale ());
        compoundShape->addChildShape (relaTrans, colliders[i]->shape);
      }
      //Shift children shape?
    }
  }

  bool wasInWorld = false;
  if (insideWorld)
  {
    wasInWorld = true;
    RemoveBulletObject ();
  }

  if (wasInWorld)
    AddBulletObject ();
}

void csBulletCollisionObject::SetCollisionGroup (const char* name)
{
  //TODO
  CollisionGroup& group = system->FindCollisionGroup (name);
}

bool csBulletCollisionObject::Collide (iCollisionObject* otherObject)
{
  //Ghost VS no matter what kind.
  if (type == COLLISION_OBJECT_GHOST)
  {
    btGhostObject* ghost = btGhostObject::upcast (btObject);
    btAlignedObjectArray<btCollisionObject*>& overObjects = ghost->getOverlappingPairs ();
    for (int i = 0; i < overObjects.size (); i++)
    {
      if (overObjects[i]->getUserPointer () == dynamic_cast<void*> (otherObject))
        return true;
    }
    return false;
  }

  //no matter what kind VS Ghost, Actor.
  if (otherObject->GetObjectType () != COLLISION_OBJECT_BASE 
    || otherObject->GetObjectType () != COLLISION_OBJECT_PHYSICAL)
    return otherObject->Collide (this);

  csBulletCollisionObject* otherObj = dynamic_cast<csBulletCollisionObject*> (otherObject);
  if (isTerrain)
  {
    //Terrain VS Terrain???
    if (otherObj->isTerrain == true)
      return false;

    //Terrain VS object/Body.
    btCollisionObject* otherBtObject = otherObj->GetBulletCollisionPointer ();
    csBulletColliderTerrain* terrainColl = dynamic_cast<csBulletColliderTerrain*> (colliders[0]);
    for (size_t i = 0; i < terrainColl->bodies.GetSize (); i++)
    {
      btRigidBody* body = terrainColl->GetBulletObject (i);
      return sector->BulletCollide (body, otherBtObject);
    }
  }
  else
  {
    //Object/Body CS terrain.
    if (otherObj->isTerrain == true)
      return otherObject->Collide (this);

    //Object/Body VS object.
    btCollisionObject* otherBtObject = dynamic_cast<csBulletCollisionObject*> (otherObject)->GetBulletCollisionPointer ();
    return sector->BulletCollide (btObject, otherBtObject);
  }
}

HitBeamResult csBulletCollisionObject::HitBeam (const csVector3& start,
                                                const csVector3& end)
{
  //Terrain part
  if (isTerrain)
  {
    csBulletColliderTerrain* terrainColl = dynamic_cast<csBulletColliderTerrain*> (colliders[0]);
    for (size_t i = 0; i < terrainColl->bodies.GetSize (); i++)
    {
      btRigidBody* body = terrainColl->GetBulletObject (i);
      HitBeamResult result = sector->RigidHitBeam (body, start, end);
      if (result.hasHit)
        return result;
    }
    return HitBeamResult();
  }
  //Others part
  else
    return sector->RigidHitBeam (btObject, start, end);
}

void csBulletCollisionObject::RemoveBulletObject ()
{
  if (insideWorld)
  {
    if (isTerrain)
    {
      csBulletColliderTerrain* terrainColl = dynamic_cast<csBulletColliderTerrain*> (colliders[0]);
      terrainColl->RemoveRigidBodies ();
    }
    else
    {
      sector->bulletWorld->removeCollisionObject (btObject);
      delete btObject;
      btObject = NULL;
    }
    insideWorld = false;
  }
}

void csBulletCollisionObject::AddBulletObject ()
{
  //Add to world in this function...
  if (insideWorld)
    RemoveBulletObject ();
  if (type == COLLISION_OBJECT_BASE)
  {
    if (!isTerrain)
    {
      btCollisionShape* shape;
      if (compoundShape)
        shape = compoundShape;
      else
        shape = colliders[0]->shape;

      btTransform trans;
      motionState->getWorldTransform (trans);
      trans = trans * motionState->inversePrincipalAxis;
      delete motionState;
      btTransform pricipalAxis = CSToBullet (relaTransforms[0], system->getInternalScale ());
      motionState = new csBulletMotionState (this, trans * pricipalAxis, pricipalAxis);

      btVector3 localInertia (0.0f, 0.0f, 0.0f);
      btRigidBody::btRigidBodyConstructionInfo infos (0.0, motionState,
        shape, localInertia);
      btObject = new btRigidBody (infos);
      btObject->setUserPointer (dynamic_cast<iCollisionObject*> (this));
      sector->bulletWorld->addRigidBody (btRigidBody::upcast(btObject));
    }
    else
      dynamic_cast<csBulletColliderTerrain*> (colliders[0])->AddRigidBodies (sector, this);
  }
  else if (type == COLLISION_OBJECT_GHOST)
  {
    btObject = new btPairCachingGhostObject ();
    btObject->setWorldTransform (transform);
    btObject->setUserPointer (static_cast<iCollisionObject*> (this));
    sector->bulletWorld->addCollisionObject (btObject, 
      short(btBroadphaseProxy::DefaultFilter), 
      short(btBroadphaseProxy::AllFilter));
  }
  insideWorld = true;
}
}
CS_PLUGIN_NAMESPACE_END (Bullet2)