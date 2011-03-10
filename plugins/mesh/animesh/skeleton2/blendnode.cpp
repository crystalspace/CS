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

#include "blendnode.h"

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{

  CS_LEAKGUARD_IMPLEMENT(BlendNodeFactory);

  BlendNodeFactory::BlendNodeFactory (const char* name)
    : scfImplementationType (this), name (name), syncMode (CS::Animation::SYNC_NONE)
  {
  }

  void BlendNodeFactory::AddNode (CS::Animation::iSkeletonAnimNodeFactory* node, float weight)
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

  CS::Animation::iSkeletonAnimNodeFactory* BlendNodeFactory::GetNode (uint node)
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

  csPtr<CS::Animation::iSkeletonAnimNode> BlendNodeFactory::CreateInstance (
    CS::Animation::iSkeletonAnimPacket* packet, CS::Animation::iSkeleton* skeleton)
  {
    csRef<BlendNode> newB;
    newB.AttachNew (new BlendNode (this));

    BaseFactoryChildren::SetupInstance (newB, packet, skeleton);

    return csPtr<CS::Animation::iSkeletonAnimNode> (newB);
  }

  const char* BlendNodeFactory::GetNodeName () const
  {
    return name;
  }

  CS::Animation::iSkeletonAnimNodeFactory* BlendNodeFactory::FindNode (const char* name)
  {
    if (this->name == name)
      return this;

    for (size_t i = 0; i < subFactories.GetSize (); ++i)
    {
      CS::Animation::iSkeletonAnimNodeFactory* r = subFactories[i]->FindNode (name);
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

  
  CS_LEAKGUARD_IMPLEMENT(BlendNode);

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

  void BlendNode::BlendState (CS::Animation::AnimatedMeshState* state, float baseWeight)
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

  CS::Animation::iSkeletonAnimNodeFactory* BlendNode::GetFactory () const
  {
    return factory;
  }

  CS::Animation::iSkeletonAnimNode* BlendNode::FindNode (const char* name)
  {
    if (factory->name == name)
      return this;

    for (size_t i = 0; i < subNodes.GetSize (); ++i)
    {
      CS::Animation::iSkeletonAnimNode* r = subNodes[i]->FindNode (name);
      if (r)
        return r;
    }

    return 0;
  }

  void BlendNode::AddAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback)
  {
    BaseNodeChildren::AddAnimationCallback (callback);
  }

  void BlendNode::RemoveAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback)
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

  void BlendNode::PlayStateChanged (CS::Animation::iSkeletonAnimNode* node, bool isPlaying)
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

  void BlendNode::DurationChanged (CS::Animation::iSkeletonAnimNode* node)
  {
    SynchronizeSubnodes ();
  }

}
CS_PLUGIN_NAMESPACE_END(Skeleton2)
