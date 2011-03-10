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

#ifndef __CS_IMESH_ANIMNODE_SKELETON2ANIM_H__
#define __CS_IMESH_ANIMNODE_SKELETON2ANIM_H__

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

namespace CS {
namespace Animation {

struct iSkeleton;

struct iSkeletonAnimPacketFactory;
struct iSkeletonAnimPacket;

struct iSkeletonAnimation;

struct iSkeletonAnimNodeFactory;
struct iSkeletonAnimNode;

struct iSkeletonAnimationNodeFactory;
struct iSkeletonAnimationNode;

struct iSkeletonBlendNodeFactory;
struct iSkeletonBlendNode;

struct iSkeletonPriorityNodeFactory;
struct iSkeletonPriorityNode;

struct iSkeletonRandomNodeFactory;
struct iSkeletonRandomNode;

struct iSkeletonFSMNodeFactory;
struct iSkeletonFSMNode;

class AnimatedMeshState;


/**\addtogroup meshplugins
 * @{ */

/**\name Skeletal animation
 * @{ */

/// Identifier for a channel within an animation
typedef unsigned int ChannelID;

/// Identifier for a keyframe within an animation channel
typedef unsigned int KeyFrameID;

/// Identifier for a state within a FSM animation node
typedef unsigned int StateID;

/// ID for an invalid channel
static const ChannelID InvalidChannelID = (ChannelID)~0;

/// ID for an invalid keyframe number
static const KeyFrameID InvalidKeyframeID = (KeyFrameID)~0;

/// ID for an invalid state
static const StateID InvalidStateID = (StateID)~0;

/// Different synchronization modes
enum SynchronizationMode
{
  /// No syncing at all
  SYNC_NONE,
  /// Synchronize on the first frame
  SYNC_FIRSTFRAME
};


/**
 * Defines a factory for a skeletal animation packet (iSkeletonAnimPacket).
 * A packet consists of a number of animations and a hierarchical
 * structure of nodes that defines how those animations are mixed.
 *
 * Main creators of instances implementing this interface:
 * - CS::Animation::iSkeletonManager::CreateAnimPacketFactory()
 *
 * Main ways to get pointers to this interface:
 * - CS::Animation::iSkeletonManager::FindAnimPacketFactory()
 * - CS::Animation::iSkeletonFactory::GetAnimationPacket()
 * - CS::Animation::iSkeletonAnimPacket::GetFactory()
 *
 * Main users of this interface:
 * - CS::Animation::iSkeletonFactory
 */
struct iSkeletonAnimPacketFactory : public virtual iBase
{
  SCF_INTERFACE(CS::Animation::iSkeletonAnimPacketFactory, 2, 0, 1);
  
  /**
   * Create an instance of this animation packet
   */
  virtual csPtr<iSkeletonAnimPacket> CreateInstance (iSkeleton* skeleton) = 0;

  /**
   * Create a new animation factory
   */
  virtual iSkeletonAnimation* CreateAnimation (const char* name) = 0;

  /**
   * Find an already created animation factory
   */
  virtual iSkeletonAnimation* FindAnimation (const char* name) = 0;

  /**
   * Remove all animation factories
   */
  virtual void ClearAnimations () = 0;

  /**
   * Get animation factory by index
   */
  virtual iSkeletonAnimation* GetAnimation (size_t index) = 0;

  /**
   * Get the number of animation factories
   */
  virtual size_t GetAnimationCount () const = 0;
  
  /**
   * Set the root node for the animation mixing hierarchy
   */
  virtual void SetAnimationRoot (iSkeletonAnimNodeFactory* root) = 0;

  /**
   * Get the root node for the animation mixing hierarchy
   */
  virtual iSkeletonAnimNodeFactory* GetAnimationRoot () const = 0;

  /**
   * Create an animation node
   */
  virtual csPtr<iSkeletonAnimationNodeFactory> CreateAnimationNode (const char* name) = 0;

  /**
   * Create a blend node
   */
  virtual csPtr<iSkeletonBlendNodeFactory> CreateBlendNode (const char* name) = 0;

  /**
   * Create a priority node
   */
  virtual csPtr<iSkeletonPriorityNodeFactory> CreatePriorityNode (const char* name) = 0;

  /**
   * Create a random switching node
   */
  virtual csPtr<iSkeletonRandomNodeFactory> CreateRandomNode (const char* name) = 0;

  /**
   * Create a FSM node
   */
  virtual csPtr<iSkeletonFSMNodeFactory> CreateFSMNode (const char* name) = 0;

  /**
   * Remove the animation of the given name
   */
  virtual void RemoveAnimation (const char* name) = 0;

  /**
   * Remove the animation of the given index
   */
  virtual void RemoveAnimation (size_t index) = 0;
};

/**
 * An animation packet instance. It is defined by a CS::Animation::iSkeletonAnimPacketFactory.
 *
 * Main creators of instances implementing this interface:
 * - CS::Animation::iSkeletonAnimPacketFactory::CreateInstance()
 *
 * Main ways to get pointers to this interface:
 * - CS::Animation::iSkeleton::GetAnimationPacket()
 *
 * Main users of this interface:
 * - CS::Animation::iSkeleton
 */
struct iSkeletonAnimPacket : public virtual iBase
{
  SCF_INTERFACE(CS::Animation::iSkeletonAnimPacket, 2, 0, 0);

  /**
   * Return the factory from which this packet was created
   */
  virtual iSkeletonAnimPacketFactory* GetFactory () const = 0;

  /**
   * Get the root node of the animation blending tree
   */
  virtual iSkeletonAnimNode* GetAnimationRoot () const = 0;
};

/**
 * Data structure for raw skeletal animations. It defines the key frames of the
 * animation but not the current playing state.
 * Each animation is made up of one or more channels, where a channel is a set
 * of key frames associated with a specific bone.
 *
 * Main creators of instances implementing this interface:
 * - CS::Animation::iSkeletonAnimPacketFactory::CreateAnimation()
 *
 * Main ways to get pointers to this interface:
 * - CS::Animation::iSkeletonAnimPacketFactory::FindAnimation()
 * - CS::Animation::iSkeletonAnimPacketFactory::GetAnimation()
 *
 * Main users of this interface:
 * - CS::Animation::iSkeleton
 */
struct iSkeletonAnimation : public virtual iBase
{
  SCF_INTERFACE(CS::Animation::iSkeletonAnimation, 2, 0, 4);

  /**
   * Get the name of the animation.
   */
  virtual const char* GetName () const = 0;

  /**
   * Add a new channel associated with the given bone.
   * If a channel already exists, then it will be returned.
   * \param bone The bone id associated to the channel.
   * \return The channel associated to the bone.
   */
  virtual ChannelID AddChannel (BoneID bone) = 0;

  /**
   * Find the channel associated with a specific bone, if any.
   * \return The associated channel, or CS::Animation::InvalidChannelID if there is none.
   */
  virtual ChannelID FindChannel (BoneID bone) const = 0;

  /**
   * Add a key frame at the given time within the given channel.
   * \param channel Id of the channel.
   * \param time The time of the key frame.
   * \param rotation The rotation of the bone for the key frame.
   * \param offset The position of the bone for the key frame.
   * \remark The rotation and offset must be in the space defined by
   * GetFramesInBoneSpace().
   */
  virtual void AddKeyFrame (ChannelID channel, float time, 
    const csQuaternion& rotation, const csVector3& offset) = 0;
  
  /**
   * Get the total number of key frames in the given channel.
   * \param channel The id of the channel.
   */
  virtual size_t GetKeyFrameCount (ChannelID channel) const = 0;

  /**
   * Get a specific key frame within the given channel.
   * \param channel The id of the channel.
   * \param keyframe The index of the key frame to get.
   * \param bone The id of the bone associated with the channel.
   * \param time The time associated with the key frame.
   * \param rotation The rotation of the bone for the key frame.
   * \param offset The position of the bone for the key frame.
   */
  virtual void GetKeyFrame (ChannelID channel, 
    KeyFrameID keyframe, BoneID& bone,
    float& time, csQuaternion& rotation, csVector3& offset) = 0;  

  /**
   * Get the two key frames on "either side" of the given time.
   * \param channel The id of the channel.
   * \param time The time to get the key frames for.
   * \param bone The id of the bone associated with the channel.
   * \param timeBefore The time associated with the key frame before the given time.
   * \param beforeRot The rotation of the bone for the key frame before the given time.
   * \param beforeOffset The position of the bone for the key frame before the given time.
   * \param timeAfter The time associated with the key frame after the given time
   * \param afterRot The rotation of the bone for the key frame after the given time.
   * \param afterOffset The position of the bone for the key frame after the given time.
   */
  virtual void GetTwoKeyFrames (ChannelID channel, float time, BoneID& bone,
    float& timeBefore, csQuaternion& beforeRot, csVector3& beforeOffset,
    float& timeAfter, csQuaternion& afterRot, csVector3& afterOffset) = 0;

  /**
   * Blend the animation into a skeletal state buffer at a specific playback 
   * position.
   * \param state The skeletal state where the result will be blended.
   * \param baseWeight The base weight to be used for blending.
   * \param playbackTime The current playback time.
   * \param isPlayingCyclic If the playing should be cyclic or not.
   */
  virtual void BlendState (AnimatedMeshState* state, 
    float baseWeight, float playbackTime, bool isPlayingCyclic) const = 0;

  /**
   * Get the total duration of the animation.
   */
  virtual float GetDuration () const = 0;

  /**
   * Reset the rotation and position values of a given key frame.
   * \param channel The id of the channel.
   * \param keyframe The index of the key frame to set.
   * \param rotation The rotation of the bone for the key frame.
   * \param offset The position of the bone for the key frame.
   */
  virtual void SetKeyFrame (ChannelID channel, 
    KeyFrameID keyframe, const csQuaternion& rotation,
    const csVector3& offset) = 0;  

  /**
   * Set whether the data defined with AddKeyFrame() are in bind
   * space or in bone space. By default the data will be in bone space,
   * but it is more efficient to introduce them directly in bind space.
   * \param isBindSpace True if the data are in bind space, false if they
   * are in bone space.
   * \sa ConvertFrameSpace()
   */
  virtual void SetFramesInBindSpace (bool isBindSpace) = 0;

  /**
   * Return whether or not the frames are defined in bind space or in bone
   * space.
   * \return True if the frames are in bind space, false if they are in bone
   * space.
   */
  virtual bool GetFramesInBindSpace () const = 0;

  /**
   * Convert the frames from bone space to bind space if needed. GetFramesInBindSpace() will
   * now return true.
   */
  virtual void ConvertFrameSpace (CS::Animation::iSkeletonFactory* skeleton) = 0;

  /**
   * Get the count of channels in this animation.
   */
  virtual size_t GetChannelCount () const = 0;

  /**
   * Get the id of the bone associated with the given channel.
   */
  virtual BoneID GetChannelBone (ChannelID channel) const = 0;

  /**
   * Set the id of the bone associated with the given channel.
   */
  virtual void SetChannelBone (ChannelID channel, BoneID bone) = 0;

  /**
   * Remove the given channel from this animation.
   * \warning This will alter the ID of the successive channels.
   */
  virtual void RemoveChannel (ChannelID channel) = 0;

  /**
   * Add or reset the rotation at the given time within the given channel.
   * \param channel Id of the channel.
   * \param time The time of the key frame.
   * \param rotation The rotation of the bone for the key frame.
   * \remark The rotation must be in the space defined by
   * GetFramesInBoneSpace().
   */
  virtual void AddOrSetKeyFrame (ChannelID channel, float time, 
    const csQuaternion& rotation) = 0;

  /**
   * Add or reset the position at the given time within the given channel.
   * \param channel Id of the channel.
   * \param time The time of the key frame.
   * \param offset The position of the bone for the key frame.
   * \remark The offset must be in the space defined by
   * GetFramesInBoneSpace().
   */
  virtual void AddOrSetKeyFrame (ChannelID channel, float time, 
    const csVector3& offset) = 0;
};


/**
 * A callback to be implemented if you want to be notified when the state of an
 * animation or animation tree is changed.
 *
 * Main users of this interface:
 * - CS::Animation::iSkeletonAnimNode::AddAnimationCallback()
 */
struct iSkeletonAnimCallback : public virtual iBase
{
  /**
   * Function called when an animation node (or all its sub-nodes) finished
   * playing.
   */
  virtual void AnimationFinished (iSkeletonAnimNode* node) = 0;

  /**
   * Function called when a cyclic animation cycles around
   */
  virtual void AnimationCycled (iSkeletonAnimNode* node) = 0;

  /**
   * Function called when animation play state changes
   */
  virtual void PlayStateChanged (iSkeletonAnimNode* node, bool isPlaying) = 0;

  /**
   * Function called when an animation changes duration for any reason.
   */
  virtual void DurationChanged (iSkeletonAnimNode* node) = 0;
};


/**
 * Base type for nodes in the hierarchical blending tree factory of the
 * skeletal animation system. It is implemented by all types of node
 * factories. It creates instances of CS::Animation::iSkeletonAnimNode.
 *
 * Main ways to get pointers to this interface:
 * - CS::Animation::iSkeletonAnimNode::GetFactory()
 * - CS::Animation::iSkeletonAnimNodeFactory::FindNode()
 *
 * Main users of this interface:
 * - CS::Animation::iSkeletonFactory
 */
struct iSkeletonAnimNodeFactory : public virtual iBase
{
  SCF_INTERFACE(CS::Animation::iSkeletonAnimNodeFactory, 1, 0, 0);

  /**
   * Create a new animation node
   */
  virtual csPtr<iSkeletonAnimNode> CreateInstance (
    iSkeletonAnimPacket* packet, iSkeleton* skeleton) = 0;

  /**
   * Get the name of this factory
   */
  virtual const char* GetNodeName () const = 0;

  /**
   * Find a sub-node with the given name
   */
  virtual iSkeletonAnimNodeFactory* FindNode (const char* name) = 0;
};

/**
 * Base type for the animation nodes in the hierarchical blending tree of the
 * skeletal animation system. This base type is implemented by all types of nodes.
 * It is defined by a CS::Animation::iSkeletonAnimNodeFactory.
 *
 * Main creators of instances implementing this interface:
 * - CS::Animation::iSkeletonAnimNodeFactory::CreateInstance()
 *
 * Main ways to get pointers to this interface:
 * - CS::Animation::iSkeletonAnimNode::FindNode()
 *
 * Main users of this interface:
 * - CS::Animation::iSkeleton
 */
struct iSkeletonAnimNode : public virtual iBase
{
  SCF_INTERFACE(CS::Animation::iSkeletonAnimNode, 1, 0, 0);

  /**
   * Start playing the node, it will therefore start modifying the state of the skeleton.
   */
  virtual void Play () = 0;

  /**
   * Stop playing the node, it will no longer modify the state of the skeleton.
   */
  virtual void Stop () = 0;

  /**
   * Set the current playback position, in seconds. If time is set beyond the end of the
   * animation then it will be capped.
   */
  virtual void SetPlaybackPosition (float time) = 0;

  /**
   * Get the current playback position, in seconds (ie a time value between 0 and GetDuration()).
   */
  virtual float GetPlaybackPosition () const = 0;

  /**
   * Get the time length of this node, in seconds
   */
  virtual float GetDuration () const = 0;

  /**
   * Set the playback speed.
   */
  virtual void SetPlaybackSpeed (float speed) = 0;

  /**
   * Get the playback speed. The default value is 1.0.
   */
  virtual float GetPlaybackSpeed () const = 0;

  /**
   * Blend the state of this node into the global skeleton state.
   *
   * \param state The global blend state to blend into
   * \param baseWeight Global weight for the blending of this node
   */
  virtual void BlendState (AnimatedMeshState* state, float baseWeight = 1.0f) = 0;

  /**
   * Update the state of the animation generated by this node
   * \param dt The time since the last update, in seconds
   */
  virtual void TickAnimation (float dt) = 0;

  /**
   * Return whether or not this node is currently playing and needs any blending.
   */
  virtual bool IsActive () const = 0;

  /**
   * Get the factory of this node
   */
  virtual iSkeletonAnimNodeFactory* GetFactory () const = 0;

  /**
   * Find a sub-node with the given name
   */
  virtual iSkeletonAnimNode* FindNode (const char* name) = 0;

  /**
   * Add a new animation callback to this node
   * \param callback The callback object
   */
  virtual void AddAnimationCallback (iSkeletonAnimCallback* callback) = 0;

  /**
   * Remove the given animation callback from this node
   * \param callback The callback object
   */
  virtual void RemoveAnimationCallback (iSkeletonAnimCallback* callback) = 0;
};

/**
 * Factory for raw animation nodes. It defines instances of CS::Animation::iSkeletonAnimationNode.
 *
 * Main creators of instances implementing this interface:
 * - CS::Animation::iSkeletonAnimPacketFactory::CreateAnimationNode()
 *
 * Main ways to get pointers to this interface:
 * - CS::Animation::iSkeletonAnimationNode::GetFactory()
 * - CS::Animation::iSkeletonAnimNodeFactory::FindNode()
 *
 * Main users of this interface:
 * - CS::Animation::iSkeletonFactory
 */
struct iSkeletonAnimationNodeFactory : public iSkeletonAnimNodeFactory
{
  SCF_INTERFACE(CS::Animation::iSkeletonAnimationNodeFactory, 1, 0, 0);

  /**
   * Set the raw animation data to be used.
   */
  virtual void SetAnimation (iSkeletonAnimation* animation) = 0;

  /**
   * Get the raw animation data in use.
   */
  virtual iSkeletonAnimation* GetAnimation () const = 0;

  /**
   * Set whether or not the animation has to be played cyclically. The default value is false.
   */
  virtual void SetCyclic (bool cyclic) = 0;

  /**
   * Get whether or not the animation has to be played cyclically.
   */
  virtual bool IsCyclic () const = 0;

  /**
   * Set the playback speed of the animation. The default value is 1.0. Negative values are allowed.
   */
  virtual void SetPlaybackSpeed (float speed) = 0;

  /**
   * Get the playback speed of the animation.
   */
  virtual float GetPlaybackSpeed () const = 0;

  /**
   * Set whether or not the playback position should be reset to the start of the animation when
   * this node is activated with CS::Animation::iSkeletonAnimNode::Play(). The default value is false.
   */
  virtual void SetAutomaticReset (bool reset) = 0;

  /**
   * Get whether or not the playback position should be reset to the start of the animation when
   * this node is activated with CS::Animation::iSkeletonAnimNode::Play().
   */
  virtual bool GetAutomaticReset () const = 0;

  /**
   * Set whether or not this animation node should be automatically stopped when the playback
   * duration has been reached. The default value is true.
   */
  virtual void SetAutomaticStop (bool enabed) = 0;

  /**
   * Get whether or not this animation node should be automatically stopped when the playback
   * duration has been reached.
   */
  virtual bool GetAutomaticStop () const = 0;
};

/**
 * Raw animation node. It takes the data from a raw animation, controls the playback
 * of it, and feeds it into the animation blending tree. It is defined by a
 * CS::Animation::iSkeletonAnimationNodeFactory.
 *
 * Main creators of instances implementing this interface:
 * - CS::Animation::iSkeletonAnimationNodeFactory::CreateInstance()
 *
 * Main ways to get pointers to this interface:
 * - CS::Animation::iSkeletonAnimNode::FindNode()
 *
 * Main users of this interface:
 * - CS::Animation::iSkeleton
 */
struct iSkeletonAnimationNode : public iSkeletonAnimNode
{
  SCF_INTERFACE(CS::Animation::iSkeletonAnimationNode, 1, 0, 0);  
};

/**
 * Factory for blend nodes, ie nodes which blend together any number of sub-nodes.
 * It defines instances of CS::Animation::iSkeletonBlendNode.
 *
 * Main creators of instances implementing this interface:
 * - CS::Animation::iSkeletonAnimPacketFactory::CreateBlendNode()
 *
 * Main ways to get pointers to this interface:
 * - CS::Animation::iSkeletonBlendNode::GetFactory()
 * - CS::Animation::iSkeletonAnimNodeFactory::FindNode()
 *
 * Main users of this interface:
 * - CS::Animation::iSkeletonFactory
 */
struct iSkeletonBlendNodeFactory : public iSkeletonAnimNodeFactory
{
  SCF_INTERFACE(CS::Animation::iSkeletonBlendNodeFactory, 1, 0, 0);

  /**
   * Add a new sub-node to be blended into the result
   * \param node the sub-node to add
   * \param weight the blend weight to use for this node
   */
  virtual void AddNode (iSkeletonAnimNodeFactory* node, float weight) = 0;

  /**
   * Set the blend weight for a specific sub-node. The weight can be of any
   * arbitrary scale, this is the proportion between all sub-nodes that matters.
   */
  virtual void SetNodeWeight (uint node, float weight) = 0;

  /**
   * Normalize the sub-node weights so that the sum is 1. Calling this is not
   * mandatory.
   */
  virtual void NormalizeWeights () = 0;

  /**
   * Get the sub-node at the given index
   * \param node node index
   */
  virtual iSkeletonAnimNodeFactory* GetNode (uint node) = 0;

  /**
   * Get number of sub-nodes
   */
  virtual uint GetNodeCount () const = 0;

  /**
   * Remove all sub-nodes
   */
  virtual void ClearNodes () = 0;

  /**
   * Set the synchronization mode
   */
  virtual void SetSynchronizationMode (SynchronizationMode mode) = 0;

  /**
   * Get the current synchronization mode
   */
  virtual SynchronizationMode GetSynchronizationMode () const = 0;
};


/**
 * An animation node that blends together the sub-nodes based on their
 * weights. It is defined by a CS::Animation::iSkeletonBlendNodeFactory.
 *
 * The weights does not have to add up to 1, upon update the active
 * animations will be combined so that the sum is 1.
 *
 * Main creators of instances implementing this interface:
 * - CS::Animation::iSkeletonBlendNodeFactory::CreateInstance()
 *
 * Main ways to get pointers to this interface:
 * - CS::Animation::iSkeletonAnimNode::FindNode()
 *
 * Main users of this interface:
 * - CS::Animation::iSkeleton
 */
struct iSkeletonBlendNode : public iSkeletonAnimNode
{
  SCF_INTERFACE(CS::Animation::iSkeletonBlendNode, 1, 0, 0);

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
 * Factory for priority blend nodes, ie nodes blending sub-nodes on
 * the base of their current priority.
 * It defines instances of CS::Animation::iSkeletonPriorityNode.
 *
 * Main creators of instances implementing this interface:
 * - CS::Animation::iSkeletonAnimPacketFactory::CreatePriorityNode()
 *
 * Main ways to get pointers to this interface:
 * - CS::Animation::iSkeletonPriorityNode::GetFactory()
 * - CS::Animation::iSkeletonAnimNodeFactory::FindNode()
 *
 * Main users of this interface:
 * - CS::Animation::iSkeletonFactory
 */
struct iSkeletonPriorityNodeFactory : public iSkeletonAnimNodeFactory
{
  SCF_INTERFACE(CS::Animation::iSkeletonPriorityNodeFactory, 1, 0, 0);

  /**
   * Add a new sub-node to be blended into the result
   * \param node the node to add
   * \param priority priority to use for the node
   */
  virtual void AddNode (iSkeletonAnimNodeFactory* node, unsigned int priority) = 0;

  /**
   * Set the initial priority of a specific sub-node
   */
  virtual void SetNodePriority (uint node, unsigned int priority) = 0;  

  /**
   * Get the sub-node at the given index
   * \param node node index
   */
  virtual iSkeletonAnimNodeFactory* GetNode (uint node) = 0;

  /**
   * Get number of sub-nodes
   */
  virtual uint GetNodeCount () const = 0;

  /**
   * Remove all sub-nodes
   */
  virtual void ClearNodes () = 0;
};

/**
 * An animation node that blends together the sub-nodes based on their priority.
 * It is defined by a CS::Animation::iSkeletonPriorityNodeFactory.
 *
 * A sub-node with a higher priority will always replace a lower priority one, for
 * the bones it animates.
 * This is useful for example when you have a base walk animation and want to 
 * add a secondary motion on top of it
 *
 * Main creators of instances implementing this interface:
 * - CS::Animation::iSkeletonPriorityNodeFactory::CreateInstance()
 *
 * Main ways to get pointers to this interface:
 * - CS::Animation::iSkeletonAnimNode::FindNode()
 *
 * Main users of this interface:
 * - CS::Animation::iSkeleton
 */
struct iSkeletonPriorityNode : public iSkeletonAnimNode
{
  SCF_INTERFACE(CS::Animation::iSkeletonPriorityNode, 1, 0, 0);

  /**
   * Set the priority for a specific sub-node
   */
  virtual void SetNodePriority (uint node, unsigned int priority) = 0;  
};

/**
 * Factory for blending nodes playing randomly their sub-nodes.
 * It defines instances of CS::Animation::iSkeletonRandomNode.
 *
 * Main creators of instances implementing this interface:
 * - CS::Animation::iSkeletonAnimPacketFactory::CreateRandomNode()
 *
 * Main ways to get pointers to this interface:
 * - CS::Animation::iSkeletonRandomNode::GetFactory()
 * - CS::Animation::iSkeletonAnimNodeFactory::FindNode()
 *
 * Main users of this interface:
 * - CS::Animation::iSkeletonFactory
 */
struct iSkeletonRandomNodeFactory : public iSkeletonAnimNodeFactory
{
  SCF_INTERFACE(CS::Animation::iSkeletonRandomNodeFactory, 1, 0, 0);

  /**
   * Add a new sub-node to be played randomly.
   * \param node the node to add
   * \param probability probability that the node will be selected to be played
   * next when switching. The probability can be of any arbitrary scale, this is
   * the proportion between the probabilities of all nodes that matters.
   *
   * \warning If you use sub-nodes of type CS::Animation::iSkeletonAnimationNodeFactory, you must ensure
   * to set CS::Animation::iSkeletonAnimationNodeFactory::SetAutomaticReset() and
   * SetAutomaticReset::SetAutomaticStop(), otherwise the sub-nodes won't restart playing
   * once they are selected again. Take also care to not use
   * CS::Animation::iSkeletonAnimationNodeFactory::SetCyclic() otherwise this node will get in a deadlock.
   */
  virtual void AddNode (iSkeletonAnimNodeFactory* node, float probability) = 0;

  /**
   * Set the selection probability for a specific node. The probability can be of
   * any arbitrary scale, this is the proportion between the probabilities of all
   * nodes that matters.
   */
  virtual void SetNodeProbability (uint node, float weight) = 0;

  /**
   * Set that the node should automatically switch to next one upon completion
   * of the current one, otherwise it will stop after the first sub-node has
   * been played. Default value is 'true'.
   */
  virtual void SetAutomaticSwitch (bool automatic) = 0;

  /**
   * Get the sub-node at the given index.
   * \param node node index
   */
  virtual iSkeletonAnimNodeFactory* GetNode (uint node) = 0;

  /**
   * Get the number of sub-nodes
   */
  virtual uint GetNodeCount () const = 0;

  /**
   * Remove all sub-nodes
   */
  virtual void ClearNodes () = 0;
};


/**
 * An animation node that selects randomly the sub-nodes to be played.
 * It is defined by a CS::Animation::iSkeletonRandomNodeFactory.
 *
 * Main creators of instances implementing this interface:
 * - CS::Animation::iSkeletonRandomNodeFactory::CreateInstance()
 *
 * Main ways to get pointers to this interface:
 * - CS::Animation::iSkeletonAnimNode::FindNode()
 *
 * Main users of this interface:
 * - CS::Animation::iSkeleton
 */
struct iSkeletonRandomNode : public iSkeletonAnimNode
{
  SCF_INTERFACE(CS::Animation::iSkeletonRandomNode, 1, 0, 0);

  /**
   * Switch to next sub-node. This next node may not be played if
   * CS::Animation::iSkeletonRandomNodeFactory::SetAutomaticSwitch() is not set.
   */
  virtual void Switch () = 0;

  /**
   * Get the sub-node which is currently selected.
   */
  virtual iSkeletonAnimNode* GetCurrentNode () const = 0;
};

/**
 * Factory for Finite State Machine (FSM) animation nodes.
 * It defines instances of CS::Animation::iSkeletonFSMNode.
 *
 * Each state of the FSM corresponds to an animation sub-node. A crossfade
 * and a transition sub-node can also be defined between the states of the FSM.
 * They will be used when the FSM is switched between the two states. Automatic
 * transitions can also be defined, they will be followed automatically at the
 * end of the current playing state if no other state has been asked by the user
 * to switch to.
 *
 * Main creators of instances implementing this interface:
 * - CS::Animation::iSkeletonAnimPacketFactory::CreateFSMNode()
 *
 * Main ways to get pointers to this interface:
 * - CS::Animation::iSkeletonFSMNode::GetFactory()
 * - CS::Animation::iSkeletonAnimNodeFactory::FindNode()
 *
 * Main users of this interface:
 * - CS::Animation::iSkeletonFactory
 */
struct iSkeletonFSMNodeFactory : public iSkeletonAnimNodeFactory
{
  SCF_INTERFACE(CS::Animation::iSkeletonFSMNodeFactory, 1, 0, 2);

  /**
   * Add a new state to the FSM and return the state identifier
   */
  virtual StateID AddState () = 0; 

  /**
   * Set the node (sub-tree) associated with a given state.
   * The sub-node will be played once the state is switched to.
   */
  virtual void SetStateNode (StateID id, iSkeletonAnimNodeFactory* nodeFact) = 0;

  /**
   * Get the node (sub-tree) associated with a given state.
   * The sub node will be played once the state is switched to.
   */
  virtual iSkeletonAnimNodeFactory* GetStateNode (StateID id) const = 0;

  /**
   * Set a name for a state (for easier later access)
   */
  virtual void SetStateName (StateID id, const char* name) = 0;

  /**
   * Get the name for a state (for easier later access)
   */
  virtual const char* GetStateName (StateID id) const = 0;

  /**
   * Find the state of the given name
   */
  virtual StateID FindState (const char* name) const = 0;

  /**
   * Set the ID of the state to use as first state before switching to any
   * other states.
   */
  virtual void SetStartState (StateID id) = 0;

  /**
   * Get the ID of the state to use as first state before switching to any
   * other states.
   */
  virtual StateID GetStartState () const = 0;

  /**
   * Get the number of states in the FSM
   */
  virtual uint GetStateCount () const = 0;

  /**
   * Remove all states.
   */
  virtual void ClearStates () = 0;

  /**
   * Set a node (sub-tree) to use when transitioning between two states.
   * The sub-tree should not be cyclic or a deadlock of the FSM will happen.
   *
   * \param fromState the originating state in the transition
   * \param toState the target state in the transition
   * \param fact node factory to use for the transition animation
   */
  virtual void SetStateTransition (StateID fromState, 
    StateID toState, iSkeletonAnimNodeFactory* fact) = 0;

  /**
   * Set the transition cross-fade times.
   * 
   * \param fromState the originating state in the transition
   * \param toState the target state in the transition
   * \param time1 first cross-fade time, before transition animation (if any)
   * \param time2 second cross-fade time, after transition animation if any is 
   * in use (otherwise ignored)
   */
  virtual void SetTransitionCrossfade (StateID fromState, 
    StateID toState, float time1, float time2) = 0;

  /**
   * Add a new state to the FSM and return the state identifier
   */
  virtual StateID AddState (const char* name,
    iSkeletonAnimNodeFactory *nodeFact) = 0; 

  /**
   * The transition between the two states will be followed automatically
   * if there are no other target state to switch to when it is the end of
   * the animation played by the state with ID fromState. If there are more than
   * one automatic transition defined from this state, then no transition will be
   * followed.
   */
  virtual void SetAutomaticTransition (StateID fromState, 
				       StateID toState,
				       bool automatic) = 0;
};


/**
 * An animation node that uses a Finite State Machine (FSM) to determine the 
 * animation to be played.
 * It is defined by a CS::Animation::iSkeletonFSMNodeFactory.
 *
 * Main creators of instances implementing this interface:
 * - CS::Animation::iSkeletonFSMNodeFactory::CreateInstance()
 *
 * Main ways to get pointers to this interface:
 * - CS::Animation::iSkeletonAnimNode::FindNode()
 *
 * Main users of this interface:
 * - CS::Animation::iSkeleton
 */
struct iSkeletonFSMNode : public iSkeletonAnimNode
{
  SCF_INTERFACE(CS::Animation::iSkeletonFSMNode, 1, 0, 1);

  /**
   * Switch to a new state. If there are some crossfade or transition sub-node
   * defined for the transition between the current state and the new state, then
   * they will be used. If no successive call to SwitchToState() is made by the user,
   * then the FSM will follow any automatic transitions from the new state at the
   * end of its animation.
   *
   * Note that you can switch with this method from any state to any other state
   * without the need to have defined a transition in the CS::Animation::iSkeletonFSMNodeFactory.
   */
  virtual void SwitchToState (StateID newState) = 0;

  /**
   * Get the ID of the state currently playing.
   */
  virtual StateID GetCurrentState () const = 0;

  /**
   * Get the animation node of the given state.
   */
  virtual iSkeletonAnimNode* GetStateNode (StateID state) const = 0;
};

/**
 * Template for an animation node manager.
 * \a FactoryInterface is the interface type for the node factory.
 * Usage example:
 * \code
 * struct iSkeletonFooNodeManager :
 *   public CS::Animation::iSkeletonAnimNodeManager<iSkeletonFooNodeFactory>
 * {
 *   SCF_ISKELETONANIMNODEMANAGER_INTERFACE(iSkeletonFooNodeManager, 1, 0, 0);
 * };
 * \endcode
 */
/* Implementation note: bump number before '*10' in
 * SCF_ISKELETONANIMNODEMANAGER_INTERFACE when changing this interface. */
template <class FactoryInterface>
struct iSkeletonAnimNodeManager : public virtual iBase
{
  typedef FactoryInterface FactoryInterfaceType;

  /// Create an animation node factory with the given name
  virtual FactoryInterface* CreateAnimNodeFactory (const char* name) = 0;

  /// Find the animation node factory with the given name
  virtual FactoryInterface* FindAnimNodeFactory (const char* name) = 0;

  /**
   * Remove the animation node factory of the given name. It will no longer
   * hold any reference to this factory.
   */
  virtual void RemoveAnimNodeFactory (const char* name) = 0;

  /// Remove all animation node factories
  virtual void ClearAnimNodeFactories () = 0;
};

#define SCF_ISKELETONANIMNODEMANAGER_INTERFACE(Name, a, b, c)	\
  SCF_INTERFACE (Name, ((0*10)+(a)), (b), (c))


} // namespace Animation
} // namespace CS

/** @} */

/** @} */

CS_DEPRECATED_METHOD_MSG("Use CS::Animation::iSkeletonAnimation instead")
typedef CS::Animation::iSkeletonAnimation iSkeletonAnimation2;
CS_DEPRECATED_METHOD_MSG("Use CS::Animation::iSkeletonAnimationNode instead")
typedef CS::Animation::iSkeletonAnimationNode iSkeletonAnimationNode2;
CS_DEPRECATED_METHOD_MSG("Use CS::Animation::iSkeletonAnimationNodeFactory instead")
typedef CS::Animation::iSkeletonAnimationNodeFactory iSkeletonAnimationNodeFactory2;
CS_DEPRECATED_METHOD_MSG("Use CS::Animation::iSkeletonAnimCallback instead")
typedef CS::Animation::iSkeletonAnimCallback iSkeletonAnimCallback2;
CS_DEPRECATED_METHOD_MSG("Use CS::Animation::iSkeletonAnimNode instead")
typedef CS::Animation::iSkeletonAnimNode iSkeletonAnimNode2;
CS_DEPRECATED_METHOD_MSG("Use CS::Animation::iSkeletonAnimNodeFactory instead")
typedef CS::Animation::iSkeletonAnimNodeFactory iSkeletonAnimNodeFactory2;
CS_DEPRECATED_METHOD_MSG("Use CS::Animation::iSkeletonAnimPacket instead")
typedef CS::Animation::iSkeletonAnimPacket iSkeletonAnimPacket2;
CS_DEPRECATED_METHOD_MSG("Use CS::Animation::iSkeletonAnimPacketFactory instead")
typedef CS::Animation::iSkeletonAnimPacketFactory iSkeletonAnimPacketFactory2;
CS_DEPRECATED_METHOD_MSG("Use CS::Animation::iSkeletonBlendNode instead")
typedef CS::Animation::iSkeletonBlendNode iSkeletonBlendNode2;
CS_DEPRECATED_METHOD_MSG("Use CS::Animation::iSkeletonBlendNodeFactory instead")
typedef CS::Animation::iSkeletonBlendNodeFactory iSkeletonBlendNodeFactory2;
CS_DEPRECATED_METHOD_MSG("Use CS::Animation::iSkeletonFSMNode instead")
typedef CS::Animation::iSkeletonFSMNode iSkeletonFSMNode2;
CS_DEPRECATED_METHOD_MSG("Use CS::Animation::iSkeletonFSMNodeFactory instead")
typedef CS::Animation::iSkeletonFSMNodeFactory iSkeletonFSMNodeFactory2;
CS_DEPRECATED_METHOD_MSG("Use CS::Animation::iSkeletonPriorityNode instead")
typedef CS::Animation::iSkeletonPriorityNode iSkeletonPriorityNode2;
CS_DEPRECATED_METHOD_MSG("Use CS::Animation::iSkeletonPriorityNodeFactory instead")
typedef CS::Animation::iSkeletonPriorityNodeFactory iSkeletonPriorityNodeFactory2;
CS_DEPRECATED_METHOD_MSG("Use CS::Animation::iSkeletonRandomNode instead")
typedef CS::Animation::iSkeletonRandomNode iSkeletonRandomNode2;
CS_DEPRECATED_METHOD_MSG("Use CS::Animation::iSkeletonRandomNodeFactory instead")
typedef CS::Animation::iSkeletonRandomNodeFactory iSkeletonRandomNodeFactory2;

#endif // __CS_IMESH_ANIMNODE_SKELETON2ANIM_H__
