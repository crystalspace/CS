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
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef __CS_IMESH_ANIMNODE_SPEED_H__
#define __CS_IMESH_ANIMNODE_SPEED_H__

/**\file
 * Speed animation node for an animated mesh.
 */

#include "csutil/scf_interface.h"
#include "imesh/animnode/skeleton2anim.h"

/**\addtogroup meshplugins
 * @{ */

namespace CS {
namespace Animation {

struct iSkeletonSpeedNodeFactory;

/**
 * A class to manage the creation and deletion of speed animation 
 * node factories.
 */
struct iSkeletonSpeedNodeManager
  : public virtual CS::Animation::iSkeletonAnimNodeManager<CS::Animation::iSkeletonSpeedNodeFactory>
{
  SCF_ISKELETONANIMNODEMANAGER_INTERFACE (CS::Animation::iSkeletonSpeedNodeManager, 1, 0, 0);
};

/**
 * Factory for the 'speed' animation node.
 * This animation node takes some animations of the animesh moving at different speed
 * (eg idle, walking, running), and blend them to achieve any custom speed.
 */
struct iSkeletonSpeedNodeFactory : public virtual iSkeletonAnimNodeFactory
{
  SCF_INTERFACE(CS::Animation::iSkeletonSpeedNodeFactory, 2, 0, 0);

  /**
   * Add a child animation node to this node. This child node should provide the
   * animation of the animesh moving at the given speed.
   * 
   * The factory node should have been made cyclic, otherwise the 'speed' animation node will not
   * work properly. However, as a user help, if the node is a CS::Animation::iSkeletonAnimationNodeFactory,
   * then the call to CS::Animation::iSkeletonAnimationNodeFactory::SetCyclic() is made automatically.
   * 
   * Also, all animations that are added must be synchronized on their first frame, eg all
   * animations starting when the left foot touches the ground. The animations can have
   * different duration, but they need to have only one cycle of the animation (eg the
   * animation stops when the left foot touches again the ground).
   * 
   * The speed scale is arbitrary and do not have to correspond to the effective mesh speed,
   * it is the ratio between the various speeds that is important. A speed of 0 is absolutely
   * allowed and will be treated differently, but a negative speed has not been tested...
   */
  virtual void AddNode (iSkeletonAnimNodeFactory* factory, float speed) = 0;
};

/**
 * An animation node that takes some animations of the animesh moving at different speed
 * (eg idle, walking, running), and blend them to achieve any custom speed.
 */
struct iSkeletonSpeedNode : public iSkeletonAnimNode
{
  SCF_INTERFACE(CS::Animation::iSkeletonSpeedNode, 1, 0, 0);

  /**
   * Set the desired custom speed. The value should be between the slowest and the quickest
   * child node, otherwise it will be truncated.
   */
  virtual void SetSpeed (float speed) = 0;
};

} // namespace Animation
} // namespace CS

/** @} */

#endif //__CS_IMESH_ANIMNODE_SPEED_H__
