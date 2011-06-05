#include "rigidbody2.h"

CS_PLUGIN_NAMESPACE_BEGIN (Bullet2)
{
csBulletRigidBody::csBulletRigidBody (iPhysicalSystem* phySys)
: scfImplementationType (this, phySys), density (1.0f),
/*bodyType (CS::Physics::BODY_RIGID),*/ physicalState (CS::Physics::STATE_DYNAMIC)
{
  
}

csBulletRigidBody::~csBulletRigidBody ()
{
  RemoveBulletObject ();
}

void csBulletRigidBody::RebuildObject ()
{
  //TODO
}

void csBulletRigidBody::RemoveBulletObject ()
{
  if (insideWorld)
  {
    sector->bulletWorld->removeRigidBody (GetBulletRigidPointer ());
    insideWorld = false;
  }
}

bool csBulletRigidBody::Disable ()
{
 SetLinearVelocity (csVector3 (0.0f));
 SetAngularVelocity (csVector3 (0.0f));
 if (btBody)
 {
   btBody->setInterpolationWorldTransform (body->getWorldTransform());
   btBody->setActivationState (ISLAND_SLEEPING);
 }
 return false;
}

bool csBulletRigidBody::Enable ()
{
  if (btBody)
    btObject->setActivationState (ACTIVE_TAG);
}

bool csBulletRigidBody::IsEnabled ()
{
  if (btBody)
    return btBody->isActive ();
  return false;
}

float csBulletRigidBody::GetMass ()
{
  if (physicalState != CS::Physics::STATE_DYNAMIC)
    return 0.0f;

  if (btObject)
    return 1.0f / btBody->getInvMass ();
  return density * GetVolume ();
}

float csBulletRigidBody::SetDensity (float density)
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
    btBody->applyImpulse (btVector3 (force.x * system->internalScale,
      force.y * system->internalScale,
      force.z * system->internalScale),
      btVector3 (0.0f, 0.0f, 0.0f));
    btBody->setActivationState(ACTIVE_TAG);
  }
}

void csBulletRigidBody::SetLinearVelocity (const csVector3& vel)
{
  CS_ASSERT (btBody);
  if (physicalState == CS::Physics::STATE_DYNAMIC)
  {
    btBody->setLinearVelocity (CSToBullet (vel, system->internalScale));
    btBody->activate ();
  }
}

csVector3 csBulletRigidBody::GetLinearVelocity (size_t index /* = 0 */) const
{
  CS_ASSERT (btBody);

  const btVector3& vel = btBody->getLinearVelocity ();
  return BulletToCS (vel, system->inverseInternalScale);
}

void csBulletRigidBody::SetFriction (float friction)
{
  this->friction = friction;
}

bool csBulletRigidBody::SetState (RigidBodyState state)
{
  if (btBody && physicalState != state)
  {
    CS::Physics::RigidBodyState previousState = physicalState;
    if (previousState == CS::Physics::STATE_STATIC)
    {
      bool hasTrimesh = false;
      for (unsigned int i = 0; i < colliders.GetSize (); i++)
        if (colliders[i]->GetGeometryType () == COLLIDER_CONCAVE_MESH
          || colliders[i]->GetGeometryType () == COLLIDER_CONCAVE_MESH_SCALED
          || colliders[i]->GetGeometryType () == COLLIDER_PLANE)
        {
          //These types do not support dynamic/kinematic object.
          return false;
        }
    }

    RemoveBulletObject ();

    btVector3 linearVelocity (0.0f, 0.0f, 0.0f);
    btVector3 angularVelocity (0.0f, 0.0f, 0.0f);

    if (previousState == CS::Physics::STATE_KINEMATIC)
    {
      btBody->setCollisionFlags (body->getCollisionFlags()
        & ~btCollisionObject::CF_KINEMATIC_OBJECT);

      linearVelocity = btBody->getInterpolationLinearVelocity ();
      angularVelocity = btBody->getInterpolationAngularVelocity ();

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

    if (state == CS::Physics::STATE_DYNAMIC)
    {
      btBody->setCollisionFlags (btBody->getCollisionFlags()
        & ~btCollisionObject::CF_STATIC_OBJECT);
      float mass = GetMass ();
      btVector3 localInertia (0.0f, 0.0f, 0.0f);
      compoundShape->calculateLocalInertia (mass, localInertia);
      btBody->setMassProps (mass, localInertia);

      btBody->forceActivationState (ACTIVE_TAG);

      btBody->setLinearVelocity (linearVelocity);
      btBody->setAngularVelocity (angularVelocity);
      btBody->updateInertiaTensor ();
    }
    else if (state == CS::Physics::STATE_KINEMATIC)
    {
      if (!kinematicCb)
        kinematicCb.AttachNew (new csBulletDefaultKinematicCallback ());

      // set body kinematic
      btBody->setMassProps (0.0f, btVector3 (0.0f, 0.0f, 0.0f));
      btBody->setCollisionFlags ((btBody->getCollisionFlags()
        | btCollisionObject::CF_KINEMATIC_OBJECT)
        & ~btCollisionObject::CF_STATIC_OBJECT);
      btBody->setActivationState (DISABLE_DEACTIVATION);    
      btBody->updateInertiaTensor ();
      btBody->setInterpolationWorldTransform (btBody->getWorldTransform ());
      btBody->setInterpolationLinearVelocity (btVector3(0.0f, 0.0f, 0.0f));
      btBody->setInterpolationAngularVelocity (btVector3(0.0f, 0.0f, 0.0f));
    }
    else if (state == CS::Physics::STATE_STATIC)
    {
      btBody->setCollisionFlags (btBody->getCollisionFlags()
        | btCollisionObject::CF_STATIC_OBJECT);
      btBody->setMassProps (0.0f, btVector3 (0.0f, 0.0f, 0.0f));
      btBody->updateInertiaTensor ();
    }

    AddBulletObject ();
    physicalState = state;
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
  CS_ASSERT (btBody);
  if (physicalState == CS::Physics::STATE_DYNAMIC)
  {
    btBody->setAngularVelocity (btVector3 (vel.x, vel.y, vel.z));
    btBody->activate ();
  }
}

csVector3 csBulletRigidBody::GetAngularVelocity () const
{
  CS_ASSERT (btBody);

  const btVector3& vel = btBody->getAngularVelocity ();
  return csVector3 (vel.getX (), vel.getY (), vel.getZ ());
}

void csBulletRigidBody::AddTorque (const csVector3& torque)
{
  CS_ASSERT (btBody);

  btBody->applyTorque (btVector3 (torque.x * system->internalScale * system->internalScale,
    torque.y * system->internalScale * system->internalScale,
    torque.z * system->internalScale * system->internalScale));
  btBody->setActivationState(ACTIVE_TAG);
}

void csBulletRigidBody::AddRelForce (const csVector3& force)
{
  CS_ASSERT (btBody);

  csOrthoTransform trans = GetTransform ();
  csVector3 absForce = trans.This2Other (force);
  btBody->applyImpulse (btVector3 (absForce.x * system->internalScale,
    absForce.y * system->internalScale,
    absForce.z * system->internalScale),
    btVector3 (0.0f, 0.0f, 0.0f));
  btBody->setActivationState(ACTIVE_TAG);
}

void csBulletRigidBody::AddRelTorque (const csVector3& torque)
{
  CS_ASSERT (btBody);

  csOrthoTransform trans = GetTransform ();
  csVector3 absTorque = trans.This2Other (torque);
  btBody->applyTorque (btVector3 (absTorque.x * system->internalScale * system->internalScale,
    absTorque.y * system->internalScale * system->internalScale,
    absTorque.z * system->internalScale * system->internalScale));
  btBody->setActivationState(ACTIVE_TAG);
}


void csBulletRigidBody::AddForceAtPos (const csVector3& force,
    const csVector3& pos)
{
  CS_ASSERT (btBody);

  btVector3 btForce (force.x * system->internalScale,
		     force.y * system->internalScale,
		     force.z * system->internalScale);
  csOrthoTransform trans = GetTransform ();
  csVector3 relPos = trans.Other2This (pos);

  btBody->applyImpulse (btForce, btVector3 (relPos.x * system->internalScale,
					  relPos.y * system->internalScale,
					  relPos.z * system->internalScale));
  btBody->setActivationState(ACTIVE_TAG);
}

void csBulletRigidBody::AddForceAtRelPos (const csVector3& force,
                                          const csVector3& pos)
{
  CS_ASSERT (btBody);

  btBody->applyImpulse (btVector3 (force.x * system->internalScale,
			   force.y * system->internalScale,
			   force.z * system->internalScale),
		btVector3 (pos.x * system->internalScale,
			   pos.y * system->internalScale,
			   pos.z * system->internalScale));
  btBody->setActivationState(ACTIVE_TAG);
}

void csBulletRigidBody::AddRelForceAtPos (const csVector3& force,
                                          const csVector3& pos)
{
  CS_ASSERT (btBody);

  csOrthoTransform trans = GetTransform ();
  csVector3 absForce = trans.This2Other (force);
  csVector3 relPos = trans.Other2This (pos);
  btBody->applyImpulse (btVector3 (absForce.x * system->internalScale,
				 absForce.y * system->internalScale,
				 absForce.z * system->internalScale),
		      btVector3 (relPos.x * system->internalScale,
				 relPos.y * system->internalScale,
				 relPos.z * system->internalScale));
  btBody->setActivationState(ACTIVE_TAG);
}

void csBulletRigidBody::AddRelForceAtRelPos (const csVector3& force,
                                             const csVector3& pos)
{
  CS_ASSERT (btBody);

  csOrthoTransform trans = GetTransform ();
  csVector3 absForce = trans.This2Other (force);
  btBody->applyImpulse (btVector3 (absForce.x * system->internalScale,
				 absForce.y * system->internalScale,
				 absForce.z * system->internalScale),
		      btVector3 (pos.x * system->internalScale,
				 pos.y * system->internalScale,
				 pos.z * system->internalScale));
  btBody->setActivationState(ACTIVE_TAG);
}

csVector3 csBulletRigidBody::GetForce () const
{
  if (!btBody)
    return csVector3 (0);

  btVector3 force = btBody->getTotalForce ();
  return csVector3 (force.getX () * system->inverseInternalScale,
    force.getY () * system->inverseInternalScale,
    force.getZ () * system->inverseInternalScale);
}

csVector3 csBulletRigidBody::GetTorque () const
{
  if (!btBody)
    return csVector3 (0);

  btVector3 torque = btBody->getTotalTorque ();
  return csVector3
    (torque.getX () * system->inverseInternalScale * system->inverseInternalScale,
    torque.getY () * system->inverseInternalScale * system->inverseInternalScale,
    torque.getZ () * system->inverseInternalScale * system->inverseInternalScale);
}

void csBulletRigidBody::SetLinearDampener (float d)
{
  linearDampening = d;

  if (!btBody)
    btBody->setDamping (linearDampening, angularDampening);
}

void csBulletRigidBody::SetRollingDampener (float d)
{
  angularDampening = d;

  if (!btBody)
    btBody->setDamping (linearDampening, angularDampening);
}
}
CS_PLUGIN_NAMESPACE_END (Bullet2)