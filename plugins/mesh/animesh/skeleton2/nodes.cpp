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
  BlendNodeFactory::BlendNodeFactory ()
    : scfImplementationType (this)
  {
  }

  void BlendNodeFactory::AddNode (iSkeletonAnimationNodeFactory2* node, float weight)
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

  iSkeletonAnimationNodeFactory2* BlendNodeFactory::GetNode (uint node)
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

  csPtr<iSkeletonAnimationNode2> BlendNodeFactory::CreateInstance (iSkeleton2* s)
  {
    return new BlendNode (this, s);
  }



  BlendNode::BlendNode (BlendNodeFactory* factory, iSkeleton2* skeleton)
    : scfImplementationType (this, factory), factory (factory)
  {
    weightList = factory->weightList;
    for (size_t i = 0; i < factory->subFactories.GetSize (); ++i)
    {
      csRef<iSkeletonAnimationNode2> node = 
        factory->subFactories[i]->CreateInstance (skeleton);
      subNodes.Push (node);
    }
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
    for (size_t i = 0; i < subNodes.GetSize (); ++i)
    {
      float nodeWeight = baseWeight * weightList[i];
      subNodes[i]->BlendState (state, nodeWeight);
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

  iSkeletonAnimationNodeFactory2* BlendNode::GetFactory () const
  {
    return factory;
  }


}
CS_PLUGIN_NAMESPACE_END(Skeleton2)
