/*
    Copyright (C) 2002 Anders Stenberg

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef __GAME_DYNAMICS_H__
#define __GAME_DYNAMICS_H__

#include "csutil/scf.h"

class csVector3;
class csOrthoTransform;
class csMatrix3;
struct iMeshWrapper;
struct iSkeletonBone;

struct iDynamicSystem;
struct iRigidBody;
struct iJoint;
struct iPolygonMesh;

SCF_VERSION (iDynamics, 0, 0, 1);

/**
 * This is the interface for the actual plugin.
 * It is responsible for creating iDynamicSystem.
 */
struct iDynamics : public iBase
{
  /// Create a rigid body and add it to the simulation
  virtual iDynamicSystem* CreateSystem () = 0;

  /// Create a rigid body and add it to the simulation
  virtual void RemoveSystem (iDynamicSystem* system) = 0;

  /// Step the simulation forward by stepsize.
  virtual void Step (float stepsize) = 0;
};

SCF_VERSION (iDynamicSystem, 0, 0, 1);

/**
 * This is the interface for the dynamics core.
 * It handles all bookkeeping for rigid bodies and joints.
 * It also handles collision response.
 * Collision detection is done in another plugin.
 */
struct iDynamicSystem : public iBase
{
  /// Set the global gravity.
  virtual void SetGravity (const csVector3& v) = 0;
  /// Get the global gravity.
  virtual const csVector3 GetGravity () const = 0;

  /// Step the simulation forward by stepsize.
  virtual void Step (float stepsize) = 0;

  /// Create a rigid body and add it to the simulation
  virtual iRigidBody* CreateBody () = 0;

  /// Create a rigid body and add it to the simulation
  virtual void RemoveBody( iRigidBody* body ) = 0;
};


SCF_VERSION (iRigidBody, 0, 0, 1);

/**
 * This is the interface for a rigid body.
 * It keeps all properties for the body.
 * It can also be attached to a movable or a bone,
 * to automatically update it.
 */
struct iRigidBody : public iBase
{
  /// Add a collider with a associated friction coefficient
  virtual bool AttachColliderMesh (iPolygonMesh* mesh,
  	csOrthoTransform& trans, float friction, float density,
	float elasticity) = 0;
  virtual bool AttachColliderBox (csVector3 size,
  	csOrthoTransform& trans, float friction, float density,
	float elasticity) = 0;
  virtual bool AttachColliderSphere (float radius, csVector3 offset,
  	float friction, float density, float elasticity) = 0;

  /// Set the position
  virtual void SetPosition (const csVector3& trans) = 0;
  /// Get the position
  virtual const csVector3 GetPosition () const = 0;
  /// Set the orientation
  virtual void SetOrientation (const csMatrix3& trans) = 0;
  /// Get the orientation
  virtual const csMatrix3 GetOrientation () const = 0;
  /// Set the transform
  virtual void SetTransform (const csOrthoTransform& trans) = 0;
  /// Get the transform
  virtual const csOrthoTransform GetTransform () const = 0;
  /// Set the linear velocity (movement)
  virtual void SetLinearVelocity (const csVector3& vel) = 0;
  /// Get the linear velocity (movement)
  virtual const csVector3 GetLinearVelocity () const = 0;
  /// Set the angular velocity (rotation)
  virtual void SetAngularVelocity (const csVector3& vel) = 0;
  /// Get the angular velocity (rotation)
  virtual const csVector3 GetAngularVelocity () const = 0;

  /// Set the bodies physic properties
  virtual void SetProperties (float mass, const csVector3& center,
  	const csMatrix3& inertia) = 0;
  /// Set total mass to targetmass, and adjust properties
  virtual void AdjustTotalMass (float targetmass) = 0;


  /// Add a force (world space) (active for one timestep)
  virtual void AddForce	(const csVector3& force) = 0;
  /// Add a torque (world space) (active for one timestep)
  virtual void AddTorque (const csVector3& force) = 0;
  /// Add a force (local space) (active for one timestep)
  virtual void AddRelForce (const csVector3& force) = 0;
  /// Add a torque (local space) (active for one timestep)
  virtual void AddRelTorque (const csVector3& force) = 0 ;
  /**
   * Add a force (world space) at a specific position (world space)
   * (active for one timestep)
   */
  virtual void AddForceAtPos (const csVector3& force, const csVector3& pos) = 0;
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
   * Add a force (local space) at a specific position (loacl space)
   * (active for one timestep)
   */
  virtual void AddRelForceAtRelPos (const csVector3& force,
  	const csVector3& pos) = 0;

  /// Get total force (world space)
  virtual const csVector3 GetForce () const = 0;
  /// Get total torque (world space)
  virtual const csVector3 GetTorque () const = 0;
	
  /*
  /// Get total force (local space)
  virtual const csVector3& GetRelForce () const = 0;
  /// Get total force (local space)
  virtual const csVector3& GetRelTorque () const = 0;
  */

  /*
  /// Get the number of joints attached to this body
  virtual int GetJointCount () const = 0;
  */

  /// Attach a iMeshWrapper to this body
  virtual void AttachMesh (iMeshWrapper* mesh) = 0;
  /// Attach a bone to this body
  virtual void AttachBone (iSkeletonBone* bone) = 0;
	
  /// Update transforms for mesh and/or bone
  virtual void Update () = 0;
};

SCF_VERSION (iJoint, 0, 0, 1);

/**
 * This is the interface for a joint.
 * It keeps all properties of a joint.
 * to automatically update it.
 */
struct iJoint : public iBase
{
  /// Set which two bodies to be affected by this joint
  virtual void Attach (iRigidBody* body1, iRigidBody* body2) = 0;
  /// Get an attached body (valid values for body are 0 and 1)
  virtual iRigidBody* GetAttachedBody (int body) = 0;
};

#endif // __GAME_DYNAMICS_H__

