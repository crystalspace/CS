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
#include "imesh/skeleton2anim.h"
#include "csutil/refarr.h"
#include "csutil/csstring.h"

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{
  /**
   * Helper class for nodes having no children
   */
  class BaseNodeSingle
  {
  protected:
    BaseNodeSingle (iSkeletonAnimNode2* owner)
      : owner (owner)
    {}
    
    void FireAnimationFinishedCb ()
    {
      for (size_t i = 0; i < callbacks.GetSize (); ++i)
      {
        callbacks[i]->AnimationFinished (owner);
      }
    }

    void FireAnimationCycleCb ()
    {
      for (size_t i = 0; i < callbacks.GetSize (); ++i)
      {
        callbacks[i]->AnimationCycled (owner);
      }
    }

    void FireStateChangeCb (bool playing)
    {
      for (size_t i = 0; i < callbacks.GetSize (); ++i)
      {
        callbacks[i]->PlayStateChanged (owner, playing);
      }
    }

    void FireDurationChangeCb ()
    {
      for (size_t i = 0; i < callbacks.GetSize (); ++i)
      {
        callbacks[i]->DurationChanged (owner);
      }
    }

    void AddAnimationCallback (iSkeletonAnimCallback2* callback)
    {
      callbacks.PushSmart (callback);
    }

    void RemoveAnimationCallback (iSkeletonAnimCallback2* callback)
    {
      callbacks.Delete (callback);
    }    
    
    csRefArray<iSkeletonAnimCallback2> callbacks;
    iSkeletonAnimNode2* owner;
  };


  /**
   * Helper class for nodes having one or more children
   */
  class BaseNodeChildren : public BaseNodeSingle
  {
  protected:
    BaseNodeChildren (iSkeletonAnimNode2* owner)
      : BaseNodeSingle (owner), manualCbInstall (false)
    {}

    void AddAnimationCallback (iSkeletonAnimCallback2* callback);
    void RemoveAnimationCallback (iSkeletonAnimCallback2* callback);

    void InstallInnerCb (bool manual);
    void RemoveInnerCb (bool manual);

    virtual void AnimationFinished (iSkeletonAnimNode2* node);
    virtual void AnimationCycled (iSkeletonAnimNode2* node);
    virtual void PlayStateChanged (iSkeletonAnimNode2* node, bool isPlaying);
    virtual void DurationChanged (iSkeletonAnimNode2* node);

    class InnerCallback : public scfImplementation1<InnerCallback,
                                                    iSkeletonAnimCallback2>
    {
    public:
      InnerCallback (BaseNodeChildren* parent);

      //-- iSkeletonAnimCallback2
      virtual void AnimationFinished (iSkeletonAnimNode2* node);
      virtual void AnimationCycled (iSkeletonAnimNode2* node);
      virtual void PlayStateChanged (iSkeletonAnimNode2* node, bool isPlaying);
      virtual void DurationChanged (iSkeletonAnimNode2* node);

    private:
      BaseNodeChildren* parent;
    };

    csRef<InnerCallback> cb;
    csRefArray<iSkeletonAnimNode2> subNodes;
    bool manualCbInstall;
    friend class BaseFactoryChildren; 
  };

  /**
   * Helper class for factories having multiple children
   */
  class BaseFactoryChildren
  {
  protected:
    void SetupInstance (BaseNodeChildren* child, iSkeletonAnimPacket2* packet, 
      iSkeleton2* skeleton)
    {
      for (size_t i = 0; i < subFactories.GetSize (); ++i)
      {
        csRef<iSkeletonAnimNode2> node = 
          subFactories[i]->CreateInstance (packet, skeleton);
        child->subNodes.Push (node);
      }
    }

    csRefArray<iSkeletonAnimNodeFactory2> subFactories;
  };



  //----------------------------------------

  class AnimationNode;

  class AnimationNodeFactory :
    public scfImplementation2<AnimationNodeFactory,
                              iSkeletonAnimationNodeFactory2,
                              scfFakeInterface<iSkeletonAnimNodeFactory2> >
  {
  public:
    AnimationNodeFactory (const char* name);

    //-- iSkeletonAnimationNodeFactory2
    virtual void SetAnimation (iSkeletonAnimation2* animation);
    virtual iSkeletonAnimation2* GetAnimation () const;
    virtual void SetCyclic (bool cyclic);
    virtual bool IsCyclic () const;
    virtual void SetPlaybackSpeed (float speed);
    virtual float GetPlaybackSpeed () const;
    virtual void SetAutomaticReset (bool reset);
    virtual bool GetAutomaticReset () const;

    //-- iSkeletonAnimNodeFactory2
    virtual csPtr<iSkeletonAnimNode2> CreateInstance (
      iSkeletonAnimPacket2* packet, iSkeleton2* skeleton);
    virtual const char* GetNodeName () const;
    virtual iSkeletonAnimNodeFactory2* FindNode (const char* name);

  private:
    csString name;

    csRef<iSkeletonAnimation2> animation;
    bool cyclic, automaticReset;
    float playbackSpeed, animationDuration;

    friend class AnimationNode;
  };

  class AnimationNode :
    public scfImplementation2<AnimationNode,
                              iSkeletonAnimationNode2,
                              scfFakeInterface<iSkeletonAnimNode2> >,
    public BaseNodeSingle
  {
  public:
    AnimationNode (AnimationNodeFactory* factory);

    //-- iSkeletonAnimationNode2
    

    //-- iSkeletonAnimNode2
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
    csRef<AnimationNodeFactory> factory;

    bool isPlaying;
    float playbackPosition;
    float playbackSpeed;
  };

  //----------------------------------------

  class BlendNodeFactory :
    public scfImplementation2<BlendNodeFactory,
                              iSkeletonBlendNodeFactory2,
                              scfFakeInterface<iSkeletonAnimNodeFactory2> >,
    public BaseFactoryChildren
  {
  public:
    BlendNodeFactory (const char* name);

    //-- iSkeletonBlendNodeFactory2
    virtual void AddNode (iSkeletonAnimNodeFactory2* node, float weight);
    virtual void SetNodeWeight (uint node, float weight);
    virtual void NormalizeWeights ();
    virtual iSkeletonAnimNodeFactory2* GetNode (uint node);
    virtual uint GetNodeCount () const;
    virtual void ClearNodes ();
    virtual void SetSynchronizationMode (CS::Animation::SynchronizationMode mode);
    virtual CS::Animation::SynchronizationMode GetSynchronizationMode () const;

    //-- iSkeletonAnimationNodeFactory2
    virtual csPtr<iSkeletonAnimNode2> CreateInstance (
      iSkeletonAnimPacket2* packet, iSkeleton2* skeleton);
    virtual const char* GetNodeName () const;
    virtual iSkeletonAnimNodeFactory2* FindNode (const char* name);

  private:    
    csString name;
    CS::Animation::SynchronizationMode syncMode;
    csArray<float> weightList;
    friend class BlendNode;
  };

  class BlendNode : 
    public scfImplementation2<BlendNode,
                              iSkeletonBlendNode2,
                              scfFakeInterface<iSkeletonAnimNode2> >,
    public BaseNodeChildren
  {
  public:
    BlendNode (BlendNodeFactory* factory);

    //-- iSkeletonBlendNode2
    virtual void SetNodeWeight (uint node, float weight);
    virtual void NormalizeWeights ();

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
    virtual void PlayStateChanged (iSkeletonAnimNode2* node, bool isPlaying);
    virtual void DurationChanged (iSkeletonAnimNode2* node);

  private:    
    void SynchronizeSubnodes ();

    csArray<float> weightList;
    csArray<float> virtualSubSpeed;
    csBitArray lastSyncNodes;

    csRef<BlendNodeFactory> factory;

    float playbackSpeed;
    float virtualDuration;
  };


  //----------------------------------------
  class PriorityNodeFactory :
    public scfImplementation2<PriorityNodeFactory,
                              iSkeletonPriorityNodeFactory2,
                              scfFakeInterface<iSkeletonAnimNodeFactory2> >,
    public BaseFactoryChildren
  {
  public:
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


  //----------------------------------------

  class FSMNodeFactory :
    public scfImplementation2<FSMNodeFactory,
                              iSkeletonFSMNodeFactory2,
                              scfFakeInterface<iSkeletonAnimNodeFactory2> >
  {
  public:
    FSMNodeFactory (const char* name);

    //-- iSkeletonFSMNodeFactory2
    virtual CS::Animation::StateID AddState (); 
    virtual void SetStateNode (CS::Animation::StateID id, 
      iSkeletonAnimNodeFactory2* nodeFact);
    virtual iSkeletonAnimNodeFactory2* GetStateNode (CS::Animation::StateID id) const;
    virtual void SetStateName (CS::Animation::StateID id, const char* name);
    virtual const char* GetStateName (CS::Animation::StateID id) const;
    virtual CS::Animation::StateID FindState (const char* name) const;
    virtual void SetStartState (CS::Animation::StateID id);
    virtual CS::Animation::StateID GetStartState () const;
    virtual uint GetStateCount () const;
    virtual void ClearStates ();

    //-- iSkeletonAnimationNodeFactory2
    virtual csPtr<iSkeletonAnimNode2> CreateInstance (
      iSkeletonAnimPacket2* packet, iSkeleton2* skeleton);
    virtual const char* GetNodeName () const;
    virtual iSkeletonAnimNodeFactory2* FindNode (const char* name);

  private:
    struct State
    {
      csRef<iSkeletonAnimNodeFactory2> nodeFactory;
      csString name;
    };

    csString name;
    csArray<State> stateList;
    CS::Animation::StateID startState;

    friend class FSMNode;
  };

  class FSMNode :
    public scfImplementation2<FSMNode,
                              iSkeletonFSMNode2,
                              scfFakeInterface<iSkeletonAnimNode2> >,
    public BaseNodeSingle
  {
  public:
    FSMNode (FSMNodeFactory* factory);

    //-- iSkeletonFSMNode2
    virtual void SwitchToState (CS::Animation::StateID newState);
    virtual CS::Animation::StateID GetCurrentState () const;

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
    struct State
    {
      csRef<iSkeletonAnimNode2> stateNode;
    };

    csRef<FSMNodeFactory> factory;
    csArray<State> stateList;
    CS::Animation::StateID currentState;
    bool isActive;

    float playbackSpeed;

    friend class FSMNodeFactory;
  };
                          

}
CS_PLUGIN_NAMESPACE_END(Skeleton2)


#endif
