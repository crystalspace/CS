/*
  Copyright (C) 2010 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "csutil/scf.h"

#include "basic.h"
#include "ivaria/reporter.h"
#include "csgeom/math.h"

CS_PLUGIN_NAMESPACE_BEGIN(BasicNodes)
{

  /********************
   *  BasicNodesManager
   ********************/

  SCF_IMPLEMENT_FACTORY(BasicNodesManager);

  CS_LEAKGUARD_IMPLEMENT(BasicNodesManager);

  BasicNodesManager::BasicNodesManager (iBase* parent)
    : scfImplementationType (this, parent)
  {
  }

  iSkeletonSpeedNodeFactory2* BasicNodesManager::CreateSpeedNodeFactory (const char* name)
  {
    csRef<iSkeletonSpeedNodeFactory2> newFact;
    newFact.AttachNew (new SpeedNodeFactory (this, name));

    return speedFactories.PutUnique (name, newFact);
  }

  iSkeletonSpeedNodeFactory2* BasicNodesManager::FindSpeedNodeFactory (const char* name)
  {
   return speedFactories.Get (name, 0);
  }

  void BasicNodesManager::ClearSpeedNodeFactories ()
  {
    speedFactories.DeleteAll ();
  }

  bool BasicNodesManager::Initialize (iObjectRegistry*)
  {
    this->object_reg = object_reg;
    return true;
  }

  void BasicNodesManager::Report (int severity, const char* msg, ...) const
  {
    va_list arg;
    va_start (arg, msg);
    csRef<iReporter> rep (csQueryRegistry<iReporter> (object_reg));
    if (rep)
      rep->ReportV (severity,
		    "crystalspace.mesh.animesh.controllers.basic",
		    msg, arg);
    else
      {
	csPrintfV (msg, arg);
	csPrintf ("\n");
      }
    va_end (arg);
  }

  /********************
   *  SpeedNodeFactory
   ********************/

  CS_LEAKGUARD_IMPLEMENT(SpeedNodeFactory);

  SpeedNodeFactory::SpeedNodeFactory (BasicNodesManager* manager, const char *name)
    : scfImplementationType (this), manager (manager), name (name)
  {
  }

  void SpeedNodeFactory::AddNode (iSkeletonAnimNodeFactory2* factory, float speed)
  {
    // user help: set it cyclic if it is an animnode, otherwise it has to be made by the user
    csRef<iSkeletonAnimationNodeFactory2> fact =
      scfQueryInterface<iSkeletonAnimationNodeFactory2> (factory);
    if (fact)
      fact->SetCyclic (true);

    // insert node in sorted position
    size_t index = speedList.InsertSorted (speed);
    subFactories.Insert (index, factory);
  }

  csPtr<iSkeletonAnimNode2> SpeedNodeFactory::CreateInstance (iSkeletonAnimPacket2* packet,
							      iSkeleton2* skeleton)
  {
    csRef<SpeedNode> newP;
    newP.AttachNew (new SpeedNode (this, skeleton));

    for (size_t i = 0; i < subFactories.GetSize (); i++)
    {
      csRef<iSkeletonAnimNode2> node = 
	subFactories[i]->CreateInstance (packet, skeleton);
      newP->subNodes.Push (node);
    }

    return csPtr<iSkeletonAnimNode2> (newP);
  }

  const char* SpeedNodeFactory::GetNodeName () const
  {
    return name;
  }

  iSkeletonAnimNodeFactory2* SpeedNodeFactory::FindNode (const char* name)
  {
    if (this->name == name)
      return this;

    for (size_t i = 0; i < subFactories.GetSize (); i++)
    {
      iSkeletonAnimNodeFactory2* r = subFactories[i]->FindNode (name);
      if (r)
        return r;
    }

    return 0;
  }

  /********************
   *  SpeedNode
   ********************/

  CS_LEAKGUARD_IMPLEMENT(SpeedNode);

  SpeedNode::SpeedNode (SpeedNodeFactory* factory, iSkeleton2* skeleton)
    : scfImplementationType (this), factory (factory), skeleton (skeleton),
    speed (0.0f), slowNode (0), fastNode (0), currentPosition (0.0f), speedRatio (1.0f),
    isPlaying (false)
  {
  }

  void SpeedNode::SetSpeed (float speed)
  {
    if (isPlaying)
      UpdateNewSpeed (speed);

    else
    {
      if (subNodes.GetSize () != 0)
      {
	// bound value
	if (speed < factory->speedList[0])
	  speed = factory->speedList[0];
	else if (speed > factory->speedList[factory->speedList.GetSize () - 1])
	  speed = factory->speedList[factory->speedList.GetSize () - 1];
      }

      this->speed = speed;
    }
  }

  void SpeedNode::UpdateNewSpeed (float speed)
  {
    // bound value
    if (speed < factory->speedList[0])
      speed = factory->speedList[0];
    else if (speed > factory->speedList[factory->speedList.GetSize () - 1])
      speed = factory->speedList[factory->speedList.GetSize () - 1];

    // check if speed has changed
    if (this->speed == speed)
      return;

    // check if the speed was 0
    if (fabs (this->speed) < EPSILON)
      currentPosition = 0.0f;

    this->speed = speed;

    // check if there is only one node
    if (factory->speedList.GetSize () == 1)
    {
      slowNode = fastNode = 0;
      if (!subNodes[0]->IsActive ())
	subNodes[0]->Play ();

      return;
    }

    // find the anims bounding that speed value
    size_t newSlowNode, newFastNode;
    for (size_t i = 0; i < factory->speedList.GetSize () - 1; i++)
    {
      if (speed == factory->speedList[i])
      {
	newSlowNode = newFastNode = i;
	break;
      }

      if (speed > factory->speedList[i]
	  && speed < factory->speedList[i + 1])
      {
	newSlowNode = i;
	newFastNode = i + 1;
	break;
      }

      if (i == factory->speedList.GetSize () - 2)
      {
	newSlowNode = newFastNode = i + 1;
	break;
      }
    }

    // stop previous anims and play the new ones
    if (slowNode != newSlowNode && slowNode != newFastNode)
      subNodes[slowNode]->Stop ();
    if (fastNode != slowNode && fastNode != newSlowNode && fastNode != newFastNode)
      subNodes[fastNode]->Stop ();
    if (!subNodes[newSlowNode]->IsActive ())
      subNodes[newSlowNode]->Play ();
    if (!subNodes[newFastNode]->IsActive ())
      subNodes[newFastNode]->Play ();

    // compute playback speed & position of anims
    if (newSlowNode == newFastNode)
    {
      subNodes[newSlowNode]->SetPlaybackSpeed (1.0f);

      if (fabs (factory->speedList[newSlowNode]) < EPSILON)
	subNodes[newSlowNode]->SetPlaybackPosition (0.0f);
      else
	subNodes[newSlowNode]->SetPlaybackPosition (currentPosition *
						    subNodes[newSlowNode]->GetDuration ());
    }

    else
    {
      float speedDifference = factory->speedList[newFastNode] - factory->speedList[newSlowNode];
      speedRatio = (speed - factory->speedList[newSlowNode]) / speedDifference;
      cycleDuration = (1.0f - speedRatio) * subNodes[newSlowNode]->GetDuration ()
	+ speedRatio * subNodes[newFastNode]->GetDuration ();

      // apply new playback speeds
      subNodes[newSlowNode]->SetPlaybackSpeed
	(subNodes[newSlowNode]->GetDuration () / cycleDuration);
      subNodes[newFastNode]->SetPlaybackSpeed
	(subNodes[newFastNode]->GetDuration () / cycleDuration);

      // apply new playback positions
      subNodes[newSlowNode]->SetPlaybackPosition
	(currentPosition * subNodes[newSlowNode]->GetDuration ());
      subNodes[newFastNode]->SetPlaybackPosition
	(currentPosition * subNodes[newFastNode]->GetDuration ());
    }

    // save new anim indexes
    slowNode = newSlowNode;
    fastNode = newFastNode;    
  }

  void SpeedNode::Play ()
  {
    if (isPlaying || subNodes.GetSize () == 0)
      return;

    isPlaying = true;
    currentPosition = 0.0f;
    speedRatio = 1.0f;

    // corrupt current speed to force the start
    float tempSpeed = speed;
    speed = factory->speedList[0] - 1.0f;

    // update to new speed
    UpdateNewSpeed (tempSpeed);
  }

  void SpeedNode::Stop ()
  {
    if (!isPlaying)
      return;

    isPlaying = false;

    // stop playing nodes
    subNodes[slowNode]->Stop ();
    if (slowNode != fastNode)
      subNodes[fastNode]->Stop ();
  }

  void SpeedNode::SetPlaybackPosition (float time)
  {
    // TODO?
  }

  float SpeedNode::GetPlaybackPosition () const
  {
    // TODO?
    return 0.0f;
  }

  float SpeedNode::GetDuration () const
  {
    // TODO?
    return 0.0f;
  }

  void SpeedNode::SetPlaybackSpeed (float speed)
  {
    // TODO!
  }

  float SpeedNode::GetPlaybackSpeed () const
  {
    // TODO!
    return 0.0f;
  }

  void SpeedNode::BlendState (csSkeletalState2* state, float baseWeight)
  {
    if (!isPlaying)
      return;

    if (slowNode == fastNode)
      subNodes[slowNode]->BlendState (state, baseWeight);

    else
    {
      // the blending should only be made by these two lines
      //subNodes[slowNode]->BlendState (state, baseWeight * (1.0f - speedRatio));
      //subNodes[fastNode]->BlendState (state, baseWeight * speedRatio);

      uint boneCount = skeleton->GetFactory ()->GetTopBoneID ();

      csRef<csSkeletalState2> slowState;
      slowState.AttachNew (new csSkeletalState2 ());
      slowState->Setup (boneCount + 1);

      csRef<csSkeletalState2> fastState;
      fastState.AttachNew (new csSkeletalState2 ());
      fastState->Setup (boneCount + 1);

      subNodes[slowNode]->BlendState (slowState, 1.0f);
      subNodes[fastNode]->BlendState (fastState, 1.0f);

      // TODO: this doesn't use the baseweight nor the previous state
      for (uint i = 0; i < boneCount; i++)
      {
	// if not both nodes use this bone then the pose can simply be copied
	if (!slowState->IsBoneUsed (i) && !fastState->IsBoneUsed (i))
	  continue;

	if (slowState->IsBoneUsed (i) && !fastState->IsBoneUsed (i))
	{
	  state->SetBoneUsed (i);
	  state->GetVector (i) = slowState->GetVector (i);
	  state->GetQuaternion (i) = slowState->GetQuaternion (i);
	  continue;
	}

	if (!slowState->IsBoneUsed (i) && fastState->IsBoneUsed (i))
	{
	  state->SetBoneUsed (i);
	  state->GetVector (i) = fastState->GetVector (i);
	  state->GetQuaternion (i) = fastState->GetQuaternion (i);
	  continue;
	}

	// put the transforms in bone space before blending them
	csQuaternion rotation; 
	csVector3 offset;
	skeleton->GetFactory ()->GetTransformAbsSpace (i, rotation, offset);

	csQuaternion slowBoneQuaternion = slowState->GetQuaternion (i) * rotation.GetConjugate ();
	csQuaternion fastBoneQuaternion = fastState->GetQuaternion (i) * rotation.GetConjugate ();

	// blend the transforms then put them back in absolute space
	state->SetBoneUsed (i);
	state->GetVector (i) = csLerp (slowState->GetVector (i) - offset,
				       fastState->GetVector (i) - offset, speedRatio) + offset;
	state->GetQuaternion (i) =
	  slowBoneQuaternion.SLerp (fastBoneQuaternion, speedRatio) * rotation;
      }
    }
  }

  void SpeedNode::TickAnimation (float dt)
  {
    if (!isPlaying)
      return;

    // update current position
    currentPosition += dt / cycleDuration;
    if (currentPosition >= 1.0f)
      currentPosition = 0.0f;
    else if (currentPosition <= 0.0f)
      currentPosition = 1.0f;

    // tick the child nodes
    if (slowNode == fastNode)
      subNodes[slowNode]->TickAnimation (dt);

    else
    {
      subNodes[slowNode]->TickAnimation (dt);
      subNodes[fastNode]->TickAnimation (dt);
    }
  }

  bool SpeedNode::IsActive () const
  {
    return isPlaying;
  }

  iSkeletonAnimNodeFactory2* SpeedNode::GetFactory () const
  {
    return factory;
  }

  iSkeletonAnimNode2* SpeedNode::FindNode (const char* name)
  {
    if (factory->name == name)
      return this;

    for (size_t i = 0; i < subNodes.GetSize (); i++)
    {
      iSkeletonAnimNode2* r = subNodes[i]->FindNode (name);
      if (r)
        return r;
    }

    return 0;
  }

  void SpeedNode::AddAnimationCallback (iSkeletonAnimCallback2* callback)
  {
    // TODO?
  }

  void SpeedNode::RemoveAnimationCallback (iSkeletonAnimCallback2* callback)
  {
    // TODO?
  }

}
CS_PLUGIN_NAMESPACE_END(BasicNodes)
