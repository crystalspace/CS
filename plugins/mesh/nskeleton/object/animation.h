/*
    Copyright (C) 2006 by Hristo Hristov

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

#ifndef __CS_NSKELETONANIMATION_H__
#define __CS_NSKELETONANIMATION_H__

#include "csutil/csstring.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"
#include "imesh/skelanim.h"

#include "interpolator.h"

namespace Skeleton
{
struct iSkeleton;
}

CS_PLUGIN_NAMESPACE_BEGIN(SkeletonAnimation)
{

using namespace Skeleton::Animation;

class Channel : public scfImplementation1<Channel, iChannel>
{
public:
  Channel ();
  void AddKeyframe (float t, const Keyframe &k);
  Keyframe ReadValue (float i) const;
  void SetID (size_t id);
  size_t GetID () const;
private:
  csHash<Keyframe, float> keyframes;
  size_t ident;
};

class AnimationFactory
  : public scfImplementation1<AnimationFactory, iAnimationFactory>
{
public:
  AnimationFactory ();
  csPtr<iAnimation> CreateAnimation ();
  void SetName (const char* n);
  const char* GetName () const;
  void AddChannel (csRef<iChannel> channel);
  void ReadChannels (float time, Frame &frame) const;
  void SetAnimationLength (float length);
  float GetAnimationLength () const;
private:
  csRefArray<iChannel> channels;
  csString name;
  float animlength;
};

class Animation : public scfImplementation2<Animation, iAnimation, scfFakeInterface<iMixingNode> >
{
public:
  Animation (csRef<iAnimationFactory> fact);
  const char* GetName () const;
  void Tick (float amount);
  void ReadChannels (Frame &frame);
  void SetPlaySpeed (float speed);
  float GetPlaySpeed ();
  void SetPlayCount (int count);
  int GetPlayCount () const;
  float GetAnimationLength () const;
  float GetTimeline () const;
  bool IsActive () const;
private:
  csRef<iAnimationFactory> fact;
  float timeline;
  float playspeed;
  int playcount;
};

class MixingBase
{
public:
  void Tick (float amount);
  size_t AddNode (float weight, csRef<iMixingNode> node);
  void SetWeight (size_t i, float weight);
  bool IsActive () const;
protected:
  void CalculateFramesAndChannels (csArray<Frame> &nodes_frames, csArray<size_t> &channel_ids);
  csArray<float> blend_weights;
private:
  csRefArray<iMixingNode> nodes;
};
class BlendNode
  : public MixingBase, public scfImplementation2<BlendNode, iBlendNode, scfFakeInterface<iMixingNode> >
{
public:
  BlendNode ();
  void ReadChannels (Frame &result_frame);

  // these just delegate to MixingBase base class because C++ is thick
  void Tick (float amount);
  size_t AddNode (float weight, csRef<iMixingNode> node);
  void SetWeight (size_t i, float weight);
  bool IsActive () const;
};

class AccumulateNode
  : public MixingBase, public scfImplementation2<AccumulateNode, iAccumulateNode, scfFakeInterface<iMixingNode> >
{
public:
  AccumulateNode ();
  void ReadChannels (Frame &result_frame);
  size_t AddNode (float weight, csRef<iMixingNode> node);
  void SetWeight (size_t i, float weight);

  void Tick (float amount);
  bool IsActive () const;
};

class AnimationLayer
  : public scfImplementation1<AnimationLayer, iAnimationLayer>
{
public:
  AnimationLayer ();
  csRef<iMixingNode> GetRootMixingNode ();
  void SetRootMixingNode (csRef<iMixingNode> root);
  void UpdateSkeleton (Skeleton::iSkeleton *s, float delta_time);

  csPtr<iBlendNode> CreateBlendNode ();
  csPtr<iAccumulateNode> CreateAccumulateNode ();
private:
  csRef<iMixingNode> mix_node;
};

class AnimationFactoryLayer
  : public scfImplementation1<AnimationFactoryLayer, iAnimationFactoryLayer>
{
public:
  AnimationFactoryLayer ();
  iAnimationFactory* CreateAnimationFactory ();
  csPtr<iChannel> CreateAnimationFactoryChannel ();
  iAnimationFactory* FindAnimationFactoryByName (const char* name);

  void Debug ();
private:
  csRefArray<AnimationFactory> animfacts;
};

}
CS_PLUGIN_NAMESPACE_END(SkeletonAnimation)

#endif // __CS_NSKELETONANIMATION_H__
