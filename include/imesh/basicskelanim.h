/*
  Copyright (C) 2010 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

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
#ifndef __CS_SKELNODES_BASIC_H__
#define __CS_SKELNODES_BASIC_H__

/**\file
 * Basic animation nodes for an animated mesh.
 */

#include "csutil/scf_interface.h"
#include "imesh/skeleton2anim.h"

/**\addtogroup meshplugins
 * @{ */

namespace CS
{
namespace Animation
{

struct iSkeletonSpeedNodeFactory2;

/**
 * A class to manage the creation and deletion of basic animation 
 * node factories.
 */
struct iSkeletonBasicNodesManager2 : public virtual iBase
{
  SCF_INTERFACE(CS::Animation::iSkeletonBasicNodesManager2, 1, 0, 0);

  /**
   * Create a 'speed' animation node factory of the given name.
   */
  virtual iSkeletonSpeedNodeFactory2* CreateSpeedNodeFactory (const char* name) = 0;

  /**
   * Find the 'speed' animation node factory of the given name.
   */
  virtual iSkeletonSpeedNodeFactory2* FindSpeedNodeFactory (const char* name) = 0;

  /**
   * Delete all 'speed' animation node factories.
   */
  virtual void ClearSpeedNodeFactories () = 0;
};

/**
 * Factory for the 'speed' animation node.
 */
struct iSkeletonSpeedNodeFactory2 : public iSkeletonAnimNodeFactory2
{
  SCF_INTERFACE(CS::Animation::iSkeletonSpeedNodeFactory2, 1, 0, 0);

  /**
   * Add a child animation node to this controller. This child node should provide the
   * animation of the animesh moving at the given speed.
   * 
   * The factory node should have been made cyclic, otherwise the 'speed' controller will not
   * work properly. However, as a user help, if the node is a CS::Animation::iSkeletonAnimationNodeFactory2,
   * then the call to CS::Animation::iSkeletonAnimationNodeFactory2::SetCyclic() is made automatically.
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
  virtual void AddNode (iSkeletonAnimNodeFactory2* factory, float speed) = 0;
};

/**
 * An animation node that takes some animations of an Animated Mesh moving at different speed
 * (eg idle, walking, running) and blend them to achieve any custom speed.
 */
struct iSkeletonSpeedNode2 : public iSkeletonAnimNode2
{
  SCF_INTERFACE(CS::Animation::iSkeletonSpeedNode2, 1, 0, 0);

  /**
   * Set the desired custom speed. The value should be between the slowest and the quickest
   * child node, otherwise it will be truncated.
   */
  virtual void SetSpeed (float speed) = 0;
};

} // namespace Animation
} // namespace CS

/** @} */

#endif //__CS_SKELNODES_BASIC_H__
