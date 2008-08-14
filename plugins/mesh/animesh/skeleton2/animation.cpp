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

#include "csgeom/math.h"

using namespace CS::Animation;

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{
  AnimationPacketFactory::AnimationPacketFactory ()
    : scfImplementationType (this)
  {
  }

  csPtr<iSkeletonAnimPacket2> AnimationPacketFactory::CreateInstance (
    iSkeleton2* skeleton)
  {
    csRef<AnimationPacket> newP;
    newP.AttachNew (new AnimationPacket (this));

    // Setup all animations
    for (size_t i = 0; i < animFactoryList.GetSize (); ++i)
    {
      csRef<Animation> newAnim;
      newAnim.AttachNew (new Animation (animFactoryList[i]));
      newP->animList.Push (newAnim);
    }

    // Setup the root
    newP->animRoot = animRoot->CreateInstance (newP, skeleton);

    return csPtr<iSkeletonAnimPacket2> (newP);
  }

  iSkeletonAnimationFactory2* AnimationPacketFactory::CreateAnimation (
    const char* name)
  {
    csRef<AnimationFactory> newFact;
    newFact.AttachNew(new AnimationFactory (name));

    animFactoryList.Push (newFact);

    return newFact;
  }

  iSkeletonAnimationFactory2* AnimationPacketFactory::FindAnimation (
    const char* name)
  {
    for (size_t i = 0; i < animFactoryList.GetSize (); ++i)
    {
      if (animFactoryList[i]->GetName () == name)
        return animFactoryList[i];
    }

    return 0;
  }

  void AnimationPacketFactory::ClearAnimations ()
  {
    animFactoryList.DeleteAll ();
  }

  iSkeletonAnimationFactory2* AnimationPacketFactory::GetAnimation (size_t index)
  {
    return animFactoryList[index];
  }

  size_t AnimationPacketFactory::GetAnimationCount () const
  {
    return animFactoryList.GetSize ();
  }

  void AnimationPacketFactory::SetAnimationRoot (iSkeletonAnimNodeFactory2* root)
  {
    animRoot = root;
  }

  iSkeletonAnimNodeFactory2* AnimationPacketFactory::GetAnimationRoot () const
  {
    return animRoot;
  }

  csPtr<iSkeletonBlendNodeFactory2> AnimationPacketFactory::CreateBlendNode (
    const char* name)
  {
    return new BlendNodeFactory (name);
  }


  AnimationPacket::AnimationPacket (AnimationPacketFactory* factory)
    : scfImplementationType (this)
  {    
  }
  
  iSkeletonAnimation2* AnimationPacket::FindAnimation (const char* name)
  {
    for (size_t i = 0; i < animList.GetSize (); ++i)
    {
      if (animList[i]->GetName () == name)
        return animList[i];
    }

    return 0; 
  }

  iSkeletonAnimation2* AnimationPacket::GetAnimation (size_t index)
  {
    return animList[index];
  }

  size_t AnimationPacket::GetAnimationCount () const
  {
    return animList.GetSize ();
  }

  iSkeletonAnimNode2* AnimationPacket::GetAnimationRoot () const
  {
    return animRoot;
  }
  


  AnimationFactory::AnimationFactory (const char* name)
    : scfImplementationType (this), name (name), duration (0)
  {
  }

  ChannelID AnimationFactory::AddChannel (BoneID bone)
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

  ChannelID AnimationFactory::FindChannel (BoneID bone) const
  {
    for (size_t i = 0; i < channels.GetSize (); ++i)
    {
      if (channels[i]->bone == bone)
        return (ChannelID)i;
    }

    return InvalidChannelID;
  }

  void AnimationFactory::AddKeyFrame (ChannelID channel, float time, 
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

  size_t AnimationFactory::GetKeyFrameCount (ChannelID channel) const
  {
    CS_ASSERT(channel < channels.GetSize ());

    return channels[channel]->keyFrames.GetSize ();
  }

  void AnimationFactory::GetKeyFrame (ChannelID channel, KeyFrameID keyframe, BoneID& bone,
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

  void AnimationFactory::GetTwoKeyFrames (ChannelID channel, float time, BoneID& bone,
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

  csPtr<iSkeletonAnimNode2> AnimationFactory::CreateInstance (
    iSkeletonAnimPacket2* packet, iSkeleton2* skeleton)
  {
    iSkeletonAnimNode2* node = packet->FindAnimation (name);
    
    if (node)
    {      
      CS_ASSERT (node->GetFactory () == static_cast<iSkeletonAnimNodeFactory2*> (this));

      // Reuse old node
      return csPtr<iSkeletonAnimNode2> (node);
    }

    return csPtr<iSkeletonAnimNode2> (new Animation (this));
  }

  const char* AnimationFactory::GetNodeName () const
  {
    return GetName();
  }

  iSkeletonAnimNodeFactory2* AnimationFactory::FindNode (const char* name)
  {
    if (name == this->name)
      return this;

    return 0;
  }

  int AnimationFactory::KeyFrameCompare (KeyFrame const& k1, KeyFrame const& k2)
  {
    return csComparator<float, float>::Compare (k1.time, k2.time);    
  }

  int AnimationFactory::KeyFrameTimeCompare (KeyFrame const& k, float const& time)
  {
    return csComparator<float, float>::Compare (k.time, time);
  }

  void AnimationFactory::BlendState (csSkeletalState2* state, float baseWeight, 
    float time, bool cycle)
  {
    csArrayCmp<KeyFrame, float> cmp (time, KeyFrameTimeCompare);

    for (size_t c = 0; c < channels.GetSize (); ++c)
    {
      AnimationChannel* channel = channels[c];

      // Find keyframes before/after time
      size_t before, after;
      size_t ki = channel->keyFrames.FindSortedKey (cmp, &before);

      // First keyframe
      if (ki != csArrayItemNotFound)
      {
        before = ki;
      }

      if (channel->keyFrames[before].time > time && before > 0)
        before--;
      
      // Second
      after = before + 1;
      if (after == channel->keyFrames.GetSize ())
      {
        // Handle end-of-frame
        after = cycle ? after = 0 : after = before;
      }      

      const KeyFrame& k1 = channel->keyFrames[before];
      const KeyFrame& k2 = channel->keyFrames[after];

      // blending factor
      const float t = (time - k1.time) / (k2.time - k1.time);

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


  Animation::Animation (AnimationFactory* factory)
    : scfImplementationType (this, factory), factory (factory),
    playbackPosition (0), playSpeed (1.0f), isPlaying (false), 
    isPlayingCyclic (false)
  {
  }

  void Animation::PlayOnce (float speed)
  {
    if (isPlaying)
      return;

    isPlaying = true;
    isPlayingCyclic = false;
    playSpeed = speed;
  }

  void Animation::PlayCyclic (float speed)
  {
    if (isPlaying)
      return;

    isPlaying = true;
    isPlayingCyclic = true;
    playSpeed = speed;
  }

  void Animation::Stop ()
  {
    isPlaying = false;
    isPlayingCyclic = false;
  }

  void Animation::Reset ()
  {
    playbackPosition = 0;
  }

  float Animation::GetPlaybackPosition () const
  {
    return playbackPosition;
  }

  void Animation::SetPlaybackPosition (float time)
  {
    if (time > factory->GetDuration ())
      time = factory->GetDuration ();

    playbackPosition = time;
  }

  iSkeletonAnimationFactory2* Animation::GetAnimationFactory ()
  {
    return factory;
  }

  void Animation::BlendState (csSkeletalState2* state, float baseWeight)
  {
    factory->BlendState (state, baseWeight, playbackPosition, isPlayingCyclic);
  }

  void Animation::TickAnimation (float dt)
  {
    if (!isPlaying)
      return;

    playbackPosition += dt * playSpeed;

    const float duration = factory->GetDuration ();
    if (playbackPosition > duration)
    {
      if (isPlayingCyclic)
      {
        while (playbackPosition > duration)
        {
          playbackPosition -= duration;
        }
      }
      else
      {
        playbackPosition = duration;
        isPlaying = false;
      }      
    }
  }

  bool Animation::IsActive () const
  {
    return isPlaying;
  }

  iSkeletonAnimNodeFactory2* Animation::GetFactory () const
  {
    return factory;
  }

  iSkeletonAnimNode2* Animation::FindNode (const char* name)
  {
    if (name == factory->GetName ())
      return this;

    return 0;
  }


}
CS_PLUGIN_NAMESPACE_END(Skeleton2)
