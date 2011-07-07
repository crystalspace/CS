/*
  Copyright (C) 2010 Christian Van Brussel, Institute of Information
      and Communication Technologies, Electronics and Applied Mathematics
      at Universite catholique de Louvain, Belgium
      http://www.uclouvain.be/en-icteam.html

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
  SCF_IMPLEMENT_FACTORY(IKPhysicalNodeManager);

  void IKPhysicalNodeManager::Report (int severity, const char* msg, ...) const
  {
    va_list arg;
    va_start (arg, msg);
    csReportV (object_reg, severity, 
	       "crystalspace.mesh.animesh.animnode.ik.physical",
	       msg, arg);
    va_end (arg);
  }

  // --------------------------  IKPhysicalNodeFactory  --------------------------

  CS_LEAKGUARD_IMPLEMENT(IKPhysicalNodeFactory);

  IKPhysicalNodeFactory::IKPhysicalNodeFactory
    (IKPhysicalNodeManager* manager, const char *name)
    : scfImplementationType (this), CS::Animation::SkeletonAnimNodeFactorySingle (name),
    manager (manager), resetChain (true)
  {
  }

  void IKPhysicalNodeFactory::SetBodySkeleton (CS::Animation::iBodySkeleton* skeleton)
  {
    bodySkeleton = skeleton;
  }

  CS::Animation::iBodySkeleton* IKPhysicalNodeFactory::GetBodySkeleton () const
  {
    return bodySkeleton;
  }

  CS::Animation::EffectorID IKPhysicalNodeFactory::AddEffector
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

  void IKPhysicalNodeFactory::RemoveEffector (CS::Animation::EffectorID effector)
  {
    effectors.DeleteAll (effector);
  }

  void IKPhysicalNodeFactory::SetChainAutoReset (bool reset)
  {
    resetChain = reset;
  }

  bool IKPhysicalNodeFactory::GetChainAutoReset () const
  {
    return resetChain;
  }

  csPtr<CS::Animation::SkeletonAnimNodeSingleBase> IKPhysicalNodeFactory::ActualCreateInstance (
    CS::Animation::iSkeletonAnimPacket* packet,
    CS::Animation::iSkeleton* skeleton)
  {
    return csPtr<CS::Animation::SkeletonAnimNodeSingleBase> (new IKPhysicalNode (this, skeleton));
  }

  // --------------------------  IKPhysicalNode  --------------------------

  CS_LEAKGUARD_IMPLEMENT(IKPhysicalNode);

  IKPhysicalNode::IKPhysicalNode (IKPhysicalNodeFactory* factory, 
				  CS::Animation::iSkeleton* skeleton)
    : scfImplementationType (this),
    CS::Animation::SkeletonAnimNodeSingle<IKPhysicalNodeFactory> (factory, skeleton),
    sceneNode (nullptr)
  {
  }

  IKPhysicalNode::~IKPhysicalNode ()
  {
    Stop ();
  }

  void IKPhysicalNode::SetRagdollNode (CS::Animation::iSkeletonRagdollNode* ragdollNode)
  {
    this->ragdollNode = ragdollNode;
  }

  CS::Animation::iSkeletonRagdollNode* IKPhysicalNode::GetRagdollNode () const
  {
    return ragdollNode;
  }

  void IKPhysicalNode::AddConstraint (CS::Animation::EffectorID effectorID,
					  csOrthoTransform& transform)
  {
    CS_ASSERT(factory->effectors.Contains (effectorID));

    // Record the constraint
    ConstraintData constraint;
    constraint.type = CONSTRAINT_FIXED;
    constraint.offset = transform;
    AddConstraint (effectorID, constraint);
  }

  void IKPhysicalNode::AddConstraint (CS::Animation::EffectorID effectorID,
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

  void IKPhysicalNode::AddConstraint (CS::Animation::EffectorID effectorID,
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

  void IKPhysicalNode::AddConstraint (CS::Animation::EffectorID effectorID,
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

  void IKPhysicalNode::RemoveConstraint (CS::Animation::EffectorID effectorID)
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

  void IKPhysicalNode::Play ()
  {
    CS_ASSERT (skeleton->GetSceneNode () && ragdollNode);

    if (isPlaying)
      return;

    // Find the scene node
    if (!sceneNode)
      sceneNode = skeleton->GetSceneNode ();

    // Find the Bullet dynamic system
    bulletDynamicSystem = scfQueryInterface<CS::Physics::Bullet::iDynamicSystem>
      (ragdollNode->GetDynamicSystem ());
    if (!bulletDynamicSystem)
    {
      factory->manager->Report (CS_REPORTER_SEVERITY_ERROR,
				"No Bullet dynamic system while starting IK animation node");
      return;
    }

    isPlaying = true;

    // Start the child node
    if (childNode)
      childNode->Play ();
  }

  void IKPhysicalNode::Stop ()
  {
    if (!isPlaying)
      return;

    isPlaying = false;

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

  void IKPhysicalNode::BlendState (CS::Animation::AnimatedMeshState* state, float baseWeight)
  {
    // Check that this node is active
    if (!isPlaying)
      return;

    // Make the child node blend the state
    if (childNode)
      childNode->BlendState (state, baseWeight);
  }

  void IKPhysicalNode::TickAnimation (float dt)
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

}
CS_PLUGIN_NAMESPACE_END(IKPhysical)
