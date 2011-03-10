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

#ifndef __CS_OTHERNODES_H__
#define __CS_OTHERNODES_H__

#include "csutil/scf_implementation.h"
#include "imesh/skeleton2.h"
#include "imesh/animnode/skeleton2anim.h"
#include "csutil/leakguard.h"
#include "csutil/refarr.h"
#include "csutil/csstring.h"

#include "nodes.h"

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{
  class PriorityNodeFactory :
    public scfImplementation2<PriorityNodeFactory,
                              CS::Animation::iSkeletonPriorityNodeFactory,
                              scfFakeInterface<CS::Animation::iSkeletonAnimNodeFactory> >,
    public BaseFactoryChildren
  {
  public:
    CS_LEAKGUARD_DECLARE(PriorityNodeFactory);
  
    PriorityNodeFactory (const char* name);

    //-- CS::Animation::iSkeletonPriorityNodeFactory
    virtual void AddNode (CS::Animation::iSkeletonAnimNodeFactory* node, unsigned int priority);
    virtual void SetNodePriority (uint node, unsigned int priority);  
    virtual CS::Animation::iSkeletonAnimNodeFactory* GetNode (uint node);
    virtual uint GetNodeCount () const;
    virtual void ClearNodes ();

    //-- CS::Animation::iSkeletonAnimationNodeFactory
    virtual csPtr<CS::Animation::iSkeletonAnimNode> CreateInstance (
      CS::Animation::iSkeletonAnimPacket* packet, CS::Animation::iSkeleton* skeleton);
    virtual const char* GetNodeName () const;
    virtual CS::Animation::iSkeletonAnimNodeFactory* FindNode (const char* name);

  private:
    csString name;

    csArray<size_t> priorityList;
    friend class PriorityNode;
  };

  class PriorityNode :
    public scfImplementation2<PriorityNode,
                              CS::Animation::iSkeletonPriorityNode,
                              scfFakeInterface<CS::Animation::iSkeletonAnimNode> >,
    public BaseNodeChildren
  {
  public:
    CS_LEAKGUARD_DECLARE(PriorityNode);
  
    PriorityNode (PriorityNodeFactory* factory);

    //-- CS::Animation::iSkeletonPriorityNode
    virtual void SetNodePriority (uint node, unsigned int priority);  

    //-- CS::Animation::iSkeletonAnimationNode
    virtual void Play ();
    virtual void Stop ();
    virtual void SetPlaybackPosition (float time);
    virtual float GetPlaybackPosition () const;
    virtual float GetDuration () const;
    virtual void SetPlaybackSpeed (float speed);
    virtual float GetPlaybackSpeed () const;
    virtual void BlendState (CS::Animation::AnimatedMeshState* state, float baseWeight = 1.0f);
    virtual void TickAnimation (float dt);
    virtual bool IsActive () const;
    virtual CS::Animation::iSkeletonAnimNodeFactory* GetFactory () const;
    virtual CS::Animation::iSkeletonAnimNode* FindNode (const char* name);
    virtual void AddAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback);
    virtual void RemoveAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback);

  private:
    void UpdateIndexList ();

    csArray<size_t> priorityList;
    csArray<size_t> indexList;
    float playbackSpeed;

    csRef<PriorityNodeFactory> factory;
  };

  //----------------------------------------
  class RandomNodeFactory :
    public scfImplementation2<RandomNodeFactory,
                              CS::Animation::iSkeletonRandomNodeFactory,
                              scfFakeInterface<CS::Animation::iSkeletonAnimNodeFactory> >,
    public BaseFactoryChildren
  {
  public:
    CS_LEAKGUARD_DECLARE(RandomNodeFactory);
  
    RandomNodeFactory (const char* name);

    //-- CS::Animation::iSkeletonPriorityNodeFactory
    virtual void AddNode (CS::Animation::iSkeletonAnimNodeFactory* node, float probability);
    virtual void SetNodeProbability (uint node, float probability);  
    virtual void SetAutomaticSwitch (bool automatic);
    virtual CS::Animation::iSkeletonAnimNodeFactory* GetNode (uint node);
    virtual uint GetNodeCount () const;
    virtual void ClearNodes ();

    //-- CS::Animation::iSkeletonAnimationNodeFactory
    virtual csPtr<CS::Animation::iSkeletonAnimNode> CreateInstance (
      CS::Animation::iSkeletonAnimPacket* packet, CS::Animation::iSkeleton* skeleton);
    virtual const char* GetNodeName () const;
    virtual CS::Animation::iSkeletonAnimNodeFactory* FindNode (const char* name);

  private:
    void BuildAccumList ();

    csString name;

    csArray<float> probabilityList;
    csArray<float> accumProbabilityList;
    bool autoSwitch;
    
    friend class RandomNode;
  };

  class RandomNode :
    public scfImplementation2<RandomNode,
                              CS::Animation::iSkeletonRandomNode,
                              scfFakeInterface<CS::Animation::iSkeletonAnimNode> >,
    public BaseNodeChildren
  {
  public:
    CS_LEAKGUARD_DECLARE(RandomNode);
  
    RandomNode (RandomNodeFactory* factory);

    //-- CS::Animation::iSkeletonPriorityNode
    virtual void Switch ();
    virtual CS::Animation::iSkeletonAnimNode* GetCurrentNode () const;

    //-- CS::Animation::iSkeletonAnimationNode
    virtual void Play ();
    virtual void Stop ();
    virtual void SetPlaybackPosition (float time);
    virtual float GetPlaybackPosition () const;
    virtual float GetDuration () const;
    virtual void SetPlaybackSpeed (float speed);
    virtual float GetPlaybackSpeed () const;
    virtual void BlendState (CS::Animation::AnimatedMeshState* state, float baseWeight = 1.0f);
    virtual void TickAnimation (float dt);
    virtual bool IsActive () const;
    virtual CS::Animation::iSkeletonAnimNodeFactory* GetFactory () const;
    virtual CS::Animation::iSkeletonAnimNode* FindNode (const char* name);
    virtual void AddAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback);
    virtual void RemoveAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback);

    //-- BaseNodeChildren
    virtual void AnimationFinished (CS::Animation::iSkeletonAnimNode* node);
    virtual void PlayStateChanged (CS::Animation::iSkeletonAnimNode* node, bool isPlaying);
    virtual void DurationChanged (CS::Animation::iSkeletonAnimNode* node);

  private:
    size_t currentNode;
    bool active;

    float playbackSpeed;

    csRef<RandomNodeFactory> factory;

    friend class RandomNodeFactory;
  };

}
CS_PLUGIN_NAMESPACE_END(Skeleton2)


#endif
