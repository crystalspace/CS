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
#include "csutil/floatrand.h"
#include "csutil/randomgen.h"

#include "nodes.h"

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{

  CS_IMPLEMENT_STATIC_VAR(GetFGen, csRandomFloatGen, ());


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



  AnimationNodeFactory::AnimationNodeFactory (const char* name)
    : scfImplementationType (this), name (name), cyclic (false),
    automaticReset (false), playbackSpeed (1.0f), animationDuration (0.0f)
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

  csPtr<iSkeletonAnimNode2> AnimationNodeFactory::CreateInstance (
    iSkeletonAnimPacket2* packet, iSkeleton2* skeleton)
  {
    return new AnimationNode (this);
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


  AnimationNode::AnimationNode (AnimationNodeFactory* factory)
    : scfImplementationType (this), BaseNodeSingle (this), factory (factory), 
    isPlaying (false), playbackPosition (0), playbackSpeed (factory->playbackSpeed)
  {}
  
  void AnimationNode::Play ()
  {
    isPlaying = true;
    FireStateChangeCb (isPlaying);
  }

  void AnimationNode::Stop ()
  {
    if(isPlaying && factory->automaticReset)
      playbackPosition = 0;

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
        if (factory->automaticReset)
          playbackPosition = 0;
        else
          playbackPosition = duration;

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

  //----------------------------------------

  BlendNodeFactory::BlendNodeFactory (const char* name)
    : scfImplementationType (this), name (name), syncMode (CS::Animation::SYNC_NONE)
  {
  }

  void BlendNodeFactory::AddNode (iSkeletonAnimNodeFactory2* node, float weight)
  {
    subFactories.Push (node);
    weightList.Push (weight);
  }

  void BlendNodeFactory::SetNodeWeight (uint node, float weight)
  {
    CS_ASSERT(node < weightList.GetSize ());
    weightList[node] = weight;
  }

  void BlendNodeFactory::NormalizeWeights ()
  {
    float weightSum = 0;

    for (size_t i = 0; i < weightList.GetSize (); ++i)
    {
      weightSum += weightList[i];
    }

    for (size_t i = 0; i < weightList.GetSize (); ++i)
    {
      weightList[i] /= weightSum;
    }
  }

  iSkeletonAnimNodeFactory2* BlendNodeFactory::GetNode (uint node)
  {
    CS_ASSERT(node < subFactories.GetSize ());
    return subFactories[node];
  }

  uint BlendNodeFactory::GetNodeCount () const
  {
    return (uint)subFactories.GetSize ();
  }

  void BlendNodeFactory::ClearNodes ()
  {
    subFactories.DeleteAll ();
    weightList.DeleteAll ();
  }

  csPtr<iSkeletonAnimNode2> BlendNodeFactory::CreateInstance (
    iSkeletonAnimPacket2* packet, iSkeleton2* skeleton)
  {
    csRef<BlendNode> newB;
    newB = new BlendNode (this);

    BaseFactoryChildren::SetupInstance (newB, packet, skeleton);

    return csPtr<iSkeletonAnimNode2> (newB);
  }

  const char* BlendNodeFactory::GetNodeName () const
  {
    return name;
  }

  iSkeletonAnimNodeFactory2* BlendNodeFactory::FindNode (const char* name)
  {
    if (this->name == name)
      return this;

    for (size_t i = 0; i < subFactories.GetSize (); ++i)
    {
      iSkeletonAnimNodeFactory2* r = subFactories[i]->FindNode (name);
      if (r)
        return r;
    }

    return 0;
  }

  void BlendNodeFactory::SetSynchronizationMode (CS::Animation::SynchronizationMode mode)
  {
    syncMode = mode;
  }

  CS::Animation::SynchronizationMode BlendNodeFactory::GetSynchronizationMode () const
  {
    return syncMode;
  }

  BlendNode::BlendNode (BlendNodeFactory* factory)
    : scfImplementationType (this), BaseNodeChildren (this), factory (factory), 
    playbackSpeed (1.0f), virtualDuration (0.0f)
  { 
    weightList = factory->weightList;
    
    virtualSubSpeed.SetSize (weightList.GetSize ());
    lastSyncNodes.SetSize (subNodes.GetSize ());

    for (size_t i = 0; i < subNodes.GetSize (); ++i)
    {
      // Use longest duration
      virtualDuration = csMax (virtualDuration, subNodes[i]->GetDuration ());
      virtualSubSpeed[i] = 1.0f;
    }

    SynchronizeSubnodes ();
  }

  void BlendNode::SetNodeWeight (uint node, float weight)
  {
    CS_ASSERT(node < weightList.GetSize ());
    weightList[node] = weight;

    SynchronizeSubnodes ();
  }

  void BlendNode::NormalizeWeights ()
  {
    float weightSum = 0;

    for (size_t i = 0; i < weightList.GetSize (); ++i)
    {
      weightSum += weightList[i];
    }

    for (size_t i = 0; i < weightList.GetSize (); ++i)
    {
      weightList[i] /= weightSum;
    }
  }

  void BlendNode::Play ()
  {
    for (size_t i = 0; i < subNodes.GetSize (); ++i)
    {
      const float nodeWeight = weightList[i];
      if (nodeWeight != 0)
        subNodes[i]->Play ();      
    }
  }

  void BlendNode::Stop ()
  {
    for (size_t i = 0; i < subNodes.GetSize (); ++i)
    {
      const float nodeWeight = weightList[i];
      if (nodeWeight != 0)
        subNodes[i]->Stop ();      
    }
  }

  void BlendNode::SetPlaybackPosition (float time)
  {
    for (size_t i = 0; i < subNodes.GetSize (); ++i)
    {
      const float nodeWeight = weightList[i];
      if (nodeWeight != 0)
        subNodes[i]->SetPlaybackPosition (time*virtualSubSpeed[i]);
    }
  }

  float BlendNode::GetPlaybackPosition () const
  {    
    for (size_t i = 0; i < subNodes.GetSize (); ++i)
    {
      const float nodeWeight = weightList[i];
      if (nodeWeight != 0)
        return subNodes[i]->GetPlaybackPosition () / virtualSubSpeed[i];
    }

    return 0;
  }

  float BlendNode::GetDuration () const
  {
    return virtualDuration;
  }

  void BlendNode::SetPlaybackSpeed (float speed)
  {
    playbackSpeed = speed;
  }

  float BlendNode::GetPlaybackSpeed () const
  {
    return playbackSpeed;
  }

  void BlendNode::BlendState (csSkeletalState2* state, float baseWeight)
  {
    float accWeight = 0.0f;

    for (size_t i = 0; i < subNodes.GetSize (); ++i)
    {
      const float nodeWeight = weightList[i];
      if (nodeWeight == 0 || !subNodes[i]->IsActive())
        continue;

      accWeight += nodeWeight;
      float w = nodeWeight / accWeight;

      subNodes[i]->BlendState (state, w * baseWeight);
    }
  }

  void BlendNode::TickAnimation (float dt)
  {
    for (size_t i = 0; i < subNodes.GetSize (); ++i)
    {
      subNodes[i]->TickAnimation (dt*playbackSpeed);
    }
  }

  bool BlendNode::IsActive () const
  {
    for (size_t i = 0; i < subNodes.GetSize (); ++i)
    {
      if (subNodes[i]->IsActive ())
      {
        return true;
      }
    }

    return false;
  }

  iSkeletonAnimNodeFactory2* BlendNode::GetFactory () const
  {
    return factory;
  }

  iSkeletonAnimNode2* BlendNode::FindNode (const char* name)
  {
    if (factory->name == name)
      return this;

    for (size_t i = 0; i < subNodes.GetSize (); ++i)
    {
      iSkeletonAnimNode2* r = subNodes[i]->FindNode (name);
      if (r)
        return r;
    }

    return 0;
  }

  void BlendNode::AddAnimationCallback (iSkeletonAnimCallback2* callback)
  {
    BaseNodeChildren::AddAnimationCallback (callback);
  }

  void BlendNode::RemoveAnimationCallback (iSkeletonAnimCallback2* callback)
  {
    BaseNodeChildren::RemoveAnimationCallback (callback);
  }

  void BlendNode::SynchronizeSubnodes ()
  {
    switch (factory->syncMode)
    {
    case CS::Animation::SYNC_NONE:
      break;

    case CS::Animation::SYNC_FIRSTFRAME:
      {
        // Make sure the first frames match up
        csBitArray newNodes;
        newNodes.SetSize (subNodes.GetSize ());

        // Start by computing the virtual duration
        const float oldVirtualDuration = virtualDuration;
        float accWeight = 0;
        virtualDuration = 0;

        for (size_t i = 0; i < subNodes.GetSize (); ++i)
        {
          const float nodeWeight = weightList[i];
          if (nodeWeight == 0 || !subNodes[i]->IsActive())
            continue;

          newNodes.SetBit (i);

          accWeight += nodeWeight;
          virtualDuration += nodeWeight * subNodes[i]->GetDuration ();
        }
        virtualDuration /= accWeight;

        // Compute the virtual speeds of the sub-nodes to match them up
        for (size_t i = 0; i < subNodes.GetSize (); ++i)
        {
          if (!newNodes.IsBitSet (i))
            continue;

          virtualSubSpeed[i] = virtualDuration / subNodes[i]->GetDuration ();
          subNodes[i]->SetPlaybackSpeed (virtualSubSpeed[i]);
        }

        // Then finally match up the positions. 
        // Use the nodes active last synchronization to work out the current position
        accWeight = 0;
        float currentPosition = 0;

        for (size_t i = 0; i < subNodes.GetSize (); ++i)
        {
          if (!newNodes.IsBitSet (i))
            continue;

          if (!lastSyncNodes.IsBitSet (i))
            continue;

          const float nodeWeight = weightList[i];
          accWeight += nodeWeight;
          currentPosition += nodeWeight * subNodes[i]->GetPlaybackPosition ();
        }
        currentPosition /= accWeight;

        for (size_t i = 0; i < subNodes.GetSize (); ++i)
        {
          if (!newNodes.IsBitSet (i))
            continue;

          subNodes[i]->SetPlaybackPosition (currentPosition * virtualSubSpeed[i]);
        }

        lastSyncNodes = newNodes;

        if (oldVirtualDuration != virtualDuration)
        {
          FireDurationChangeCb ();
        }

        // Make sure we're notified about changes
        InstallInnerCb (true);
      }
      break;
    }
  }

  void BlendNode::PlayStateChanged (iSkeletonAnimNode2* node, bool isPlaying)
  {
    FireStateChangeCb (isPlaying);

    if (IsActive ())
    {
      SynchronizeSubnodes ();
    }
    else
    {
      FireAnimationFinishedCb ();
    }    
  }

  void BlendNode::DurationChanged (iSkeletonAnimNode2* node)
  {
    SynchronizeSubnodes ();
  }

  //----------------------------------------
  PriorityNodeFactory::PriorityNodeFactory (const char* name)
    : scfImplementationType (this), name (name)
  {
  }

  //-- iSkeletonPriorityNodeFactory2
  void PriorityNodeFactory::AddNode (iSkeletonAnimNodeFactory2* node, 
    unsigned int priority)
  {
    subFactories.Push (node);
    priorityList.Push (priority);
  }

  void PriorityNodeFactory::SetNodePriority (uint node, unsigned int priority)
  {
    CS_ASSERT(node < priorityList.GetSize ());
    priorityList[node] = 0;
  }

  iSkeletonAnimNodeFactory2* PriorityNodeFactory::GetNode (uint node)
  {
    CS_ASSERT(node < subFactories.GetSize ());
    return subFactories[node];
  }

  uint PriorityNodeFactory::GetNodeCount () const
  {
    return (uint)subFactories.GetSize ();
  }

  void PriorityNodeFactory::ClearNodes ()
  {
    subFactories.DeleteAll ();
    priorityList.DeleteAll ();
  }

  //-- iSkeletonAnimationNodeFactory2
  csPtr<iSkeletonAnimNode2> PriorityNodeFactory::CreateInstance (
    iSkeletonAnimPacket2* packet, iSkeleton2* skeleton)
  {
    csRef<PriorityNode> newp;
    newp = new PriorityNode (this);

    BaseFactoryChildren::SetupInstance (newp, packet, skeleton);

    return csPtr<iSkeletonAnimNode2> (newp);
  }
  
  const char* PriorityNodeFactory::GetNodeName () const
  {
    return name;
  }

  iSkeletonAnimNodeFactory2* PriorityNodeFactory::FindNode (const char* name)
  {
    if (this->name == name)
      return this;

    for (size_t i = 0; i < subFactories.GetSize (); ++i)
    {
      iSkeletonAnimNodeFactory2* r = subFactories[i]->FindNode (name);
      if (r)
        return r;
    }

    return 0;
  }

  PriorityNode::PriorityNode (PriorityNodeFactory* factory)
    : scfImplementationType (this), factory (factory),
    BaseNodeChildren (this), playbackSpeed (1.0f)
  {
    if (factory)
    {
      priorityList = factory->priorityList;
      UpdateIndexList ();
    }
  }

  void PriorityNode::SetNodePriority (uint node, unsigned int priority)
  {
    CS_ASSERT(node < priorityList.GetSize ());
    priorityList[node] = priority;

    UpdateIndexList ();
  }

  void PriorityNode::Play ()
  {
    for (size_t i = 0; i < subNodes.GetSize (); ++i)
    {
      subNodes[i]->Play ();
    }
  }

  void PriorityNode::Stop ()
  {
    for (size_t i = 0; i < subNodes.GetSize (); ++i)
    {
      subNodes[i]->Stop ();
    }
  }

  void PriorityNode::SetPlaybackPosition (float time)
  {
    for (size_t i = 0; i < subNodes.GetSize (); ++i)
    {
      if (!subNodes[i]->IsActive())
        continue;

      subNodes[i]->SetPlaybackPosition (time);      
    }
  }

  float PriorityNode::GetPlaybackPosition () const
  {
    float num = 0;
    float pos = 0;

    for (size_t i = 0; i < subNodes.GetSize (); ++i)
    {
      if (!subNodes[i]->IsActive())
        continue;

      pos += subNodes[i]->GetPlaybackPosition ();
      num++;
    }

    return pos / num;
  }

  float PriorityNode::GetDuration () const
  {
    float duration = 0;
    for (size_t i = 0; i < subNodes.GetSize (); ++i)
    {
      if (!subNodes[i]->IsActive())
        continue;

      duration = csMax (duration, subNodes[i]->GetDuration ());
    }

    return duration;
  }

  void PriorityNode::SetPlaybackSpeed (float speed)
  {
    playbackSpeed = speed;
  }

  float PriorityNode::GetPlaybackSpeed () const
  {
    return playbackSpeed;
  }

  void PriorityNode::BlendState (csSkeletalState2* state, float baseWeight)
  {
    csRef<csSkeletalState2> locState;
    locState.AttachNew (new csSkeletalState2); //@@TODO: cache these

    csRef<csSkeletalState2> totalState;
    totalState.AttachNew (new csSkeletalState2);
    totalState->Setup (state->GetBoneCount ());

    for (size_t i = 0; i < indexList.GetSize (); ++i)
    {
      if (!subNodes[indexList[i]]->IsActive())
        continue;
      
      locState->Setup (state->GetBoneCount ());

      subNodes[indexList[i]]->BlendState (locState, 1.0f);

      for (size_t b = 0; b < locState->GetBoneCount (); ++b)
      {
        if (locState->IsBoneUsed (b))
        {
          totalState->GetQuaternion (b) = locState->GetQuaternion (b);
          totalState->GetVector (b) = locState->GetVector (b);

          totalState->SetBoneUsed (b);
        }
      }
    }

    for (size_t b = 0; b < totalState->GetBoneCount (); ++b)
    {
      if (totalState->IsBoneUsed (b))
      {
        const csQuaternion tmpQ = state->GetQuaternion (b);
        const csVector3 tmpV = state->GetVector (b);

        state->GetQuaternion (b) = tmpQ.SLerp (totalState->GetQuaternion (b), baseWeight);
        state->GetVector (b) = csLerp (tmpV, totalState->GetVector (b), baseWeight);
        state->SetBoneUsed (b);
      }
    }
  }

  void PriorityNode::TickAnimation (float dt)
  {
    for (size_t i = 0; i < subNodes.GetSize (); ++i)
    {
      subNodes[i]->TickAnimation (dt * playbackSpeed);
    }
  }

  bool PriorityNode::IsActive () const
  {
    for (size_t i = 0; i < subNodes.GetSize (); ++i)
    {
      if (subNodes[i]->IsActive ())
      {
        return true;
      }
    }

    return false;
  }

  iSkeletonAnimNodeFactory2* PriorityNode::GetFactory () const
  {
    return factory;
  }

  iSkeletonAnimNode2* PriorityNode::FindNode (const char* name)
  {
    if (factory->name == name)
      return this;

    for (size_t i = 0; i < subNodes.GetSize (); ++i)
    {
      iSkeletonAnimNode2* r = subNodes[i]->FindNode (name);
      if (r)
        return r;
    }

    return 0;
  }
  
  void PriorityNode::AddAnimationCallback (iSkeletonAnimCallback2* callback)
  {
    BaseNodeChildren::AddAnimationCallback (callback);
  }

  void PriorityNode::RemoveAnimationCallback (iSkeletonAnimCallback2* callback)
  {
    BaseNodeChildren::RemoveAnimationCallback (callback);
  }

  // Helpers for index list updating
  struct IndexPriority
  {
    IndexPriority (size_t index, size_t priority) 
      : index (index), priority (priority) 
    {}

    size_t index;
    size_t priority;

    bool operator < (const IndexPriority& other) const
    { return priority < other.priority; }
  };

  void PriorityNode::UpdateIndexList ()
  {
    indexList.DeleteAll ();

    csArray<IndexPriority> priSorter;
    for (size_t i = 0; i < priorityList.GetSize (); ++i)
    {
      priSorter.Push (IndexPriority (i, priorityList[i]));
    }
    priSorter.Sort ();

    for (size_t i = 0; i < priSorter.GetSize (); ++i)
    {
      indexList.Push (priSorter[i].index);
    }
  }

  //----------------------------------------

  RandomNodeFactory::RandomNodeFactory (const char* name)
    : scfImplementationType (this), name (name), autoSwitch (true)
  {}

  void RandomNodeFactory::AddNode (iSkeletonAnimNodeFactory2* node, float probability)
  {
    subFactories.Push (node); 
    probabilityList.Push (probability);

    accumProbabilityList.DeleteAll ();
  }

  void RandomNodeFactory::SetNodeProbability (uint node, float probability)
  {
    CS_ASSERT(node < probabilityList.GetSize ());
    probabilityList[node] = probability;

    accumProbabilityList.DeleteAll ();
  }

  void RandomNodeFactory::SetAutomaticSwitch (bool automatic)
  {
    autoSwitch = automatic;
  }

  iSkeletonAnimNodeFactory2* RandomNodeFactory::GetNode (uint node)
  {
    CS_ASSERT(node < subFactories.GetSize ());

    return subFactories[node];
  }

  uint RandomNodeFactory::GetNodeCount () const
  {
    return subFactories.GetSize ();
  }

  void RandomNodeFactory::ClearNodes ()
  {
    subFactories.DeleteAll ();
    probabilityList.DeleteAll ();
    accumProbabilityList.DeleteAll ();
  }
  
  csPtr<iSkeletonAnimNode2> RandomNodeFactory::CreateInstance (
    iSkeletonAnimPacket2* packet, iSkeleton2* skeleton)
  {
    csRef<RandomNode> newp;
    newp = new RandomNode (this);

    BaseFactoryChildren::SetupInstance (newp, packet, skeleton);

    return csPtr<iSkeletonAnimNode2> (newp);
  }

  const char* RandomNodeFactory::GetNodeName () const
  {
    return name;
  }

  iSkeletonAnimNodeFactory2* RandomNodeFactory::FindNode (const char* name)
  {
    if (this->name == name)
      return this;

    for (size_t i = 0; i < subFactories.GetSize (); ++i)
    {
      iSkeletonAnimNodeFactory2* r = subFactories[i]->FindNode (name);
      if (r)
        return r;
    }

    return 0;
  }

  void RandomNodeFactory::BuildAccumList ()
  {
    if (probabilityList.GetSize () == accumProbabilityList.GetSize () ||
      probabilityList.GetSize() == 0)
      return;

    accumProbabilityList.SetSize (probabilityList.GetSize (), 0);
    accumProbabilityList[0] = probabilityList[0];
    float sum = probabilityList[0];

    for (size_t i = 1; i < probabilityList.GetSize (); ++i)
    {
      accumProbabilityList[i] = accumProbabilityList[i-1] + probabilityList[i];
      sum += probabilityList[i];
    }

    // Normalize
    for (size_t i = 0; i < accumProbabilityList.GetSize (); ++i)
    {
      accumProbabilityList[i] /= sum;
    }
  }

  RandomNode::RandomNode (RandomNodeFactory* factory)
    : scfImplementationType (this), BaseNodeChildren (this), factory (factory), 
    currentNode (0), active (false), playbackSpeed (1.0f)
  {
    // Need CB for possible automatic switch
    InstallInnerCb (true);
    Switch ();
  }

  void RandomNode::Switch ()
  {
    factory->BuildAccumList ();

    float rnd = GetFGen ()->Get ();

    // Find node
    size_t newNode = 0;
    for (size_t i = 0; i < factory->accumProbabilityList.GetSize (); ++i)
    {
      if (factory->accumProbabilityList[i] > rnd)
      {
        newNode = i;
        break;
      }
    }


    const bool nodeSwitched = newNode != currentNode;
    if (nodeSwitched && active)
    {
      subNodes[currentNode]->Stop ();
      subNodes[newNode]->Play ();      
    }

    currentNode = newNode;

    if (nodeSwitched)
    {
      FireDurationChangeCb ();
    }
  }

  iSkeletonAnimNode2* RandomNode::GetCurrentNode () const
  {
    return subNodes[currentNode];
  }

  void RandomNode::Play ()
  {
    if (active)
      return;

    active = true;
    subNodes[currentNode]->Play ();
    FireStateChangeCb (true);
  }

  void RandomNode::Stop ()
  {
    if (!active)
      return;

    active = false;
    subNodes[currentNode]->Stop ();
    FireStateChangeCb (false);
  }

  void RandomNode::SetPlaybackPosition (float time)
  {
    subNodes[currentNode]->SetPlaybackPosition (time);
  }

  float RandomNode::GetPlaybackPosition () const
  {
    return subNodes[currentNode]->GetPlaybackPosition ();
  }

  float RandomNode::GetDuration () const
  {
    return subNodes[currentNode]->GetDuration ();
  }

  void RandomNode::SetPlaybackSpeed (float speed)
  {
    playbackSpeed = speed;
  }

  float RandomNode::GetPlaybackSpeed () const
  {
    return playbackSpeed;
  }

  void RandomNode::BlendState (csSkeletalState2* state, float baseWeight)
  {
    if (!active)
      return;

    subNodes[currentNode]->BlendState (state, baseWeight);    
  }

  void RandomNode::TickAnimation (float dt)
  {
    if (!active)
      return;

    subNodes[currentNode]->TickAnimation (dt * playbackSpeed);
  }

  bool RandomNode::IsActive () const
  {
    return active && subNodes[currentNode]->IsActive ();
  }

  iSkeletonAnimNodeFactory2* RandomNode::GetFactory () const
  {
    return factory;
  }

  iSkeletonAnimNode2* RandomNode::FindNode (const char* name)
  {
    if (factory->name == name)
      return this;

    for (size_t i = 0; i < subNodes.GetSize (); ++i)
    {
      iSkeletonAnimNode2* r = subNodes[i]->FindNode (name);
      if (r)
        return r;
    }

    return 0;
  }

  void RandomNode::AddAnimationCallback (iSkeletonAnimCallback2* callback)
  {
    BaseNodeChildren::AddAnimationCallback (callback);
  }

  void RandomNode::RemoveAnimationCallback (iSkeletonAnimCallback2* callback)
  {
    BaseNodeChildren::RemoveAnimationCallback (callback);
  }

  void RandomNode::AnimationFinished (iSkeletonAnimNode2* node)
  {
    if (node == subNodes[currentNode] && factory->autoSwitch)
    {
      Switch ();
    }
  }

  void RandomNode::PlayStateChanged (iSkeletonAnimNode2* node, bool isPlaying)
  {
    if (node == subNodes[currentNode] && !isPlaying)
    {
      active == false;
      FireStateChangeCb (false);
    }
  }

  void RandomNode::DurationChanged (iSkeletonAnimNode2* node)
  {
    FireDurationChangeCb ();
  }

 

  //----------------------------------------

  FSMNodeFactory::FSMNodeFactory (const char* name)
    : scfImplementationType (this), name (name), 
    startState (CS::Animation::InvalidStateID)
  {}

  CS::Animation::StateID FSMNodeFactory::AddState ()
  {
    State newState;
    
    return stateList.Push (newState);    
  }

  void FSMNodeFactory::SetStateNode (CS::Animation::StateID id, 
    iSkeletonAnimNodeFactory2* nodeFact)
  {
    CS_ASSERT(id < stateList.GetSize ());

    stateList[id].nodeFactory = nodeFact;
  }

  iSkeletonAnimNodeFactory2* FSMNodeFactory::GetStateNode (CS::Animation::StateID id) const
  {
    CS_ASSERT(id < stateList.GetSize ());

    return stateList[id].nodeFactory;
  }

  void FSMNodeFactory::SetStateName (CS::Animation::StateID id, const char* name)
  {
    CS_ASSERT(id < stateList.GetSize ());

    stateList[id].name = name;
  }

  const char* FSMNodeFactory::GetStateName (CS::Animation::StateID id) const
  {
    CS_ASSERT(id < stateList.GetSize ());
    
    return stateList[id].name;
  }

  CS::Animation::StateID FSMNodeFactory::FindState (const char* name) const
  {
    for (size_t i = 0; i < stateList.GetSize (); ++i)
    {
      if (stateList[i].name == name)
      {
        return i;
      }
    }

    return CS::Animation::InvalidStateID;
  }

  void FSMNodeFactory::SetStartState (CS::Animation::StateID id)
  {
    CS_ASSERT(id < stateList.GetSize ());
    startState = id;
  }

  CS::Animation::StateID FSMNodeFactory::GetStartState () const
  {
    return startState;
  }

  uint FSMNodeFactory::GetStateCount () const
  {
    return (uint)stateList.GetSize ();
  }

  void FSMNodeFactory::ClearStates ()
  {
    stateList.DeleteAll ();
  }

  csPtr<iSkeletonAnimNode2> FSMNodeFactory::CreateInstance (
    iSkeletonAnimPacket2* packet, iSkeleton2* skeleton)
  {
    csRef<FSMNode> newn;

    newn.AttachNew (new FSMNode (this));

    // Setup nodes
    for (size_t i = 0; i < stateList.GetSize (); ++i)
    {
      FSMNode::State newState;

      newState.stateNode = stateList[i].nodeFactory->CreateInstance (packet, skeleton);

      newn->stateList.Push (newState);
    }

    return csPtr<iSkeletonAnimNode2> (newn);
  }

  const char* FSMNodeFactory::GetNodeName () const
  {
    return name;
  }

  iSkeletonAnimNodeFactory2* FSMNodeFactory::FindNode (const char* name)
  {
    if (this->name == name)
      return this;

    for (size_t i = 0; i < stateList.GetSize (); ++i)
    {
      iSkeletonAnimNodeFactory2* r = stateList[i].nodeFactory;
      if (r)
      {
        r = r->FindNode (name);

        if (r)
          return r;
      }      
    }

    return 0;
  }


  FSMNode::FSMNode (FSMNodeFactory* factory)
    : scfImplementationType (this), BaseNodeSingle (this), factory (factory),
    currentState (factory->startState), isActive (false), playbackSpeed (1.0f)
  {}

  void FSMNode::SwitchToState (CS::Animation::StateID newState)
  {
    if (isActive)
    {
      stateList[currentState].stateNode->Stop ();
      
      if (newState != CS::Animation::InvalidStateID)
      {
        stateList[newState].stateNode->Play ();
      }
    }

    currentState = newState;
  }

  CS::Animation::StateID FSMNode::GetCurrentState () const
  {
    return currentState;
  }

  void FSMNode::Play ()
  {
    if (isActive)
      return;    

    if (currentState != CS::Animation::InvalidStateID)
    {
      isActive = true;
      stateList[currentState].stateNode->Play ();
    }    
  }

  void FSMNode::Stop ()
  {
    if (!isActive)
      return;

    isActive = false;
    stateList[currentState].stateNode->Stop ();
  }

  void FSMNode::SetPlaybackPosition (float time)
  {
    stateList[currentState].stateNode->SetPlaybackPosition (time);
  }
  
  float FSMNode::GetPlaybackPosition () const
  {
    return stateList[currentState].stateNode->GetPlaybackPosition ();
  }

  float FSMNode::GetDuration () const
  {
    return stateList[currentState].stateNode->GetDuration ();
  }
  
  void FSMNode::SetPlaybackSpeed (float speed)
  {
    playbackSpeed = speed;
  }
  
  float FSMNode::GetPlaybackSpeed () const
  {
    return playbackSpeed;
  }

  void FSMNode::BlendState (csSkeletalState2* state, float baseWeight)
  {
    if (!isActive)
      return;

    stateList[currentState].stateNode->BlendState (state, baseWeight);
  }

  void FSMNode::TickAnimation (float dt)
  {
    if (!isActive)
      return;

    stateList[currentState].stateNode->TickAnimation (dt * playbackSpeed);
  }

  bool FSMNode::IsActive () const
  {
    return isActive;
  }

  iSkeletonAnimNodeFactory2* FSMNode::GetFactory () const
  {
    return factory;
  }

  iSkeletonAnimNode2* FSMNode::FindNode (const char* name)
  {
    if (factory->name == name)
      return this;

    for (size_t i = 0; i < stateList.GetSize (); ++i)
    {
      iSkeletonAnimNode2* r = stateList[i].stateNode;
      if (r)
      {
        r = r->FindNode (name);

        if (r)
          return r;
      }      
    }

    return 0;
  }

  void FSMNode::AddAnimationCallback (iSkeletonAnimCallback2* callback)
  {
    BaseNodeSingle::AddAnimationCallback (callback);
  }

  void FSMNode::RemoveAnimationCallback (iSkeletonAnimCallback2* callback)
  {
    BaseNodeSingle::AddAnimationCallback (callback);
  }


}
CS_PLUGIN_NAMESPACE_END(Skeleton2)
