#include "rigidbody2.h"

CS_PLUGIN_NAMESPACE_BEGIN (Bullet2)
{
csBulletRigidBody::csBulletRigidBody (iPhysicalSystem* phySys)
: scfImplementationType (this), csBulletCollisionObject (phySys), density (1.0f),
bodyType (CS::Physics::BODY_RIGID), physicalState (CS::Physics::STATE_DYNAMIC)
{
  
}

csBulletRigidBody::~csBulletRigidBody ()
{
  RemoveBulletObject ();
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
 if (btObject)
 {
   btRigidBody* body = GetBulletRigidPointer ();
   body->setInterpolationWorldTransform (body->getWorldTransform());
   body->setActivationState (ISLAND_SLEEPING);
 }
 return false;
}

bool csBulletRigidBody::Enable ()
{
  if (btObject)
    btObject->setActivationState (ISLAND_SLEEPING);
}

bool csBulletRigidBody::IsEnabled ()
{
  if (btObject)
    return btObject->isActive ();
  return false;
}

float csBulletRigidBody::GetMass ()
{
  if (physicalState != CS::Physics::STATE_DYNAMIC)
    return 0.0f;

  if (btObject)
  {
    btRigidBody* body = GetBulletRigidPointer ();
    return 1.0f / body->getInvMass ();
  }
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
  if (btObject)
  {
    btRigidBody* body = GetBulletRigidPointer ();
    body->applyImpulse (btVector3 (force.x * system->internalScale,
      force.y * system->internalScale,
      force.z * system->internalScale),
      btVector3 (0.0f, 0.0f, 0.0f));
    body->setActivationState(ACTIVE_TAG);
  }
}

void csBulletRigidBody::SetLinearVelocity (const csVector3& vel)
{
  CS_ASSERT (btObject);
  if (physicalState == CS::Physics::STATE_DYNAMIC)
  {
    GetBulletRigidPointer ()->setLinearVelocity (CSToBullet (vel, system->internalScale));
    GetBulletRigidPointer ()->activate ();
  }
}

csVector3 csBulletRigidBody::GetLinearVelocity (size_t index /* = 0 */) const
{
  CS_ASSERT (btObject);

  const btVector3& vel = GetBulletRigidPointer ()->getLinearVelocity ();
  return BulletToCS (vel, system->inverseInternalScale);
}

void csBulletRigidBody::SetFriction (float friction)
{
  this->friction = friction;
}

void csBulletRigidBody::SetState (RigidBodyState state)
{
  //TODO
}

void csBulletRigidBody::SetElasticity (float elasticity)
{
  this->elasticity = elasticity;
}

void csBulletRigidBody::SetAngularVelocity (const csVector3& vel)
{
  CS_ASSERT (btObject);
  if (physicalState == CS::Physics::STATE_DYNAMIC)
  {
    GetBulletRigidPointer ()->setAngularVelocity (btVector3 (vel.x, vel.y, vel.z));
    GetBulletRigidPointer ()->activate ();
  }
}

csVector3 csBulletRigidBody::GetAngularVelocity () const
{
  CS_ASSERT (btObject);

  const btVector3& vel = GetBulletRigidPointer ()->getAngularVelocity ();
  return csVector3 (vel.getX (), vel.getY (), vel.getZ ());
}

void csBulletRigidBody::AddTorque (const csVector3& torque)
{
  CS_ASSERT (btObject);

  btRigidBody* body = GetBulletRigidPointer ();
  body->applyTorque (btVector3 (torque.x * system->internalScale * system->internalScale,
    torque.y * system->internalScale * system->internalScale,
    torque.z * system->internalScale * system->internalScale));
  body->setActivationState(ACTIVE_TAG);
}

void csBulletRigidBody::AddRelForce (const csVector3& force)
{
  CS_ASSERT (btObject);

  csOrthoTransform trans = GetTransform ();
  csVector3 absForce = trans.This2Other (force);
  GetBulletRigidPointer ()->applyImpulse (btVector3 (absForce.x * system->internalScale,
    absForce.y * system->internalScale,
    absForce.z * system->internalScale),
    btVector3 (0.0f, 0.0f, 0.0f));
  GetBulletRigidPointer ()->setActivationState(ACTIVE_TAG);
}

void csBulletRigidBody::AddRelTorque (const csVector3& torque)
{
  CS_ASSERT (btObject);

  csOrthoTransform trans = GetTransform ();
  csVector3 absTorque = trans.This2Other (torque);
  GetBulletRigidPointer ()->applyTorque (btVector3 (absTorque.x * system->internalScale * system->internalScale,
    absTorque.y * system->internalScale * system->internalScale,
    absTorque.z * system->internalScale * system->internalScale));
  GetBulletRigidPointer ()->setActivationState(ACTIVE_TAG);
}


void csBulletRigidBody::AddForceAtPos (const csVector3& force,
    const csVector3& pos)
{
  CS_ASSERT (btObject);

  btRigidBody* body = GetBulletRigidPointer ();

  btVector3 btForce (force.x * system->internalScale,
		     force.y * system->internalScale,
		     force.z * system->internalScale);
  csOrthoTransform trans = GetTransform ();
  csVector3 relPos = trans.Other2This (pos);

  body->applyImpulse (btForce, btVector3 (relPos.x * system->internalScale,
					  relPos.y * system->internalScale,
					  relPos.z * system->internalScale));
  body->setActivationState(ACTIVE_TAG);
}

void csBulletRigidBody::AddForceAtRelPos (const csVector3& force,
                                          const csVector3& pos)
{
  CS_ASSERT (btObject);

  btRigidBody* body = GetBulletRigidPointer ();
  body->applyImpulse (btVector3 (force.x * system->internalScale,
			   force.y * system->internalScale,
			   force.z * system->internalScale),
		btVector3 (pos.x * system->internalScale,
			   pos.y * system->internalScale,
			   pos.z * system->internalScale));
  body->setActivationState(ACTIVE_TAG);
}

void csBulletRigidBody::AddRelForceAtPos (const csVector3& force,
                                          const csVector3& pos)
{
  CS_ASSERT (btObject);

  btRigidBody* body = GetBulletRigidPointer ();

  csOrthoTransform trans = GetTransform ();
  csVector3 absForce = trans.This2Other (force);
  csVector3 relPos = trans.Other2This (pos);
  body->applyImpulse (btVector3 (absForce.x * system->internalScale,
				 absForce.y * system->internalScale,
				 absForce.z * system->internalScale),
		      btVector3 (relPos.x * system->internalScale,
				 relPos.y * system->internalScale,
				 relPos.z * system->internalScale));
  body->setActivationState(ACTIVE_TAG);
}

void csBulletRigidBody::AddRelForceAtRelPos (const csVector3& force,
                                             const csVector3& pos)
{
  CS_ASSERT (btObject);

  btRigidBody* body = GetBulletRigidPointer ();

  csOrthoTransform trans = GetTransform ();
  csVector3 absForce = trans.This2Other (force);
  body->applyImpulse (btVector3 (absForce.x * system->internalScale,
				 absForce.y * system->internalScale,
				 absForce.z * system->internalScale),
		      btVector3 (pos.x * system->internalScale,
				 pos.y * system->internalScale,
				 pos.z * system->internalScale));
  body->setActivationState(ACTIVE_TAG);
}

csVector3 csBulletRigidBody::GetForce () const
{
  if (!btObject)
    return csVector3 (0);

  btVector3 force = GetBulletRigidPointer ()->getTotalForce ();
  return csVector3 (force.getX () * system->inverseInternalScale,
    force.getY () * system->inverseInternalScale,
    force.getZ () * system->inverseInternalScale);
}

csVector3 csBulletRigidBody::GetTorque () const
{
  if (!btObject)
    return csVector3 (0);

  btVector3 torque = GetBulletRigidPointer ()->getTotalTorque ();
  return csVector3
    (torque.getX () * system->inverseInternalScale * system->inverseInternalScale,
    torque.getY () * system->inverseInternalScale * system->inverseInternalScale,
    torque.getZ () * system->inverseInternalScale * system->inverseInternalScale);
}

void csBulletRigidBody::SetLinearDampener (float d)
{
  linearDampening = d;

  if (!btObject)
    GetBulletRigidPointer ()->setDamping (linearDampening, angularDampening);
}

void csBulletRigidBody::SetRollingDampener (float d)
{
  angularDampening = d;

  if (!btObject)
    GetBulletRigidPointer ()->setDamping (linearDampening, angularDampening);
}
}
CS_PLUGIN_NAMESPACE_END (Bullet2)