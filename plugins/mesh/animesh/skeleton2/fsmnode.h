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

#ifndef __CS_FSMNODE_H__
#define __CS_FSMNODE_H__

#include "csutil/fifo.h"
#include "csutil/csstring.h"
#include "csutil/hash.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"
#include "imesh/skeleton2.h"
#include "imesh/skeleton2anim.h"
#include "csutil/leakguard.h"
#include "csutil/refarr.h"
#include "csutil/csstring.h"

#include "nodes.h"

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{
  class AnimationFifoCb
  {
  public:
    virtual void NewAnimation (size_t data) = 0;
  protected:
    virtual ~AnimationFifoCb() {}
  };

  class AnimationFifo
  {
  public:
    AnimationFifo (AnimationFifoCb* cb);

    void PushAnimation (iSkeletonAnimNode2* node, bool directSwitch,
      float blendInTime = 0.0f, size_t cbData = ~0);

    void BlendState (csSkeletalState2* state, float baseWeight = 1.0f);
    void TickAnimation (float dt);

    void Stop ();

  private:
    struct AnimationInstruction
    {
      csRef<iSkeletonAnimNode2> node;
      size_t cbData;
      float blendInTime;
      bool directSwitch;
    };

    enum
    { 
      STATE_STOPPED,
      STATE_PLAYING,      
      STATE_BLENDING
    } currentState;

    AnimationFifoCb* cb;

    csFIFO<AnimationInstruction> instructions;
    AnimationInstruction currentAnimation;
    float blendTime;
  };


  class FSMNodeFactory :
    public scfImplementation2<FSMNodeFactory,
                              iSkeletonFSMNodeFactory2,
                              scfFakeInterface<iSkeletonAnimNodeFactory2> >
  {
  public:
    CS_LEAKGUARD_DECLARE(FSMNodeFactory);
  
    FSMNodeFactory (const char* name);

    //-- iSkeletonFSMNodeFactory2
    virtual CS::Animation::StateID AddState (); 
    virtual CS::Animation::StateID AddState (const char* name, iSkeletonAnimNodeFactory2 *nodeFact);
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
    virtual void SetStateTransition (CS::Animation::StateID fromState, 
      CS::Animation::StateID toState, iSkeletonAnimNodeFactory2* fact);
    virtual void SetTransitionCrossfade (CS::Animation::StateID fromState, 
      CS::Animation::StateID toState, float time1, float time2);

    //-- iSkeletonAnimationNodeFactory2
    virtual csPtr<iSkeletonAnimNode2> CreateInstance (
      iSkeletonAnimPacket2* packet, iSkeleton2* skeleton);
    virtual const char* GetNodeName () const;
    virtual iSkeletonAnimNodeFactory2* FindNode (const char* name);

    struct State
    {
      csRef<iSkeletonAnimNodeFactory2> nodeFactory;
      csString name;
    };

    struct StateTransitionKey
    {
      CS::Animation::StateID fromState, toState;

      bool operator< (const StateTransitionKey& other) const
      {
        return (fromState < other.fromState) || 
          (fromState == other.fromState && toState < other.toState);          
      }
    };

    struct StateTransitionInfo
    {
      StateTransitionInfo ()
        : time1 (0.0f), time2 (0.0f), directSwitch (true)
      {}

      csRef<iSkeletonAnimNodeFactory2> nodeFactory;
      float time1, time2;
      bool directSwitch;
    };

  private:
   
    csString name;
    csArray<State> stateList;
    csHash<StateTransitionInfo, StateTransitionKey> transitions;

    CS::Animation::StateID startState;

    friend class FSMNode;
  };

  class FSMNode :
    public scfImplementation2<FSMNode,
                              iSkeletonFSMNode2,
                              scfFakeInterface<iSkeletonAnimNode2> >,
    public BaseNodeSingle,
    public AnimationFifoCb
  {
  public:
    CS_LEAKGUARD_DECLARE(FSMNode);
  
    FSMNode (FSMNodeFactory* factory);

    //-- iSkeletonFSMNode2
    virtual void SwitchToState (CS::Animation::StateID newState);
    virtual CS::Animation::StateID GetCurrentState () const;
    virtual iSkeletonAnimNode2* GetStateNode (CS::Animation::StateID state) const;

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

    //-- AnimationFifoCb
    virtual void NewAnimation (size_t data);
  private:
    struct State
    {
      csRef<iSkeletonAnimNode2> stateNode;
    };

    struct StateTransitionInfo
    {
      StateTransitionInfo ()
        : time1 (0.0f), time2 (0.0f), directSwitch (true)
      {}      

      csRef<iSkeletonAnimNode2> transitionNode;
      float time1, time2;
      bool directSwitch;
    };

    csRef<FSMNodeFactory> factory;
    csArray<State> stateList;
    csHash<StateTransitionInfo, FSMNodeFactory::StateTransitionKey> transitions;

    CS::Animation::StateID currentState;    
    float playbackSpeed;

    bool isActive;   
    AnimationFifo blendFifo;


    friend class FSMNodeFactory;
  };

}
CS_PLUGIN_NAMESPACE_END(Skeleton2)

#endif
