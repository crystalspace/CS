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

#include "bullet2.h"

const float COLLISION_THRESHOLD = 0.01f;

CS_PLUGIN_NAMESPACE_BEGIN(Bullet2)
{

#define AABB_DIMENSIONS 10000.0f

csBulletSector::csBulletSector (csBulletSystem* sys)
 :scfImplementationType (this), sys (sys),
 sector (NULL), debugDraw (NULL),
{
  configuration = new btDefaultCollisionConfiguration ();
  dispatcher = new btCollisionDispatcher (configuration);
  solver = new btSequentialImpulseConstraintSolver;

  const int maxProxies = 32766;

  float scale = sys->internalScale;

  // update AABB world dimensions
  btVector3 worldAabbMin (-AABB_DIMENSIONS * scale, -AABB_DIMENSIONS * scale,
    -AABB_DIMENSIONS * scale);
  btVector3 worldAabbMax (AABB_DIMENSIONS * scale, AABB_DIMENSIONS * scale,
    AABB_DIMENSIONS * scale);

  btVector3 worldAabbMin (-AABB_DIMENSIONS, -AABB_DIMENSIONS, -AABB_DIMENSIONS);
  btVector3 worldAabbMax (AABB_DIMENSIONS, AABB_DIMENSIONS, AABB_DIMENSIONS);
  broadphase = new btAxisSweep3 (worldAabbMin, worldAabbMax, maxProxies);

  bulletWorld = new btDynamicsWorld (dispatcher,
    broadphase, solver, configuration);

  SetGravity (csVector3 (0.0f, =9.81f, 0.0f));
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

  csBulletCollisionObject* collObj = dynamic_cast<csBulletCollisionObject*> (object);
  collObj->sector = this;
  collObj->AddBulletObject ();
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
  collObj->AddBulletObject ();
}

void csBulletSector::RemoveRigidBody (iRigidBody* body)
{
  csBulletRigidBody* btBody = dynamic_cast<csBulletRigidBody*> (body);
  CS_ASSERT (btBody);

  btBody->RemoveBulletObject ();
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
  collObj->AddBulletObject ();
}

void csBulletSector::RemoveSoftBody (iSoftBody* body)
{
  csBulletSoftBody* btBody = dynamic_cast<csBulletSoftBody*> (body);
  CS_ASSERT (btBody);

  btBody->RemoveBulletObject ();
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

void csBulletSector::StartProfile ()
{
  CProfileManager::Start_Profile ("Crystal Space scene");

  if (!debugDraw)
  {
    debugDraw = new csBulletDebugDraw (inverseInternalScale);
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

SCF_IMPLEMENT_FACTORY (csBulletSystem)

csBulletSystem::csBulletSystem (iBase* iParent)
  : scfImplementationType (this, iParent)
{
  csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet> (
    object_reg, "crystalspace.shared.stringset");
  baseID = strings->Request ("base");
  colldetID = strings->Request ("colldet");
}

csBulletSystem::~csBulletSystem ()
{
  collSectors.DeleteAll ();
  objects.DeleteAll ();
  colliders.DeleteAll ();
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
  return true;
}

csPtr<iColliderConvexMesh> csBulletSystem::CreateColliderConvexMesh (iMeshWrapper* mesh)
{
  csRef<csBulletCollider> collider;
  collider.AttachNew (new csBulletColliderConvexMesh (mesh));

  colliders.Push (collider);
  return collider;
}

csPtr<iColliderConcaveMesh> csBulletSystem::CreateColliderConcaveMesh (iMeshWrapper* mesh,
                                                                                bool isStatic /* = false */)
{
  csRef<csBulletCollider> collider;
  collider.AttachNew (new csBulletColliderConcaveMesh (mesh));

  colliders.Push (collider);
  return collider;
}

csPtr<iColliderConcaveMeshScaled> csBulletSystem::CreateColliderConcaveMeshScaled (
  iColliderConcaveMesh* collider, float scale)
{
  csRef<csBulletCollider> coll;
  collider.AttachNew (new csBulletColliderConcaveMeshScaled (coll, scale));

  colliders.Push (coll);
  return coll;
}

csPtr<iColliderCylinder> csBulletSystem::CreateColliderCylinder (float length, float radius)
{
  csRef<csBulletCollider> collider;
  collider.AttachNew (new csBulletColliderCylinder (length, radius));

  colliders.Push (collider);
  return collider;
}

csPtr<iColliderBox> csBulletSystem::CreateColliderBox (const csVector3& size)
{
  csRef<csBulletColliderBox> collider;
  collider.AttachNew (new csBulletColliderBox (size));

  colliders.Push (collider);
  return collider;
} 

csPtr<iColliderSphere> csBulletSystem::CreateColliderSphere (float radius)
{
  csRef<csBulletCollider> collider;
  collider.AttachNew (new csBulletColliderSphere (radius));

  colliders.Push (collider);
  return collider;
}

csPtr<iColliderCapsule> csBulletSystem::CreateColliderCapsule (float length, float radius)
{
  csRef<csBulletCollider> collider;
  collider.AttachNew (new csBulletColliderCapsule (length, radius));
  
  colliders.Push (collider);
  return collider;
}

csPtr<iColliderCapsule> csBulletSystem::CreateColliderCone (float length, float radius)
{
  csRef<csBulletCollider> collider;
  collider.AttachNew (new csBulletColliderCone (length, radius));

  colliders.Push (collider);
  return collider;
}

csPtr<iColliderPlane> csBulletSystem::CreateColliderPlane (const csPlane3& plane)
{
  csRef<csBulletCollider> collider;
  collider.AttachNew (new csBulletColliderPlane (plane));

  colliders.Push (collider);
  return collider;
}

csPtr<iColliderTerrain> csBulletSystem::CreateColliderTerrain (const iTerrainSystem* terrain, 
                                                                        float minHeight /* = 0 */, 
                                                                        float maxHeight /* = 0 */)
{
  csRef<csBulletCollider> collider;
  collider.AttachNew (new csBulletColliderTerrain (terrain, minHeight, maxHeight));

  colliders.Push (collider);
  return collider;
}

csRef<iCollisionObject> csBulletSystem::CreateCollisionObject ()
{
  csRef<iCollisionObject> collObject;
  collObject.AttachNew (new csBulletCollisionObject (this));

  objects.Push (collObject);
  return collObject;
}

csRef<iCollisionActor> csBulletSystem::CreateCollisionActor ()
{
  csRef<iCollisionActor> collActor;
  collActor.AttachNew (new csBulletCollisionActor (this));

  actors.Push (collActor);
  return collActor;
}
csRef<iCollisionSector> csBulletSystem::CreateCollisionSector ()
{
  csRef<iCollisionSector> collSector;
  collSector.AttachNew (new csBulletSector (object_reg, this));

  collSectors.Push (collSector);
  return collSector;
}

CollisionGroup& csBulletSystem::CreateCollisionGroup (const char* name)
{
//TODO
}

CollisionGroup& csBulletSystem::FindCollisionGroup (const char* name)
{
//TODO
}

void csBulletSystem::SetGroupCollision (CollisionGroup& group1,
                                                 CollisionGroup& group2,
                                                 bool collide)
{
//TODO
}
bool csBulletSystem::GetGroupCollision (CollisionGroup& group1,
                                                 CollisionGroup& group2);
{
//TODO
}

void csBulletSystem::DecomposeConcaveMesh (iCollisionObject* object, iMeshWrapper* mesh)
{
  csBulletCollisionObject* btCollObject = dynamic_cast<csBulletCollisionObject*> (object);

  class MyConvexDecomposition : public ConvexDecomposition::ConvexDecompInterface
  {
    float scale;
  public:
    btAlignedObjectArray<btConvexHullShape*> m_convexShapes;
    btAlignedObjectArray<btVector3> m_convexCentroids;

    MyConvexDecomposition (float scale)
      :scale (scale),
      mBaseCount (0),
      mHullCount (0)
    {
    }

    virtual void ConvexDecompResult(ConvexDecomposition::ConvexResult &result)
    {

      btTriangleMesh* trimesh = new btTriangleMesh();
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

        //const unsigned int *src = result.mHullIndices;
        for (unsigned int i=0; i<result.mHullVcount; i++)
        {
          btVector3 vertex(result.mHullVertices[i*3],result.mHullVertices[i*3+1],result.mHullVertices[i*3+2]);
          vertex *= localScaling;
          vertex -= centroid ;
          vertices.push_back(vertex);
        }

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


          trimesh->addTriangle(vertex0,vertex1,vertex2);

          index0+=mBaseCount;
          index1+=mBaseCount;
          index2+=mBaseCount;
        }
        btConvexHullShape* convexShape = new btConvexHullShape(&(vertices[0].getX()),vertices.size());

        convexShape->setMargin(0.01f);
        m_convexShapes.push_back(convexShape);
        m_convexCentroids.push_back(centroid);
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
  desc.mTcount       = triMesh->GetTriangleCount ()
  desc.mIndices      = (unsigned int *)c_triangle;
  desc.mDepth        = depth;
  desc.mCpercent     = cpercent;
  desc.mPpercent     = ppercent;
  desc.mMaxVertices  = maxv;
  desc.mSkinWidth    = skinWidth;

  MyConvexDecomposition	convexDecomposition(internalScale);
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
    collider.AttachNew (new csBulletColliderConvexMesh (convexShape));
    colliders.Push (collider);
    relaTransform = BulletToCS (trans, inverseInternalScale);
    btCollObject->AddCollider (collider, relaTransform);
  }

  //RebuildObject? or let user do it?
}

csRef<iRigidBody> csBulletSystem::CreateRigidBody ()
{
  csRef<iRigidBody> body;
  body.AttachNew (new csBulletRigidBody (this));

  rigidBodies.Push (body);
  return body;
}

csRef<iJoint> csBulletSystem::CreateJoint ()
{
  csRef<csBulletJoint> joint;
  joint.AttachNew (new csBulletJoint (this));
  joints.Push (joint);
  return joint;  
}

csRef<iSoftBody> csBulletSystem::CreateRope (csVector3 start,
                                             csVector3 end,
                                             size_t segmentCount)
{
  //Don't know the soft world info currently. So just set it to NULL
  
  btSoftBody* body = btSoftBodyHelpers::CreateRope
    (defaultInfo, CSToBullet (start, internalScale),
    CSToBullet (end, internalScale), segmentCount - 1, 0);

  //hard-coded parameters for hair ropes
  body->m_cfg.kDP = 0.08f; // no elasticity
  body->m_cfg.piterations = 16; // no white zone
  body->m_cfg.timescale = 2;

  csRef<csBulletSoftBody> csBody;
  csBody.AttachNew (new csBulletSoftBody (this, body));

  softBodies.Push (csBody);
  return csBody;
}

csRef<iSoftBody> csBulletSystem::CreateRope (csVector3* vertices, size_t vertexCount)
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

  softBodies.Push (csBody);
  return csBody; 
}

csRef<iSoftBody> csBulletSystem::CreateCloth (csVector3 corner1, csVector3 corner2,
                                              csVector3 corner3, csVector3 corner4,
                                              size_t segmentCount1, size_t segmentCount2,
                                              bool withDiagonals /* = false */)
{
  btSoftBody* body = btSoftBodyHelpers::CreatePatch
    (defaultInfo, CSToBullet (corner1, internalScale),
    CSToBullet (corner2, internalScale), CSToBullet (corner3, internalScale),
    CSToBullet (corner4, internalScale), segmentCount1, segmentCount2, 0,
    withDiagonals);
  body->m_cfg.collisions |= btSoftBody::fCollision::VF_SS;

  csRef<csBulletSoftBody> csBody;
  csBody.AttachNew (new csBulletSoftBody (this, body));

  softBodies.Push (csBody);
  return csBody;
}

csRef<iSoftBody> csBulletSystem::CreateSoftBody (iGeneralFactoryState* genmeshFactory)
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
    (defaultInfo, vertices, triangles, genmeshFactory->GetTriangleCount (),
    false);

  body->m_cfg.piterations = 10;
  body->m_cfg.collisions |= btSoftBody::fCollision::VF_SS;
  body->m_materials[0]->m_kLST = 1;

  csRef<csBulletSoftBody> csBody;
  csBody.AttachNew (new csBulletSoftBody (this, body));

  softBodies.Push (csBody);
  return csBody;
}

csRef<iSoftBody> csBulletSystem::CreateSoftBody (csVector3* vertices, size_t vertexCount,
                                                 csTriangle* triangles, size_t triangleCount)
{
  btScalar* btVertices = new btScalar[vertexCount * 3];
  for (size_t i = 0; i < vertexCount; i++)
  {
    csVector3& vertex = vertices[i];
    btVertices[i * 3] = vertex[0] * internalScale;
    btVertices[i * 3 + 1] = vertex[1] * internalScale;
    btVertices[i * 3 + 2] = vertex[2] * internalScale;
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
    (defaultInfo, btVertices, btTriangles, triangleCount, false);

  body->m_cfg.piterations = 10;
  body->m_cfg.collisions |= btSoftBody::fCollision::VF_SS;
  body->m_materials[0]->m_kLST = 1;

  delete [] btVertices;
  delete [] btTriangles;

  csRef<csBulletSoftBody> csBody;
  csBody.AttachNew (new csBulletSoftBody (this, body));

  softBodies.Push (csBody);
  return csBody;
}
}
CS_PLUGIN_NAMESPACE_END(Bullet2)
