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

#ifndef __CS_IMESH_SKELETON2ANIM_H__
#define __CS_IMESH_SKELETON2ANIM_H__

#include "csutil/scf_interface.h"
#include "csutil/ref.h"
#include "csutil/refcount.h"
#include "csutil/bitarray.h"
#include "csgeom/dualquaternion.h"
#include "imesh/skeleton2.h"

class csQuaternion;
class csVector3;

/**\file
 * Skeleton2 animation interface files
 */

struct iSkeleton2;

struct iSkeletonAnimPacketFactory2;
struct iSkeletonAnimPacket2;

struct iSkeletonAnimNodeFactory2;
struct iSkeletonAnimNode2;

struct iSkeletonAnimationFactory2;
struct iSkeletonAnimation2;

struct iSkeletonBlendNodeFactory2;
struct iSkeletonBlendNode2;

class csSkeletalState2;


/**\addtogroup meshplugins
 * @{ */

/**\name Skeletal animation
 * @{ */

namespace CS
{
namespace Animation
{
  /// Identifier for channel within animation
  typedef unsigned int ChannelID;

  /// Identifier for keyframes within animation channel
  typedef unsigned int KeyFrameID;

  /// ID for an invalid channel
  static const ChannelID InvalidChannelID = (ChannelID)~0;

  /// ID for an invalid keyframe number
  static const KeyFrameID InvalidKeyframeID = (KeyFrameID)~0;
}
}

/**
 * Defines a factory for a skeletal animation packet.
 * A packet consists of a number of animations and a hierarchical
 * structure of nodes that defines how those animations are mixed.
 */
struct iSkeletonAnimPacketFactory2 : public virtual iBase
{
  SCF_INTERFACE(iSkeletonAnimPacketFactory2, 1, 0, 0);
  
  /**
   * Create an instance of this animation packet
   */
  virtual csPtr<iSkeletonAnimPacket2> CreateInstance (iSkeleton2* skeleton) = 0;

  /**
   * Create a new animation factory
   */
  virtual iSkeletonAnimationFactory2* CreateAnimation (const char* name) = 0;

  /**
   * Find an already created animation factory
   */
  virtual iSkeletonAnimationFactory2* FindAnimation (const char* name) = 0;

  /**
   * Remove all animation factories
   */
  virtual void ClearAnimations () = 0;

  /**
   * Get animation factory by index
   */
  virtual iSkeletonAnimationFactory2* GetAnimation (size_t index) = 0;

  /**
   * Get the number of animation factories
   */
  virtual size_t GetAnimationCount () const = 0;
  
  /**
   * Set the root node for the animation mixing hierarchy
   */
  virtual void SetAnimationRoot (iSkeletonAnimNodeFactory2* root) = 0;

  /**
   * Get the root node for the animation mixing hierarchy
   */
  virtual iSkeletonAnimNodeFactory2* GetAnimationRoot () const = 0;

  /**
   * Create a blend node
   */
  virtual csPtr<iSkeletonBlendNodeFactory2> CreateBlendNode (const char* name) = 0;
};

/**
 * A animation packet instance
 */
struct iSkeletonAnimPacket2 : public virtual iBase
{
  SCF_INTERFACE(iSkeletonAnimPacket2, 1, 0, 0);

  /**
   * Find an animation within the packet
   */
  virtual iSkeletonAnimation2* FindAnimation (const char* name) = 0;

  /**
  * Get animation factory by index
  */
  virtual iSkeletonAnimation2* GetAnimation (size_t index) = 0;

  /**
  * Get the number of animation factories
  */
  virtual size_t GetAnimationCount () const = 0;

  /**
   * Get the root node for the animation mixing hierarchy
   */
  virtual iSkeletonAnimNode2* GetAnimationRoot () const = 0;
};


/**
 * Base type for nodes in the hierarchical blending tree factory
 */
struct iSkeletonAnimNodeFactory2 : public virtual iBase
{
  SCF_INTERFACE(iSkeletonAnimNodeFactory2, 1, 0, 0);

  /**
   * Create the contained node
   */
  virtual csPtr<iSkeletonAnimNode2> CreateInstance (
    iSkeletonAnimPacket2* packet, iSkeleton2* skeleton) = 0;

  /**
   * Get the name used when creating the factory
   */
  virtual const char* GetNodeName () const = 0;

  /**
   * Find a sub-node with given name
   */
  virtual iSkeletonAnimNodeFactory2* FindNode (const char* name) = 0;
};

/**
 * Base type for nodes in the hierarchical blending tree for 
 * skeletal animation system. 
 */
struct iSkeletonAnimNode2 : public virtual iBase
{
  SCF_INTERFACE(iSkeletonAnimNode2, 1, 0, 0);

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
  virtual iSkeletonAnimNodeFactory2* GetFactory () const = 0;

  /**
   * Find a sub-node with given name
   */
  virtual iSkeletonAnimNode2* FindNode (const char* name) = 0;
};

/**
 * Factory for skeletal animations. Defines the key frames but not the current
 * playing state.
 * Each animation is made up of one or more channels, where a channel is a set
 * of key frames associated with a specific bone.
 */
struct iSkeletonAnimationFactory2 : public iSkeletonAnimNodeFactory2
{
  SCF_INTERFACE(iSkeletonAnimationFactory2, 1, 0, 0);

  /**
   * Add a new channel associated with bone.
   * If a channel already exists, it will be returned.
   * \param bone bone id to associate channel with.
   */
  virtual CS::Animation::ChannelID AddChannel (BoneID bone) = 0;

  /**
   * Find the channel associated with a specific bone, if any.
   */
  virtual CS::Animation::ChannelID FindChannel (BoneID bone) const = 0;

  /**
   * Add a key frame at given time within channel.
   * \param channel channel id
   * \param time key frame time
   * \param key key frame data
   */
  virtual void AddKeyFrame (CS::Animation::ChannelID channel, float time, 
    const csQuaternion& rotation, const csVector3& offset) = 0;
  
  /**
   * Get total number of key frames in channel.
   * \param channel channel id
   */
  virtual size_t GetKeyFrameCount (CS::Animation::ChannelID channel) const = 0;

  /**
   * Get a specific key frame within channel.
   * \param channel channel id
   * \param keyframe key frame number to get
   * \param bone bone id associated with channel
   * \param time time associated with key frame
   * \param key key frame data
   */
  virtual void GetKeyFrame (CS::Animation::ChannelID channel, CS::Animation::KeyFrameID keyframe, BoneID& bone,
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
  virtual void GetTwoKeyFrames (CS::Animation::ChannelID channel, float time, BoneID& bone,
    float& timeBefore, csQuaternion& beforeRot, csVector3& beforeOffset,
    float& timeAfter, csQuaternion& afterRot, csVector3& afterOffset) = 0;

};

/**
 * Skeleton animation instance.
 * Defines a currently or possibly playing animation state.
 */
struct iSkeletonAnimation2 : public iSkeletonAnimNode2
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
struct iSkeletonBlendNodeFactory2 : public iSkeletonAnimNodeFactory2
{
  SCF_INTERFACE(iSkeletonBlendNodeFactory2, 1, 0, 0);
  /**
   * Add a new sub node to be blended into the result
   * \param node the node to add
   * \param weight the blend weight to use for this node
   */
  virtual void AddNode (iSkeletonAnimNodeFactory2* node, float weight) = 0;

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
  virtual iSkeletonAnimNodeFactory2* GetNode (uint node) = 0;

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
struct iSkeletonBlendNode2 : public iSkeletonAnimNode2
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
