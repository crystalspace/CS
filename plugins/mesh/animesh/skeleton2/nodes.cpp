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

  void BaseNodeChildren::AddAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback)
  {
    // First CB, install sub-callback
    InstallInnerCb (false);

    BaseNodeSingle::AddAnimationCallback (callback);
  }

  void BaseNodeChildren::RemoveAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback)
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

  void BaseNodeChildren::AnimationFinished (CS::Animation::iSkeletonAnimNode* node)
  {
    // If all subnodes are inactive, call the upper CBs
    for (size_t i = 0; i < subNodes.GetSize (); ++i)
    {
      if (subNodes[i]->IsActive ())
        return;
    }

    BaseNodeSingle::FireAnimationFinishedCb ();
  }

  void BaseNodeChildren::AnimationCycled (CS::Animation::iSkeletonAnimNode* node)
  {
    BaseNodeSingle::FireAnimationCycleCb ();
  }

  void BaseNodeChildren::PlayStateChanged (CS::Animation::iSkeletonAnimNode* node, bool isPlaying)
  {}

  void BaseNodeChildren::DurationChanged (CS::Animation::iSkeletonAnimNode* node)
  {}

  BaseNodeChildren::InnerCallback::InnerCallback (BaseNodeChildren* parent)
    : scfImplementationType (this), parent (parent)
  {}

  void BaseNodeChildren::InnerCallback::AnimationFinished (CS::Animation::iSkeletonAnimNode* node)
  {
    parent->AnimationFinished (node);
  }

  void BaseNodeChildren::InnerCallback::AnimationCycled (CS::Animation::iSkeletonAnimNode* node)
  {
    parent->AnimationCycled (node);
  }

  void BaseNodeChildren::InnerCallback::PlayStateChanged (
    CS::Animation::iSkeletonAnimNode* node, bool isPlaying)
  {
    parent->PlayStateChanged (node, isPlaying);
  }

  void BaseNodeChildren::InnerCallback::DurationChanged (CS::Animation::iSkeletonAnimNode* node)
  {
    parent->DurationChanged (node);
  }


  //----------------------------------------



  CS_LEAKGUARD_IMPLEMENT(AnimationNodeFactory);

  AnimationNodeFactory::AnimationNodeFactory (const char* name)
    : scfImplementationType (this), name (name), cyclic (false),
    automaticReset (false), automaticStop (true), playbackSpeed (1.0f)
  {}

  void AnimationNodeFactory::SetAnimation (CS::Animation::iSkeletonAnimation* animation)
  {
    this->animation = animation;
  }
  
  CS::Animation::iSkeletonAnimation* AnimationNodeFactory::GetAnimation () const
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

  csPtr<CS::Animation::iSkeletonAnimNode> AnimationNodeFactory::CreateInstance (
    CS::Animation::iSkeletonAnimPacket* packet, CS::Animation::iSkeleton* skeleton)
  {
    csRef<CS::Animation::iSkeletonAnimNode> ref;
    ref.AttachNew (new AnimationNode (this));

    // Convert the animation data to bind space if it was not yet made.
    if (animation)
      animation->ConvertFrameSpace (skeleton->GetFactory ());

    return csPtr<CS::Animation::iSkeletonAnimNode> (ref);
  }

  const char* AnimationNodeFactory::GetNodeName () const
  {
    return name;
  }

  CS::Animation::iSkeletonAnimNodeFactory* AnimationNodeFactory::FindNode (const char* name)
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
    playbackPosition = csMin (time, GetDuration ());
  }

  float AnimationNode::GetPlaybackPosition () const
  {
    return playbackPosition;
  }

  float AnimationNode::GetDuration () const
  {
    if (factory->animation)
      return factory->animation->GetDuration ();

    return 0.0f;
  }

  void AnimationNode::SetPlaybackSpeed (float speed)
  {
    playbackSpeed = speed;
  }

  float AnimationNode::GetPlaybackSpeed () const
  {
    return playbackSpeed;
  }

  void AnimationNode::BlendState (CS::Animation::AnimatedMeshState* state, float baseWeight)
  {
    if (!isPlaying)
      return;

    if (factory->animation)
      factory->animation->BlendState (state, baseWeight, playbackPosition);
  }
  
  void AnimationNode::TickAnimation (float dt)
  {
    if (!isPlaying)
      return;

    playbackPosition += dt * playbackSpeed;

    const float duration = GetDuration ();
    if (duration < SMALL_EPSILON)
    {
      playbackPosition = 0;

      if (factory->cyclic)
          BaseNodeSingle::FireAnimationCycleCb ();

      else
      {
        if (factory->automaticStop)
          isPlaying = false;

        BaseNodeSingle::FireAnimationFinishedCb ();
      }
    }

    else if (playbackPosition > duration)
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

    else if (playbackPosition < 0.0f)
    {
      if (factory->cyclic)
      {
        while (playbackPosition < 0.0f)
        {
          playbackPosition += duration;
          BaseNodeSingle::FireAnimationCycleCb ();
        }
      }
      else
      {
        playbackPosition = 0.0f;
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

  CS::Animation::iSkeletonAnimNodeFactory* AnimationNode::GetFactory () const
  {
    return factory;
  }

  CS::Animation::iSkeletonAnimNode* AnimationNode::FindNode (const char* name)
  {
    if (factory->name == name)
      return this;
    else
      return 0;
  }

  void AnimationNode::AddAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback)
  {
    BaseNodeSingle::AddAnimationCallback (callback);
  }

  void AnimationNode::RemoveAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback)
  {
    BaseNodeSingle::RemoveAnimationCallback (callback);
  }
 
}
CS_PLUGIN_NAMESPACE_END(Skeleton2)
