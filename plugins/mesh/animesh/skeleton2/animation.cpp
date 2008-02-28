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

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{




  AnimationFactory::AnimationFactory ()
    : scfImplementationType (this), duration (0)
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
    const csDualQuaternion& key)
  {
    CS_ASSERT(channel < channels.GetSize ());

    AnimationChannel* ch = channels[channel];
    
    KeyFrame k;
    k.time = time;
    k.key = key;

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
    float& time, csDualQuaternion& key)
  {
    CS_ASSERT(channel < channels.GetSize ());

    AnimationChannel* ch = channels[channel];

    CS_ASSERT(keyframe < ch->keyFrames.GetSize ());

    const KeyFrame& k = ch->keyFrames[keyframe];
    bone = ch->bone;
    time = k.time;
    key = k.key;
  }

  void AnimationFactory::GetTwoKeyFrames (ChannelID channel, float time, BoneID& bone,
    float& timeBefore, csDualQuaternion& beforeDQ, float& timeAfter, csDualQuaternion& afterDQ)
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
    beforeDQ = k1.key;
    timeAfter = k2.time;
    afterDQ = k2.key;
  }

  csPtr<iSkeletonAnimationNode2> AnimationFactory::CreateInstance (iSkeleton2*)
  {
    return new Animation (this);
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
      csDualQuaternion& dq = state->GetDualQuaternion (channel->bone);
      dq += k1.key * (t * baseWeight);
      dq += k2.key * ((1-t) * baseWeight);

      state->SetQuatUsed (channel->bone);
    }
  }


  Animation::Animation (AnimationFactory* factory)
    : scfImplementationType (this, factory), factory (factory),
    playbackPosition (0)
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
      }      
    }
  }

  bool Animation::IsActive () const
  {
    return isPlaying;
  }

  iSkeletonAnimationNodeFactory2* Animation::GetFactory () const
  {
    return factory;
  }



}
CS_PLUGIN_NAMESPACE_END(Skeleton2)
