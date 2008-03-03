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
#include "csgeom/dualquaternion.h"

class csQuaternion;
class csVector3;

/**\file
 * Skeleton2 interface files
 */

struct iSceneNode;


struct iSkeletonFactory2;
struct iSkeleton2;

struct iSkeletonAnimationNodeFactory2;
struct iSkeletonAnimationNode2;

struct iSkeletonAnimationFactory2;
struct iSkeletonAnimation2;

struct iSkeletonBlendNodeFactory2;
struct iSkeletonBlendNode2;

class csSkeletalState2;


/**\addtogroup meshplugins
 * @{ */

/**\name Skeleton
 * @{ */


/// Identifier for single bone data
typedef unsigned int BoneID;

/// Identifier for channel within animation
typedef unsigned int ChannelID;

/// Identifier for keyframes within animation channel
typedef unsigned int KeyFrameID;

///
static const BoneID InvalidBoneID = (BoneID)~0;

///
static const ChannelID InvalidChannelID = (ChannelID)~0;


/**
 * Skeletal system base object, representing the entire skeletal and
 * skeletal animation system.
 */
struct iSkeletonManager2 : public virtual iBase
{
  SCF_INTERFACE(iSkeletonManager2, 1, 0, 0);

  /**
   * Create a new empty skeleton factory
   */
  virtual iSkeletonFactory2* CreateSkeletonFactory (const char* name) = 0;

  /**
   * Find an already created skeleton factory
   */
  virtual iSkeletonFactory2* FindSkeletonFactory (const char* name) = 0;

  /**
   * Remove all skeleton factories
   */
  virtual void ClearSkeletonFactories () = 0;


  /**
   * Register a named animation hierarchy
   */
  virtual void RegisterAnimationTree (iSkeletonAnimationNodeFactory2* node, const char* name) = 0;

  /**
   * Get an animation hierarchy by name
   */
  virtual iSkeletonAnimationNodeFactory2* FindAnimationTree (const char* name) = 0;

  /**
   * Remove all registered animation hierarchies
   */
  virtual void ClearAnimationTrees () = 0;


  /**
   * Create a new skeletal animation factory
   */
  virtual csPtr<iSkeletonAnimationFactory2> CreateAnimationFactory () = 0;

  /**
   * Create a new blend node factory
   */
  virtual csPtr<iSkeletonBlendNodeFactory2> CreateBlendNodeFactory () = 0;
};

/**
 * A skeleton factory is an object defining the base pose and topology
 * from which skeleton instances can be created.
 *
 * A note on coordinate spaces, cause there are a few to keep track of.
 * Within the skeleton factory there are two types coordinate spaces:
 *
 * - Absolute space. Absolute space is defined by the skeleton ,
 *   that is, the root bone(s) is defined in absolute space.
 * - Bone space. Bone space for every bone is defined by its parent.
 */
struct iSkeletonFactory2 : public virtual iBase
{
  SCF_INTERFACE(iSkeletonFactory2, 1, 0, 0);

  /**\name Bone handling
   * @{ */

  /**
   * Create a new bone with given parent
   * \param parent bone id of parent or ~0 for no parent which creates a top
   * level bone
   */
  virtual BoneID CreateBone (BoneID parent = InvalidBoneID) = 0;

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
  virtual csPtr<iSkeleton2> CreateSkeleton () = 0;

  /**
   * Get the root node of the animation/blend tree
   */
  virtual iSkeletonAnimationNodeFactory2* GetAnimationRoot () const = 0;

  /**
   * Set the root node of the animation/blend tree
   */
  virtual void SetAnimationRoot (iSkeletonAnimationNodeFactory2* fact) = 0;
};

/**
 * A skeleton instance is a specific copy of a skeleton with base pose and
 * topology defined by the factory but current state internally defined. 
 * 
 * Skeleton instance adds one coordinate space per bone, the bind space.
 * Bind space is defined by the skeleton factory, so bind space is relative
 * transform compared to the default orientation.
 *
 * \sa iSkeletonFactory2 for more information on coordinate spaces
 */
struct iSkeleton2 : public virtual iBase
{
  SCF_INTERFACE(iSkeleton2, 1, 0, 0);

  /**
   * Get the scene node associated with the skeleton
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
  virtual csPtr<csSkeletalState2> GetStateAbsSpace () = 0;

  /**
   * Get the entire skeleton state (all transforms) in bone space
   */
  virtual csPtr<csSkeletalState2> GetStateBoneSpace () = 0;

  /**
   * Get the entire skeleton state (all transforms) in bind space
   */
  virtual csPtr<csSkeletalState2> GetStateBindSpace () = 0;

  /** @} */

  /**
   * Get the factory used to create this skeleton instance
   */
  virtual iSkeletonFactory2* GetFactory () const = 0;


  /**
   * Get root node of the blend tree
   */
  virtual iSkeletonAnimationNode2* GetAnimationRoot () const = 0;

  /**
   * Recreate animation tree from the factory
   */
  virtual void RecreateAnimationTree () = 0;

  /**
   * Recreate the skeleton structure from the factory
   */
  virtual void RecreateSkeleton () = 0;

  /**
   * Update the skeleton
   */
  virtual void UpdateSkeleton (float dt) = 0;


};

/**
 * 
 */
class csSkeletalState2 : public csRefCount
{
public:

  ///
  csSkeletalState2 ()
    : boneQuats (0), numberOfBones (0)
  {}

  //
  virtual inline ~csSkeletalState2 ()
  {
    delete[] boneQuats;
  }

  /**
   * 
   */
  inline csDualQuaternion* GetDualQuaternions () const 
  {
    return boneQuats;
  }

  /**
   * 
   */
  inline const csDualQuaternion& GetDualQuaternion (size_t i) const
  {
    return boneQuats[i];
  }

  inline csDualQuaternion& GetDualQuaternion (size_t i) 
  {
    return boneQuats[i];
  }

  /**
   * 
   */
  inline bool IsQuatUsed (BoneID bone) const
  {
    return bitSet.IsBitSet (bone);
  }

  /**
   * 
   */
  inline void SetQuatUsed (BoneID bone)
  {
    bitSet.SetBit (bone);
  }

  /**
   * 
   */
  inline size_t GetBoneCount () const
  {
    return numberOfBones;
  }

  /**
   * 
   */
  inline void Setup (size_t numBones)
  {
    if (boneQuats)
      delete[] boneQuats;

    bitSet.SetSize (numBones);
    boneQuats = new csDualQuaternion [numBones];
    numberOfBones = numberOfBones;
  }

protected:
  csBitArray bitSet;
  csDualQuaternion* boneQuats;  
  size_t numberOfBones;
};



/** @} */

/**\name Skeletal animation
 * @{ */

/**
 * Base type for nodes in the hierarchical blending tree factory
 */
struct iSkeletonAnimationNodeFactory2 : public virtual iBase
{
  SCF_INTERFACE(iSkeletonAnimationNodeFactory2, 1, 0, 0);

  /**
   * Create the contained node
   */
  virtual csPtr<iSkeletonAnimationNode2> CreateInstance (iSkeleton2* skeleton) = 0;
};

/**
 * Base type for nodes in the hierarchical blending tree for 
 * skeletal animation system. 
 */
struct iSkeletonAnimationNode2 : public virtual iBase
{
  SCF_INTERFACE(iSkeletonAnimationNode2, 1, 0, 0);

  /**
   * Blend the state of this node into the global state.
   *
   * \param state The global blend state to blend into
   * \param baseWeight Global weight for this node
   */
  virtual void BlendState (csSkeletalState2* state, float baseWeight = 1.0f) = 0;

  /**
   * Update the animation state
   * \param dt Time since last update
   */
  virtual void TickAnimation (float dt) = 0;

  /**
   * Is this or any subnode active and needs any blending.
   */
  virtual bool IsActive () const = 0;

  /**
   * Get the node factory
   */
  virtual iSkeletonAnimationNodeFactory2* GetFactory () const = 0;
};

/**
 * Factory for skeletal animations. Defines the key frames but not the current
 * playing state.
 * Each animation is made up of one or more channels, where a channel is a set
 * of key frames associated with a specific bone.
 */
struct iSkeletonAnimationFactory2 : public iSkeletonAnimationNodeFactory2
{
  SCF_INTERFACE(iSkeletonAnimationFactory2, 1, 0, 0);

  /**
   * Add a new channel associated with bone.
   * If a channel already exists, it will be returned.
   * \param bone bone id to associate channel with.
   */
  virtual ChannelID AddChannel (BoneID bone) = 0;

  /**
   * Find the channel associated with a specific bone, if any.
   */
  virtual ChannelID FindChannel (BoneID bone) const = 0;

  /**
   * Add a key frame at given time within channel.
   * \param channel channel id
   * \param time key frame time
   * \param key key frame data
   */
  virtual void AddKeyFrame (ChannelID channel, float time, 
    const csQuaternion& rotation, const csVector3& offset) = 0;
  
  /**
   * Get total number of key frames in channel.
   * \param channel channel id
   */
  virtual size_t GetKeyFrameCount (ChannelID channel) const = 0;

  /**
   * Get a specific key frame within channel.
   * \param channel channel id
   * \param keyframe key frame number to get
   * \param bone bone id associated with channel
   * \param time time associated with key frame
   * \param key key frame data
   */
  virtual void GetKeyFrame (ChannelID channel, KeyFrameID keyframe, BoneID& bone,
    float& time, csQuaternion& rotation, csVector3& offset) = 0;  

  /**
   * Get the two key frames on "either side" of time.
   * \param channel channel id
   * \param time time to get key frames for
   * \param bone bone id associated with channel
   * \param timeBefore time associated with key frame before given time
   * \param before key frame data before given time
   * \param timeAfter time associated with key frame after given time
   * \param after key frame data after given time
   */
  virtual void GetTwoKeyFrames (ChannelID channel, float time, BoneID& bone,
    float& timeBefore, csQuaternion& beforeRot, csVector3& beforeOffset,
    float& timeAfter, csQuaternion& afterRot, csVector3& afterOffset) = 0;

};

/**
 * Skeleton animation instance.
 * Defines a currently or possibly playing animation state.
 */
struct iSkeletonAnimation2 : public iSkeletonAnimationNode2
{
  SCF_INTERFACE(iSkeletonAnimation2, 1, 0, 0);

  /**
   * Play the animation once, stop when reaching the end.
   * \param speed Relative playback speed.
   */
  virtual void PlayOnce (float speed = 1.0f) = 0;

  /**
   * Play animation in a continuous cycle until stopped.
   * \param speed Relative playback speed.
   */
  virtual void PlayCyclic (float speed = 1.0f) = 0;

  /**
   * Stop any currently playing animation. 
   * Does not reset playback position back to beginning.
   */
  virtual void Stop () = 0;

  /**
   * Reset playback position.
   */
  virtual void Reset () = 0;

  /**
   * Get the current playback position
   */
  virtual float GetPlaybackPosition () const = 0;


  /**
   * Set the playback position to specific time.
   */
  virtual void SetPlaybackPosition (float time) = 0;

  /**
   * Get the factory from which this instance was created.
   */
  virtual iSkeletonAnimationFactory2* GetAnimationFactory () = 0;
};

/**
 * Factory for blend node
 */
struct iSkeletonBlendNodeFactory2 : public iSkeletonAnimationNodeFactory2
{
  SCF_INTERFACE(iSkeletonBlendNodeFactory2, 1, 0, 0);
  /**
   * Add a new sub node to be blended into the result
   * \param node the node to add
   * \param weight the blend weight to use for this node
   */
  virtual void AddNode (iSkeletonAnimationNodeFactory2* node, float weight) = 0;

  /**
   * Set the blend weight for a specific node
   */
  virtual void SetNodeWeight (uint node, float weight) = 0;

  /**
   * Normalize the node weights so that the sum is 1
   */
  virtual void NormalizeWeights () = 0;

  /**
   * Get specific node
   * \param node node index
   */
  virtual iSkeletonAnimationNodeFactory2* GetNode (uint node) = 0;

  /**
   * Get number of nodes
   */
  virtual uint GetNodeCount () const = 0;

  /**
   * Remove all ndoes
   */
  virtual void ClearNodes () = 0;
};


/**
 * An animation node that blends together the sub nodes.
 */
struct iSkeletonBlendNode2 : public iSkeletonAnimationNode2
{
  SCF_INTERFACE(iSkeletonBlendNode2, 1, 0, 0);

  /**
   * Set the blend weight for a specific node
   */
  virtual void SetNodeWeight (uint node, float weight) = 0;

  /**
   * Normalize the node weights so that the sum is 1
   */
  virtual void NormalizeWeights () = 0;
};


/** @} */

/** @} */


#endif // __CS_IMESH_SKELETON2_H__
