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

struct iSkeletonAnimation2;

struct iSkeletonAnimNodeFactory2;
struct iSkeletonAnimNode2;

struct iSkeletonAnimationNodeFactory2;
struct iSkeletonAnimationNode2;

struct iSkeletonBlendNodeFactory2;
struct iSkeletonBlendNode2;

struct iSkeletonPriorityNodeFactory2;
struct iSkeletonPriorityNode2;

struct iSkeletonRandomNodeFactory2;
struct iSkeletonRandomNode2;

struct iSkeletonFSMNodeFactory2;
struct iSkeletonFSMNode2;

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

  /// Identifier for state within FSM node
  typedef unsigned int StateID;

  /// ID for an invalid channel
  static const ChannelID InvalidChannelID = (ChannelID)~0;

  /// ID for an invalid keyframe number
  static const KeyFrameID InvalidKeyframeID = (KeyFrameID)~0;

  /// ID for an invalid state
  static const StateID InvalidStateID = (CS::Animation::StateID)~0;

  /// Different synchronization modes
  enum SynchronizationMode
  {
    /// No syncing at all
    SYNC_NONE,
    /// Synchronize first frame
    SYNC_FIRSTFRAME
  };
}
}

/**
 * Defines a factory for a skeletal animation packet.
 * A packet consists of a number of animations and a hierarchical
 * structure of nodes that defines how those animations are mixed.
 */
struct iSkeletonAnimPacketFactory2 : public virtual iBase
{
  SCF_INTERFACE(iSkeletonAnimPacketFactory2, 2, 0, 0);
  
  /**
   * Create an instance of this animation packet
   */
  virtual csPtr<iSkeletonAnimPacket2> CreateInstance (iSkeleton2* skeleton) = 0;

  /**
   * Create a new animation factory
   */
  virtual iSkeletonAnimation2* CreateAnimation (const char* name) = 0;

  /**
   * Find an already created animation factory
   */
  virtual iSkeletonAnimation2* FindAnimation (const char* name) = 0;

  /**
   * Remove all animation factories
   */
  virtual void ClearAnimations () = 0;

  /**
   * Get animation factory by index
   */
  virtual iSkeletonAnimation2* GetAnimation (size_t index) = 0;

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
   * Create an animation node
   */
  virtual csPtr<iSkeletonAnimationNodeFactory2> CreateAnimationNode (const char* name) = 0;

  /**
   * Create a blend node
   */
  virtual csPtr<iSkeletonBlendNodeFactory2> CreateBlendNode (const char* name) = 0;

  /**
   * Create a priority node
   */
  virtual csPtr<iSkeletonPriorityNodeFactory2> CreatePriorityNode (const char* name) = 0;

  /**
   * Create a random switching node
   */
  virtual csPtr<iSkeletonRandomNodeFactory2> CreateRandomNode (const char* name) = 0;

  /**
   * Create a FSM node
   */
  virtual csPtr<iSkeletonFSMNodeFactory2> CreateFSMNode (const char* name) = 0;
};

/**
 * A animation packet instance
 */
struct iSkeletonAnimPacket2 : public virtual iBase
{
  SCF_INTERFACE(iSkeletonAnimPacket2, 2, 0, 0);

  /**
   * Return the factory from which this packet was created
   */
  virtual iSkeletonAnimPacketFactory2* GetFactory () const = 0;

  /**
   * Get the root node for the animation mixing hierarchy
   */
  virtual iSkeletonAnimNode2* GetAnimationRoot () const = 0;
};

/**
 * Factory for skeletal animations. Defines the key frames but not the current
 * playing state.
 * Each animation is made up of one or more channels, where a channel is a set
 * of key frames associated with a specific bone.
 */
struct iSkeletonAnimation2 : public virtual iBase
{
  SCF_INTERFACE(iSkeletonAnimation2, 2, 0, 0);

  /**
   * Get the animation name
   */
  virtual const char* GetName () const = 0;

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
  virtual void GetKeyFrame (CS::Animation::ChannelID channel, 
    CS::Animation::KeyFrameID keyframe, BoneID& bone,
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

  /**
   * Blend the animation into a skeletal state buffer at a specific playback 
   * position.
   * \param state result state object
   * \param baseWeight base weight for blending
   * \param playbackTime current playback time
   * \param isPlayingCyclic if the playing should be cyclic
   */
  virtual void BlendState (csSkeletalState2* state, 
    float baseWeight, float playbackTime, bool isPlayingCyclic) const = 0;

  /**
   * Get the total duration of the animation
   */
  virtual float GetDuration () const = 0;
};


/**
 * A callback that is called when an animation or animation tree have finished 
 * playing
 */
struct iSkeletonAnimCallback2 : public virtual iBase
{
  /**
   * Function called when an animation node (or all its sub-nodes) finished
   * playing.
   */
  virtual void AnimationFinished (iSkeletonAnimNode2* node) = 0;

  /**
   * Function called when a cyclic animation cycles around
   */
  virtual void AnimationCycled (iSkeletonAnimNode2* node) = 0;

  /**
   * Function called when animation play state changes
   */
  virtual void PlayStateChanged (iSkeletonAnimNode2* node, bool isPlaying) = 0;

  /**
   * Function called when an animation changes duration for any reason.
   */
  virtual void DurationChanged (iSkeletonAnimNode2* node) = 0;
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
   * Start playing the node. 
   * Exactly what this results in depends on the specific node type.
   */
  virtual void Play () = 0;

  /**
   * Stop playing the node (deactivate it).
   */
  virtual void Stop () = 0;

  /**
   * Set the current playback position. If set beyond the end of the
   * animation it will be capped.
   */
  virtual void SetPlaybackPosition (float time) = 0;

  /**
   * Get the current playback position (time).
   */
  virtual float GetPlaybackPosition () const = 0;

  /**
   * Get the length of the node
   */
  virtual float GetDuration () const = 0;

  /**
   * Set the playback speed.
   */
  virtual void SetPlaybackSpeed (float speed) = 0;

  /**
   * Get the playback speed.
   */
  virtual float GetPlaybackSpeed () const = 0;

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
   * Is this or any sub-node active and needs any blending.
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

  /**
   * Add a new animation callback to a node
   * \param callback the callback object
   */
  virtual void AddAnimationCallback (iSkeletonAnimCallback2* callback) = 0;

  /**
   * Remove a animation callback from a node
   * \param callback the callback object
   */
  virtual void RemoveAnimationCallback (iSkeletonAnimCallback2* callback) = 0;
};

/**
 * Factory for animation node
 */
struct iSkeletonAnimationNodeFactory2 : public iSkeletonAnimNodeFactory2
{
  SCF_INTERFACE(iSkeletonAnimationNodeFactory2, 1, 0, 0);

  /**
   * Set the animation containing the raw data to base this node of.
   */
  virtual void SetAnimation (iSkeletonAnimation2* animation) = 0;

  /**
   * Get the animation containing the raw data.
   */
  virtual iSkeletonAnimation2* GetAnimation () const = 0;

  /**
   * Set animation to be playing cyclically (repeating until stopped).
   */
  virtual void SetCyclic (bool cyclic) = 0;

  /**
   * Get if animation is cyclic.
   */
  virtual bool IsCyclic () const = 0;

  /**
   * Set the playback speed.
   */
  virtual void SetPlaybackSpeed (float speed) = 0;

  /**
   * Get the playback speed.
   */
  virtual float GetPlaybackSpeed () const = 0;

  /**
   * Set if animation should automatically reset when stopped and start
   * playing from the beginning.
   */
  virtual void SetAutomaticReset (bool reset) = 0;

  /**
   * Get if animation should automatically reset when stopped and start
   * playing from the beginning. 
   */
  virtual bool GetAutomaticReset () const = 0;

  /**
   * Set if animation should automatically stop when finishing.
   */
  virtual void SetAutomaticStop (bool enabed) = 0;

  /**
   * Get if animation should automatically stop when finishing.
   */
  virtual bool GetAutomaticStop () const = 0;

};

/**
 * Animation node. Takes data from a raw animation and controls the playback
 * of it and feeds it into the blending tree.
 */
struct iSkeletonAnimationNode2 : public iSkeletonAnimNode2
{
  SCF_INTERFACE(iSkeletonAnimationNode2, 1, 0, 0);  
};

/**
 * Factory for blend node
 */
struct iSkeletonBlendNodeFactory2 : public iSkeletonAnimNodeFactory2
{
  SCF_INTERFACE(iSkeletonBlendNodeFactory2, 1, 0, 0);

  /**
   * Add a new sub-node to be blended into the result
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
   * Remove all nodes
   */
  virtual void ClearNodes () = 0;

  /**
   * Set the synchronization mode
   */
  virtual void SetSynchronizationMode (CS::Animation::SynchronizationMode mode) = 0;

  /**
   * Get the current synchronization mode
   */
  virtual CS::Animation::SynchronizationMode GetSynchronizationMode () const = 0;
};


/**
 * An animation node that blends together the sub-nodes based on their
 * weights. The weights does not have to add up to 1, upon update the active
 * animations will be combined so the sum is 1.
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


/**
 * Factory for priority blend node
 */
struct iSkeletonPriorityNodeFactory2 : public iSkeletonAnimNodeFactory2
{
  SCF_INTERFACE(iSkeletonPriorityNodeFactory2, 1, 0, 0);

  /**
   * Add a new sub-node to be blended into the result
   * \param node the node to add
   * \param priority priority to use for the node
   */
  virtual void AddNode (iSkeletonAnimNodeFactory2* node, unsigned int priority) = 0;

  /**
   * Set the priority for a specific node
   */
  virtual void SetNodePriority (uint node, unsigned int priority) = 0;  

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
   * Remove all noodes
   */
  virtual void ClearNodes () = 0;
};

/**
 * An animation node that blends together the sub-nodes based on their priority.
 * A sub-node with a higher priority will always replace a lower priority one, for
 * the bones it animates.
 * This is useful for example when you have a base walk animation and want to 
 * add a secondary motion on top of it
 */
struct iSkeletonPriorityNode2 : public iSkeletonAnimNode2
{
  SCF_INTERFACE(iSkeletonPriorityNode2, 1, 0, 0);

  /**
   * Set the priority for a specific node
   */
  virtual void SetNodePriority (uint node, unsigned int priority) = 0;  
};

/**
 * Factory for randomized sub-node blending node
 */
struct iSkeletonRandomNodeFactory2 : public iSkeletonAnimNodeFactory2
{
  SCF_INTERFACE(iSkeletonRandomNodeFactory2, 1, 0, 0);

  /**
   * Add a new sub-node to be blended into the result
   * \param node the node to add
   * \param probability probability that the node will be selected to be played
   * next when switching.
   */
  virtual void AddNode (iSkeletonAnimNodeFactory2* node, float probability) = 0;

  /**
   * Set the selection probability for a specific node
   */
  virtual void SetNodeProbability (uint node, float weight) = 0;

  /**
   * Set that the node should automatically switch to next one upon completion
   * of the current one.
   */
  virtual void SetAutomaticSwitch (bool automatic) = 0;

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
   * Remove all nodes
   */
  virtual void ClearNodes () = 0;
};


/**
 * An animation node that selects random sub-nodes 
 */
struct iSkeletonRandomNode2 : public iSkeletonAnimNode2
{
  SCF_INTERFACE(iSkeletonRandomNode2, 1, 0, 0);

  /**
   * Switch to next sub-node and, and optionally start it playing    
   */
  virtual void Switch () = 0;

  /**
   * Get the currently selected sub-node
   */
  virtual iSkeletonAnimNode2* GetCurrentNode () const = 0;
};



/**
 * Factory for FSM animation node
 */
struct iSkeletonFSMNodeFactory2 : public iSkeletonAnimNodeFactory2
{
  SCF_INTERFACE(iSkeletonFSMNodeFactory2, 1, 0, 0);

  /**
   * Add a new state to the FSM and return the state identifier
   */
  virtual CS::Animation::StateID AddState () = 0; 

  /**
   * Set the node (sub-tree) associated with a given state.
   * The sub node will be played once the state is switched to.
   */
  virtual void SetStateNode (CS::Animation::StateID id, iSkeletonAnimNodeFactory2* nodeFact) = 0;

  /**
   * Get the node (sub-tree) associated with a given state.
   * The sub node will be played once the state is switched to.
   */
  virtual iSkeletonAnimNodeFactory2* GetStateNode (CS::Animation::StateID id) const = 0;

  /**
   * Set a name for a state (for later access)
   */
  virtual void SetStateName (CS::Animation::StateID id, const char* name) = 0;

  /**
   * Get the name for a state (for later access)
   */
  virtual const char* GetStateName (CS::Animation::StateID id) const = 0;

  /**
   * Find the state with a given name
   */
  virtual CS::Animation::StateID FindState (const char* name) const = 0;

  /**
   * Set the ID of the state to use as first state before switching to any
   * other states.
   */
  virtual void SetStartState (CS::Animation::StateID id) = 0;

  /**
   * Set the ID of the state to use as first state before switching to any
   * other states.
   */
  virtual CS::Animation::StateID GetStartState () const = 0;

  /**
   * Get the number of states in the FSM
   */
  virtual uint GetStateCount () const = 0;

  /**
   * Remove all states.
   */
  virtual void ClearStates () = 0;
};


/**
 * 
 */
struct iSkeletonFSMNode2 : public iSkeletonAnimNode2
{
  SCF_INTERFACE(iSkeletonFSMNode2, 1, 0, 0);

  /**
   * Switch to a new state.
   */
  virtual void SwitchToState (CS::Animation::StateID newState) = 0;

  /**
   * Get the currently playing state id.
   */
  virtual CS::Animation::StateID GetCurrentState () const = 0;
};

/** @} */

/** @} */


#endif // __CS_IMESH_SKELETON2ANIM_H__
