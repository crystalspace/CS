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

#include "csgeom/math.h"

#include "nodes.h"

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{
  CS_LEAKGUARD_IMPLEMENT(BaseNodeSingle);

  CS_LEAKGUARD_IMPLEMENT(BaseFactoryChildren);
  CS_LEAKGUARD_IMPLEMENT(BaseNodeChildren);

  void BaseNodeChildren::AddAnimationCallback (iSkeletonAnimCallback2* callback)
  {
    // First CB, install sub-callback
    InstallInnerCb (false);

    BaseNodeSingle::AddAnimationCallback (callback);
  }

  void BaseNodeChildren::RemoveAnimationCallback (iSkeletonAnimCallback2* callback)
  {
    BaseNodeSingle::RemoveAnimationCallback (callback);

    // Last CB, remove sub-callback
    RemoveInnerCb (false);    
  }

  void BaseNodeChildren::InstallInnerCb (bool manual)
  {
    if (!cb)
    {
      cb.AttachNew (new InnerCallback (this));

      for (size_t i = 0; i < subNodes.GetSize (); ++i)
      {
        subNodes[i]->AddAnimationCallback (cb);
      }
    }
    manualCbInstall |= manual;
  }

  void BaseNodeChildren::RemoveInnerCb (bool manual)
  {
    if (callbacks.GetSize () == 0 && manual == manualCbInstall)
    {
      for (size_t i = 0; i < subNodes.GetSize (); ++i)
      {
        subNodes[i]->RemoveAnimationCallback (cb);
      }

      cb = 0;
    }
  }

  void BaseNodeChildren::AnimationFinished (iSkeletonAnimNode2* node)
  {
    // If all subnodes are inactive, call the upper CBs
    for (size_t i = 0; i < subNodes.GetSize (); ++i)
    {
      if (subNodes[i]->IsActive ())
        return;
    }

    BaseNodeSingle::FireAnimationFinishedCb ();
  }

  void BaseNodeChildren::AnimationCycled (iSkeletonAnimNode2* node)
  {
    BaseNodeSingle::FireAnimationCycleCb ();
  }

  void BaseNodeChildren::PlayStateChanged (iSkeletonAnimNode2* node, bool isPlaying)
  {}

  void BaseNodeChildren::DurationChanged (iSkeletonAnimNode2* node)
  {}

  BaseNodeChildren::InnerCallback::InnerCallback (BaseNodeChildren* parent)
    : scfImplementationType (this), parent (parent)
  {}

  void BaseNodeChildren::InnerCallback::AnimationFinished (iSkeletonAnimNode2* node)
  {
    parent->AnimationFinished (node);
  }

  void BaseNodeChildren::InnerCallback::AnimationCycled (iSkeletonAnimNode2* node)
  {
    parent->AnimationCycled (node);
  }

  void BaseNodeChildren::InnerCallback::PlayStateChanged (
    iSkeletonAnimNode2* node, bool isPlaying)
  {
    parent->PlayStateChanged (node, isPlaying);
  }

  void BaseNodeChildren::InnerCallback::DurationChanged (iSkeletonAnimNode2* node)
  {
    parent->DurationChanged (node);
  }


  //----------------------------------------



  CS_LEAKGUARD_IMPLEMENT(AnimationNodeFactory);

  AnimationNodeFactory::AnimationNodeFactory (const char* name)
    : scfImplementationType (this), name (name), cyclic (false),
    automaticReset (false), automaticStop (true), playbackSpeed (1.0f), 
    animationDuration (0.0f)
  {}

  void AnimationNodeFactory::SetAnimation (iSkeletonAnimation2* animation)
  {
    this->animation = animation;
    if (animation)
      animationDuration = animation->GetDuration ();
  }
  
  iSkeletonAnimation2* AnimationNodeFactory::GetAnimation () const
  {
    return animation;
  }
  
  void AnimationNodeFactory::SetCyclic (bool cyclic)
  {
    this->cyclic = cyclic;
  }
  
  bool AnimationNodeFactory::IsCyclic () const
  {
    return cyclic;
  }
  
  void AnimationNodeFactory::SetPlaybackSpeed (float speed)
  {
    playbackSpeed = speed;
  }
  
  float AnimationNodeFactory::GetPlaybackSpeed () const
  {
    return playbackSpeed;
  }

  void AnimationNodeFactory::SetAutomaticReset (bool reset)
  {
    automaticReset = reset;
  }

  bool AnimationNodeFactory::GetAutomaticReset () const
  {
    return automaticReset;
  }

  void AnimationNodeFactory::SetAutomaticStop (bool enabled)
  { 
    automaticStop = enabled; 
  }

  bool AnimationNodeFactory::GetAutomaticStop () const 
  { 
    return automaticStop; 
  }

  csPtr<iSkeletonAnimNode2> AnimationNodeFactory::CreateInstance (
    iSkeletonAnimPacket2* packet, iSkeleton2* skeleton)
  {
    csRef<iSkeletonAnimNode2> ref;
    ref.AttachNew (new AnimationNode (this));
    return csPtr<iSkeletonAnimNode2> (ref);
  }

  const char* AnimationNodeFactory::GetNodeName () const
  {
    return name;
  }

  iSkeletonAnimNodeFactory2* AnimationNodeFactory::FindNode (const char* name)
  {
    if (this->name == name)
      return this;
    else
      return 0;
  }


  CS_LEAKGUARD_IMPLEMENT(AnimationNode);

  AnimationNode::AnimationNode (AnimationNodeFactory* factory)
    : scfImplementationType (this), BaseNodeSingle (this), factory (factory), 
    isPlaying (false), playbackPosition (0), playbackSpeed (factory->playbackSpeed)
  {}
  
  void AnimationNode::Play ()
  {
    CS_ASSERT (factory->animation);

    if (!isPlaying && factory->automaticReset)
      playbackPosition = 0;

    isPlaying = true;
    FireStateChangeCb (isPlaying);
  }

  void AnimationNode::Stop ()
  {
    isPlaying = false;
    FireStateChangeCb (isPlaying);
  }

  void AnimationNode::SetPlaybackPosition (float time)
  {
    playbackPosition = csMin (time, factory->animationDuration);
  }

  float AnimationNode::GetPlaybackPosition () const
  {
    return playbackPosition;
  }

  float AnimationNode::GetDuration () const
  {
    return factory->animation->GetDuration ();
  }

  void AnimationNode::SetPlaybackSpeed (float speed)
  {
    playbackSpeed = speed;
  }

  float AnimationNode::GetPlaybackSpeed () const
  {
    return playbackSpeed;
  }

  void AnimationNode::BlendState (csSkeletalState2* state, float baseWeight)
  {
    factory->animation->BlendState (state, baseWeight, playbackPosition, factory->cyclic);
  }
  
  void AnimationNode::TickAnimation (float dt)
  {
    if (!isPlaying)
      return;

    playbackPosition += dt * playbackSpeed;

    const float duration = factory->animationDuration;
    if (playbackPosition > duration)
    {
      if (factory->cyclic)
      {
        while (playbackPosition > duration)
        {
          playbackPosition -= duration;
          BaseNodeSingle::FireAnimationCycleCb ();
        }
      }
      else
      {
        playbackPosition = duration;
        if (factory->automaticStop)
          isPlaying = false;

        BaseNodeSingle::FireAnimationFinishedCb ();
      }
    }
  }

  bool AnimationNode::IsActive () const
  {
    return isPlaying;
  }

  iSkeletonAnimNodeFactory2* AnimationNode::GetFactory () const
  {
    return factory;
  }

  iSkeletonAnimNode2* AnimationNode::FindNode (const char* name)
  {
    if (factory->name == name)
      return this;
    else
      return 0;
  }

  void AnimationNode::AddAnimationCallback (iSkeletonAnimCallback2* callback)
  {
    BaseNodeSingle::AddAnimationCallback (callback);
  }

  void AnimationNode::RemoveAnimationCallback (iSkeletonAnimCallback2* callback)
  {
    BaseNodeSingle::RemoveAnimationCallback (callback);
  }
 
}
CS_PLUGIN_NAMESPACE_END(Skeleton2)
