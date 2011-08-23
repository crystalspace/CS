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
#include "rigidbody2.h"
#include "softbody2.h"
#include "joint2.h"

// Bullet includes.
#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"

CS_PLUGIN_NAMESPACE_BEGIN (Bullet2)
{
csBulletJoint::csBulletJoint (csBulletSystem* system): scfImplementationType (this), sys (system), 
  rigidJoint (NULL), threshold (FLT_MAX), transConstraintX (false), transConstraintY (false), 
  transConstraintZ (false), minDist (1.0f, 1.0f, 1.0f), maxDist (1.0f, 1.0f, 1.0f), rotConstraintX (false), 
  rotConstraintY (false), rotConstraintZ (false), minAngle (PI / 2.0f), maxAngle (PI / 2.0f), bounce (0.0f), 
  desiredVelocity (0.0f), softJoint (NULL), linearStiff (0.f, 0.f, 0.f), angularStiff (0.f, 0.f, 0.f),
  linearDamp (1.f, 1.f, 1.f), angularDamp (1.f, 1.f, 1.f), linearEquilPoint (0.f, 0.f, 0.f), 
  angularEquilPoint (0.f, 0.f, 0.f), type (RIGID_6DOF_JOINT), jointFlag (0)
{
  float squaredScale = sys->getInternalScale () * sys->getInternalScale ();
  maxforce = btVector3 (0.1f * squaredScale,
    0.1f * squaredScale,
    0.1f * squaredScale);
}

csBulletJoint::~csBulletJoint ()
{
  RemoveBulletJoint ();
}

void csBulletJoint::Attach (CS::Physics2::iPhysicalBody* body1, CS::Physics2::iPhysicalBody* body2, bool forceUpdate)
{
  CS_ASSERT (body1);

  csBulletCollisionObject *collBody1 = dynamic_cast<csBulletCollisionObject*> (body1);
  csBulletCollisionObject* collBody2 = NULL;

  if (body2)
    collBody2 = dynamic_cast<csBulletCollisionObject*> (body2);

  jointFlag |= JOINT_SOFT;
  // If the joint is attached to two bodies.
  if (body2)
  {
    if (body1->GetBodyType () == CS::Physics2::BODY_SOFT)
    {
      bodies[0] = body1;
      bodies[1] = body2;
    }
    else
    {
      // Two rigid bodies.
      if (body2->GetBodyType () == CS::Physics2::BODY_RIGID)
      {
        jointFlag &= ~JOINT_SOFT;
        bool static2 = (body2->QueryRigidBody ()->GetState () == CS::Physics2::STATE_STATIC);

        // The static body should be the first body.
        if (!static2)
        {
          bodies[0] = body1;
          bodies[1] = body2;
        }
        else
        {
          bodies[0] = body2;
          bodies[1] = body1;
        }

      }
      else
      {
        bodies[1] = body1;
        bodies[0] = body2;
      }
    }
  }
  else
  {
    bodies[0] = body1;
    bodies[1] = NULL;

    if (body1->GetBodyType () == CS::Physics2::BODY_RIGID)
      jointFlag &= ~JOINT_SOFT;
    else
    {
      csFPrintf (stderr, "csBulletJoint: Can not attach a joint to only one soft body.\n");
      return;
    }
  }

  if (forceUpdate)
    RebuildJoint ();
}

void csBulletJoint::SetTransform (const csOrthoTransform& trans, bool forceUpdate)
{
  this->transform = trans;
  jointFlag |= JOINT_TRANSFORM;
  if (forceUpdate)
      RebuildJoint ();
}

void csBulletJoint::SetPosition (const csVector3& position, bool forceUpdate)
{
  this->position = position;
  jointFlag |= JOINT_POSITION;

  if (rigidJoint && type == RIGID_PIVOT_JOINT)
  {
    this->transform = bodies[0]->GetTransform ();
    transform.SetOrigin (position);
   
    csBulletRigidBody* body1;
    body1 = dynamic_cast<csBulletRigidBody*> (bodies[0]);

    btTransform jointTransform = CSToBullet (transform , sys->getInternalScale ());

    if (!body1->btBody)
      return;
    
    btGeneric6DofConstraint* dof6 = dynamic_cast<btGeneric6DofConstraint*> (rigidJoint);
    frB = dof6->getFrameOffsetB ();
    dof6->setFrames (jointTransform, frB);

    body1->btBody->forceActivationState (ACTIVE_TAG);
  }
  else if (forceUpdate)
    RebuildJoint ();
}

void csBulletJoint::SetTransConstraints (bool X, bool Y, bool Z, bool forceUpdate)
{
  transConstraintX = X;
  transConstraintY = Y;
  transConstraintZ = Z;
  if (forceUpdate)
    RebuildJoint ();
}

void csBulletJoint::SetMinimumDistance (const csVector3& dist, bool forceUpdate)

{
  minDist = CSToBullet (dist, sys->getInternalScale ());
  if (forceUpdate)
    RebuildJoint ();
}

void csBulletJoint::SetMaximumDistance (const csVector3& dist, bool forceUpdate)
{
  maxDist = CSToBullet (dist, sys->getInternalScale ());
  if (forceUpdate)
    RebuildJoint ();
}

void csBulletJoint::SetRotConstraints (bool X, bool Y, bool Z, bool forceUpdate)
{
  rotConstraintX = X;
  rotConstraintY = Y;
  rotConstraintZ = Z;
  if (forceUpdate)
    RebuildJoint ();
}

void csBulletJoint::SetMinimumAngle (const csVector3& angle, bool forceUpdate)
{
  minAngle = angle;
  if (forceUpdate)
    RebuildJoint ();
}

void csBulletJoint::SetMaximumAngle (const csVector3& angle, bool forceUpdate)
{
  maxAngle = angle;
  if (forceUpdate)
    RebuildJoint ();
}

void csBulletJoint::SetBounce (const csVector3& bounce, bool forceUpdate)
{
  this->bounce = bounce;
  if (forceUpdate)
    RebuildJoint ();
}

void csBulletJoint::SetDesiredVelocity (const csVector3& velo, bool forceUpdate)
{
  desiredVelocity = velo;
  if (forceUpdate)
    RebuildJoint ();
}

void csBulletJoint::SetMaxForce (const csVector3& force, bool forceUpdate)
{
  float squaredScale = sys->getInternalScale () * sys->getInternalScale ();
  maxforce = btVector3 (force.x * squaredScale,
    force.y * squaredScale,
    force.z * squaredScale);

  if (forceUpdate)
    RebuildJoint ();
}

csVector3 csBulletJoint::GetMaxForce () const
{
  float squaredInverseScale = sys->getInverseInternalScale () * sys->getInverseInternalScale ();
  return csVector3 (maxforce.getX () * squaredInverseScale,
    maxforce.getY () * squaredInverseScale,
    maxforce.getZ () * squaredInverseScale);
}

bool csBulletJoint::RebuildJoint ()
{
  if (jointFlag & JOINT_INSIDE_WORLD)
    RemoveBulletJoint ();

  if (bodies[0] == NULL && bodies[1] == NULL) return false;

  if (!(jointFlag & JOINT_SOFT))
  {
    csBulletRigidBody* body1, *body2 = NULL;
    body1 = dynamic_cast<csBulletRigidBody*> (bodies[0]);

    if (type == RIGID_PIVOT_JOINT)
      bodies[1] = NULL;

    if (jointFlag & JOINT_POSITION)
    {
      this->transform = body1->GetTransform ();
      transform.SetOrigin (position);
    }

    if (jointFlag & (JOINT_POSITION | JOINT_TRANSFORM))
    {
      btTransform jointTransform = CSToBullet (transform , sys->getInternalScale ());
      frA = body1->btBody->getCenterOfMassTransform().inverse() * jointTransform;
      if (bodies[1])
      {
        body2 = dynamic_cast<csBulletRigidBody*> (bodies[1]);
        if (!body2 || !body2->btBody)
          return false;
        frB = body2->btBody->getCenterOfMassTransform().inverse() * jointTransform;
      }
    }

    if (!body1->btBody)
      return false;

    if (type == RIGID_HINGE_JOINT)
    {
      btHingeConstraint* pHinge;
      btVector3 btPivotA = frA.getOrigin ();
      btVector3 btAxisA( 0.0f, 0.0f, 0.0f );
      btAxisA[axis] = 1.0f;
      if (bodies[1])
      {
        body2 = dynamic_cast<csBulletRigidBody*> (bodies[1]);
        if (!body2 || !body2->btBody)
          return false;
        btVector3 btPivotB = frB.getOrigin ();
        pHinge = new btHingeConstraint( *body1->btBody, *body2->btBody,
          btPivotA, btPivotB, btAxisA, btAxisA, true);
      }
      else
        pHinge = new btHingeConstraint( *body1->btBody, btPivotA, btAxisA );

      if (!desiredVelocity.IsZero (EPSILON))
        pHinge->enableAngularMotor(true, desiredVelocity[axis], maxforce[axis]); 
      rigidJoint = pHinge;
    }
    else
    {
      btGeneric6DofConstraint* dofJoint;
      if (bodies[1])
      {
        body2 = dynamic_cast<csBulletRigidBody*> (bodies[1]);
        if (!body2 || !body2->btBody)
          return false;

        if (jointFlag & JOINT_SPRING)
        {
          btGeneric6DofSpringConstraint* springJoint = new btGeneric6DofSpringConstraint (
            *(body1->btBody), *(body2->btBody), frA, frB, true);
          if (transConstraintX && abs(linearStiff[0]) > EPSILON)
          {
            springJoint->enableSpring (0, true);
            springJoint->setStiffness (0, linearStiff[0]);
            springJoint->setDamping (0, linearDamp[0]);
            if (jointFlag & JOINT_EQUIL_POINT)
              springJoint->setEquilibriumPoint (0, linearEquilPoint[0]);
          }
          if (transConstraintY && abs(linearStiff[1]) > EPSILON)
          {
            springJoint->enableSpring (1, true);
            springJoint->setStiffness (1, linearStiff[1]);
            springJoint->setDamping (1, linearDamp[1]);
            if (jointFlag & JOINT_EQUIL_POINT)
              springJoint->setEquilibriumPoint (1, linearEquilPoint[1]);
          }
          if (transConstraintZ && abs(linearStiff[2]) > EPSILON)
          {
            springJoint->enableSpring (2, true);
            springJoint->setStiffness (2, linearStiff[2]);
            springJoint->setDamping (2, linearDamp[2]);
            if (jointFlag & JOINT_EQUIL_POINT)
              springJoint->setEquilibriumPoint (2, linearEquilPoint[2]);
          }
          if (rotConstraintX && abs(angularStiff[0]) > EPSILON)
          {
            springJoint->enableSpring (3, true);
            springJoint->setStiffness (3, angularStiff[0]);
            springJoint->setDamping (3, angularDamp[0]);
            if (jointFlag & JOINT_EQUIL_POINT)
              springJoint->setEquilibriumPoint (3, angularEquilPoint[0]);
          }
          if (rotConstraintY && abs(angularStiff[1]) > EPSILON)
          {
            springJoint->enableSpring (4, true);
            springJoint->setStiffness (4, angularStiff[1]);
            springJoint->setDamping (4, angularDamp[1]);
            if (jointFlag & JOINT_EQUIL_POINT)
              springJoint->setEquilibriumPoint (4, angularEquilPoint[1]);
          }
          if (rotConstraintZ && abs(angularStiff[2]) > EPSILON)
          {
            springJoint->enableSpring (5, true);
            springJoint->setStiffness (5, angularStiff[2]);
            springJoint->setDamping (5, angularDamp[2]);
            if (jointFlag & JOINT_EQUIL_POINT)
              springJoint->setEquilibriumPoint (5, angularEquilPoint[2]);
          }
          if (!(jointFlag & JOINT_EQUIL_POINT))
            springJoint->setEquilibriumPoint ();
          dofJoint = springJoint;
        }
        else
          dofJoint = new btGeneric6DofConstraint (*(body1->btBody), *(body2->btBody),
          frA, frB, true);
      }
      else
      {
        if (jointFlag & JOINT_SPRING)
          return false;
        else
          dofJoint = new btGeneric6DofConstraint (*(body1->btBody), frA, true);
      }


      btVector3 minLinear(0.0f, 0.0f, 0.0f);
      btVector3 maxLinear(-1.0f, -1.0f, -1.0f);
      btVector3 minAngular(0.0f, 0.0f, 0.0f);
      btVector3 maxAngular(-1.0f, -1.0f, -1.0f);

      if (transConstraintX)
      {
        minLinear.setX(minDist[0]);
        maxLinear.setX(maxDist[0]);
      }
      if (transConstraintY)
      {
        minLinear.setY(minDist[1]);
        maxLinear.setY(maxDist[1]);
      }
      if (transConstraintZ)
      {
        minLinear.setZ(minDist[2]);
        maxLinear.setZ(maxDist[2]);
      }

      if (rotConstraintX)
      {
        minAngular.setX(minAngle[0]);
        maxAngular.setX(maxAngle[0]);
      }
      if (rotConstraintY)
      {
        minAngular.setY(minAngle[1]);
        maxAngular.setY(maxAngle[1]);
      }
      if (rotConstraintZ)
      {
        minAngular.setZ(minAngle[2]);
        maxAngular.setZ(maxAngle[2]);
      }

      // apply min/max values
      dofJoint->setLinearLowerLimit (minLinear);
      dofJoint->setLinearUpperLimit (maxLinear);
      dofJoint->setAngularLowerLimit (minAngular);
      dofJoint->setAngularUpperLimit (maxAngular);

      // apply the parameters for the motor
      if (fabs (desiredVelocity[0]) > EPSILON)
      {
        btRotationalLimitMotor* motor = dofJoint->getRotationalLimitMotor (0);
        motor->m_enableMotor = true;
        motor->m_targetVelocity = desiredVelocity[0];
        motor->m_maxMotorForce = maxforce[0];
      }
      dofJoint->getRotationalLimitMotor (0)->m_bounce = bounce[0];

      if (fabs (desiredVelocity[1]) > EPSILON)
      {
        printf ("Setting motor\n");
        btRotationalLimitMotor* motor = dofJoint->getRotationalLimitMotor (1);
        motor->m_enableMotor = true;
        motor->m_targetVelocity = desiredVelocity[1];
        motor->m_maxMotorForce = maxforce[1];
        motor->m_damping = 0.1f;
      }
      dofJoint->getRotationalLimitMotor (1)->m_bounce = bounce[1];

      if (fabs (desiredVelocity[2]) > EPSILON)
      {
        btRotationalLimitMotor* motor = dofJoint->getRotationalLimitMotor (2);
        motor->m_enableMotor = true;
        motor->m_targetVelocity = desiredVelocity[2];
        motor->m_maxMotorForce = maxforce[2];
        motor->m_damping = 0.1f;
      }
      dofJoint->getRotationalLimitMotor (2)->m_bounce = bounce[2];
      rigidJoint = dofJoint;
    } 
    rigidJoint->setBreakingImpulseThreshold (threshold * sys->getInternalScale ());
    rigidJoint->setDbgDrawSize(sys->getInternalScale ());
  }
  return true;
}

void csBulletJoint::SetSpring (bool isSpring, bool forceUpdate)
{
  if (isSpring)
    jointFlag |= JOINT_SPRING;
  else
    jointFlag &= ~JOINT_SPRING;
  if (forceUpdate)
    RebuildJoint ();
}

void csBulletJoint::SetLinearStiffness (csVector3 stiff, bool forceUpdate)
{
  if (jointFlag & JOINT_SPRING)
  {
    linearStiff = stiff;
    if (forceUpdate)
      RebuildJoint ();
  }
}

void csBulletJoint::SetAngularStiffness (csVector3 stiff, bool forceUpdate)
{
  if (jointFlag & JOINT_SPRING)
  {
    angularStiff = stiff;
    if (forceUpdate)
      RebuildJoint ();
  }
}

void csBulletJoint::SetLinearDamping (csVector3 damp, bool forceUpdate)
{
  if (jointFlag & JOINT_SPRING)
  {
    linearDamp = damp;
    if (forceUpdate)
      RebuildJoint ();
  }
}

void csBulletJoint::SetAngularDamping (csVector3 damp, bool forceUpdate)
{
  if (jointFlag & JOINT_SPRING)
  {
    angularDamp = damp;
    if (forceUpdate)
      RebuildJoint ();
  }
}

void csBulletJoint::SetLinearEquilibriumPoint (csVector3 point, bool forceUpdate)
{
  if (jointFlag & JOINT_SPRING)
  {
    linearEquilPoint = point;
    jointFlag |= JOINT_EQUIL_POINT;
    if (forceUpdate)
      RebuildJoint ();
  }
}

void csBulletJoint::SetAngularEquilibriumPoint (csVector3 point, bool forceUpdate)
{
  if (jointFlag & JOINT_SPRING)
  {
    angularEquilPoint = point;
    jointFlag |= JOINT_EQUIL_POINT;
    if (forceUpdate)
      RebuildJoint ();
  }
}

void csBulletJoint::SetBreakingImpulseThreshold (float threshold, bool forceUpdate)
{
  this->threshold = threshold;
  if (rigidJoint)
    rigidJoint->setBreakingImpulseThreshold (threshold * sys->getInternalScale ());
  else
    if (forceUpdate)
      RebuildJoint ();
}

void csBulletJoint::AddBulletJoint ()
{
  if (jointFlag & JOINT_INSIDE_WORLD)
    RemoveBulletJoint ();

  csBulletCollisionObject *collBody1 = dynamic_cast<csBulletCollisionObject*> (bodies[0]);
  csBulletCollisionObject* collBody2 = NULL;
  if (bodies[1])
    collBody2 = dynamic_cast<csBulletCollisionObject*> (bodies[1]);
  if (collBody1->sector != sector)
    csFPrintf (stderr, "csBulletJoint: Can not attach a joint to bodies in different sectors.\n");
  else if (collBody2 && (collBody2->sector != sector))
    csFPrintf (stderr, "csBulletJoint: Can not attach a joint to bodies in different sectors.\n");

  collBody1->joints.Push (this);
  if (bodies[1])
    collBody2->joints.Push (this);

  if (!(jointFlag & JOINT_SOFT))
  {
    sector->bulletWorld->addConstraint (rigidJoint, true);
    jointFlag |= JOINT_INSIDE_WORLD;
  }
  else
  {
    csBulletSoftBody* body = dynamic_cast<csBulletSoftBody*> (bodies[0]);
    if (!body->GetClusterCollisionRS () && !body->GetClusterCollisionSS ())
      return;
    if (type == SOFT_LINEAR_JOINT)
    {
      btSoftBody::LJoint::Specs	lspecs;
      lspecs.cfm		=	1;
      lspecs.erp		=	1; 
      lspecs.position = CSToBullet (position, sys->getInternalScale ());
      if (bodies[1]->GetBodyType () == CS::Physics2::BODY_RIGID)
      {  
        csBulletRigidBody* body2 = dynamic_cast<csBulletRigidBody*> (bodies[1]);
        body->btBody->appendLinearJoint (lspecs, body2->btBody);
      }
      else
      {
        csBulletSoftBody* body2 = dynamic_cast<csBulletSoftBody*> (bodies[1]);
        if (!body2->GetClusterCollisionRS () && !body2->GetClusterCollisionSS ())
          return;
        body->btBody->appendLinearJoint (lspecs, body2->btBody);
      }
    }
    else if (type == SOFT_ANGULAR_JOINT)
    {
      btSoftBody::AJoint::Specs	aspecs;
      aspecs.cfm		=	1;
      aspecs.erp		=	1;
      if (!rotConstraintX)
        aspecs.axis = btVector3(1,0,0);
      else if (!rotConstraintY)
        aspecs.axis = btVector3(0,1,0);
      else if (!rotConstraintZ)
        aspecs.axis = btVector3(0,0,1);

      if (bodies[1]->GetBodyType () == CS::Physics2::BODY_RIGID)
      {  
        csBulletRigidBody* body2 = dynamic_cast<csBulletRigidBody*> (bodies[1]);
        body->btBody->appendAngularJoint (aspecs, body2->btBody);
      }
      else
      {
        csBulletSoftBody* body2 = dynamic_cast<csBulletSoftBody*> (bodies[1]);
        if (!body2->GetClusterCollisionRS () && !body2->GetClusterCollisionSS ())
          return;
        body->btBody->appendAngularJoint (aspecs, body2->btBody);
      }
    }
    else
      return;
    softJoint = body->btBody->m_joints[body->btBody->m_joints.size()-1];
    jointFlag |= JOINT_INSIDE_WORLD;
  }
}

void csBulletJoint::RemoveBulletJoint ()
{
  if (jointFlag & JOINT_INSIDE_WORLD)
  {
    if (rigidJoint)
    {
      sector->bulletWorld->removeConstraint (rigidJoint);
      delete rigidJoint;
      rigidJoint = NULL;
    }
    if (softJoint)
    {
      csBulletSoftBody* body = dynamic_cast<csBulletSoftBody*> (bodies[0]);
      body->btBody->m_joints.remove (softJoint);
      softJoint = NULL;
    }

    jointFlag &= ~(JOINT_INSIDE_WORLD | JOINT_POSITION | JOINT_TRANSFORM);
    //jointFlag = 0;
    sector = NULL;
    /*transConstraintX = transConstraintY = transConstraintZ = false;
    rotConstraintX = rotConstraintY = rotConstraintZ = false;
    linearStiff.Set (0.f, 0.f, 0.f);
    angularStiff.Set (0.f, 0.f, 0.f);
    linearDamp.Set (1.f, 1.f, 1.f);
    angularDamp.Set (1.f, 1.f, 1.f);
    desiredVelocity.Set (0.0f);
    maxforce.setZero ();
    bounce.Set (0.0f);*/
    csBulletCollisionObject *collBody1 = dynamic_cast<csBulletCollisionObject*> (bodies[0]);
    collBody1->joints.Delete (this);
    if (bodies[1])
    {  
      csBulletCollisionObject* collBody2 = dynamic_cast<csBulletCollisionObject*> (bodies[1]);
      collBody2->joints.Delete (this);
    }
  }
}
}
CS_PLUGIN_NAMESPACE_END (Bullet2)
