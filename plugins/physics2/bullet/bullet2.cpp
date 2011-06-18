#include "cssysdef.h"
#include "csgeom/sphere.h"
#include "csgeom/tri.h"
#include "imesh/genmesh.h"
#include "imesh/object.h"
#include "csutil/sysfunc.h"
#include "iutil/objreg.h"
#include "ivaria/view.h"
#include "ivaria/collision2.h"
#include "igeom/trimesh.h"

#include "csutil/custom_new_disable.h"

#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"
#include "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"
#include "BulletSoftBody/btSoftBody.h"
#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"
#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "BulletSoftBody/btSoftBodyHelpers.h"
#include "ConvexBuilder.h"

#include "csutil/custom_new_enable.h"

#include "bullet2.h"
#include "common2.h"
#include "colliders2.h"
#include "rigidbody2.h"
#include "softbody2.h"
#include "joint2.h"

const float COLLISION_THRESHOLD = 0.01f;
#define AABB_DIMENSIONS 10000.0f

CS_PLUGIN_NAMESPACE_BEGIN(Bullet2)
{

struct PointContactResult : public btCollisionWorld::ContactResultCallback
{
  csArray<CollisionData>& colls;
  csBulletSystem* sys;
  PointContactResult(csBulletSystem* sys, csArray<CollisionData>& collisions) : colls(collisions), sys(sys) 
  {
  }
  virtual	btScalar	addSingleResult (btManifoldPoint& cp,	const btCollisionObject* colObj0, int partId0,int index0,const btCollisionObject* colObj1,int partId1,int index1)
  {
    CollisionData data;
    data.penetration = cp.m_distance1 * sys->getInternalScale ();
    data.positionWorldOnA = BulletToCS (cp.m_positionWorldOnA, sys->getInternalScale ());
    data.positionWorldOnB = BulletToCS (cp.m_positionWorldOnB, sys->getInternalScale ());
    colls.Push (data);
    return 0;
  }
};

csBulletSector::csBulletSector (csBulletSystem* sys)
 :scfImplementationType (this), sys (sys),
 sector (NULL), debugDraw (NULL)
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

  SetGravity (csVector3 (0.0f, -9.81f, 0.0f));
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
  btVector3 gravity = CSToBullet (v, sys->getInternalScale ());
  bulletWorld->setGravity (gravity);

  if (isSoftWorld)
    softWorldInfo->m_gravity = gravity;
}

void csBulletSector::AddCollisionObject (iCollisionObject* object)
{
  csRef<csBulletCollisionObject> obj (dynamic_cast<csBulletCollisionObject*>(object));
  collisionObjects.Push (obj);
  obj->sector = this;
  obj->AddBulletObject ();
}

void csBulletSector::RemoveCollisionObject (iCollisionObject* object)
{
  csBulletCollisionObject* collObject = dynamic_cast<csBulletCollisionObject*> (object);
  CS_ASSERT (collObject);

  if (collObject->GetObjectType () == COLLISION_OBJECT_PHYSICAL)
  {
    iPhysicalBody* phyBody = dynamic_cast<iPhysicalBody*> (object);
    if (phyBody->GetBodyType () == CS::Physics::BODY_RIGID)
      RemoveRigidBody (phyBody->QueryRigidBody ());
    else
      RemoveSoftBody (phyBody->QuerySoftBody ());
  }
  else
  {
    collObject->RemoveBulletObject ();
    collObject->insideWorld = false;
    collisionObjects.Delete (collObject);
  }
}

iCollisionObject* csBulletSector::GetCollisionObject (size_t index)
{
  return collisionObjects[index]->QueryCollisionObject ();
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

HitBeamResult csBulletSector::HitBeam (const csVector3& start, const csVector3& end)
{
  btVector3 rayFrom = CSToBullet (start, sys->getInternalScale ());
  btVector3 rayTo = CSToBullet (end, sys->getInternalScale ());

  btCollisionWorld::ClosestRayResultCallback rayCallback (rayFrom, rayTo);
  bulletWorld->rayTest (rayFrom, rayTo, rayCallback);

  CS::Collision::HitBeamResult result;

  if(rayCallback.hasHit ())
  {
    csBulletCollisionObject* collObject = (csBulletCollisionObject*) (
      rayCallback.m_collisionObject->getUserPointer ());

    if (!collObject->GetObjectType () == COLLISION_OBJECT_PHYSICAL)
    {
      if(collObject->GetObjectType () == CS::Collision::CollisionObjectType::COLLISION_OBJECT_GHOST)
      {
        //Portals are not included.
        for (size_t i = 0; i < portals.GetSize (); i++)
        {
          if (portals[i].ghostPortal1 == collObject
            || portals[i].ghostPortal2 == collObject)
            return result;
        }
      }
      result.hasHit = true;
      result.object = collObject;
      result.isect = BulletToCS (rayCallback.m_hitPointWorld,
        sys->getInverseInternalScale ());
      result.normal = BulletToCS (rayCallback.m_hitNormalWorld,
        sys->getInverseInternalScale ());
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
        }
      }
      else
      {
        result.hasHit = true;
        result.object = collObject;
        result.isect = BulletToCS (rayCallback.m_hitPointWorld,
          sys->getInverseInternalScale ());
        result.normal = BulletToCS (rayCallback.m_hitNormalWorld,
          sys->getInverseInternalScale ());
        return result;
      }
    }
  }
  return result;
}

HitBeamResult csBulletSector::HitBeamPortal (const csVector3& start, const csVector3& end)
{
  btVector3 rayFrom = CSToBullet (start, sys->getInternalScale ());
  btVector3 rayTo = CSToBullet (end, sys->getInternalScale ());

  btCollisionWorld::ClosestRayResultCallback rayCallback (rayFrom, rayTo);
  bulletWorld->rayTest (rayFrom, rayTo, rayCallback);

  CS::Collision::HitBeamResult result;

  if(rayCallback.hasHit ())
  {
    csBulletCollisionObject* collObject = static_cast<csBulletCollisionObject*> (
      rayCallback.m_collisionObject->getUserPointer ());

    if (!collObject->GetObjectType () == COLLISION_OBJECT_PHYSICAL)
    {
      result.hasHit = true;
      result.object = collObject;
      result.isect = BulletToCS (rayCallback.m_hitPointWorld,
        sys->getInverseInternalScale ());
      result.normal = BulletToCS (rayCallback.m_hitNormalWorld,
        sys->getInverseInternalScale ());
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
        }
      }
      else
      {
        result.hasHit = true;
        result.object = collObject;
        result.isect = BulletToCS (rayCallback.m_hitPointWorld,
          sys->getInverseInternalScale ());
        result.normal = BulletToCS (rayCallback.m_hitNormalWorld,
          sys->getInverseInternalScale ());
        return result;
      }
    }
  }
  return result;
}

bool csBulletSector::CollisionTest (iCollisionObject* object, csArray<CollisionData>& collisions)
{
  size_t length = collisions.GetSize ();
  PointContactResult result(sys, collisions);

  csBulletCollisionObject* collObject = dynamic_cast<csBulletCollisionObject*> (object);
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
  bulletWorld->contactTest (collObject->btObject, result);
  if (length != collisions.GetSize ())
    return true;
  else
    return false;
}

bool csBulletSector::BulletCollide (btCollisionObject* objectA,
                                    btCollisionObject* objectB)
{
  //contactPairTest
  //Does user need the collision data?
  csArray<CollisionData> data;
  PointContactResult result(sys, data);
  bulletWorld->contactPairTest (objectA, objectB, result);
  if (data.IsEmpty ())
    return true;
  else
    return false;
}

HitBeamResult csBulletSector::RigidHitBeam (btCollisionObject* object, 
                                            const csVector3& start,
                                            const csVector3& end)
{
  btVector3 rayFrom = CSToBullet (start, sys->getInternalScale ());
  btVector3 rayTo = CSToBullet (end, sys->getInternalScale ());

  btCollisionWorld::ClosestRayResultCallback rayCallback (rayFrom, rayTo);

  //Call rayTestSingle or use callback.process ? 
  //The latter calls the former, but before that it will check the collision filter.
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

  CS::Collision::HitBeamResult result;

  if(rayCallback.hasHit ())
  {
    csBulletCollisionObject* collObject = static_cast<csBulletCollisionObject*> (
      rayCallback.m_collisionObject->getUserPointer ());

    if (!collObject->GetObjectType () == COLLISION_OBJECT_PHYSICAL)
    {
      result.hasHit = true;
      result.object = collObject;
      result.isect = BulletToCS (rayCallback.m_hitPointWorld,
        sys->getInverseInternalScale ());
      result.normal = BulletToCS (rayCallback.m_hitNormalWorld,
        sys->getInverseInternalScale ());
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
        }
      }
      else
      {
        result.hasHit = true;
        result.object = collObject;
        result.isect = BulletToCS (rayCallback.m_hitPointWorld,
          sys->getInverseInternalScale ());
        result.normal = BulletToCS (rayCallback.m_hitNormalWorld,
          sys->getInverseInternalScale ());
        return result;
      }
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

void csBulletSector::AddRigidBody (iRigidBody* body)
{
  csRef<csBulletRigidBody> btBody (dynamic_cast<csBulletRigidBody*>(body));
  rigidBodies.Push (btBody);

  btBody->sector = this;
  btBody->AddBulletObject ();
  btBody->SetLinearDampener (linearDampening);
  btBody->SetRollingDampener (angularDampening);
}

void csBulletSector::RemoveRigidBody (iRigidBody* body)
{
  csBulletRigidBody* btBody = dynamic_cast<csBulletRigidBody*> (body);
  CS_ASSERT (btBody);

  btBody->RemoveBulletObject ();
  rigidBodies.Delete (btBody);
}

iRigidBody* csBulletSector::GetRigidBody (size_t index)
{
  return rigidBodies[index]->QueryRigidBody ();
}

void csBulletSector::AddSoftBody (iSoftBody* body)
{
  csRef<csBulletSoftBody> btBody (dynamic_cast<csBulletSoftBody*>(body));
  softBodies.Push (btBody);
  btBody->sector = this;
  btBody->AddBulletObject ();
}

void csBulletSector::RemoveSoftBody (iSoftBody* body)
{
  csBulletSoftBody* btBody = dynamic_cast<csBulletSoftBody*> (body);
  CS_ASSERT (btBody);

  btBody->RemoveBulletObject ();
  softBodies.Delete (btBody);
}

iSoftBody* csBulletSector::GetSoftBody (size_t index)
{
  return softBodies[index]->QuerySoftBody ();
}

void csBulletSector::RemoveJoint (iJoint* joint)
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

void csBulletSector::SetDebugMode (CS::Physics::Bullet::DebugMode mode)
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
    debugDraw = new csBulletDebugDraw (sys->getInverseInternalScale ());
    bulletWorld->setDebugDrawer (debugDraw);
  }

  debugDraw->SetDebugMode (mode);
}

CS::Physics::Bullet::DebugMode csBulletSector::GetDebugMode ()
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

SCF_IMPLEMENT_FACTORY (csBulletSystem)

csBulletSystem::csBulletSystem (iBase* iParent)
  : scfImplementationType (this, iParent)
{
  csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet> (
    object_reg, "crystalspace.shared.stringset");
  baseID = strings->Request ("base");
  colldetID = strings->Request ("colldet");
  defaultInfo = new btSoftBodyWorldInfo;
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

csRef<iColliderConvexMesh> csBulletSystem::CreateColliderConvexMesh (iMeshWrapper* mesh)
{
  csRef<csBulletColliderConvexMesh> collider;
  collider.AttachNew (new csBulletColliderConvexMesh (mesh, this));

  colliders.Push (collider);
  return (iColliderConvexMesh*)collider;
}

csRef<iColliderConcaveMesh> csBulletSystem::CreateColliderConcaveMesh (iMeshWrapper* mesh,
                                                                                bool isStatic /* = false */)
{
  csRef<csBulletColliderConcaveMesh> collider;
  collider.AttachNew (new csBulletColliderConcaveMesh (mesh,this));

  colliders.Push (collider);
  return collider;
}

csRef<iColliderConcaveMeshScaled> csBulletSystem::CreateColliderConcaveMeshScaled (
  iColliderConcaveMesh* collider, float scale)
{
  csRef<csBulletColliderConcaveMeshScaled> coll;
  coll.AttachNew (new csBulletColliderConcaveMeshScaled (collider, scale,this));

  colliders.Push (coll);
  return coll;
}

csRef<iColliderCylinder> csBulletSystem::CreateColliderCylinder (float length, float radius)
{
  csRef<csBulletColliderCylinder> collider;
  collider.AttachNew (new csBulletColliderCylinder (length, radius, this));

  colliders.Push (collider);
  return collider;
}

csRef<iColliderBox> csBulletSystem::CreateColliderBox (const csVector3& size)
{
  csRef<csBulletColliderBox> collider;
  collider.AttachNew (new csBulletColliderBox (size, this));

  colliders.Push (collider);
  return collider;
} 

csRef<iColliderSphere> csBulletSystem::CreateColliderSphere (float radius)
{
  csRef<csBulletColliderSphere> collider;
  collider.AttachNew (new csBulletColliderSphere (radius, this));

  colliders.Push (collider);
  return collider;
}

csRef<iColliderCapsule> csBulletSystem::CreateColliderCapsule (float length, float radius)
{
  csRef<csBulletColliderCapsule> collider;
  collider.AttachNew (new csBulletColliderCapsule (length, radius, this));
  
  colliders.Push (collider);
  return collider;
}

csRef<iColliderCone> csBulletSystem::CreateColliderCone (float length, float radius)
{
  csRef<csBulletColliderCone> collider;
  collider.AttachNew (new csBulletColliderCone (length, radius, this));

  colliders.Push (collider);
  return collider;
}

csRef<iColliderPlane> csBulletSystem::CreateColliderPlane (const csPlane3& plane)
{
  csRef<csBulletColliderPlane> collider;
  collider.AttachNew (new csBulletColliderPlane (plane, this));

  colliders.Push (collider);
  return collider;
}

csRef<iColliderTerrain> csBulletSystem::CreateColliderTerrain (iTerrainSystem* terrain, 
                                                               float minHeight /* = 0 */, 
                                                               float maxHeight /* = 0 */)
{
  csRef<csBulletColliderTerrain> collider;
  collider.AttachNew (new csBulletColliderTerrain (terrain, minHeight, maxHeight, this));

  colliders.Push (collider);
  return collider;
}

csRef<iCollisionObject> csBulletSystem::CreateCollisionObject ()
{
  csRef<csBulletCollisionObject> collObject;
  collObject.AttachNew (new csBulletCollisionObject (this));

  objects.Push (collObject);
  return collObject;
}

csRef<iCollisionActor> csBulletSystem::CreateCollisionActor ()
{
  csRef<iCollisionActor> collActor;
  /*collActor.AttachNew (new csBulletCollisionActor (this));

  actors.Push (collActor);*/
  return collActor;
}
csRef<iCollisionSector> csBulletSystem::CreateCollisionSector ()
{
  csRef<iCollisionSector> collSector;
  collSector.AttachNew (new csBulletSector (this));

  collSectors.Push (collSector);
  return collSector;
}

CollisionGroup& csBulletSystem::CreateCollisionGroup (const char* name)
{
//TODO
  return CollisionGroup();
}

CollisionGroup& csBulletSystem::FindCollisionGroup (const char* name)
{
//TODO
  return CollisionGroup();
}

void csBulletSystem::SetGroupCollision (CollisionGroup& group1,
                                        CollisionGroup& group2,
                                        bool collide)
{
//TODO
}
bool csBulletSystem::GetGroupCollision (CollisionGroup& group1,
                                        CollisionGroup& group2)
{
//TODO
  return true;
}

void csBulletSystem::DecomposeConcaveMesh (iCollisionObject* object, iMeshWrapper* mesh)
{
  csBulletCollisionObject* btCollObject = dynamic_cast<csBulletCollisionObject*> (object);

  class MyConvexDecomposition : public ConvexDecomposition::ConvexDecompInterface
  {
    float scale;
    btVector3 centroid;
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
  desc.mTcount       = triMesh->GetTriangleCount ();
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
    collider.AttachNew (new csBulletColliderConvexMesh (convexShape,this));
    colliders.Push (collider);
    relaTransform = BulletToCS (trans, inverseInternalScale);
    btCollObject->AddCollider (collider, relaTransform);
  }

  //RebuildObject? or let user do it?
}

csRef<iRigidBody> csBulletSystem::CreateRigidBody ()
{
  csRef<csBulletRigidBody> body;
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

csRef<iJoint> csBulletSystem::CreateRigidP2PJoint (const csVector3 position)
{
  csRef<csBulletJoint> joint;
  joint.AttachNew (new csBulletJoint (this));
  joint->SetTransConstraints (true, true, true);
  csVector3 trans (0.0f,0.0f,0.0f);
  joint->SetMaximumDistance (trans);
  joint->SetMinimumDistance (trans);
  joint->SetPosition (position);
  joint->SetType (RIGID_P2P_JOINT);
  joints.Push (joint);
  return joint;
}

csRef<iJoint> csBulletSystem::CreateRigidSlideJoint (const csOrthoTransform trans,
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
  joints.Push (joint);
  return joint;
}

csRef<iJoint> csBulletSystem::CreateRigidHingeJoint (const csVector3 position, 
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
  joint->SetMinimumDistance (minDistant);
  joint->SetMaximumDistance (maxDistant);
  minDistant[axis] = minAngle;
  maxDistant[axis] = maxAngle;
  joint->SetMinimumAngle (minDistant);
  joint->SetMaximumAngle (maxDistant);
  joint->SetPosition (position);  
  joint->SetType (RIGID_HINGE_JOINT);
  joints.Push (joint);
  return joint;
}

csRef<iJoint> csBulletSystem::CreateSoftLinearJoint (const csVector3 position)
{
  csRef<csBulletJoint> joint;
  joint.AttachNew (new csBulletJoint (this));
  joint->SetPosition (position);
  joint->SetType (SOFT_LINEAR_JOINT);
  joints.Push (joint);
  return joint;
}

csRef<iJoint> csBulletSystem::CreateSoftAngularJoint (int axis)
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
  joints.Push (joint);
  return joint;
}

csRef<iSoftBody> csBulletSystem::CreateRope (csVector3 start,
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
  softBodies.Push (csBody);
  return csRef<iSoftBody>(csBody->QuerySoftBody());
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
    (*defaultInfo, CSToBullet (corner1, internalScale),
    CSToBullet (corner2, internalScale), CSToBullet (corner3, internalScale),
    CSToBullet (corner4, internalScale), segmentCount1, segmentCount2, 0,
    withDiagonals);
  body->m_cfg.collisions |= btSoftBody::fCollision::VF_SS;

  csRef<csBulletSoftBody> csBody;
  csBody.AttachNew (new csBulletSoftBody (this, body));

  softBodies.Push (csBody);
  return csBody;
}

csRef<iSoftBody> csBulletSystem::CreateSoftBody (iGeneralFactoryState* genmeshFactory,
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
    (*defaultInfo, btVertices, btTriangles, triangleCount, false);

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
