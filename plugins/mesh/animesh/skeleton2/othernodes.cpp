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

#include "othernodes.h"

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{

  CS_IMPLEMENT_STATIC_VAR(GetFGen, csRandomFloatGen, ());


  CS_LEAKGUARD_IMPLEMENT(PriorityNodeFactory);

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
    newp.AttachNew (new PriorityNode (this));

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
  
  
  CS_LEAKGUARD_IMPLEMENT(PriorityNode);

  PriorityNode::PriorityNode (PriorityNodeFactory* factory)
    : scfImplementationType (this), BaseNodeChildren (this), 
    playbackSpeed (1.0f), factory (factory)
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

      for (BoneID b = 0; b < locState->GetBoneCount (); ++b)
      {
        if (locState->IsBoneUsed (b))
        {
          totalState->GetQuaternion (b) = locState->GetQuaternion (b);
          totalState->GetVector (b) = locState->GetVector (b);

          totalState->SetBoneUsed (b);
        }
      }
    }

    for (BoneID b = 0; b < totalState->GetBoneCount (); ++b)
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

  CS_LEAKGUARD_IMPLEMENT(RandomNodeFactory);

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
    newp.AttachNew (new RandomNode (this));

    BaseFactoryChildren::SetupInstance (newp, packet, skeleton);

    // Need CB for possible automatic switch
    newp->InstallInnerCb (true);
    newp->Switch ();

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

  
  CS_LEAKGUARD_IMPLEMENT(RandomNode);

  RandomNode::RandomNode (RandomNodeFactory* factory)
    : scfImplementationType (this), BaseNodeChildren (this), currentNode (0), 
    active (false), playbackSpeed (1.0f), factory (factory)
  {
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
    if (!subNodes.GetSize ())
      return 0;

    return subNodes[currentNode];
  }

  void RandomNode::Play ()
  {
    if (active)
      return;

    active = true;
    if (subNodes.GetSize ())
      subNodes[currentNode]->Play ();
    FireStateChangeCb (true);
  }

  void RandomNode::Stop ()
  {
    if (!active)
      return;

    active = false;
    if (subNodes.GetSize ())
      subNodes[currentNode]->Stop ();
    FireStateChangeCb (false);
  }

  void RandomNode::SetPlaybackPosition (float time)
  {
    if (!subNodes.GetSize ())
      return;

    subNodes[currentNode]->SetPlaybackPosition (time);
  }

  float RandomNode::GetPlaybackPosition () const
  {
    if (!subNodes.GetSize ())
      return 0.0f;

    return subNodes[currentNode]->GetPlaybackPosition ();
  }

  float RandomNode::GetDuration () const
  {
    if (!subNodes.GetSize ())
      return 0.0f;

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
    if (!active || !subNodes.GetSize ())
      return;

    subNodes[currentNode]->BlendState (state, baseWeight);    
  }

  void RandomNode::TickAnimation (float dt)
  {
    if (!active || !subNodes.GetSize ())
      return;

    subNodes[currentNode]->TickAnimation (dt * playbackSpeed);
  }

  bool RandomNode::IsActive () const
  {
    return active
      && (!subNodes.GetSize () || subNodes[currentNode]->IsActive ());
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
    if (node == subNodes[currentNode] && !isPlaying && !factory->autoSwitch)
    {
      active = false;
      FireStateChangeCb (false);
    }
  }

  void RandomNode::DurationChanged (iSkeletonAnimNode2* node)
  {
    FireDurationChangeCb ();
  }

 }
 CS_PLUGIN_NAMESPACE_END(Skeleton2)
