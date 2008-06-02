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
#include "imesh/skeleton2anim.h"
#include "csutil/parray.h"
#include "csutil/refarr.h"
#include "csutil/csstring.h"

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{
  class AnimationFactory;
  class Animation;


  class AnimationPacketFactory : 
    public scfImplementation1<AnimationPacketFactory,
                              iSkeletonAnimPacketFactory2>
  {
  public:
    AnimationPacketFactory ();

    //-- iSkeletonAnimPacketFactory2
    virtual csPtr<iSkeletonAnimPacket2> CreateInstance (iSkeleton2* skeleton);

    virtual iSkeletonAnimationFactory2* CreateAnimation (const char* name);
    virtual iSkeletonAnimationFactory2* FindAnimation (const char* name);
    virtual void ClearAnimations ();
    virtual iSkeletonAnimationFactory2* GetAnimation (size_t index);
    virtual size_t GetAnimationCount () const;

    virtual void SetAnimationRoot (iSkeletonAnimNodeFactory2* root);
    virtual iSkeletonAnimNodeFactory2* GetAnimationRoot () const;

    virtual csPtr<iSkeletonBlendNodeFactory2> CreateBlendNode (const char* name);

  private:
    csRefArray<AnimationFactory> animFactoryList;
    csRef<iSkeletonAnimNodeFactory2> animRoot;
  };

  class AnimationPacket :
    public scfImplementation1<AnimationPacket,
                              iSkeletonAnimPacket2>
  {
  public:
    AnimationPacket (AnimationPacketFactory* factory);

    //-- iSkeletonAnimPacket2
    virtual iSkeletonAnimation2* FindAnimation (const char* name);
    virtual iSkeletonAnimation2* GetAnimation (size_t index);
    virtual size_t GetAnimationCount () const;
    virtual iSkeletonAnimNode2* GetAnimationRoot () const;

  private:
    friend class AnimationPacketFactory;

    csRefArray<Animation> animList;
    csRef<iSkeletonAnimNode2> animRoot;
  };

  class AnimationFactory : 
    public scfImplementation2<AnimationFactory,
                              iSkeletonAnimationFactory2,
                              scfFakeInterface<iSkeletonAnimNodeFactory2> >
  {
  public:
    AnimationFactory (const char* name);

    //-- iSkeletonAnimFactory2
    virtual CS::Animation::ChannelID AddChannel (BoneID bone);

    virtual CS::Animation::ChannelID FindChannel (BoneID bone) const;

    virtual void AddKeyFrame (CS::Animation::ChannelID channel, float time, 
      const csQuaternion& rotation, const csVector3& offset);

    virtual size_t GetKeyFrameCount (CS::Animation::ChannelID channel) const;

    virtual void GetKeyFrame (CS::Animation::ChannelID channel, CS::Animation::KeyFrameID keyframe, 
      BoneID& bone, float& time, csQuaternion& rotation, csVector3& offset);  

    virtual void GetTwoKeyFrames (CS::Animation::ChannelID channel, float time, BoneID& bone,
      float& timeBefore, csQuaternion& beforeRot, csVector3& beforeOffset,
      float& timeAfter, csQuaternion& afterRot, csVector3& afterOffset);

    //-- iSkeletonAnimNodeFactory2
    virtual csPtr<iSkeletonAnimNode2> CreateInstance (
      iSkeletonAnimPacket2* packet, iSkeleton2* skeleton);
    virtual const char* GetNodeName () const;
    virtual iSkeletonAnimNodeFactory2* FindNode (const char* name);

    //-- "Private"
    void BlendState (csSkeletalState2* state, float baseWeight, float position, bool cycle);
    inline float GetDuration () const
    {
      return duration;
    }

    inline const csString& GetName () const
    {
      return name;
    }

  private:
    csString name;

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
                              scfFakeInterface<iSkeletonAnimNode2> >
  {
  public:
    Animation (AnimationFactory* factory);

    //-- iSkeletonAnim2
    virtual void PlayOnce (float speed = 1.0f);
    virtual void PlayCyclic (float speed = 1.0f);
    virtual void Stop ();
    virtual void Reset ();
    virtual float GetPlaybackPosition () const;
    virtual void SetPlaybackPosition (float time);

    virtual iSkeletonAnimationFactory2* GetAnimationFactory ();

    //-- iSkeletonAnimNode2
    virtual void BlendState (csSkeletalState2* state, float baseWeight = 1.0f);
    virtual void TickAnimation (float dt);

    virtual bool IsActive () const;

    virtual iSkeletonAnimNodeFactory2* GetFactory () const;
    virtual iSkeletonAnimNode2* FindNode (const char* name);

    //-- Private
    inline const csString& GetName () const
    {
      return factory->GetName ();
    }

  private:
    AnimationFactory* factory;
    
    float playbackPosition, playSpeed;
    bool isPlaying, isPlayingCyclic;
  };
                              

}
CS_PLUGIN_NAMESPACE_END(Skeleton2)

#endif
