/*
Copyright (C) 2001 by Jorrit Tyberghein
Piotr Obrzut

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
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "imesh/genmesh.h"

//bullet includes
#include <CcdPhysics/CcdPhysicsController.h>
#include <Dynamics/RigidBody.h>
#include <CcdPhysics/CcdPhysicsEnvironment.h>
#include <ConstraintSolver/SimpleConstraintSolver.h>
#include <BroadphaseCollision/SimpleBroadphase.h>
#include <CollisionShapes/BoxShape.h>
#include <CollisionShapes/EmptyShape.h>
#include <CollisionShapes/SphereShape.h>
#include <CollisionShapes/CylinderShape.h>
#include <PHY_IMotionState.h>
#include <SimdTransform.h>
#include <SimdMatrix3x3.h>
#include <Dynamics/MassProps.h>
#include <CollisionDispatch/CollisionDispatcher.h>

#include "bullet.h"


CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csBulletDynamics)

#define ODE2CSMATRIX(odemat, csmat) \
{   \
  csmat.m11 = odemat[0]; csmat.m12 = odemat[4]; csmat.m13 = odemat[8]; \
  csmat.m21 = odemat[1]; csmat.m22 = odemat[5]; csmat.m23 = odemat[9]; \
  csmat.m31 = odemat[2]; csmat.m32 = odemat[6]; csmat.m33 = odemat[10]; \
}

//---------------------------------------------------------------------------

csBulletDynamics::csBulletDynamics (iBase *iParent)
:  scfImplementationType (this, iParent)
{

}
csBulletDynamics::~csBulletDynamics ()
{
}
bool csBulletDynamics::Initialize (iObjectRegistry* object_reg)
{
  csBulletDynamics::object_reg = object_reg;
  return true;
}
csPtr<iDynamicSystem> csBulletDynamics::CreateSystem ()
{
  csBulletDynamicsSystem* system = new csBulletDynamicsSystem ();
  csRef<iDynamicSystem> isystem (SCF_QUERY_INTERFACE (system, iDynamicSystem));
  systems.Push (system);
  isystem->DecRef ();

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
}
//-------------------------------csBulletDynamicsSystem----------------------------------------------//

csBulletDynamicsSystem::csBulletDynamicsSystem ()
:  scfImplementationType (this)
{
  CollisionDispatcher* dispatcher = new CollisionDispatcher ();
  BroadphaseInterface* broadphase = new SimpleBroadphase();

  bullet_sys = new CcdPhysicsEnvironment(dispatcher,broadphase);
}
csBulletDynamicsSystem::~csBulletDynamicsSystem ()
{
  bodies.DeleteAll ();
  delete bullet_sys;
}
void csBulletDynamicsSystem::SetGravity (const csVector3& v)
{
  gravity = v; 
  bullet_sys->setGravity(v.x, v.y, v.z);
}
const csVector3 csBulletDynamicsSystem::GetGravity () const
{
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
  bullet_sys->proceedDeltaTime (0.f, stepsize);

  for (size_t i = 0; i < bodies.GetSize (); i++)
    bodies[i]->Update ();

}
csPtr<iRigidBody> csBulletDynamicsSystem::CreateBody ()
{
  csBulletRigidBody *b = new csBulletRigidBody (this);
  bullet_sys->addCcdPhysicsController (b->GetBulletBody ());

  csRef<iRigidBody> ib = SCF_QUERY_INTERFACE (b, iRigidBody);
  bodies.Push (ib);
  ib->DecRef ();

  return (csPtr<iRigidBody>) ib;
}
void csBulletDynamicsSystem::RemoveBody (iRigidBody*)
{
}
iRigidBody *csBulletDynamicsSystem::FindBody (const char * /*name*/)
{
  return 0;
}
iRigidBody *csBulletDynamicsSystem::GetBody (unsigned int /*index*/)
{
  return 0;
}
int csBulletDynamicsSystem::GetBodysCount ()
{
  return 0;
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
  return 0;
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
  bullet_sys->addCcdPhysicsController (bulletc->GetBulletController ());
  bulletc->SetElasticity (elasticity);
  bulletc->SetFriction (friction);
  bulletc->SetSoftness (softness);
  bulletc->CreateBoxGeometry (size);

  bulletc->SetTransform (trans);
  colliders.Push (bulletc);

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
  return 0;
}

//-------------------------------csBulletRigidBody----------------------------------------------//
csBulletRigidBody::csBulletRigidBody (csBulletDynamicsSystem* dynsys)
:  scfImplementationType (this)
{
  ds = dynsys;
  ms = new MyMotionState ();
  ms->setWorldPosition (0,0,0);
  ms->setWorldOrientation (0,0,0,1);

  CcdConstructionInfo ccdObjectCi;

  ccdObjectCi.m_collisionShape = new EmptyShape ();
  ccdObjectCi.m_collisionShape->SetMargin (1);
  ccdObjectCi.m_gravity = SimdVector3(0,0,0);
  ccdObjectCi.m_localInertiaTensor = SimdVector3(0,0,0);
  ccdObjectCi.m_physicsEnv = dynsys->GetBulletSys ();
  mass = ccdObjectCi.m_mass = 1;

  ccdObjectCi.m_MotionState = ms;
  pc = new CcdPhysicsController(ccdObjectCi);

  move_cb = (iDynamicsMoveCallback*)new csBulletDefaultMoveCallback ();
}
csBulletRigidBody::~csBulletRigidBody ()
{
  delete pc;
  if (move_cb) move_cb->DecRef ();
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
  pc->SetActive (true);
  return true;
}

bool csBulletRigidBody::IsEnabled (void)
{
  return pc->GetRigidBody()->GetActivationState() == 1;
}

csRef<iBodyGroup> csBulletRigidBody::GetGroup (void)
{
  return 0;
}

void csBulletRigidBody::ResetShape ()
{
  if (pc->GetCollisionShape ()->GetShapeType () != EMPTY_SHAPE_PROXYTYPE)
  {
    BroadphaseProxy* bpproxy = (BroadphaseProxy*)pc->GetRigidBody ()->m_broadphaseHandle;
    ds->GetBulletSys ()->GetBroadphase()->CleanProxyFromPairs(bpproxy);

    SimdVector3 newinertia;
    pc->GetRigidBody()->GetCollisionShape()->CalculateLocalInertia(mass,newinertia);
    pc->GetRigidBody()->setMassProps(mass,newinertia);
    pc->GetRigidBody()->updateInertiaTensor();
  }
}

bool csBulletRigidBody::AttachColliderMesh (iMeshWrapper*,
  const csOrthoTransform&, float /*friction*/, float /*density*/,
  float /*elasticity*/, float /*softness*/)
{
  return false;
}

bool csBulletRigidBody::AttachColliderCylinder (float /*length*/, float /*radius*/,
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

bool csBulletRigidBody::AttachColliderBox (const csVector3 &size,
                                           const csOrthoTransform& /*trans*/, float friction,
                                           float /*density*/, float /*elasticity*/, 
                                           float /*softness*/)
{
  //if (pc->GetRigidBody ()->GetCollisionShape ())
  //  delete pc->GetCollisionShape ();

  SimdVector3 s (size.x / 2, size.y / 2, size.z / 2); 
  pc->GetRigidBody ()->SetCollisionShape (new BoxShape (s));
  pc->GetRigidBody ()->setFriction (friction);

  ResetShape ();

  return true;
}

bool csBulletRigidBody::AttachColliderSphere (float radius, const csVector3 &/*offset*/,
                                              float friction, float /*density*/, float /*elasticity*/,
                                              float /*softness*/)
{
  if (pc->GetRigidBody ()->GetCollisionShape ())
    delete pc->GetCollisionShape ();

  pc->GetRigidBody ()->SetCollisionShape (new SphereShape (radius)); 
  pc->GetRigidBody ()->setFriction (friction);

  ResetShape ();

  return true;
}

bool csBulletRigidBody::AttachColliderPlane (const csPlane3 &,
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

void csBulletRigidBody::SetPosition (const csVector3& trans)
{
  pc->setPosition (trans.x, trans.y, trans.z);
}

const csVector3 csBulletRigidBody::GetPosition () const
{
  csVector3 v;
  ms->getWorldPosition (v.x, v.y, v.z);
  return v;
}

void csBulletRigidBody::SetOrientation (const csMatrix3& rot)
{
  csQuaternion q; q.SetMatrix (rot);
  pc->setOrientation (q.v.x, q.v.y, q.v.z, q.w);
}

const csMatrix3 csBulletRigidBody::GetOrientation () const
{
  csQuaternion q;
  ms->getWorldOrientation(q.v.x,q.v.y,q.v.z,q.w);
  return csMatrix3 (q).GetTranspose ();
}

void csBulletRigidBody::SetTransform (const csOrthoTransform& trans)
{
  SetPosition (trans.GetOrigin ());
  SetOrientation (trans.GetO2T ());
}

const csOrthoTransform csBulletRigidBody::GetTransform () const
{
  return csOrthoTransform (GetOrientation (), GetPosition ());
}

void csBulletRigidBody::SetLinearVelocity (const csVector3& vel)
{
  pc->SetLinearVelocity (vel.x,vel.y,vel.z, false);
}

const csVector3 csBulletRigidBody::GetLinearVelocity () const
{
  return csVector3 (0);
}

void csBulletRigidBody::SetAngularVelocity (const csVector3& vel)
{
  pc->SetAngularVelocity (vel.x, vel.y, vel.z, false);
}

const csVector3 csBulletRigidBody::GetAngularVelocity () const
{
  return csVector3 (0);
}

void csBulletRigidBody::SetProperties (float mass, const csVector3& /*center*/,
                                       const csMatrix3& /*inertia*/)
{
  csBulletRigidBody::mass = mass;
  
  ResetShape ();

  csVector3 g = ds->GetGravity ();
  pc->GetRigidBody ()->setGravity (SimdVector3 (g.x, g.y, g.z));
}

void csBulletRigidBody::GetProperties (float* /*mass*/, csVector3* /*center*/,
                                       csMatrix3* /*inertia*/)
{
}

float csBulletRigidBody::GetMass ()
{
  return pc->GetRigidBody ()->getInvMass ();
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
  SimdVector3 f (force.x, force.y, force.z);
  pc->GetRigidBody ()->applyCentralForce(f);
}
void csBulletRigidBody::AddTorque (const csVector3& /*force*/)
{
}
void csBulletRigidBody::AddRelForce (const csVector3& /*force*/)
{
}
void csBulletRigidBody::AddRelTorque (const csVector3& /*force*/) 
{
}
void csBulletRigidBody::AddForceAtPos (const csVector3& /*force*/, const csVector3& /*pos*/)
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
  if (m) m->IncRef ();
  if (mesh) mesh->DecRef ();
  mesh = m;
}
iMeshWrapper* csBulletRigidBody::GetAttachedMesh ()
{
  return 0;
}

void csBulletRigidBody::SetMoveCallback (iDynamicsMoveCallback* cb)
{
  if (cb) cb->IncRef ();
  if (move_cb) move_cb->DecRef ();
  move_cb = cb;
}
void csBulletRigidBody::SetCollisionCallback (iDynamicsCollisionCallback*)
{
}
void csBulletRigidBody::Collision (iRigidBody *)
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
  if (pc && move_cb)
  {
    csOrthoTransform trans;
    trans = GetTransform ();
    if (mesh) move_cb->Execute (mesh, trans);
    /* remainder case for all other callbacks */
    move_cb->Execute (trans);
  }
}
//-------------------------------MyMotionState----------------------------------------------//
MyMotionState::MyMotionState()
{
  m_worldTransform.setIdentity();
}


MyMotionState::~MyMotionState()
{

}

void	MyMotionState::getWorldPosition(float& posX,float& posY,float& posZ)
{
  posX = m_worldTransform.getOrigin().x();
  posY = m_worldTransform.getOrigin().y();
  posZ = m_worldTransform.getOrigin().z();
}

void	MyMotionState::getWorldScaling(float& scaleX,float& scaleY,float& scaleZ)
{
  scaleX = 1.;
  scaleY = 1.;
  scaleZ = 1.;
}

void	MyMotionState::getWorldOrientation(float& quatIma0,float& quatIma1,float& quatIma2,float& quatReal)
{
  quatIma0 = m_worldTransform.getRotation().x();
  quatIma1 = m_worldTransform.getRotation().y();
  quatIma2 = m_worldTransform.getRotation().z();
  quatReal = m_worldTransform.getRotation()[3];
}

void	MyMotionState::setWorldPosition(float posX,float posY,float posZ)
{
  SimdPoint3 pos(posX,posY,posZ);
  m_worldTransform.setOrigin( pos );
}

void	MyMotionState::setWorldOrientation(float quatIma0,float quatIma1,float quatIma2,float quatReal)
{
  SimdQuaternion orn(quatIma0,quatIma1,quatIma2,quatReal);
  m_worldTransform.setRotation( orn );
}

void	MyMotionState::calculateWorldTransformations()
{

}

//-------------------------------csBulletDefaultMoveCallback----------------------------------------------//

csBulletDefaultMoveCallback::csBulletDefaultMoveCallback () 
:  scfImplementationType (this)
{

}

csBulletDefaultMoveCallback::~csBulletDefaultMoveCallback ()
{

}

void csBulletDefaultMoveCallback::Execute (iMeshWrapper* mesh,
                                           csOrthoTransform& t)
{
  // Dont do anything if nothing has changed
  if (mesh->GetMovable()->GetPosition() == t.GetOrigin() &&
    mesh->GetMovable()->GetTransform().GetT2O() == t.GetT2O())
    return;

  // Update movable
  mesh->GetMovable ()->SetPosition (t.GetOrigin ());
  mesh->GetMovable ()->GetTransform ().SetT2O (t.GetT2O ());
  mesh->GetMovable ()->UpdateMove ();
}

void csBulletDefaultMoveCallback::Execute (csOrthoTransform&)
{
  /* do nothing by default */
}

//-------------------------------csBulletCollider----------------------------------------------//
csBulletCollider::csBulletCollider (csBulletDynamicsSystem* dynsys)
:  scfImplementationType (this)
{
  CcdConstructionInfo ccdObjectCi;

  ms = new MyMotionState ();

  ms->setWorldPosition (0, 0, 0);
  ms->setWorldOrientation (0,0,0,1);

  ccdObjectCi.m_collisionShape = new EmptyShape ();
  ccdObjectCi.m_collisionShape->SetMargin (1.f);
  ccdObjectCi.m_gravity = SimdVector3(0,0,0);
  ccdObjectCi.m_localInertiaTensor = SimdVector3(0,0,0);
  ccdObjectCi.m_mass = 0; //collider need 0 mass
  ccdObjectCi.m_MotionState = ms;

  pc = new CcdPhysicsController (ccdObjectCi);

  ds = dynsys;
}
csBulletCollider::~csBulletCollider ()
{
  delete pc;
}
void csBulletCollider::ResetShape ()
{
  BroadphaseProxy* bpproxy = (BroadphaseProxy*)pc->GetRigidBody ()->m_broadphaseHandle;
  ds->GetBulletSys ()->GetBroadphase()->CleanProxyFromPairs(bpproxy);

  SimdVector3 newinertia;
  pc->GetRigidBody()->GetCollisionShape()->CalculateLocalInertia(0,newinertia);
  pc->GetRigidBody()->setMassProps(0,newinertia);
  pc->GetRigidBody()->updateInertiaTensor();
}

bool csBulletCollider::CreateSphereGeometry (const csSphere&)
{
  return false;
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
  if (pc->GetRigidBody ()->GetCollisionShape ())
    delete pc->GetCollisionShape ();

  SimdVector3 s (size.x / 2, size.y / 2, size.z / 2); 
  pc->GetRigidBody ()->SetCollisionShape (new BoxShape (s));
  ResetShape ();
  geom_type = BOX_COLLIDER_GEOMETRY;

  return true;
}
bool csBulletCollider::CreateCCylinderGeometry (float /*length*/,
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
  pc->GetRigidBody ()->setFriction (friction);
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
  return pc->GetRigidBody ()->getFriction ();
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
void csBulletCollider::FillWithColliderGeometry (csRef<iGeneralFactoryState> genmesh_fact)
{
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
}
csOrthoTransform csBulletCollider::GetTransform ()
{
  csQuaternion q;
  ms->getWorldOrientation(q.v.x,q.v.y,q.v.z,q.w);
  csMatrix3 rot (q);

  csVector3 v;
  ms->getWorldPosition (v.x, v.y, v.z);

  return csOrthoTransform (rot.GetTranspose (), v);
}
csOrthoTransform csBulletCollider::GetLocalTransform ()
{
  return csOrthoTransform ();
}
void csBulletCollider::SetTransform (const csOrthoTransform& trans)
{
  csQuaternion q; q.SetMatrix (trans.GetO2T ());
  pc->setOrientation (q.v.x, q.v.y, q.v.z, q.w);
  ms->setWorldOrientation (q.v.x, q.v.y, q.v.z, q.w);
  csVector3 pos = trans.GetOrigin ();
  pc->setPosition (pos.x, pos.y, pos.z);
  ms->setWorldPosition (pos.x, pos.y, pos.z);
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
