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
    virtual void InstructionQueueEmpty (size_t data) = 0;
  protected:
    virtual ~AnimationFifoCb() {}
  };

  class AnimationFifo
  {
  public:
    AnimationFifo (AnimationFifoCb* cb);

    void PushAnimation (CS::Animation::iSkeletonAnimNode2* node, bool directSwitch,
      float blendInTime = 0.0f, size_t cbData = ~0);
    void RemoveAnimations (size_t cbData);

    void BlendState (CS::Animation::csSkeletalState2* state, float baseWeight = 1.0f);
    void TickAnimation (float dt);

    void Stop ();

  private:
    struct AnimationInstruction
    {
      csRef<CS::Animation::iSkeletonAnimNode2> node;
      size_t cbData;
      float blendInTime;
      bool directSwitch;

      bool operator== (const AnimationInstruction& other) const
      {
        return cbData == other.cbData;
      }
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
                              CS::Animation::iSkeletonFSMNodeFactory2,
                              scfFakeInterface<CS::Animation::iSkeletonAnimNodeFactory2> >
  {
  public:
    CS_LEAKGUARD_DECLARE(FSMNodeFactory);
  
    FSMNodeFactory (const char* name);

    //-- CS::Animation::iSkeletonFSMNodeFactory2
    virtual CS::Animation::StateID AddState (); 
    virtual CS::Animation::StateID AddState (const char* name, CS::Animation::iSkeletonAnimNodeFactory2 *nodeFact);
    virtual void SetStateNode (CS::Animation::StateID id, 
      CS::Animation::iSkeletonAnimNodeFactory2* nodeFact);
    virtual CS::Animation::iSkeletonAnimNodeFactory2* GetStateNode (CS::Animation::StateID id) const;
    virtual void SetStateName (CS::Animation::StateID id, const char* name);
    virtual const char* GetStateName (CS::Animation::StateID id) const;
    virtual CS::Animation::StateID FindState (const char* name) const;
    virtual void SetStartState (CS::Animation::StateID id);
    virtual CS::Animation::StateID GetStartState () const;
    virtual uint GetStateCount () const;
    virtual void ClearStates ();
    virtual void SetStateTransition (CS::Animation::StateID fromState, 
      CS::Animation::StateID toState, CS::Animation::iSkeletonAnimNodeFactory2* fact);
    virtual void SetTransitionCrossfade (CS::Animation::StateID fromState, 
      CS::Animation::StateID toState, float time1, float time2);
    virtual void SetAutomaticTransition (CS::Animation::StateID fromState, 
      CS::Animation::StateID toState, bool automatic);

    //-- CS::Animation::iSkeletonAnimationNodeFactory2
    virtual csPtr<CS::Animation::iSkeletonAnimNode2> CreateInstance (
      CS::Animation::iSkeletonAnimPacket2* packet, CS::Animation::iSkeleton2* skeleton);
    virtual const char* GetNodeName () const;
    virtual CS::Animation::iSkeletonAnimNodeFactory2* FindNode (const char* name);

    struct State
    {
      csRef<CS::Animation::iSkeletonAnimNodeFactory2> nodeFactory;
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
      : time1 (0.0f), time2 (0.0f)//, directSwitch (true)
      {}

      csRef<CS::Animation::iSkeletonAnimNodeFactory2> nodeFactory;
      float time1, time2;
      //bool directSwitch;
    };

  private:
   
    csString name;
    csArray<State> stateList;
    csHash<StateTransitionInfo, StateTransitionKey> transitions;
    csHash<CS::Animation::StateID, CS::Animation::StateID> automaticTransitions;

    CS::Animation::StateID startState;

    friend class FSMNode;
  };

  class FSMNode :
    public scfImplementation2<FSMNode,
                              CS::Animation::iSkeletonFSMNode2,
                              scfFakeInterface<CS::Animation::iSkeletonAnimNode2> >,
    public BaseNodeSingle,
    public AnimationFifoCb
  {
  public:
    CS_LEAKGUARD_DECLARE(FSMNode);
  
    FSMNode (FSMNodeFactory* factory);

    //-- CS::Animation::iSkeletonFSMNode2
    virtual void SwitchToState (CS::Animation::StateID newState);
    void SwitchToState (CS::Animation::StateID newState, bool directSwitch);
    virtual CS::Animation::StateID GetCurrentState () const;
    virtual CS::Animation::iSkeletonAnimNode2* GetStateNode (CS::Animation::StateID state) const;

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

    //-- AnimationFifoCb
    virtual void NewAnimation (size_t data);
    virtual void InstructionQueueEmpty (size_t data);
  private:
    struct State
    {
      csRef<CS::Animation::iSkeletonAnimNode2> stateNode;
    };

    struct StateTransitionInfo
    {
      StateTransitionInfo ()
      : time1 (0.0f), time2 (0.0f)//, directSwitch (true)
      {}      

      csRef<CS::Animation::iSkeletonAnimNode2> transitionNode;
      float time1, time2;
      //bool directSwitch;
    };

    csRef<FSMNodeFactory> factory;
    csArray<State> stateList;
    csHash<StateTransitionInfo, FSMNodeFactory::StateTransitionKey> transitions;
    csHash<CS::Animation::StateID, CS::Animation::StateID> automaticTransitions;

    CS::Animation::StateID currentState;    
    CS::Animation::StateID automaticState;    
    float playbackSpeed;

    bool isActive;   
    AnimationFifo blendFifo;

    friend class FSMNodeFactory;
  };

}
CS_PLUGIN_NAMESPACE_END(Skeleton2)

#endif
