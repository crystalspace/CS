#ifndef __CS_BULLET_JOINT_H__
#define __CS_BULLET_JOINT_H__

CS_PLUGIN_NAMESPACE_BEGIN (Bullet2)
{

class csBulletJoint : scfImplementation1<
  csBulletJoint, CS::Physics::iJoint>
{
private:
  csBulletSystem* sys;
  int btJointType;
  btTypedConstraint* rigidConstraint;
  //joint in btSoftBody?
  csRef<iPhysicalBody> bodies[2];
  bool trans_constraint_x;
  bool trans_constraint_y;
  bool trans_constraint_z;
  btVector3 min_dist;
  btVector3 max_dist;

  bool rot_constraint_x;
  bool rot_constraint_y;
  bool rot_constraint_z;
  csVector3 min_angle;
  csVector3 max_angle;

  csVector3 bounce;
  csVector3 desired_velocity;
  btVector3 maxforce;

  csOrthoTransform transform;
  csVector3 angular_constraints_axis[2];

public:
  virtual void Attach (iPhysicalBody* body1, iPhysicalBody* body2,
    const csOrthoTransform& trans1, const csOrthoTransform& trans2,
    bool forceUpdate = true);

  virtual iPhysicalBody* GetAttachedBody (int index);

  virtual void SetTransform (const csOrthoTransform& trans,
    bool forceUpdate = true);

  virtual csOrthoTransform GetTransform () const;

  virtual void SetPosition (const csVector3& position);
  virtual csVector3 GetPosition () const;

  virtual void SetTransConstraints (bool X, 
    bool Y, bool Z, bool forceUpdate = true);
  virtual bool IsXTransConstrained ();
  virtual bool IsYTransConstrained ();
  virtual bool IsZTransConstrained ();

  virtual void SetMinimumDistance (const csVector3& dist,
    bool forceUpdate = true);
  virtual csVector3 GetMinimumDistance () const;
  virtual void SetMaximumDistance (const csVector3& dist,
    bool forceUpdate = true);
  virtual csVector3 GetMaximumDistance () const;

  virtual void SetRotConstraints (bool X, 
    bool Y, bool Z, bool forceUpdate = true);
  virtual bool IsXRotConstrained ();
  virtual bool IsYRotConstrained ();
  virtual bool IsZRotConstrained ();

  virtual void SetMinimumAngle (const csVector3& angle,
    bool forceUpdate = true);
  virtual csVector3 GetMinimumAngle () const;
  virtual void SetMaximumAngle (const csVector3& dist,
    bool forceUpdate = true);
  virtual csVector3 GetMaximumAngle () const;

  virtual void SetBounce (const csVector3& bounce,
    bool forceUpdate = true);
  virtual csVector3 GetBounce () const;

  virtual void SetDesiredVelocity (const csVector3& velo,
    bool forceUpdate = true);
  virtual csVector3 GetDesiredVelocity () const;

  virtual void SetMaxForce (const csVector3& force,
    bool forceUpdate = true);
  virtual csVector3 GetMaxForce () const;

  virtual void SetAngularConstraintAxis (const csVector3& axis,
    bool forceUpdate = true);
  virtual csVector3 GetAngularConstraintAxis () const;

  virtual bool RebuildJoint ();

  virtual void SetSpringConstraints (bool X, 
    bool Y, bool Z, bool forceUpdate = true);
  virtual bool IsXSpringConstrained ();
  virtual bool IsYSpringConstrained ();
  virtual bool IsZSpringConstrained ();

  virtual void SetStiffness (float stiff);
  virtual void SetDamping (float damp);

  virtual void SetEquilibriumPoint (int index = -1);
  virtual void SetEquilibriumPoint (int index, float value);

  virtual void SetBreakingImpulseThreshold (float threshold);
  virtual float GetBreakingImpulseThreshold ();
};
}
CS_PLUGIN_NAMESPACE_END (Bullet2)
#endif