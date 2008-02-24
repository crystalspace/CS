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
  }

  void BlendNodeFactory::SetNodeWeight (uint node, float weight)
  {
  }

  iSkeletonAnimationNodeFactory2* BlendNodeFactory::GetNode (uint node)
  {
    return 0;
  }

  uint BlendNodeFactory::GetNodeCount () const
  {
    return 0;
  }

  void BlendNodeFactory::ClearNodes ()
  {
  }

  csPtr<iSkeletonAnimationNode2> BlendNodeFactory::CreateInstance ()
  {
    return 0;
  }



  BlendNode::BlendNode ()
    : scfImplementationType (this)
  {
  }

  void BlendNode::SetNodeWeight (uint node, float weight)
  {
  }

  void BlendNode::BlendState (csSkeletalState2* state, float baseWeight)
  {
  }

  void BlendNode::TickAnimation (float dt)
  {
  }

  bool BlendNode::IsActive () const
  {
    return false;
  }

  iSkeletonAnimationNodeFactory2* BlendNode::GetFactory () const
  {
    return 0;
  }


}
CS_PLUGIN_NAMESPACE_END(Skeleton2)
