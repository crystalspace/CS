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
    : scfImplementationType (this)
  {
  }

  ChannelID AnimationFactory::AddChannel (BoneID bone)
  {
    return 0;
  }

  void AnimationFactory::AddKeyFrame (ChannelID channel, float time, 
    const csDualQuaternion& key)
  {
  }

  unsigned int AnimationFactory::GetKeyFrameCount (ChannelID channel) const
  {
    return 0;
  }

  void AnimationFactory::GetKeyFrame (ChannelID channel, KeyFrameID keyframe, BoneID& bone,
    float& time, csDualQuaternion& key)
  {
  }

  void AnimationFactory::GetTwoKeyFrames (ChannelID channel, float time, BoneID& bone,
    float& timeBefore, csDualQuaternion& before, 
    float& timeAfter, csDualQuaternion& after)
  {
  }

  csPtr<iSkeletonAnimationNode2> AnimationFactory::CreateInstance ()
  {
    return 0;
  }




  Animation::Animation ()
    : scfImplementationType (this)
  {
  }

  void Animation::PlayOnce (float speed)
  {
  }

  void Animation::PlayCyclic (float speed)
  {
  }

  void Animation::Stop ()
  {
  }

  void Animation::Reset ()
  {
  }

  float Animation::GetPlaybackPosition () const
  {
    return 0;
  }

  void Animation::SetPlaybackPosition (float time)
  {
  }

  iSkeletonAnimationFactory2* Animation::GetAnimationFactory ()
  {
    return 0;
  }

  void Animation::BlendState (csSkeletalState2* state, float baseWeight)
  {
  }

  void Animation::TickAnimation (float dt)
  {
  }

  bool Animation::IsActive () const
  {
    return false;
  }

  iSkeletonAnimationNodeFactory2* Animation::GetFactory () const
  {
    return 0;
  }



}
CS_PLUGIN_NAMESPACE_END(Skeleton2)
