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

#include "csgeom/math.h"

#include "fsmnode.h"

template <>
class csHashComputer<CS::Plugin::Skeleton2::FSMNodeFactory::StateTransitionKey>
{
public:
  /// Compute a hash value for \a key.
  static uint ComputeHash (const CS::Plugin::Skeleton2::FSMNodeFactory::StateTransitionKey& key)
  {
    return (key.fromState ^ key.toState) ^ 0xABCDEF98;
  }
};


CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{
  AnimationFifo::AnimationFifo (AnimationFifoCb* cb)
    : currentState (STATE_STOPPED), cb (cb)
  {}

  void AnimationFifo::PushAnimation (CS::Animation::iSkeletonAnimNode* node, bool directSwitch,
    float blendInTime, size_t cbData)
  {
    if (!node)
      return;

    AnimationInstruction newInstr = {node, cbData, blendInTime, directSwitch};
    instructions.Push (newInstr);

    if (currentState == STATE_STOPPED)
    {
      currentState = STATE_PLAYING;
      currentAnimation = instructions.PopTop ();
      currentAnimation.node->Play ();

      if (cb)
	cb->NewAnimation (currentAnimation.cbData);
    }
  }

  void AnimationFifo::RemoveAnimations (size_t cbData)
  {
    AnimationInstruction instruction;
    instruction.cbData = cbData;

    // There can be at most two animations
    instructions.Delete (instruction);
    instructions.Delete (instruction);
  }

  void AnimationFifo::BlendState (CS::Animation::AnimatedMeshState* state, float baseWeight /* = 1.0f */)
  {
    switch (currentState)
    {
    case STATE_STOPPED:
      break;
    case STATE_PLAYING:
      {
        currentAnimation.node->BlendState (state, baseWeight);        
      }
      break;    
    case STATE_BLENDING:
      {
        if (instructions.GetSize () > 0)
        {
          AnimationInstruction& top = instructions.Top ();

          const float blendAmount = csClamp (blendTime / top.blendInTime, 1.0f, 0.0f);
          currentAnimation.node->BlendState (state, baseWeight);
          top.node->BlendState (state, baseWeight * blendAmount);
        }        
        else
        {
          currentAnimation.node->BlendState (state, baseWeight);
        }       
      }
      break;    
    }
  }

  void AnimationFifo::TickAnimation (float dt)
  {
    switch (currentState)
    {
    case STATE_STOPPED:
      break;
    case STATE_PLAYING:
      {
        if (instructions.GetSize () > 0)
        {
          AnimationInstruction& top = instructions.Top ();

          if (top.directSwitch ||
	      currentAnimation.node->GetPlaybackPosition ()
	      + dt * currentAnimation.node->GetPlaybackSpeed ()
	      + top.blendInTime
	      > currentAnimation.node->GetDuration ())
          {
            if (top.blendInTime > SMALL_EPSILON)
            {
              // Blend
              currentState = STATE_BLENDING;
              blendTime = 0;
              top.node->Play ();

              if (cb)
                cb->NewAnimation (top.cbData);
            }
            else
            {
              // Switch
	      if (currentAnimation.node)
		currentAnimation.node->Stop ();
              currentAnimation = instructions.PopTop ();
              currentAnimation.node->Play ();

              if (cb)
                cb->NewAnimation (currentAnimation.cbData);
            }
          }
        }

        currentAnimation.node->TickAnimation (dt);        
      }
      break;    
    case STATE_BLENDING:
      {
        AnimationInstruction& top = instructions.Top ();

        if (blendTime + dt > top.blendInTime)
        {
          // Blend over, finalize switch
          currentAnimation.node->TickAnimation (dt);
	  currentAnimation.node->Stop ();
          currentAnimation = instructions.PopTop ();
          if (cb)
          {
            cb->NewAnimation (currentAnimation.cbData);
          }

          currentAnimation.node->TickAnimation (dt);

          currentState = STATE_PLAYING;
        }
        else
        {
          // Tick both
          currentAnimation.node->TickAnimation (dt);
          top.node->TickAnimation (dt);
	  blendTime += dt;
        }
      }
      break;
    }

    if (cb && !instructions.GetSize ())
      cb->InstructionQueueEmpty (currentAnimation.cbData);
  }

  void AnimationFifo::Stop ()
  {
    currentState = STATE_STOPPED;
    currentAnimation.node = 0;
    instructions.DeleteAll ();
  }


  CS_LEAKGUARD_IMPLEMENT(FSMNodeFactory);

  FSMNodeFactory::FSMNodeFactory (const char* name)
    : scfImplementationType (this), name (name), 
    startState (CS::Animation::InvalidStateID)
  {}

  CS::Animation::StateID FSMNodeFactory::AddState ()
  {
    State newState;

    return (CS::Animation::StateID)stateList.Push (newState);
  }

  CS::Animation::StateID FSMNodeFactory::AddState (const char* name,
						   CS::Animation::iSkeletonAnimNodeFactory *nodeFact)
  {
    State newState;
    CS::Animation::StateID id = (CS::Animation::StateID)stateList.Push (newState);
    stateList[id].name = name;
    stateList[id].nodeFactory = nodeFact;
    return id;
  }

  void FSMNodeFactory::SetStateNode (CS::Animation::StateID id, 
    CS::Animation::iSkeletonAnimNodeFactory* nodeFact)
  {
    CS_ASSERT(id < stateList.GetSize ());

    stateList[id].nodeFactory = nodeFact;
  }

  CS::Animation::iSkeletonAnimNodeFactory* FSMNodeFactory::GetStateNode (CS::Animation::StateID id) const
  {
    CS_ASSERT(id < stateList.GetSize ());

    return stateList[id].nodeFactory;
  }

  void FSMNodeFactory::SetStateName (CS::Animation::StateID id, const char* name)
  {
    CS_ASSERT(id < stateList.GetSize ());

    stateList[id].name = name;
  }

  const char* FSMNodeFactory::GetStateName (CS::Animation::StateID id) const
  {
    CS_ASSERT(id < stateList.GetSize ());

    return stateList[id].name;
  }

  CS::Animation::StateID FSMNodeFactory::FindState (const char* name) const
  {
    for (size_t i = 0; i < stateList.GetSize (); ++i)
    {
      if (stateList[i].name == name)
      {
        return (CS::Animation::StateID)i;
      }
    }

    return CS::Animation::InvalidStateID;
  }

  void FSMNodeFactory::SetStartState (CS::Animation::StateID id)
  {
    CS_ASSERT(id < stateList.GetSize ());
    startState = id;
  }

  CS::Animation::StateID FSMNodeFactory::GetStartState () const
  {
    return startState;
  }

  uint FSMNodeFactory::GetStateCount () const
  {
    return (uint)stateList.GetSize ();
  }

  void FSMNodeFactory::ClearStates ()
  {
    stateList.DeleteAll ();
  }

  csPtr<CS::Animation::iSkeletonAnimNode> FSMNodeFactory::CreateInstance (
    CS::Animation::iSkeletonAnimPacket* packet, CS::Animation::iSkeleton* skeleton)
  {
    csRef<FSMNode> newn;

    newn.AttachNew (new FSMNode (this));

    // Setup nodes
    for (size_t i = 0; i < stateList.GetSize (); ++i)
    {
      FSMNode::State newState;
      if (stateList[i].nodeFactory)
	newState.stateNode = stateList[i].nodeFactory->CreateInstance (packet, skeleton);
      newn->stateList.Push (newState);
    }

    // Setup all the transitions
    {
      csHash<StateTransitionInfo, StateTransitionKey>::GlobalIterator it = 
        transitions.GetIterator ();

      while (it.HasNext ())
      {
        StateTransitionKey key;
        StateTransitionInfo& info = it.Next (key);

        FSMNode::StateTransitionInfo& childInfo = newn->transitions.GetOrCreate (key);
        
        childInfo.transitionNode = info.nodeFactory ? 
          info.nodeFactory->CreateInstance (packet, skeleton) : 0;
        childInfo.time1 = info.time1;
        childInfo.time2 = info.time2;
        //childInfo.directSwitch = info.directSwitch;
      }
    }

    // Setup all the automatic transitions
    {
      csHash<CS::Animation::StateID, CS::Animation::StateID>::GlobalIterator it = 
        automaticTransitions.GetIterator ();

      while (it.HasNext ())
      {
	CS::Animation::StateID fromState;
	CS::Animation::StateID toState = it.Next (fromState);

	if (toState != CS::Animation::InvalidStateID)
	  newn->automaticTransitions.Put (fromState, toState);
      }
    }

    return csPtr<CS::Animation::iSkeletonAnimNode> (newn);
  }

  const char* FSMNodeFactory::GetNodeName () const
  {
    return name;
  }

  CS::Animation::iSkeletonAnimNodeFactory* FSMNodeFactory::FindNode (const char* name)
  {
    if (this->name == name)
      return this;

    for (size_t i = 0; i < stateList.GetSize (); ++i)
    {
      CS::Animation::iSkeletonAnimNodeFactory* r = stateList[i].nodeFactory;
      if (r)
      {
        r = r->FindNode (name);

        if (r)
          return r;
      }      
    }

    return 0;
  }

  void FSMNodeFactory::SetStateTransition (CS::Animation::StateID fromState, 
    CS::Animation::StateID toState, CS::Animation::iSkeletonAnimNodeFactory* fact)
  {
    StateTransitionKey key = {fromState, toState};
    StateTransitionInfo& info = transitions.GetOrCreate (key);
    
    info.nodeFactory = fact;
  }

  void FSMNodeFactory::SetTransitionCrossfade (CS::Animation::StateID fromState, 
    CS::Animation::StateID toState, float time1, float time2)
  {
    StateTransitionKey key = {fromState, toState};
    StateTransitionInfo& info = transitions.GetOrCreate (key);

    info.time1 = time1;
    info.time2 = time2;
  }

  void FSMNodeFactory::SetAutomaticTransition (CS::Animation::StateID fromState, 
					       CS::Animation::StateID toState,
					       bool automatic)
  {
    if (automaticTransitions.Contains (fromState))
    {
      if (*automaticTransitions[fromState] == toState)
      {
	if (!automatic)
	  automaticTransitions.DeleteAll (fromState);
      }

      else
	*automaticTransitions[fromState] = CS::Animation::InvalidStateID;
    }
    else if (automatic)
      automaticTransitions.Put (fromState, toState);
  }

  //----------------------------------------

  CS_LEAKGUARD_IMPLEMENT(FSMNode);

  FSMNode::FSMNode (FSMNodeFactory* factory)
    : scfImplementationType (this), BaseNodeSingle (this), factory (factory),
    currentState (factory->startState), automaticState (CS::Animation::InvalidStateID),
    playbackSpeed (1.0f), isActive (false), blendFifo (this)
  {}

  void FSMNode::SwitchToState (CS::Animation::StateID newState)
  {
    if (automaticState != CS::Animation::InvalidStateID)
    {
      blendFifo.RemoveAnimations (automaticState);
      automaticState = CS::Animation::InvalidStateID;
    }
    SwitchToState (newState, true);
  }

  void FSMNode::SwitchToState (CS::Animation::StateID newState, bool directSwitch)
  {
    CS_ASSERT(newState < stateList.GetSize ());

    if (currentState == newState)
      return;

    if (isActive)
    {
      FSMNodeFactory::StateTransitionKey key = {currentState, newState};
      StateTransitionInfo* info = transitions.GetElementPointer (key);

      if (info)
      {
        // Not a direct transition
        if (info->transitionNode)
        {
          // Use transition animation
          blendFifo.PushAnimation (info->transitionNode, directSwitch, info->time1, newState);
          blendFifo.PushAnimation (stateList[newState].stateNode, false, info->time2, newState);
        }
        else
        {          
          blendFifo.PushAnimation (stateList[newState].stateNode, directSwitch, info->time1, newState);
        }        
      }
      else
      {
         blendFifo.PushAnimation (stateList[newState].stateNode, directSwitch, 0, newState);
      }      
    }
    else
    {
      currentState = newState;
    }
  }

  CS::Animation::StateID FSMNode::GetCurrentState () const
  {
    return currentState;
  }

  CS::Animation::iSkeletonAnimNode* FSMNode::GetStateNode (CS::Animation::StateID state) const
  {
    CS_ASSERT(state < stateList.GetSize ());
    return stateList[state].stateNode;
  }

  void FSMNode::Play ()
  {
    if (isActive)
      return;    

    if (currentState != CS::Animation::InvalidStateID)
    {
      blendFifo.PushAnimation (stateList[currentState].stateNode, true, 0, currentState);
    }

    isActive = true;
  }

  void FSMNode::Stop ()
  {
    if (!isActive)
      return;

    isActive = false;
    blendFifo.Stop ();
  }

  void FSMNode::SetPlaybackPosition (float time)
  {
    stateList[currentState].stateNode->SetPlaybackPosition (time);
  }

  float FSMNode::GetPlaybackPosition () const
  {
    return stateList[currentState].stateNode->GetPlaybackPosition ();
  }

  float FSMNode::GetDuration () const
  {
    return stateList[currentState].stateNode->GetDuration ();
  }

  void FSMNode::SetPlaybackSpeed (float speed)
  {
    playbackSpeed = speed;
  }

  float FSMNode::GetPlaybackSpeed () const
  {
    return playbackSpeed;
  }

  void FSMNode::BlendState (CS::Animation::AnimatedMeshState* state, float baseWeight)
  {
    if (!isActive)
      return;

    blendFifo.BlendState (state, baseWeight);
  }

  void FSMNode::TickAnimation (float dt)
  {
    if (!isActive)
      return;

    blendFifo.TickAnimation (dt * playbackSpeed);
  }

  bool FSMNode::IsActive () const
  {
    return isActive;
  }

  CS::Animation::iSkeletonAnimNodeFactory* FSMNode::GetFactory () const
  {
    return factory;
  }

  CS::Animation::iSkeletonAnimNode* FSMNode::FindNode (const char* name)
  {
    if (factory->name == name)
      return this;

    for (size_t i = 0; i < stateList.GetSize (); ++i)
    {
      CS::Animation::iSkeletonAnimNode* r = stateList[i].stateNode;
      if (r)
      {
        r = r->FindNode (name);

        if (r)
          return r;
      }      
    }

    return 0;
  }

  void FSMNode::AddAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback)
  {
    BaseNodeSingle::AddAnimationCallback (callback);
  }

  void FSMNode::RemoveAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback)
  {
    BaseNodeSingle::AddAnimationCallback (callback);
  }

  void FSMNode::NewAnimation (size_t data)
  {
    currentState = (CS::Animation::StateID) data;
  }

  void FSMNode::InstructionQueueEmpty (size_t data)
  {
    // Check if we can find an automatic transition to add to the queue
    if (automaticTransitions.Contains ((CS::Animation::StateID) data))
    {
      automaticState = (CS::Animation::StateID) data;
      SwitchToState (*automaticTransitions[(CS::Animation::StateID) data], false);
    }
  }
}
CS_PLUGIN_NAMESPACE_END(Skeleton2)
