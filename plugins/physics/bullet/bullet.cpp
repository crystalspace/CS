/*
Copyright (C) 2007 by Jorrit Tyberghein

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
#include "csgeom/sphere.h"
#include "csgeom/tri.h"
#include "iengine/mesh.h"
#include "iengine/light.h"
#include "imesh/genmesh.h"
#include "imesh/object.h"
#include "csutil/sysfunc.h"
#include "iutil/objreg.h"
#include "ivaria/view.h"

#include "csutil/custom_new_disable.h"

// Bullet includes.
#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"
#include "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"
#include "BulletSoftBody/btSoftBody.h"
#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"
#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "BulletSoftBody/btSoftBodyHelpers.h"

#include "csutil/custom_new_enable.h"

#include "common.h"
#include "bullet.h"
#include "rigidbodies.h"
#include "colliders.h"
#include "joints.h"
#include "softbodies.h"

#define COLLISION_THRESHOLD 0.01f

CS_PLUGIN_NAMESPACE_BEGIN(Bullet)
{

SCF_IMPLEMENT_FACTORY (csBulletDynamics)

//----------------------- csBulletDynamics ----------------------------

csBulletDynamics::csBulletDynamics (iBase *iParent)
  : scfImplementationType (this, iParent)
{
}

csBulletDynamics::~csBulletDynamics ()
{
  systems.DeleteAll ();
}

bool csBulletDynamics::Initialize (iObjectRegistry* object_reg)
{
  csBulletDynamics::object_reg = object_reg;
  return true;
}

csPtr< ::iDynamicSystem> csBulletDynamics::CreateSystem ()
{
  csRef<csBulletDynamicsSystem> system;
  system.AttachNew (new csBulletDynamicsSystem (object_reg));
  systems.Push (system);

  return csPtr< ::iDynamicSystem> (system);
}

void csBulletDynamics::RemoveSystem (::iDynamicSystem* system)
{
  systems.Delete (system);
}

void csBulletDynamics::RemoveSystems ()
{
  systems.DeleteAll ();
}

::iDynamicSystem* csBulletDynamics::FindSystem (const char *name)
{
  return systems.FindByName (name);
}

void csBulletDynamics::Step (float stepsize)
{
  // step each system
  for (size_t i = 0; i < systems.GetSize (); i++)
    systems[i]->Step (stepsize);

  // call the callbacks
  for (size_t i = 0; i < stepCallbacks.GetSize (); i++)
    stepCallbacks[i]->Step (stepsize);
}

//----------------------- csBulletDynamicsSystem ----------------------------

// TODO: these AABB values will not fit to every worlds
#define AABB_DIMENSIONS 10000.0f

csBulletDynamicsSystem::csBulletDynamicsSystem
  (iObjectRegistry* object_reg)
    : scfImplementationType (this), isSoftWorld (false), softWorldInfo (0),
      gimpactRegistered (false), internalScale (1.0f), inverseInternalScale (1.0f),
      worldTimeStep (1.0f / 60.0f), worldMaxSteps (1), linearDampening (0.0f),
      angularDampening (0.0f), autoDisableEnabled (true),
      linearDisableThreshold (0.8f), angularDisableThreshold (1.0f),
      timeDisableThreshold (0.0), debugDraw (0)
{
  // create base Bullet objects
  configuration = new btDefaultCollisionConfiguration ();
  dispatcher = new btCollisionDispatcher (configuration);
  solver = new btSequentialImpulseConstraintSolver;

  const int maxProxies = 32766;
  btVector3 worldAabbMin (-AABB_DIMENSIONS, -AABB_DIMENSIONS, -AABB_DIMENSIONS);
  btVector3 worldAabbMax (AABB_DIMENSIONS, AABB_DIMENSIONS, AABB_DIMENSIONS);
  broadphase = new btAxisSweep3 (worldAabbMin, worldAabbMax, maxProxies);

  // create dynamics world
  bulletWorld = new btDiscreteDynamicsWorld (dispatcher,
      broadphase, solver, configuration);
  SetGravity (csVector3 (0.0f, -9.81f, 0.0f));

  // register default callback
  moveCb.AttachNew (new csBulletDefaultMoveCallback ());

  // init string IDs
  csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet> (
      object_reg, "crystalspace.shared.stringset");
  baseId = strings->Request ("base");
  colldetId = strings->Request ("colldet");
}

csBulletDynamicsSystem::~csBulletDynamicsSystem ()
{
  pivotJoints.DeleteAll ();
  joints.DeleteAll ();
  dynamicBodies.DeleteAll ();
  colliderBodies.DeleteAll ();
  softBodies.DeleteAll ();
  terrainColliders.DeleteAll ();

  delete bulletWorld;
  delete debugDraw;
  delete dispatcher;
  delete configuration;
  delete solver;
  delete broadphase;
  delete softWorldInfo;
}

void csBulletDynamicsSystem::SetGravity (const csVector3& v)
{
  btVector3 gravity = CSToBullet (v, internalScale);
  bulletWorld->setGravity (gravity);

  if (isSoftWorld)
    softWorldInfo->m_gravity = gravity;
}

const csVector3 csBulletDynamicsSystem::GetGravity () const
{
  btVector3 v = bulletWorld->getGravity ();
  return BulletToCS (v, inverseInternalScale);
}

void csBulletDynamicsSystem::SetLinearDampener (float dampening)
{
  linearDampening = dampening;
}

float csBulletDynamicsSystem::GetLinearDampener () const
{
  return linearDampening;
}

void csBulletDynamicsSystem::SetRollingDampener (float dampening)
{
  angularDampening = dampening;
}

float csBulletDynamicsSystem::GetRollingDampener () const
{
  return angularDampening;
}

void csBulletDynamicsSystem::EnableAutoDisable (bool enable)
{
  autoDisableEnabled = enable;
}

bool csBulletDynamicsSystem::AutoDisableEnabled ()
{
  return autoDisableEnabled;
}

void csBulletDynamicsSystem::SetAutoDisableParams (float linear,
  float angular, int steps, float time)
{
  linearDisableThreshold = linear;
  angularDisableThreshold = angular;
  timeDisableThreshold = time;
}

void csBulletDynamicsSystem::CheckCollision (csBulletRigidBody& cs_obA,
					     btCollisionObject *obB,
					     btPersistentManifold &contactManifold)
{
  // @@@ TODO: make both calls in just one pass
  if (cs_obA.lastContactObjects.Contains(obB) == csArrayItemNotFound)
  {
    cs_obA.contactObjects.Push(obB);
    cs_obA.lastContactObjects.Push(obB);

    // fire callback
    float total = 0.0f;
    for (int j = 0; j < contactManifold.getNumContacts(); j++)
    {
      btManifoldPoint& pt = contactManifold.getContactPoint(j);
      total += pt.m_appliedImpulse;
    }

    if (total > COLLISION_THRESHOLD)
    {
      csBulletRigidBody *cs_obB;
      cs_obB = (csBulletRigidBody*) obB->getUserPointer();
      if (cs_obB)
	// TODO: use the real position and normal of the contact
        cs_obA.Collision(cs_obB, csVector3 (0.0f, 0.0f, 0.0f),
			 csVector3 (0.0f, 1.0f, 0.0f), total);
    }
  }
  else if (cs_obA.contactObjects.Contains(obB) == csArrayItemNotFound)
  {
    cs_obA.contactObjects.Push(obB);
  }
}

void csBulletDynamicsSystem::CheckCollisions ()
{
  // TODO: use gContactAddedCallback instead?

  int numManifolds = bulletWorld->getDispatcher()->getNumManifolds();

  // clear contact information
  for (size_t i = 0; i < dynamicBodies.GetSize(); i++)
  {
    csBulletRigidBody *body = static_cast<csBulletRigidBody*> (dynamicBodies.Get(i));
    if (body->IsEnabled() && !body->IsStatic())
    {
      body->lastContactObjects.Empty();
      csArray<btCollisionObject*>::Iterator it = body->contactObjects.GetIterator();
      while (it.HasNext())
      {
        body->lastContactObjects.Push(it.Next());
      }
      body->contactObjects.Empty();
    }
  }

  // find new contacts by inspecting the bullet contact manifolds.
  for (int i = 0; i < numManifolds; i++)
  {
    btPersistentManifold* contactManifold =
      bulletWorld->getDispatcher ()->getManifoldByIndexInternal (i);
    btCollisionObject* obA =
      static_cast<btCollisionObject*> (contactManifold->getBody0 ());
    btCollisionObject* obB =
      static_cast<btCollisionObject*> (contactManifold->getBody1 ());
    if (contactManifold->getNumContacts ())
    {
      csBulletRigidBody *cs_obA =
	static_cast<csBulletRigidBody*> (reinterpret_cast<iBody*> (obA->getUserPointer ()));
      if (cs_obA)
	CheckCollision(*cs_obA, obB, *contactManifold);

      csBulletRigidBody *cs_obB =
	static_cast<csBulletRigidBody*> (reinterpret_cast<iBody*> (obB->getUserPointer ()));
      if (cs_obB)
	CheckCollision(*cs_obB, obA, *contactManifold);
    }
  }
}

void csBulletDynamicsSystem::Step (float stepsize)
{
  bulletWorld->stepSimulation (stepsize, (int)worldMaxSteps, worldTimeStep);
  CheckCollisions();
}

csPtr< ::iRigidBody> csBulletDynamicsSystem::CreateBody ()
{
  csRef<csBulletRigidBody> body;
  body.AttachNew (new csBulletRigidBody (this));

  dynamicBodies.Push (body);
  return csPtr< ::iRigidBody> (body);
}

void csBulletDynamicsSystem::AddBody (::iRigidBody* body)
{
  csBulletRigidBody* csBody = static_cast<csBulletRigidBody*> (body);
  CS_ASSERT (csBody);
  if (csBody->body)
  {
    // remove from the previous dynamic system
    if (csBody->insideWorld)
      csBody->dynSys->bulletWorld->removeRigidBody (csBody->body);

    // add the body to this dynamic system
    bulletWorld->addRigidBody (csBody->body);
    csBody->body->forceActivationState (ACTIVE_TAG);
    csBody->dynSys = this;
    csBody->insideWorld = true;
  }
  dynamicBodies.Push (csBody);
}

void csBulletDynamicsSystem::RemoveBody (::iRigidBody* body)
{
  csBulletRigidBody* csBody = static_cast<csBulletRigidBody*> (body);
  CS_ASSERT (csBody);
  if (csBody->body)
  {
    // wake up all connected bodies
    for (size_t i = 0; i < csBody->contactObjects.GetSize (); i++)
      csBody->contactObjects[i]->activate ();

    // remove the body from the world
    bulletWorld->removeRigidBody (csBody->body);
  }
  csBody->insideWorld = false;

  dynamicBodies.Delete (body);
}

::iRigidBody* csBulletDynamicsSystem::FindBody (const char* name)
{
  return dynamicBodies.FindByName (name);
}

::iRigidBody* csBulletDynamicsSystem::GetBody (unsigned int index)
{
  CS_ASSERT(index < dynamicBodies.GetSize ());
  return dynamicBodies[index];
}

int csBulletDynamicsSystem::GetBodysCount ()
{
  return (int)dynamicBodies.GetSize ();
}

csPtr<iBodyGroup> csBulletDynamicsSystem::CreateGroup ()
{
  // @@@ TODO
  return 0;
}

void csBulletDynamicsSystem::RemoveGroup (iBodyGroup*)
{
  // @@@ TODO
}

csPtr<iJoint> csBulletDynamicsSystem::CreateJoint ()
{
  csRef<csBulletJoint> joint;
  joint.AttachNew (new csBulletJoint (this));
  joints.Push (joint);
  return csPtr<iJoint> (joint);
}

void csBulletDynamicsSystem::RemoveJoint (iJoint* joint)
{
  joints.Delete (joint);
}

iDynamicsMoveCallback* csBulletDynamicsSystem::GetDefaultMoveCallback ()
{
  return moveCb;
}

bool csBulletDynamicsSystem::AttachColliderConvexMesh (iMeshWrapper* mesh,
  const csOrthoTransform& trans, float friction,
  float elasticity, float softness)
{
  csRef<csBulletRigidBody> body;
  body.AttachNew (new csBulletRigidBody (this, true));
  colliderBodies.Push (body);

  body->SetTransform (trans);
  csOrthoTransform identity;
  body->AttachColliderConvexMesh (mesh, identity, friction, elasticity, softness);

  return true;
}

bool csBulletDynamicsSystem::AttachColliderMesh (iMeshWrapper* mesh,
  const csOrthoTransform& trans, float friction,
  float elasticity, float softness)
{
  csRef<csBulletRigidBody> body;
  body.AttachNew (new csBulletRigidBody (this, true));
  colliderBodies.Push (body);

  body->SetTransform (trans);
  csOrthoTransform identity;
  body->AttachColliderMesh (mesh, identity, friction, elasticity, softness);

  return true;
}

bool csBulletDynamicsSystem::AttachColliderCylinder (float length,
  float radius, const csOrthoTransform& trans, float friction,
  float elasticity, float softness)
{
  csRef<csBulletRigidBody> body;
  body.AttachNew (new csBulletRigidBody (this, true));
  colliderBodies.Push (body);

  body->SetTransform (trans);
  csOrthoTransform identity;
  body->AttachColliderCylinder (length, radius, identity, friction, elasticity, softness);

  return true;
}

bool csBulletDynamicsSystem::AttachColliderCapsule (float length,
  float radius, const csOrthoTransform& trans, float friction,
  float elasticity, float softness)
{
  csRef<csBulletRigidBody> body;
  body.AttachNew (new csBulletRigidBody (this, true));
  colliderBodies.Push (body);

  body->SetTransform (trans);
  csOrthoTransform identity;
  body->AttachColliderCapsule (length, radius, identity, friction, elasticity, softness);

  return true;
}

bool csBulletDynamicsSystem::AttachColliderBox (const csVector3 &size,
  const csOrthoTransform& trans, float friction,
  float elasticity, float softness)
{
  csRef<csBulletRigidBody> body;
  body.AttachNew (new csBulletRigidBody (this, true));
  colliderBodies.Push (body);

  body->SetTransform (trans);
  csOrthoTransform identity;
  body->AttachColliderBox (size, identity, friction, elasticity, softness);

  return true;
}

bool csBulletDynamicsSystem::AttachColliderSphere (float radius,
  const csVector3 &offset, float friction,
  float elasticity, float softness)
{
  csRef<csBulletRigidBody> body;
  body.AttachNew (new csBulletRigidBody (this, true));
  colliderBodies.Push (body);

  body->AttachColliderSphere (radius, offset, friction, elasticity, softness);

  return true;
}

bool csBulletDynamicsSystem::AttachColliderPlane (const csPlane3 &plane,
  float friction, float elasticity, float softness)
{
  csRef<csBulletRigidBody> body;
  body.AttachNew (new csBulletRigidBody (this, true));
  colliderBodies.Push (body);

  body->AttachColliderPlane (plane, friction, elasticity, softness);

  return true;
}

void csBulletDynamicsSystem::AttachCollider (iDynamicsSystemCollider*)
{
  // nonsense?
}

csRef<iDynamicsSystemCollider> csBulletDynamicsSystem::CreateCollider () 
{
  // create static rigid body
  csRef<csBulletRigidBody> body;
  body.AttachNew (new csBulletRigidBody (this, true));
  colliderBodies.Push (body);

  // create collider
  csRef<csBulletCollider> collider;
  collider.AttachNew (new csBulletCollider (this, body, false));

  collider->isStaticBody = true;
  body->colliders.Push (collider);

  return collider;
}

void csBulletDynamicsSystem::DestroyColliders ()
{
  // TODO: destroy linked joints before
  colliderBodies.DeleteAll ();
}

void csBulletDynamicsSystem::DestroyCollider (iDynamicsSystemCollider* collider)
{
  // TODO: destroy linked joints before
  size_t i = colliderBodies.GetSize ();
  while (i >= 0)
  {
    i--;
    if (((csBulletRigidBody*) colliderBodies[i])->colliders[0] == collider)
    {
      colliderBodies.DeleteIndex (i);
      return;
    }
  }
}

csRef<iDynamicsSystemCollider> csBulletDynamicsSystem::GetCollider (unsigned int index) 
{
  CS_ASSERT(index < colliderBodies.GetSize ());
  return ((csBulletRigidBody*) colliderBodies[index])->colliders[0];
}

int csBulletDynamicsSystem::GetColliderCount () 
{
  return (int) colliderBodies.GetSize ();
}

void csBulletDynamicsSystem::RegisterGimpact ()
{
  if (!gimpactRegistered)
  {
    btCollisionDispatcher* dispatcher =
      static_cast<btCollisionDispatcher*> (bulletWorld->getDispatcher ());
    btGImpactCollisionAlgorithm::registerAlgorithm (dispatcher);
    gimpactRegistered = true;
  }
}

void csBulletDynamicsSystem::DebugDraw (iView* view)
{
  if (!debugDraw)
  {
    debugDraw = new csBulletDebugDraw (inverseInternalScale);
    bulletWorld->setDebugDrawer (debugDraw);
  }

  bulletWorld->debugDrawWorld();
  debugDraw->DebugDraw (view);
}

void csBulletDynamicsSystem::SetDebugMode (CS::Physics::Bullet::DebugMode mode)
{
  if (mode == CS::Physics::Bullet::DEBUG_NOTHING)
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

CS::Physics::Bullet::DebugMode csBulletDynamicsSystem::GetDebugMode ()
{
  if (!debugDraw)
    return CS::Physics::Bullet::DEBUG_NOTHING;

  return debugDraw->GetDebugMode ();
}

CS::Physics::Bullet::HitBeamResult csBulletDynamicsSystem::HitBeam
(const csVector3 &start, const csVector3 &end)
{
  btVector3 rayFrom = CSToBullet (start, internalScale);
  btVector3 rayTo = CSToBullet (end, internalScale);
  btCollisionWorld::ClosestRayResultCallback rayCallback (rayFrom, rayTo);
  bulletWorld->rayTest (rayFrom, rayTo, rayCallback);

  CS::Physics::Bullet::HitBeamResult result;

  if (rayCallback.hasHit())
  {
    iBody* bulletBody =
      static_cast<iBody*> (rayCallback.m_collisionObject->getUserPointer ());

    switch (bulletBody->GetType ())
      {
      case CS::Physics::Bullet::RIGID_BODY:
	{
	  result.hasHit = true;
	  result.body = bulletBody;
	  result.isect = BulletToCS (rayCallback.m_hitPointWorld,
				     inverseInternalScale);
	  result.normal = BulletToCS (rayCallback.m_hitNormalWorld,
				      inverseInternalScale);	
	  return result;
	  break;
	}

      case CS::Physics::Bullet::TERRAIN:
	{
	  result.hasHit = true;
	  result.body = bulletBody;
	  result.isect = BulletToCS (rayCallback.m_hitPointWorld,
				     inverseInternalScale);
	  result.normal = BulletToCS (rayCallback.m_hitNormalWorld,
				      inverseInternalScale);	
	  return result;
	  break;
	}

      case CS::Physics::Bullet::SOFT_BODY:
	{
	btSoftBody* body = btSoftBody::upcast (rayCallback.m_collisionObject);
	btSoftBody::sRayCast ray;
	if (body->rayTest (rayFrom, rayTo, ray))
	{
	  result.hasHit = true;
	  result.body = bulletBody;
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
	    }
	    break;

	  default:
	    // TODO: may need other types?
	    break;
	  }

	  return result;
	}
	}

	break;

      default:
	break;
      }
  }

  return result;
}

void csBulletDynamicsSystem::SetInternalScale (float scale)
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

  // re-create dynamics world
  delete broadphase;
  const int maxProxies = 32766;
  broadphase = new btAxisSweep3 (worldAabbMin, worldAabbMax, maxProxies);

  if (isSoftWorld)
  {
    bulletWorld = new btSoftRigidDynamicsWorld
      (dispatcher, broadphase, solver, configuration);
    softWorldInfo->m_broadphase = broadphase;
  }

  else
    bulletWorld = new btDiscreteDynamicsWorld
      (dispatcher, broadphase, solver, configuration);

  SetGravity (tempGravity);
}

void csBulletDynamicsSystem::SetStepParameters (float timeStep, size_t maxSteps,
						size_t iterations)
{
  worldTimeStep = timeStep;
  worldMaxSteps = maxSteps;
  btContactSolverInfo& info = bulletWorld->getSolverInfo();
  info.m_numIterations = (int)iterations;
}

void csBulletDynamicsSystem::SetSoftBodyWorld (bool isSoftBodyWorld)
{
  CS_ASSERT(!dynamicBodies.GetSize ()
	    && !colliderBodies.GetSize ()
	    && !terrainColliders.GetSize ());

  if (isSoftWorld == isSoftBodyWorld)
    return;

  isSoftWorld = isSoftBodyWorld;

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
}

size_t csBulletDynamicsSystem::GetSoftBodyCount ()
{
  if (!isSoftWorld)
    return 0;

  btSoftRigidDynamicsWorld* softWorld =
    static_cast<btSoftRigidDynamicsWorld*> (bulletWorld);
  return softWorld->getSoftBodyArray ().size ();
}

iSoftBody* csBulletDynamicsSystem::GetSoftBody (size_t index)
{
  CS_ASSERT(isSoftWorld && index < softBodies.GetSize ());
  return softBodies[index];
}

bool csBulletDynamicsSystem::GetSoftBodyWorld ()
{
  return isSoftWorld;
}

iSoftBody* csBulletDynamicsSystem::CreateRope
(csVector3 start, csVector3 end, uint segmentCount)
{
  CS_ASSERT(isSoftWorld
	    && segmentCount > 1);

  btSoftBody* body = btSoftBodyHelpers::CreateRope
    (*softWorldInfo, CSToBullet (start, internalScale),
     CSToBullet (end, internalScale), segmentCount - 1, 0);

  //hard-coded parameters for ropes
  body->m_cfg.kDP = 0.08f; // no elasticity
  body->m_cfg.piterations = 16; // no white zone
  body->m_cfg.timescale = 2;

  btSoftRigidDynamicsWorld* softWorld =
    static_cast<btSoftRigidDynamicsWorld*> (bulletWorld);
  softWorld->addSoftBody (body);

  csRef<csBulletSoftBody> csBody;
  csBody.AttachNew (new csBulletSoftBody (this, body));

  softBodies.Push (csBody);
  return csBody;
}

iSoftBody* csBulletDynamicsSystem::CreateRope (csVector3* vertices, size_t vertexCount)
{
  CS_ASSERT(isSoftWorld);

  // Create the nodes
  CS_ALLOC_STACK_ARRAY(btVector3, nodes, vertexCount);
  CS_ALLOC_STACK_ARRAY(btScalar, materials, vertexCount);
  for (size_t i = 0; i < vertexCount; i++)
  {
    nodes[i] = CSToBullet (vertices[i], internalScale);
    materials[i] = 1;
  }

  btSoftBody* body = new btSoftBody(softWorldInfo, vertexCount, &nodes[0], &materials[0]);

  // Create the links between the nodes
  for (size_t i = 1; i < vertexCount; i++)
    body->appendLink (i - 1, i);

  //hard-coded parameters for ropes
  body->m_cfg.kDP = 0.08f; // no elasticity
  body->m_cfg.piterations = 16; // no white zone
  body->m_cfg.timescale = 2;

  btSoftRigidDynamicsWorld* softWorld =
    static_cast<btSoftRigidDynamicsWorld*> (bulletWorld);
  softWorld->addSoftBody (body);

  csRef<csBulletSoftBody> csBody;
  csBody.AttachNew (new csBulletSoftBody (this, body));

  softBodies.Push (csBody);
  return csBody; 
}

iSoftBody* csBulletDynamicsSystem::CreateCloth
(csVector3 corner1, csVector3 corner2, csVector3 corner3, csVector3 corner4,
 uint segmentCount1, uint segmentCount2, bool withDiagonals)
{
  CS_ASSERT(isSoftWorld);

  btSoftBody* body = btSoftBodyHelpers::CreatePatch
    (*softWorldInfo, CSToBullet (corner1, internalScale),
     CSToBullet (corner2, internalScale), CSToBullet (corner3, internalScale),
     CSToBullet (corner4, internalScale), segmentCount1, segmentCount2, 0,
     withDiagonals);
  body->m_cfg.collisions |= btSoftBody::fCollision::VF_SS;

  btSoftRigidDynamicsWorld* softWorld =
    static_cast<btSoftRigidDynamicsWorld*> (bulletWorld);
  softWorld->addSoftBody (body);

  csRef<csBulletSoftBody> csBody;
  csBody.AttachNew (new csBulletSoftBody (this, body));

  softBodies.Push (csBody);
  return csBody;
}

iSoftBody* csBulletDynamicsSystem::CreateSoftBody
(iGeneralFactoryState* genmeshFactory, const csOrthoTransform& bodyTransform)
{
  CS_ASSERT(isSoftWorld);

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
    (*softWorldInfo, vertices, triangles, genmeshFactory->GetTriangleCount ());

  body->generateBendingConstraints(2);
  body->m_cfg.piterations = 10;
  body->m_cfg.collisions |= btSoftBody::fCollision::VF_SS;
  body->m_materials[0]->m_kLST = 1;

  btSoftRigidDynamicsWorld* softWorld =
    static_cast<btSoftRigidDynamicsWorld*> (bulletWorld);
  softWorld->addSoftBody (body);

  csRef<csBulletSoftBody> csBody;
  csBody.AttachNew (new csBulletSoftBody (this, body));

  softBodies.Push (csBody);
  return csBody;
}

iSoftBody* csBulletDynamicsSystem::CreateSoftBody
(csVector3* vertices, size_t vertexCount,
 csTriangle* triangles, size_t triangleCount)
{
  CS_ASSERT(isSoftWorld);

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
    (*softWorldInfo, btVertices, btTriangles, triangleCount);

  body->generateBendingConstraints(2);
  body->m_cfg.piterations = 10;
  body->m_cfg.collisions |= btSoftBody::fCollision::VF_SS;
  body->m_materials[0]->m_kLST = 1;

  delete btVertices;
  delete btTriangles;

  btSoftRigidDynamicsWorld* softWorld =
    static_cast<btSoftRigidDynamicsWorld*> (bulletWorld);
  softWorld->addSoftBody (body);

  csRef<csBulletSoftBody> csBody;
  csBody.AttachNew (new csBulletSoftBody (this, body));

  softBodies.Push (csBody);
  return csBody;
}

void csBulletDynamicsSystem::RemoveSoftBody (iSoftBody* body)
{
  csBulletSoftBody* csBody = static_cast<csBulletSoftBody*> (body);
  CS_ASSERT (csBody);
  btSoftRigidDynamicsWorld* softWorld =
    static_cast<btSoftRigidDynamicsWorld*> (bulletWorld);
  softWorld->removeSoftBody (csBody->body);

  softBodies.Delete (body);
}

csPtr<iPivotJoint> csBulletDynamicsSystem::CreatePivotJoint ()
{
  csRef<csBulletPivotJoint> joint;
  joint.AttachNew (new csBulletPivotJoint (this));
  pivotJoints.Push (joint);
  return csPtr<iPivotJoint> (joint);
}

void csBulletDynamicsSystem::RemovePivotJoint (iPivotJoint* joint)
{
  pivotJoints.Delete (joint);
}

bool csBulletDynamicsSystem::SaveBulletWorld (const char* filename)
{
#ifndef CS_HAVE_BULLET_SERIALIZER
  return false;
#else

  //create a large enough buffer. There is no method to pre-calculate the buffer size yet.
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

iTerrainCollider* csBulletDynamicsSystem::AttachColliderTerrain
(csLockedHeightData& heightData, int gridWidth, int gridHeight,
 csVector3 gridSize, csOrthoTransform& transform,
 float minimumHeight, float maximumHeight)
{
  csRef<csBulletTerrainCellCollider> terrain;
  terrain.AttachNew
    (new csBulletTerrainCellCollider (this, heightData, gridWidth, gridHeight,
				      gridSize, transform, minimumHeight, maximumHeight));
  terrainColliders.Push (terrain);
  return terrain;
}

iTerrainCollider* csBulletDynamicsSystem::AttachColliderTerrain
(iTerrainCell* cell, float minimumHeight, float maximumHeight)
{
  csRef<csBulletTerrainCellCollider> terrain;
  terrain.AttachNew
    (new csBulletTerrainCellCollider (this, cell, minimumHeight, maximumHeight));
  terrainColliders.Push (terrain);
  return terrain;
}

iTerrainCollider* csBulletDynamicsSystem::AttachColliderTerrain
(iTerrainSystem* system, float minimumHeight, float maximumHeight)
{
  csRef<csBulletTerrainCollider> terrain;
  terrain.AttachNew
    (new csBulletTerrainCollider (this, system, minimumHeight, maximumHeight));
  terrainColliders.Push (terrain);
  return terrain;
}

void csBulletDynamicsSystem::DestroyCollider (iTerrainCollider* collider)
{
  terrainColliders.Delete (collider);
}

}
CS_PLUGIN_NAMESPACE_END(Bullet)
