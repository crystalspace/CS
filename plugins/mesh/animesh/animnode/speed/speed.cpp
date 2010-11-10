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

#include "speed.h"
#include "ivaria/reporter.h"
#include "csgeom/math.h"

CS_PLUGIN_NAMESPACE_BEGIN(SpeedNode)
{

  /********************
   *  SpeedNodeManager
   ********************/

  SCF_IMPLEMENT_FACTORY(SpeedNodeManager);

  CS_LEAKGUARD_IMPLEMENT(SpeedNodeManager);

  SpeedNodeManager::SpeedNodeManager (iBase* parent)
    : scfImplementationType (this, parent)
  {
  }

  CS::Animation::iSkeletonSpeedNodeFactory* SpeedNodeManager::CreateAnimNodeFactory (const char* name)
  {
    csRef<CS::Animation::iSkeletonSpeedNodeFactory> newFact;
    newFact.AttachNew (new SpeedNodeFactory (this, name));

    return speedFactories.PutUnique (name, newFact);
  }

  CS::Animation::iSkeletonSpeedNodeFactory* SpeedNodeManager::FindAnimNodeFactory (const char* name)
  {
   return speedFactories.Get (name, 0);
  }

  void SpeedNodeManager::ClearAnimNodeFactories ()
  {
    speedFactories.DeleteAll ();
  }

  bool SpeedNodeManager::Initialize (iObjectRegistry*)
  {
    this->object_reg = object_reg;
    return true;
  }

  void SpeedNodeManager::Report (int severity, const char* msg, ...) const
  {
    va_list arg;
    va_start (arg, msg);
    csRef<iReporter> rep (csQueryRegistry<iReporter> (object_reg));
    if (rep)
      rep->ReportV (severity,
		    "crystalspace.mesh.animesh.animnode.speed",
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

  SpeedNodeFactory::SpeedNodeFactory (SpeedNodeManager* manager, const char *name)
    : scfImplementationType (this), manager (manager), name (name)
  {
  }

  void SpeedNodeFactory::AddNode (CS::Animation::iSkeletonAnimNodeFactory* factory, float speed)
  {
    // user help: set it cyclic if it is an animnode, otherwise it has to be made by the user
    csRef<CS::Animation::iSkeletonAnimationNodeFactory> fact =
      scfQueryInterface<CS::Animation::iSkeletonAnimationNodeFactory> (factory);
    if (fact)
      fact->SetCyclic (true);

    // insert node in sorted position
    size_t index = speedList.InsertSorted (speed);
    subFactories.Insert (index, factory);
  }

  csPtr<CS::Animation::iSkeletonAnimNode> SpeedNodeFactory::CreateInstance (CS::Animation::iSkeletonAnimPacket* packet,
							      CS::Animation::iSkeleton* skeleton)
  {
    csRef<SpeedNode> newP;
    newP.AttachNew (new SpeedNode (this, skeleton));

    for (size_t i = 0; i < subFactories.GetSize (); i++)
    {
      csRef<CS::Animation::iSkeletonAnimNode> node = 
	subFactories[i]->CreateInstance (packet, skeleton);
      newP->subNodes.Push (node);
    }

    return csPtr<CS::Animation::iSkeletonAnimNode> (newP);
  }

  const char* SpeedNodeFactory::GetNodeName () const
  {
    return name;
  }

  CS::Animation::iSkeletonAnimNodeFactory* SpeedNodeFactory::FindNode (const char* name)
  {
    if (this->name == name)
      return this;

    for (size_t i = 0; i < subFactories.GetSize (); i++)
    {
      CS::Animation::iSkeletonAnimNodeFactory* r = subFactories[i]->FindNode (name);
      if (r)
        return r;
    }

    return 0;
  }

  /********************
   *  SpeedNode
   ********************/

  CS_LEAKGUARD_IMPLEMENT(SpeedNode);

  SpeedNode::SpeedNode (SpeedNodeFactory* factory, CS::Animation::iSkeleton* skeleton)
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

  void SpeedNode::BlendState (CS::Animation::csSkeletalState* state, float baseWeight)
  {
    if (!isPlaying)
      return;

    if (slowNode == fastNode)
      subNodes[slowNode]->BlendState (state, baseWeight);

    else
    {
      subNodes[slowNode]->BlendState (state, baseWeight);
      subNodes[fastNode]->BlendState (state, baseWeight * speedRatio);
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

  CS::Animation::iSkeletonAnimNodeFactory* SpeedNode::GetFactory () const
  {
    return factory;
  }

  CS::Animation::iSkeletonAnimNode* SpeedNode::FindNode (const char* name)
  {
    if (factory->name == name)
      return this;

    for (size_t i = 0; i < subNodes.GetSize (); i++)
    {
      CS::Animation::iSkeletonAnimNode* r = subNodes[i]->FindNode (name);
      if (r)
        return r;
    }

    return 0;
  }

  void SpeedNode::AddAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback)
  {
    // TODO?
  }

  void SpeedNode::RemoveAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback)
  {
    // TODO?
  }

}
CS_PLUGIN_NAMESPACE_END(SpeedNode)
