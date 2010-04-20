/*
    Copyright (C) 2007 by Jorrit Tyberghein

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

#ifndef __CS_IVARIA_BULLET_H__
#define __CS_IVARIA_BULLET_H__

/**\file
 * Bullet-specific interfaces
 */

#include "csutil/scf_interface.h"

struct iView;
struct iRigidBody;
struct iBulletKinematicCallback;

/**
 * Return structure for the iBulletDynamicSystem::HitBeam() routine.
 * \sa csHitBeamResult csSectorHitBeamResult
 */
struct csBulletHitBeamResult
{
  csBulletHitBeamResult () : body (0), isect (0.0f) {}

  /**
   * The resulting dynamic or kinematic body that was hit, or 0 if no body was hit.
   */
  iRigidBody* body;

  /**
   * Intersection point in world space.
   */
  csVector3 isect;
};

/**
 * The debug modes to be used with iBulletDynamicSystem::DebugDraw().
 */
enum csBulletDebugMode
{
  BULLET_DEBUG_NOTHING = 0,     /*!< Nothing will be displayed. */
  BULLET_DEBUG_COLLIDERS = 1,   /*!< Display the colliders of the bodies. */
  BULLET_DEBUG_AABB = 2,        /*!< Display the axis aligned bounding boxes of the bodies. */
  BULLET_DEBUG_JOINTS = 4       /*!< Display the joint positions and limits. */
};

/**
 * The Bullet implementation of iDynamicSystem also implements this
 * interface.
 * \sa iDynamicSystem iODEDynamicSystemState
 */
struct iBulletDynamicSystem : public virtual iBase
{
  SCF_INTERFACE(iBulletDynamicSystem, 2, 0, 2);

  /**
   * Draw the debug informations of the dynamic system. This has to be called
   * at each frame, and will add 2D lines on top of the rendered scene. The
   * objects to be displayed are defined by SetDebugMode().
   */
  virtual void DebugDraw (iView* rview) = 0;

  /**
   * Follow a beam from start to end and return the first dynamic or kinematic rigid body
   * that is hit. Static objects doesn't count.
   * \sa csBulletHitBeamResult iMeshWrapper::HitBeam() iSector::HitBeam()
   * iSector::HitBeamPortals()
   */
  virtual csBulletHitBeamResult HitBeam (const csVector3 &start, const csVector3 &end) = 0;


  /**
   * Set the internal scale to be applied to the whole dynamic world. Use this
   * to put back the range of dimensions you use for your objects to the one
   * Bullet was designed for.
   *
   * Bullet does not work well if the dimensions of your objects are smaller
   * than 0.1 to 1.0 units or bigger than 10 to 100 units. Use this method to
   * fix the problem.
   *
   * \warning You have to call this method before adding any objects in the
   * dynamic world, otherwise the objects won't have the same scale.
   */
  virtual void SetInternalScale (float scale) = 0;

  /**
   * Set the parameters of the constraint solver. Use this if you want to find a
   * compromise between accuracy of the simulation and performance cost.
   * \param timeStep The internal, constant, time step of the simulation, in seconds.
   * A smaller value gives better accuracy. Default value is 1/60 s (ie 0.0166 s).
   * \param maxSteps Maximum number of steps that Bullet is allowed to take each
   * time you call iDynamicSystem::Step(). If you pass a very small time step as
   * the first parameter, then you must increase the number of maxSteps to
   * compensate for this, otherwise your simulation is 'losing' time. Default value
   * is 1. If you pass maxSteps=0 to the function, then it will assume a variable
   * tick rate. Don't do it.
   * \param iterations Number of iterations of the constraint solver. A reasonable
   * range of iterations is from 4 (low quality, good performance) to 20 (good
   * quality, less but still reasonable performance). Default value is 10. 
   */
  virtual void SetStepParameters (float timeStep, size_t maxSteps,
				  size_t iterations) = 0;

  /**
   * Set the mode to be used when displaying debug informations. The default value
   * is 'BULLET_DEBUG_COLLIDERS | BULLET_DEBUG_JOINTS'.
   * \remark Don't forget to call DebugDraw() at each frame to effectively display
   * the debug informations.
   */
  virtual void SetDebugMode (csBulletDebugMode mode) = 0;

  /**
   * Return the current mode used when displaying debug informations.
   */
  virtual csBulletDebugMode GetDebugMode () = 0;
};

/**
 * The physical state of a rigid body.
 */
enum csBulletState
{
  BULLET_STATE_STATIC = 0,     /*!< The body is static, ie this body won't move
				 anymore but dynamic objects will still collide with it. */
  BULLET_STATE_DYNAMIC,        /*!< The body is dynamic, ie the motion of 
				  the body is controlled by the dynamic simulation. */
  BULLET_STATE_KINEMATIC       /*!< The body is kinematic, ie the motion 
				  of the body is controlled by the animation system,
				  but it interacts with the dynamic simulation. */
};

/**
 * The Bullet implementation of iRigidBody also implements this
 * interface.
 * \sa iRigidBody
 */
struct iBulletRigidBody : public virtual iBase
{
  SCF_INTERFACE(iBulletRigidBody, 1, 0, 0);

  /**
   * Set a body in the kinematic state, ie the motion of the body is
   * controlled by you, but it interacts with the dynamic simulation.
   * 
   * You may need to set a callback with SetKinematicCallback() to let
   * the dynamic system know how to update the transform of the body.
   * \sa SetDynamicState() iRigidBody::MakeStatic() iRigidBody::MakeDynamic()
   */
  virtual void MakeKinematic () = 0;

  /**
   * Return the current state of the body.
   */
  virtual csBulletState GetDynamicState () const = 0;

  /**
   * Set the current state of the body.
   * \sa iRigidBody::MakeStatic() iRigidBody::MakeDynamic() MakeKinematic()
   */
  virtual void SetDynamicState (csBulletState state) = 0;

  /**
   * Set the callback to be used to update the transform of the kinematic body.
   * If no callback are provided then the dynamic system will use a default one.
   */
  virtual void SetKinematicCallback (iBulletKinematicCallback* callback) = 0;

  /**
   * Get the callback used to update the transform of the kinematic body.
   */
  virtual iBulletKinematicCallback* GetKinematicCallback () = 0;
};

/**
 * A callback to be implemented when you are using kinematic bodies. If no
 * callback are provided then the dynamic system will use a default one which
 * will update the transform of the body from the position of the attached
 * mesh, body or camera (see iRigidBody::AttachMesh(),
 * iRigidBody::AttachLight(), iRigidBody::AttachCamera()).
 * \sa iBulletRigidBody::SetKinematicCallback()
 */
struct iBulletKinematicCallback : public virtual iBase
{
  SCF_INTERFACE (iBulletKinematicCallback, 1, 0, 0);

  /**
   * Update the new transform of the rigid body.
   */
  virtual void GetBodyTransform (iRigidBody* body,
				 csOrthoTransform& transform) const = 0;
};

#endif // __CS_IVARIA_BULLET_H__

