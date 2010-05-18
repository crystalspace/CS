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
#include "csgeom/quaternion.h"
#include "csgeom/sphere.h"
#include "csgeom/tri.h"
#include "igeom/trimesh.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/light.h"
#include "imesh/genmesh.h"
#include "imesh/objmodel.h"
#include "imesh/object.h"
#include "csutil/sysfunc.h"
#include "iutil/objreg.h"
#include "ivaria/view.h"

#include "csutil/custom_new_disable.h"

// Bullet includes.
#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"
#include "BulletSoftBody/btSoftBody.h"
#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"
#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "BulletSoftBody/btSoftBodyHelpers.h"

#include "csutil/custom_new_enable.h"

#include "common.h"
#include "bullet.h"
#include "joints.h"
#include "softbodies.h"

#define COLLISION_THRESHOLD 0.01f

CS_PLUGIN_NAMESPACE_BEGIN(Bullet)
{

SCF_IMPLEMENT_FACTORY (csBulletDynamics)

//---------------------------------------------------------------------------

static csRef<iTriangleMesh> FindColdetTriangleMesh(iMeshWrapper* mesh,
				csStringID base_id, csStringID colldet_id)
{
  iObjectModel* objmodel = mesh->GetMeshObject ()->GetObjectModel ();
  csRef<iTriangleMesh> trimesh;
  bool use_trimesh = objmodel->IsTriangleDataSet (base_id);
  if (use_trimesh)
  {
    if (objmodel->GetTriangleData(colldet_id))
      trimesh = objmodel->GetTriangleData (colldet_id);
    else
      trimesh = objmodel->GetTriangleData (base_id);
  }

  if (!trimesh || trimesh->GetVertexCount () == 0
      || trimesh->GetTriangleCount () == 0)
  {
    csFPrintf (stderr, "csBulletRigidBody: No collision polygons, triangles or vertices on mesh factory '%s'\n",
      mesh->QueryObject()->GetName());

    return 0;
  }
  return trimesh;
}

#include "csutil/custom_new_disable.h"

static btTriangleIndexVertexArray* GenerateTriMeshData
  (iMeshWrapper* mesh, int*& indices, size_t& triangleCount, btVector3*& vertices,
   size_t& vertexCount, csStringID base_id, csStringID colldet_id,
   float internalScale)
{
  csRef<iTriangleMesh> trimesh = FindColdetTriangleMesh(mesh, base_id, colldet_id);
  if (!trimesh)
    return 0;

  // TODO: remove double vertices
  csTriangle *c_triangle = trimesh->GetTriangles();
  triangleCount = trimesh->GetTriangleCount();
  vertexCount = trimesh->GetVertexCount ();

  delete[] indices;
  indices = new int[triangleCount * 3];
  int indexStride = 3 * sizeof (int);

  size_t i;
  int* id = indices;
  for (i = 0 ; i < triangleCount ; i++)
  {
    *id++ = c_triangle[i].a;
    *id++ = c_triangle[i].b;
    *id++ = c_triangle[i].c;
  }

  delete[] vertices;
  vertices = new btVector3[vertexCount];
  csVector3 *c_vertex = trimesh->GetVertices();
  int vertexStride = sizeof (btVector3);

  for (i = 0 ; i < vertexCount ; i++)
    vertices[i].setValue (c_vertex[i].x * internalScale,
			  c_vertex[i].y * internalScale,
			  c_vertex[i].z * internalScale);

  btTriangleIndexVertexArray* indexVertexArrays =
    new btTriangleIndexVertexArray ((int)triangleCount, indices, (int)indexStride,
	(int)vertexCount, (btScalar*) &vertices[0].x (), vertexStride);
  return indexVertexArrays;
}

#include "csutil/custom_new_enable.h"

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

csPtr<iDynamicSystem> csBulletDynamics::CreateSystem ()
{
  csRef<csBulletDynamicsSystem> system;
  system.AttachNew (new csBulletDynamicsSystem (object_reg));
  systems.Push (system);

  return csPtr<iDynamicSystem> (system);
}

void csBulletDynamics::RemoveSystem (iDynamicSystem* system)
{
  systems.Delete (system);
}

void csBulletDynamics::RemoveSystems ()
{
  systems.DeleteAll ();
}

iDynamicSystem* csBulletDynamics::FindSystem (const char *name)
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

void csBulletDynamicsSystem::CheckCollision(csBulletRigidBody& cs_obA,btCollisionObject *obB,btPersistentManifold &contactManifold)
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
    btPersistentManifold* contactManifold = bulletWorld->getDispatcher()->getManifoldByIndexInternal(i);
    btCollisionObject* obA = static_cast<btCollisionObject*>(contactManifold->getBody0());
    btCollisionObject* obB = static_cast<btCollisionObject*>(contactManifold->getBody1());
    if (contactManifold->getNumContacts())
    {
        csBulletRigidBody *cs_obA = (csBulletRigidBody*)obA->getUserPointer();
        if (cs_obA)
          CheckCollision(*cs_obA, obB, *contactManifold);
        csBulletRigidBody *cs_obB = (csBulletRigidBody*)obB->getUserPointer();
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

csPtr<iRigidBody> csBulletDynamicsSystem::CreateBody ()
{
  csRef<csBulletRigidBody> body;
  body.AttachNew (new csBulletRigidBody (this));

  dynamicBodies.Push (body);
  return csPtr<iRigidBody> (body);
}

void csBulletDynamicsSystem::AddBody (iRigidBody* body)
{
  csBulletRigidBody* csBody = dynamic_cast<csBulletRigidBody*> (body);
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

void csBulletDynamicsSystem::RemoveBody (iRigidBody* body)
{
  csBulletRigidBody* csBody = dynamic_cast<csBulletRigidBody*> (body);
  CS_ASSERT (csBody);
  if (csBody->body)
    bulletWorld->removeRigidBody (csBody->body);
  csBody->insideWorld = false;

  dynamicBodies.Delete (body);
}

iRigidBody* csBulletDynamicsSystem::FindBody (const char* name)
{
  return dynamicBodies.FindByName (name);
}

iRigidBody* csBulletDynamicsSystem::GetBody (unsigned int index)
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

void csBulletDynamicsSystem::SetDebugMode (csBulletDebugMode mode)
{
  if (mode == CS_BULLET_DEBUG_NOTHING)
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

csBulletDebugMode csBulletDynamicsSystem::GetDebugMode ()
{
  if (!debugDraw)
    return CS_BULLET_DEBUG_NOTHING;

  return debugDraw->GetDebugMode ();
}

csBulletHitBeamResult csBulletDynamicsSystem::HitBeam
(const csVector3 &start, const csVector3 &end)
{
  btVector3 rayFrom = CSToBullet (start, internalScale);
  btVector3 rayTo = CSToBullet (end, internalScale);
  btCollisionWorld::ClosestRayResultCallback rayCallback (rayFrom, rayTo);
  bulletWorld->rayTest (rayFrom, rayTo, rayCallback);

  csBulletHitBeamResult result;
  if (rayCallback.hasHit())
  {
    btRigidBody* body = btRigidBody::upcast (rayCallback.m_collisionObject);
    if (body)
    {
      result.body = (csBulletRigidBody*) body->getUserPointer ();
      result.isect = BulletToCS (rayCallback.m_hitPointWorld,
				 inverseInternalScale);
    }
  }

  // Check for soft bodies
  if (!result.body && isSoftWorld)
  {
    btSoftRigidDynamicsWorld* softWorld =
      static_cast<btSoftRigidDynamicsWorld*> (bulletWorld);

    btSoftBodyArray& sbs = softWorld->getSoftBodyArray ();
    for (int ib = 0; ib < sbs.size (); ++ib)
    {
      btSoftBody* psb = sbs[ib];
      btSoftBody::sRayCast ray;
      if (psb->rayTest (rayFrom, rayTo, ray) && ray.fraction < 1.00f)
      {
	result.softBody = (csBulletSoftBody*) psb->getUserPointer ();
	btVector3 impact = rayFrom + (rayTo - rayFrom) * ray.fraction;
	result.isect = BulletToCS (impact, inverseInternalScale);

	// find the closest vertex
	switch(ray.feature)
	{
	case btSoftBody::eFeature::Face:
	  {
	    btSoftBody::Face& face = psb->m_faces[ray.index];
	    btSoftBody::Node* node = face.m_n[0];
	    float distance = (node->m_x - impact).length2();
	    for (int i = 1; i < 3; i++)
	    {
	      float nodeDistance = (face.m_n[i]->m_x - impact).length2();
	      if (nodeDistance < distance)
	      {
		node = face.m_n[i];
		distance = nodeDistance;
	      }
	    }

	    result.vertexIndex = size_t (node - &psb->m_nodes[0]);
	  }
	  break;

	default:
	  // TODO: may need other types?
	  break;
	}

	break;
      }
    }
  }

  return result;
}

void csBulletDynamicsSystem::SetInternalScale (float scale)
{
  CS_ASSERT(!dynamicBodies.GetSize ());

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
  CS_ASSERT(!dynamicBodies.GetSize ());

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

iBulletSoftBody* csBulletDynamicsSystem::GetSoftBody (size_t index)
{
  CS_ASSERT(isSoftWorld && index < softBodies.GetSize ());
  return softBodies[index];
}

bool csBulletDynamicsSystem::GetSoftBodyWorld ()
{
  return isSoftWorld;
}

iBulletSoftBody* csBulletDynamicsSystem::CreateRope
(csVector3 start, csVector3 end, uint segmentCount)
{
  CS_ASSERT(isSoftWorld);

  btSoftBody* body = btSoftBodyHelpers::CreateRope
    (*softWorldInfo, CSToBullet (start, internalScale),
     CSToBullet (end, internalScale), segmentCount, 0);

  btSoftRigidDynamicsWorld* softWorld =
    static_cast<btSoftRigidDynamicsWorld*> (bulletWorld);
  softWorld->addSoftBody (body);

  csRef<csBulletSoftBody> csBody;
  csBody.AttachNew (new csBulletSoftBody (this, body));

  softBodies.Push (csBody);
  return csBody;
}

iBulletSoftBody* csBulletDynamicsSystem::CreateCloth
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

iBulletSoftBody* csBulletDynamicsSystem::CreateSoftBody
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

iBulletSoftBody* csBulletDynamicsSystem::CreateSoftBody
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

void csBulletDynamicsSystem::RemoveSoftBody (iBulletSoftBody* body)
{
  csBulletSoftBody* csBody = dynamic_cast<csBulletSoftBody*> (body);
  CS_ASSERT (csBody);
  btSoftRigidDynamicsWorld* softWorld =
    static_cast<btSoftRigidDynamicsWorld*> (bulletWorld);
  softWorld->removeSoftBody (csBody->body);

  softBodies.Delete (body);
}

csPtr<iBulletPivotJoint> csBulletDynamicsSystem::CreatePivotJoint ()
{
  csRef<csBulletPivotJoint> joint;
  joint.AttachNew (new csBulletPivotJoint (this));
  pivotJoints.Push (joint);
  return csPtr<iBulletPivotJoint> (joint);
}

void csBulletDynamicsSystem::RemovePivotJoint (iBulletPivotJoint* joint)
{
  pivotJoints.Delete (joint);
}

//-------------------- csBulletRigidBody -----------------------------------

csBulletRigidBody::csBulletRigidBody (csBulletDynamicsSystem* dynSys, bool isStatic)
  : scfImplementationType (this), dynSys (dynSys), body (0),
    dynamicState (isStatic? CS_BULLET_STATE_STATIC : CS_BULLET_STATE_DYNAMIC),
    customMass (false), mass (1.0f), compoundChanged (false), insideWorld (false),
    linearDampening (dynSys->linearDampening), angularDampening (dynSys->angularDampening)
{
  btTransform identity;
  identity.setIdentity ();
  motionState = new csBulletMotionState (this, identity, identity);
  compoundShape = new btCompoundShape ();
  moveCb = dynSys->GetDefaultMoveCallback ();
}

csBulletRigidBody::~csBulletRigidBody ()
{
  if (insideWorld)
    dynSys->bulletWorld->removeRigidBody (body);

  delete body;
  delete motionState;
  delete compoundShape;
}

void csBulletRigidBody::RebuildBody ()
{
  // delete previous body
  bool wasBody = false;
  btVector3 linearVelocity;
  btVector3 angularVelocity;
  if (body)
  {
    // save body's state
    wasBody = true;
    linearVelocity = body->getLinearVelocity ();
    angularVelocity = body->getAngularVelocity ();

    if (insideWorld)
      dynSys->bulletWorld->removeRigidBody (body);

    delete body;
    body = 0;
  }

  // create body infos
  btVector3 localInertia (0.0f, 0.0f, 0.0f);
  void* userPointer (0);
  float bodyMass (0);

  // update the compound shape if changed
  if (compoundChanged)
  {
    compoundChanged = false;

    // create a new compound shape
    btCompoundShape* newCompoundShape = new btCompoundShape ();
    for (unsigned int i = 0; i < colliders.GetSize (); i++)
      if (colliders[i]->shape)
	newCompoundShape->addChildShape(CSToBullet (colliders[i]->localTransform,
						    dynSys->internalScale),
					colliders[i]->shape);

    delete compoundShape;
    compoundShape = newCompoundShape;
    int shapeCount = compoundShape->getNumChildShapes ();

    // compute new principal axis
    if (dynamicState == CS_BULLET_STATE_DYNAMIC)
    {
      // compute the masses of the shapes
      CS_ALLOC_STACK_ARRAY(btScalar, masses, shapeCount); 
      float totalMass = 0.0f;

      // check if a custom mass has been defined
      if (customMass)
      {
	if (shapeCount == 1)
	  masses[0] = mass;

	else
	{
	  CS_ALLOC_STACK_ARRAY(float, volumes, shapeCount); 
	  float totalVolume = 0.0f;

	  // compute the volume of each shape
	  for (int j = 0; j < shapeCount; j++)
	  {
	    volumes[j] = colliders[j]->GetVolume ();
	    totalVolume += volumes[j];
	  }

	  // assign masses
	  for (int j = 0; j < shapeCount; j++)
	    masses[j] = mass * volumes[j] / totalVolume;
	}

	totalMass = mass;
      }

      // if no custom mass defined then use colliders density
      else for (int j = 0; j < shapeCount; j++)
      {
	masses[j] = colliders[j]->density * colliders[j]->GetVolume ();
	totalMass += masses[j];
      }

      // compute principal axis
      btTransform principalAxis;
      btVector3 principalInertia;
      compoundShape->calculatePrincipalAxisTransform
	(masses, principalAxis, principalInertia);

      // create new motion state
      btTransform trans;
      motionState->getWorldTransform (trans);
      trans = trans * motionState->inversePrincipalAxis;
      delete motionState;
      motionState = new csBulletMotionState (this, trans * principalAxis,
					     principalAxis);

      // apply principal axis
      // creation is faster using a new compound to store the shifted children
      newCompoundShape = new btCompoundShape();
      for (int i = 0; i < shapeCount; i++)
      {
	btTransform newChildTransform =
	  principalAxis.inverse() * compoundShape->getChildTransform (i);
	newCompoundShape->addChildShape(newChildTransform,
					compoundShape->getChildShape (i));
      }
      
      delete compoundShape;
      compoundShape = newCompoundShape;

      mass = bodyMass = totalMass;
      compoundShape->calculateLocalInertia (totalMass, localInertia);
      userPointer = (void *) this;
    }
  }

  else if (dynamicState == CS_BULLET_STATE_DYNAMIC)
  {
    // compound hasn't been changed
    bodyMass = mass;
    compoundShape->calculateLocalInertia (bodyMass, localInertia);
    userPointer = (void *) this;
  }

  // don't do anything if there are no valid colliders
  if (!compoundShape->getNumChildShapes ())
    return;

  // create rigid body's info
  btRigidBody::btRigidBodyConstructionInfo infos (bodyMass, motionState,
						  compoundShape, localInertia);

  // TODO: add ability to have different material properties for each collider?
  // TODO: use collider's softness
  infos.m_friction = colliders[0]->friction;
  infos.m_restitution = colliders[0]->elasticity;
  infos.m_linearDamping = linearDampening;
  infos.m_angularDamping = angularDampening;

  // create new rigid body
  body = new btRigidBody (infos);
  body->setUserPointer (userPointer);
  dynSys->bulletWorld->addRigidBody (body);
  insideWorld = true;

  // put back angular/linear velocity
  if (wasBody)
  {
    body->setLinearVelocity (linearVelocity);
    body->setAngularVelocity (angularVelocity);
  }

  // set deactivation parameters
  if (!dynSys->autoDisableEnabled)
    body->setActivationState (DISABLE_DEACTIVATION);
  body->setSleepingThresholds (dynSys->linearDisableThreshold,
			       dynSys->angularDisableThreshold);
  body->setDeactivationTime (dynSys->timeDisableThreshold);

  // TODO: update any connected joints
}

bool csBulletRigidBody::MakeStatic (void)
{
  if (body && dynamicState != CS_BULLET_STATE_STATIC)
  {
    csBulletState previousState = dynamicState;

    // rebuild body if a child collider was a concave mesh
    bool hasTrimesh = false;
    for (unsigned int i = 0; i < colliders.GetSize (); i++)
      if (colliders[i]->shape
	  && colliders[i]->geomType == TRIMESH_COLLIDER_GEOMETRY)
      {
	dynamicState = CS_BULLET_STATE_STATIC;
	colliders[i]->RebuildMeshGeometry ();
	hasTrimesh = true;
      }

    if (hasTrimesh)
	RebuildBody ();

    // remove body from world
    if (insideWorld)
      dynSys->bulletWorld->removeRigidBody (body);

    // set in static state
    body->setCollisionFlags (body->getCollisionFlags()
			     | btCollisionObject::CF_STATIC_OBJECT);
    body->setMassProps (0.0f, btVector3 (0.0f, 0.0f, 0.0f));
    body->updateInertiaTensor ();

    // reverse kinematic state
    if (previousState == CS_BULLET_STATE_KINEMATIC)
    {
      body->setCollisionFlags (body->getCollisionFlags()
			       & ~btCollisionObject::CF_KINEMATIC_OBJECT);
      
      // create new motion state
      btTransform principalAxis = motionState->inversePrincipalAxis.inverse ();
      btTransform trans;
      motionState->getWorldTransform (trans);
      trans = trans * motionState->inversePrincipalAxis;
      delete motionState;
      motionState = new csBulletMotionState
	(this, trans * principalAxis, principalAxis);
      body->setMotionState (motionState);
    }

    // put body back in world
    if (insideWorld)
      dynSys->bulletWorld->addRigidBody (body);
  }

  dynamicState = CS_BULLET_STATE_STATIC;

  return true;
}

bool csBulletRigidBody::MakeDynamic (void)
{
  if (body && dynamicState != CS_BULLET_STATE_DYNAMIC)
  {
    csBulletState previousState = dynamicState;

    // rebuild body if a child collider was a concave mesh
    if (previousState == CS_BULLET_STATE_STATIC)
    {
      bool hasTrimesh = false;
      for (unsigned int i = 0; i < colliders.GetSize (); i++)
	if (colliders[i]->shape
	    && colliders[i]->geomType == TRIMESH_COLLIDER_GEOMETRY)
	{
	  dynamicState = CS_BULLET_STATE_DYNAMIC;
	  colliders[i]->RebuildMeshGeometry ();
	  hasTrimesh = true;
	}

      if (hasTrimesh)
	RebuildBody ();
    }

    // remove body from world
    if (insideWorld)
      dynSys->bulletWorld->removeRigidBody (body);

    // set body dynamic
    body->setCollisionFlags (body->getCollisionFlags()
			     & ~btCollisionObject::CF_STATIC_OBJECT);

    btVector3 linearVelocity (0.0f, 0.0f, 0.0f);
    btVector3 angularVelocity (0.0f, 0.0f, 0.0f);

    // reverse kinematic state
    if (previousState == CS_BULLET_STATE_KINEMATIC)
    {
      body->setCollisionFlags (body->getCollisionFlags()
			       & ~btCollisionObject::CF_KINEMATIC_OBJECT);

      linearVelocity = body->getInterpolationLinearVelocity ();
      angularVelocity = body->getInterpolationAngularVelocity ();

      // create new motion state
      btTransform principalAxis = motionState->inversePrincipalAxis.inverse ();
      btTransform trans;
      motionState->getWorldTransform (trans);
      trans = trans * motionState->inversePrincipalAxis;
      delete motionState;
      motionState = new csBulletMotionState
	(this, trans * principalAxis, principalAxis);
      body->setMotionState (motionState);
    }

    // set body dynamic
    btVector3 localInertia (0.0f, 0.0f, 0.0f);
    compoundShape->calculateLocalInertia (mass, localInertia);
    body->setMassProps (mass, localInertia);

    if (!dynSys->autoDisableEnabled)
      body->setActivationState (DISABLE_DEACTIVATION);
    else
      body->forceActivationState (ACTIVE_TAG);

    body->setLinearVelocity (linearVelocity);
    body->setAngularVelocity (angularVelocity);
    body->updateInertiaTensor ();

    // put body back in world
    if (insideWorld)
      dynSys->bulletWorld->addRigidBody (body);
  }

  dynamicState = CS_BULLET_STATE_DYNAMIC;

  return true;
}

void csBulletRigidBody::MakeKinematic ()
{
  if (body && dynamicState != CS_BULLET_STATE_KINEMATIC)
  {
    csBulletState previousState = dynamicState;

    // rebuild body if a child collider was a concave mesh
    if (previousState == CS_BULLET_STATE_STATIC)
    {
      bool hasTrimesh = false;
      for (unsigned int i = 0; i < colliders.GetSize (); i++)
	if (colliders[i]->shape
	    && colliders[i]->geomType == TRIMESH_COLLIDER_GEOMETRY)
	{
	  dynamicState = CS_BULLET_STATE_KINEMATIC;
	  colliders[i]->RebuildMeshGeometry ();
	  hasTrimesh = true;
	}

      if (hasTrimesh)
	RebuildBody ();
    }

    // remove body from world
    if (insideWorld)
      dynSys->bulletWorld->removeRigidBody (body);

    // check if we need to create a default kinematic callback
    if (!kinematicCb)
      kinematicCb.AttachNew (new csBulletDefaultKinematicCallback ());

    // create new motion state
    btTransform principalAxis = motionState->inversePrincipalAxis.inverse ();
    btTransform trans;
    motionState->getWorldTransform (trans);
    trans = trans * motionState->inversePrincipalAxis;
    delete motionState;
    motionState = new csBulletKinematicMotionState
      (this, trans, principalAxis);
    body->setMotionState (motionState);

    // set body kinematic
    body->setMassProps (0.0f, btVector3 (0.0f, 0.0f, 0.0f));
    body->setCollisionFlags ((body->getCollisionFlags()
			      | btCollisionObject::CF_KINEMATIC_OBJECT)
			     & ~btCollisionObject::CF_STATIC_OBJECT);
    body->setActivationState (DISABLE_DEACTIVATION);    
    body->updateInertiaTensor ();
    body->setInterpolationWorldTransform (body->getWorldTransform ());
    body->setInterpolationLinearVelocity (btVector3(0.0f, 0.0f, 0.0f));
    body->setInterpolationAngularVelocity (btVector3(0.0f, 0.0f, 0.0f));

    // put body back in world
    if (insideWorld)
      dynSys->bulletWorld->addRigidBody (body);
  }

  dynamicState = CS_BULLET_STATE_KINEMATIC;

  return;
}

bool csBulletRigidBody::IsStatic (void)
{
  return dynamicState == CS_BULLET_STATE_STATIC;
}

csBulletState csBulletRigidBody::GetDynamicState () const
{
  return dynamicState;
}

void csBulletRigidBody::SetDynamicState (csBulletState state)
{
  switch (state)
    {
    case CS_BULLET_STATE_STATIC:
      MakeStatic ();
      break;

    case CS_BULLET_STATE_DYNAMIC:
      MakeDynamic ();
      break;

    case CS_BULLET_STATE_KINEMATIC:
      MakeKinematic ();
      break;

    default:
      break;
    }
}

void csBulletRigidBody::SetKinematicCallback (iBulletKinematicCallback* callback)
{
  kinematicCb = callback;
}

iBulletKinematicCallback* csBulletRigidBody::GetKinematicCallback ()
{
  return kinematicCb;
}

bool csBulletRigidBody::Disable (void)
{
  SetAngularVelocity(csVector3(0));
  SetLinearVelocity(csVector3(0));
  body->setInterpolationWorldTransform(body->getWorldTransform());
  if (body)
    body->setActivationState(ISLAND_SLEEPING);
  return false;
}

bool csBulletRigidBody::Enable (void)
{
  if (body)
    body->setActivationState(ACTIVE_TAG);
  return true;
}

bool csBulletRigidBody::IsEnabled (void)
{
  if (body)
    return body->isActive();
  return false;
}

csRef<iBodyGroup> csBulletRigidBody::GetGroup (void)
{
  // @@@ TODO
  return 0;
}

bool csBulletRigidBody::AttachColliderConvexMesh (iMeshWrapper* mesh,
  const csOrthoTransform& trans, float friction, float density,
  float elasticity, float softness)
{
  // create collider
  csRef<csBulletCollider> collider;
  collider.AttachNew (new csBulletCollider (dynSys, this, false));

  collider->SetDensity (density);
  collider->SetFriction (friction);
  collider->SetElasticity (elasticity);
  collider->SetSoftness (softness);
  collider->localTransform = trans;

  // create new shape
  if (!collider->CreateConvexMeshGeometry (mesh))
    return false;

  // add it to the collider list
  colliders.Push (collider);
  compoundChanged = true;

  // rebuild body
  RebuildBody ();

  return true;
}

bool csBulletRigidBody::AttachColliderMesh (iMeshWrapper* mesh,
  const csOrthoTransform& trans, float friction, float density,
  float elasticity, float softness)
{
  // create collider
  csRef<csBulletCollider> collider;
  collider.AttachNew (new csBulletCollider (dynSys, this, false));

  collider->SetDensity (density);
  collider->SetFriction (friction);
  collider->SetElasticity (elasticity);
  collider->SetSoftness (softness);
  collider->localTransform = trans;

  // create new shape
  if (!collider->CreateMeshGeometry (mesh))
    return false;

  // add it to the collider list
  colliders.Push (collider);
  compoundChanged = true;

  // rebuild body
  RebuildBody ();

  return true;
}

bool csBulletRigidBody::AttachColliderCylinder (
    float length, float radius,
    const csOrthoTransform& trans, float friction,
    float density, float elasticity, 
    float softness)
{
  // create collider
  csRef<csBulletCollider> collider;
  collider.AttachNew (new csBulletCollider (dynSys, this, false));

  collider->SetDensity (density);
  collider->SetFriction (friction);
  collider->SetElasticity (elasticity);
  collider->SetSoftness (softness);
  collider->localTransform = trans;

  // create new shape
  if (!collider->CreateCylinderGeometry (length, radius))
    return false;

  // add it to the collider list
  colliders.Push (collider);
  compoundChanged = true;

  // rebuild body
  RebuildBody ();

  return true;
}

bool csBulletRigidBody::AttachColliderCapsule (
    float length, float radius,
    const csOrthoTransform& trans, float friction,
    float density, float elasticity, 
    float softness)
{
  // create collider
  csRef<csBulletCollider> collider;
  collider.AttachNew (new csBulletCollider (dynSys, this, false));

  collider->SetDensity (density);
  collider->SetFriction (friction);
  collider->SetElasticity (elasticity);
  collider->SetSoftness (softness);
  collider->localTransform = trans;

  // create new shape
  if (!collider->CreateCapsuleGeometry (length, radius))
    return false;

  // add it to the collider list
  colliders.Push (collider);
  compoundChanged = true;

  // rebuild body
  RebuildBody ();

  return true;
}

bool csBulletRigidBody::AttachColliderBox (
    const csVector3 &size,
    const csOrthoTransform& trans, float friction,
    float density, float elasticity, 
    float softness)
{
  // create collider
  csRef<csBulletCollider> collider;
  collider.AttachNew (new csBulletCollider (dynSys, this, false));

  collider->SetDensity (density);
  collider->SetFriction (friction);
  collider->SetElasticity (elasticity);
  collider->SetSoftness (softness);
  collider->localTransform = trans;

  // create new shape
  if (!collider->CreateBoxGeometry (size))
    return false;

  // add it to the collider list
  colliders.Push (collider);
  compoundChanged = true;

  // rebuild body
  RebuildBody ();

  return true;
}

bool csBulletRigidBody::AttachColliderSphere (
    float radius, const csVector3& offset,
    float friction, float density, float elasticity,
    float softness)
{
  // create collider
  csRef<csBulletCollider> collider;
  collider.AttachNew (new csBulletCollider (dynSys, this, false));

  collider->SetDensity (density);
  collider->SetFriction (friction);
  collider->SetElasticity (elasticity);
  collider->SetSoftness (softness);
  collider->localTransform.Identity ();
  collider->localTransform.SetOrigin (offset);

  // create new shape
  if (!collider->CreateSphereGeometry (csSphere (csVector3 (0), radius)))
    return false;

  // add it to the collider list
  colliders.Push (collider);
  compoundChanged = true;

  // rebuild body
  RebuildBody ();

  return true;
}

bool csBulletRigidBody::AttachColliderPlane (
    const csPlane3 &plane,
    float friction, float density,
    float elasticity, float softness)
{
  // create collider
  csRef<csBulletCollider> collider;
  collider.AttachNew (new csBulletCollider (dynSys, this, false));

  collider->SetDensity (density);
  collider->SetFriction (friction);
  collider->SetElasticity (elasticity);
  collider->SetSoftness (softness);
  collider->localTransform.Identity ();

  // create new shape
  if (!collider->CreatePlaneGeometry (plane))
    return false;

  // add it to the collider list
  colliders.Push (collider);
  compoundChanged = true;

  // rebuild body
  RebuildBody ();

  return true;
}

void csBulletRigidBody::AttachCollider (iDynamicsSystemCollider* collider)
{
  csBulletCollider* csCollider = dynamic_cast<csBulletCollider*> (collider);
  CS_ASSERT (csCollider);

  // add it to the collider list
  colliders.Push (csCollider);
  compoundChanged = true;

  // rebuild body
  RebuildBody ();
}

void csBulletRigidBody::DestroyColliders ()
{
  // remove colliders
  colliders.DeleteAll ();
  compoundChanged = true;

  // rebuild body
  RebuildBody ();
}

void csBulletRigidBody::DestroyCollider (iDynamicsSystemCollider* collider)
{
  // remove collider
  csBulletCollider* csCollider = dynamic_cast<csBulletCollider*> (collider);
  CS_ASSERT (csCollider);

  colliders.Delete (csCollider);
  compoundChanged = true;

  // rebuild body
  RebuildBody ();
}

void csBulletRigidBody::SetPosition (const csVector3& pos)
{
  // TODO: refuse if kinematic

  // remove body from the world
  if (insideWorld)
    dynSys->bulletWorld->removeRigidBody (body);

  // create new motion state
  // TODO: is it really necessary? 
  btVector3 position = CSToBullet (pos, dynSys->internalScale);
  position = motionState->inversePrincipalAxis.invXform (position);

  btTransform trans;
  motionState->getWorldTransform (trans);
  trans.setOrigin (position);

  btTransform principalAxis = motionState->inversePrincipalAxis.inverse ();
  delete motionState;
  motionState = new csBulletMotionState (this, trans, principalAxis);

  // put back body in the world
  if (body)
    body->setMotionState (motionState);

  if (insideWorld)
    dynSys->bulletWorld->addRigidBody (body);
}

const csVector3 csBulletRigidBody::GetPosition () const
{
  return GetTransform ().GetOrigin ();
}

void csBulletRigidBody::SetOrientation (const csMatrix3& rot)
{
  // remove body from the world
  if (insideWorld)
    dynSys->bulletWorld->removeRigidBody (body);

  // create new motion state
  btMatrix3x3 rotation (CSToBullet (rot));
  btTransform rotTrans (rotation, btVector3 (0.0f, 0.0f, 0.0f));
  rotTrans = rotTrans * motionState->inversePrincipalAxis;
  rotation = rotTrans.getBasis ();

  btTransform trans;
  motionState->getWorldTransform (trans);
  trans.setBasis (rotation);

  btTransform principalAxis = motionState->inversePrincipalAxis.inverse ();
  delete motionState;
  motionState = new csBulletMotionState (this, trans, principalAxis);

  // put back body in the world
  if (body)
    body->setMotionState (motionState);

  if (insideWorld)
    dynSys->bulletWorld->addRigidBody (body);
}

const csMatrix3 csBulletRigidBody::GetOrientation () const
{
  return GetTransform ().GetO2T ();
}

void csBulletRigidBody::SetTransform (const csOrthoTransform& trans)
{
  // remove body from the world
  if (insideWorld)
    dynSys->bulletWorld->removeRigidBody (body);

  // create new motion state
  btTransform tr = CSToBullet (trans, dynSys->internalScale);
  btTransform principalAxis = motionState->inversePrincipalAxis.inverse ();
  delete motionState;
  motionState = new csBulletMotionState (this, tr * principalAxis, principalAxis);

  // put back body in the world
  if (body)
    body->setMotionState (motionState);

  if (insideWorld)
    dynSys->bulletWorld->addRigidBody (body);
}

const csOrthoTransform csBulletRigidBody::GetTransform () const
{
  btTransform trans;
  motionState->getWorldTransform (trans);
  return BulletToCS (trans * motionState->inversePrincipalAxis,
		     dynSys->inverseInternalScale);
}

void csBulletRigidBody::SetLinearVelocity (const csVector3& vel)
{
  CS_ASSERT (body);

  if (dynamicState == CS_BULLET_STATE_DYNAMIC)
  {
    body->setLinearVelocity (CSToBullet (vel, dynSys->internalScale));
    body->activate ();
  }
}

const csVector3 csBulletRigidBody::GetLinearVelocity () const
{
  CS_ASSERT (body);

  const btVector3& vel = body->getLinearVelocity ();
  return BulletToCS (vel, dynSys->inverseInternalScale);
}

void csBulletRigidBody::SetAngularVelocity (const csVector3& vel)
{
  CS_ASSERT (body);

  if (dynamicState == CS_BULLET_STATE_DYNAMIC)
  {
    body->setAngularVelocity (btVector3 (vel.x, vel.y, vel.z));
    body->activate ();
  }
}

const csVector3 csBulletRigidBody::GetAngularVelocity () const
{
  CS_ASSERT (body);

  const btVector3& vel = body->getAngularVelocity ();
  return csVector3 (vel.getX (), vel.getY (), vel.getZ ());
}

void csBulletRigidBody::SetProperties (float mass, const csVector3& center,
                                       const csMatrix3& inertia)
{
  CS_ASSERT (mass >= 0.0);

  this->mass = mass;

  if (mass < SMALL_EPSILON)
  {
    MakeStatic ();
    return;
  }
  else
    customMass = true;

  if (body)
  {
    // TODO: use center and inertia
    btVector3 localInertia (0.0f, 0.0f, 0.0f);
    body->getCollisionShape()->calculateLocalInertia (mass, localInertia);
    body->setMassProps(mass, localInertia);
  }
}

void csBulletRigidBody::GetProperties (float* mass, csVector3* center,
                                       csMatrix3* inertia)
{
  *mass = GetMass ();
  *center = GetCenter ();
  *inertia = GetInertia ();
}

float csBulletRigidBody::GetMass ()
{
  if (dynamicState != CS_BULLET_STATE_DYNAMIC)
    return 0.0f;

  if (body)
    return 1.0 / body->getInvMass ();    

  return mass;
}

csVector3 csBulletRigidBody::GetCenter ()
{
  return BulletToCS (motionState->inversePrincipalAxis.inverse (),
		     dynSys->inverseInternalScale).GetOrigin ();
}

csMatrix3 csBulletRigidBody::GetInertia ()
{
  // @@@ TODO
  return csMatrix3 ();
}

void csBulletRigidBody::AdjustTotalMass (float targetmass)
{
  CS_ASSERT (targetmass >= 0.0);

  this->mass = targetmass;

  if (mass < SMALL_EPSILON)
  {
    MakeStatic ();
    return;
  }
  else
    customMass = true;

  if (body)
  {
    // TODO: update density of colliders?
    btVector3 localInertia (0.0f, 0.0f, 0.0f);
    body->getCollisionShape()->calculateLocalInertia (mass, localInertia);
    body->setMassProps(mass, localInertia);
  }
}

void csBulletRigidBody::AddForce (const csVector3& force)
{
  if (body)
  {
    body->applyImpulse (btVector3 (force.x * dynSys->internalScale,
				   force.y * dynSys->internalScale,
				   force.z * dynSys->internalScale),
			btVector3 (0.0f, 0.0f, 0.0f));
    body->setActivationState(ACTIVE_TAG);
  }
}

void csBulletRigidBody::AddTorque (const csVector3& force)
{
  if (body)
  {
    body->applyTorque (btVector3 (force.x * dynSys->internalScale * dynSys->internalScale,
				  force.y * dynSys->internalScale * dynSys->internalScale,
				  force.z * dynSys->internalScale * dynSys->internalScale));
    body->setActivationState(ACTIVE_TAG);
  }
}

void csBulletRigidBody::AddRelForce (const csVector3& force)
{
  if (!body)
    return;

  csOrthoTransform trans = GetTransform ();
  csVector3 absForce = trans.This2Other (force);
  body->applyImpulse (btVector3 (absForce.x * dynSys->internalScale,
				 absForce.y * dynSys->internalScale,
				 absForce.z * dynSys->internalScale),
		      btVector3 (0.0f, 0.0f, 0.0f));
  body->setActivationState(ACTIVE_TAG);
}

void csBulletRigidBody::AddRelTorque (const csVector3& torque) 
{
  if (!body)
    return;

  csOrthoTransform trans = GetTransform ();
  csVector3 absTorque = trans.This2Other (torque);
  body->applyTorque (btVector3 (absTorque.x * dynSys->internalScale * dynSys->internalScale,
				absTorque.y * dynSys->internalScale * dynSys->internalScale,
				absTorque.z * dynSys->internalScale * dynSys->internalScale));
  body->setActivationState(ACTIVE_TAG);
}

void csBulletRigidBody::AddForceAtPos (const csVector3& force,
    const csVector3& pos)
{
  if (!body)
    return;

  btVector3 btForce (force.x * dynSys->internalScale,
		     force.y * dynSys->internalScale,
		     force.z * dynSys->internalScale);
  csOrthoTransform trans = GetTransform ();
  csVector3 relPos = trans.Other2This (pos);

  body->applyImpulse (btForce, btVector3 (relPos.x * dynSys->internalScale,
					  relPos.y * dynSys->internalScale,
					  relPos.z * dynSys->internalScale));
  body->setActivationState(ACTIVE_TAG);
}

void csBulletRigidBody::AddForceAtRelPos (const csVector3& force,
                                          const csVector3& pos)
{
  if (body)
  {
    body->applyImpulse (btVector3 (force.x * dynSys->internalScale,
				   force.y * dynSys->internalScale,
				   force.z * dynSys->internalScale),
			btVector3 (pos.x * dynSys->internalScale,
				   pos.y * dynSys->internalScale,
				   pos.z * dynSys->internalScale));
    body->setActivationState(ACTIVE_TAG);
  }
}

void csBulletRigidBody::AddRelForceAtPos (const csVector3& force,
                                          const csVector3& pos)
{
  if (!body)
    return;

  csOrthoTransform trans = GetTransform ();
  csVector3 absForce = trans.This2Other (force);
  csVector3 relPos = trans.Other2This (pos);
  body->applyImpulse (btVector3 (absForce.x * dynSys->internalScale,
				 absForce.y * dynSys->internalScale,
				 absForce.z * dynSys->internalScale),
		      btVector3 (relPos.x * dynSys->internalScale,
				 relPos.y * dynSys->internalScale,
				 relPos.z * dynSys->internalScale));
  body->setActivationState(ACTIVE_TAG);
}

void csBulletRigidBody::AddRelForceAtRelPos (const csVector3& force,
                                             const csVector3& pos)
{
  if (!body)
    return;

  csOrthoTransform trans = GetTransform ();
  csVector3 absForce = trans.This2Other (force);
  body->applyImpulse (btVector3 (absForce.x * dynSys->internalScale,
				 absForce.y * dynSys->internalScale,
				 absForce.z * dynSys->internalScale),
		      btVector3 (pos.x * dynSys->internalScale,
				 pos.y * dynSys->internalScale,
				 pos.z * dynSys->internalScale));
  body->setActivationState(ACTIVE_TAG);
}

const csVector3 csBulletRigidBody::GetForce () const
{
  if (!body)
    return csVector3 (0);

  btVector3 force = body->getTotalForce ();
  return csVector3 (force.getX () * dynSys->inverseInternalScale,
		    force.getY () * dynSys->inverseInternalScale,
		    force.getZ () * dynSys->inverseInternalScale);
}

const csVector3 csBulletRigidBody::GetTorque () const
{
  if (!body)
    return csVector3 (0);

  btVector3 torque = body->getTotalTorque ();
  return csVector3
    (torque.getX () * dynSys->inverseInternalScale * dynSys->inverseInternalScale,
     torque.getY () * dynSys->inverseInternalScale * dynSys->inverseInternalScale,
     torque.getZ () * dynSys->inverseInternalScale * dynSys->inverseInternalScale);
}

void csBulletRigidBody::AttachMesh (iMeshWrapper* mesh)
{
  this->mesh = mesh;

  // TODO: put the mesh in the good sector?

  if (mesh && moveCb)
  {
    csOrthoTransform tr = GetTransform ();
    moveCb->Execute (mesh, tr);
  }
}

iMeshWrapper* csBulletRigidBody::GetAttachedMesh ()
{
  return mesh;
}

void csBulletRigidBody::AttachLight (iLight* light)
{
  this->light = light;

  // TODO: put it in the good sector?

  if (light && moveCb)
  {
    csOrthoTransform tr = GetTransform ();
    moveCb->Execute (light, tr);
  }
}

iLight* csBulletRigidBody::GetAttachedLight ()
{
  return light;
}

void csBulletRigidBody::AttachCamera (iCamera* camera)
{
  this->camera = camera;

  // TODO: put it in the good sector?

  if (camera && moveCb)
  {
    csOrthoTransform tr = GetTransform ();
    moveCb->Execute (camera, tr);
  }
}

iCamera* csBulletRigidBody::GetAttachedCamera ()
{
  return camera;
}

void csBulletRigidBody::SetMoveCallback (iDynamicsMoveCallback* cb)
{
  moveCb = cb;
}

void csBulletRigidBody::SetCollisionCallback (iDynamicsCollisionCallback* cb)
{
  collCb = cb;
}

void csBulletRigidBody::Collision (iRigidBody * other, const csVector3& pos,
      const csVector3& normal, float depth)
{
  if (collCb)
    collCb->Execute (this, other, pos, normal, depth);
}

csRef<iDynamicsSystemCollider> csBulletRigidBody::GetCollider (unsigned int index)
{
  return colliders[index];
}

int csBulletRigidBody::GetColliderCount ()
{
  return (int)colliders.GetSize ();
}

void csBulletRigidBody::Update ()
{
  if (body && moveCb)
  {
    csOrthoTransform trans = GetTransform ();
    if (mesh) moveCb->Execute (mesh, trans);
    if (light) moveCb->Execute (light, trans);
    if (camera) moveCb->Execute (camera, trans);

    // remainder case for all other callbacks
    moveCb->Execute (trans);
  }
}

void csBulletRigidBody::SetLinearDampener (float d)
{
  linearDampening = d;

  if (body)
    body->setDamping (linearDampening, angularDampening);
}

float csBulletRigidBody::GetLinearDampener () const
{
  return linearDampening;
}

void csBulletRigidBody::SetRollingDampener (float d)
{
  angularDampening = d;

  if (body)
    body->setDamping (linearDampening, angularDampening);
}

float csBulletRigidBody::GetRollingDampener () const
{
  return angularDampening;
}

//--------------------- csBulletDefaultMoveCallback -------------------------

csBulletDefaultMoveCallback::csBulletDefaultMoveCallback () 
  : scfImplementationType (this)
{
}

csBulletDefaultMoveCallback::~csBulletDefaultMoveCallback ()
{
}

void csBulletDefaultMoveCallback::Execute (iMovable* movable, csOrthoTransform& t)
{
  // Dont do anything if nothing has changed
  // @@@ TODO Is comparing that transform efficient and correct?
  if (movable->GetPosition () == t.GetOrigin () &&
      movable->GetTransform ().GetT2O () == t.GetT2O ())
    return;

  // Update movable
  movable->SetTransform (t);
  movable->UpdateMove ();
}

void csBulletDefaultMoveCallback::Execute (iMeshWrapper* mesh,
                                           csOrthoTransform& t)
{
  Execute (mesh->GetMovable (), t);
}

void csBulletDefaultMoveCallback::Execute (iLight* light, csOrthoTransform& t)
{
  Execute (light->GetMovable (), t);
}

void csBulletDefaultMoveCallback::Execute (iCamera* camera, csOrthoTransform& t)
{
  // Dont do anything if nothing has changed
  csOrthoTransform& cameraTrans = camera->GetTransform ();
  if (cameraTrans.GetOrigin () == t.GetOrigin () &&
    cameraTrans.GetT2O () == t.GetT2O ())
    return;

  // Update camera position
  cameraTrans.SetOrigin (t.GetOrigin ());
  cameraTrans.SetT2O (t.GetT2O ());
}

void csBulletDefaultMoveCallback::Execute (csOrthoTransform&)
{
  /* do nothing by default */
}

//--------------------- csBulletDefaultKinematicCallback -----------------------------------
csBulletDefaultKinematicCallback::csBulletDefaultKinematicCallback ()
  : scfImplementationType (this)
{
}

csBulletDefaultKinematicCallback::~csBulletDefaultKinematicCallback ()
{
}

void csBulletDefaultKinematicCallback::GetBodyTransform
(iRigidBody* body, csOrthoTransform& transform) const
{
  iMeshWrapper* mesh = body->GetAttachedMesh ();
  if (mesh)
  {
    transform = mesh->GetMovable ()->GetTransform ();
    return;
  }

  iLight* light = body->GetAttachedLight ();
  if (light)
  {
    transform = light->GetMovable ()->GetTransform ();
    return;
  }

  iCamera* camera = body->GetAttachedCamera ();
  if (camera)
  {
    transform = camera->GetTransform ();
    return;
  }  
}

//--------------------- csBulletCollider -----------------------------------

csBulletCollider::csBulletCollider (csBulletDynamicsSystem* dynSys,
				    csBulletRigidBody* body, bool isStaticBody)
  :  scfImplementationType (this), dynSys (dynSys), body (body),
     isStaticBody (isStaticBody), geomType (NO_GEOMETRY), shape (0),
     density (0.1f), friction (0.5f), softness (0.0f), elasticity (0.2f),
     vertices (0), indices (0)
{
}

csBulletCollider::~csBulletCollider ()
{
  delete shape;
  delete[] vertices;
  delete[] indices;
}

bool csBulletCollider::CreateSphereGeometry (const csSphere& sphere)
{
  // TODO: the body won't be set if one create a body, then AttachCollider,
  //       then CreateGeometry on the collider

  delete shape; shape = 0;
  delete[] vertices; vertices = 0;
  delete[] indices; indices = 0;

  shape = new btSphereShape (sphere.GetRadius () * dynSys->internalScale);
  geomType = SPHERE_COLLIDER_GEOMETRY;

  if (isStaticBody)
  {
    // TODO: add local transform?
    localTransform.Identity ();
    if (!(sphere.GetCenter () < 0.0001f))
      localTransform.SetOrigin (sphere.GetCenter ());

    body->compoundChanged = true;
    body->RebuildBody ();
  }

  return true;
}

bool csBulletCollider::CreatePlaneGeometry (const csPlane3& plane)
{
  delete shape; shape = 0;
  delete[] vertices; vertices = 0;
  delete[] indices; indices = 0;

  // Bullet doesn't support dynamic plane shapes
  if (!isStaticBody && body->dynamicState != CS_BULLET_STATE_STATIC)
    return false;
  
  csVector3 normal = plane.GetNormal ();
  shape = new btStaticPlaneShape (btVector3 (normal.x, normal.y, normal.z),
				  plane.D () * dynSys->internalScale);                                       
  geomType = PLANE_COLLIDER_GEOMETRY;

  if (isStaticBody)
  {
    body->compoundChanged = true;
    body->RebuildBody ();
  }

  return true;
}

bool csBulletCollider::CreateConvexMeshGeometry (iMeshWrapper* mesh)
{
  delete shape; shape = 0;
  delete[] vertices; vertices = 0;
  delete[] indices; indices = 0;

  btTriangleIndexVertexArray* indexVertexArrays =
    GenerateTriMeshData (mesh, indices, triangleCount, vertices, vertexCount,
			 dynSys->baseId, dynSys->colldetId, dynSys->internalScale);
  if (!indexVertexArrays)
    return false;

  //shape = new btConvexTriangleMeshShape (indexVertexArrays);
  btConvexHullShape* convexShape = new btConvexHullShape ();
  for (size_t i = 0; i < vertexCount; i++)
    convexShape->addPoint(*(vertices + i));
  shape = convexShape;

  geomType = CONVEXMESH_COLLIDER_GEOMETRY;

  if (isStaticBody)
  {
    body->compoundChanged = true;
    body->RebuildBody ();
  }

  return true;
}

bool csBulletCollider::CreateMeshGeometry (iMeshWrapper* mesh)
{
  delete shape; shape = 0;
  delete[] vertices; vertices = 0;
  delete[] indices; indices = 0;

  btTriangleIndexVertexArray* indexVertexArrays =
    GenerateTriMeshData (mesh, indices, triangleCount, vertices, vertexCount,
			 dynSys->baseId, dynSys->colldetId, dynSys->internalScale);
  if (!indexVertexArrays)
    return false;

  // this shape is optimized for static concave meshes
  if (isStaticBody || body->dynamicState == CS_BULLET_STATE_STATIC)
  {
    btBvhTriangleMeshShape* concaveShape =
      new btBvhTriangleMeshShape (indexVertexArrays, true);
    // it seems to work without that line, to be tested if problems occur
    //concaveShape->refitTree (btVector3 (-1000, -1000, -1000),
    //                         btVector3 (1000, 1000, 1000));
    shape = concaveShape;
  }

  // this one is for dynamic meshes
  else
  {
    dynSys->RegisterGimpact ();
    btGImpactMeshShape* concaveShape = new btGImpactMeshShape (indexVertexArrays);
    concaveShape->updateBound();
    shape = concaveShape;
  }
  geomType = TRIMESH_COLLIDER_GEOMETRY;

  if (isStaticBody)
  {
    body->compoundChanged = true;
    body->RebuildBody ();
  }

  return true;
}

void csBulletCollider::RebuildMeshGeometry ()
{
  if (geomType != TRIMESH_COLLIDER_GEOMETRY
      || !triangleCount)
    return;

  btTriangleIndexVertexArray* indexVertexArrays =
    new btTriangleIndexVertexArray ((int)triangleCount, indices, 3 * sizeof (int),
	      (int)vertexCount, (btScalar*) &vertices[0].x (), sizeof (btVector3));

  // this shape is optimized for static concave meshes
  if (isStaticBody || body->dynamicState == CS_BULLET_STATE_STATIC)
  {
    btBvhTriangleMeshShape* concaveShape =
      new btBvhTriangleMeshShape (indexVertexArrays, true);
    // it seems to work without that line, to be tested if problems occur
    //concaveShape->refitTree (btVector3 (-1000, -1000, -1000),
    //                         btVector3 (1000, 1000, 1000));
    shape = concaveShape;
  }

  // this one is for dynamic meshes
  else
  {
    dynSys->RegisterGimpact ();
    btGImpactMeshShape* concaveShape = new btGImpactMeshShape (indexVertexArrays);
    concaveShape->updateBound();
    shape = concaveShape;
  }
}

bool csBulletCollider::CreateBoxGeometry (const csVector3& size)
{
  delete shape; shape = 0;
  delete[] vertices; vertices = 0;
  delete[] indices; indices = 0;

  shape = new btBoxShape (CSToBullet (size * 0.5f, dynSys->internalScale));
  geomType = BOX_COLLIDER_GEOMETRY;

  if (isStaticBody)
  {
    body->compoundChanged = true;
    body->RebuildBody ();
  }

  return true;
}

bool csBulletCollider::CreateCylinderGeometry (float length,
  float radius)
{
  delete shape; shape = 0;
  delete[] vertices; vertices = 0;
  delete[] indices; indices = 0;

  shape = new btCylinderShapeZ (btVector3 (radius * dynSys->internalScale,
					   radius * dynSys->internalScale,
					   length * dynSys->internalScale * 0.5f));
  geomType = CYLINDER_COLLIDER_GEOMETRY;

  if (isStaticBody)
  {
    body->compoundChanged = true;
    body->RebuildBody ();
  }

  return true;
}

bool csBulletCollider::CreateCapsuleGeometry (float length,
  float radius)
{
  delete shape; shape = 0;
  delete[] vertices; vertices = 0;
  delete[] indices; indices = 0;

  shape = new btCapsuleShapeZ (radius * dynSys->internalScale,
			       length * dynSys->internalScale);
  geomType = CAPSULE_COLLIDER_GEOMETRY;

  if (isStaticBody)
  {
    body->compoundChanged = true;
    body->RebuildBody ();
  }

  return true;
}

void csBulletCollider::SetCollisionCallback (
  iDynamicsColliderCollisionCallback* cb)
{
  collCb = cb;
}

void csBulletCollider::SetFriction (float friction)
{
  // @@@ TODO: check value range
  this->friction = friction;
  // @@@ TODO: update body
}

void csBulletCollider::SetSoftness (float softness)
{
  // @@@ TODO: check value range
  this->softness = softness;
  // @@@ TODO: update body
}

void csBulletCollider::SetDensity (float density)
{
  // @@@ TODO: check value range
  this->density = density * dynSys->inverseInternalScale
    * dynSys->inverseInternalScale * dynSys->inverseInternalScale;
  // @@@ TODO: update body
}

void csBulletCollider::SetElasticity (float elasticity)
{
  // @@@ TODO: check value range
  this->elasticity = elasticity;
  // @@@ TODO: update body
}

float csBulletCollider::GetFriction ()
{
  return friction;
}

float csBulletCollider::GetSoftness ()
{
  return softness;
}

float csBulletCollider::GetDensity ()
{
  return density * dynSys->internalScale * dynSys->internalScale * dynSys->internalScale;
}

float csBulletCollider::GetElasticity ()
{
  return elasticity;
}

void csBulletCollider::FillWithColliderGeometry (
    csRef<iGeneralFactoryState> genmesh_fact)
{
  // @@@ TODO
#if 0
  switch (geomType)
  {
    case BOX_COLLIDER_GEOMETRY:
    {
      SimdTransform trans;
      SimdVector3 max;
      SimdVector3 min;
      BoxShape*  b = (BoxShape*)pc->GetRigidBody ()->GetCollisionShape ();

      csBox3 box;
      for (int i = 0; i < b->GetNumVertices (); i++)
      {
        SimdVector3 vtx;
        b->GetVertex (i, vtx); 
        box.AddBoundingVertexTest (csVector3 (vtx[0], vtx[1], vtx[2]));
      }
      genmesh_fact->GenerateBox (box);
      genmesh_fact->CalculateNormals (); 
    }
    break;
    default:
    break;
  }
#endif
}

csOrthoTransform csBulletCollider::GetTransform ()
{
  csOrthoTransform trans = body->GetTransform ();
  return localTransform * trans;
}

csOrthoTransform csBulletCollider::GetLocalTransform ()
{
  if (isStaticBody || body->dynamicState == CS_BULLET_STATE_STATIC)
    return localTransform * body->GetTransform ();

  return localTransform;
}

void csBulletCollider::SetTransform (const csOrthoTransform& trans)
{
  if (isStaticBody)
    body->SetTransform (localTransform.GetInverse () * trans);

  else
  {
    localTransform = trans;
    body->compoundChanged = true;
    body->RebuildBody ();
  }
}

bool csBulletCollider::GetBoxGeometry (csVector3& size)
{
  if (geomType != BOX_COLLIDER_GEOMETRY)
    return false;

  btBoxShape* geometry = static_cast<btBoxShape*> (shape);
  btVector3 btSize = geometry->getHalfExtentsWithMargin ();
  size.Set (BulletToCS (btSize, dynSys->inverseInternalScale));
  size *= 2.0f;

  return true;
}

bool csBulletCollider::GetSphereGeometry (csSphere& sphere)
{
  if (geomType != SPHERE_COLLIDER_GEOMETRY)
    return false;

  btSphereShape* geometry = static_cast<btSphereShape*> (shape);
  sphere.SetCenter (localTransform.GetOrigin ());
  sphere.SetRadius (geometry->getRadius () * dynSys->inverseInternalScale);

  return true;
}

bool csBulletCollider::GetPlaneGeometry (csPlane3& plane)
{
  // TODO
  return false;
}

bool csBulletCollider::GetCylinderGeometry (float& length,
  float& radius)
{
  if (geomType != CYLINDER_COLLIDER_GEOMETRY)
    return false;

  btCylinderShapeZ* geometry = static_cast<btCylinderShapeZ*> (shape);
  btVector3 btSize = geometry->getHalfExtentsWithMargin ();
  radius = btSize.getX () * dynSys->inverseInternalScale;
  length = btSize.getZ () * 2.0f * dynSys->inverseInternalScale;

  return true;
}

bool csBulletCollider::GetCapsuleGeometry (float& length,
  float& radius)
{
  if (geomType != CAPSULE_COLLIDER_GEOMETRY)
    return false;

  btCapsuleShapeZ* geometry = static_cast<btCapsuleShapeZ*> (shape);
  radius = geometry->getRadius () * dynSys->inverseInternalScale;
  length = geometry->getHalfHeight () * 2.0f * dynSys->inverseInternalScale;

  return true;
}

bool csBulletCollider::GetMeshGeometry (csVector3*& vertices, size_t& vertexCount,
					int*& indices, size_t& triangleCount)
{
  if (geomType != TRIMESH_COLLIDER_GEOMETRY)
    return false;

  triangleCount = this->triangleCount;
  delete[] indices;
  indices = new int[this->triangleCount * 3];
  for (unsigned int i = 0; i < triangleCount * 3; i++)
    indices[i] = this->indices[i];

  vertexCount = this->vertexCount;
  delete[] vertices;
  vertices = new csVector3[this->vertexCount];
  for (unsigned int i = 0; i < vertexCount; i++)
    vertices[i].Set (BulletToCS (this->vertices[i], dynSys->inverseInternalScale));

  return true;
}

bool csBulletCollider::GetConvexMeshGeometry (csVector3*& vertices, size_t& vertexCount,
					      int*& indices, size_t& triangleCount)
{
  if (geomType != CONVEXMESH_COLLIDER_GEOMETRY)
    return false;

  triangleCount = this->triangleCount;
  delete[] indices;
  indices = new int[this->triangleCount * 3];
  for (unsigned int i = 0; i < triangleCount * 3; i++)
    indices[i] = this->indices[i];

  vertexCount = this->vertexCount;
  delete[] vertices;
  vertices = new csVector3[this->vertexCount];
  for (unsigned int i = 0; i < vertexCount; i++)
    vertices[i].Set (BulletToCS (this->vertices[i], dynSys->inverseInternalScale));

  return true;
}

void csBulletCollider::MakeStatic ()
{
  // nonsense?
}

void csBulletCollider::MakeDynamic ()
{
  // nonsense?
}

bool csBulletCollider::IsStatic ()
{
  return isStaticBody || body->dynamicState == CS_BULLET_STATE_STATIC;
}

float csBulletCollider::GetVolume ()
{
  switch (geomType)
  {
    case BOX_COLLIDER_GEOMETRY:
      {
	csVector3 size;
	GetBoxGeometry (size);
	return size[0] * size[1] * size[2];
      }

    case SPHERE_COLLIDER_GEOMETRY:
      {
	csSphere sphere;
	GetSphereGeometry (sphere);
	return 1.333333f * PI * sphere.GetRadius () * sphere.GetRadius ()
	  * sphere.GetRadius ();
      }

    case CYLINDER_COLLIDER_GEOMETRY:
      {
	float length;
	float radius;
	GetCylinderGeometry (length, radius);
	return PI * radius * radius * length;
      }

    case CAPSULE_COLLIDER_GEOMETRY:
      {
	float length;
	float radius;
	GetCapsuleGeometry (length, radius);
	return PI * radius * radius * length
	  + 1.333333f * PI * radius * radius * radius;
      }

    case CONVEXMESH_COLLIDER_GEOMETRY:
      {
	if (vertexCount == 0)
	  return 0.0f;

	float volume = 0.0f;
	int faceCount = (int)vertexCount / 3;
	btVector3 origin = vertices[indices[0]];
	for (int i = 1; i < faceCount; i++)
	{
	  int index = i * 3;
	  volume += fabsl (btDot
			   (vertices[indices[index]] - origin,
			    btCross (vertices[indices[index + 1]] - origin,
				     vertices[indices[index + 2]] - origin)));
	}

	return volume / 6.0f;
      }

    case TRIMESH_COLLIDER_GEOMETRY:
      {
	if (vertexCount == 0)
	  return 0.0f;

	// TODO: this is a really rough estimation
	btVector3 center;
	btScalar radius;
	shape->getBoundingSphere (center, radius);
	return 1.333333f * PI * radius * radius * radius;
      }

  default:
    return 0.0f;
  }

  return 0.0f;
}

}
CS_PLUGIN_NAMESPACE_END(Bullet)
