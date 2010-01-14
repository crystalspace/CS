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
 * \sa csSectorHitBeamResult
 */
struct csBulletHitBeamResult
{
  /**
   * The resulting rigid or kinematic body that was hit, or 0 if no body was hit.
   */
  iRigidBody* body;

  /**
   * Intersection point in world space.
   */
  csVector3 isect;
};

/**
 * The Bullet implementation of iDynamicSystem also implements this
 * interface.
 * \sa iDynamicSystem iODEDynamicSystemState
 */
struct iBulletDynamicSystem : public virtual iBase
{
  SCF_INTERFACE(iBulletDynamicSystem, 2, 0, 1);

  /**
   * Draw debug information for all colliders managed by bullet.
   */
  virtual void DebugDraw (iView* rview) = 0;

  /**
   * Follow a beam from start to end and return the first dynamic or kinematic rigid body
   * that is hit. Static objects doesn't count.
   * \sa csBulletHitBeamResult iSector::HitBeam() iSector::HitBeamPortals()
   */
  virtual csBulletHitBeamResult HitBeam (const csVector3 &start, const csVector3 &end) = 0;
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

