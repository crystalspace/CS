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

#ifndef __CS_BLENDNODE_H__
#define __CS_BLENDNODE_H__

#include "csutil/scf_implementation.h"
#include "imesh/skeleton2.h"
#include "imesh/skeleton2anim.h"
#include "csutil/leakguard.h"
#include "csutil/refarr.h"
#include "csutil/csstring.h"

#include "nodes.h"

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{
  
  class BlendNodeFactory :
    public scfImplementation2<BlendNodeFactory,
                              CS::Animation::iSkeletonBlendNodeFactory2,
                              scfFakeInterface<CS::Animation::iSkeletonAnimNodeFactory2> >,
    public BaseFactoryChildren
  {
  public:
    CS_LEAKGUARD_DECLARE(BlendNodeFactory);
  
    BlendNodeFactory (const char* name);

    //-- CS::Animation::iSkeletonBlendNodeFactory2
    virtual void AddNode (CS::Animation::iSkeletonAnimNodeFactory2* node, float weight);
    virtual void SetNodeWeight (uint node, float weight);
    virtual void NormalizeWeights ();
    virtual CS::Animation::iSkeletonAnimNodeFactory2* GetNode (uint node);
    virtual uint GetNodeCount () const;
    virtual void ClearNodes ();
    virtual void SetSynchronizationMode (CS::Animation::SynchronizationMode mode);
    virtual CS::Animation::SynchronizationMode GetSynchronizationMode () const;

    //-- CS::Animation::iSkeletonAnimationNodeFactory2
    virtual csPtr<CS::Animation::iSkeletonAnimNode2> CreateInstance (
      CS::Animation::iSkeletonAnimPacket2* packet, CS::Animation::iSkeleton2* skeleton);
    virtual const char* GetNodeName () const;
    virtual CS::Animation::iSkeletonAnimNodeFactory2* FindNode (const char* name);

  private:    
    csString name;
    CS::Animation::SynchronizationMode syncMode;
    csArray<float> weightList;
    friend class BlendNode;
  };

  class BlendNode : 
    public scfImplementation2<BlendNode,
                              CS::Animation::iSkeletonBlendNode2,
                              scfFakeInterface<CS::Animation::iSkeletonAnimNode2> >,
    public BaseNodeChildren
  {
  public:
    CS_LEAKGUARD_DECLARE(BlendNode);
  
    BlendNode (BlendNodeFactory* factory);

    //-- CS::Animation::iSkeletonBlendNode2
    virtual void SetNodeWeight (uint node, float weight);
    virtual void NormalizeWeights ();

    //-- CS::Animation::iSkeletonAnimationNode2
    virtual void Play ();
    virtual void Stop ();
    virtual void SetPlaybackPosition (float time);
    virtual float GetPlaybackPosition () const;
    virtual float GetDuration () const;
    virtual void SetPlaybackSpeed (float speed);
    virtual float GetPlaybackSpeed () const;
    virtual void BlendState (CS::Animation::csSkeletalState2* state, float baseWeight = 1.0f);
    virtual void TickAnimation (float dt);
    virtual bool IsActive () const;
    virtual CS::Animation::iSkeletonAnimNodeFactory2* GetFactory () const;
    virtual CS::Animation::iSkeletonAnimNode2* FindNode (const char* name);
    virtual void AddAnimationCallback (CS::Animation::iSkeletonAnimCallback2* callback);
    virtual void RemoveAnimationCallback (CS::Animation::iSkeletonAnimCallback2* callback);
  
    //-- BaseNodeChildren
    virtual void PlayStateChanged (CS::Animation::iSkeletonAnimNode2* node, bool isPlaying);
    virtual void DurationChanged (CS::Animation::iSkeletonAnimNode2* node);

  private:    
    void SynchronizeSubnodes ();

    csArray<float> weightList;
    csArray<float> virtualSubSpeed;
    csBitArray lastSyncNodes;

    csRef<BlendNodeFactory> factory;

    float playbackSpeed;
    float virtualDuration;
  };

}
CS_PLUGIN_NAMESPACE_END(Skeleton2)

#endif
