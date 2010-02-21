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

#include "cssysdef.h"
#include "animation.h"

#include "nodes.h"
#include "blendnode.h"
#include "fsmnode.h"
#include "othernodes.h"

#include "csgeom/math.h"

using namespace CS::Animation;

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{
  CS_LEAKGUARD_IMPLEMENT(AnimationPacketFactory);
  
  AnimationPacketFactory::AnimationPacketFactory ()
    : scfImplementationType (this)
  {
  }

  csPtr<iSkeletonAnimPacket2> AnimationPacketFactory::CreateInstance (
    iSkeleton2* skeleton)
  {
    csRef<AnimationPacket> newP;
    newP.AttachNew (new AnimationPacket (this));

    // Setup the root
    newP->animRoot = animRoot->CreateInstance (newP, skeleton);

    return csPtr<iSkeletonAnimPacket2> (newP);
  }

  iSkeletonAnimation2* AnimationPacketFactory::CreateAnimation (
    const char* name)
  {
    csRef<Animation> newFact;
    newFact.AttachNew(new Animation (name));

    animationList.Push (newFact);

    return newFact;
  }

  iSkeletonAnimation2* AnimationPacketFactory::FindAnimation (
    const char* name)
  {
    for (size_t i = 0; i < animationList.GetSize (); ++i)
    {
      if (strcmp (animationList[i]->GetName (), name) == 0)
        return animationList[i];
    }

    return 0;
  }

  void AnimationPacketFactory::ClearAnimations ()
  {
    animationList.DeleteAll ();
  }

  iSkeletonAnimation2* AnimationPacketFactory::GetAnimation (size_t index)
  {
    return animationList[index];
  }

  size_t AnimationPacketFactory::GetAnimationCount () const
  {
    return animationList.GetSize ();
  }

  void AnimationPacketFactory::SetAnimationRoot (iSkeletonAnimNodeFactory2* root)
  {
    animRoot = root;
  }

  iSkeletonAnimNodeFactory2* AnimationPacketFactory::GetAnimationRoot () const
  {
    return animRoot;
  }

  csPtr<iSkeletonAnimationNodeFactory2> AnimationPacketFactory::CreateAnimationNode (
    const char* name)
  {
    csRef<iSkeletonAnimationNodeFactory2> ref;
    ref.AttachNew (new AnimationNodeFactory (name));
    return csPtr<iSkeletonAnimationNodeFactory2> (ref);
  }
  
  csPtr<iSkeletonBlendNodeFactory2> AnimationPacketFactory::CreateBlendNode (
    const char* name)
  {
    csRef<iSkeletonBlendNodeFactory2> ref;
    ref.AttachNew (new BlendNodeFactory (name));
    return csPtr<iSkeletonBlendNodeFactory2> (ref);
  }

  csPtr<iSkeletonPriorityNodeFactory2> AnimationPacketFactory::CreatePriorityNode (
    const char* name)
  {
    csRef<iSkeletonPriorityNodeFactory2> ref;
    ref.AttachNew (new PriorityNodeFactory (name));
    return csPtr<iSkeletonPriorityNodeFactory2> (ref);
  }

  csPtr<iSkeletonRandomNodeFactory2> AnimationPacketFactory::CreateRandomNode (
    const char* name)
  {
    csRef<iSkeletonRandomNodeFactory2> ref;
    ref.AttachNew (new RandomNodeFactory (name));
    return csPtr<iSkeletonRandomNodeFactory2> (ref);
  }

  csPtr<iSkeletonFSMNodeFactory2> AnimationPacketFactory::CreateFSMNode (
    const char* name)
  {
    csRef<iSkeletonFSMNodeFactory2> ref;
    ref.AttachNew (new FSMNodeFactory (name));
    return csPtr<iSkeletonFSMNodeFactory2> (ref);
  }


  CS_LEAKGUARD_IMPLEMENT(AnimationPacket);

  AnimationPacket::AnimationPacket (AnimationPacketFactory* factory)
    : scfImplementationType (this), factory (factory)
  {    
  }
  
  iSkeletonAnimPacketFactory2* AnimationPacket::GetFactory () const
  {
    return factory;
  }

  iSkeletonAnimNode2* AnimationPacket::GetAnimationRoot () const
  {
    return animRoot;
  }
  


  CS_LEAKGUARD_IMPLEMENT(Animation);

  Animation::Animation (const char* name)
    : scfImplementationType (this), name (name), duration (0)
  {
  }

  const char* Animation::GetName () const
  {
    return name;
  }

  ChannelID Animation::AddChannel (BoneID bone)
  {
    // Check if there is any channel with this bone before
    ChannelID channelId = FindChannel (bone);
    
    if (channelId == InvalidChannelID)
    {
      AnimationChannel* channel = new AnimationChannel (bone);      

      channelId = (ChannelID)channels.Push (channel);
    }

    return channelId;
  }

  ChannelID Animation::FindChannel (BoneID bone) const
  {
    for (size_t i = 0; i < channels.GetSize (); ++i)
    {
      if (channels[i]->bone == bone)
        return (ChannelID)i;
    }

    return InvalidChannelID;
  }

  void Animation::AddKeyFrame (ChannelID channel, float time, 
    const csQuaternion& rotation, const csVector3& offset)
  {
    CS_ASSERT(channel < channels.GetSize ());

    AnimationChannel* ch = channels[channel];
    
    KeyFrame k;
    k.time = time;
    k.rotation = rotation;
    k.offset = offset;

    if (time > duration)
      duration = time;

    ch->keyFrames.InsertSorted (k, KeyFrameCompare);
  }

  size_t Animation::GetKeyFrameCount (ChannelID channel) const
  {
    CS_ASSERT(channel < channels.GetSize ());

    return channels[channel]->keyFrames.GetSize ();
  }

  void Animation::GetKeyFrame (ChannelID channel, KeyFrameID keyframe, BoneID& bone,
    float& time, csQuaternion& rotation, csVector3& offset)
  {
    CS_ASSERT(channel < channels.GetSize ());

    AnimationChannel* ch = channels[channel];

    CS_ASSERT(keyframe < ch->keyFrames.GetSize ());

    const KeyFrame& k = ch->keyFrames[keyframe];
    bone = ch->bone;
    time = k.time;
    rotation = k.rotation;
    offset = k.offset;
  }

  void Animation::GetTwoKeyFrames (ChannelID channel, float time, BoneID& bone,
    float& timeBefore, csQuaternion& beforeRot, csVector3& beforeOffset,
    float& timeAfter, csQuaternion& afterRot, csVector3& afterOffset)
  {
    CS_ASSERT(channel < channels.GetSize ());

    AnimationChannel* ch = channels[channel];

    // Find last key with key.time <= time
    size_t before, after;
    csArrayCmp<KeyFrame, float> cmp (time, KeyFrameTimeCompare);
    size_t index = ch->keyFrames.FindSortedKey (cmp, &before);

    // Exact match
    if (index != csArrayItemNotFound)
    {
      before = index;
    }

    after = before+1;
    //@@TODO: Maybe setting for this?
    if (after == ch->keyFrames.GetSize ())
    {
      after = 0;
    }

    const KeyFrame& k1 = ch->keyFrames[before];
    const KeyFrame& k2 = ch->keyFrames[after];

    bone = ch->bone;
    timeBefore = k1.time;
    beforeRot = k1.rotation;
    beforeOffset = k1.offset;

    timeAfter = k2.time;
    afterRot = k2.rotation;
    afterOffset = k2.offset;
  }  

  void Animation::BlendState (csSkeletalState2* state, 
    float baseWeight, float playbackTime, bool isPlayingCyclic) const
  {
    csArrayCmp<KeyFrame, float> cmp (playbackTime, KeyFrameTimeCompare);

    for (size_t c = 0; c < channels.GetSize (); ++c)
    {
      AnimationChannel* channel = channels[c];

      // Find keyframes before/after time
      size_t before, after;

      if (channel->keyFrames.GetSize () == 0)
	continue;

      if (channel->keyFrames.GetSize () == 1)
	before = after = 0;

      else
      {
	size_t ki = channel->keyFrames.FindSortedKey (cmp, &before);

	// First keyframe
	if (ki != csArrayItemNotFound)
	{
	  before = ki;
	}

	if (channel->keyFrames[before].time > playbackTime && before > 0)
	  before--;

	// Second
	after = before + 1;
	if (after == channel->keyFrames.GetSize ())
	{
	  // Handle end-of-frame
	  after = isPlayingCyclic ? 0 : before;
	}
      }

      const KeyFrame& k1 = channel->keyFrames[before];
      const KeyFrame& k2 = channel->keyFrames[after];

      // blending factor
      const float t = before == after ? 0 : (playbackTime - k1.time) / (k2.time - k1.time);

      // Blend together
      csQuaternion& q = state->GetQuaternion (channel->bone);
      csVector3& v = state->GetVector (channel->bone);

      csQuaternion qResult;
      csVector3 vResult;

      if (t <= 0.0f)
      {
        qResult = k1.rotation;
        vResult = k1.offset;
      }
      else if (t >= 1.0f)
      {
        qResult = k2.rotation;
        vResult = k2.offset;
      }
      else
      {
        qResult = k1.rotation.SLerp (k2.rotation, t);
        vResult = csLerp (k1.offset, k2.offset, t);
      }

      q = q.SLerp (qResult, baseWeight);
      v = csLerp (v, vResult, baseWeight);

      state->SetBoneUsed (channel->bone);
    }
  }

  float Animation::GetDuration () const
  {
    return duration;
  }

  int Animation::KeyFrameCompare (KeyFrame const& k1, KeyFrame const& k2)
  {
    return csComparator<float, float>::Compare (k1.time, k2.time);    
  }

  int Animation::KeyFrameTimeCompare (KeyFrame const& k, float const& time)
  {
    return csComparator<float, float>::Compare (k.time, time);
  }

}
CS_PLUGIN_NAMESPACE_END(Skeleton2)
