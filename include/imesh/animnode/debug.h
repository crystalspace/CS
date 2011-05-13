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
#ifndef __CS_IMESH_ANIMNODE_DEBUG_H__
#define __CS_IMESH_ANIMNODE_DEBUG_H__

/**\file
 * Debug animation node for an animated mesh.
 */

#include "csutil/scf_interface.h"
#include "csutil/cscolor.h"
#include "imesh/animnode/skeleton2anim.h"
#include "ivideo/graph3d.h"

struct iCamera;
struct iMaterialWrapper;
struct iSector;
class csPixmap;

/**\addtogroup meshplugins
 * @{ */

namespace CS {
namespace Animation {

struct iSkeletonDebugNodeFactory;
struct iBodySkeleton;
struct iBodyChain;

/**
 * A class to manage the creation and deletion of debug animation 
 * node factories.
 */
struct iSkeletonDebugNodeManager
  : public virtual CS::Animation::iSkeletonAnimNodeManager<CS::Animation::iSkeletonDebugNodeFactory>
{
  SCF_ISKELETONANIMNODEMANAGER_INTERFACE (CS::Animation::iSkeletonDebugNodeManager, 1, 0, 0);
};

// ----------------------------- iSkeletonDebugNode -----------------------------

/**
 * The visualization mode to be used by the CS::Animation::iSkeletonDebugNode.
 */
enum SkeletonDebugMode
{
  DEBUG_NONE = 0,             /*!< No debug shapes are displayed. */
  DEBUG_2DLINES = 1 << 1,     /*!< The debug shapes displayed are 2D lines between the bones. */
  DEBUG_SQUARES = 1 << 2,     /*!< The debug shapes displayed are 2D squares at the bone positions. */
  DEBUG_IMAGES  = 1 << 3,     /*!< The debug shapes displayed are images at the bone positions.
			       *   SetDebugImage() must therefore be used. */
  DEBUG_BBOXES  = 1 << 4      /*!< The debug shapes displayed are the bounding boxes of the bones. */
};

/**
 * Factory for the 'debug' animation node (see CS::Animation::iSkeletonDebugNode).
 */
struct iSkeletonDebugNodeFactory : public virtual iSkeletonAnimNodeFactory
{
  SCF_INTERFACE(CS::Animation::iSkeletonDebugNodeFactory, 2, 0, 0);

  /**
   * Set the combination of visualization modes to be used for displaying the animation.
   * Default value is CS::Animation::DEBUG_SQUARES.
   */
  virtual void SetDebugModes (SkeletonDebugMode modes) = 0;

  /**
   * Get the combination of visualization modes to be used for displaying the animation.
   */
  virtual SkeletonDebugMode GetDebugModes () = 0;

  /**
   * Set the image to be displayed when CS::Animation::DEBUG_IMAGES is used.
   */
  virtual void SetDebugImage (csPixmap* image) = 0;

  /**
   * Set the bone mask to be used. If this mask is provided, then only the bones from this
   * mask will be displayed.
   */
  virtual void SetBoneMask (csBitArray& boneMask) = 0;

  /**
   * Unset the mask of bones that are displayed. All bones will now be displayed.
   */
  virtual void UnsetBoneMask () = 0;

  /**
   * Set whether or not the leaf bones of the skeleton are displayed.
   */
  virtual void SetLeafBonesDisplayed (bool displayed) = 0;

  /**
   * Set the child animation node of this node. It is valid to set a null 
   * reference as chid node.
   */
  virtual void SetChildNode (iSkeletonAnimNodeFactory* factory) = 0;

  /**
   * Get the child animation node of this node.
   */
  virtual iSkeletonAnimNodeFactory* GetChildNode () const = 0;

  /**
   * Set wether or not random colors are used to display the bone data.
   * The default value is 'true'.
   */
  virtual void SetRandomColor (bool random) = 0;

  /**
   * Return whether or not random colors are used to display the bone data
   */
  virtual bool GetRandomColor () const = 0;
};

/**
 * An animation node that let visualize the dynamics of any animation node.
 * Place it anywhere in the animation blending tree to visualize the children
 * of this node.
 * \warning Currently, this node actually displays the last state of the skeleton,
 * not the state defined by the child animation node.
 * \warning You have to call Draw() at each frame if you want the 2D information
 * displayed by this node.
 */
struct iSkeletonDebugNode : public virtual iSkeletonAnimNode
{
  SCF_INTERFACE(iSkeletonDebugNode, 2, 0, 0);

  /**
   * Draw the 2D visual information
   */
  virtual void Draw (iCamera* camera, csColor color = csColor (255, 0, 255)) = 0;
};

} // namespace Animation
} // namespace CS

/** @} */

#endif //__CS_IMESH_ANIMNODE_DEBUG_H__
