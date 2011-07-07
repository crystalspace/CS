/*
  Copyright (C) 2010 Christian Van Brussel, Institute of Information
      and Communication Technologies, Electronics and Applied Mathematics
      at Universite catholique de Louvain, Belgium
      http://www.uclouvain.be/en-icteam.html

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
#ifndef __CS_IMESH_ANIMNODE_IK_H__
#define __CS_IMESH_ANIMNODE_IK_H__

/**\file
 * Inverse Kinematics animation nodes for an animated mesh.
 */

#include "csutil/scf_interface.h"
#include "imesh/animnode/skeleton2anim.h"

struct iMovable;
struct iCamera;
class csOrthoTransform;

namespace CS {
namespace Mesh {

struct iAnimatedMesh;

} // namespace Mesh
} // namespace CS

/**\addtogroup meshplugins
 * @{ */

namespace CS {
namespace Animation {

struct iBodyChain;
struct iBodySkeleton;
struct iSkeletonRagdollNode;

/// Identifier for an effector within an Inverse Kinematics animation node
typedef unsigned int EffectorID;

struct iSkeletonIKNodeFactory;

/**
 * A class to manage the creation and deletion of Inverse Kinematics animation 
 * node factories.
 */
struct iSkeletonIKNodeManager
  : public virtual CS::Animation::iSkeletonAnimNodeManager<CS::Animation::iSkeletonIKNodeFactory>
{
  SCF_ISKELETONANIMNODEMANAGER_INTERFACE (CS::Animation::iSkeletonIKNodeManager, 1, 0, 0);
};

/**
 * Factory for the Inverse Kinematics animation node. With Inverse Kinematics, you can
 * generate an animation for a sub-part of the skeleton (a CS::Animation::iBodyChain) by
 * specifying some geometrical constraints on some effectors placed on the skeleton. This
 * can be used eg to grab an object or to avoid foot sliding problems within a locomotion
 * system.
 *
 * You must first define some effectors, ie some points on the skeleton. Then in the
 * CS::Animation::iSkeletonIKNode, you will be able to define some geometric constraints
 * on these effectors.
 *
 * \sa iSkeletonIKPhysicalNodeFactory
 */
struct iSkeletonIKNodeFactory : public virtual iSkeletonAnimNodeFactory
{
  SCF_INTERFACE(CS::Animation::iSkeletonIKNodeFactory, 2, 0, 0);

  /**
   * Set the physical description of the skeleton.
   */
  virtual void SetBodySkeleton (CS::Animation::iBodySkeleton* skeleton) = 0;

  /**
   * Get the physical description of the skeleton.
   */
  virtual CS::Animation::iBodySkeleton* GetBodySkeleton () const = 0;

  /**
   * Set the child animation node of this node. The IK controller will
   * add its control on top of the animation of the child node. This child
   * node is not mandatory.
   *
   * It is valid to set a null reference as chid node.
   */
  virtual void SetChildNode (CS::Animation::iSkeletonAnimNodeFactory* node) = 0;

  /**
   * Return the child animation node of this node.
   */
  virtual CS::Animation::iSkeletonAnimNodeFactory* GetChildNode () const = 0;

  /**
   * Add an effector to this factory.
   * \param chain The sub-part of the skeleton that is controlled by this node when
   * the effector is constrained.
   * \param bone The bone where sits the effector.
   * \param transform The position of the effector, relatively to the bone.
   */
  virtual CS::Animation::EffectorID AddEffector (CS::Animation::iBodyChain* chain,
						 BoneID bone,
						 csOrthoTransform& transform) = 0;

  /**
   * Remove the given effector
   */
  virtual void RemoveEffector (CS::Animation::EffectorID effector) = 0;

  // TODO: listeners
};

/**
 * An animation node that generates an animation for a sub-part of the skeleton (a
 * CS::Animation::iBodyChain) by specifying some geometrical constraints on the
 * effectors placed on the skeleton.
 *
 * This node is inactive until there are some constraints on some effector. The
 * effectors are defined by iSkeletonIKNodeFactory::AddEffector().
 *
 * \sa iSkeletonIKPhysicalNode
 */
struct iSkeletonIKNode : public virtual iSkeletonAnimNode
{
  SCF_INTERFACE(CS::Animation::iSkeletonIKNode, 2, 0, 0);

  /**
   * Add a constraint on the given effector so that it sticks to the given world transform.
   * \param effector ID of the effector
   * \param transform Transform to stick to, in world coordinates
   */
  virtual void AddConstraint (CS::Animation::EffectorID effector,
			      csOrthoTransform& transform) = 0;

  /**
   * Add a constraint on the given effector so that it sticks to the given transform of the
   * iMovable.
   * \param effector ID of the effector
   * \param target The iMovable to stick to.
   * \param offset Offset transform to the iMovable, in the iMovable local coordinates.
   */
  virtual void AddConstraint (CS::Animation::EffectorID effector,
			      iMovable* target, const csOrthoTransform& offset) = 0;

  /**
   * Add a constraint on the given effector so that it sticks to the given transform of the
   * iCamera.
   * \param effector ID of the effector
   * \param target The iCamera to stick to.
   * \param offset Offset transform to the iCamera, in the iCamera local coordinates.
   */
  virtual void AddConstraint (CS::Animation::EffectorID effector,
			      iCamera* target, const csOrthoTransform& offset) = 0;

  /**
   * Remove the constraint on the given effector. This animation node won't be active anymore
   * if there are no more constraints.
   */
  virtual void RemoveConstraint (CS::Animation::EffectorID effector) = 0;
};

/**
 * An implementation of the CS::Animation::iSkeletonIKNodeFactory based on physical
 * simulation. This node will use a CS::Animation::iSkeletonRagdollNode and apply
 * physical constraints on the rigid bodies created by the ragdoll node.
 *
 * This IK method has the advantage to be able to be physically accurate. It has for
 * example the uncommon capability to manage the collisions with the body and the
 * environment. This IK method is however not suited in applications where you need
 * the IK solution to be independant of the history of the motion. This method is also
 * less efficient than traditionial IK algorithms.
 *
 * \warning The current implementation does not care about the rotational component
 * of the constraints.
 *
 * \sa iSkeletonIKNodeFactory
 */
struct iSkeletonIKPhysicalNodeFactory : public iSkeletonIKNodeFactory
{
  SCF_INTERFACE(CS::Animation::iSkeletonIKPhysicalNodeFactory, 1, 0, 0);

  /**
   * Set whether or not the CS::Animation::iBodyChain controlled by this node have to be
   * reset to their previous dynamic state when there are no more constraints on the chain.
   * Default value is true.
   *
   * The body chain is indeed put in the CS::Animation::STATE_DYNAMIC state when there is
   * at least one active constraint (see CS::Animation::iSkeletonRagdollNode::SetBodyChainState()).
   */
  virtual void SetChainAutoReset (bool reset) = 0;

  /**
   * Get whether or not the CS::Animation::iBodyChain controlled by this node have to be
   * reset to their previous dynamic state when there are no more constraints on the chain.
   *
   * The body chain is indeed put in the CS::Animation::STATE_DYNAMIC state when there is
   * at least one active constraint (see CS::Animation::iSkeletonRagdollNode::SetBodyChainState()).
   */
  virtual bool GetChainAutoReset () const = 0;
};

/**
 * An implementation of the CS::Animation::iSkeletonIKNode based on physical
 * simulation. This node will use a CS::Animation::iSkeletonRagdollNode and apply
 * physical constraints on the rigid bodies created by the ragdoll node.
 *
 * This IK method has the advantage to be able to be physically accurate. It has for
 * example the uncommon capability to manage the collisions with the body and the
 * environment. This IK method is however not suited in applications where you need
 * the IK solution to be independant of the history of the motion. This method is also
 * less efficient than traditionial IK algorithms.
 *
 * \warning The current implementation does not care about the rotational component
 * of the constraints.
 *
 * \sa iSkeletonIKNode
 */
struct iSkeletonIKPhysicalNode : public iSkeletonIKNode
{
  SCF_INTERFACE(CS::Animation::iSkeletonIKPhysicalNode, 1, 0, 0);

  /**
   * Set the ragdoll node to be used by this node. The ragdoll node has to be active
   * somewhere inside the animation blending tree (but its effective position has no importance).
   */
  virtual void SetRagdollNode (CS::Animation::iSkeletonRagdollNode* ragdollNode) = 0;

  /**
   * Get the ragdoll node to be used by this node.
   */
  virtual CS::Animation::iSkeletonRagdollNode* GetRagdollNode () const = 0;
};

/**
 * An implementation of the CS::Animation::iSkeletonIKNodeFactory based on a quaternion version
 * of the Cyclic Coordinate Descent algorithm.
 *
 * The general behavior of the CCD algorithm is to iterate over each bone of the
 * chain, and apply a portion of the rotation needed to get the effector closer
 * to the target. 
 *
 * The CCD algorithm is the most common algorithm for Inverse Kinematics because
 * it is simple and rather efficient. One of the main disadvantage is that it is
 * not stable in the sense that a small change to the configuration of the IK target
 * can lead to a big change in the result of the algorithm.
 *
 * \warning The current implementation does not care about the rotational component
 * of the constraints.
 * \warning The current implementation can only manage one constraint per chain.
 *
 * \sa iSkeletonIKNodeFactory
 */
struct iSkeletonIKCCDNodeFactory : public iSkeletonIKNodeFactory
{
  SCF_INTERFACE(CS::Animation::iSkeletonIKCCDNodeFactory, 1, 0, 0);

  /**
   * Set the maximum number of iterations that can be done before the algorithm stops.
   * The default value is 50.
   */
  virtual void SetMaximumIterations (size_t max) = 0;

  /**
   * Get the maximum number of iterations that can be done before the algorithm stops.
   */
  virtual size_t GetMaximumIterations () = 0;

  /**
   * Set the minimum distance between the target and the effector that has to be
   * reached before the algorithm stops. The default value is 0.001.
   */
  virtual void SetTargetDistance (float distance) = 0;

  /**
   * Get the minimum distance between the target and the effector that has to be
   * reached before the algorithm stops.
   */
  virtual float GetTargetDistance () = 0;

  /**
   * Set the portion of the rotation that has to be applied at each joint at each
   * iteration of the algorithm. The value should be kept bigger than 0.0, and
   * smaller or equal to 1.0. The default value is 0.1.
   */
  virtual void SetMotionRatio (float ratio) = 0;

  /**
   * Get the portion of the rotation that has to be applied at each joint at each
   * iteration of the algorithm.
   */
  virtual float GetMotionRatio () = 0;

  /**
   * Set whether the rotation of each joint must be initialized with the value from
   * the child node. This can be used to achieve more realistic results by directing
   * the motion, but it can also generate less stable results. The default value is true.
   */
  virtual void SetJointInitialization (bool initialized) = 0;

  /**
   * Get whether the rotation of each joint must be initialized with the value from
   * the child node. This can be used to achieve more realistic results by directing
   * the motion, but it can also generate less stable results.
   */
  virtual bool GetJointInitialization () = 0;

  /**
   * Set whether or not the joints should be iterated upward, from the leaf node to the
   * parent node. Iterating upward favours the motion for the end nodes to the detriment
   * of the root nodes. The default value is true.
   */
  virtual void SetUpwardIterations (bool upward) = 0;

  /**
   * Get whether or not the joints should be iterated upward, from the leaf node to the
   * parent node. Iterating upward favours the motion for the end nodes to the detriment
   * of the root nodes.
   */
  virtual bool GetUpwardIterations () = 0;
};

/**
 * An implementation of the CS::Animation::iSkeletonIKNodeFactory based on a quaternion version
 * of the Cyclic Coordinate Descent algorithm.
 *
 * The general behavior of the CCD algorithm is to iterate over each bone of the
 * chain, and apply a portion of the rotation needed to get the effector closer
 * to the target. 
 *
 * The CCD algorithm is the most common algorithm for Inverse Kinematics because
 * it is simple and rather efficient. One of the main disadvantage is that it is
 * not stable in the sense that a small change to the configuration of the IK target
 * can lead to a big change in the result of the algorithm.
 *
 * \warning The current implementation does not care about the rotational component
 * of the constraints.
 * \warning The current implementation can only manage one constraint per chain.
 *
 * \sa iSkeletonIKNode
 */
struct iSkeletonIKCCDNode : public iSkeletonIKNode
{
  SCF_INTERFACE(CS::Animation::iSkeletonIKCCDNode, 1, 0, 0);

};

} // namespace Animation
} // namespace CS

/** @} */

#endif //__CS_IMESH_ANIMNODE_IK_H__
