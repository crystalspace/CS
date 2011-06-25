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
#ifndef __CS_IMESH_ANIMNODE_RETARGET_H__
#define __CS_IMESH_ANIMNODE_RETARGET_H__

/**\file
 * Retarget animation node for an animated mesh.
 */

#include "csutil/scf_interface.h"

#include "imesh/animnode/skeleton2anim.h"
#include "csutil/hash.h"
#include "csutil/stringquote.h"

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

  /// Remove a mapping between the given bones
  void RemoveMapping (CS::Animation::BoneID sourceBone, CS::Animation::BoneID targetBone)
  {
    sourceBones.DeleteAll (sourceBone);
    targetBones.DeleteAll (targetBone);
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

  void DebugPrint (CS::Animation::iSkeletonFactory* sourceSkeleton,
		   CS::Animation::iSkeletonFactory* targetSkeleton) const
  {
    csPrintf ("Bone mapping:\n");

    for (csHash<CS::Animation::BoneID, CS::Animation::BoneID>::ConstGlobalIterator it =
	   sourceBones.GetIterator (); it.HasNext (); )
    {
      csTuple2<CS::Animation::BoneID, CS::Animation::BoneID> tuple = it.NextTuple ();

      /* Note: 'real' BoneID format is %zu, but use %lu here to quell warnings
	 when compiling with -ansi -pedantic. */
      csPrintf ("source bone %lu", (unsigned long)tuple.second);
      if (sourceSkeleton->HasBone (tuple.second))
	csPrintf (" (%s) to target bone %lu (",
		  CS::Quote::Single (sourceSkeleton->GetBoneName (tuple.second)),
		  (unsigned long)tuple.first);
      else
	csPrintf ("(invalid) to target bone %lu (", (unsigned long)tuple.first);
      if (targetSkeleton->HasBone (tuple.first))
	csPrintf ("%s)\n", CS::Quote::Single (targetSkeleton->GetBoneName (tuple.first)));
      else
	csPrintf ("invalid)\n");
    }

    csPrintf ("End of bone mapping:\n");
  };

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
struct iSkeletonRetargetNodeManager
  : public virtual CS::Animation::iSkeletonAnimNodeManager<CS::Animation::iSkeletonRetargetNodeFactory>
{
  SCF_ISKELETONANIMNODEMANAGER_INTERFACE (CS::Animation::iSkeletonRetargetNodeManager, 1, 0, 0);
};

/**
 * Factory for the 'Retarget' animation node. This animation node can retarget an animation from
 * one skeleton to another. It is useful for example to import motion capture data into your animesh.
 *
 * This node works by simply copying the rotation and position of the bones from the source to the
 * target skeleton, after having converted the transformations from one space to another. This works
 * effectively only if the two skeletons have a similar topology and default pose.
 *
 * To overcome the problem of a different default pose, this node has the ability to align the bones
 * in world space instead of naively copying the applied rotation. To use this more advanced mode,
 * you need to define some more semantic about the structure of the skeleton, this is made through
 * the AddBodyChain()/RemoveBodyChain() methods.
 *
 * In all cases, this node will only be able to retarget the animations of the bones covered by the
 * bone mapping provided by the user (see SetBoneMapping()).
 */
struct iSkeletonRetargetNodeFactory : public virtual iSkeletonAnimNodeFactory
{
  SCF_INTERFACE(CS::Animation::iSkeletonRetargetNodeFactory, 2, 0, 0);

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
   * to another. The bones from this chain will be aligned in world space instead of
   * simply copied from the source skeleton.
   *
   * The given body chain can hold only one child bone for each bone of the chain
   * (it is therefore effectively a chain, not a tree).
   */
  virtual void AddBodyChain (CS::Animation::iBodyChain* chain) = 0;

  /**
   * Remove a body chain indicating a main structure that can be retargeted from one skeleton
   * to another.
   */
  virtual void RemoveBodyChain (CS::Animation::iBodyChain* chain) = 0;
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
