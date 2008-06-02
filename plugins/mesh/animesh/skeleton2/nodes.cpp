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
#include "nodes.h"

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{
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
    return subFactories.GetSize ();
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

    newB->weightList = weightList;
    for (size_t i = 0; i < subFactories.GetSize (); ++i)
    {
      csRef<iSkeletonAnimNode2> node = 
        subFactories[i]->CreateInstance (packet, skeleton);
      newB->subNodes.Push (node);
    }

    return csPtr<iSkeletonAnimNode2> (newB);
  }

  const char* BlendNodeFactory::GetNodeName () const
  {
    return name;
  }

  iSkeletonAnimNodeFactory2* BlendNodeFactory::FindNode (const char* name)
  {
    for (size_t i = 0; i < subFactories.GetSize (); ++i)
    {
      iSkeletonAnimNodeFactory2* r = subFactories[i]->FindNode (name);
      if (r)
        return r;
    }

    return 0;
  }


  BlendNode::BlendNode (BlendNodeFactory* factory)
    : scfImplementationType (this, factory), factory (factory)
  {
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
    for (size_t i = 0; i < subNodes.GetSize (); ++i)
    {
      iSkeletonAnimNode2* r = subNodes[i]->FindNode (name);
      if (r)
        return r;
    }

    return 0;
  }

}
CS_PLUGIN_NAMESPACE_END(Skeleton2)
