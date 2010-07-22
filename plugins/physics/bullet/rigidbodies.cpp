/*
  Copyright (C) 2010 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "iengine/movable.h"
#include "csgeom/sphere.h"

// Bullet includes.
#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"

#include "rigidbodies.h"
#include "colliders.h"

CS_PLUGIN_NAMESPACE_BEGIN(Bullet)
{

//-------------------- csBulletRigidBody -----------------------------------

csBulletRigidBody::csBulletRigidBody (csBulletDynamicsSystem* dynSys, bool isStatic)
  : scfImplementationType (this), dynSys (dynSys), body (0),
    dynamicState (isStatic? CS::Physics::Bullet::CS_BULLET_STATE_STATIC
		  : CS::Physics::Bullet::CS_BULLET_STATE_DYNAMIC),
    customMass (false), mass (1.0f), compoundChanged (false), insideWorld (false),
    linearDampening (dynSys->linearDampening), angularDampening (dynSys->angularDampening)
{
  bodyType = CS::Physics::Bullet::CS_BULLET_RIGID_BODY;
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
    if (dynamicState == CS::Physics::Bullet::CS_BULLET_STATE_DYNAMIC)
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
    }
  }

  else if (dynamicState == CS::Physics::Bullet::CS_BULLET_STATE_DYNAMIC)
  {
    // compound hasn't been changed
    bodyMass = mass;
    compoundShape->calculateLocalInertia (bodyMass, localInertia);
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
  body->setUserPointer ((iBulletBody*) this);
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
  if (body && dynamicState != CS::Physics::Bullet::CS_BULLET_STATE_STATIC)
  {
    CS::Physics::Bullet::BodyState previousState = dynamicState;

    // rebuild body if a child collider was a concave mesh
    bool hasTrimesh = false;
    for (unsigned int i = 0; i < colliders.GetSize (); i++)
      if (colliders[i]->shape
	  && colliders[i]->geomType == TRIMESH_COLLIDER_GEOMETRY)
      {
	dynamicState = CS::Physics::Bullet::CS_BULLET_STATE_STATIC;
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
    if (previousState == CS::Physics::Bullet::CS_BULLET_STATE_KINEMATIC)
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

  dynamicState = CS::Physics::Bullet::CS_BULLET_STATE_STATIC;

  return true;
}

bool csBulletRigidBody::MakeDynamic (void)
{
  if (body && dynamicState != CS::Physics::Bullet::CS_BULLET_STATE_DYNAMIC)
  {
    CS::Physics::Bullet::BodyState previousState = dynamicState;

    // rebuild body if a child collider was a concave mesh
    if (previousState == CS::Physics::Bullet::CS_BULLET_STATE_STATIC)
    {
      bool hasTrimesh = false;
      for (unsigned int i = 0; i < colliders.GetSize (); i++)
	if (colliders[i]->shape
	    && colliders[i]->geomType == TRIMESH_COLLIDER_GEOMETRY)
	{
	  dynamicState = CS::Physics::Bullet::CS_BULLET_STATE_DYNAMIC;
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
    if (previousState == CS::Physics::Bullet::CS_BULLET_STATE_KINEMATIC)
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

  dynamicState = CS::Physics::Bullet::CS_BULLET_STATE_DYNAMIC;

  return true;
}

void csBulletRigidBody::MakeKinematic ()
{
  if (body && dynamicState != CS::Physics::Bullet::CS_BULLET_STATE_KINEMATIC)
  {
    CS::Physics::Bullet::BodyState previousState = dynamicState;

    // rebuild body if a child collider was a concave mesh
    if (previousState == CS::Physics::Bullet::CS_BULLET_STATE_STATIC)
    {
      bool hasTrimesh = false;
      for (unsigned int i = 0; i < colliders.GetSize (); i++)
	if (colliders[i]->shape
	    && colliders[i]->geomType == TRIMESH_COLLIDER_GEOMETRY)
	{
	  dynamicState = CS::Physics::Bullet::CS_BULLET_STATE_KINEMATIC;
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

  dynamicState = CS::Physics::Bullet::CS_BULLET_STATE_KINEMATIC;

  return;
}

bool csBulletRigidBody::IsStatic (void)
{
  return dynamicState == CS::Physics::Bullet::CS_BULLET_STATE_STATIC;
}

CS::Physics::Bullet::BodyState csBulletRigidBody::GetDynamicState () const
{
  return dynamicState;
}

void csBulletRigidBody::SetDynamicState (CS::Physics::Bullet::BodyState state)
{
  switch (state)
    {
    case CS::Physics::Bullet::CS_BULLET_STATE_STATIC:
      MakeStatic ();
      break;

    case CS::Physics::Bullet::CS_BULLET_STATE_DYNAMIC:
      MakeDynamic ();
      break;

    case CS::Physics::Bullet::CS_BULLET_STATE_KINEMATIC:
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
  body->setInterpolationWorldTransform (body->getWorldTransform());
  if (body)
    body->setActivationState (ISLAND_SLEEPING);
  return false;
}

bool csBulletRigidBody::Enable (void)
{
  if (body)
    body->setActivationState (ACTIVE_TAG);
  return true;
}

bool csBulletRigidBody::IsEnabled (void)
{
  if (body)
    return body->isActive ();
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
  {
    dynSys->bulletWorld->removeRigidBody (body);

    // wake up all connected bodies
    for (size_t i = 0; i < contactObjects.GetSize (); i++)
      contactObjects[i]->activate ();
  }

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

  if (body)
    body->setMotionState (motionState);

  // put back body in the world
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
  {
    dynSys->bulletWorld->removeRigidBody (body);

    // wake up all connected bodies
    for (size_t i = 0; i < contactObjects.GetSize (); i++)
      contactObjects[i]->activate ();
  }

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

  if (body)
    body->setMotionState (motionState);

  // put back body in the world
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
  {
    dynSys->bulletWorld->removeRigidBody (body);

    // wake up all connected bodies
    for (size_t i = 0; i < contactObjects.GetSize (); i++)
      contactObjects[i]->activate ();
  }

  // create new motion state
  btTransform tr = CSToBullet (trans, dynSys->internalScale);
  btTransform principalAxis = motionState->inversePrincipalAxis.inverse ();
  delete motionState;
  motionState = new csBulletMotionState (this, tr * principalAxis, principalAxis);

  if (body)
    body->setMotionState (motionState);

  // put back body in the world
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

  if (dynamicState == CS::Physics::Bullet::CS_BULLET_STATE_DYNAMIC)
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

  if (dynamicState == CS::Physics::Bullet::CS_BULLET_STATE_DYNAMIC)
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
  if (dynamicState != CS::Physics::Bullet::CS_BULLET_STATE_DYNAMIC)
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

}
CS_PLUGIN_NAMESPACE_END(Bullet)
