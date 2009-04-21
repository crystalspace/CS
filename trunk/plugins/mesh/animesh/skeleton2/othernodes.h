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
#include "imesh/skeleton2anim.h"
#include "csutil/leakguard.h"
#include "csutil/refarr.h"
#include "csutil/csstring.h"

#include "nodes.h"

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{
  class PriorityNodeFactory :
    public scfImplementation2<PriorityNodeFactory,
                              iSkeletonPriorityNodeFactory2,
                              scfFakeInterface<iSkeletonAnimNodeFactory2> >,
    public BaseFactoryChildren
  {
  public:
    CS_LEAKGUARD_DECLARE(PriorityNodeFactory);
  
    PriorityNodeFactory (const char* name);

    //-- iSkeletonPriorityNodeFactory2
    virtual void AddNode (iSkeletonAnimNodeFactory2* node, unsigned int priority);
    virtual void SetNodePriority (uint node, unsigned int priority);  
    virtual iSkeletonAnimNodeFactory2* GetNode (uint node);
    virtual uint GetNodeCount () const;
    virtual void ClearNodes ();

    //-- iSkeletonAnimationNodeFactory2
    virtual csPtr<iSkeletonAnimNode2> CreateInstance (
      iSkeletonAnimPacket2* packet, iSkeleton2* skeleton);
    virtual const char* GetNodeName () const;
    virtual iSkeletonAnimNodeFactory2* FindNode (const char* name);

  private:
    csString name;

    csArray<size_t> priorityList;
    friend class PriorityNode;
  };

  class PriorityNode :
    public scfImplementation2<PriorityNode,
                              iSkeletonPriorityNode2,
                              scfFakeInterface<iSkeletonAnimNode2> >,
    public BaseNodeChildren
  {
  public:
    CS_LEAKGUARD_DECLARE(PriorityNode);
  
    PriorityNode (PriorityNodeFactory* factory);

    //-- iSkeletonPriorityNode2
    virtual void SetNodePriority (uint node, unsigned int priority);  

    //-- iSkeletonAnimationNode2
    virtual void Play ();
    virtual void Stop ();
    virtual void SetPlaybackPosition (float time);
    virtual float GetPlaybackPosition () const;
    virtual float GetDuration () const;
    virtual void SetPlaybackSpeed (float speed);
    virtual float GetPlaybackSpeed () const;
    virtual void BlendState (csSkeletalState2* state, float baseWeight = 1.0f);
    virtual void TickAnimation (float dt);
    virtual bool IsActive () const;
    virtual iSkeletonAnimNodeFactory2* GetFactory () const;
    virtual iSkeletonAnimNode2* FindNode (const char* name);
    virtual void AddAnimationCallback (iSkeletonAnimCallback2* callback);
    virtual void RemoveAnimationCallback (iSkeletonAnimCallback2* callback);

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
                              iSkeletonRandomNodeFactory2,
                              scfFakeInterface<iSkeletonAnimNodeFactory2> >,
    public BaseFactoryChildren
  {
  public:
    CS_LEAKGUARD_DECLARE(RandomNodeFactory);
  
    RandomNodeFactory (const char* name);

    //-- iSkeletonPriorityNodeFactory2
    virtual void AddNode (iSkeletonAnimNodeFactory2* node, float probability);
    virtual void SetNodeProbability (uint node, float probability);  
    virtual void SetAutomaticSwitch (bool automatic);
    virtual iSkeletonAnimNodeFactory2* GetNode (uint node);
    virtual uint GetNodeCount () const;
    virtual void ClearNodes ();

    //-- iSkeletonAnimationNodeFactory2
    virtual csPtr<iSkeletonAnimNode2> CreateInstance (
      iSkeletonAnimPacket2* packet, iSkeleton2* skeleton);
    virtual const char* GetNodeName () const;
    virtual iSkeletonAnimNodeFactory2* FindNode (const char* name);

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
                              iSkeletonRandomNode2,
                              scfFakeInterface<iSkeletonAnimNode2> >,
    public BaseNodeChildren
  {
  public:
    CS_LEAKGUARD_DECLARE(RandomNode);
  
    RandomNode (RandomNodeFactory* factory);

    //-- iSkeletonPriorityNode2
    virtual void Switch ();
    virtual iSkeletonAnimNode2* GetCurrentNode () const;

    //-- iSkeletonAnimationNode2
    virtual void Play ();
    virtual void Stop ();
    virtual void SetPlaybackPosition (float time);
    virtual float GetPlaybackPosition () const;
    virtual float GetDuration () const;
    virtual void SetPlaybackSpeed (float speed);
    virtual float GetPlaybackSpeed () const;
    virtual void BlendState (csSkeletalState2* state, float baseWeight = 1.0f);
    virtual void TickAnimation (float dt);
    virtual bool IsActive () const;
    virtual iSkeletonAnimNodeFactory2* GetFactory () const;
    virtual iSkeletonAnimNode2* FindNode (const char* name);
    virtual void AddAnimationCallback (iSkeletonAnimCallback2* callback);
    virtual void RemoveAnimationCallback (iSkeletonAnimCallback2* callback);

    //-- BaseNodeChildren
    virtual void AnimationFinished (iSkeletonAnimNode2* node);
    virtual void PlayStateChanged (iSkeletonAnimNode2* node, bool isPlaying);
    virtual void DurationChanged (iSkeletonAnimNode2* node);

  private:
    size_t currentNode;
    bool active;

    float playbackSpeed;

    csRef<RandomNodeFactory> factory;
  };

}
CS_PLUGIN_NAMESPACE_END(Skeleton2)


#endif
