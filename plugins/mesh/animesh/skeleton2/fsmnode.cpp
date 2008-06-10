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

#include "fsmnode.h"


CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{

  FSMNodeFactory::FSMNodeFactory (const char* name)
    : scfImplementationType (this), name (name), 
    startState (CS::Animation::InvalidStateID)
  {}

  CS::Animation::StateID FSMNodeFactory::AddState ()
  {
    State newState;

    return stateList.Push (newState);    
  }

  void FSMNodeFactory::SetStateNode (CS::Animation::StateID id, 
    iSkeletonAnimNodeFactory2* nodeFact)
  {
    CS_ASSERT(id < stateList.GetSize ());

    stateList[id].nodeFactory = nodeFact;
  }

  iSkeletonAnimNodeFactory2* FSMNodeFactory::GetStateNode (CS::Animation::StateID id) const
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
        return i;
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

  csPtr<iSkeletonAnimNode2> FSMNodeFactory::CreateInstance (
    iSkeletonAnimPacket2* packet, iSkeleton2* skeleton)
  {
    csRef<FSMNode> newn;

    newn.AttachNew (new FSMNode (this));

    // Setup nodes
    for (size_t i = 0; i < stateList.GetSize (); ++i)
    {
      FSMNode::State newState;

      newState.stateNode = stateList[i].nodeFactory->CreateInstance (packet, skeleton);

      newn->stateList.Push (newState);
    }

    return csPtr<iSkeletonAnimNode2> (newn);
  }

  const char* FSMNodeFactory::GetNodeName () const
  {
    return name;
  }

  iSkeletonAnimNodeFactory2* FSMNodeFactory::FindNode (const char* name)
  {
    if (this->name == name)
      return this;

    for (size_t i = 0; i < stateList.GetSize (); ++i)
    {
      iSkeletonAnimNodeFactory2* r = stateList[i].nodeFactory;
      if (r)
      {
        r = r->FindNode (name);

        if (r)
          return r;
      }      
    }

    return 0;
  }


  FSMNode::FSMNode (FSMNodeFactory* factory)
    : scfImplementationType (this), BaseNodeSingle (this), factory (factory),
    currentState (factory->startState), isActive (false), playbackSpeed (1.0f)
  {}

  void FSMNode::SwitchToState (CS::Animation::StateID newState)
  {
    if (isActive)
    {
      stateList[currentState].stateNode->Stop ();

      if (newState != CS::Animation::InvalidStateID)
      {
        stateList[newState].stateNode->Play ();
      }
    }

    currentState = newState;
  }

  CS::Animation::StateID FSMNode::GetCurrentState () const
  {
    return currentState;
  }

  void FSMNode::Play ()
  {
    if (isActive)
      return;    

    if (currentState != CS::Animation::InvalidStateID)
    {
      isActive = true;
      stateList[currentState].stateNode->Play ();
    }    
  }

  void FSMNode::Stop ()
  {
    if (!isActive)
      return;

    isActive = false;
    stateList[currentState].stateNode->Stop ();
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

  void FSMNode::BlendState (csSkeletalState2* state, float baseWeight)
  {
    if (!isActive)
      return;

    stateList[currentState].stateNode->BlendState (state, baseWeight);
  }

  void FSMNode::TickAnimation (float dt)
  {
    if (!isActive)
      return;

    stateList[currentState].stateNode->TickAnimation (dt * playbackSpeed);
  }

  bool FSMNode::IsActive () const
  {
    return isActive;
  }

  iSkeletonAnimNodeFactory2* FSMNode::GetFactory () const
  {
    return factory;
  }

  iSkeletonAnimNode2* FSMNode::FindNode (const char* name)
  {
    if (factory->name == name)
      return this;

    for (size_t i = 0; i < stateList.GetSize (); ++i)
    {
      iSkeletonAnimNode2* r = stateList[i].stateNode;
      if (r)
      {
        r = r->FindNode (name);

        if (r)
          return r;
      }      
    }

    return 0;
  }

  void FSMNode::AddAnimationCallback (iSkeletonAnimCallback2* callback)
  {
    BaseNodeSingle::AddAnimationCallback (callback);
  }

  void FSMNode::RemoveAnimationCallback (iSkeletonAnimCallback2* callback)
  {
    BaseNodeSingle::AddAnimationCallback (callback);
  }


}
CS_PLUGIN_NAMESPACE_END(Skeleton2)
