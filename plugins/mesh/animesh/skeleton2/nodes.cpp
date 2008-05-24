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
    if (callbacks.GetSize() == 0)
    {
      // First CB, install sub-callback
      cb.AttachNew (new InnerCallback (this));

      for (size_t i = 0; i < subNodes.GetSize (); ++i)
      {
        subNodes[i]->AddAnimationCallback (cb);
      }
    }

    BaseNodeSingle::AddAnimationCallback (callback);
  }

  void BaseNodeChildren::RemoveAnimationCallback (iSkeletonAnimCallback2* callback)
  {
    BaseNodeSingle::RemoveAnimationCallback (callback);

    if (callbacks.GetSize() == 0)
    {
      // Last CB, remove sub-callback
      for (size_t i = 0; i < subNodes.GetSize (); ++i)
      {
        subNodes[i]->RemoveAnimationCallback (cb);
      }

      cb = 0;
    }
  }

  void BaseNodeChildren::HandleAnimationFinished ()
  {
    // If all subnodes are inactive, call the upper CBs
    for (size_t i = 0; i < subNodes.GetSize (); ++i)
    {
      if (subNodes[i]->IsActive ())
        return;
    }

    BaseNodeSingle::FireAnimationFinishedCallback ();
  }

  BaseNodeChildren::InnerCallback::InnerCallback (BaseNodeChildren* parent)
    : scfImplementationType (this), parent (parent)
  {}

  void BaseNodeChildren::InnerCallback::AnimationFinished ()
  {
    parent->HandleAnimationFinished ();
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
    : scfImplementationType (this), factory (factory), isPlaying (false),
    playbackPosition (0)
  {}

  void AnimationNode::SetPlaybackPosition (float time)
  {
    playbackPosition = csMin (time, factory->animationDuration);
  }

  float AnimationNode::GetPlaybackPosition () const
  {
    return playbackPosition;
  }

  void AnimationNode::Play ()
  {
    isPlaying = true;
  }

  void AnimationNode::Stop ()
  {
    if(isPlaying && factory->automaticReset)
      playbackPosition = 0;

    isPlaying = false;
  }

  void AnimationNode::BlendState (csSkeletalState2* state, float baseWeight)
  {
    if (factory->animation)
      factory->animation->BlendState (state, baseWeight, playbackPosition, factory->cyclic);
  }
  
  void AnimationNode::TickAnimation (float dt)
  {
    if (!isPlaying)
      return;

    playbackPosition += dt * factory->playbackSpeed;

    const float duration = factory->animationDuration;
    if (playbackPosition > duration)
    {
      if (factory->cyclic)
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

        BaseNodeSingle::FireAnimationFinishedCallback ();
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
    : scfImplementationType (this), name (name)
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


  BlendNode::BlendNode (BlendNodeFactory* factory)
    : scfImplementationType (this), factory (factory)
  {
    if (factory)
      weightList = factory->weightList;
  }

  void BlendNode::SetNodeWeight (uint node, float weight)
  {
    CS_ASSERT(node < weightList.GetSize ());
    weightList[node] = weight;
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
      subNodes[i]->TickAnimation (dt);
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
    : scfImplementationType (this), factory (factory)
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
      subNodes[i]->TickAnimation (dt);
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
    : scfImplementationType (this), factory (factory), currentNode (0),
    active (false)
  {
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

    if (newNode != currentNode && active)
    {
      if (factory->autoSwitch)
      {
        subNodes[newNode]->AddAnimationCallback (this);
        subNodes[currentNode]->RemoveAnimationCallback (this);
      }

      subNodes[currentNode]->Stop ();
      subNodes[newNode]->Play ();      
    }

    currentNode = newNode;
  }

  iSkeletonAnimNode2* RandomNode::GetCurrentNode () const
  {
    return subNodes[currentNode];
  }

  void RandomNode::Play ()
  {
    if (active)
      return;

    // Handle auto-switching
    if (factory->autoSwitch)
    {
      subNodes[currentNode]->AddAnimationCallback (this);
    }

    active = true;
    subNodes[currentNode]->Play ();
  }

  void RandomNode::Stop ()
  {
    if (!active)
      return;

    if (factory->autoSwitch)
    {
      subNodes[currentNode]->RemoveAnimationCallback (this);
    }

    active = false;
    subNodes[currentNode]->Stop ();
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

    subNodes[currentNode]->TickAnimation (dt);
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

  void RandomNode::AnimationFinished ()
  {
    if (factory->autoSwitch)
    {
      Switch ();
    }
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
    : scfImplementationType (this), factory (factory),
    currentState (factory->startState), isActive (false)
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

    stateList[currentState].stateNode->TickAnimation (dt);
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
