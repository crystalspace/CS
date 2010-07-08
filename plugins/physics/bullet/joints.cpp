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

// Bullet includes.
#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"

struct btSoftBodyWorldInfo;

#include "joints.h"

CS_PLUGIN_NAMESPACE_BEGIN(Bullet)
{

//------------------------ csBulletJoint ------------------------------------

csBulletJoint::csBulletJoint (csBulletDynamicsSystem* dynsys)
  : scfImplementationType (this), dynSys (dynsys), jointType (BULLET_JOINT_NONE),
    constraint (0), trans_constraint_x (false), trans_constraint_y (false),
    trans_constraint_z (false), min_dist (1000.0f), max_dist (-1000.0f),
    rot_constraint_x (false), rot_constraint_y (false), rot_constraint_z (false),
    min_angle (PI / 2.0f), max_angle (- PI / 2.0f), bounce (0.0f),
    desired_velocity (0.0f), maxforce (0.0f)
{
  angular_constraints_axis[0].Set (0.0f, 1.0f, 0.0f);
  angular_constraints_axis[1].Set (0.0f, 0.0f, 1.0f);
}

csBulletJoint::~csBulletJoint ()
{
  if (constraint)
  {
    dynSys->bulletWorld->removeConstraint (constraint);
    delete constraint;
  }
}

int csBulletJoint::ComputeBestBulletJointType ()
{
  if (trans_constraint_x && trans_constraint_y && trans_constraint_z)
  {
    // All translations are constrainted.
    if (rot_constraint_x && rot_constraint_y && rot_constraint_z)
    {
      // All rotations are constrainted.
      return BULLET_JOINT_6DOF;
    }

    // It seems that the 6DOF joint type is always a better choice, because
    // it is more stable and more powerful.
    else return BULLET_JOINT_6DOF;//BULLET_JOINT_CONETWIST;

    // TODO: other joint types when more appropriate
    // (eg, BULLET_JOINT_POINT2POINT when there are no min/max values)
  }

  return BULLET_JOINT_6DOF;
}

bool csBulletJoint::RebuildJoint ()
{
  // TODO: use transform, bounce, desired_velocity, maxforce, angular_constraints_axis
  // TODO: use btGeneric6DofSpringConstraint if there is some stiffness/damping

  if (constraint)
  {
    dynSys->bulletWorld->removeConstraint (constraint);
    delete constraint;
    constraint = 0;
  }

  if (!bodies[0] || !bodies[1]) return false;
  if (!bodies[0]->body || !bodies[1]->body) return false;

  jointType = ComputeBestBulletJointType ();
  switch (jointType)
  {
    case BULLET_JOINT_6DOF:
      {
	// compute local transforms of the joint
	btTransform frA;
	btTransform frB;

	btTransform jointTransform = CSToBullet (bodies[1]->GetTransform (),
						 dynSys->internalScale);

	bodies[0]->motionState->getWorldTransform (frA);
        frA = frA.inverse () * jointTransform;
	bodies[1]->motionState->getWorldTransform (frB);
        frB = frB.inverse () * jointTransform;

	// create joint
	btGeneric6DofConstraint* dof6 =
	  new btGeneric6DofConstraint (*bodies[0]->body, *bodies[1]->body,
				       frA, frB, true);

	// compute min/max values
	btVector3 minLinear(0.0f, 0.0f, 0.0f);
	btVector3 maxLinear(0.0f, 0.0f, 0.0f);

	if (!trans_constraint_x)
	{
	  minLinear.setX(min_dist[0]);
	  maxLinear.setX(max_dist[0]);
	}
	if (!trans_constraint_y)
	{
	  minLinear.setY(min_dist[1]);
	  maxLinear.setY(max_dist[1]);
	}
	if (!trans_constraint_z)
	{
	  minLinear.setZ(min_dist[2]);
	  maxLinear.setZ(max_dist[2]);
	}

	btVector3 minAngular(0.0f, 0.0f, 0.0f);
	btVector3 maxAngular(0.0f, 0.0f, 0.0f);

	if (!rot_constraint_x)
	{
	  minAngular.setX(min_angle[0]);
	  maxAngular.setX(max_angle[0]);
	}
	if (!rot_constraint_y)
	{
	  minAngular.setY(min_angle[1]);
	  maxAngular.setY(max_angle[1]);
	}
	if (!rot_constraint_z)
	{
	  minAngular.setZ(min_angle[2]);
	  maxAngular.setZ(max_angle[2]);
	}

	// apply min/max values
	dof6->setLinearLowerLimit (minLinear);
	dof6->setLinearUpperLimit (maxLinear);
	dof6->setAngularLowerLimit (minAngular);
	dof6->setAngularUpperLimit (maxAngular);

	constraint = dof6;
      }
      break;

    case BULLET_JOINT_CONETWIST:
      {
	// compute local transforms of the joint
	btTransform frA;
	btTransform frB;

	btTransform jointTransform = CSToBullet (bodies[1]->GetTransform (),
						 dynSys->internalScale);

	bodies[0]->motionState->getWorldTransform (frA);
        frA = frA.inverse () * jointTransform;
	bodies[1]->motionState->getWorldTransform (frB);
        frB = frB.inverse () * jointTransform;

	// create joint
	btConeTwistConstraint* coneTwist = new btConeTwistConstraint (*bodies[0]->body, *bodies[1]->body,
								      frA, frB);

	// apply min/max values
	if (min_angle[0] < max_angle[0])
	  coneTwist->setLimit (0, (max_angle[0] - min_angle[0]) * 5.0f);
	if (min_angle[1] < max_angle[1])
	  coneTwist->setLimit (1, (max_angle[1] - min_angle[1]) * 5.0f);
	if (min_angle[2] < max_angle[2])
	  coneTwist->setLimit (2, (max_angle[2] - min_angle[2]) * 5.0f);

	constraint = coneTwist;
      }
      break;

    case BULLET_JOINT_POINT2POINT:
      {
	// TODO
      }
      break;

    default:
      // @@@ TODO
      break;
  }

  if (constraint)
  {
    dynSys->bulletWorld->addConstraint (constraint, true);
    return true;
  }

  return false;
}
 
void csBulletJoint::Attach (iRigidBody* body1, iRigidBody* body2, bool force_update)
{
  bodies[0] = static_cast<csBulletRigidBody*> ((csBulletRigidBody*) body1);
  bodies[1] = static_cast<csBulletRigidBody*> ((csBulletRigidBody*) body2);
  CS_ASSERT(bodies[0] && bodies[1]);
  if (force_update)
    RebuildJoint ();
}

void csBulletJoint::SetTransform (const csOrthoTransform& trans, bool force_update)
{
  transform = trans;
  if (force_update)
    RebuildJoint ();
}

void csBulletJoint::SetTransConstraints (bool x, bool y, bool z, bool force_update)
{
  trans_constraint_x = x;
  trans_constraint_y = y;
  trans_constraint_z = z;
  if (force_update)
    RebuildJoint ();
}

void csBulletJoint::SetMinimumDistance (const csVector3& min, bool force_update) 
{
  min_dist = min;
  if (force_update)
    RebuildJoint ();
}

void csBulletJoint::SetMaximumDistance (const csVector3& max, bool force_update)
{
  max_dist = max;
  if (force_update)
    RebuildJoint ();
}

void csBulletJoint::SetRotConstraints (bool x, bool y, bool z, bool force_update)
{
  rot_constraint_x = x;
  rot_constraint_y = y;
  rot_constraint_z = z;
  if (force_update)
    RebuildJoint ();
}

void csBulletJoint::SetMinimumAngle (const csVector3& min, bool force_update)
{
  min_angle = min;
  if (force_update)
    RebuildJoint ();
}

void csBulletJoint::SetMaximumAngle (const csVector3& max, bool force_update)
{
  max_angle = max;
  if (force_update)
    RebuildJoint ();
}

void csBulletJoint::SetBounce (const csVector3& bounce, bool force_update)
{
  csBulletJoint::bounce = bounce;
  if (force_update)
    RebuildJoint ();
}

void csBulletJoint::SetDesiredVelocity (const csVector3& velocity, bool force_update)
{
  desired_velocity = velocity;
  if (force_update)
    RebuildJoint ();
}

void csBulletJoint::SetMaxForce (const csVector3& maxForce, bool force_update)
{
  maxforce = maxForce;
  if (force_update)
    RebuildJoint ();
}

void csBulletJoint::SetAngularConstraintAxis (const csVector3& axis,
    int body, bool force_update)
{
  CS_ASSERT (body >= 0 && body <= 1);
  angular_constraints_axis[body] = axis;
  if (force_update)
    RebuildJoint ();
}

csVector3 csBulletJoint::GetAngularConstraintAxis (int body)
{
  CS_ASSERT (body >= 0 && body <= 1);
  return angular_constraints_axis[body];
}

//------------------------ csBulletPivotJoint ------------------------------------

csBulletPivotJoint::csBulletPivotJoint (csBulletDynamicsSystem* dynSys)
  : scfImplementationType (this), dynSys (dynSys)
{
}

csBulletPivotJoint::~csBulletPivotJoint ()
{
  if (constraint)
  {
    dynSys->bulletWorld->removeConstraint (constraint);
    delete constraint;
  }
}

void csBulletPivotJoint::Attach (iRigidBody* body,
				 const csVector3& position)
{
  CS_ASSERT (body);
  this->body = static_cast<csBulletRigidBody*> ((csBulletRigidBody*) body);
  CS_ASSERT (this->body);

  btVector3 localPivot = this->body->body->getCenterOfMassTransform ().inverse ()
    * CSToBullet (position, dynSys->internalScale);

  constraint = new btPoint2PointConstraint
    (*this->body->body, localPivot);
  dynSys->bulletWorld->addConstraint (constraint);

  constraint->m_setting.m_tau = 0.1f;
}

iRigidBody* csBulletPivotJoint::GetAttachedBody () const
{
  return body;
}

void csBulletPivotJoint::SetPosition (const csVector3& position)
{
  if (constraint)
  {
    constraint->setPivotB (CSToBullet (position, dynSys->internalScale));
    body->body->forceActivationState (ACTIVE_TAG);
  }
}

csVector3 csBulletPivotJoint::GetPosition () const
{
  if (!constraint)
    return csVector3 (0.0f);

  return BulletToCS (constraint->getPivotInB (), dynSys->inverseInternalScale);
}

}
CS_PLUGIN_NAMESPACE_END(Bullet)
