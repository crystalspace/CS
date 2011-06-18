#include "cssysdef.h"
#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"

#include "rigidbody2.h"

CS_PLUGIN_NAMESPACE_BEGIN (Bullet2)
{
csBulletRigidBody::csBulletRigidBody (csBulletSystem* phySys)
: scfImplementationType (this, phySys), density (1.0f),
physicalState (CS::Physics::STATE_DYNAMIC)
{
  type = CS::Collision::COLLISION_OBJECT_PHYSICAL;
}

csBulletRigidBody::~csBulletRigidBody ()
{
  RemoveBulletObject ();
}

//Lulu: Seems like not neccessary..
//void csBulletRigidBody::RebuildObject ()
//{
//  //TODO
//  size_t colliderCount = colliders.GetSize ();
//  if (colliderCount == 0)
//  {  
//    csFPrintf  (stderr, "csBulletCollisionObject: Haven't add any collider to the object.\nRebuild failed.\n");
//    return;
//  }
//
//  if(compoundShape)
//    delete compoundShape;
//
//  if(colliderCount >= 2)
//  {  
//    compoundShape = new btCompoundShape();
//    for (size_t i = 0; i < colliderCount; i++)
//    {
//      btTransform relaTrans = CSToBullet (relaTransforms[i], system->getInternalScale ());
//      compoundShape->addChildShape (relaTrans, colliders[i]->shape);
//    }
//    //Shift children shape?
//  }
//
//  bool wasInWorld = false;
//  if (insideWorld)
//  {
//    wasInWorld = true;
//    RemoveBulletObject ();
//  }
//
//  btCollisionShape* shape;
//  if (compoundShape == NULL)
//  {
//    //only one collider.
//    shape = colliders[0]->shape;
//  }
//  else if (compoundChanged)
//  {
//    //use compound shape.
//    shape = compoundShape;
//  }
//
//  if (wasInWorld)
//    AddBulletObject ();
//}

void csBulletRigidBody::AddCollider (CS::Collision::iCollider* collider, 
                                     const csOrthoTransform& relaTrans)
{
  csRef<csBulletCollider> coll (dynamic_cast<csBulletCollider*>(collider));
  if (physicalState != CS::Physics::STATE_STATIC)
  {
    CS::Collision::ColliderType type = collider->GetGeometryType ();
    if (type == CS::Collision::COLLIDER_CONCAVE_MESH
      ||type == CS::Collision::COLLIDER_CONCAVE_MESH_SCALED
      ||type == CS::Collision::COLLIDER_PLANE
      ||type == CS::Collision::COLLIDER_TERRAIN)
    {
      csFPrintf (stderr, "csBulletRigidBody: Can not add static collider to non-static body.\n");
      return;
    }
  }
  colliders.Push (coll);
  relaTransforms.Push (relaTrans);
  shapeChanged = true;
}

void csBulletRigidBody::RemoveBulletObject ()
{
  if (insideWorld)
  {
    sector->bulletWorld->removeRigidBody (btBody);
    delete btBody;
    btBody = NULL;
    btObject = NULL;
    insideWorld = false;
  }
}

void csBulletRigidBody::AddBulletObject ()
{
  if (insideWorld)
    RemoveBulletObject ();

  btVector3 localInertia (0.0f, 0.0f, 0.0f);
  float mass = GetMass ();
  //Create btRigidBody
  if (compoundShape)
  {
    int shapeCount = compoundShape->getNumChildShapes ();
    if (shapeChanged)
    {
      btTransform principalAxis;
      btVector3 principalInertia;
      compoundShape->calculatePrincipalAxisTransform
        (&mass, principalAxis, principalInertia);

      // create new motion state
      btTransform trans;
      motionState->getWorldTransform (trans);
      trans = trans * motionState->inversePrincipalAxis;
      delete motionState;
      motionState = new csBulletMotionState (this, trans * principalAxis,
        principalAxis);

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

    compoundShape->calculateLocalInertia (mass, localInertia);
  }

  btRigidBody::btRigidBodyConstructionInfo infos (mass, motionState,
    compoundShape, localInertia);

  infos.m_friction = friction;
  infos.m_restitution = elasticity;
  infos.m_linearDamping = linearDampening;
  infos.m_angularDamping = angularDampening;

  // create new rigid body
  btBody = new btRigidBody (infos);
  btObject = btBody;

  sector->bulletWorld->addRigidBody (btBody);
  btBody->setUserPointer (static_cast<iPhysicalBody*> (this));
  insideWorld = true;
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

float csBulletRigidBody::GetMass ()
{
  if (physicalState != CS::Physics::STATE_DYNAMIC)
    return 0.0f;

  if (btObject)
    return 1.0f / btBody->getInvMass ();
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
  CS_ASSERT (btBody);
  if (physicalState == CS::Physics::STATE_DYNAMIC)
  {
    btBody->setLinearVelocity (CSToBullet (vel, system->getInternalScale ()));
    btBody->activate ();
  }
}

csVector3 csBulletRigidBody::GetLinearVelocity (size_t index /* = 0 */) const
{
  CS_ASSERT (btBody);

  const btVector3& vel = btBody->getLinearVelocity ();
  return BulletToCS (vel, system->getInverseInternalScale ());
}

void csBulletRigidBody::SetFriction (float friction)
{
  this->friction = friction;
}

bool csBulletRigidBody::SetState (CS::Physics::RigidBodyState state)
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

    if (insideWorld)
      sector->bulletWorld->removeRigidBody (btBody);

    btVector3 linearVelocity (0.0f, 0.0f, 0.0f);
    btVector3 angularVelocity (0.0f, 0.0f, 0.0f);

    if (previousState == CS::Physics::STATE_KINEMATIC)
    {
      btBody->setCollisionFlags (btBody->getCollisionFlags()
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

    if (insideWorld)
      sector->bulletWorld->addRigidBody (btBody);
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

  btBody->applyTorque (btVector3 (torque.x * system->getInternalScale () * system->getInternalScale (),
    torque.y * system->getInternalScale () * system->getInternalScale (),
    torque.z * system->getInternalScale () * system->getInternalScale ()));
  btBody->setActivationState(ACTIVE_TAG);
}

void csBulletRigidBody::AddRelForce (const csVector3& force)
{
  CS_ASSERT (btBody);

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
  CS_ASSERT (btBody);

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
  CS_ASSERT (btBody);

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
  CS_ASSERT (btBody);

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
  CS_ASSERT (btBody);

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
  CS_ASSERT (btBody);

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

  if (!btBody)
    btBody->setDamping (linearDampening, angularDampening);
}

void csBulletRigidBody::SetRollingDampener (float d)
{
  angularDampening = d;

  if (!btBody)
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
    transform = movable->GetTransform ();
    return;
  }  
}
}
CS_PLUGIN_NAMESPACE_END (Bullet2)