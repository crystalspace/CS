/*
  Copyright (C) 2008 by Marten Svanfeldt

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

#ifndef __CS_IMESH_SKELETON2_H__
#define __CS_IMESH_SKELETON2_H__

#include "csutil/scf_interface.h"
#include "csutil/ref.h"
#include "csutil/refcount.h"
#include "csutil/bitarray.h"
#include "csgeom/quaternion.h"
#include "csgeom/vector3.h"

class csDualQuaternion;

/**\file
 * Skeleton2 interface files
 */

struct iSceneNode;

namespace CS {
namespace Mesh {

struct iAnimatedMesh;

} // namespace Mesh
} // namespace CS

namespace CS {
namespace Animation {

struct iSkeletonFactory;
struct iSkeleton;

class csSkeletalState;

struct iSkeletonAnimPacketFactory;
struct iSkeletonAnimPacket;

/**\addtogroup meshplugins
 * @{ */

/**\name Skeleton
 * @{ */


/// Identifier for single bone data
typedef unsigned int BoneID;

/// ID for an invalid bone
static const BoneID InvalidBoneID = (BoneID)~0;

/**
 * Skeletal system base object, representing the entire skeletal and
 * skeletal animation system.
 */
struct iSkeletonManager : public virtual iBase
{
  SCF_INTERFACE(CS::Animation::iSkeletonManager, 1, 0, 0);

  /**
   * Create a new empty skeleton factory
   */
  virtual iSkeletonFactory* CreateSkeletonFactory (const char* name) = 0;

  /**
   * Find an already created skeleton factory
   */
  virtual iSkeletonFactory* FindSkeletonFactory (const char* name) = 0;

  /**
   * Remove all skeleton factories
   */
  virtual void ClearSkeletonFactories () = 0;

  /**
   * Create a new empty skeletal animation packet factory
   */
  virtual iSkeletonAnimPacketFactory* CreateAnimPacketFactory (const char* name) = 0;

  /**
   * Find a skeletal animation packet factory
   */
  virtual iSkeletonAnimPacketFactory* FindAnimPacketFactory (const char* name) = 0;

  /**
   * Remove all animation packet factories
   */
  virtual void ClearAnimPacketFactories () = 0;

  /**
   * Remove all internal data
   */
  virtual void ClearAll () = 0;
};

/**
 * A skeleton factory is an object defining the base pose and topology
 * from which CS::Animation::iSkeleton2 instances can be created.
 *
 * A note on coordinate spaces, cause there are a few to keep track of.
 * Within the skeleton factory there are two types coordinate spaces:
 *
 * - Absolute space. Absolute space is defined by the skeleton ,
 *   that is, the root bone(s) is defined in absolute space.
 * - Bone space. Bone space for every bone is defined by its parent.
 */
struct iSkeletonFactory : public virtual iBase
{
  SCF_INTERFACE(CS::Animation::iSkeletonFactory, 1, 0, 2);

  /**\name Bone handling
   * @{ */

  /**
   * Create a new bone with given parent
   * \param parent bone id of parent or ~0 for no parent which creates a top
   * level bone
   */
  virtual BoneID CreateBone (BoneID parent = InvalidBoneID) = 0;

  /**
   * Find a bone id from its name
   * \param name bone name
   */
  virtual BoneID FindBone (const char *name) const = 0;

  /**
   * Remove a bone from skeleton. Any bones having the removed bone as parent
   * will be reparented one step up the chain.
   * \param bone bone id of bone to remove
   */
  virtual void RemoveBone (BoneID bone) = 0;

  /**
   * Get the bone parent id
   * \return parent id on success. Non existing bones will return ~0 as parent.
   * \param bone bone Id
   */
  virtual BoneID GetBoneParent (BoneID bone) const = 0;

  /**
   * Return true if bone with given id exists within skeleton factory
   */
  virtual bool HasBone (BoneID bone) const = 0;

  /**
   * Set a name of a bone for later identificaton
   */
  virtual void SetBoneName (BoneID bone, const char* name) = 0;

  /**
   * Get the name of a bone
   */
  virtual const char* GetBoneName (BoneID bone) const = 0;

  /**
   * Get the highest bone ID used in skeleton
   */
  virtual BoneID GetTopBoneID () const = 0;

  /**
   * Get the bone transform in bone space.
   * \param bone bone id to get the transform for
   * \param rot rotation quaternion
   * \param offset movement offset
   */
  virtual void GetTransformBoneSpace (BoneID bone, csQuaternion& rot, 
    csVector3& offset) const = 0;

  /**
   * Set the bone transform in bone space.
   * \param bone bone id to set the transform for
   * \param rot rotation quaternion
   * \param offset movement offset
   */
  virtual void SetTransformBoneSpace (BoneID bone, const csQuaternion& rot, 
    const csVector3& offset) = 0;

  /**
   * Get the bone transform in skeleton absolute space.
   * \param bone bone id to get the transform for
   * \param rot rotation quaternion
   * \param offset movement offset
   */
  virtual void GetTransformAbsSpace (BoneID bone, csQuaternion& rot, 
    csVector3& offset) const = 0;

  /**
   * Set the bone transform in skeleton absolute space.
   * \param bone bone id to set the transform for
   * \param rot rotation quaternion
   * \param offset movement offset
   */
  virtual void SetTransformAbsSpace (BoneID bone, const csQuaternion& rot, 
    const csVector3& offset) = 0;

  /** @} */

  /**
   * Create a skeleton instance from this skeleton factory.
   */
  virtual csPtr<iSkeleton> CreateSkeleton () = 0;

  /**
   * Get the animation packet associated with this skeleton
   */
  virtual iSkeletonAnimPacketFactory* GetAnimationPacket () const = 0;

  /**
   * Set the animation packet associated with this skeleton
   */
  virtual void SetAnimationPacket (iSkeletonAnimPacketFactory* fact) = 0;

  /**
   * Set whether or not the skeleton should start its animation automatically
   * after its initialization. Default value is true.
   * \remarks The animation will be effectively started at the first frame
   * subsequent to the creation of the CS::Mesh::iAnimatedMesh. You may therefore 
   * still need to start manually the animation nodes, eg if you want to have the
   * rigid bodies created directly by the ragdoll animation node.
   */
  virtual void SetAutoStart (bool autostart) = 0;

  /**
   * Set whether or not the skeleton should start its animation automatically
   * after its initialization.
   */
  virtual bool GetAutoStart () = 0;

  /**
   * Return a textual representation of the bone tree of this skeleton factory
   */
  virtual csString Description () const = 0;
};

/**
 * A skeleton instance defines the state of a CS::Mesh::iAnimatedMesh. It is therefore a specific
 * copy of a skeleton, with the base pose and topology defined by the factory, but with the current
 * state defined internally. 
 * 
 * Skeleton instance adds one coordinate space per bone, the bind space.
 * Bind space is defined by the skeleton factory, so bind space is relative
 * transform compared to the default orientation.
 *
 * \sa CS::Animation::iSkeletonFactory for more information on coordinate spaces
 */
struct iSkeleton : public virtual iBase
{
  SCF_INTERFACE(CS::Animation::iSkeleton, 1, 0, 2);

  /**
   * Get the scene node associated with this skeleton
   */
  virtual iSceneNode* GetSceneNode () = 0;

  /**\name Bone handling
   * @{ */

  /**
   * Get the bone transform in bone space.
   * \param bone bone id to get the transform for
   * \param rot rotation quaternion
   * \param offset movement offset
   */
  virtual void GetTransformBoneSpace (BoneID bone, csQuaternion& rot, 
    csVector3& offset) const = 0;

  /**
   * Set the bone transform in bone space.
   * \param bone bone id to set the transform for
   * \param rot rotation quaternion
   * \param offset movement offset
   */
  virtual void SetTransformBoneSpace (BoneID bone, const csQuaternion& rot, 
    const csVector3& offset) = 0;

  /**
   * Get the bone transform in skeleton absolute space.
   * \param bone bone id to get the transform for
   * \param rot rotation quaternion
   * \param offset movement offset
   */
  virtual void GetTransformAbsSpace (BoneID bone, csQuaternion& rot, 
    csVector3& offset) const = 0;

  /**
   * Set the bone transform in skeleton absolute space.
   * \param bone bone id to set the transform for
   * \param rot rotation quaternion
   * \param offset movement offset
   */
  virtual void SetTransformAbsSpace (BoneID bone, const csQuaternion& rot, 
    const csVector3& offset) = 0;
  
  /**
   * Get the bone transform in bind space.
   * \param bone bone id to get the transform for
   * \param rot rotation quaternion
   * \param offset movement offset
   */
  virtual void GetTransformBindSpace (BoneID bone, csQuaternion& rot, 
    csVector3& offset) const = 0;

    
  /**
   * Set the bone transform in bind space.
   * \param bone bone id to set the transform for
   * \param rot rotation quaternion
   * \param offset movement offset
   */
  virtual void SetTransformBindSpace (BoneID bone, const csQuaternion& rot, 
    const csVector3& offset) = 0;
  
  /**
   * Get the entire skeleton state (all transforms) in absolute space
   */
  virtual csPtr<csSkeletalState> GetStateAbsSpace () = 0;

  /**
   * Get the entire skeleton state (all transforms) in bone space
   */
  virtual csPtr<csSkeletalState> GetStateBoneSpace () = 0;

  /**
   * Get the entire skeleton state (all transforms) in bind space
   */
  virtual csPtr<csSkeletalState> GetStateBindSpace () = 0;

  /** @} */

  /**
   * Get the factory used to create this skeleton instance
   */
  virtual iSkeletonFactory* GetFactory () const = 0;


  /**
   * Get the animation packet associated with this skeleton
   */
  virtual iSkeletonAnimPacket* GetAnimationPacket () const = 0;

  /**
   * Set the animation packet associated with this skeleton
   */
  virtual void SetAnimationPacket (iSkeletonAnimPacket* packet) = 0;


  /**
   * Recreate the skeleton structure from the factory
   */
  virtual void RecreateSkeleton () = 0;

  /**
   * Update the skeleton
   */
  virtual void UpdateSkeleton (float dt) = 0;

  /**
   * Get skeleton update version number
   */
  virtual unsigned int GetSkeletonStateVersion () const = 0;

  /**
   * Set the animated mesh associated with this skeleton
   */
  virtual void SetAnimatedMesh (CS::Mesh::iAnimatedMesh* animesh) = 0;

  /**
   * Get the animated mesh associated with this skeleton
   */
  virtual CS::Mesh::iAnimatedMesh* GetAnimatedMesh () = 0;

  /**
   * Reset the transform of all the bones of the skeleton to the ones of the
   * skeleton factory.
   */
  virtual void ResetSkeletonState () = 0;
};

/**
 * Holds the state of an animesh skeleton for a frame, ie the position
 * and rotation of each bone of the skeleton. These transforms are in
 * bind space.
 */
class csSkeletalState : public csRefCount
{
public:

  /// Constructor
  csSkeletalState ()
    : boneVecs (0), boneQuats (0), numberOfBones (0)
  {}

  ///
  virtual inline ~csSkeletalState ()
  {
    delete[] boneVecs;
    delete[] boneQuats;
  }

  /**
   * Return the position vector of the specified bone, in bone space.
   * \param i The BoneID of the bone.
   */
  inline const csVector3& GetVector (size_t i) const
  {
    return boneVecs[i];
  }

  /**
   * Return the position vector of the specified bone, in bone space.
   * \param i The BoneID of the bone.
   */
  inline csVector3& GetVector (size_t i) 
  {
    return boneVecs[i];
  }


  /**
   * Return the rotation quaternion of the specified bone, in bone space.
   * \param i The BoneID of the bone.
   */
  inline const csQuaternion& GetQuaternion (size_t i) const
  {
    return boneQuats[i];
  }

  /**
   * Return the rotation quaternion of the specified bone, in bone space.
   * \param i The BoneID of the bone.
   */
  inline csQuaternion& GetQuaternion (size_t i) 
  {
    return boneQuats[i];
  }

  /**
   * Return true if the position and rotation values have been set for
   * the specified bone, false otherwise (last position and rotation values
   * which have been set for this bone will therefore be kept).
   */
  inline bool IsBoneUsed (BoneID bone) const
  {
    return bitSet.IsBitSet (bone);
  }

  /**
   * Mark that the position and rotation values have been set for
   * the specified bone. Both position and rotation must therefore be set.
   */
  inline void SetBoneUsed (BoneID bone)
  {
    bitSet.SetBit (bone);
  }

  /**
   * Return the count of bones of the animesh skeleton.
   */
  inline size_t GetBoneCount () const
  {
    return numberOfBones;
  }

  /**
   * Initialize the skeleton state.
   * \param numBones The count of bones of the animesh skeleton.
   */
  inline void Setup (size_t numBones)
  {
    delete[] boneVecs;
    delete[] boneQuats;

    bitSet.SetSize (numBones);
    bitSet.Clear ();
    boneVecs = new csVector3 [numBones];
    boneQuats = new csQuaternion [numBones];
    numberOfBones = numBones;

    for (size_t i = 0; i < numBones; ++i)
      boneVecs[i].Set (0,0,0);
  }

protected:
  csBitArray bitSet;
  csVector3* boneVecs;
  csQuaternion* boneQuats;  
  size_t numberOfBones;
};


} // namespace Animation
} // namespace CS

CS_DEPRECATED_METHOD_MSG("Use CS::Animation::csSkeletalState instead")
typedef CS::Animation::csSkeletalState csSkeletalState2;
CS_DEPRECATED_METHOD_MSG("Use CS::Animation::iSkeleton instead")
typedef CS::Animation::iSkeleton iSkeleton2;
CS_DEPRECATED_METHOD_MSG("Use CS::Animation::iSkeletonFactory instead")
typedef CS::Animation::iSkeletonFactory iSkeletonFactory2;
CS_DEPRECATED_METHOD_MSG("Use CS::Animation::iSkeletonManager instead")
typedef CS::Animation::iSkeletonManager iSkeletonManager2;

/** @} */

/** @} */


#endif // __CS_IMESH_SKELETON2_H__

