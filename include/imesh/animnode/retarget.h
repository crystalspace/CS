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
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef __CS_IMESH_ANIMNODE_RETARGET_H__
#define __CS_IMESH_ANIMNODE_RETARGET_H__

/**\file
 * Retarget animation node for an animated mesh.
 */

#include "csutil/scf_interface.h"

#include "imesh/animnode/skeleton2anim.h"
#include "csutil/hash.h"

/**\addtogroup meshplugins
 * @{ */

namespace CS {
namespace Animation {

struct iBodyChain;
struct iSkeletonFactory;
struct iSkeletonRetargetNodeFactory;

/**
 * This class holds the mapping between the bones of a source and a target skeleton
 * \sa CS::Animation::NameBoneMappingHelper
 */
struct BoneMapping
{
  /// Add a mapping between the given bones
  void AddMapping (CS::Animation::BoneID sourceBone, CS::Animation::BoneID targetBone)
  {
    sourceBones.PutUnique (sourceBone, targetBone);
    targetBones.PutUnique (targetBone, sourceBone);
  }

  /// Get the corresponding bone on the source skeleton
  CS::Animation::BoneID GetSourceBone (CS::Animation::BoneID bone)
  {
    if (!targetBones.Contains (bone))
      return CS::Animation::InvalidBoneID;

    return *targetBones.GetElementPointer (bone);
  }

  /// Get the corresponding bone on the target skeleton
  CS::Animation::BoneID GetTargetBone (CS::Animation::BoneID bone)
  {
    if (!sourceBones.Contains (bone))
      return CS::Animation::InvalidBoneID;

    return *sourceBones.GetElementPointer (bone);
  }

private:
  csHash<CS::Animation::BoneID, CS::Animation::BoneID> sourceBones;
  csHash<CS::Animation::BoneID, CS::Animation::BoneID> targetBones;
};

/// This helper class can generate a bone mapping by simply taking the corresponding bones that
/// have the same name
struct NameBoneMappingHelper
{
  /// Generate a bone mapping by taking the corresponding bones that have the same name
  static void GenerateMapping (CS::Animation::BoneMapping& mapping,
			       CS::Animation::iSkeletonFactory* sourceSkeleton,
			       CS::Animation::iSkeletonFactory* targetSkeleton)
  {
    for (size_t i = 0; i <= targetSkeleton->GetTopBoneID (); i++)
    {
      if (!targetSkeleton->HasBone (i))
	continue;

      csString targetBoneName = targetSkeleton->GetBoneName (i);
      CS::Animation::BoneID sourceBoneID =
	sourceSkeleton->FindBone (targetBoneName.GetData ());

      if (sourceBoneID != CS::Animation::InvalidBoneID)
	mapping.AddMapping (sourceBoneID, i);
    }
  }
};

/**
 * A class to manage the creation and deletion of 'Retarget' animation 
 * node factories.
 */
struct iSkeletonRetargetNodeManager : public virtual iBase
{
  SCF_INTERFACE(CS::Animation::iSkeletonRetargetNodeManager, 1, 0, 0);

  /**
   * Create a new 'Retarget' animation node factory.
   * \param name The name of the new factory.
   */
  virtual iSkeletonRetargetNodeFactory* CreateAnimNodeFactory (const char *name) = 0;

  /**
   * Find the specified 'Retarget' animation node factory.
   */
  virtual iSkeletonRetargetNodeFactory* FindAnimNodeFactory (const char* name) const = 0;

  /**
   * Delete all 'Retarget' animation node factories.
   */
  virtual void ClearAnimNodeFactories () = 0;
};

/**
 * The mode of animation retargeting
 * \sa CS::Animation::iSkeletonRetargetNodeFactory
 */
enum RetargetMode
{
  RETARGET_NAIVE = 0,     /*!< The retargeting mode is a naive application of the bone rotation.
			    This mode does not work very well but is the most easier to use. */
  RETARGET_ALIGN_BONES    /*!< The retargeting mode is made by trying to align the bones of the skeletons.
			    In order to work effectively, this mode needs to use the definition
			    of body chains covering the skeleton bones as most as possible. See
			    CS::Animation::iSkeletonRetargetNodeFactory::AddBodyChain() for more
			    information.*/
};

/**
 * Factory for the 'Retarget' animation node. This animation node can retarget an animation from
 * one skeleton to another. It is useful for example to import motion capture data into your animesh.
 *
 * To work properly, this animation node needs a bone mapping (see SetBoneMapping()), and it may also
 * need to define body chains (see AddBodyChain()) if you want to use the more advanced retargeting mode
 * CS::Animation::RETARGET_ALIGN_BONES (see SetRetargetMode()).
 */
struct iSkeletonRetargetNodeFactory : public iSkeletonAnimNodeFactory
{
  SCF_INTERFACE(CS::Animation::iSkeletonRetargetNodeFactory, 1, 0, 0);

  /**
   * Set the child animation node of this node. This child node plays the animation for the source skeleton.
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

  /**
   * Set the source skeleton that has to be used to retarget the animation from.
   */
  virtual void SetSourceSkeleton (CS::Animation::iSkeletonFactory* skeleton) = 0;

  /**
   * Set the source skeleton that has to be used to retarget the animation from.
   */
  virtual void SetBoneMapping (CS::Animation::BoneMapping& mapping) = 0;

  /**
   * Add a body chain indicating a main structure that can be retargeted from one skeleton
   * to another. This body chain can hold only one child bone for each bone of the chain.
   * This method is only useful if you use CS::Animation::RETARGET_ALIGN_BONES
   * (see SetRetargetMode()).
   */
  virtual void AddBodyChain (CS::Animation::iBodyChain* chain) = 0;

  /**
   * Remove a body chain indicating a main structure that can be retargeted from one skeleton
   * to another.
   */
  virtual void RemoveBodyChain (CS::Animation::iBodyChain* chain) = 0;

  /**
   * Set the retargeting mode to be used. The default value is CS::Animation::RETARGET_NAIVE.
   */
  virtual void SetRetargetMode (CS::Animation::RetargetMode mode) = 0;

  /**
   * Get the retargeting mode to be used
   */
  virtual CS::Animation::RetargetMode GetRetargetMode () const = 0;
};

/**
 * An animation node that can retarget an animation from one skeleton to another. It is
 * useful for example to import motion capture data into your animesh.
 */
struct iSkeletonRetargetNode : public iSkeletonAnimNode
{
  SCF_INTERFACE(CS::Animation::iSkeletonRetargetNode, 1, 0, 0);

};

} // namespace Animation
} // namespace CS

/** @} */

#endif //__CS_IMESH_ANIMNODE_RETARGET_H__
