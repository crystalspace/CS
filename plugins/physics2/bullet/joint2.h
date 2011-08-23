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

#ifndef __CS_BULLET_JOINT_H__
#define __CS_BULLET_JOINT_H__

#include "bullet2.h"
#include "common2.h"
#include "BulletSoftBody/btSoftBody.h"

class btGeneric6DofConstraint;

CS_PLUGIN_NAMESPACE_BEGIN (Bullet2)
{
#define JOINT_SOFT 1
#define JOINT_SPRING 2
#define JOINT_POSITION 4
#define JOINT_TRANSFORM 8
#define JOINT_EQUIL_POINT 16
#define JOINT_INSIDE_WORLD 32

enum csJointType
{
  RIGID_PIVOT_JOINT,
  RIGID_P2P_JOINT,
  RIGID_HINGE_JOINT,
  RIGID_SLIDE_JOINT,
  RIGID_CONETWIST_JOINT,
  RIGID_6DOF_JOINT,
  SOFT_LINEAR_JOINT,
  SOFT_ANGULAR_JOINT,
};

class csBulletJoint : public scfImplementation1<
  csBulletJoint, CS::Physics2::iJoint>
{
  friend class csBulletSystem;
  friend class csBulletSector;
private:
  csOrthoTransform transform;
  csVector3 linearStiff;
  csVector3 angularStiff;
  csVector3 linearDamp;
  csVector3 angularDamp;
  csVector3 linearEquilPoint;
  csVector3 angularEquilPoint;

  csVector3 position;
  csVector3 minAngle;
  csVector3 maxAngle;

  csVector3 bounce;
  csVector3 desiredVelocity;
  btVector3 maxforce;
  btTransform frA, frB;
  btVector3 minDist;
  btVector3 maxDist;
  csJointType type;
  csBulletSystem* sys;
  csBulletSector* sector;
  btTypedConstraint* rigidJoint;
  btSoftBody::Joint* softJoint;
  iPhysicalBody* bodies[2];
  float threshold;
  int axis;

  bool transConstraintX;
  bool transConstraintY;
  bool transConstraintZ;

  bool rotConstraintX;
  bool rotConstraintY;
  bool rotConstraintZ;

  char jointFlag;

public:
  csBulletJoint (csBulletSystem* system);
  virtual ~csBulletJoint ();

  void SetType (csJointType type) {this->type = type;}

  virtual void Attach (CS::Physics2::iPhysicalBody* body1, CS::Physics2::iPhysicalBody* body2,
    bool forceUpdate = true);

  virtual CS::Physics2::iPhysicalBody* GetAttachedBody (int index)
  {
    CS_ASSERT (index >= 0 && index <= 1);
    return bodies[index];
  }

  virtual void SetTransform (const csOrthoTransform& trans,
    bool forceUpdate = false);

  virtual csOrthoTransform GetTransform () const 
  {return transform;}

  virtual void SetPosition (const csVector3& position,
    bool forceUpdate = false);
  virtual csVector3 GetPosition () const {return position;}

  virtual void SetTransConstraints (bool X, 
    bool Y, bool Z, bool forceUpdate = false);
  virtual bool IsXTransConstrained () { return transConstraintX; }
  virtual bool IsYTransConstrained () { return transConstraintY; }
  virtual bool IsZTransConstrained () { return transConstraintZ; }

  virtual void SetMinimumDistance (const csVector3& dist,
    bool forceUpdate = false);
  virtual csVector3 GetMinimumDistance () const
  { return BulletToCS (minDist, sys->getInverseInternalScale ()); }
  virtual void SetMaximumDistance (const csVector3& dist,
    bool forceUpdate = false);
  virtual csVector3 GetMaximumDistance () const
  { return BulletToCS (maxDist, sys->getInverseInternalScale ()); }

  virtual void SetRotConstraints (bool X, 
    bool Y, bool Z, bool forceUpdate = false);
  virtual bool IsXRotConstrained () { return rotConstraintX; }
  virtual bool IsYRotConstrained () { return rotConstraintY; }
  virtual bool IsZRotConstrained () { return rotConstraintZ; }

  virtual void SetMinimumAngle (const csVector3& angle,
    bool forceUpdate = false);
  virtual csVector3 GetMinimumAngle () const {return minAngle;}
  virtual void SetMaximumAngle (const csVector3& angle,
    bool forceUpdate = false);
  virtual csVector3 GetMaximumAngle () const {return maxAngle;}

  virtual void SetBounce (const csVector3& bounce,
    bool forceUpdate = false);
  virtual csVector3 GetBounce () const {return bounce;}

  virtual void SetDesiredVelocity (const csVector3& velo,
    bool forceUpdate = false);
  virtual csVector3 GetDesiredVelocity () const {return desiredVelocity;}

  virtual void SetMaxForce (const csVector3& force,
    bool forceUpdate = false);
  virtual csVector3 GetMaxForce () const;

  virtual bool RebuildJoint ();

  virtual void SetSpring(bool isSpring, bool forceUpdate = false);

  virtual void SetLinearStiffness (csVector3 stiff, bool forceUpdate = false);
  virtual csVector3 GetLinearStiffness () const {return linearStiff;}
  virtual void SetAngularStiffness (csVector3 stiff, bool forceUpdate = false);
  virtual csVector3 GetAngularStiffness () const {return angularStiff;}

  virtual void SetLinearDamping (csVector3 damp, bool forceUpdate = false);
  virtual csVector3 GetLinearDamping () const {return linearDamp;}
  virtual void SetAngularDamping (csVector3 damp, bool forceUpdate = false);
  virtual csVector3 GetAngularDamping () const {return angularDamp;}

  virtual void SetLinearEquilibriumPoint (csVector3 point, bool forceUpdate = false);
  virtual void SetAngularEquilibriumPoint (csVector3 point, bool forceUpdate = false);

  virtual void SetBreakingImpulseThreshold (float threshold, bool forceUpdate = false);
  virtual float GetBreakingImpulseThreshold () {return threshold;}

  void AddBulletJoint ();
  void RemoveBulletJoint ();
};
}
CS_PLUGIN_NAMESPACE_END (Bullet2)
#endif