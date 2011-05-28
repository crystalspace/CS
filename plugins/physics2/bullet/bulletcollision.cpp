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
  isPhysics = false;
  isTerrain = false;
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
    if (!isTerrain)
    {
      btTransform principalAxis = motionState->inversePrincipalAxis.inverse ();

      delete motionState;
      motionState = new csBulletMotionState (this, transform * principalAxis, principalAxis);

      if (btObject)
        dynamic_cast<btRigidBody*>(btObject)->setMotionState (motionState);
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
  float inverseScale;
  if (sector)
    inverseScale = sector->inverseInternalScale;
  else
    inverseScale = 1.0f;
  if (type == COLLISION_OBJECT_BASE)
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
    return BulletToCS (btObject->getWorldTransform(), sector->inverseInternalScale);
}

void csBulletCollisionObject::AddCollider (CS::Collision::iCollider* collider,
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
    // kickvb: why would it fail?
    // Lulu: because the sector->internalScale is unknown... The parameter is needed to build btCollisionShape.
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

    //set user pointer.
    if (isTerrain)
    {
      csBulletColliderTerrain* terrainColl = dynamic_cast<csBulletColliderTerrain*> (colliders[0]);
      for (size_t i = 0; i < terrainColl->bodies.GetSize (); i++)
      {
        btRigidBody* body = terrainColl->GetBulletObject (i);
        body->setUserPointer (this);
      }
    }
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

  //Add to world in this function...
  if (type == COLLISION_OBJECT_BASE && !isTerrain)
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
  if (otherObject->GetObjectType () != COLLISION_OBJECT_BASE)
    return otherObject->Collide (this);

  csBulletCollisionObject* otherObj = dynamic_cast<csBulletCollisionObject*> (otherObject);
  if (isTerrain)
  {
    //Terrain VS Terrain???
    if (otherObj->isTerrain == true)
      return false;

    //Terrain VS object.
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
    //Object CS terrain.
    if (otherObj->isTerrain == ture)
      return otherObject->Collide (this);

    //Object VS object.
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
      for (size_t i = 0; i < terrainColl->bodies.GetSize (); i++)
      {
        btRigidBody* body = terrainColl->GetBulletObject (i);
        sector->bulletWorld->removeRigidObject (body);
      }
    }
    else
        sector->bulletWorld->removeCollisionObject (btObject);
  }
}

// TODO..
//csBulletTerrainObject::csBulletTerrainObject (csBulletCollisionSystem* sys)
//:scfImplementationType (this), collSys (sys)
//{
//  collCb = NULL;
//  sector = NULL;
//  movable = NULL;
//  terrainCollider = NULL;
//  insideWorld = false;
//  totalTransform.Identity();
//}
//
//csBulletTerrainObject::~csBulletTerrainObject ()
//{
//  if (insideWorld)
//  {
//    for (size_t i = 0; i < bodies.GetSize(); i++)
//    {
//      sector->bulletWorld->removeRigidBody (bodies[i]);
//      delete bodies[i];
//    }
//  }
//}
//
//void csBulletTerrainObject::SetTransform (const csOrthoTransform& trans)
//{
//  //Allow user to set transform of terrain?
//  totalTransform = trans;
//
//  for(size_t i =0; i < transforms.GetSize(); i++)
//  {
//    csOrthoTransform cellTransform (totalTransform);
//    csVector3 position = collider->GetCellPosition (i);
//    cellTransform.SetOrigin (totalTransform.GetOrigin()
//      + totalTransform.This2OtherRelative (position));
//    transforms[i] = CSToBullet (cellTransform, sector->internalScale);
//    bodies[i]->setTransform (tr);
//  }
//}
//
//csOrthoTransform csBulletTerrainObject::GetTransform ()
//{
//  return totalTransform;
//}
//
//void csBulletTerrainObject::AddCollider (CS::Collision::iCollider* collider,
//                                         const csOrthoTransform& relaTrans)
//{
//  if (collider->GetGeometryType() != COLLIDER_TERRAIN)
//    return;
//  else
//    this->terrainCollider = dynamic_cast<csBulletColliderTerrain*> (collider);
//}
//
//void csBulletTerrainObject::RemoveCollider (CS::Collision::iCollider* collider)
//{
//  csRef<csBulletColliderTerrain> coll = dynamic_cast<csBulletColliderTerrain*>(collider);
//  if (this->terrainCollider == coll)
//    this->terrainCollider = NULL;
//}
//
//void csBulletTerrainObject::RemoveCollider (size_t index)
//{
//  if (index == 0)
//    this->terrainCollider = NULL;
//}
//
//CS::Collision::iCollider* csBulletTerrainObject::GetCollider (size_t index)
//{
//  if (index == 1)
//  {
//    return dynamic_cast<iColliderTerrain*>((csBulletColliderTerrain*)terrainCollider);
//  }
//}
//
//void csBulletTerrainObject::RebuildObject ()
//{
//  //TODO
//  if (!sector)
//  {
//    csFPrintf  (stderr, "csBulletTerrainObject: Haven't add the object to a sector.\nRebuild failed.\n");
//    return;
//  }
//  if (!terrainCollider)
//  {  
//    csFPrintf  (stderr, "csBulletTerrainObject: Haven't add any collider to the object.\nRebuild failed.\n");
//    return;
//  }
//
//  //set the sector to collider. Then create btCollisionShape for collider.
//  terrainCollider->collSector = sector;
//  terrainCollider->GenerateShape ();
//
//  if (insideWorld)
//  {
//    for (size_t i = 0; i < bodies.GetSize(); i++)
//    {
//      sector->bulletWorld->removeRigidBody (bodies[i]);
//      delete bodies[i];
//    }
//    bodies.Empty ();
//  }
//
//  btVector3 localInertia (0.0f, 0.0f, 0.0f);
//
//  for (size_t i = 0; i < terrainCollider->colliders.GetSize(); i++ )
//  {
//    btCollisionShape* shape = terrainCollider->colliders[i];
//    btRigidBody body = new btRigidBody (0, 0, shape, localInertia);	
//    body->setWorldTransform (transforms[i]);
//    body->setUserPointer ((iColliderTerrain*)this);
//    sector->bulletWorld->addRigidBody (body);
//    bodies.Push (body);
//  }
//  insideWorld = true;
//}
//
//void csBulletTerrainObject::SetCollisionGroup (CollisionGroup group)
//{
//  //TODO
//}
//
//bool csBulletTerrainObject::Collide (iCollisionObject* otherObject)
//{
//  //TODO
//  return false;
//}
//
//HitBeamResult csBulletTerrainObject::HitBeam (const csVector3& start, const csVector3& end)
//{
//  //TODO
//  return HitBeamResult();
//}

#define AABB_DIMENSIONS 10000.0f

csBulletSector::csBulletSector (iObjectRegistry* object_reg,
                                csBulletCollisionSystem* sys)
 :scfImplementationType (this), sys (sys), sector (NULL),
  internalScale (1.0f), debugDraw (NULL),
  inverseInternalScale (1.0f)
{
  configuration = new btDefaultCollisionConfiguration ();
  dispatcher = new btCollisionDispatcher (configuration);
  solver = new btSequentialImpulseConstraintSolver;

  const int maxProxies = 32766;
  btVector3 worldAabbMin (-AABB_DIMENSIONS, -AABB_DIMENSIONS, -AABB_DIMENSIONS);
  btVector3 worldAabbMax (AABB_DIMENSIONS, AABB_DIMENSIONS, AABB_DIMENSIONS);
  broadphase = new btAxisSweep3 (worldAabbMin, worldAabbMax, maxProxies);

  bulletWorld = new btDynamicsWorld (dispatcher,
    broadphase, solver, configuration);
  SetGravity (csVector3 (0.0f, =9.81f, 0.0f));

  csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet> (
    object_reg, "crystalspace.shared.stringset");
  baseID = strings->Request ("base");
  colldetID = strings->Request ("colldet");
}

csBulletSector::~csBulletSector ()
{
  /*joints.DeleteAll ();
  rigidBodies.DeleteAll ();
  softBodies.DeleteAll ();
  collisionObjects.DeleteAll ();*/
  portals.DeleteAll ();
  
  delete bulletWorld;
  delete debugDraw;
  delete dispatcher;
  delete configuration;
  delete solver;
  delete broadphase;
  delete softWorldInfo;
}

void csBulletSector::SetInternalScale (float scale)
{
  CS_ASSERT(!dynamicBodies.GetSize ()
    && !colliderBodies.GetSize ()
    && !terrainColliders.GetSize ());

  // save gravity
  csVector3 tempGravity = GetGravity ();

  // update parameters
  internalScale = scale;
  inverseInternalScale = 1.0f / scale;

  // update AABB world dimensions
  btVector3 worldAabbMin (-AABB_DIMENSIONS * scale, -AABB_DIMENSIONS * scale,
    -AABB_DIMENSIONS * scale);
  btVector3 worldAabbMax (AABB_DIMENSIONS * scale, AABB_DIMENSIONS * scale,
    AABB_DIMENSIONS * scale);

  delete broadphase;

  const int maxProxies = 32766;
  broadphase = new btAxisSweep3 (worldAabbMin, worldAabbMax, maxProxies);

  if (isSoftWorld)
  {
    bulletWorld = new btSoftRigidDynamicsWorld (
      dispatcher, broadphase, solver, configuration);
    softWorldInfo->m_broadphase = broadphase;
  }
  else
    bulletWorld = new btDiscreteDynamicsWorld (
    dispatcher, broadphase, solver, configuration);

  SetGravity (tempGravity);
}

void csBulletSector::SetGravity (const csVector3& v)
{
  btVector3 gravity = CSToBullet (v, internalScale);
  bulletWorld->setGravity (gravity);

  if (isSoftWorld)
    softWorldInfo->m_gravity = gravity;
}

void csBulletSector::AddCollisionObject (iCollisionObject* object)
{
  csRef<iCollisionObject> obj (object);
  collisionObjects.Push (obj);

  //Currenly object has to add to sector before they RebuildObject()
  //So the add to bullet world function should be called in RebuildObject()
  csBulletCollisionObject* collObj = dynamic_cast<csBulletCollisionObject*> (object);
  collObj->sector = this;
}

void csBulletSector::RemoveCollisionObject (iCollisionObject* object)
{
  csBulletCollisionObject* collObject = dynamic_cast<csBulletCollisionObject*> (object);
  CS_ASSERT (collObject);

  collObject->RemoveBulletObject ();

  collObject->insideWorld = false;

  collisionObjects.Delete (object);
}

void csBulletSector::AddPortal (iPortal* portal)
{
  //TODO
}

void csBulletSector::RemovePortal (iPortal* portal)
{
  //TODO
}

void csBulletSector::SetSector (iSector* sector)
{
  //TODO
}

iSector* csBulletSector::GetSector ()
{
  //TODO
}

HitBeamResult csBulletSector::HitBeam (const csVector3& start, const csVector3& end)
{
  btVector3 rayFrom = CSToBullet (start, internalScale);
  btVector3 rayTo = CSToBullet (end, internalScale);

  btCollisionWorld::ClosestRayResultCallback rayCallback (rayFrom, rayTo);
  bulletWorld->rayTest (rayFrom, rayTo, rayCallback);

  CS::Collision::HitBeamResult result;

  if(rayCallback.hasHit ())
  {
    csBulletCollisionObject* collObject = dynamic_cast<csBulletCollisionObject*> (
      rayCallback.m_collisionObject->getUserPointer ());

    if (!collObject->isPhysics)
    {
      if(collObject->GetObjectType () == CS::Collision::CollisionObjectType::COLLISION_OBJECT_GHOST)
      {
        //Portals are not included.
        for (size_t i = 0; i < portals.GetSize (); i++)
        {
          if (portals[i]->ghostPortal1 == collObject
            || portals[i]->ghostPortal2 == collObject)
            return result;
        }
      }
      result.hasHit = true;
      result.object = collObject;
      result.isect = BulletToCS (rayCallback.m_hitPointWorld,
        inverseInternalScale);
      result.normal = BulletToCS (rayCallback.m_hitNormalWorld,
        inverseInternalScale);
      return result;
    }
    else
    {
      iPhysicalBody* phyBody = dynamic_cast<iPhysicalBody*> (collObject);
      if(phyBody->GetBodyType () == BODY_SOFT)
      {
        btSoftBody* body = btSoftBody::upcast (rayCallback.m_collisionObject);
        btSoftBody::sRayCast ray;

        if (body->rayTest (rayFrom, rayTo, ray))
        {
          result.hasHit = true;
          result.body = collisionObjects;
          result.isect = BulletToCS (rayCallback.m_hitPointWorld,
            inverseInternalScale);
          result.normal = BulletToCS (rayCallback.m_hitNormalWorld,
            inverseInternalScale);	

          // Find the closest vertex that was hit
          // TODO: there must be something more efficient than a second ray test
          btVector3 impact = rayFrom + (rayTo - rayFrom) * ray.fraction;
          switch (ray.feature)
          {
            case btSoftBody::eFeature::Face:
            {
              btSoftBody::Face& face = body->m_faces[ray.index];
              btSoftBody::Node* node = face.m_n[0];
              float distance = (node->m_x - impact).length2 ();

              for (int i = 1; i < 3; i++)
              {
                float nodeDistance = (face.m_n[i]->m_x - impact).length2 ();
                if (nodeDistance < distance)
                {
                  node = face.m_n[i];
                  distance = nodeDistance;
                }
              }

              result.vertexIndex = size_t (node - &body->m_nodes[0]);
              break;
            }
            default:
              break;
          }
          return result;
        }
      }
      else
      {
        result.hasHit = true;
        result.object = collObject;
        result.isect = BulletToCS (rayCallback.m_hitPointWorld,
          inverseInternalScale);
        result.normal = BulletToCS (rayCallback.m_hitNormalWorld,
          inverseInternalScale);
        return result;
      }
    }
  }
  return result;
}

HitBeamResult csBulletSector::HitBeamPortal (const csVector3& start, const csVector3& end)
{
  btVector3 rayFrom = CSToBullet (start, internalScale);
  btVector3 rayTo = CSToBullet (end, internalScale);

  btCollisionWorld::ClosestRayResultCallback rayCallback (rayFrom, rayTo);
  bulletWorld->rayTest (rayFrom, rayTo, rayCallback);

  CS::Collision::HitBeamResult result;

  if(rayCallback.hasHit ())
  {
    csBulletCollisionObject* collObject = dynamic_cast<csBulletCollisionObject*> (
      rayCallback.m_collisionObject->getUserPointer ());

    if (!collObject->isPhysics)
    {
      result.hasHit = true;
      result.object = collObject;
      result.isect = BulletToCS (rayCallback.m_hitPointWorld,
        inverseInternalScale);
      result.normal = BulletToCS (rayCallback.m_hitNormalWorld,
        inverseInternalScale);
      return result;
    }
    else
    {
      iPhysicalBody* phyBody = dynamic_cast<iPhysicalBody*> (collObject);
      if(phyBody->GetBodyType () == BODY_SOFT)
      {
        btSoftBody* body = btSoftBody::upcast (rayCallback.m_collisionObject);
        btSoftBody::sRayCast ray;

        if (body->rayTest (rayFrom, rayTo, ray))
        {
          result.hasHit = true;
          result.body = collisionObjects;
          result.isect = BulletToCS (rayCallback.m_hitPointWorld,
            inverseInternalScale);
          result.normal = BulletToCS (rayCallback.m_hitNormalWorld,
            inverseInternalScale);	

          // Find the closest vertex that was hit
          // TODO: there must be something more efficient than a second ray test
          btVector3 impact = rayFrom + (rayTo - rayFrom) * ray.fraction;
          switch (ray.feature)
          {
          case btSoftBody::eFeature::Face:
            {
              btSoftBody::Face& face = body->m_faces[ray.index];
              btSoftBody::Node* node = face.m_n[0];
              float distance = (node->m_x - impact).length2 ();

              for (int i = 1; i < 3; i++)
              {
                float nodeDistance = (face.m_n[i]->m_x - impact).length2 ();
                if (nodeDistance < distance)
                {
                  node = face.m_n[i];
                  distance = nodeDistance;
                }
              }

              result.vertexIndex = size_t (node - &body->m_nodes[0]);
              break;
            }
          default:
            break;
          }
          return result;
        }
      }
      else
      {
        result.hasHit = true;
        result.object = collObject;
        result.isect = BulletToCS (rayCallback.m_hitPointWorld,
          inverseInternalScale);
        result.normal = BulletToCS (rayCallback.m_hitNormalWorld,
          inverseInternalScale);
        return result;
      }
    }
  }
  return result;
}

bool csBulletSector::CollisionTest (iCollisionObject* object, csArray<CollisionData>& collisions)
{
  size_t length = collisions.GetSize ();
  PointContactResult result(collisions);

  csBulletCollisionObject* collObject = dynamic_cast<csBulletCollisionObject*> (object);
  if (collObject->isTerrain)
  {
    //Here is a question. Should we let user to do collision test on terrain object?
    csBulletColliderTerrain* terrainShape = collObject->colliders[0];
    for (size_t i = 0; i< terrainShape->colliders.GetSize (); i++)
    {
      btRigidBody* body = terrainShape->GetBulletObject (i);
      bulletWorld->contactTest (body, result);
    }
  }
  bulletWorld->contactTest (collObject->btObject, result);
  if (length != collisions.GetSize ())
    return trus;
  else
    return false;
}

bool csBulletSector::BulletCollide (btCollisionObject* objectA,
                                    btCollisionObject* objectB)
{
  //contactPairTest
  //Does user need the collision data?
  csArray<CollisionData> data;
  PointContactResult result(data);
  bulletWorld->contactPairTest (objectA, objectB, data);
  if (data.IsEmpty ())
    return true;
  else
    return false;
}

HitBeamResult csBulletSector::RigidHitBeam (btCollisionObject* object, 
                                            const csVector3& start,
                                            const csVector3& end)
{
  btVector3 rayFrom = CSToBullet (start, internalScale);
  btVector3 rayTo = CSToBullet (end, internalScale);

  btCollisionWorld::ClosestRayResultCallback rayCallback (rayFrom, rayTo);

  //Call RayTestSingle or use callback.process ? 
  //The latter calls the former, but before that it will check the collision filter.

  bulletWorld->rayTestSingle (rayFrom, rayTo, object,
                              object->getCollisionShape(),
                              object->getWorldTransform(),
                              rayCallback);

  CS::Collision::HitBeamResult result;

  if(rayCallback.hasHit ())
  {
    csBulletCollisionObject* collObject = dynamic_cast<csBulletCollisionObject*> (
      rayCallback.m_collisionObject->getUserPointer ());

    if (!collObject->isPhysics)
    {
      result.hasHit = true;
      result.object = collObject;
      result.isect = BulletToCS (rayCallback.m_hitPointWorld,
        inverseInternalScale);
      result.normal = BulletToCS (rayCallback.m_hitNormalWorld,
        inverseInternalScale);
      return result;
    }
    else
    {
      iPhysicalBody* phyBody = dynamic_cast<iPhysicalBody*> (collObject);
      if(phyBody->GetBodyType () == BODY_SOFT)
      {
        btSoftBody* body = btSoftBody::upcast (rayCallback.m_collisionObject);
        btSoftBody::sRayCast ray;

        if (body->rayTest (rayFrom, rayTo, ray))
        {
          result.hasHit = true;
          result.body = collisionObjects;
          result.isect = BulletToCS (rayCallback.m_hitPointWorld,
            inverseInternalScale);
          result.normal = BulletToCS (rayCallback.m_hitNormalWorld,
            inverseInternalScale);	

          // Find the closest vertex that was hit
          // TODO: there must be something more efficient than a second ray test
          btVector3 impact = rayFrom + (rayTo - rayFrom) * ray.fraction;
          switch (ray.feature)
          {
          case btSoftBody::eFeature::Face:
            {
              btSoftBody::Face& face = body->m_faces[ray.index];
              btSoftBody::Node* node = face.m_n[0];
              float distance = (node->m_x - impact).length2 ();

              for (int i = 1; i < 3; i++)
              {
                float nodeDistance = (face.m_n[i]->m_x - impact).length2 ();
                if (nodeDistance < distance)
                {
                  node = face.m_n[i];
                  distance = nodeDistance;
                }
              }

              result.vertexIndex = size_t (node - &body->m_nodes[0]);
              break;
            }
          default:
            break;
          }
          return result;
        }
      }
      else
      {
        result.hasHit = true;
        result.object = collObject;
        result.isect = BulletToCS (rayCallback.m_hitPointWorld,
          inverseInternalScale);
        result.normal = BulletToCS (rayCallback.m_hitNormalWorld,
          inverseInternalScale);
        return result;
      }
    }
  }
  return result;
}

void csBulletSector::SetSimulationSpeed (float speed)
{
}

void csBulletSector::SetStepParameters (float timeStep,
                                        size_t maxSteps,
                                        size_t interations)
{
  worldTimeStep = timeStep;
  worldMaxSteps = maxSteps;
  btContactSolverInfo& info = bulletWorld->getSolverInfo ();
  info.m_numIterations = (int)iterations;
}

void csBulletSector::Step (float duration)
{
  // Update the soft body anchors
  for (csWeakRefArray<csBulletSoftBody>::Iterator it = anchoredSoftBodies.GetIterator (); it.HasNext (); )
  {
    csBulletSoftBody* body = static_cast<csBulletSoftBody*> (it.Next ());
    body->UpdateAnchorPositions ();
  }

  // Step the simulation
  bulletWorld->stepSimulation (duration, (int)worldMaxSteps, worldTimeStep);

  // Check for collisions
  // Collision detection is already did in stepSimulation.
  //CheckCollisions();
}

void csBulletSector::SetLinearDampener (float d)
{
  linearDampening = d;
}

void csBulletSector::SetRollingDampener (float d)
{
  angularDampening = d;
}

void csBulletSector::SetAutoDisableParams (float linear, float angular,
                                           float time)
{
  linearDisableThreshold = linear;
  angularDisableThreshold = angular;
  timeDisableThreshold = time;
}

void csBulletSector::AddRidigBody (iRigidBody* body)
{
  csRef<iRigidBody> btBody (body);
  rigidBodies.Push (btBody);

  //Currenly object has to add to sector before they RebuildObject()
  //So the add to bullet world function should be called in RebuildObject()
  csBulletCollisionObject* collObj = dynamic_cast<csBulletCollisionObject*> (body);
  collObj->sector = this;
}

void csBulletSector::RemoveRigidBody (iRigidBody* body)
{
  csBulletRigidBody* btBody = dynamic_cast<csBulletRigidBody*> (body);
  CS_ASSERT (btBody);

  btBody->RemoveBulletObject ();

  btBody->insideWorld = false;

  rigidBodies.Delete (btBody);
}

void csBulletSector::AddSoftBody (iSoftBody* body)
{
  csRef<iSoftBody> btBody (body);
  softBodies.Push (btBody);

  //Currenly object has to add to sector before they RebuildObject()
  //So the add to bullet world function should be called in RebuildObject()
  csBulletCollisionObject* collObj = dynamic_cast<csBulletCollisionObject*> (body);
  collObj->sector = this;
}

void csBulletSector::RemoveSoftBody (iSoftBody* body)
{
  csBulletSoftBody* btBody = dynamic_cast<csBulletSoftBody*> (body);
  CS_ASSERT (btBody);

  btBody->RemoveBulletObject ();

  btBody->insideWorld = false;

  softBodies.Delete (btBody);
}

void PreTickCallback (btDynamicsWorld* world, btScalar timeStep)
{
  csBulletDynamicsSystem* system = (csBulletDynamicsSystem*) world->getWorldUserInfo ();
  system->UpdateSoftBodies (timeStep);
}

void csBulletSector::SetSoftBodyEnabled (bool enabled)
{
  CS_ASSERT(!rigidBodies.GetSize ()
    && !collisionObjects.GetSize ()
    && !softBodies.GetSize ());

  if (enabled == isSoftWorld)
    return;

  isSoftWorld = enabled
  // re-create configuration, dispatcher & dynamics world
  btVector3 gravity = bulletWorld->getGravity ();
  delete bulletWorld;
  delete dispatcher;
  delete configuration;

  if (isSoftWorld)
  {
    configuration = new btSoftBodyRigidBodyCollisionConfiguration ();
    dispatcher = new btCollisionDispatcher (configuration);
    bulletWorld = new btSoftRigidDynamicsWorld
      (dispatcher, broadphase, solver, configuration);

    softWorldInfo = new btSoftBodyWorldInfo ();
    softWorldInfo->m_broadphase = broadphase;
    softWorldInfo->m_dispatcher = dispatcher;
    softWorldInfo->m_gravity = gravity;
    softWorldInfo->air_density = 1.2f;
    softWorldInfo->water_density = 0.0f;
    softWorldInfo->water_offset = 0.0f;
    softWorldInfo->water_normal = btVector3 (0.0f, 1.0f, 0.0f);
    softWorldInfo->m_sparsesdf.Initialize ();
  }
  else
  {
    configuration = new btDefaultCollisionConfiguration ();
    dispatcher = new btCollisionDispatcher (configuration);
    bulletWorld = new btDiscreteDynamicsWorld
      (dispatcher, broadphase, solver, configuration);
    delete softWorldInfo;
  }

  bulletWorld->setGravity (gravity);

  // Register a pre-tick callback
  bulletWorld->setInternalTickCallback (PreTickCallback, this, true);
}

void csBulletSector::SaveWorld (const char* filename)
{
  //What's this?
#ifndef CS_HAVE_BULLET_SERIALIZER
  return false;
#else

  // create a large enough buffer
  int maxSerializeBufferSize = 1024 * 1024 * 5;

  btDefaultSerializer* serializer = new btDefaultSerializer (maxSerializeBufferSize);
  bulletWorld->serialize (serializer);

  FILE* file = fopen (filename,"wb");
  if (!file) return false;

  if (fwrite (serializer->getBufferPointer (), serializer->getCurrentBufferSize (), 1, file)
    != 1)
    return false;

  if (fclose(file) == EOF) return false;

  return true;

#endif
}

void csBulletSector::DebugDraw (iView* rview)
{
  if (!debugDraw)
  {
    debugDraw = new csBulletDebugDraw (inverseInternalScale);
    bulletWorld->setDebugDrawer (debugDraw);
  }

  bulletWorld->debugDrawWorld ();
  debugDraw->DebugDraw (rview);
}

void csBulletSector::SetDebugMode (DebugMode mode)
{
  if (mode = CS::Physics::Bullet::DEBUG_NOTHING)
  {
    if (debugDraw)
    {
      delete debugDraw;
      debugDraw = 0;
      bulletWorld->setDebugDrawer (0);
    }
    return;
  }
  if (!debugDraw)
  {
    debugDraw = new csBulletDebugDraw (inverseInternalScale);
    bulletWorld->setDebugDrawer (debugDraw);
  }

  debugDraw->SetDebugMode (mode);
}

DebugMode csBulletSector::GetDebugMode ()
{
  if (!debugDraw)
    return CS::Physics::Bullet::DEBUG_NOTHING;

  return debugDraw->GetDebugMode ();
}

SCF_IMPLEMENT_FACTORY (csBulletCollisionSystem)

csBulletCollisionSystem::csBulletCollisionSystem (iBase* iParent)
  : scfImplementationType (this, iParent)
{
}

csBulletCollisionSystem::~csBulletCollisionSystem ()
{
  colliders.DeleteAll ();
  objects.DeleteAll ();
  collSectors.DeleteAll ();
}

bool csBulletCollisionSystem::Initialize (iObjectRegistry* object_reg)
{
  this->object_reg = object_reg;
  return true;
}

csPtr<iColliderConvexMesh> csBulletCollisionSystem::CreateColliderConvexMesh (iMeshWrapper* mesh)
{
  csRef<csBulletCollider> collider;
  collider.AttachNew (new csBulletColliderConvexMesh (mesh));

  colliders.Push (collider);
  return collider;
}

csPtr<iColliderConcaveMesh> csBulletCollisionSystem::CreateColliderConcaveMesh (iMeshWrapper* mesh,
                                                                                bool isStatic /* = false */)
{
  csRef<csBulletCollider> collider;
  collider.AttachNew (new csBulletColliderConcaveMesh (mesh));

  colliders.Push (collider);
  return collider;
}

csPtr<iColliderConcaveMeshScaled> csBulletCollisionSystem::CreateColliderConcaveMeshScaled (
  iColliderConcaveMesh* collider, float scale)
{
  csRef<csBulletCollider> coll;
  collider.AttachNew (new csBulletColliderConcaveMeshScaled (coll, scale));

  colliders.Push (coll);
  return coll;
}

csPtr<iColliderCylinder> csBulletCollisionSystem::CreateColliderCylinder (float length, float radius)
{
  csRef<csBulletCollider> collider;
  collider.AttachNew (new csBulletColliderCylinder (length, radius));

  colliders.Push (collider);
  return collider;
}

csPtr<iColliderBox> csBulletCollisionSystem::CreateColliderBox (const csVector3& size)
{
  csRef<csBulletColliderBox> collider;
  collider.AttachNew (new csBulletColliderBox (size));

  colliders.Push (collider);
  return collider;
} 

csPtr<iColliderSphere> csBulletCollisionSystem::CreateColliderSphere (float radius)
{
  csRef<csBulletCollider> collider;
  collider.AttachNew (new csBulletColliderSphere (radius));

  colliders.Push (collider);
  return collider;
}

csPtr<iColliderCapsule> csBulletCollisionSystem::CreateColliderCapsule (float length, float radius)
{
  csRef<csBulletCollider> collider;
  collider.AttachNew (new csBulletColliderCapsule (length, radius));
  
  colliders.Push (collider);
  return collider;
}

csPtr<iColliderCapsule> csBulletCollisionSystem::CreateColliderCone (float length, float radius)
{
  csRef<csBulletCollider> collider;
  collider.AttachNew (new csBulletColliderCone (length, radius));

  colliders.Push (collider);
  return collider;
}

csPtr<iColliderPlane> csBulletCollisionSystem::CreateColliderPlane (const csPlane3& plane)
{
  csRef<csBulletCollider> collider;
  collider.AttachNew (new csBulletColliderPlane (plane));

  colliders.Push (collider);
  return collider;
}

csPtr<iColliderTerrain> csBulletCollisionSystem::CreateColliderTerrain (const iTerrainSystem* terrain, 
                                                                        float minHeight /* = 0 */, 
                                                                        float maxHeight /* = 0 */)
{
  csRef<csBulletCollider> collider;
  collider.AttachNew (new csBulletColliderTerrain (terrain, minHeight, maxHeight));

  colliders.Push (collider);
  return collider;
}

csRef<iCollisionObject> csBulletCollisionSystem::CreateCollisionObject ()
{
  csRef<iCollisionObject> collObject;
  collObject.AttachNew (new csBulletCollisionObject (this));

  objects.Push (collObject);
  return collObject;
}

csRef<iCollisionActor> csBulletCollisionSystem::CreateCollisionActor ()
{
  csRef<iCollisionActor> collActor;
  collActor.AttachNew (new csBulletCollisionActor (this));

  actors.Push (collActor);
  return collActor;
}
csRef<iCollisionSector> csBulletCollisionSystem::CreateCollisionSector ()
{
  csRef<iCollisionSector> collSector;
  collSector.AttachNew (new csBulletSector (object_reg, this));

  collSectors.Push (collSector);
  return collSector;
}

CollisionGroup& csBulletCollisionSystem::CreateCollisionGroup (const char* name)
{
//TODO
}

CollisionGroup& csBulletCollisionSystem::FindCollisionGroup (const char* name)
{
//TODO
}

void csBulletCollisionSystem::SetGroupCollision (CollisionGroup& group1,
                                                 CollisionGroup& group2,
                                                 bool collide)
{
//TODO
}
bool csBulletCollisionSystem::GetGroupCollision (CollisionGroup& group1,
                                                 CollisionGroup& group2);
{
//TODO
}

void csBulletCollisionSystem::DecomposeConcaveMesh (iCollisionObject* object, iMeshWrapper* mesh)
{
  csBulletCollisionObject* btCollObject = dynamic_cast<csBulletCollisionObject*> (object);
  //Question: This function will create btCollisionShape from the mesh.
  //But didn't know the internalScale, how to create a collision shape?
}
}
CS_PLUGIN_NAMESPACE_END(Bullet2)
