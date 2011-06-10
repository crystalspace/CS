#ifndef __CS_BULLET_JOINT_H__
#define __CS_BULLET_JOINT_H__

CS_PLUGIN_NAMESPACE_BEGIN (Bullet2)
{

enum csJointType
{
  RIGID_P2P_JOINT,
  RIGID_HINGE_JOINT,
  RIGID_SLIDE_JOINT,
  RIGID_6DOF_JOINT,
  SOFT_LINEAR_JOINT,
  SOFT_ANGULAR_JOINT,
};

class csBulletJoint : scfImplementation1<
  csBulletJoint, CS::Physics::iJoint>
{
private:
  csBulletSystem* sys;
  csBulletSector* sector;
  csJointType type;
  btGeneric6DofConstraint* rigidJoint;
  btSoftBody::Joint* softJoint;
  csRef<iPhysicalBody> bodies[2];
  bool transConstraintX;
  bool transConstraintY;
  bool transConstraintZ;
  btVector3 minDist;
  btVector3 maxDist;

  bool rotConstraintX;
  bool rotConstraintY;
  bool rotConstraintZ;
  csVector3 minAngle;
  csVector3 maxAngle;

  csVector3 bounce;
  csVector3 desiredVelocity;
  btVector3 maxforce;

  float threshold;
  csVector3 linearStiff;
  csVector3 angularStiff;
  csVector3 linearDamp;
  csVector3 angularDamp;
  csVector3 linearEquilPoint;
  csVector3 angularEquilPoint;

  csOrthoTransform transform;
  csVector3 position;

  bool isSoft;
  bool isSpring;
  bool equilPointSet;

public:
  csBulletJoint (csBulletSystem* system);
  virtual ~csBulletJoint ();

  void Attach (iPhysicalBody* body, bool forceUpdate = true);
  void SetType (csJointType type) {this->type = type;}

  virtual void Attach (iPhysicalBody* body1, iPhysicalBody* body2,
    bool forceUpdate = true);

  virtual iPhysicalBody* GetAttachedBody (int index)
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
  { return BulletToCS (minDist, sys->inverseInternalScale); }
  virtual void SetMaximumDistance (const csVector3& dist,
    bool forceUpdate = false);
  virtual csVector3 GetMaximumDistance () const
  { return BulletToCS (maxDist, sys->inverseInternalScale); }

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

  virtual void SetBreakingImpulseThreshold (float threshold);
  virtual float GetBreakingImpulseThreshold () {return threshold;}
};
}
CS_PLUGIN_NAMESPACE_END (Bullet2)
#endif