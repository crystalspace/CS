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
#include "csutil/leakguard.h"
#include "csutil/parray.h"
#include "csutil/refarr.h"
#include "csutil/csstring.h"

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{
  class Animation;


  class AnimationPacketFactory : 
    public scfImplementation1<AnimationPacketFactory,
                              CS::Animation::iSkeletonAnimPacketFactory2>
  {
  public:
    CS_LEAKGUARD_DECLARE(AnimationPacketFactory);
  
    AnimationPacketFactory ();

    //-- CS::Animation::iSkeletonAnimPacketFactory2
    virtual csPtr<CS::Animation::iSkeletonAnimPacket2> CreateInstance (CS::Animation::iSkeleton2* skeleton);

    virtual CS::Animation::iSkeletonAnimation2* CreateAnimation (const char* name);
    virtual CS::Animation::iSkeletonAnimation2* FindAnimation (const char* name);
    virtual void ClearAnimations ();
    virtual CS::Animation::iSkeletonAnimation2* GetAnimation (size_t index);
    virtual size_t GetAnimationCount () const;

    virtual void SetAnimationRoot (CS::Animation::iSkeletonAnimNodeFactory2* root);
    virtual CS::Animation::iSkeletonAnimNodeFactory2* GetAnimationRoot () const;

    virtual csPtr<CS::Animation::iSkeletonAnimationNodeFactory2> CreateAnimationNode (const char* name);
    virtual csPtr<CS::Animation::iSkeletonBlendNodeFactory2> CreateBlendNode (const char* name);
    virtual csPtr<CS::Animation::iSkeletonPriorityNodeFactory2> CreatePriorityNode (const char* name);
    virtual csPtr<CS::Animation::iSkeletonRandomNodeFactory2> CreateRandomNode (const char* name);
    virtual csPtr<CS::Animation::iSkeletonFSMNodeFactory2> CreateFSMNode (const char* name);

  private:
    csRefArray<Animation> animationList;
    csRef<CS::Animation::iSkeletonAnimNodeFactory2> animRoot;
  };

  class AnimationPacket :
    public scfImplementation1<AnimationPacket,
                              CS::Animation::iSkeletonAnimPacket2>
  {
  public:
    CS_LEAKGUARD_DECLARE(AnimationPacket);
  
    AnimationPacket (AnimationPacketFactory* factory);

    //-- CS::Animation::iSkeletonAnimPacket2
    virtual CS::Animation::iSkeletonAnimPacketFactory2* GetFactory () const;
    virtual CS::Animation::iSkeletonAnimNode2* GetAnimationRoot () const;

  private:
    friend class AnimationPacketFactory;
    
    csRef<CS::Animation::iSkeletonAnimNode2> animRoot;
    csRef<AnimationPacketFactory> factory;
  };

  class Animation : 
    public scfImplementation1<Animation,
                              CS::Animation::iSkeletonAnimation2>
  {
  public:
    CS_LEAKGUARD_DECLARE(Animation);
  
    Animation (const char* name);

    //-- CS::Animation::iSkeletonAnimation2
    virtual const char* GetName () const;

    virtual CS::Animation::ChannelID AddChannel (CS::Animation::BoneID bone);

    virtual CS::Animation::ChannelID FindChannel (CS::Animation::BoneID bone) const;

    virtual size_t GetChannelCount () const;

    virtual CS::Animation::BoneID GetChannelBone (CS::Animation::ChannelID channel) const;

    virtual void AddKeyFrame (CS::Animation::ChannelID channel, float time, 
      const csQuaternion& rotation, const csVector3& offset);

    virtual void SetKeyFrame (CS::Animation::ChannelID channel, 
			      CS::Animation::KeyFrameID keyframe,
			      const csQuaternion& rotation, const csVector3& offset);

    virtual size_t GetKeyFrameCount (CS::Animation::ChannelID channel) const;

    virtual void GetKeyFrame (CS::Animation::ChannelID channel, CS::Animation::KeyFrameID keyframe, 
      CS::Animation::BoneID& bone, float& time, csQuaternion& rotation, csVector3& offset);  

    virtual void GetTwoKeyFrames (CS::Animation::ChannelID channel, float time, CS::Animation::BoneID& bone,
      float& timeBefore, csQuaternion& beforeRot, csVector3& beforeOffset,
      float& timeAfter, csQuaternion& afterRot, csVector3& afterOffset);

    virtual void BlendState (CS::Animation::csSkeletalState2* state, 
      float baseWeight, float playbackTime, bool isPlayingCyclic) const;

    virtual float GetDuration () const;

    virtual void SetFramesInBindSpace (bool isBindSpace);

    virtual bool GetFramesInBindSpace () const;

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
      CS::Animation::BoneID bone;

      csArray<KeyFrame> keyFrames;

      AnimationChannel (CS::Animation::BoneID bone)
        : bone (bone)
      {}
    };

    csPDelArray<AnimationChannel> channels;

    float duration;
    bool isBindSpace;
  };     

}
CS_PLUGIN_NAMESPACE_END(Skeleton2)

#endif
