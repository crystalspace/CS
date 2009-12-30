/*
  Copyright (C) 2009 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

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
#ifndef __CS_CONTROLLER_LOOKAT_H__
#define __CS_CONTROLLER_LOOKAT_H__

/**\file
 * LookAt animation node for an animated mesh.
 */

#include "csutil/scf_interface.h"

#include "imesh/skeleton2anim.h"

/**\addtogroup meshplugins
 * @{ */

struct iLookAtAnimNodeFactory;
struct iBodySkeleton;
struct iAnimatedMesh;
struct iMovable;
struct iCamera;
struct iLookAtListener;

/**
 * A class to manage the creation and deletion of 'LookAt' animation 
 * node factories.
 */
struct iLookAtManager : public virtual iBase
{
  SCF_INTERFACE(iLookAtManager, 1, 0, 0);

  /**
   * Create a new 'LookAt' animation node factory.
   * \param name The name of the new factory.
   * \param skeleton A iBodySkeleton specifying the geometrical constraints of the bone. A 
   * iBodyBone and a iBodyBoneJoint must be defined for the bone animated by the 'LookAt' 
   * controller. If skeleton is 0 or if there is no iBodyBoneJoint defined, then the
   * animation won't have any geometrical constraints.
   */
  virtual iLookAtAnimNodeFactory* CreateAnimNodeFactory (const char *name,
							 iBodySkeleton* skeleton) = 0;

  /**
   * Find the specified 'LookAt' animation node factory.
   */
  virtual iLookAtAnimNodeFactory* FindAnimNodeFactory (const char* name) const = 0;

  /**
   * Delete all 'LookAt' animation node factories.
   */
  virtual void ClearAnimNodeFactories () = 0;
};

/**
 * Factory for the 'LookAt' animation node.
 */
struct iLookAtAnimNodeFactory : public iSkeletonAnimNodeFactory2
{
  SCF_INTERFACE(iLookAtAnimNodeFactory, 1, 0, 0);

  /**
   * Set the child animation node of this node. The 'Lookat' controller will
   * add its control on top of the animation of the child node. The child node
   * may animate the bone controlled by this controller, and this animation may be
   * played if the 'LookAt' target is not reachable.
   */
  virtual void SetChildNode (iSkeletonAnimNodeFactory2* node) = 0;

  /**
   * Return the child animation node of this node.
   */
  virtual iSkeletonAnimNodeFactory2* GetChildNode () = 0;

  /**
   * Clear the child animation node of this node.
   */
  virtual void ClearChildNode () = 0;
};

/**
 * An animation node that controls a bone of an animesh in order to make it look
 * at a target.
 * There are three types of constraints that will modify the 'LookAt' control:
 * - Whether or not a iBodyBoneJoint for the bone controlled has been defined while
 * creating the 'LookAt' factory (see LookAtManager::CreateAnimNodeFactory ()).
 * - Whether or not SetAlwaysRotate () has been set.
 * - The maximum rotation speed allowed through SetMaximumSpeed ().
 * This controller uses only the pitch and yaw (ie rotations around X and Y axis)
 * in order to achieve the look at the target, the roll is not used.
 */
struct iLookAtAnimNode : public iSkeletonAnimNode2
{
  SCF_INTERFACE(iLookAtAnimNode, 1, 0, 0);

  // TODO: remove this function and implement iSkeleton2::GetSceneNode ()
  /**
   * Set the animated mesh associated with this animation node.
   */
  virtual void SetAnimatedMesh (iAnimatedMesh* mesh) = 0;

  /**
   * Set the bone controlled by this controller.
   */
  virtual void SetBone (BoneID boneID) = 0;

  /**
   * Set the target of the controller as a fixed position (in world coordinates). Don't
   * be afraid to update often this position if you want it moving.
   * Listeners will be called with the 'target lost' event if a target was specified and
   * was reached.
   */
  virtual void SetTarget (csVector3 target) = 0;

  /**
   * Set the target of the controller as a iMovable.
   * Listeners will be called with the 'target lost' event if a target was specified and
   * was reached.
   * \param target The iMovable target.
   * \param offset An offset on the target (in iMovable coordinates).
   */
  virtual void SetTarget (iMovable* target, const csVector3& offset) = 0;

  // TODO: remove this method once iCamera has iMovable implemented
  /**
   * Set the target of the controller as a iCamera.
   * Listeners will be called with the 'target lost' event if a target was specified and
   * was reached.
   * \param target The iCamera target.
   * \param offset An offset on the target (in iCamera coordinates).
   */
  virtual void SetTarget (iCamera* target, const csVector3& offset) = 0;

  /**
   * Remove the current target, ie the controller will not act anymore.
   * Listeners will be called with the 'target lost' event if a target was specified and
   * was reached.
   */
  virtual void RemoveTarget () = 0;

  /**
   * Set the maximum rotation speed while trying to look at the target (in radian per second).
   * A speed of 0.0 means that the movement is immediate.
   * \param speed The maximum rotation speed, in radian per second. Default value is 'PI'
   * (ie 3.1415).
   */
  virtual void SetMaximumSpeed (float speed) = 0;

  /**
   * Set if whether or not the controller must keep the rotation even if the target is 
   * not reachable. Default value is 'false'.
   */
  virtual void SetAlwaysRotate (bool alwaysRotate) = 0;

  /**
   * Set how many time delay must be waited before the listeners are called with the
   * 'target lost' event. This can be useful if the maximum rotation speed of the bone is
   * slow regarding the target's movement. In this case it may prevent swapping continuously
   * between 'target reached' and 'target lost' events.
   * \param delay The time delay, in second. Default value is '0.1f'.
   */
  virtual void SetListenerDelay (float delay) = 0;

  /**
   * Add a listener to be notified when the target has been reached or lost.
   */
  virtual void AddListener (iLookAtListener* listener) = 0;

  /**
   * Remove the specified listener.
   */
  virtual void RemoveListener (iLookAtListener* listener) = 0;
};

/**
 * A listener to be implemented if you want to be notified when the target has been
 * reached or lost.
 */
struct iLookAtListener : public virtual iBase
{
  SCF_INTERFACE (iLookAtListener, 1, 0, 0);

  /**
   * The target is now looked at.
   */
  virtual void TargetReached () = 0;

  /**
   * The target is no more looked at, ie it was unreachable given the various constraints.
   */
  virtual void TargetLost () = 0;
};

/** @} */

#endif //__CS_CONTROLLER_LOOKAT_H__
