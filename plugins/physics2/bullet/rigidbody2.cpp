/*
  Copyright (C) 2011 by Liu Lu

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
#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"

#include "rigidbody2.h"

CS_PLUGIN_NAMESPACE_BEGIN (Bullet2)
{
csBulletRigidBody::csBulletRigidBody (csBulletSystem* phySys)
: scfImplementationType (this, phySys), btBody (NULL), density (0.2f), totalMass (0.0f),
physicalState (CS::Physics2::STATE_DYNAMIC), softness (0.01f), anchorCount (0),
friction (5.0f), elasticity (0.2f), linearVelocity (0.0f, 0.0f, 0.0f),
angularVelocity (0.0f, 0.0f, 0.0f), linearDampening (0.0f), angularDampening (0.0f)
{
  type = CS::Collision2::COLLISION_OBJECT_PHYSICAL;
}

csBulletRigidBody::~csBulletRigidBody ()
{
  RemoveBulletObject ();
}

void csBulletRigidBody::AddCollider (CS::Collision2::iCollider* collider, 
                                     const csOrthoTransform& relaTrans)
{
  csRef<csBulletCollider> coll (dynamic_cast<csBulletCollider*>(collider));

  CS::Collision2::ColliderType type = collider->GetGeometryType ();
  if (type == CS::Collision2::COLLIDER_CONCAVE_MESH
    ||type == CS::Collision2::COLLIDER_CONCAVE_MESH_SCALED
    ||type == CS::Collision2::COLLIDER_PLANE)
    haveStaticColliders ++;
  else if (type == CS::Collision2::COLLIDER_TERRAIN)
  {
    csFPrintf (stderr, "csBulletRigidBody: Can not add terrain collider to physical body.\n");
    return;
  }

  colliders.Push (coll);
  relaTransforms.Push (relaTrans);
  shapeChanged = true;
}

void csBulletRigidBody::RemoveCollider (CS::Collision2::iCollider* collider)
{
  for (size_t i =0; i < colliders.GetSize(); i++)
  {
    if (colliders[i] == collider)
    {
      RemoveCollider (i);
      return;
    }
  }
}

void csBulletRigidBody::RemoveCollider (size_t index)
{
  if (index >= colliders.GetSize ())
    return;
  CS::Collision2::ColliderType type = colliders[index]->GetGeometryType ();
  if (type == CS::Collision2::COLLIDER_CONCAVE_MESH
    ||type == CS::Collision2::COLLIDER_CONCAVE_MESH_SCALED
    ||type == CS::Collision2::COLLIDER_PLANE)
    haveStaticColliders --;
  colliders.DeleteIndex (index);
  relaTransforms.DeleteIndex (index);
}

bool csBulletRigidBody::RemoveBulletObject ()
{
  if (insideWorld)
  {
    if (anchorCount > 0)
    {
      csFPrintf (stderr, "csBulletRigidBody: Please remove the soft body attached with this body first.\n");
      return false;
    }

    for (size_t i = 0; i < joints.GetSize (); i++)
      sector->RemoveJoint (joints[i]);

    linearVelocity = GetLinearVelocity ();
    angularVelocity = GetAngularVelocity ();
    sector->bulletWorld->removeRigidBody (btBody);
    delete btBody;
    btBody = NULL;
    btObject = NULL;
    insideWorld = false;
    csBulletRigidBody* rb = dynamic_cast<csBulletRigidBody*> (objectCopy);
    if (objectCopy)
      rb->sector->RemoveRigidBody (rb);
    rb = dynamic_cast<csBulletRigidBody*> (objectOrigin);
    if (objectOrigin)
      rb->objectCopy = NULL;

    objectCopy = NULL;
    objectOrigin = NULL;
    return true;
  }
  return false;
}

bool csBulletRigidBody::AddBulletObject ()
{
  if (insideWorld)
    RemoveBulletObject ();

  btVector3 localInertia (0.0f, 0.0f, 0.0f);
  float mass = GetMass ();

  btCollisionShape* shape;
  btTransform principalAxis;
  principalAxis.setIdentity ();
  btVector3 principalInertia(0,0,0);

  //Create btRigidBody
  if (compoundShape)
  {
    int shapeCount = compoundShape->getNumChildShapes ();
    CS_ALLOC_STACK_ARRAY(btScalar, masses, shapeCount); 
    for (int i = 0; i < shapeCount; i++)
      masses[i] = density * colliders[i]->GetVolume ();
    if (shapeChanged)
    {
      compoundShape->calculatePrincipalAxisTransform
        (masses, principalAxis, principalInertia);
      // apply principal axis
      // creation is faster using a new compound to store the shifted children
      btCompoundShape* newCompoundShape = new btCompoundShape();
      for (int i = 0; i < shapeCount; i++)
      {
        btTransform newChildTransform =
          principalAxis.inverse() * compoundShape->getChildTransform (i);
        newCompoundShape->addChildShape(newChildTransform,
          compoundShape->getChildShape (i));
        shapeChanged = false;
      }
      delete compoundShape;
      compoundShape = newCompoundShape;
    }

    shape = compoundShape;
  }
  else
  {
    shape = colliders[0]->shape;
    principalAxis = CSToBullet (relaTransforms[0], system->getInternalScale ());
  }

  // create new motion state
  btTransform trans;
  motionState->getWorldTransform (trans);
  trans = trans * motionState->inversePrincipalAxis;
  delete motionState;
  motionState = new csBulletMotionState (this, trans * principalAxis,
    principalAxis);

  //shape->calculateLocalInertia (mass, localInertia);

  btRigidBody::btRigidBodyConstructionInfo infos (mass, motionState,
    shape, localInertia);

  infos.m_friction = friction;
  infos.m_restitution = elasticity;
  infos.m_linearDamping = linearDampening;
  infos.m_angularDamping = angularDampening;

  // create new rigid body
  btBody = new btRigidBody (infos);
  btObject = btBody;

  if (haveStaticColliders > 0)
    physicalState = CS::Physics2::STATE_STATIC;

  SetState (physicalState);

  sector->bulletWorld->addRigidBody (btBody, collGroup.value, collGroup.mask);
  btBody->setUserPointer (dynamic_cast<CS::Collision2::iCollisionObject*> (
    dynamic_cast<csBulletCollisionObject*>(this)));
 
  insideWorld = true;
  return true;
}

bool csBulletRigidBody::Disable ()
{
 SetLinearVelocity (csVector3 (0.0f));
 SetAngularVelocity (csVector3 (0.0f));
 if (btBody)
 {
   btBody->setInterpolationWorldTransform (btBody->getWorldTransform());
   btBody->setActivationState (ISLAND_SLEEPING);
   return true;
 }
 return false;
}

bool csBulletRigidBody::Enable ()
{
  if (btBody)
  {
    btObject->setActivationState (ACTIVE_TAG);
    return true;
  }
  else
    return false;
}

bool csBulletRigidBody::IsEnabled ()
{
  if (btBody)
    return btBody->isActive ();
  return false;
}

void csBulletRigidBody::SetMass (float mass)
{
  if (mass > SMALL_EPSILON)
    totalMass = mass;
}

float csBulletRigidBody::GetMass ()
{
  if (physicalState != CS::Physics2::STATE_DYNAMIC)
    return 0.0f;

  if (totalMass > SMALL_EPSILON)
    return totalMass;

  return density * this->GetVolume ();
}

void csBulletRigidBody::SetDensity (float density)
{
  if (density < EPSILON)
    this->density = density;
}

float csBulletRigidBody::GetVolume ()
{
  float volume = 0;
  for (size_t i = 0; i < colliders.GetSize (); i++)
  {
    float vol = colliders[i]->GetVolume ();
    if (vol < FLT_MAX)
      volume += vol;
    else
      return FLT_MAX;
  }
  return volume;
}

void csBulletRigidBody::AddForce (const csVector3& force)
{
  if (btBody)
  {
    btBody->applyImpulse (btVector3 (force.x * system->getInternalScale (),
      force.y * system->getInternalScale (),
      force.z * system->getInternalScale ()),
      btVector3 (0.0f, 0.0f, 0.0f));
    btBody->setActivationState(ACTIVE_TAG);
  }
}

void csBulletRigidBody::SetLinearVelocity (const csVector3& vel)
{
  linearVelocity = vel;
  if (!btBody)
    return;
  if (physicalState == CS::Physics2::STATE_DYNAMIC)
  {
    btBody->setLinearVelocity (CSToBullet (vel, system->getInternalScale ()));
    btBody->activate ();
  }
}

csVector3 csBulletRigidBody::GetLinearVelocity (size_t index /* = 0 */) const
{
  if (!btBody)
    return linearVelocity;

  const btVector3& vel = btBody->getLinearVelocity ();
  return BulletToCS (vel, system->getInverseInternalScale ());
}

void csBulletRigidBody::SetFriction (float friction)
{
  this->friction = friction;
}

bool csBulletRigidBody::SetState (CS::Physics2::RigidBodyState state)
{
  if (physicalState != state || !insideWorld)
  {
    CS::Physics2::RigidBodyState previousState = physicalState;
    physicalState = state;

    if (!btBody)
      return false;

    if (haveStaticColliders > 0 && state != CS::Physics2::STATE_STATIC)
      return false;

    if (insideWorld)
      sector->bulletWorld->removeRigidBody (btBody);

    btVector3 linearVelo = CSToBullet (linearVelocity, system->getInternalScale ());
    btVector3 angularVelo = btVector3 (angularVelocity.x, angularVelocity.y, angularVelocity.z);

    if (previousState == CS::Physics2::STATE_KINEMATIC && insideWorld)
    {
      btBody->setCollisionFlags (btBody->getCollisionFlags()
        & ~btCollisionObject::CF_KINEMATIC_OBJECT);

      linearVelo = btBody->getInterpolationLinearVelocity ();
      angularVelo = btBody->getInterpolationAngularVelocity ();

      // create new motion state
      btTransform principalAxis = motionState->inversePrincipalAxis.inverse ();
      btTransform trans;
      motionState->getWorldTransform (trans);
      trans = trans * motionState->inversePrincipalAxis;
      delete motionState;
      motionState = new csBulletMotionState
        (this, trans * principalAxis, principalAxis);
      btBody->setMotionState (motionState);
    }

    if (state == CS::Physics2::STATE_DYNAMIC)
    {
      btBody->setCollisionFlags (btBody->getCollisionFlags()
        & ~btCollisionObject::CF_STATIC_OBJECT);

      float mass = GetMass ();
      btVector3 localInertia (0.0f, 0.0f, 0.0f);

      if (compoundShape)
        compoundShape->calculateLocalInertia (mass, localInertia);
      else
        colliders[0]->shape->calculateLocalInertia (mass, localInertia);

      btBody->setMassProps (mass, localInertia);

      btBody->forceActivationState (ACTIVE_TAG);

      btBody->setLinearVelocity (linearVelo);
      btBody->setAngularVelocity (angularVelo);
      btBody->updateInertiaTensor ();
    }
    else if (state == CS::Physics2::STATE_KINEMATIC)
    {
      if (!kinematicCb)
        kinematicCb.AttachNew (new csBulletDefaultKinematicCallback ());

      // create new motion state
      btTransform principalAxis = motionState->inversePrincipalAxis.inverse ();
      btTransform trans;
      motionState->getWorldTransform (trans);
      delete motionState;
      motionState = new csBulletKinematicMotionState
        (this, trans, principalAxis);
      btBody->setMotionState (motionState);

      // set body kinematic

      btBody->setMassProps (0.0f, btVector3 (0.0f, 0.0f, 0.0f));

      btBody->setCollisionFlags (btBody->getCollisionFlags()
        | btCollisionObject::CF_KINEMATIC_OBJECT
        & ~btCollisionObject::CF_STATIC_OBJECT);
      btBody->setActivationState (DISABLE_DEACTIVATION);    
      btBody->updateInertiaTensor ();
      btBody->setInterpolationWorldTransform (btBody->getWorldTransform ());
      btBody->setInterpolationLinearVelocity (btVector3(0.0f, 0.0f, 0.0f));
      btBody->setInterpolationAngularVelocity (btVector3(0.0f, 0.0f, 0.0f));
    }
    else if (state == CS::Physics2::STATE_STATIC)
    {
      btBody->setCollisionFlags (btBody->getCollisionFlags()
        | btCollisionObject::CF_STATIC_OBJECT
        & ~btCollisionObject::CF_KINEMATIC_OBJECT);
      btBody->setActivationState (ISLAND_SLEEPING);    
      btBody->setMassProps (0.0f, btVector3 (0.0f, 0.0f, 0.0f));
      btBody->updateInertiaTensor ();
    }

    if (insideWorld)
      sector->bulletWorld->addRigidBody (btBody);
    return true;
  }
  else
    return false;
}

void csBulletRigidBody::SetElasticity (float elasticity)
{
  this->elasticity = elasticity;
}

void csBulletRigidBody::SetAngularVelocity (const csVector3& vel)
{
  angularVelocity = vel;
  if (!btBody)
    return; 
  if (physicalState == CS::Physics2::STATE_DYNAMIC)
  {
    btBody->setAngularVelocity (btVector3 (vel.x, vel.y, vel.z));
    btBody->activate ();
  }
}

csVector3 csBulletRigidBody::GetAngularVelocity () const
{
  if (!btBody)
    return angularVelocity; 

  const btVector3& vel = btBody->getAngularVelocity ();
  return csVector3 (vel.getX (), vel.getY (), vel.getZ ());
}

void csBulletRigidBody::AddTorque (const csVector3& torque)
{
  if (!btBody)
    return; 

  btBody->applyTorque (btVector3 (torque.x * system->getInternalScale () * system->getInternalScale (),
    torque.y * system->getInternalScale () * system->getInternalScale (),
    torque.z * system->getInternalScale () * system->getInternalScale ()));
  btBody->setActivationState(ACTIVE_TAG);
}

void csBulletRigidBody::AddRelForce (const csVector3& force)
{
  if (!btBody)
    return; 

  csOrthoTransform trans =  csBulletCollisionObject::GetTransform ();
  csVector3 absForce = trans.This2Other (force);
  btBody->applyImpulse (btVector3 (absForce.x * system->getInternalScale (),
    absForce.y * system->getInternalScale (),
    absForce.z * system->getInternalScale ()),
    btVector3 (0.0f, 0.0f, 0.0f));
  btBody->setActivationState(ACTIVE_TAG);
}

void csBulletRigidBody::AddRelTorque (const csVector3& torque)
{
  if (!btBody)
    return; 
  csOrthoTransform trans = csBulletCollisionObject::GetTransform ();
  csVector3 absTorque = trans.This2Other (torque);
  btBody->applyTorque (btVector3 (absTorque.x * system->getInternalScale () * system->getInternalScale (),
    absTorque.y * system->getInternalScale () * system->getInternalScale (),
    absTorque.z * system->getInternalScale () * system->getInternalScale ()));
  btBody->setActivationState(ACTIVE_TAG);
}


void csBulletRigidBody::AddForceAtPos (const csVector3& force,
    const csVector3& pos)
{
  if (!btBody)
    return; 

  btVector3 btForce (force.x * system->getInternalScale (),
		     force.y * system->getInternalScale (),
		     force.z * system->getInternalScale ());
  csOrthoTransform trans = csBulletCollisionObject::GetTransform ();
  csVector3 relPos = trans.Other2This (pos);

  btBody->applyImpulse (btForce, btVector3 (relPos.x * system->getInternalScale (),
					  relPos.y * system->getInternalScale (),
					  relPos.z * system->getInternalScale ()));
  btBody->setActivationState(ACTIVE_TAG);
}

void csBulletRigidBody::AddForceAtRelPos (const csVector3& force,
                                          const csVector3& pos)
{
  if (!btBody)
    return; 

  btBody->applyImpulse (btVector3 (force.x * system->getInternalScale (),
			   force.y * system->getInternalScale (),
			   force.z * system->getInternalScale ()),
		btVector3 (pos.x * system->getInternalScale (),
			   pos.y * system->getInternalScale (),
			   pos.z * system->getInternalScale ()));
  btBody->setActivationState(ACTIVE_TAG);
}

void csBulletRigidBody::AddRelForceAtPos (const csVector3& force,
                                          const csVector3& pos)
{
  if (!btBody)
    return; 

  csOrthoTransform trans = csBulletCollisionObject::GetTransform ();
  csVector3 absForce = trans.This2Other (force);
  csVector3 relPos = trans.Other2This (pos);
  btBody->applyImpulse (btVector3 (absForce.x * system->getInternalScale (),
				 absForce.y * system->getInternalScale (),
				 absForce.z * system->getInternalScale ()),
		      btVector3 (relPos.x * system->getInternalScale (),
				 relPos.y * system->getInternalScale (),
				 relPos.z * system->getInternalScale ()));
  btBody->setActivationState(ACTIVE_TAG);
}

void csBulletRigidBody::AddRelForceAtRelPos (const csVector3& force,
                                             const csVector3& pos)
{
  if (!btBody)
    return; 

  csOrthoTransform trans = csBulletCollisionObject::GetTransform ();
  csVector3 absForce = trans.This2Other (force);
  btBody->applyImpulse (btVector3 (absForce.x * system->getInternalScale (),
				 absForce.y * system->getInternalScale (),
				 absForce.z * system->getInternalScale ()),
		      btVector3 (pos.x * system->getInternalScale (),
				 pos.y * system->getInternalScale (),
				 pos.z * system->getInternalScale ()));
  btBody->setActivationState(ACTIVE_TAG);
}

csVector3 csBulletRigidBody::GetForce () const
{
  if (!btBody)
    return csVector3 (0);

  btVector3 force = btBody->getTotalForce ();
  return csVector3 (force.getX () * system->getInverseInternalScale (),
    force.getY () * system->getInverseInternalScale (),
    force.getZ () * system->getInverseInternalScale ());
}

csVector3 csBulletRigidBody::GetTorque () const
{
  if (!btBody)
    return csVector3 (0);

  btVector3 torque = btBody->getTotalTorque ();
  return csVector3
    (torque.getX () * system->getInverseInternalScale () * system->getInverseInternalScale (),
    torque.getY () * system->getInverseInternalScale () * system->getInverseInternalScale (),
    torque.getZ () * system->getInverseInternalScale () * system->getInverseInternalScale ());
}

void csBulletRigidBody::SetLinearDampener (float d)
{
  linearDampening = d;

  if (btBody)
    btBody->setDamping (linearDampening, angularDampening);
}

void csBulletRigidBody::SetRollingDampener (float d)
{
  angularDampening = d;

  if (btBody)
    btBody->setDamping (linearDampening, angularDampening);
}

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

  csBulletRigidBody* rigBody = dynamic_cast<csBulletRigidBody*> (body);
  iMovable* movable = rigBody->GetAttachedMovable ();
  if (movable)
  {
    transform = movable->GetFullTransform ();
    return;
  }
  iCamera* camera = rigBody->GetAttachedCamera ();
  if (camera)
  {
    transform = camera->GetTransform ();
    return;
  }
}
}
CS_PLUGIN_NAMESPACE_END (Bullet2)