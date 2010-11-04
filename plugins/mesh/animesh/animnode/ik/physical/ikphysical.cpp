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
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"

#include "csutil/scf.h"

#include "ikphysical.h"
#include "ivaria/reporter.h"
#include "iengine/camera.h"
#include "iengine/scenenode.h"
#include "iengine/movable.h"
#include "imesh/bodymesh.h"

CS_PLUGIN_NAMESPACE_BEGIN(IKPhysical)
{

  // --------------------------  IKPhysicalManager  --------------------------

  SCF_IMPLEMENT_FACTORY(IKPhysicalManager);

  CS_LEAKGUARD_IMPLEMENT(IKPhysicalManager);

  IKPhysicalManager::IKPhysicalManager (iBase* parent)
    : scfImplementationType (this, parent)
  {
  }

  CS::Animation::iSkeletonIKNodeFactory* IKPhysicalManager::CreateAnimNodeFactory
    (const char *name, CS::Animation::iBodySkeleton* skeleton)
  {
    CS_ASSERT(skeleton);

    csRef<CS::Animation::iSkeletonIKPhysicalNodeFactory> newFact;
    newFact.AttachNew (new IKPhysicalAnimNodeFactory (this, name, skeleton));

    return factoryHash.PutUnique (name, newFact);
  }

  CS::Animation::iSkeletonIKNodeFactory* IKPhysicalManager::FindAnimNodeFactory
    (const char* name) const
  {
    return factoryHash.Get (name, 0);
  }

  void IKPhysicalManager::ClearAnimNodeFactories ()
  {
    factoryHash.DeleteAll ();
  }

  bool IKPhysicalManager::Initialize (iObjectRegistry* object_reg)
  {
    this->object_reg = object_reg;
    return true;
  }

  void IKPhysicalManager::Report (int severity, const char* msg, ...) const
  {
    va_list arg;
    va_start (arg, msg);
    csRef<iReporter> rep (csQueryRegistry<iReporter> (object_reg));
    if (rep)
      rep->ReportV (severity,
		    "crystalspace.mesh.animesh.animnode.IKPhysical",
		    msg, arg);
    else
      {
	csPrintfV (msg, arg);
	csPrintf ("\n");
      }
    va_end (arg);
  }

  // --------------------------  IKPhysicalAnimNodeFactory  --------------------------

  CS_LEAKGUARD_IMPLEMENT(IKPhysicalAnimNodeFactory);

  IKPhysicalAnimNodeFactory::IKPhysicalAnimNodeFactory
    (IKPhysicalManager* manager, const char *name, CS::Animation::iBodySkeleton* skeleton)
    : scfImplementationType (this), manager (manager), name (name),
    bodySkeleton (skeleton), maxEffectorID (0), resetChain (true)
  {
  }

  csPtr<CS::Animation::iSkeletonAnimNode> IKPhysicalAnimNodeFactory::CreateInstance
    (CS::Animation::iSkeletonAnimPacket* packet, CS::Animation::iSkeleton* skeleton)
  {
    csRef<CS::Animation::iSkeletonAnimNode> child;
    if (childNode)
      child = childNode->CreateInstance (packet, skeleton);

    csRef<CS::Animation::iSkeletonAnimNode> newP;
    newP.AttachNew (new IKPhysicalAnimNode (this, skeleton, child));
    return csPtr<CS::Animation::iSkeletonAnimNode> (newP);
  }

  const char* IKPhysicalAnimNodeFactory::GetNodeName () const
  {
    return name;
  }

  CS::Animation::iSkeletonAnimNodeFactory* IKPhysicalAnimNodeFactory::FindNode
    (const char* name)
  {
    if (this->name == name)
      return this;

    if (childNode)
      return childNode->FindNode (name);

      return 0;
  }

  CS::Animation::EffectorID IKPhysicalAnimNodeFactory::AddEffector
    (CS::Animation::iBodyChain* chain, CS::Animation::BoneID bone,
     csOrthoTransform& transform)
  {
    // TODO: check chain in ragdoll & bone in chain

    EffectorData effector;
    effector.chain = chain;
    effector.bone = bone;
    effector.transform = transform;
    effectors.Put (maxEffectorID, effector);

    return maxEffectorID++;
  }

  void IKPhysicalAnimNodeFactory::RemoveEffector (CS::Animation::EffectorID effector)
  {
    effectors.DeleteAll (effector);
  }

  void IKPhysicalAnimNodeFactory::SetChildNode (CS::Animation::iSkeletonAnimNodeFactory* node)
  {
    childNode = node;
  }

  CS::Animation::iSkeletonAnimNodeFactory* IKPhysicalAnimNodeFactory::GetChildNode () const
  {
    return childNode;
  }

  void IKPhysicalAnimNodeFactory::ClearChildNode ()
  {
    childNode = 0;
  }

  void IKPhysicalAnimNodeFactory::SetChainAutoReset (bool reset)
  {
    resetChain = reset;
  }

  bool IKPhysicalAnimNodeFactory::GetChainAutoReset () const
  {
    return resetChain;
  }

  // --------------------------  IKPhysicalAnimNode  --------------------------

  CS_LEAKGUARD_IMPLEMENT(IKPhysicalAnimNode);

  IKPhysicalAnimNode::IKPhysicalAnimNode (IKPhysicalAnimNodeFactory* factory, 
					  CS::Animation::iSkeleton* skeleton,
					  CS::Animation::iSkeletonAnimNode* childNode)
    : scfImplementationType (this), factory (factory), sceneNode (nullptr),
    skeleton (skeleton), childNode (childNode), isActive (false)
  {
  }

  IKPhysicalAnimNode::~IKPhysicalAnimNode ()
  {
    Stop ();
  }

  void IKPhysicalAnimNode::SetRagdollNode (CS::Animation::iSkeletonRagdollNode* ragdollNode)
  {
    this->ragdollNode = ragdollNode;
  }

  CS::Animation::iSkeletonRagdollNode* IKPhysicalAnimNode::GetRagdollNode () const
  {
    return ragdollNode;
  }

  void IKPhysicalAnimNode::AddConstraint (CS::Animation::EffectorID effectorID,
					  csOrthoTransform& transform)
  {
    CS_ASSERT(factory->effectors.Contains (effectorID));

    // Record the constraint
    ConstraintData constraint;
    constraint.type = CONSTRAINT_FIXED;
    constraint.offset = transform;
    AddConstraint (effectorID, constraint);
  }

  void IKPhysicalAnimNode::AddConstraint (CS::Animation::EffectorID effectorID,
					  iMovable* target,
					  const csOrthoTransform& offset)
  {
    CS_ASSERT(factory->effectors.Contains (effectorID));

    ConstraintData constraint;
    constraint.type = CONSTRAINT_MOVABLE;
    constraint.movable = target;
    constraint.offset = offset;
    AddConstraint (effectorID, constraint);
  }

  void IKPhysicalAnimNode::AddConstraint (CS::Animation::EffectorID effectorID,
					  iCamera* target,
					  const csOrthoTransform& offset)
  {
    CS_ASSERT(factory->effectors.Contains (effectorID));

    ConstraintData constraint;
    constraint.type = CONSTRAINT_CAMERA;
    constraint.camera = target;
    constraint.offset = offset;
    AddConstraint (effectorID, constraint);
  }

  void IKPhysicalAnimNode::AddConstraint (CS::Animation::EffectorID effectorID,
					  ConstraintData& constraint)
  {
    // Find the effector data
    EffectorData* effector = factory->effectors[effectorID];

    // Check if this chain has already some constraints
    ChainData chainData;
    if (!chains.Contains (effector->chain))
    {
      // Create a new entry for the chain
      chainData.previousState = ragdollNode->GetBodyChainState (effector->chain);
      chainData.constraintCount = 1;
      chains.Put (effector->chain, chainData);

      // Set the chain as dynamic
      ragdollNode->SetBodyChainState (effector->chain, CS::Animation::STATE_DYNAMIC);
    }

    else
    {
      chainData = chains.Get (effector->chain, chainData);
      chainData.constraintCount++;
    }

    // Compute the world transform of the effector
    csQuaternion boneRotation;
    csVector3 boneOffset;
    skeleton->GetTransformAbsSpace (effector->bone, boneRotation, boneOffset);
    csOrthoTransform boneTransform (csMatrix3 (boneRotation.GetConjugate ()), boneOffset);
    csOrthoTransform effectorTransform =
      effector->transform * boneTransform * sceneNode->GetMovable ()->GetFullTransform ();

    // Compute the world transform of the constraint target
    csOrthoTransform targetTransform;
    switch (constraint.type)
    {
    case CONSTRAINT_FIXED:
      targetTransform = constraint.offset;
      break;

    case CONSTRAINT_MOVABLE:
      targetTransform = constraint.offset * constraint.movable->GetFullTransform ();
      break;

    case CONSTRAINT_CAMERA:
      targetTransform = constraint.offset * constraint.camera->GetTransform ();
      break;

    default:
      break;
    }

    // Create a pivot joint
    constraint.dragJoint = bulletDynamicSystem->CreatePivotJoint ();
    constraint.dragJoint->Attach (ragdollNode->GetBoneRigidBody (effector->bone),
				  effectorTransform.GetOrigin ());
    constraint.dragJoint->SetPosition (targetTransform.GetOrigin ());

    constraints.PutUnique (effectorID, constraint);
  }

  void IKPhysicalAnimNode::RemoveConstraint (CS::Animation::EffectorID effectorID)
  {
    CS_ASSERT(factory->effectors.Contains (effectorID));

    // Check if there is really a constraint on this effector
    if (!constraints.Contains (effectorID))
      return;

    // Remove and delete the pivot joint
    ConstraintData* constraint = constraints[effectorID];
    bulletDynamicSystem->RemovePivotJoint (constraint->dragJoint);
    constraints.DeleteAll (effectorID);

    // Find the effector & chain data
    EffectorData* effector = factory->effectors[effectorID];
    ChainData* chain = chains[effector->chain];

    // Check if there are no more constraints on this chain
    if (--chain->constraintCount == 0)
    {
      // Set back the chain in its previous state
      if (factory->resetChain)
	ragdollNode->SetBodyChainState (effector->chain, chain->previousState);

      // Remove the entry for the chain
      chains.DeleteAll (effector->chain);
    }
  }

  void IKPhysicalAnimNode::Play ()
  {
    CS_ASSERT (skeleton->GetSceneNode () && ragdollNode);

    if (isActive)
      return;

    // Find the scene node
    if (!sceneNode)
      sceneNode = skeleton->GetSceneNode ();

    // Find the Bullet dynamic system
    csRef<CS::Animation::iSkeletonRagdollNodeFactory> ragdollNodeFactory =
      scfQueryInterface<CS::Animation::iSkeletonRagdollNodeFactory>
      (ragdollNode->GetFactory ());

    bulletDynamicSystem = scfQueryInterface<CS::Physics::Bullet::iDynamicSystem>
      (ragdollNodeFactory->GetDynamicSystem ());
    if (!bulletDynamicSystem)
    {
      factory->manager->Report (CS_REPORTER_SEVERITY_ERROR,
				"No Bullet dynamic system while starting IK animation node");
      return;
    }

    isActive = true;

    // Start the child node
    if (childNode)
      childNode->Play ();
  }

  void IKPhysicalAnimNode::Stop ()
  {
    if (!isActive)
      return;

    isActive = false;

    // Stop the child node
    if (childNode)
      childNode->Stop ();

    // Remove all constraints
    // TODO: really?
    for (csHash<ConstraintData, CS::Animation::EffectorID>::GlobalIterator it =
	   constraints.GetIterator (); it.HasNext (); )
    {
      csTuple2<ConstraintData, CS::Animation::EffectorID> tuple = it.NextTuple ();
      RemoveConstraint (tuple.second);
    }
  }

  void IKPhysicalAnimNode::SetPlaybackPosition (float time)
  {
    if (childNode)
      childNode->SetPlaybackPosition (time);
  }

  float IKPhysicalAnimNode::GetPlaybackPosition () const
  {
    if (childNode)
      return childNode->GetPlaybackPosition ();

    return 0.0;
  }

  float IKPhysicalAnimNode::GetDuration () const
  {
    if (childNode)
      return childNode->GetDuration ();

    return 0.0;
  }

  void IKPhysicalAnimNode::SetPlaybackSpeed (float speed)
  {
    if (childNode)
      childNode->SetPlaybackSpeed (speed);
  }

  float IKPhysicalAnimNode::GetPlaybackSpeed () const
  {
    if (childNode)
      return childNode->GetPlaybackSpeed ();

    return 1.0;
  }

  void IKPhysicalAnimNode::BlendState (CS::Animation::csSkeletalState* state, float baseWeight)
  {
    // Check that this node is active
    if (!isActive)
      return;

    // Make the child node blend the state
    if (childNode)
      childNode->BlendState (state, baseWeight);
  }

  void IKPhysicalAnimNode::TickAnimation (float dt)
  {
    // Update the child node
    if (childNode)
      childNode->TickAnimation (dt);

    // Update the position of the active constraints
    for (csHash<ConstraintData, CS::Animation::EffectorID>::GlobalIterator it =
	   constraints.GetIterator (); it.HasNext (); )
    {
      csTuple2<ConstraintData, CS::Animation::EffectorID> tuple = it.NextTuple ();
      ConstraintData constraint = tuple.first;

      switch (constraint.type)
      {
      case CONSTRAINT_MOVABLE:
	{
	  csOrthoTransform transform = constraint.offset * constraint.movable->GetFullTransform ();
	  constraint.dragJoint->SetPosition (transform.GetOrigin ());
	}
	break;

      case CONSTRAINT_CAMERA:
	{
	  csOrthoTransform transform = constraint.offset * constraint.camera->GetTransform ();
	  constraint.dragJoint->SetPosition (transform.GetOrigin ());
	}
	break;

      default:
	break;
      }
    }
  }

  bool IKPhysicalAnimNode::IsActive () const
  {
    return isActive;
  }

  CS::Animation::iSkeletonAnimNodeFactory* IKPhysicalAnimNode::GetFactory () const
  {
    return factory;
  }

  CS::Animation::iSkeletonAnimNode* IKPhysicalAnimNode::FindNode (const char* name)
  {
    if (factory->name == name)
      return this;

    if (childNode)
      return childNode->FindNode (name);

    return 0;
  }

  void IKPhysicalAnimNode::AddAnimationCallback
    (CS::Animation::iSkeletonAnimCallback* callback)
  {
    // TODO
  }

  void IKPhysicalAnimNode::RemoveAnimationCallback
    (CS::Animation::iSkeletonAnimCallback* callback)
  {
    // TODO
  }

}
CS_PLUGIN_NAMESPACE_END(IKPhysical)
