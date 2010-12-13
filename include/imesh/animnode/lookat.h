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
#ifndef __CS_IMESH_ANIMNODE_LOOKAT_H__
#define __CS_IMESH_ANIMNODE_LOOKAT_H__

/**\file
 * LookAt animation node for an animated mesh.
 */

#include "csutil/scf_interface.h"

#include "imesh/animnode/skeleton2anim.h"

/**\addtogroup meshplugins
 * @{ */

struct iMovable;
struct iCamera;

namespace CS
{
namespace Mesh
{

  struct iAnimatedMesh;

} // namespace Mesh
} // namespace CS


namespace CS
{
namespace Animation
{

struct iBodySkeleton;
struct iSkeletonLookAtNodeFactory;
struct iSkeletonLookAtListener;

/**
 * A class to manage the creation and deletion of 'LookAt' animation 
 * node factories.
 */
struct iSkeletonLookAtNodeManager : public virtual iBase
{
  SCF_INTERFACE(CS::Animation::iSkeletonLookAtNodeManager, 1, 0, 0);

  /**
   * Create a new 'LookAt' animation node factory.
   * \param name The name of the new factory.
   */
  virtual iSkeletonLookAtNodeFactory* CreateAnimNodeFactory (const char *name) = 0;

  /**
   * Find the specified 'LookAt' animation node factory.
   */
  virtual iSkeletonLookAtNodeFactory* FindAnimNodeFactory (const char* name) const = 0;

  /**
   * Delete all 'LookAt' animation node factories.
   */
  virtual void ClearAnimNodeFactories () = 0;
};

/**
 * Factory for the 'LookAt' animation node. This node can control a bone of an animesh in
 * order to make it look at a target.
 *
 * There are three types of constraints that will modify the 'LookAt' control:
 * - Whether or not a CS::Animation::iBodyBoneJoint for the bone controlled has been defined
 * (see SetBodySkeleton()).
 * - Whether or not SetAlwaysRotate() has been set.
 * - The maximum rotation speed allowed through SetMaximumSpeed().
 *
 * This animation node can manage by itself the transitions while changing the current 'LookAt'
 * targets, therefore it doesn't need to be blended in and out.
 *
 * This animation node uses only the pitch and yaw (ie rotations around X and Y axis)
 * in order to achieve the look at the target, the roll is not used.
 */
struct iSkeletonLookAtNodeFactory : public iSkeletonAnimNodeFactory
{
  SCF_INTERFACE(CS::Animation::iSkeletonLookAtNodeFactory, 1, 0, 0);

  /**
   * Set the body skeleton that is used to specify the geometrical constraints of the animated
   * bone. A CS::Animation::iBodyBone and a CS::Animation::iBodyBoneJoint must be defined for
   * the bone animated by the 'LookAt' node. If \a skeleton is nullptr or if there is no
   * CS::Animation::iBodyBoneJoint defined, then the animation won't have any geometrical
   * constraints.
   */
  virtual void SetBodySkeleton (iBodySkeleton* skeleton) = 0;

  /**
   * Set the bone animated by this node
   */
  virtual void SetBone (BoneID boneID) = 0;

  /**
   * Set the maximum rotation speed while trying to look at the target (in radian per second).
   * A speed of 0.0 means that the movement is immediate.
   * \param speed The maximum rotation speed, in radian per second. The default value is 'PI'
   * (ie 3.1415).
   */
  virtual void SetMaximumSpeed (float speed) = 0;

  /**
   * Set whether or not the LookAt must still be kept even if the target is 
   * not reachable. The default value is 'false'.
   */
  virtual void SetAlwaysRotate (bool alwaysRotate) = 0;

  /**
   * Set how many time delay must be waited before the listeners are called with the
   * 'target lost' event. This can be useful if the maximum rotation speed of the bone is
   * slow regarding the target's movement. In this case it may prevent swapping continuously
   * between 'target reached' and 'target lost' events.
   * \param delay The time delay, in second. The default value is '0.1f'.
   */
  virtual void SetListenerDelay (float delay) = 0;

  /**
   * Set the child animation node of this node. The 'Lookat' animation node will
   * add its control on top of the animation of the child node. The animation of
   * the bone made by the child node may still be used, for example when the node
   * is transitioning between targets, or when the target is not reachable.
   */
  virtual void SetChildNode (iSkeletonAnimNodeFactory* node) = 0;

  /**
   * Return the child animation node of this node.
   */
  virtual iSkeletonAnimNodeFactory* GetChildNode () = 0;

  /**
   * Clear the child animation node of this node.
   */
  virtual void ClearChildNode () = 0;
};

/**
 * An animation node that controls a bone of an animesh in order to make it look
 * at a target.
 */
struct iSkeletonLookAtNode : public iSkeletonAnimNode
{
  SCF_INTERFACE(CS::Animation::iSkeletonLookAtNode, 1, 0, 0);

  /**
   * Return whether or not there is currently a target defined
   */
  virtual bool HasTarget () = 0;

  /**
   * Set the target to look at as a fixed position (in world coordinates). Don't
   * be afraid to update often this position if you want it moving.
   * Listeners will be called with the 'target lost' event if a target was specified and
   * was reached.
   */
  virtual void SetTarget (csVector3 target) = 0;

  /**
   * Set the target to look at as a iMovable.
   * Listeners will be called with the 'target lost' event if a target was specified and
   * was reached.
   * \param target The iMovable target.
   * \param offset An offset on the target (in iMovable coordinates).
   */
  virtual void SetTarget (iMovable* target, const csVector3& offset) = 0;

  // TODO: remove this method once iCamera has iMovable implemented
  /**
   * Set the target to look at as a iCamera.
   * Listeners will be called with the 'target lost' event if a target was specified and
   * was reached.
   * \param target The iCamera target.
   * \param offset An offset on the target (in iCamera coordinates).
   */
  virtual void SetTarget (iCamera* target, const csVector3& offset) = 0;

  /**
   * Remove the current target, ie the animation node will stop acting once the bone has
   * reached the position given by the child node.
   * The listeners will be called with the 'target lost' event iff a target was specified and
   * was reached.
   */
  virtual void RemoveTarget () = 0;

  /**
   * Add a listener to be notified when the target has been reached or lost.
   */
  virtual void AddListener (iSkeletonLookAtListener* listener) = 0;

  /**
   * Remove the specified listener.
   */
  virtual void RemoveListener (iSkeletonLookAtListener* listener) = 0;
};

/**
 * A listener to be implemented if you want to be notified when the target has been
 * reached or lost.
 */
struct iSkeletonLookAtListener : public virtual iBase
{
  SCF_INTERFACE (CS::Animation::iSkeletonLookAtListener, 1, 0, 0);

  /**
   * The target is now looked at.
   */
  virtual void TargetReached () = 0;

  /**
   * The target is no more looked at, ie it was unreachable given the various constraints.
   */
  virtual void TargetLost () = 0;
};

} // namespace Animation
} // namespace CS

/** @} */

#endif //__CS_IMESH_ANIMNODE_LOOKAT_H__
