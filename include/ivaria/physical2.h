#ifndef __IVARIA_PHYSIH__
#define __IVARIA_PHYSIH__

namespace CS
{
namespace Collision
{
struct iCollisionCallback;
struct iCollisionObject;
struct CollisionGroup;
struct iCollisionObject;
}
}

namespace CS
{
namespace Physics
{

struct iJoint;
struct iObject;
struct iRigidBody;
struct iSoftBody;
struct iKinematicCallback;
struct iPhysicalSystem;

enum PhysicalBodyType
{
  BODY_RIGID = 0,
  BODY_SOFT
};

enum RigidBodyState
{
  STATE_STATIC = 0,
  STATE_DYNAMIC,
  STATE_KINEMATIC
};
/*
enum JointType
{
  JOINT_P2P;
  JOINT_CONETWIST;
  JOINT_6DOF;
  JOINT_SPRING;
};

struct iJointHelper : public virtual iBase
{
  SCF_INTERFACE (CS::Physics::iJointHelper ,1, 0, 0);

  virtual csPtr<iJoint> CreateP2PJoint();

  virtual csPtr<iJoint> CreateConeTwistJoint();

  virtual csPtr<iJoint> Create6DOFJoint();

  virtual csPtr<iJoint> CreateSpringJoint();
  };
  */

/**
* A base interface of physical bodies. 
* iRigidBody and iSoftBody will be derived from this one.
*/
struct iPhysicalBody : public virtual iCollisionObject
{
  SCF_INTERFACE (CS::Physics::iPhysicalBody, 1, 0, 0);

  virtual PhysicalBodyType GetBodyType () const = 0;

  virtual iRigidBody* QueryRigidBody () = 0;

  virtual iSoftBody* QuerySoftBody () = 0;

  /// Disable this collision object.
  virtual bool Disable () = 0;

  /// Enable this collision object.
  virtual bool Enable () = 0;

  /// Check if the collision object is enabled.
  virtual bool IsEnabled () = 0;

  // move "virtual iRigidBody::RigidBodyState GetState() = 0;" here and implement it for soft bodies by switching to rigid bodies when static/dynamic?
  //Lulu: If soft body can switch to rigid body, what about the parameters? Dose user have to call functions to set the parameters of rigid body? 
  //      If iPhysicalSystem create a new rigidbody, it's based on the softbody's current shape or original shape? 
  //      And the collision system will create a collision shape for the mesh? How to decide which type of collider is appropriate?

  /// Get the mass of this body.
  virtual float GetMass () const = 0;

  /// Set the mass of this body.
  virtual void SetMass (float mass) = 0;

  virtual float GetDensity () const = 0;

  virtual void SetDensity (float density) = 0;

  /// Return the volume of this body.
  virtual float GetVolume () = 0;
  
  /// Add a force to the whole body.
  virtual void AddForce (const csVector3& force) = 0;
  
  /// Set the linear velocity (movement).
  virtual void SetLinearVelocity (const csVector3& vel) = 0;
  
  /// Get the linear velocity (movement).
  virtual csVector3 GetLinearVelocity (size_t index = 0) const = 0;

  /**
  * Set the friction of this body.
  * [0,1] for soft body.
  */
  virtual void SetFriction (float friction) = 0;

  /// Get the friction of this rigid body.
  virtual void GetFriction (float& friction) = 0;
};

/**
* This is the interface for a rigid body.
* It keeps all properties for the body.
* It can also be attached to a movable or a bone,
* to automatically update it.
*
* Main creators of instances implementing this interface:
* - iPhysicalSystem::CreateRigidBody()
* 
* Main ways to get pointers to this interface:
* - iPhysicalSystem::GetRigidBody()
* 
* Main users of this interface:
* - iPhysicalSystem
*
* \sa iSoftBody
*/
struct iRigidBody : public iPhysicalBody
{
  SCF_INTERFACE (CS::Physics::iRigidBody, 1, 0, 2);

  /// Get the iCollisionObject pointer of this body.
  virtual iCollisionObject* QueryCollisionObject () = 0;

  /// Get the current state of the body.
  virtual RigidBodyState GetState () = 0;
  
  /// Set the current state of the body.
  virtual void SetState (RigidBodyState state) = 0;


  /// Set the elasticity of this rigid body.
  virtual void SetElasticity (float elasticity) = 0;

  /// Get the elasticity of this rigid body.
  virtual void GetElasticity (float elasticity) = 0;

  /// Set the angular velocity (rotation).
  virtual void SetAngularVelocity (const csVector3& vel) = 0;

  /// Get the angular velocity (rotation)
  virtual csVector3 GetAngularVelocity () const = 0;

  /// Add a torque (world space) (active for one timestep).
  virtual void AddTorque (const csVector3& force) = 0;

  /// Add a force (local space) (active for one timestep).
  virtual void AddRelForce (const csVector3& force) = 0;

  /// Add a torque (local space) (active for one timestep).
  virtual void AddRelTorque (const csVector3& force) = 0;

  /**
  * Add a force (world space) at a specific position (world space)
  * (active for one timestep)
  */
  virtual void AddForceAtPos (const csVector3& force,
      const csVector3& pos) = 0;

  /**
  * Add a force (world space) at a specific position (local space)
  * (active for one timestep)
  */
  virtual void AddForceAtRelPos (const csVector3& force,
      const csVector3& pos) = 0;

  /**
  * Add a force (local space) at a specific position (world space)
  * (active for one timestep)
  */
  virtual void AddRelForceAtPos (const csVector3& force,
      const csVector3& pos) = 0;

  /**
  * Add a force (local space) at a specific position (local space)
  * (active for one timestep)
  */
  virtual void AddRelForceAtRelPos (const csVector3& force,
      const csVector3& pos) = 0;

  /// Get total force (world space).
  virtual csVector3 GetForce () const = 0;

  /// Get total torque (world space).
  virtual csVector3 GetTorque () const = 0;

  /**
  * Set the callback to be used to update the transform of the kinematic body.
  * If no callback are provided then the dynamic system will use a default one.
  */
  virtual void SetKinematicCallback (iKinematicCallback* cb) = 0;

  /// Get the callback used to update the transform of the kinematic body.
  virtual iKinematicCallback* GetKinematicCallback () = 0;

  /**
  * Set the linear dampener for this rigid body. The dampening correspond to
  * how much the movements of the objects will be reduced. It is a value
  * between 0 and 1, giving the ratio of speed that will be reduced
  * in one second. 0 means that the movement will not be reduced, while
  * 1 means that the object will not move.
  * The default value is 0.
  * \sa iDynamicSystem::SetLinearDampener()
  */
  virtual void SetLinearDampener (float d) = 0;

  /// Get the linear dampener for this rigid body.
  virtual float GetLinearDampener () = 0;

  /**
  * Set the angular dampener for this rigid body. The dampening correspond to
  * how much the movements of the objects will be reduced. It is a value
  * between 0 and 1, giving the ratio of speed that will be reduced
  * in one second. 0 means that the movement will not be reduced, while
  * 1 means that the object will not move.
  * The default value is 0.
  * \sa iDynamicSystem::SetRollingDampener()
  */
  virtual void SetRollingDampener (float d) = 0;

  /// Get the angular dampener for this rigid body.
  virtual float GetRollingDampener () = 0;
};

/**
* A soft body is a physical body that can be deformed by the physical
* simulation. It can be used to simulate eg ropes, clothes or any soft
* volumetric object.
*
* A soft body does not have a positional transform by itself, but the
* position of every vertex of the body can be queried through GetVertexPosition().
*
* A soft body can neither be static or kinematic, it is always dynamic.
* \sa iRigidBody 
*/
struct iSoftBody : public iPhysicalBody
{
  SCF_INTERFACE (CS::Physics::iSoftBody, 2, 0, 3);

  /// Set the mass of a node by index.
  virtual void SetVertexMass (float mass, size_t index) = 0;

  /// Get the mass of a node by index.
  virtual float GetVertexMass (size_t index) = 0;

  /// Return the count of vertices of this soft body.
  virtual size_t GetVertexCount () = 0;

  /// Return the position in world coordinates of the given vertex.
  virtual csVector3 GetVertexPosition (size_t index) const = 0;

  /// Anchor the given vertex to its current position. This vertex will no more move.
  virtual void AnchorVertex (size_t vertexIndex) = 0;

  /**
  * Anchor the given vertex to the given rigid body. The relative position of the
  * vertex and the body will remain constant.
  */
  virtual void AnchorVertex (size_t vertexIndex,
      iRigidBody* body) = 0;

  /**
  * Anchor the given vertex to the given controller. The relative position of the
  * vertex and the controller will remain constant.
  */
  virtual void AnchorVertex (size_t vertexIndex,
      iAnchorAnimationControl* controller) = 0;

  /**
  * Update the position of the anchor of the given vertex relatively to the anchored
  * rigid body. This can be used to have a finer control of the anchor position
  * relatively to the rigid body.
  *
  * This would work only if you called AnchorVertex(size_t,iRigidBody*) before.
  * The position to be provided is in world coordinates.
  *
  * \warning The stability of the simulation can be lost if you move the position too far
  * from the previous position.
  * \sa CS::Animation::iSoftBodyAnimationControl::CreateAnimatedMeshAnchor()
  */
  virtual void UpdateAnchor (size_t vertexIndex,
      csVector3& position) = 0;

  /**
  * Remove the given anchor. This won't work if you anchored the vertex to a rigid body, due
  * to a limitation in the Bullet library.
  */
  virtual void RemoveAnchor (size_t vertexIndex) = 0;

  /**
  * Set the rigidity of this body. The value should be in the 0 to 1 range, with
  * 0 meaning soft and 1 meaning rigid.
  */
  virtual void SetRigidity (float rigidity) = 0;

  /// Get the rigidity of this body.
  virtual float GetRidigity () = 0;

  /// Set the linear velocity of the given vertex of the body.
  virtual void SetLinearVelocity (const csVector3& velocity,
      size_t vertexIndex) = 0;

  /**
  * Set the wind velocity of the whole body.
  */
  virtual void setWindVelocity (const csVector3& velocity) = 0;

  /// Get the wind velocity of the whole body.
virtual const csVector3 getWindVelocity () const = 0;

  /// Add a force at the given vertex of the body.
  virtual void AddForce (const csVector3& force, size_t vertexIndex) = 0;

  /// Return the count of triangles of this soft body.
  virtual size_t GetTriangleCount () = 0;

  /// Return the triangle with the given index.
  virtual csTriangle GetTriangle (size_t index) const = 0;

  /// Return the normal vector in world coordinates for the given vertex.
  virtual csVector3 GetVertexNormal (size_t index) const = 0;

  /**
  * Currently Blender set this to 0 when creating a soft body.
  * Used to create a btTriangleMesh for soft body.
  */
  //virtual void SetWelding(float welding) = 0;
};

/**
* A joint that can constrain the relative motion between two iRigidBody.
* For instance if all motion in along the local X axis is constrained
* then the bodies will stay motionless relative to each other
* along an x axis rotated and positioned by the joint's transform.
*
* Main creators of instances implementing this interface:
* - iPhysicalSystem::CreateJoint()
* 
* Main users of this interface:
* - iPhysicalSystem
*/
struct iJoint : public virtual iBase
{
  SCF_INTERFACE (CS::Physics::iJoint, 1, 0, 0);

  /**
  * Set the rigid bodies that will be affected by this joint. Set force_update to true if 
  * you want to apply the changes right away.
  */
  virtual void Attach (iPhysicalBody* body1, iPhysicalBody* body2,
      const csOrthoTransform& trans1,
      const csOrthoTransform& trans2,
      bool forceUpdate = true) = 0;

  /// Get the attached body with the given index (valid values for body are 0 and 1).
  virtual iPhysicalBody* GetAttachedBody (int index) = 0;

  /**
  * Set the local transformation of the joint.
  *
  * Set force_update to true if you want to apply the changes right away.
  */
  virtual void SetTransform (const csOrthoTransform& trans,
      bool forceUpdate = true) = 0;

  /// Get the local transformation of the joint.
  virtual csOrthoTransform GetTransform () const = 0;

  /// Set the new position of the joint, in world coordinates.
  virtual void SetPosition (const csVector3& position) = 0;

  /// Get the current position of the joint, in world coordinates.
  virtual csVector3 GetPosition () const = 0;

  /**
  * Set the translation constraints on the 3 axes. If true is
  * passed for an axis then the Joint will constrain all motion along
  * that axis (ie no motion will be allowed). If false is passed in then all motion along that
  * axis is free, but bounded by the minimum and maximum distance
  * if set. Set force_update to true if you want to apply the changes 
  * right away.
  */
  virtual void SetTransConstraints (bool X, 
      bool Y, bool Z, 
      bool forceUpdate = true) = 0;

  /// True if this axis' translation is constrained.
  virtual bool IsXTransConstrained () = 0;

  /// True if this axis' translation is constrained.
  virtual bool IsYTransConstrained () = 0;

  /// True if this axis' translation is constrained.
  virtual bool IsZTransConstrained () = 0;

  /**
  * Set the minimum allowed distance between the two bodies. Set force_update to true if 
  * you want to apply the changes right away.
  */
  virtual void SetMinimumDistance (const csVector3& dist,
      bool forceUpdate = true) = 0;

  /// Get the minimum allowed distance between the two bodies.
  virtual csVector3 GetMinimumDistance () const = 0;

  /**
  * Set the maximum allowed distance between the two bodies. Set force_update to true if 
  * you want to apply the changes right away.
  */
  virtual void SetMaximumDistance (const csVector3& dist,
      bool forceUpdate = true) = 0;

  /// Get the maximum allowed distance between the two bodies.
  virtual csVector3 GetMaximumDistance () const = 0;

  /**
  * Set the rotational constraints on the 3 axes. If true is
  * passed for an axis then the Joint will constrain all rotation around
  * that axis (ie no motion will be allowed). If false is passed in then all rotation around that
  * axis is free, but bounded by the minimum and maximum angle
  * if set. Set force_update to true if you want to apply the changes 
  * right away.
  */
  virtual void SetRotConstraints (bool X, 
      bool Y, bool Z, 
      bool forceUpdate = true) = 0;

  /// True if this axis' rotation is constrained.
  virtual bool IsXRotConstrained () = 0;

  /// True if this axis' rotation is constrained.
  virtual bool IsYRotConstrained () = 0;

  /// True if this axis' rotation is constrained.
  virtual bool IsZRotConstrained () = 0;

  /**
  * Set the minimum allowed angle between the two bodies, in radian. Set force_update to true if 
  * you want to apply the changes right away.
  */
  virtual void SetMinimumAngle (const csVector3& angle,
      bool forceUpdate = true) = 0;

  /// Get the minimum allowed angle between the two bodies (in radian).
  virtual csVector3 GetMinimumAngle () const = 0;

  /**
  * Set the maximum allowed angle between the two bodies (in radian). Set force_update to true if 
  * you want to apply the changes right away.
  */
  virtual void SetMaximumAngle (const csVector3& dist,
      bool forceUpdate = true) = 0;

  /// Get the maximum allowed angle between the two bodies (in radian).
  virtual csVector3 GetMaximumAngle () const = 0;

  /** 
  * Set the restitution of the joint's stop point (this is the 
  * elasticity of the joint when say throwing open a door how 
  * much it will bounce the door back closed when it hits).
  */
  virtual void SetBounce (const csVector3& bounce,
      bool forceUpdate = true) = 0;

  /// Get the joint restitution.
  virtual csVector3 GetBounce () const = 0;

  /**
  * Apply a motor velocity to joint (for instance on wheels). Set force_update to true if 
  * you want to apply the changes right away.
  */
  virtual void SetDesiredVelocity (const csVector3& velo,
      bool forceUpdate = true) = 0;

  /// Get the desired velocity of the joint motor.
  virtual csVector3 GetDesiredVelocity () const = 0;

  /**
  * Set the maximum force that can be applied by the joint motor to reach the desired velocity.
  * Set force_update to true if  you want to apply the changes right away.
  */
  virtual void SetMaxForce (const csVector3& force,
      bool forceUpdate = true) = 0;

  /// Get the maximum force that can be applied by the joint motor to reach the desired velocity.
  virtual csVector3 GetMaxForce () const = 0;

  /**
  * Set a custom angular constraint axis (have sense only with rotation free minimum along 2 axis).
  * Set force_update to true if you want to apply the changes right away.
  */
  virtual void SetAngularConstraintAxis (const csVector3& axis,
      bool forceUpdate = true) = 0;

  /// Get the custom angular constraint axis.
  virtual csVector3 GetAngularConstraintAxis () const = 0;

  /**
  * Rebuild the joint using the current setup. Return true if the rebuilding operation was successful
  * (otherwise the joint won't be active).
  */
  virtual bool RebuildJoint () = 0;

  /**
  * Set the spring constraints on the 3 axes. If true is
  * passed for an axis then the Joint will have a spring constraint on
  * that axis. If false is passed in then no spring constraint on the
  * axis. Set force_update to true if you want to apply the changes 
  * right away.
  */
  virtual void SetSpringConstraints (bool X, 
      bool Y, bool Z, 
      bool forceUpdate = true) = 0;

  /// True if this axis has a spring constraint.
  virtual bool IsXSpringConstrained () = 0;

  /// True if this axis has a spring constraint.
  virtual bool IsYSpringConstrained () = 0;

  /// True if this axis has a spring constraint.
  virtual bool IsZSpringConstrained () = 0;

  /// Set the stiffness of the spring.
  virtual void SetStiffness (float stiff) = 0;

  /// Set the damping of the spring.
  virtual void SetDamping (float damp) = 0;

  /**
  * Set the current constraint position/orientation as an equilibrium point.
  * If index = -1, then set equilibrium point for all DOF, else set it for given DOF.
  */
  virtual void SetEquilibriumPoint (int index = -1);
  
  /// Set the value to an equilibrium point for given DOF.
  virtual void SetEquilibriumPoint (int index, float value);

  virtual void SetBreakingImpulseThreshold (float threshold) = 0;

  virtual float GetBreakingImpulseThreshold () = 0;
};

struct iPhysicalSystem : public virtual iBase
{
  SCF_INTERFACE (CS::Physics::iPhysicalSystem, 1, 0, 0);

  /**
  * Create a rigid body, if there's an iCollisionObject pointer, 
  * Need to call iCollisionObject::RebuildObject.
  */
  virtual csRef<iRigidBody> CreateRigidBody () = 0;

  /// Create a joint and add it to the simulation.
  virtual csRef<iJoint> CreateJoint () = 0;
  
  /**
  * Create a soft body rope.
  * \param start Start position of the rope.
  * \param end End position of the rope.
  * \param segmentCount Number of segments in the rope.
  * \remark You must call SetSoftBodyWorld() prior to this.
  */
  virtual csRef<iSoftBody> CreateRope (csVector3 start,
      csVector3 end, size_t segmentCount) = 0;

  /**
  * Create a soft body rope with explicit positions of the vertices.
  * \param vertices The array of positions to use for the vertices.
  * \param vertexCount The amount of vertices for the rope.
  * \remark You must call SetSoftBodyWorld() prior to this.
  */
  virtual csRef<iSoftBody> CreateRope (csVector3* vertices, size_t vertexCount) = 0;

  /**
  * Create a soft body cloth.
  * \param corner1 The position of the top left corner.
  * \param corner2 The position of the top right corner.
  * \param corner3 The position of the bottom left corner.
  * \param corner4 The position of the bottom right corner.
  * \param segmentCount1 Number of horizontal segments in the cloth.
  * \param segmentCount2 Number of vertical segments in the cloth.
  * \param withDiagonals Whether there must be diagonal segments in the cloth
  * or not. Diagonal segments will make the cloth more rigid.
  * \remark You must call SetSoftBodyWorld() prior to this.
  */
  virtual csRef<iSoftBody> CreateCloth (csVector3 corner1, csVector3 corner2,
      csVector3 corner3, csVector3 corner4,
      size_t segmentCount1, size_t segmentCount2,
      bool withDiagonals = false) = 0;

  /**
  * Create a volumetric soft body from a genmesh.
  * \param genmeshFactory The genmesh factory to use.
  * \param if there's an iCollisionObject pointer, attach the iCollisionObject to it.
  * \remark You must call SetSoftBodyWorld() prior to this.
  */
  virtual csRef<iSoftBody> CreateSoftBody (iGeneralFactoryState* genmeshFactory) = 0;

  /**
  * Create a custom volumetric soft body.
  * \param vertices The vertices of the soft body. The position is absolute.
  * \param vertexCount The count of vertices of the soft body.
  * \param triangles The faces of the soft body.
  * \param triangleCount The count of faces of the soft body.
  \param if there's an iCollisionObject pointer, attach the iCollisionObject to it.
  * \remark You must call SetSoftBodyWorld() prior to this.
  */
  virtual csRef<iSoftBody> CreateSoftBody (csVector3* vertices,
      size_t vertexCount, csTriangle* triangles,
      size_t triangleCount) = 0;
};

struct iPhysicalSector : public virtual iBase
{
  SCF_INTERFACE (CS::Physics::iPhysicalSector, 1, 0, 0);

  /**
  * Set the simulation speed. A value of 0 means that the simulation is not made
  * automatically (but it can still be made manually through Step())
  */
  virtual void SetSimulationSpeed (float speed) = 0;

  virtual void SetStepParameters (float timeStep, size_t maxSteps,
    size_t iterations) = 0;  

  /// Step the simulation forward by the given duration, in second
  virtual void Step (float duration) = 0;

  /**
  * Set the global linear dampener. The dampening correspond to how
  * much the movements of the objects will be reduced. It is a value
  * between 0 and 1, giving the ratio of speed that will be reduced
  * in one second. 0 means that the movement will not be reduced, while
  * 1 means that the object will not move.
  * The default value is 0.
  * \sa CS::Physics::Bullet::iRigidBody::SetLinearDampener()
  */
  virtual void SetLinearDampener (float d) = 0;

  /**
  * Get the global linear dampener setting.
  */
  virtual float GetLinearDampener () const = 0;

  /**
  * Set the global angular dampener. The dampening correspond to how
  * much the movements of the objects will be reduced. It is a value
  * between 0 and 1, giving the ratio of speed that will be reduced
  * in one second. 0 means that the movement will not be reduced, while
  * 1 means that the object will not move.
  * The default value is 0.
  * \sa CS::Physics::Bullet::iRigidBody::SetRollingDampener()
  */
  virtual void SetRollingDampener (float d) = 0;

  /// Get the global rolling dampener setting.
  virtual float GetRollingDampener () const = 0;

  /**
  * Turn on/off AutoDisable functionality.
  * AutoDisable will stop moving objects if they are stable in order
  * to save processing time. By default this is enabled.
  */
  // always enabled?
  //virtual void EnableAutoDisable(bool enable) = 0;
 
  /**
  * Return whether the AutoDisable is on or off.
  */
  //virtual bool AutoDisableEnabled() = 0;
  
  /**
  * Set the parameters for AutoDisable.
  * \param linear Maximum linear movement to disable a body. Default value is 0.8.
  * \param angular Maximum angular movement to disable a body. Default value is 1.0.
  * \param steps Minimum number of steps the body meets linear and angular
  * requirements before it is disabled. Default value is 0.
  * \param time Minimum time the body needs to meet linear and angular
  * movement requirements before it is disabled. Default value is 0.0.
  * \remark With the Bullet plugin, the 'steps' parameter is ignored.
  * \remark With the Bullet plugin, calling this method will not affect bodies already
  * created.
  */
  virtual void SetAutoDisableParams (float linear, float angular,
      float time) = 0;

  /**
  * Add a rigid body into the sector.
  * The rigid body has to be initialized.
  */
  virtual void AddRidigBody (iRigidBody* body) = 0;

  /// Remove a rigid body by pointer.
  virtual void RemoveRigidBody (iRigidBody* body) = 0;

  /// Add a soft body into the sector.
  virtual void AddSoftBody (iSoftBody* body) = 0;

  /// Remove a soft body by pointer.
  virtual void RemoveSoftBody (iSoftBody* body) = 0;
};
}
}
#endif