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

#ifndef __CS_NODES_H__
#define __CS_NODES_H__

#include "csutil/scf_implementation.h"
#include "imesh/skeleton2.h"
#include "csutil/refarr.h"

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{
  class BlendNodeFactory :
    public scfImplementation2<BlendNodeFactory,
                              iSkeletonBlendNodeFactory2,
                              scfFakeInterface<iSkeletonAnimationNodeFactory2> >
  {
  public:
    BlendNodeFactory ();

    //-- iSkeletonBlendNodeFactory2
    virtual void AddNode (iSkeletonAnimationNodeFactory2* node, float weight);
    virtual void SetNodeWeight (uint node, float weight);
    virtual void NormalizeWeights ();
    virtual iSkeletonAnimationNodeFactory2* GetNode (uint node);
    virtual uint GetNodeCount () const;
    virtual void ClearNodes ();

    //-- iSkeletonAnimationNodeFactory2
    virtual csPtr<iSkeletonAnimationNode2> CreateInstance (iSkeleton2*);

  private:
    friend class BlendNode;

    csRefArray<iSkeletonAnimationNodeFactory2> subFactories;
    csArray<float> weightList;
  };


  class BlendNode : 
    public scfImplementation2<BlendNode,
                              iSkeletonBlendNode2,
                              scfFakeInterface<iSkeletonAnimationNode2> >
  {
  public:
    BlendNode (BlendNodeFactory* factory, iSkeleton2* skeleton);

    //-- iSkeletonBlendNode2
    virtual void SetNodeWeight (uint node, float weight);
    virtual void NormalizeWeights ();

    //-- iSkeletonAnimationNode2
    virtual void BlendState (csSkeletalState2* state, float baseWeight = 1.0f);
    virtual void TickAnimation (float dt);
    virtual bool IsActive () const;
    virtual iSkeletonAnimationNodeFactory2* GetFactory () const;
  
  private:

    csRefArray<iSkeletonAnimationNode2> subNodes;
    csArray<float> weightList;

    BlendNodeFactory* factory;

  };

}
CS_PLUGIN_NAMESPACE_END(Skeleton2)


#endif
