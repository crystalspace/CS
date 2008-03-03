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

#ifndef __CS_ANIMATION_H__
#define __CS_ANIMATION_H__


#include "csutil/scf_implementation.h"
#include "imesh/skeleton2.h"
#include "csutil/parray.h"

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{

  class AnimationFactory : 
    public scfImplementation2<AnimationFactory,
                              iSkeletonAnimationFactory2,
                              scfFakeInterface<iSkeletonAnimationNodeFactory2> >
  {
  public:
    AnimationFactory ();

    //-- iSkeletonAnimationFactory2
    virtual ChannelID AddChannel (BoneID bone);

    virtual ChannelID FindChannel (BoneID bone) const;

    virtual void AddKeyFrame (ChannelID channel, float time, 
      const csQuaternion& rotation, const csVector3& offset);

    virtual size_t GetKeyFrameCount (ChannelID channel) const;

    virtual void GetKeyFrame (ChannelID channel, KeyFrameID keyframe, BoneID& bone,
      float& time, csQuaternion& rotation, csVector3& offset);  

    virtual void GetTwoKeyFrames (ChannelID channel, float time, BoneID& bone,
      float& timeBefore, csQuaternion& beforeRot, csVector3& beforeOffset,
      float& timeAfter, csQuaternion& afterRot, csVector3& afterOffset);

    //-- iSkeletonAnimationNodeFactory2
    virtual csPtr<iSkeletonAnimationNode2> CreateInstance (iSkeleton2*);

    //-- "Private"
    void BlendState (csSkeletalState2* state, float baseWeight, float position, bool cycle);
    inline float GetDuration () const
    {
      return duration;
    }

  private:

    struct KeyFrame
    {
      float time;
      csQuaternion rotation;
      csVector3 offset;
    };

    static int KeyFrameCompare (KeyFrame const& k1, KeyFrame const& k2);
    static int KeyFrameTimeCompare (KeyFrame const& k, float const& t);

    struct AnimationChannel
    {
      BoneID bone;

      csArray<KeyFrame> keyFrames;

      AnimationChannel (BoneID bone)
        : bone (bone)
      {}
    };

    csPDelArray<AnimationChannel> channels;

    float duration;
  };


  class Animation : 
    public scfImplementation2<Animation,
                              iSkeletonAnimation2,
                              scfFakeInterface<iSkeletonAnimationNode2> >
  {
  public:
    Animation (AnimationFactory* factory);

    //-- iSkeletonAnimation2
    virtual void PlayOnce (float speed = 1.0f);
    virtual void PlayCyclic (float speed = 1.0f);
    virtual void Stop ();
    virtual void Reset ();
    virtual float GetPlaybackPosition () const;
    virtual void SetPlaybackPosition (float time);

    virtual iSkeletonAnimationFactory2* GetAnimationFactory ();

    //-- iSkeletonAnimationNode2
    virtual void BlendState (csSkeletalState2* state, float baseWeight = 1.0f);
    virtual void TickAnimation (float dt);

    virtual bool IsActive () const;

    virtual iSkeletonAnimationNodeFactory2* GetFactory () const;

  private:
    AnimationFactory* factory;
    
    float playbackPosition, playSpeed;
    bool isPlaying, isPlayingCyclic;
  };
                              

}
CS_PLUGIN_NAMESPACE_END(Skeleton2)

#endif
