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
#include "csgeom/matrix3.h"
#include "csgeom/transfrm.h"
#include "csgeom/quaternion.h"
#include "csgeom/vector3.h"
#include "csgeom/sphere.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "imesh/genmesh.h"

// Bullet includes.
#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"

#include "bullet.h"

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(Bullet)
{

SCF_IMPLEMENT_FACTORY (csBulletDynamics)

//---------------------------------------------------------------------------

static csReversibleTransform BulletToCS (const btTransform& trans)
{
  const btVector3& trans_o = trans.getOrigin ();
  csVector3 origin (trans_o.getX (), trans_o.getY (), trans_o.getZ ());
  const btMatrix3x3& trans_m = trans.getBasis ();
  const btVector3& row0 = trans_m.getRow (0);
  const btVector3& row1 = trans_m.getRow (1);
  const btVector3& row2 = trans_m.getRow (2);
  csMatrix3 m (
      row0.getX (), row0.getY (), row0.getZ (),
      row1.getX (), row1.getY (), row1.getZ (),
      row2.getX (), row2.getY (), row2.getZ ());
  return csReversibleTransform (m, origin);
}

static btTransform CSToBullet (const csReversibleTransform& tr)
{
  const csVector3& origin = tr.GetOrigin ();
  btVector3 trans_o (origin.x, origin.y, origin.z);
  const csMatrix3& m = tr.GetO2T ();
  btMatrix3x3 trans_m (
      m.m11, m.m12, m.m13,
      m.m21, m.m22, m.m23,
      m.m31, m.m32, m.m33);
  return btTransform (trans_m, trans_o);
}

//---------------------------------------------------------------------------

class csBulletMotionState : public btDefaultMotionState
{
private:
  iMeshWrapper* mesh;
  csRef<iDynamicsMoveCallback> move_cb;
  bool use_offset;
  csVector3 offset;
  bool use_transform;
  csReversibleTransform offset_transform;

public:
  csBulletMotionState (iMeshWrapper* mesh, const btTransform& trans)
      : btDefaultMotionState (trans), mesh (mesh)
  {
    use_offset = false;
    use_transform = false;
  }
  virtual ~csBulletMotionState () { }

  void SetMesh (iMeshWrapper* mesh)
  {
    csBulletMotionState::mesh = mesh;
  }

  void SetOffset (const csVector3& v)
  {
    use_offset = true;
    use_transform = false;
    offset = v;
  }
  void SetOffsetTransform (const csReversibleTransform& tr)
  {
    use_transform = true;
    use_offset = false;
    offset_transform = tr.GetInverse ();
  }

  void SetMoveCallback (iDynamicsMoveCallback* cb)
  {
    move_cb = cb;
  }
  iDynamicsMoveCallback* GetMoveCallback ()
  {
    return move_cb;
  }

  virtual void setWorldTransform (const btTransform& trans)
  {
    btDefaultMotionState::setWorldTransform (trans);
    if (!mesh || !move_cb) return;
    csOrthoTransform tr = BulletToCS (trans);
    if (use_offset)
    {
      csVector3 troffset = mesh->GetMovable ()->GetTransform ()
	.This2OtherRelative (-offset);
      tr.Translate (troffset);
    }
    else if (use_transform)
    {
      tr = offset_transform * tr;
    }
    move_cb->Execute (mesh, tr);
  }
};

//---------------------------------------------------------------------------

csBulletDynamics::csBulletDynamics (iBase *iParent)
  : scfImplementationType (this, iParent)
{
  configuration = 0;
  dispatcher = 0;
  solver = 0;
  broadphase = 0;
}

csBulletDynamics::~csBulletDynamics ()
{
  delete configuration;
  delete dispatcher;
  delete solver;
  delete broadphase;
}

bool csBulletDynamics::Initialize (iObjectRegistry* object_reg)
{
  csBulletDynamics::object_reg = object_reg;

  configuration = new btDefaultCollisionConfiguration ();
  dispatcher = new btCollisionDispatcher (configuration);
  solver = new btSequentialImpulseConstraintSolver;

  const int maxProxies = 32766;
  btVector3 worldAabbMin (-1000,-1000,-1000);
  btVector3 worldAabbMax (1000,1000,1000);
  broadphase = new btAxisSweep3 (
      worldAabbMin, worldAabbMax, maxProxies);

  return true;
}

csPtr<iDynamicSystem> csBulletDynamics::CreateSystem ()
{
  btDynamicsWorld* world = new btDiscreteDynamicsWorld (dispatcher,
      broadphase, solver, configuration);
  csBulletDynamicsSystem* system = new csBulletDynamicsSystem (world);

  iDynamicSystem* isystem = static_cast<iDynamicSystem*> (system);
  systems.Push (isystem);
  return (csPtr<iDynamicSystem>)isystem;
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
  for (size_t i = 0; i < systems.GetSize (); i++)
  {
    systems[i]->Step (stepsize);
  }

  // Step callbacks.
  for (size_t i = 0; i < step_callbacks.GetSize (); i++)
  {
    step_callbacks[i]->Step (stepsize);
  }
}

//----------------------- csBulletDynamicsSystem ----------------------------

csBulletDynamicsSystem::csBulletDynamicsSystem (btDynamicsWorld* world)
:  scfImplementationType (this)
{
  bullet_world = world;
  move_cb.AttachNew (new csBulletDefaultMoveCallback ());
  SetGravity (csVector3 (0, -10, 0));
}

csBulletDynamicsSystem::~csBulletDynamicsSystem ()
{
  bodies.DeleteAll ();
  colliders.DeleteAll ();
  delete bullet_world;
}

void csBulletDynamicsSystem::SetGravity (const csVector3& v)
{
  gravity = v; 
  bullet_world->setGravity(btVector3 (v.x, v.y, v.z));
}

const csVector3 csBulletDynamicsSystem::GetGravity () const
{
  //@@@ Get from bullet?
  return gravity;
}

void csBulletDynamicsSystem::SetLinearDampener (float)
{
}

float csBulletDynamicsSystem::GetLinearDampener () const
{
  return 0;
}

void csBulletDynamicsSystem::SetRollingDampener (float)
{
}

float csBulletDynamicsSystem::GetRollingDampener () const
{
  return 0;
}

void csBulletDynamicsSystem::EnableAutoDisable (bool)
{
}

bool csBulletDynamicsSystem::AutoDisableEnabled ()
{
  return false;
}

void csBulletDynamicsSystem::SetAutoDisableParams (float /*linear*/,
  float /*angular*/, int /*steps*/, float /*time*/)
{
}

void csBulletDynamicsSystem::Step (float stepsize)
{
  bullet_world->stepSimulation (stepsize);
}

csPtr<iRigidBody> csBulletDynamicsSystem::CreateBody ()
{
  csBulletRigidBody* b = new csBulletRigidBody (this);

  iRigidBody* ib = static_cast<iRigidBody*> (b);
  bodies.Push (ib);
  return (csPtr<iRigidBody>) ib;
}

void csBulletDynamicsSystem::RemoveBody (iRigidBody* body)
{
  bodies.Delete (body);
}

iRigidBody* csBulletDynamicsSystem::FindBody (const char* name)
{
  return bodies.FindByName (name);
}

iRigidBody* csBulletDynamicsSystem::GetBody (unsigned int index)
{
  return bodies[index];
}

int csBulletDynamicsSystem::GetBodysCount ()
{
  return bodies.GetSize ();
}

csPtr<iBodyGroup> csBulletDynamicsSystem::CreateGroup ()
{
  return 0;
}

void csBulletDynamicsSystem::RemoveGroup (iBodyGroup*)
{
}

csPtr<iJoint> csBulletDynamicsSystem::CreateJoint ()
{
  return 0;
}

void csBulletDynamicsSystem::RemoveJoint (iJoint*)
{
}

iDynamicsMoveCallback* csBulletDynamicsSystem::GetDefaultMoveCallback ()
{
  return move_cb;
}

bool csBulletDynamicsSystem::AttachColliderMesh (iMeshWrapper*,
  const csOrthoTransform&, float /*friction*/,
  float /*elasticity*/, float /*softness*/)
{
  return false;
}

bool csBulletDynamicsSystem::AttachColliderCylinder (float /*length*/,
  float /*radius*/, const csOrthoTransform&, float /*frictio*/,
  float /*elasticity*/, float /*softness*/)
{
  return false;
}

bool csBulletDynamicsSystem::AttachColliderBox (const csVector3 &size,
  const csOrthoTransform& trans, float friction,
  float elasticity, float softness)
{
  csBulletCollider *bulletc = new csBulletCollider (this);
  bulletc->SetElasticity (elasticity);
  bulletc->SetFriction (friction);
  bulletc->SetSoftness (softness);

  bulletc->SetTransform (trans);
  bulletc->CreateBoxGeometry (size);
  colliders.Push (bulletc);
  bulletc->DecRef ();

  return true;
}

bool csBulletDynamicsSystem::AttachColliderSphere (float /*radius*/,
  const csVector3 &/*offset*/, float /*friction*/,
  float /*elasticity*/, float /*softness*/)
{
  return false;
}

bool csBulletDynamicsSystem::AttachColliderPlane (const csPlane3 &,
  float /*friction*/, float /*elasticity*/, float /*softness*/)
{
  return false;
}

void csBulletDynamicsSystem::AttachCollider (iDynamicsSystemCollider*)
{
}

csRef<iDynamicsSystemCollider> csBulletDynamicsSystem::CreateCollider () 
{
  csRef<csBulletCollider> b;
  b.AttachNew (new csBulletCollider (this));

  iDynamicsSystemCollider* ib = static_cast<iDynamicsSystemCollider*> (b);
  colliders.Push (ib);
  return b;
}

//-------------------- csBulletRigidBody -----------------------------------

csBulletRigidBody::csBulletRigidBody (csBulletDynamicsSystem* dynsys)
  : scfImplementationType (this)
{
  ds = dynsys;
  body = 0;
  mass = 1.0f;
  btTransform trans;
  trans.setIdentity ();
  motionState = new csBulletMotionState (0, trans);
  motionState->SetMoveCallback (dynsys->GetDefaultMoveCallback ());
}

csBulletRigidBody::~csBulletRigidBody ()
{
  if (body)
  {
    ds->GetWorld ()->removeRigidBody (body);
    delete body;
  }
  delete motionState;
}

bool csBulletRigidBody::MakeStatic (void)
{
  return true;
}

bool csBulletRigidBody::MakeDynamic (void)
{
  return false;
}

bool csBulletRigidBody::IsStatic (void)
{
  return false;
}

bool csBulletRigidBody::Disable (void)
{
  return false;
}

bool csBulletRigidBody::Enable (void)
{
  //pc->SetActive (true);
  return true;
}

bool csBulletRigidBody::IsEnabled (void)
{
  //return pc->GetRigidBody()->GetActivationState() == 1;
  return true;
}

csRef<iBodyGroup> csBulletRigidBody::GetGroup (void)
{
  return 0;
}

bool csBulletRigidBody::AttachColliderMesh (iMeshWrapper*,
  const csOrthoTransform&, float /*friction*/, float /*density*/,
  float /*elasticity*/, float /*softness*/)
{
  return false;
}

bool csBulletRigidBody::AttachColliderCylinder (
    float /*length*/, float /*radius*/,
    const csOrthoTransform& /*trans*/, float /*friction*/,
    float /*density*/, float /*elasticity*/, 
    float /*softness*/)
{
  //if (pc->GetRigidBody ()->GetCollisionShape ())
  //  delete pc->GetCollisionShape ();

  //pc->GetRigidBody ()->SetCollisionShape (new CylinderShape ());

  //ResetShape ();

  return false;
}

bool csBulletRigidBody::AttachColliderBox (
    const csVector3 &size,
    const csOrthoTransform& trans, float friction,
    float /*density*/, float /*elasticity*/, 
    float /*softness*/)
{
  if (body)
  {
    ds->GetWorld ()->removeRigidBody (body);
    delete body;
  }
  if (!(trans.IsIdentity ()))
    motionState->SetOffsetTransform (trans);

  btBoxShape* shape = new btBoxShape (btVector3 (
	size.x / 2.0f, size.y / 2.0f, size.z / 2.0f));
  btVector3 localInertia (0, 0, 0);
  shape->calculateLocalInertia (mass, localInertia);
  body = new btRigidBody (mass, motionState, shape, localInertia,
      0, 0, friction);
  ds->GetWorld ()->addRigidBody (body);

  return true;
}

bool csBulletRigidBody::AttachColliderSphere (
    float radius, const csVector3& offset,
    float friction, float /*density*/, float /*elasticity*/,
    float /*softness*/)
{
  if (body)
  {
    ds->GetWorld ()->removeRigidBody (body);
    delete body;
  }
  if (!(offset < 0.0001f))
    motionState->SetOffset (offset);
  btSphereShape* shape = new btSphereShape (radius);
  btVector3 localInertia (0, 0, 0);
  shape->calculateLocalInertia (mass, localInertia);
  body = new btRigidBody (mass, motionState, shape, localInertia,
      0, 0, friction);
  ds->GetWorld ()->addRigidBody (body);

  return true;
}

bool csBulletRigidBody::AttachColliderPlane (
    const csPlane3 &,
    float /*friction*/, float /*density*/,
    float /*elasticity*/, float /*softness*/)
{
  return false;
}

void csBulletRigidBody::AttachCollider (iDynamicsSystemCollider*)
{
}

void csBulletRigidBody::DestroyColliders ()
{
}

void csBulletRigidBody::DestroyCollider (iDynamicsSystemCollider*)
{
}

void csBulletRigidBody::SetPosition (const csVector3& pos)
{
  if (body)
  {
    btTransform trans = body->getCenterOfMassTransform ();
    trans.setOrigin (btVector3 (pos.x, pos.y, pos.z));
    body->setCenterOfMassTransform (trans);
  }
  else
  {
    btTransform trans;
    motionState->getWorldTransform (trans);
    trans.setOrigin (btVector3 (pos.x, pos.y, pos.z));
    motionState->setWorldTransform (trans);
  }
}

const csVector3 csBulletRigidBody::GetPosition () const
{
  csBulletMotionState* motionState = static_cast<csBulletMotionState*> (
	body->getMotionState());
  btTransform trans;
  motionState->getWorldTransform (trans);
  btVector3& orig = trans.getOrigin ();
  return csVector3 (orig.getX (), orig.getY (), orig.getZ ());
}

void csBulletRigidBody::SetOrientation (const csMatrix3& rot)
{
  btTransform trans;
  motionState->getWorldTransform (trans);
  csReversibleTransform cstrans = BulletToCS (trans);
  cstrans.SetO2T (rot);
  motionState->setWorldTransform (CSToBullet (cstrans));
}

const csMatrix3 csBulletRigidBody::GetOrientation () const
{
  btTransform trans;
  motionState->getWorldTransform (trans);
  csReversibleTransform cstrans = BulletToCS (trans);
  return cstrans.GetO2T ();
}

void csBulletRigidBody::SetTransform (const csOrthoTransform& trans)
{
  if (body)
    body->setCenterOfMassTransform (CSToBullet (trans));
  else
    motionState->setWorldTransform (CSToBullet (trans));
}

const csOrthoTransform csBulletRigidBody::GetTransform () const
{
  btTransform trans;
  motionState->getWorldTransform (trans);
  return BulletToCS (trans);
}

void csBulletRigidBody::SetLinearVelocity (const csVector3& vel)
{
  body->setLinearVelocity (btVector3 (vel.x, vel.y, vel.z));
}

const csVector3 csBulletRigidBody::GetLinearVelocity () const
{
  const btVector3& vel = body->getLinearVelocity ();
  return csVector3 (vel.getX (), vel.getY (), vel.getZ ());
}

void csBulletRigidBody::SetAngularVelocity (const csVector3& vel)
{
  body->setAngularVelocity (btVector3 (vel.x, vel.y, vel.z));
}

const csVector3 csBulletRigidBody::GetAngularVelocity () const
{
  const btVector3& vel = body->getAngularVelocity ();
  return csVector3 (vel.getX (), vel.getY (), vel.getZ ());
}

void csBulletRigidBody::SetProperties (float mass, const csVector3& /*center*/,
                                       const csMatrix3& /*inertia*/)
{
  csBulletRigidBody::mass = mass;
  
  //ResetShape ();

  //csVector3 g = ds->GetGravity ();
  //pc->GetRigidBody ()->setGravity (SimdVector3 (g.x, g.y, g.z));
}

void csBulletRigidBody::GetProperties (float* /*mass*/, csVector3* /*center*/,
                                       csMatrix3* /*inertia*/)
{
}

float csBulletRigidBody::GetMass ()
{
  //return pc->GetRigidBody ()->getInvMass ();
  return 0.0f;
}

csVector3 csBulletRigidBody::GetCenter ()
{
  return csVector3 (0);
}

csMatrix3 csBulletRigidBody::GetInertia ()
{
  return csMatrix3 ();
}

void csBulletRigidBody::AdjustTotalMass (float /*targetmass*/)
{
}

void csBulletRigidBody::AddForce (const csVector3& force)
{
  body->applyForce (btVector3 (force.x, force.y, force.z),
      btVector3 (0, 0, 0));
}

void csBulletRigidBody::AddTorque (const csVector3& force)
{
  body->applyTorque (btVector3 (force.x, force.y, force.z));
}

void csBulletRigidBody::AddRelForce (const csVector3& /*force*/)
{
}

void csBulletRigidBody::AddRelTorque (const csVector3& /*force*/) 
{
}

void csBulletRigidBody::AddForceAtPos (const csVector3& /*force*/,
    const csVector3& /*pos*/)
{
}

void csBulletRigidBody::AddForceAtRelPos (const csVector3& /*force*/,
                                          const csVector3& /*pos*/)
{
}

void csBulletRigidBody::AddRelForceAtPos (const csVector3& /*force*/,
                                          const csVector3& /*pos*/)
{
}

void csBulletRigidBody::AddRelForceAtRelPos (const csVector3& /*force*/,
                                             const csVector3& /*pos*/)
{
}

const csVector3 csBulletRigidBody::GetForce () const
{
  return csVector3 (0);
}

const csVector3 csBulletRigidBody::GetTorque () const
{
  return csVector3 (0);
}

void csBulletRigidBody::AttachMesh (iMeshWrapper* m)
{
  mesh = m;
  motionState->SetMesh (mesh);
  if (body)
  {
    btTransform trans = body->getCenterOfMassTransform ();
    if (motionState->GetMoveCallback ())
    {
      csOrthoTransform tr = BulletToCS (trans);
      motionState->GetMoveCallback ()->Execute (mesh, tr);
    }
  }
}

iMeshWrapper* csBulletRigidBody::GetAttachedMesh ()
{
  return mesh;
}

void csBulletRigidBody::SetMoveCallback (iDynamicsMoveCallback* cb)
{
  motionState->SetMoveCallback (cb);
}

void csBulletRigidBody::SetCollisionCallback (iDynamicsCollisionCallback*)
{
}

void csBulletRigidBody::Collision (iRigidBody *, const csVector3&,
      const csVector3&, float)
{
}

csRef<iDynamicsSystemCollider> csBulletRigidBody::GetCollider (unsigned int)
{
  return 0;
}

int csBulletRigidBody::GetColliderCount ()
{
  return 0;
}

void csBulletRigidBody::Update ()
{
  //if (pc && move_cb)
  //{
    //csOrthoTransform trans;
    //trans = GetTransform ();
    //if (mesh) move_cb->Execute (mesh, trans);
    ///* remainder case for all other callbacks */
    //move_cb->Execute (trans);
  //}
}

//--------------------- csBulletDefaultMoveCallback --------------------------

csBulletDefaultMoveCallback::csBulletDefaultMoveCallback () 
  : scfImplementationType (this)
{
}

csBulletDefaultMoveCallback::~csBulletDefaultMoveCallback ()
{
}

void csBulletDefaultMoveCallback::Execute (iMeshWrapper* mesh,
                                           csOrthoTransform& t)
{
  // Dont do anything if nothing has changed
  // @@@ Is comparing that transform efficient and correct?
  if (mesh->GetMovable()->GetPosition() == t.GetOrigin() &&
      mesh->GetMovable()->GetTransform().GetT2O() == t.GetT2O())
    return;

  // Update movable
  mesh->GetMovable ()->SetTransform (t);
  mesh->GetMovable ()->UpdateMove ();
}

void csBulletDefaultMoveCallback::Execute (csOrthoTransform&)
{
  /* do nothing by default */
}

//--------------------- csBulletCollider -----------------------------------

csBulletCollider::csBulletCollider (csBulletDynamicsSystem* dynsys)
:  scfImplementationType (this)
{
  ds = dynsys;
  body = 0;
  mass = 0;
  friction = 0.5f;
  shape = 0;
  btTransform trans;
  trans.setIdentity ();
  motionState = new csBulletMotionState (0, trans);
}

csBulletCollider::~csBulletCollider ()
{
  if (body)
  {
    ds->GetWorld ()->removeRigidBody (body);
    delete body;
  }
  delete motionState;
  delete shape;
}

bool csBulletCollider::CreateSphereGeometry (const csSphere& sphere)
{
  if (body)
  {
    ds->GetWorld ()->removeRigidBody (body);
    delete body;
  }
  delete shape;
  // @@@ Need to support sphere center!
  btSphereShape* shape = new btSphereShape (sphere.GetRadius ());
  geom_type = SPHERE_COLLIDER_GEOMETRY;

  btVector3 localInertia (0, 0, 0);
  shape->calculateLocalInertia (mass, localInertia);
  body = new btRigidBody (mass, motionState, shape, localInertia,
      0, 0, friction);
  ds->GetWorld ()->addRigidBody (body);

  return true;
}

bool csBulletCollider::CreatePlaneGeometry (const csPlane3&)
{
  return false;
}

bool csBulletCollider::CreateMeshGeometry (iMeshWrapper *)
{
  return false;
}

bool csBulletCollider::CreateBoxGeometry (const csVector3& size)
{
  if (body)
  {
    ds->GetWorld ()->removeRigidBody (body);
    delete body;
  }
  delete shape;
  shape = new btBoxShape (btVector3 (
	size.x / 2.0f, size.y / 2.0f, size.z / 2.0f));
  geom_type = BOX_COLLIDER_GEOMETRY;

  btVector3 localInertia (0, 0, 0);
  shape->calculateLocalInertia (mass, localInertia);
  body = new btRigidBody (mass, motionState, shape, localInertia,
      0, 0, friction);
  ds->GetWorld ()->addRigidBody (body);

  return true;
}

bool csBulletCollider::CreateCapsuleGeometry (float /*length*/,
  float /*radius*/)
{
  return false;
}

void csBulletCollider::SetCollisionCallback (
  iDynamicsColliderCollisionCallback*)
{
}

void csBulletCollider::SetFriction (float friction)
{
  csBulletCollider::friction = friction;
  if (body)
    body->setFriction (friction);
}

void csBulletCollider::SetSoftness (float)
{
}

void csBulletCollider::SetDensity (float)
{
}

void csBulletCollider::SetElasticity (float)
{
}

float csBulletCollider::GetFriction ()
{
  return friction;
}

float csBulletCollider::GetSoftness ()
{
  return 0;
}

float csBulletCollider::GetDensity ()
{
  return 0;
}

float csBulletCollider::GetElasticity ()
{
  return 0;
}

void csBulletCollider::FillWithColliderGeometry (
    csRef<iGeneralFactoryState> genmesh_fact)
{
#if 0
  switch (geom_type)
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
  btTransform trans;
  motionState->getWorldTransform (trans);
  return BulletToCS (trans);
}

csOrthoTransform csBulletCollider::GetLocalTransform ()
{
  return csOrthoTransform ();
}

void csBulletCollider::SetTransform (const csOrthoTransform& trans)
{
  delete motionState;
  motionState = new csBulletMotionState (0, CSToBullet (trans));
  //motionState->setWorldTransform (CSToBullet (trans));

  // @@@ Not ideal. We recreate the body here since with static
  // colliders you can't move objects later.
  if (body)
  {
    ds->GetWorld ()->removeRigidBody (body);
    delete body;
    body = 0;
  }

  if (shape)
  {
    btVector3 localInertia (0, 0, 0);
    shape->calculateLocalInertia (mass, localInertia);
    body = new btRigidBody (mass, motionState, shape, localInertia);
    ds->GetWorld ()->addRigidBody (body);
  }
}

bool csBulletCollider::GetBoxGeometry (csVector3& /*size*/)
{
  return false;
}

bool csBulletCollider::GetSphereGeometry (csSphere&)
{
  return false;
}

bool csBulletCollider::GetPlaneGeometry (csPlane3&)
{
  return false;
}

bool csBulletCollider::GetCylinderGeometry (float& /*length*/,
  float& /*radius*/)
{
  return false;
}

void csBulletCollider::MakeStatic ()
{
}

void csBulletCollider::MakeDynamic ()
{
}

bool csBulletCollider::IsStatic ()
{
  return false;
}

}
CS_PLUGIN_NAMESPACE_END(Bullet)
