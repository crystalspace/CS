#include "cssysdef.h"
#include "ivaria/softanim.h"
#include "imesh/animesh.h"
#include "iengine/scenenode.h"
#include "iengine/movable.h"
#include "csgeom/sphere.h"
#include "csgeom/tri.h"
#include "imesh/genmesh.h"
#include "imesh/object.h"
#include "csutil/sysfunc.h"
#include "iutil/objreg.h"
#include "ivaria/view.h"
#include "ivaria/collision2.h"
#include "igeom/trimesh.h"
#include "iengine/portal.h"
#include "iengine/portalcontainer.h"

#include "csutil/custom_new_disable.h"

#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"
#include "BulletSoftBody/btSoftBody.h"
#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"
#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "BulletSoftBody/btSoftBodyHelpers.h"
#include "convexdecompose/ConvexBuilder.h"

#include "csutil/custom_new_enable.h"

#include "bullet2.h"
#include "common2.h"
#include "colliders2.h"
#include "rigidbody2.h"
#include "softbody2.h"
#include "collisionactor2.h"
#include "joint2.h"

const float COLLISION_THRESHOLD = 0.01f;
#define AABB_DIMENSIONS 10000.0f

CS_PLUGIN_NAMESPACE_BEGIN(Bullet2)
{

struct PointContactResult : public btCollisionWorld::ContactResultCallback
{
  csArray<CS::Collision2::CollisionData>& colls;
  csBulletSystem* sys;
  PointContactResult(csBulletSystem* sys, csArray<CS::Collision2::CollisionData>& collisions) : colls(collisions), sys(sys) 
  {
  }
  virtual	btScalar	addSingleResult (btManifoldPoint& cp,	const btCollisionObject* colObj0, int partId0,int index0,const btCollisionObject* colObj1,int partId1,int index1)
  {
    CS::Collision2::CollisionData data;
    data.objectA = static_cast<CS::Collision2::iCollisionObject*>(colObj0->getUserPointer ());
    data.objectB = static_cast<CS::Collision2::iCollisionObject*>(colObj1->getUserPointer ());
    data.penetration = cp.m_distance1 * sys->getInverseInternalScale ();
    data.positionWorldOnA = BulletToCS (cp.m_positionWorldOnA, sys->getInverseInternalScale ());
    data.positionWorldOnB = BulletToCS (cp.m_positionWorldOnB, sys->getInverseInternalScale ());
    data.normalWorldOnB = BulletToCS (cp.m_normalWorldOnB, sys->getInverseInternalScale ());
    colls.Push (data);
    return 0;
  }
};

csBulletSector::csBulletSector (csBulletSystem* sys)
 :scfImplementationType (this), sys (sys), isSoftWorld (false), hitPortal (NULL),
 sector (NULL), softWorldInfo (NULL), worldTimeStep (1.0f / 60.0f),
 worldMaxSteps (1), linearDampening (0.0f), angularDampening (0.0f), linearDisableThreshold (0.8f),
 angularDisableThreshold (1.0f), timeDisableThreshold (0.0f), debugDraw (NULL), allFilter (-1)
{
  configuration = new btDefaultCollisionConfiguration ();
  dispatcher = new btCollisionDispatcher (configuration);
  solver = new btSequentialImpulseConstraintSolver;

  const int maxProxies = 32766;

  btVector3 worldAabbMin (-AABB_DIMENSIONS, -AABB_DIMENSIONS, -AABB_DIMENSIONS);
  btVector3 worldAabbMax (AABB_DIMENSIONS, AABB_DIMENSIONS, AABB_DIMENSIONS);
  broadphase = new btAxisSweep3 (worldAabbMin, worldAabbMax, maxProxies);

  bulletWorld = new btDiscreteDynamicsWorld (dispatcher,
    broadphase, solver, configuration);

  broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());

  SetGravity (csVector3 (0.0f, -9.81f, 0.0f));

  CS::Collision2::CollisionGroup defaultGroup ("Default");
  defaultGroup.value = 1;
  collGroups.Push (defaultGroup);

  CS::Collision2::CollisionGroup staticGroup ("Static");
  staticGroup.value = 2;
  collGroups.Push (staticGroup);

  CS::Collision2::CollisionGroup kinematicGroup ("Kinematic");
  kinematicGroup.value = 4;
  collGroups.Push (kinematicGroup);

  CS::Collision2::CollisionGroup debrisGroup ("Debris");
  debrisGroup.value = 8;
  collGroups.Push (debrisGroup);

  CS::Collision2::CollisionGroup sensorGroup ("Sensor");
  sensorGroup.value = 16;
  collGroups.Push (sensorGroup);

  CS::Collision2::CollisionGroup characterGroup ("Character");
  characterGroup.value = 32;
  collGroups.Push (characterGroup);

  CS::Collision2::CollisionGroup portalGroup ("Portal");
  portalGroup.value = 64;
  collGroups.Push (portalGroup);

  systemFilterCount = 7;
}

csBulletSector::~csBulletSector ()
{
  for (size_t i = 0; i < portals.GetSize (); i++)
  {
    bulletWorld->removeCollisionObject (portals[i]->ghostPortal);
    delete portals[i];
  }

  joints.DeleteAll ();
  rigidBodies.DeleteAll ();
  softBodies.DeleteAll ();
  collisionObjects.DeleteAll ();
  portals.DeleteAll ();
  collisionActor = NULL;
  
  delete bulletWorld;
  delete debugDraw;
  delete dispatcher;
  delete configuration;
  delete solver;
  delete broadphase;
  delete softWorldInfo;
}

void csBulletSector::SetGravity (const csVector3& v)
{
  gravity = v;
  btVector3 gravity = CSToBullet (v, sys->getInternalScale ());
  bulletWorld->setGravity (gravity);

  if (isSoftWorld)
    softWorldInfo->m_gravity = gravity;
}

void csBulletSector::AddCollisionObject (CS::Collision2::iCollisionObject* object)
{
  csBulletCollisionObject* obj (dynamic_cast<csBulletCollisionObject*>(object));

  if (obj->GetObjectType () == CS::Collision2::COLLISION_OBJECT_PHYSICAL)
  {
    iPhysicalBody* phyBody = obj->QueryPhysicalBody ();
    if (phyBody->GetBodyType () == CS::Physics2::BODY_RIGID)
      AddRigidBody (phyBody->QueryRigidBody ());
    else
      AddSoftBody (phyBody->QuerySoftBody ());
  }
  else
  {
    collisionObjects.Push (obj);
    obj->sector = this;
    obj->collGroup = collGroups[1]; // Static Group.
    obj->AddBulletObject ();

    AddMovableToSector (object);
  }
}

void csBulletSector::RemoveCollisionObject (CS::Collision2::iCollisionObject* object)
{
  csBulletCollisionObject* collObject = dynamic_cast<csBulletCollisionObject*> (object);
  if (!collObject)
    return;

  if (collObject->GetObjectType () == CS::Collision2::COLLISION_OBJECT_PHYSICAL)
  {
    iPhysicalBody* phyBody = dynamic_cast<iPhysicalBody*> (object);
    if (phyBody->GetBodyType () == CS::Physics2::BODY_RIGID)
      RemoveRigidBody (phyBody->QueryRigidBody ());
    else
      RemoveSoftBody (phyBody->QuerySoftBody ());
  }
  else
  {
    collObject->RemoveBulletObject ();
    collObject->insideWorld = false;
    collisionObjects.Delete (collObject);

    RemoveMovableFromSector (object);
  }
}

CS::Collision2::iCollisionObject* csBulletSector::GetCollisionObject (size_t index)
{
  if (index >= 0 && index < collisionObjects.GetSize ())
    return collisionObjects[index]->QueryCollisionObject ();
  else
    return NULL;
}

void csBulletSector::AddPortal (iPortal* portal)
{
  CollisionPortal* newPortal = new CollisionPortal(portal);
  iSector* desSector = portal->GetSector ();

  newPortal->ghostPortal = new btGhostObject ();
  csSphere sphere = portal->GetWorldSphere ();

  csOrthoTransform trans (csMatrix3 (), sphere.GetCenter ());
  newPortal->ghostPortal->setWorldTransform (CSToBullet (trans, sys->getInternalScale ()));

  btCollisionShape* shape = new btSphereShape (sphere.GetRadius () * sys->getInternalScale ());
  newPortal->ghostPortal->setCollisionShape (shape);

  newPortal->ghostPortal->setCollisionFlags (newPortal->ghostPortal->getCollisionFlags() 
    | btCollisionObject::CF_NO_CONTACT_RESPONSE);

  bulletWorld->addCollisionObject (newPortal->ghostPortal, collGroups[6].value, allFilter ^ collGroups[6].value);

  for (size_t i = 0; i < sys->collSectors.GetSize (); i++)
    if (sys->collSectors[i]->GetSector () == desSector)
    {
      newPortal->desSector = sys->collSectors[i];
      break;
    }

  portals.Push (newPortal);
}

void csBulletSector::RemovePortal (iPortal* portal)
{
  for (size_t i = 0; i < portals.GetSize (); i++)
  {
    if (portals[i]->portal == portal)
    {
      for (size_t j = 0; j < portals[i]->objects.GetSize (); j++)
        portals[i]->desSector->RemoveCollisionObject (portals[i]->objects[j]->objectCopy);
      bulletWorld->removeCollisionObject (portals[i]->ghostPortal);
      delete portals[i];
      portals.DeleteIndexFast (i);
      return;
    }
  }
}

void csBulletSector::SetSector (iSector* sector)
{
   this->sector = sector;
   const csSet<csPtrKey<iMeshWrapper> >& portal_meshes = 
     sector->GetPortalMeshes ();
   csSet<csPtrKey<iMeshWrapper> >::GlobalIterator it = 
     portal_meshes.GetIterator ();

   while (it.HasNext ())
   {
     iMeshWrapper* portalMesh = it.Next ();
     iPortalContainer* portalContainer = portalMesh->GetPortalContainer ();
     int i; 
     for (i = 0; i < portalContainer->GetPortalCount (); i++)
     {
       iPortal* portal = portalContainer->GetPortal (i);
       AddPortal (portal);
     }
   }
}

CS::Collision2::HitBeamResult csBulletSector::HitBeam (const csVector3& start, const csVector3& end)
{
  btVector3 rayFrom = CSToBullet (start, sys->getInternalScale ());
  btVector3 rayTo = CSToBullet (end, sys->getInternalScale ());

  btCollisionWorld::ClosestRayResultCallback rayCallback (rayFrom, rayTo);
  bulletWorld->rayTest (rayFrom, rayTo, rayCallback);

  CS::Collision2::HitBeamResult result;

  if(rayCallback.hasHit ())
  {
    CS::Collision2::iCollisionObject* collObject = static_cast<CS::Collision2::iCollisionObject*> (
      rayCallback.m_collisionObject->getUserPointer ());

    // It's a portal..
    if (rayCallback.m_collisionObject->getInternalType () == btCollisionObject::CO_GHOST_OBJECT
      && rayCallback.m_collisionObject->getUserPointer () == NULL)
    {
      collObject = NULL;
      hitPortal = btGhostObject::upcast (rayCallback.m_collisionObject);
    }

    if (rayCallback.m_collisionObject->getInternalType () == btCollisionObject::CO_SOFT_BODY)
    {
      btSoftBody* body = btSoftBody::upcast (rayCallback.m_collisionObject);
      btSoftBody::sRayCast ray;

      if (body->rayTest (rayFrom, rayTo, ray))
      {
        result.hasHit = true;
        result.object = collObject;
        result.isect = BulletToCS (rayCallback.m_hitPointWorld,
          sys->getInverseInternalScale ());
        result.normal = BulletToCS (rayCallback.m_hitNormalWorld,
          sys->getInverseInternalScale ());	

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
      } //has hit softbody
    } //softBody
    else
    { 
      result.hasHit = true;
      result.object = collObject;
      result.isect = BulletToCS (rayCallback.m_hitPointWorld,
        sys->getInverseInternalScale ());
      result.normal = BulletToCS (rayCallback.m_hitNormalWorld,
        sys->getInverseInternalScale ());
      return result;
    } // not softBody
  } //has hit
  return result;
}

CS::Collision2::HitBeamResult csBulletSector::HitBeamPortal (const csVector3& start, const csVector3& end)
{
  hitPortal = NULL;

  CS::Collision2::HitBeamResult result = HitBeam (start, end);

  if (result.hasHit && result.object == NULL && hitPortal)
  {
    //Portals are not included.
    for (size_t i = 0; i < portals.GetSize (); i++)
    {
      if (portals[i]->ghostPortal == hitPortal)
      {
        if (!portals[i]->portal->CompleteSector (0))
        {
          result.hasHit = false;
          return result;
        }
        else
        {
          csOrthoTransform warpWor;
          csVector3 newStart = start;
          csVector3 newEnd = end;
          if (portals[i]->portal->GetFlags ().Check (CS_PORTAL_WARP))
          {
            portals[i]->portal->ObjectToWorld (
              BulletToCS (hitPortal->getWorldTransform (), sys->getInverseInternalScale ()),
              warpWor);
            newStart = warpWor.Other2This (start);
            newEnd = warpWor.Other2This (end);
          }

          result = portals[i]->desSector->HitBeamPortal (newStart, newEnd);
          return result;
        }
      }
    }
    
  }
  return result;

}

CS::Collision2::CollisionGroup& csBulletSector::CreateCollisionGroup (const char* name)
{
  size_t groupCount = collGroups.GetSize ();
  if (groupCount >= sizeof (CS::Collision2::CollisionGroupMask) * 8)
    return collGroups[0];

  CS::Collision2::CollisionGroup newGroup(name);
  newGroup.value = 1 << groupCount;
  collGroups.Push (newGroup);
  return collGroups[groupCount];
}

CS::Collision2::CollisionGroup& csBulletSector::FindCollisionGroup (const char* name)
{
  size_t index = collGroups.FindKey (CollisionGroupVector::KeyCmp (name));
  if (index == csArrayItemNotFound)
    return collGroups[0];
  else
    return collGroups[index];
}

void csBulletSector::SetGroupCollision (const char* name1,
                                        const char* name2,
                                        bool collide)
{
  int index1 = collGroups.FindKey (CollisionGroupVector::KeyCmp (name1));
  int index2 = collGroups.FindKey (CollisionGroupVector::KeyCmp (name2));
  if (index1 == csArrayItemNotFound || index2 == csArrayItemNotFound)
    return;
  if (collide)
  {
    if (index1 >= systemFilterCount)
      collGroups[index1].value &= ~(1 << index2);
    if (index2 >= systemFilterCount)
      collGroups[index2].value &= ~(1 << index1);
  }
  else
  {
    if (index1 >= systemFilterCount)
      collGroups[index1].value |= 1 << index2;
    if (index2 >= systemFilterCount)
      collGroups[index2].value |= 1 << index1;
  }
}

bool csBulletSector::GetGroupCollision (const char* name1,
                                        const char* name2)
{
  int index1 = collGroups.FindKey (CollisionGroupVector::KeyCmp (name1));
  int index2 = collGroups.FindKey (CollisionGroupVector::KeyCmp (name2));
  if (index1 == csArrayItemNotFound || index2 == csArrayItemNotFound)
    return false;
  if ((collGroups[index1].value & (1 << index2)) != 0 
    || (collGroups[index2].value & (1 << index1)) != 0)
    return false;
  else
    return true;
}

bool csBulletSector::CollisionTest (CS::Collision2::iCollisionObject* object, 
                                    csArray<CS::Collision2::CollisionData>& collisions)
{

  if (!object)
    return false;

  size_t length = collisions.GetSize ();
  PointContactResult result(sys, collisions);

  csBulletCollisionObject* collObject = dynamic_cast<csBulletCollisionObject*> (object);
  if (collObject->GetObjectType () == CS::Collision2::COLLISION_OBJECT_BASE
    || collObject->GetObjectType () == CS::Collision2::COLLISION_OBJECT_PHYSICAL)
  {
    if (collObject->isTerrain)
    {
      //Here is a question. Should we let user to do collision test on terrain object?
      csBulletColliderTerrain* terrainShape = dynamic_cast<csBulletColliderTerrain*> (collObject->colliders[0]);
      for (size_t i = 0; i< terrainShape->colliders.GetSize (); i++)
      {
        btRigidBody* body = terrainShape->GetBulletObject (i);
        bulletWorld->contactTest (body, result);
      }
    }
    else
      bulletWorld->contactTest (collObject->btObject, result);
  }
  else
  {
    btPairCachingGhostObject* ghost = dynamic_cast<btPairCachingGhostObject*> (
      btGhostObject::upcast (collObject->btObject));

    bulletWorld->getDispatcher()->dispatchAllCollisionPairs(
      ghost->getOverlappingPairCache(), bulletWorld->getDispatchInfo(), bulletWorld->getDispatcher());

    for (int i = 0; i < ghost->getOverlappingPairCache()->getNumOverlappingPairs(); i++)
    {
      btManifoldArray manifoldArray;
      btBroadphasePair* collisionPair = &ghost->getOverlappingPairCache()->getOverlappingPairArray()[i];

      if (collisionPair->m_algorithm)
        collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);

      for (int j=0;j<manifoldArray.size();j++)
      {
        btPersistentManifold* manifold = manifoldArray[j];
        btCollisionObject* objA = static_cast<btCollisionObject*> (manifold->getBody0 ());
        btCollisionObject* objB = static_cast<btCollisionObject*> (manifold->getBody1 ());
        CS::Collision2::iCollisionObject* csCOA = static_cast<CS::Collision2::iCollisionObject*>(objA->getUserPointer ());
        CS::Collision2::iCollisionObject* csCOB = static_cast<CS::Collision2::iCollisionObject*>(objB->getUserPointer ());
        for (int p=0;p<manifold->getNumContacts();p++)
        {
          CS::Collision2::CollisionData data;
          data.objectA = csCOA;
          data.objectB = csCOB;

          const btManifoldPoint& pt = manifold->getContactPoint(p);
          data.penetration = pt.m_distance1 * sys->getInverseInternalScale ();
          data.positionWorldOnA = BulletToCS (pt.m_positionWorldOnA, sys->getInverseInternalScale ());
          data.positionWorldOnB = BulletToCS (pt.m_positionWorldOnB, sys->getInverseInternalScale ());
          data.normalWorldOnB = BulletToCS (pt.m_normalWorldOnB, sys->getInverseInternalScale ());
          collisions.Push (data);
        }
      }
    }
  }

  // Check the copy of this object in another sector.
  if (collObject->objectCopy)
    collObject->objectCopy->sector->CollisionTest (collObject->objectCopy, collisions);

  if (length != collisions.GetSize ())
    return true;
  else
    return false;
}

void csBulletSector::AddCollisionActor (CS::Collision2::iCollisionActor* actor)
{
  csRef<csBulletCollisionActor> obj (dynamic_cast<csBulletCollisionActor*>(actor));
  collisionActor = obj;
  obj->sector = this;
  obj->collGroup = collGroups[5]; // Actor Group.
  obj->AddBulletObject ();
}

void csBulletSector::RemoveCollisionActor ()
{
  collisionActor->RemoveBulletObject ();
  collisionActor->insideWorld = false;
  collisionActor = NULL;
}

CS::Collision2::iCollisionActor* csBulletSector::GetCollisionActor ()
{
  return collisionActor;
}

bool csBulletSector::BulletCollide (btCollisionObject* objectA,
                                    btCollisionObject* objectB)
{
  //contactPairTest
  //Does user need the collision data?
  csArray<CS::Collision2::CollisionData> data;
  PointContactResult result(sys, data);
  bulletWorld->contactPairTest (objectA, objectB, result);
  if (data.IsEmpty ())
    return false;
  else
    return true;
}

CS::Collision2::HitBeamResult csBulletSector::RigidHitBeam (btCollisionObject* object, 
                                            const csVector3& start,
                                            const csVector3& end)
{
  btVector3 rayFrom = CSToBullet (start, sys->getInternalScale ());
  btVector3 rayTo = CSToBullet (end, sys->getInternalScale ());

  CS::Collision2::HitBeamResult result;

  if (object->getInternalType () == btCollisionObject::CO_SOFT_BODY)
  {
    btSoftBody* body = btSoftBody::upcast (object);
    btSoftBody::sRayCast ray;

    if (body->rayTest (rayFrom, rayTo, ray))
    {
      result.hasHit = true;
      result.object = static_cast<csBulletCollisionObject*> (object->getUserPointer ());

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

          result.isect = BulletToCS (node->m_x,
            sys->getInverseInternalScale ());
          result.normal = BulletToCS (node->m_n,
            sys->getInverseInternalScale ());	
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
    btCollisionWorld::ClosestRayResultCallback rayCallback (rayFrom, rayTo);

    //Ghost Object part?

    btTransform	rayFromTrans;
    btTransform	rayToTrans;

    rayFromTrans.setIdentity();
    rayFromTrans.setOrigin(rayFrom);
    rayToTrans.setIdentity();
    rayToTrans.setOrigin(rayTo);

    bulletWorld->rayTestSingle (rayFromTrans, rayToTrans, object,
      object->getCollisionShape(),
      object->getWorldTransform(),
      rayCallback);

    if(rayCallback.hasHit ())
    {
      result.hasHit = true;
      result.object = static_cast<csBulletCollisionObject*> (object->getUserPointer ());
      result.isect = BulletToCS (rayCallback.m_hitPointWorld,
        sys->getInverseInternalScale ());
      result.normal = BulletToCS (rayCallback.m_hitNormalWorld,
        sys->getInverseInternalScale ());
      return result;
    }
  }
  return result;
}

void csBulletSector::SetSimulationSpeed (float speed)
{
  //TODO
}

void csBulletSector::SetStepParameters (float timeStep,
                                        size_t maxSteps,
                                        size_t interations)
{
  worldTimeStep = timeStep;
  worldMaxSteps = maxSteps;
  btContactSolverInfo& info = bulletWorld->getSolverInfo ();
  info.m_numIterations = (int)interations;
}

void csBulletSector::Step (float duration)
{
  // Update the soft body anchors
  for (csWeakRefArray<csBulletSoftBody>::Iterator it = anchoredSoftBodies.GetIterator (); it.HasNext (); )
  {
    csBulletSoftBody* body = static_cast<csBulletSoftBody*> (it.Next ());
    body->UpdateAnchorPositions ();
  }

  // Update the state of collision collide with portals.
  UpdateCollisionPortals ();

  // Step the simulation
  bulletWorld->stepSimulation (duration, (int)worldMaxSteps, worldTimeStep);

  // Send the collision response of copies to source object.
  //for (size_t i = 0; i < collisionObjects.GetSize (); i++)
  //  if (collisionObjects[i]->objectOrigin)
  //    GetInformationFromCopy (collisionObjects[i]->objectOrigin, collisionObjects[i], duration);

  //for (size_t i = 0; i < rigidBodies.GetSize (); i++)
  //  if (rigidBodies[i]->objectOrigin)
  //    GetInformationFromCopy (rigidBodies[i]->objectOrigin, rigidBodies[i], duration);

  // Check for collisions
  CheckCollisions();
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

void csBulletSector::AddRigidBody (CS::Physics2::iRigidBody* body)
{
  csRef<csBulletRigidBody> btBody (dynamic_cast<csBulletRigidBody*>(body));
  rigidBodies.Push (btBody);

  btBody->sector = this;
  btBody->collGroup = collGroups[0]; // Default Group.
  btBody->AddBulletObject ();

  AddMovableToSector (body);
}

void csBulletSector::RemoveRigidBody (CS::Physics2::iRigidBody* body)
{
  csBulletRigidBody* btBody = dynamic_cast<csBulletRigidBody*> (body);
  CS_ASSERT (btBody);

  btBody->RemoveBulletObject ();
  RemoveMovableFromSector (body);

  rigidBodies.Delete (btBody);
}

iRigidBody* csBulletSector::GetRigidBody (size_t index)
{
  return rigidBodies[index]->QueryRigidBody ();
}

void csBulletSector::AddSoftBody (CS::Physics2::iSoftBody* body)
{
  csRef<csBulletSoftBody> btBody (dynamic_cast<csBulletSoftBody*>(body));
  softBodies.Push (btBody);
  btBody->sector = this;
  btBody->collGroup = collGroups[0];
  btBody->AddBulletObject ();

  iMovable* movable = body->GetAttachedMovable ();
  if (movable)
  {
    iMeshWrapper* mesh = movable->GetSceneNode ()->QueryMesh ();
    if (!mesh)
      return;
    csRef<iGeneralMeshState> meshState =
      scfQueryInterface<iGeneralMeshState> (mesh->GetMeshObject ());
    csRef<CS::Physics2::iSoftBodyAnimationControl> animationControl =
      scfQueryInterface<CS::Physics2::iSoftBodyAnimationControl> (meshState->GetAnimationControl ());
    if (!animationControl->GetSoftBody ())
      animationControl->SetSoftBody (body);

    sector->GetMeshes ()->Add (mesh);
  }
}

void csBulletSector::RemoveSoftBody (CS::Physics2::iSoftBody* body)
{
  csBulletSoftBody* btBody = dynamic_cast<csBulletSoftBody*> (body);
  CS_ASSERT (btBody);

  btBody->RemoveBulletObject ();
  softBodies.Delete (btBody);

  RemoveMovableFromSector (body);
}

CS::Physics2::iSoftBody* csBulletSector::GetSoftBody (size_t index)
{
  return softBodies[index]->QuerySoftBody ();
}

void csBulletSector::RemoveJoint (CS::Physics2::iJoint* joint)
{
  csBulletJoint* btJoint = dynamic_cast<csBulletJoint*> (joint);
  CS_ASSERT(btJoint);

  btJoint->RemoveBulletJoint ();
  joints.Delete (btJoint);
}

void PreTickCallback (btDynamicsWorld* world, btScalar timeStep)
{
  csBulletSector* sector = (csBulletSector*) (world->getWorldUserInfo ());
  sector->UpdateSoftBodies (timeStep);
}

void csBulletSector::SetSoftBodyEnabled (bool enabled)
{
  CS_ASSERT(!rigidBodies.GetSize ()
    && !collisionObjects.GetSize ()
    && !softBodies.GetSize ());

  if (enabled == isSoftWorld)
    return;

  isSoftWorld = enabled;
  // re-create configuration, dispatcher & dynamics world
  btVector3 gra = bulletWorld->getGravity ();
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
    softWorldInfo->m_gravity = gra;
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

  bulletWorld->setGravity (gra);
  bulletWorld->setDebugDrawer (debugDraw);

  // Register a pre-tick callback
  bulletWorld->setInternalTickCallback (PreTickCallback, this, true);
}

bool csBulletSector::SaveWorld (const char* filename)
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
    debugDraw = new csBulletDebugDraw (sys->getInverseInternalScale ());
    bulletWorld->setDebugDrawer (debugDraw);
  }

  bulletWorld->debugDrawWorld ();
  debugDraw->DebugDraw (rview);
}

void csBulletSector::SetDebugMode (CS::Physics2::Bullet2::DebugMode mode)
{
  if (!debugDraw)
  {
    debugDraw = new csBulletDebugDraw (sys->getInverseInternalScale ());
    bulletWorld->setDebugDrawer (debugDraw);
  }

  debugDraw->SetDebugMode (mode);
}

CS::Physics2::Bullet2::DebugMode csBulletSector::GetDebugMode ()
{
  if (!debugDraw)
    return CS::Physics2::Bullet2::DEBUG_NOTHING;

  return debugDraw->GetDebugMode ();
}

void csBulletSector::StartProfile ()
{
  CProfileManager::Start_Profile ("Crystal Space scene");

  if (!debugDraw)
  {
    debugDraw = new csBulletDebugDraw (sys->getInverseInternalScale ());
    bulletWorld->setDebugDrawer (debugDraw);
  }
  debugDraw->StartProfile ();
}

void csBulletSector::StopProfile ()
{
  CProfileManager::Stop_Profile ();
  debugDraw->StopProfile ();
}

void csBulletSector::DumpProfile (bool resetProfile /* = true */)
{
  printf ("\n");
  printf ("==========================================================\n");
  printf ("====           Bullet dynamic scene profile           ====\n");
  printf ("==========================================================\n");
  CProfileManager::dumpAll ();
  printf ("==========================================================\n");
  printf ("\n");

  if (resetProfile)
    CProfileManager::Reset ();
}

void csBulletSector::UpdateSoftBodies (float timeStep)
{
  for (csWeakRefArray<csBulletSoftBody>::Iterator it = anchoredSoftBodies.GetIterator (); it.HasNext (); )
  {
    csBulletSoftBody* body = static_cast<csBulletSoftBody*> (it.Next ());
    body->UpdateAnchorInternalTick (timeStep);
  }
}

void csBulletSector::AddMovableToSector (CS::Collision2::iCollisionObject* obj)
{
  iMovable* movable = obj->GetAttachedMovable ();
  if (movable && sector)
  {
    iMeshWrapper* mesh = movable->GetSceneNode ()->QueryMesh ();
    iLight* light = movable->GetSceneNode ()->QueryLight ();
    if (mesh)
      sector->GetMeshes ()->Add (mesh);
    else
      sector->GetLights ()->Add (light);
  }
}

void csBulletSector::RemoveMovableFromSector (CS::Collision2::iCollisionObject* obj)
{
  iMovable* movable = obj->GetAttachedMovable ();
  if (movable && sector)
  {
    iMeshWrapper* mesh = movable->GetSceneNode ()->QueryMesh ();
    iLight* light = movable->GetSceneNode ()->QueryLight ();
    if (mesh)
      sector->GetMeshes ()->Remove (mesh);
    else
      sector->GetLights ()->Remove (light);
  }
}

void csBulletSector::CheckCollisions ()
{
  int numManifolds = bulletWorld->getDispatcher()->getNumManifolds();

  for (size_t i = 0; i < collisionObjects.GetSize (); i++)
    collisionObjects[i]->contactObjects.Empty ();
  for (size_t i = 0; i < rigidBodies.GetSize (); i++)
    rigidBodies[i]->contactObjects.Empty ();
  for (size_t i = 0; i < softBodies.GetSize (); i++)
    softBodies[i]->contactObjects.Empty ();

  // Could not get contacted softBody?
  for (int i = 0; i < numManifolds; i++)
  {
    btPersistentManifold* contactManifold =
      bulletWorld->getDispatcher ()->getManifoldByIndexInternal (i);
    if (contactManifold->getNumContacts ())
    {
      btCollisionObject* obA =
        static_cast<btCollisionObject*> (contactManifold->getBody0 ());
      btCollisionObject* obB =
        static_cast<btCollisionObject*> (contactManifold->getBody1 ());

      CS::Collision2::iCollisionObject* cs_obA = 
        static_cast<CS::Collision2::iCollisionObject*> (obA->getUserPointer ());
      CS::Collision2::iCollisionObject* cs_obB = 
        static_cast<CS::Collision2::iCollisionObject*> (obB->getUserPointer ());

      if (!cs_obA || !cs_obB)
        continue;
      
      csBulletCollisionObject* csCOA = dynamic_cast<csBulletCollisionObject*> (cs_obA);
      csBulletCollisionObject* csCOB = dynamic_cast<csBulletCollisionObject*> (cs_obB);

      if (csCOA->GetObjectType () == CS::Collision2::COLLISION_OBJECT_BASE
        || csCOA->GetObjectType () == CS::Collision2::COLLISION_OBJECT_PHYSICAL)
        if (csCOA->contactObjects.Contains (csCOB) == csArrayItemNotFound)
          csCOA->contactObjects.Push (csCOB);

      if (csCOB->GetObjectType () == CS::Collision2::COLLISION_OBJECT_BASE
        || csCOB->GetObjectType () == CS::Collision2::COLLISION_OBJECT_PHYSICAL)
        if (csCOB->contactObjects.Contains (csCOA) == csArrayItemNotFound)
          csCOB->contactObjects.Push (csCOA);
    }
  }
}

void csBulletSector::UpdateCollisionPortals ()
{
  for (size_t i = 0; i < portals.GetSize (); i++)
  {
    csRefArray<csBulletCollisionObject> oldObjects (portals[i]->objects);
    csArray<csOrthoTransform> oldTrans (portals[i]->oldTrans);
    portals[i]->objects.Empty ();
    portals[i]->oldTrans.Empty ();
    for (int j = 0; j < portals[i]->ghostPortal->getNumOverlappingObjects (); j++)
    {
      btTransform tran = portals[i]->ghostPortal->getWorldTransform ();
      btCollisionObject* obj = portals[i]->ghostPortal->getOverlappingObject (j);
      CS::Collision2::iCollisionObject* csObj = 
        static_cast<CS::Collision2::iCollisionObject*> (obj->getUserPointer ());
      csBulletCollisionObject* csBulletObj = dynamic_cast<csBulletCollisionObject*> (csObj);
      csBulletCollisionObject* newObject;

      // Collide with static object?
      if (csBulletObj->GetObjectType () == CS::Collision2::COLLISION_OBJECT_BASE)
        continue;

      size_t index = oldObjects.Find (csBulletObj);
      if (index != csArrayItemNotFound)
      {

        // Check if it has traversed the portal
        bool mirror = false;
        csVector3 newPosition = csObj->GetTransform ().GetOrigin ();
        iSector* desSector = sector->FollowSegment (oldTrans[index],
          newPosition, mirror, true);

        if (desSector != sector)
        {
          // Move the body to the new sector.
          portals[i]->desSector->RemoveCollisionObject (csBulletObj->objectCopy);
          RemoveCollisionObject (csObj);
          portals[i]->desSector->AddCollisionObject (csObj);
          //TODO Set the transform...
          continue;
        }
        else
        {
          portals[i]->AddObject (csBulletObj);
          newObject = csBulletObj->objectCopy;
        }
      }
      // Create a new copy.
      else
      {
        portals[i]->AddObject (csBulletObj);
        if (csObj->GetObjectType () == CS::Collision2::COLLISION_OBJECT_PHYSICAL)
        {
          btVector3 localInertia (0.0f, 0.0f, 0.0f);
          CS::Physics2::iPhysicalBody* pb = csObj->QueryPhysicalBody ();
          if (pb->GetBodyType () == CS::Physics2::BODY_RIGID)
          {
            csRef<CS::Physics2::iRigidBody> nb = sys->CreateRigidBody ();
            csBulletRigidBody* newBody = dynamic_cast<csBulletRigidBody*> ((CS::Physics2::iRigidBody*)nb);
            csBulletRigidBody* rb = dynamic_cast<csBulletRigidBody*> (pb->QueryRigidBody ());
            
            for (int k = 0; k < rb->GetColliderCount (); k++)
              newBody->AddCollider (rb->GetCollider (k), rb->relaTransforms[k]);

            newBody->SetFriction (rb->friction);
            newBody->SetElasticity (rb->elasticity);
            newBody->SetLinearDampener (rb->GetLinearDampener ());
            newBody->SetRollingDampener (rb->GetRollingDampener ());
            
            newBody->SetState (rb->GetState ());
            newBody->RebuildObject ();
            portals[i]->desSector->AddRigidBody (newBody);

            newBody->SetCollisionGroup ("Portal");

            // Don't know if it's right. The desSector may don't have a floor in the position of this copy.
            newBody->btBody->setGravity (btVector3 (0.0,0.0,0.0));
            newObject = newBody;

            rb->objectCopy = newBody;
            newBody->objectOrigin = rb;
            csOrthoTransform trans;
            if (portals[i]->portal->GetFlags ().Check (CS_PORTAL_WARP))
            {
              csOrthoTransform trans;
              portals[i]->portal->ObjectToWorld (rb->GetTransform (), trans);
              newBody->portalWarp = CSToBullet (trans.GetInverse (), sys->getInternalScale ());
            }
          }
          else
          {
            //TODO Soft Body
          }
        }
        else if (csObj->GetObjectType () == CS::Collision2::COLLISION_OBJECT_GHOST
          || csObj->GetObjectType () == CS::Collision2::COLLISION_OBJECT_ACTOR)
        {
          csRef<CS::Collision2::iCollisionObject> co = sys->CreateCollisionObject ();
          newObject = dynamic_cast<csBulletCollisionObject*> ((CS::Collision2::iCollisionObject*)co);
          newObject->SetObjectType (CS::Collision2::COLLISION_OBJECT_GHOST, false);

          for (int k = 0; k < csBulletObj->GetColliderCount (); k++)
            newObject->AddCollider (csBulletObj->GetCollider (k), csBulletObj->relaTransforms[k]);

          newObject->RebuildObject ();
          portals[i]->desSector->AddCollisionObject (co);

          newObject->SetCollisionGroup ("Portal");

          csBulletObj->objectCopy = newObject;
          newObject->objectOrigin = csBulletObj;

          csOrthoTransform trans;
          if (portals[i]->portal->GetFlags ().Check (CS_PORTAL_WARP))
          {
            csOrthoTransform trans;
            portals[i]->portal->ObjectToWorld (csBulletObj->GetTransform (), trans);
            newObject->portalWarp = CSToBullet (trans.GetInverse (), sys->getInternalScale ());
          }
        }
      }
      // And set the transform and record old transforms.
      SetInformationToCopy (csBulletObj, newObject, portals[i]->portal);
      portals[i]->oldTrans.Push (csObj->GetTransform ());
    }

    // Remove the rest objects from portal list. 
    for (size_t j = 0; j < oldObjects.GetSize (); j ++)
    {
      size_t index = portals[i]->objects.Find (oldObjects[j]);
      if (index == csArrayItemNotFound)
        portals[i]->desSector->RemoveCollisionObject (oldObjects[j]->objectCopy );
    }
  }
}

void csBulletSector::SetInformationToCopy (csBulletCollisionObject* obj, 
                                           csBulletCollisionObject* cpy, 
                                           iPortal* portal)
{
  // TODO warp the transform.

  csOrthoTransform trans = obj->GetTransform ();
  if (portal->GetFlags ().Check (CS_PORTAL_WARP))
    trans *= portal->GetWarp ();

  btTransform btTrans = CSToBullet (trans, sys->getInternalScale ());

  if (obj->type == CS::Collision2::COLLISION_OBJECT_PHYSICAL)
  {
    CS::Physics2::iPhysicalBody* pb = obj->QueryPhysicalBody ();
    if (pb->GetBodyType () == CS::Physics2::BODY_RIGID)
    {
      csBulletRigidBody* btCopy = dynamic_cast<csBulletRigidBody*> (cpy->QueryPhysicalBody ()->QueryRigidBody ());
      csBulletRigidBody* rb = dynamic_cast<csBulletRigidBody*> (pb->QueryRigidBody ());

      btCopy->SetTransform (trans);

      if (rb->GetState () == CS::Physics2::STATE_DYNAMIC)
      {
        btCopy->btBody->setLinearVelocity (quatRotate (btTrans.getRotation (), rb->btBody->getLinearVelocity ()));
        btCopy->btBody->setAngularVelocity (quatRotate (btTrans.getRotation (), rb->btBody->getAngularVelocity ()));
      }
    }
    else
    {
      //TODO Soft Body
    }
  }
  else if (obj->type == CS::Collision2::COLLISION_OBJECT_GHOST
    || obj->type == CS::Collision2::COLLISION_OBJECT_ACTOR)
  {
    cpy->btObject->setWorldTransform (btTrans);
  }
}

void csBulletSector::GetInformationFromCopy (csBulletCollisionObject* obj, 
                                             csBulletCollisionObject* cpy, 
                                             float duration)
{
  // TODO Warp the velocity.
  if (obj->type == CS::Collision2::COLLISION_OBJECT_PHYSICAL)
  {
    CS::Physics2::iPhysicalBody* pb = obj->QueryPhysicalBody ();
    if (pb->GetBodyType () == CS::Physics2::BODY_RIGID)
    {
      csBulletRigidBody* btCopy = dynamic_cast<csBulletRigidBody*> (cpy->QueryPhysicalBody ()->QueryRigidBody ());
      csBulletRigidBody* rb = dynamic_cast<csBulletRigidBody*> (pb->QueryRigidBody ());
      if (rb->GetState () == CS::Physics2::STATE_DYNAMIC)
      {
        btQuaternion qua = cpy->portalWarp.getRotation ();

        rb->btBody->internalGetDeltaLinearVelocity ()= quatRotate (qua, btCopy->btBody->internalGetDeltaLinearVelocity ());
        rb->btBody->internalGetDeltaAngularVelocity ()= quatRotate (qua, btCopy->btBody->internalGetDeltaAngularVelocity ());
        rb->btBody->internalGetPushVelocity ()= quatRotate (qua, btCopy->btBody->internalGetPushVelocity ());
        rb->btBody->internalGetTurnVelocity ()= quatRotate (qua, btCopy->btBody->internalGetTurnVelocity ());
        // I don't know if there are any other parameters exist.
        rb->btBody->internalWritebackVelocity (duration);
      }
      else if (rb->GetState () == CS::Physics2::STATE_KINEMATIC)
      {
        //Nothing to do?
      }
    }
    else
    {
      //TODO Soft Body
    }
  }
  else if (obj->type == CS::Collision2::COLLISION_OBJECT_GHOST
    || obj->type == CS::Collision2::COLLISION_OBJECT_ACTOR)
  {
    //btPairCachingGhostObject* ghostCopy = btPairCachingGhostObject::upcast (cpy);
    //btPairCachingGhostObject* ghostObject = btPairCachingGhostObject::upcast (obj->btObject);

    // Need to think about the implementation of actor.
  }
}

SCF_IMPLEMENT_FACTORY (csBulletSystem)

csBulletSystem::csBulletSystem (iBase* iParent)
  : scfImplementationType (this, iParent), internalScale (1.0f), inverseInternalScale (1.0f)
{
  defaultInfo = new btSoftBodyWorldInfo;
}

csBulletSystem::~csBulletSystem ()
{
  collSectors.DeleteAll ();
  //objects.DeleteAll ();
  //colliders.DeleteAll ();
}

void csBulletSystem::SetInternalScale (float scale)
{
  // update parameters
  internalScale = scale;
  inverseInternalScale = 1.0f / scale;
}

bool csBulletSystem::Initialize (iObjectRegistry* object_reg)
{
  this->object_reg = object_reg;
  csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet> (
    object_reg, "crystalspace.shared.stringset");
  baseID = strings->Request ("base");
  colldetID = strings->Request ("colldet");
  return true;
}

csRef<CS::Collision2::iColliderConvexMesh> csBulletSystem::CreateColliderConvexMesh (iMeshWrapper* mesh, bool simplify)
{
  csRef<csBulletColliderConvexMesh> collider;
  collider.AttachNew (new csBulletColliderConvexMesh (mesh, this, simplify));

  //colliders.Push (collider);
  return (CS::Collision2::iColliderConvexMesh*)collider;
}

csRef<CS::Collision2::iColliderConcaveMesh> csBulletSystem::CreateColliderConcaveMesh (iMeshWrapper* mesh)
{
  csRef<csBulletColliderConcaveMesh> collider;
  collider.AttachNew (new csBulletColliderConcaveMesh (mesh,this));

  //colliders.Push (collider);
  return collider;
}

csRef<CS::Collision2::iColliderConcaveMeshScaled> csBulletSystem::CreateColliderConcaveMeshScaled (
  CS::Collision2::iColliderConcaveMesh* collider, csVector3 scale)
{
  csRef<csBulletColliderConcaveMeshScaled> coll;
  coll.AttachNew (new csBulletColliderConcaveMeshScaled (collider, scale,this));

  //colliders.Push (coll);
  return coll;
}

csRef<CS::Collision2::iColliderCylinder> csBulletSystem::CreateColliderCylinder (float length, float radius)
{
  csRef<csBulletColliderCylinder> collider;
  collider.AttachNew (new csBulletColliderCylinder (length, radius, this));

  //colliders.Push (collider);
  return collider;
}

csRef<CS::Collision2::iColliderBox> csBulletSystem::CreateColliderBox (const csVector3& size)
{
  csRef<csBulletColliderBox> collider;
  collider.AttachNew (new csBulletColliderBox (size, this));

  //colliders.Push (collider);
  return collider;
} 

csRef<CS::Collision2::iColliderSphere> csBulletSystem::CreateColliderSphere (float radius)
{
  csRef<csBulletColliderSphere> collider;
  collider.AttachNew (new csBulletColliderSphere (radius, this));

  //colliders.Push (collider);
  return collider;
}

csRef<CS::Collision2::iColliderCapsule> csBulletSystem::CreateColliderCapsule (float length, float radius)
{
  csRef<csBulletColliderCapsule> collider;
  collider.AttachNew (new csBulletColliderCapsule (length, radius, this));
  
  //colliders.Push (collider);
  return collider;
}

csRef<CS::Collision2::iColliderCone> csBulletSystem::CreateColliderCone (float length, float radius)
{
  csRef<csBulletColliderCone> collider;
  collider.AttachNew (new csBulletColliderCone (length, radius, this));

  //colliders.Push (collider);
  return collider;
}

csRef<CS::Collision2::iColliderPlane> csBulletSystem::CreateColliderPlane (const csPlane3& plane)
{
  csRef<csBulletColliderPlane> collider;
  collider.AttachNew (new csBulletColliderPlane (plane, this));

  //colliders.Push (collider);
  return collider;
}

csRef<CS::Collision2::iColliderTerrain> csBulletSystem::CreateColliderTerrain (iTerrainSystem* terrain, 
                                                               float minHeight /* = 0 */, 
                                                               float maxHeight /* = 0 */)
{
  csRef<csBulletColliderTerrain> collider;
  collider.AttachNew (new csBulletColliderTerrain (terrain, minHeight, maxHeight, this));

  //colliders.Push (collider);
  return collider;
}

csRef<CS::Collision2::iCollisionObject> csBulletSystem::CreateCollisionObject ()
{
  csRef<csBulletCollisionObject> collObject;
  collObject.AttachNew (new csBulletCollisionObject (this));

  //objects.Push (collObject);
  return collObject;
}

csRef<CS::Collision2::iCollisionActor> csBulletSystem::CreateCollisionActor ()
{
  csRef<CS::Collision2::iCollisionActor> collActor;
  collActor.AttachNew (new csBulletCollisionActor (this));

  return collActor;
}
csRef<CS::Collision2::iCollisionSector> csBulletSystem::CreateCollisionSector ()
{
  csRef<csBulletSector> collSector;
  collSector.AttachNew (new csBulletSector (this));

  collSectors.Push (collSector);
  return collSector;
}

void csBulletSystem::DecomposeConcaveMesh (CS::Collision2::iCollisionObject* object, iMeshWrapper* mesh, bool simplify)
{
  csBulletCollisionObject* btCollObject = dynamic_cast<csBulletCollisionObject*> (object);

  class MyConvexDecomposition : public ConvexDecomposition::ConvexDecompInterface
  {
    float scale;
    btVector3 centroid;
    bool simp;
  public:
    btAlignedObjectArray<btConvexHullShape*> m_convexShapes;
    btAlignedObjectArray<btVector3> m_convexCentroids;
    btAlignedObjectArray<float> m_convexVolume;

    MyConvexDecomposition (float scale, bool simplify)
      :scale (scale),
      mBaseCount (0),
      mHullCount (0),
      simp (simplify)
    {
    }

    virtual void ConvexDecompResult(ConvexDecomposition::ConvexResult &result)
    {
      btVector3 localScaling(scale, scale, scale);

      //calc centroid, to shift vertices around center of mass
      centroid.setValue(0,0,0);

      btAlignedObjectArray<btVector3> vertices;
      //const unsigned int *src = result.mHullIndices;
      for (unsigned int i=0; i<result.mHullVcount; i++)
      {
        btVector3 vertex(result.mHullVertices[i*3],result.mHullVertices[i*3+1],result.mHullVertices[i*3+2]);
        vertex *= localScaling;
        centroid += vertex;

      }

      centroid *= 1.f/(float(result.mHullVcount) );

      if (simp)
        for (unsigned int i=0; i<result.mHullVcount; i++)
        {
          btVector3 vertex(result.mHullVertices[i*3],result.mHullVertices[i*3+1],result.mHullVertices[i*3+2]);
          vertex *= localScaling;
          vertex -= centroid;
          vertices.push_back(vertex);
        }
      else
      {
        const unsigned int *src = result.mHullIndices;
        for (unsigned int i=0; i<result.mHullTcount; i++)
        {
          unsigned int index0 = *src++;
          unsigned int index1 = *src++;
          unsigned int index2 = *src++;

          btVector3 vertex0(result.mHullVertices[index0*3], result.mHullVertices[index0*3+1],result.mHullVertices[index0*3+2]);
          btVector3 vertex1(result.mHullVertices[index1*3], result.mHullVertices[index1*3+1],result.mHullVertices[index1*3+2]);
          btVector3 vertex2(result.mHullVertices[index2*3], result.mHullVertices[index2*3+1],result.mHullVertices[index2*3+2]);
          vertex0 *= localScaling;
          vertex1 *= localScaling;
          vertex2 *= localScaling;

          vertex0 -= centroid;
          vertex1 -= centroid;
          vertex2 -= centroid;

          //TODO this will add duplicate vertices to convex shape. But the debug draw result is right now.
          vertices.push_back(vertex0);
          vertices.push_back(vertex1);
          vertices.push_back(vertex2);

          index0+=mBaseCount;
          index1+=mBaseCount;
          index2+=mBaseCount;
        }
      }

      btConvexHullShape* convexShape = new btConvexHullShape(&(vertices[0].getX()),vertices.size());
      convexShape->setMargin(0.01f);
      m_convexShapes.push_back(convexShape);
      m_convexCentroids.push_back(centroid);
      m_convexVolume.push_back(result.mHullVolume);
      mBaseCount+=result.mHullVcount; // advance the 'base index' counter.
    }

    int   mBaseCount;
    int		mHullCount;
  };

  csRef<iTriangleMesh> triMesh = FindColdetTriangleMesh (mesh, baseID, colldetID);
  if (! triMesh)
    return;

  unsigned int depth = 5;
  float cpercent     = 5;
  float ppercent     = 15;
  unsigned int maxv  = 16;
  float skinWidth    = 0.0;

  csTriangle *c_triangle = triMesh->GetTriangles ();
  csVector3 *c_vertex = triMesh->GetVertices ();

  ConvexDecomposition::DecompDesc desc;
  desc.mVcount       = triMesh->GetVertexCount ();
  desc.mVertices     = (float*)c_vertex;
  desc.mTcount       = triMesh->GetTriangleCount ();
  desc.mIndices      = (unsigned int *)c_triangle;
  desc.mDepth        = depth;
  desc.mCpercent     = cpercent;
  desc.mPpercent     = ppercent;
  desc.mMaxVertices  = maxv;
  desc.mSkinWidth    = skinWidth;

  MyConvexDecomposition	convexDecomposition(internalScale, simplify);
  desc.mCallback = &convexDecomposition;

  ConvexBuilder cb(desc.mCallback);
  cb.process(desc);

  btTransform trans;
  trans.setIdentity();
  csOrthoTransform relaTransform;
  for (int i=0;i<convexDecomposition.m_convexShapes.size();i++)
  {

    btVector3 centroid = convexDecomposition.m_convexCentroids[i];
    trans.setOrigin(centroid);
    btConvexHullShape* convexShape = convexDecomposition.m_convexShapes[i];
    csRef<csBulletCollider> collider;
    collider.AttachNew (new csBulletColliderConvexMesh (convexShape, convexDecomposition.m_convexVolume[i], this));
    //colliders.Push (collider);
    relaTransform = BulletToCS (trans, inverseInternalScale);
    btCollObject->AddCollider (collider, relaTransform);
  }

  //RebuildObject? or let user do it?
}

csRef<CS::Physics2::iRigidBody> csBulletSystem::CreateRigidBody ()
{
  csRef<csBulletRigidBody> body;
  body.AttachNew (new csBulletRigidBody (this));

  //rigidBodies.Push (body);
  return body;
}

csRef<CS::Physics2::iJoint> csBulletSystem::CreateJoint ()
{
  csRef<csBulletJoint> joint;
  joint.AttachNew (new csBulletJoint (this));
  //joints.Push (joint);
  return joint;  
}

csRef<CS::Physics2::iJoint> csBulletSystem::CreateRigidP2PJoint (const csVector3 position)
{
  csRef<csBulletJoint> joint;
  joint.AttachNew (new csBulletJoint (this));
  joint->SetTransConstraints (true, true, true);
  csVector3 trans (0.0f,0.0f,0.0f);
  joint->SetMaximumDistance (trans);
  joint->SetMinimumDistance (trans);
  joint->SetPosition (position);
  joint->SetType (RIGID_P2P_JOINT);
  //joints.Push (joint);
  return joint;
}

csRef<CS::Physics2::iJoint> csBulletSystem::CreateRigidSlideJoint (const csOrthoTransform trans,
                                                    float minDist, float maxDist, 
                                                    float minAngle, float maxAngle, int axis)
{
  csRef<csBulletJoint> joint;
  if (axis < 0 || axis > 2)
    return joint;
  joint.AttachNew (new csBulletJoint (this));
  joint->SetTransConstraints (true, true, true);
  joint->SetRotConstraints (true, true, true);
  csVector3 minDistant (0.0f, 0.0f, 0.0f);
  csVector3 maxDistant (0.0f, 0.0f, 0.0f);

  minDistant[axis] = minDist;
  maxDistant[axis] = maxDist;
  joint->SetMinimumDistance (minDistant);
  joint->SetMaximumDistance (maxDistant);
  minDistant[axis] = minAngle;
  maxDistant[axis] = maxAngle;
  joint->SetMinimumAngle (minDistant);
  joint->SetMaximumAngle (maxDistant);
  joint->SetTransform (trans);
  joint->SetType (RIGID_SLIDE_JOINT);
  //joints.Push (joint);
  return joint;
}

csRef<CS::Physics2::iJoint> csBulletSystem::CreateRigidHingeJoint (const csVector3 position, 
                                                     float minAngle, float maxAngle, int axis)
{
  csRef<csBulletJoint> joint;
  if (axis < 0 || axis > 2)
    return joint;
  joint.AttachNew (new csBulletJoint (this));
  csVector3 minDistant (0.0f, 0.0f, 0.0f);
  csVector3 maxDistant (0.0f, 0.0f, 0.0f);
  minDistant[axis] = minAngle;
  maxDistant[axis] = maxAngle;
  joint->SetMinimumAngle (minDistant);
  joint->SetMaximumAngle (maxDistant);
  joint->SetPosition (position);  
  joint->SetType (RIGID_HINGE_JOINT);
  joint->axis = axis;
  //joints.Push (joint);
  return joint;
}

csRef<CS::Physics2::iJoint> csBulletSystem::CreateSoftLinearJoint (const csVector3 position)
{
  csRef<csBulletJoint> joint;
  joint.AttachNew (new csBulletJoint (this));
  joint->SetPosition (position);
  joint->SetType (SOFT_LINEAR_JOINT);
  //joints.Push (joint);
  return joint;
}

csRef<CS::Physics2::iJoint> csBulletSystem::CreateSoftAngularJoint (int axis)
{
  csRef<csBulletJoint> joint;
  if (axis < 0 || axis > 2)
    return joint;
  joint.AttachNew (new csBulletJoint (this));
  if (axis == 0)
    joint->SetRotConstraints (false, true, true);
  else if (axis == 1)
    joint->SetRotConstraints (true, false, false);
  else if (axis == 2)
    joint->SetRotConstraints (false, false, true);

  joint->SetType (SOFT_ANGULAR_JOINT);
  //joints.Push (joint);
  return joint;
}

csRef<CS::Physics2::iJoint> csBulletSystem::CreateRigidPivotJoint (iRigidBody* body, const csVector3 position)
{
  csRef<csBulletJoint> joint;
  joint.AttachNew (new csBulletJoint (this));
  joint->SetTransConstraints (true, true, true);
  csVector3 trans (0.0f,0.0f,0.0f);
  joint->SetMaximumDistance (trans);
  joint->SetMinimumDistance (trans);
  joint->SetPosition (position);
  joint->SetType (RIGID_PIVOT_JOINT);
  joint->Attach (body, NULL);
  //joints.Push (joint);
  return joint;
}

csRef<CS::Physics2::iSoftBody> csBulletSystem::CreateRope (csVector3 start,
                                             csVector3 end,
                                             size_t segmentCount)
{
  //Don't know the soft world info currently. So just set it to NULL
  
  btSoftBody* body = btSoftBodyHelpers::CreateRope
    (*defaultInfo, CSToBullet (start, internalScale),
    CSToBullet (end, internalScale), segmentCount - 1, 0);

  //hard-coded parameters for hair ropes
  body->m_cfg.kDP = 0.08f; // no elasticity
  body->m_cfg.piterations = 16; // no white zone
  body->m_cfg.timescale = 2;

  csRef<csBulletSoftBody> csBody;
  csBody.AttachNew (new csBulletSoftBody (this, body));
  //softBodies.Push (csBody);
  return csRef<iSoftBody>(csBody->QuerySoftBody());
}

csRef<CS::Physics2::iSoftBody> csBulletSystem::CreateRope (csVector3* vertices, size_t vertexCount)
{
  // Create the nodes
  CS_ALLOC_STACK_ARRAY(btVector3, nodes, vertexCount);
  CS_ALLOC_STACK_ARRAY(btScalar, materials, vertexCount);

  for (size_t i = 0; i < vertexCount; i++)
  {
    nodes[i] = CSToBullet (vertices[i], internalScale);
    materials[i] = 1;
  }

  btSoftBody* body = new btSoftBody(NULL, vertexCount, &nodes[0], &materials[0]);

  // Create the links between the nodes
  for (size_t i = 1; i < vertexCount; i++)
    body->appendLink (i - 1, i);

  //hard-coded parameters for hair ropes
  body->m_cfg.kDP = 0.08f; // no elasticity
  body->m_cfg.piterations = 16; // no white zone
  body->m_cfg.timescale = 2;

  csRef<csBulletSoftBody> csBody;
  csBody.AttachNew (new csBulletSoftBody (this, body));

  //softBodies.Push (csBody);
  return csBody; 
}

csRef<CS::Physics2::iSoftBody> csBulletSystem::CreateCloth (csVector3 corner1, csVector3 corner2,
                                              csVector3 corner3, csVector3 corner4,
                                              size_t segmentCount1, size_t segmentCount2,
                                              bool withDiagonals /* = false */)
{
  btSoftBody* body = btSoftBodyHelpers::CreatePatch
    (*defaultInfo, CSToBullet (corner1, internalScale),
    CSToBullet (corner2, internalScale), CSToBullet (corner3, internalScale),
    CSToBullet (corner4, internalScale), segmentCount1, segmentCount2, 0,
    withDiagonals);
  body->m_cfg.collisions |= btSoftBody::fCollision::VF_SS;

  csRef<csBulletSoftBody> csBody;
  csBody.AttachNew (new csBulletSoftBody (this, body));

  //softBodies.Push (csBody);
  return csBody;
}

csRef<CS::Physics2::iSoftBody> csBulletSystem::CreateSoftBody (iGeneralFactoryState* genmeshFactory,
                                                 const csOrthoTransform& bodyTransform)
{
  btScalar* vertices = new btScalar[genmeshFactory->GetVertexCount () * 3];
  for (int i = 0; i < genmeshFactory->GetVertexCount (); i++)
  {
    csVector3 vertex = genmeshFactory->GetVertices ()[i]
    * bodyTransform.GetInverse() * internalScale;
    vertices[i * 3] = vertex[0];
    vertices[i * 3 + 1] = vertex[1];
    vertices[i * 3 + 2] = vertex[2];
  }

  int* triangles = new int[genmeshFactory->GetTriangleCount () * 3];
  for (int i = 0; i < genmeshFactory->GetTriangleCount (); i++)
  {
    csTriangle& triangle = genmeshFactory->GetTriangles ()[i];
    triangles[i * 3] = triangle.a;
    triangles[i * 3 + 1] = triangle.b;
    triangles[i * 3 + 2] = triangle.c;
  }

  btSoftBody* body = btSoftBodyHelpers::CreateFromTriMesh
    (*defaultInfo, vertices, triangles, genmeshFactory->GetTriangleCount (),
    false);

  body->m_cfg.piterations = 10;
  body->m_cfg.collisions |=	btSoftBody::fCollision::SDF_RS;
  body->m_cfg.collisions |= btSoftBody::fCollision::VF_SS;
  body->m_materials[0]->m_kLST = 1;

  csRef<csBulletSoftBody> csBody;
  csBody.AttachNew (new csBulletSoftBody (this, body));

  //softBodies.Push (csBody);
  return csBody;
}

csRef<CS::Physics2::iSoftBody> csBulletSystem::CreateSoftBody (csVector3* vertices, size_t vertexCount,
                                                 csTriangle* triangles, size_t triangleCount,
                                                 const csOrthoTransform& bodyTransform)
{
  btScalar* btVertices = new btScalar[vertexCount * 3];
  for (size_t i = 0; i < vertexCount; i++)
  {
    csVector3 vertex = vertices[i]
    * bodyTransform.GetInverse() * internalScale;
    btVertices[i * 3] = vertex[0];
    btVertices[i * 3 + 1] = vertex[1];
    btVertices[i * 3 + 2] = vertex[2];
  }

  int* btTriangles = new int[triangleCount * 3];
  for (size_t i = 0; i < triangleCount; i++)
  {
    csTriangle& triangle = triangles[i];
    btTriangles[i * 3] = triangle.a;
    btTriangles[i * 3 + 1] = triangle.b;
    btTriangles[i * 3 + 2] = triangle.c;
  }

  btSoftBody* body = btSoftBodyHelpers::CreateFromTriMesh
    (*defaultInfo, btVertices, btTriangles, triangleCount, false);

  body->m_cfg.piterations = 10;
  body->m_cfg.collisions |=	btSoftBody::fCollision::SDF_RS;
  body->m_cfg.collisions |= btSoftBody::fCollision::VF_SS;
  body->m_materials[0]->m_kLST = 1;

  delete [] btVertices;
  delete [] btTriangles;

  csRef<csBulletSoftBody> csBody;
  csBody.AttachNew (new csBulletSoftBody (this, body));

  //softBodies.Push (csBody);
  return csBody;
}
}
CS_PLUGIN_NAMESPACE_END(Bullet2)
