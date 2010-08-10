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

#ifndef __CS_BULLET_JOINTS_H__
#define __CS_BULLET_JOINTS_H__

#include "bullet.h"
#include "common.h"

CS_PLUGIN_NAMESPACE_BEGIN(Bullet)
{

#define BULLET_JOINT_NONE 0
#define BULLET_JOINT_HINGE 1
#define BULLET_JOINT_POINT2POINT 2
#define BULLET_JOINT_6DOF 3
#define BULLET_JOINT_CONETWIST 4

class csBulletJoint : public scfImplementation1<csBulletJoint, iJoint>
{
private:
  csBulletDynamicsSystem* dynSys;

  int jointType; // One of BULLET_JOINT_xxx

  // Pointer to either btGeneric6DofContraint (in case of BULLET_JOINT_6DOF),
  // btHingeConstraint (BULLET_JOINT_HINGE), or btPoint2PointConstraint
  // (BULLET_JOINT_POINT2POINT).
  btTypedConstraint* constraint;

  csRef<csBulletRigidBody> bodies[2];

  bool trans_constraint_x;
  bool trans_constraint_y;
  bool trans_constraint_z;
  csVector3 min_dist;
  csVector3 max_dist;

  bool rot_constraint_x;
  bool rot_constraint_y;
  bool rot_constraint_z;
  csVector3 min_angle;
  csVector3 max_angle;

  csVector3 bounce;
  csVector3 desired_velocity;
  csVector3 maxforce;

  csOrthoTransform transform;
  csVector3 angular_constraints_axis[2];

  /**
   * Compute the bullet joint type that best matches the current
   * configuration.
   */
  int ComputeBestBulletJointType ();

public:
  csBulletJoint (csBulletDynamicsSystem* dynSys);
  virtual ~csBulletJoint ();
 
  virtual bool RebuildJoint ();

  virtual void Attach (::iRigidBody* body1, ::iRigidBody* body2,
		       bool force_update = true);
  virtual csRef< ::iRigidBody> GetAttachedBody (int body)
  {
    CS_ASSERT (body >= 0 && body <= 1);
    return bodies[body];
  }

  virtual void SetTransform (const csOrthoTransform& trans,
			     bool force_update = true);
  virtual csOrthoTransform GetTransform () { return transform; }

  virtual void SetTransConstraints (bool x, bool y, bool z,
				    bool force_update = true);
  virtual bool IsXTransConstrained () { return trans_constraint_x; }
  virtual bool IsYTransConstrained () { return trans_constraint_y; }
  virtual bool IsZTransConstrained () { return trans_constraint_z; }
  virtual void SetMinimumDistance (const csVector3& min,
				   bool force_update = true);
  virtual csVector3 GetMinimumDistance () { return min_dist; }
  virtual void SetMaximumDistance (const csVector3& max,
				   bool force_update = true);
  virtual csVector3 GetMaximumDistance () { return max_dist; }

  virtual void SetRotConstraints (bool x, bool y, bool z,
				  bool force_update = true);
  virtual bool IsXRotConstrained () { return rot_constraint_x; }
  virtual bool IsYRotConstrained () { return rot_constraint_y; }
  virtual bool IsZRotConstrained () { return rot_constraint_z; }

  virtual void SetMinimumAngle (const csVector3& min, bool force_update = true);
  virtual csVector3 GetMinimumAngle () { return min_angle; }
  virtual void SetMaximumAngle (const csVector3& max, bool force_update = true);
  virtual csVector3 GetMaximumAngle () { return max_angle; }

  virtual void SetBounce (const csVector3& bounce, bool force_update = true);
  virtual csVector3 GetBounce () { return bounce; }

  virtual void SetDesiredVelocity (const csVector3& velocity,
				   bool force_update = true);
  virtual csVector3 GetDesiredVelocity () { return desired_velocity; }

  virtual void SetMaxForce (const csVector3& maxForce, bool force_update = true);
  virtual csVector3 GetMaxForce () { return maxforce; }

  virtual void SetAngularConstraintAxis (const csVector3& axis, int body,
					 bool force_update = true);
  virtual csVector3 GetAngularConstraintAxis (int body);
};



class csBulletPivotJoint : public scfImplementation1<csBulletPivotJoint,
  iPivotJoint>
{
  friend class csBulletDynamicsSystem;

 public:
  csBulletPivotJoint (csBulletDynamicsSystem* dynSys);
  ~csBulletPivotJoint ();

  //-- iPivotJoint
  virtual void Attach (::iRigidBody* body, const csVector3& position);
  virtual iRigidBody* GetAttachedBody () const;
  virtual void SetPosition (const csVector3& position);
  virtual csVector3 GetPosition () const;

 private:
  csBulletDynamicsSystem* dynSys;
  csRef<csBulletRigidBody> body;
  btPoint2PointConstraint* constraint;
};

}
CS_PLUGIN_NAMESPACE_END(Bullet)

#endif //__CS_BULLET_JOINTS_H__
