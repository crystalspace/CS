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

#include "ikccd.h"
#include "ivaria/reporter.h"
#include "iengine/camera.h"
#include "iengine/scenenode.h"
#include "iengine/movable.h"
#include "imesh/bodymesh.h"

#include "iutil/visualdebug.h"

CS_PLUGIN_NAMESPACE_BEGIN(IKCCD)
{

  // --------------------------  IKCCDNodeManager  --------------------------

  SCF_IMPLEMENT_FACTORY(IKCCDNodeManager);

  CS_LEAKGUARD_IMPLEMENT(IKCCDNodeManager);

  IKCCDNodeManager::IKCCDNodeManager (iBase* parent)
    : scfImplementationType (this, parent)
  {
  }

  CS::Animation::iSkeletonIKNodeFactory* IKCCDNodeManager::CreateAnimNodeFactory
    (const char *name, CS::Animation::iBodySkeleton* skeleton)
  {
    CS_ASSERT(skeleton);

    csRef<CS::Animation::iSkeletonIKCCDNodeFactory> newFact;
    newFact.AttachNew (new IKCCDNodeFactory (this, name, skeleton));

    return factoryHash.PutUnique (name, newFact);
  }

  CS::Animation::iSkeletonIKNodeFactory* IKCCDNodeManager::FindAnimNodeFactory
    (const char* name) const
  {
    return factoryHash.Get (name, 0);
  }

  void IKCCDNodeManager::ClearAnimNodeFactories ()
  {
    factoryHash.DeleteAll ();
  }

  bool IKCCDNodeManager::Initialize (iObjectRegistry* object_reg)
  {
    this->object_reg = object_reg;
    return true;
  }

  void IKCCDNodeManager::Report (int severity, const char* msg, ...) const
  {
    va_list arg;
    va_start (arg, msg);
    csRef<iReporter> rep (csQueryRegistry<iReporter> (object_reg));
    if (rep)
      rep->ReportV (severity,
		    "crystalspace.mesh.animesh.controllers.IKCCD",
		    msg, arg);
    else
      {
	csPrintfV (msg, arg);
	csPrintf ("\n");
      }
    va_end (arg);
  }

  // --------------------------  IKCCDNodeFactory  --------------------------

  CS_LEAKGUARD_IMPLEMENT(IKCCDNodeFactory);

  IKCCDNodeFactory::IKCCDNodeFactory
    (IKCCDNodeManager* manager, const char *name, CS::Animation::iBodySkeleton* skeleton)
    : scfImplementationType (this), manager (manager), name (name),
    bodySkeleton (skeleton), maxEffectorID (0), maximumIterations (50), targetDistance (0.001f),
    motionRatio (0.1f), jointInitialized (true), upwardIterations (true)
  {
  }

  csPtr<CS::Animation::iSkeletonAnimNode> IKCCDNodeFactory::CreateInstance
    (CS::Animation::iSkeletonAnimPacket* packet, CS::Animation::iSkeleton* skeleton)
  {
    csRef<CS::Animation::iSkeletonAnimNode> child;
    if (childNode)
      child = childNode->CreateInstance (packet, skeleton);

    csRef<CS::Animation::iSkeletonAnimNode> newP;
    newP.AttachNew (new IKCCDNode (this, skeleton, child));
    return csPtr<CS::Animation::iSkeletonAnimNode> (newP);
  }

  const char* IKCCDNodeFactory::GetNodeName () const
  {
    return name;
  }

  CS::Animation::iSkeletonAnimNodeFactory* IKCCDNodeFactory::FindNode
    (const char* name)
  {
    if (this->name == name)
      return this;

    if (childNode)
      return childNode->FindNode (name);

      return 0;
  }

  CS::Animation::EffectorID IKCCDNodeFactory::AddEffector
    (CS::Animation::iBodyChain* chain, CS::Animation::BoneID bone,
     csOrthoTransform& transform)
  {
    CS_ASSERT (chain->GetRootNode ()->FindSubChild (bone));

    EffectorData effector;
    effector.chain = chain;
    effector.bone = bone;
    effector.transform = transform;
    effectors.Put (maxEffectorID, effector);

    return maxEffectorID++;
  }

  void IKCCDNodeFactory::RemoveEffector (CS::Animation::EffectorID effector)
  {
    effectors.DeleteAll (effector);
  }

  void IKCCDNodeFactory::SetChildNode (CS::Animation::iSkeletonAnimNodeFactory* node)
  {
    childNode = node;
  }

  CS::Animation::iSkeletonAnimNodeFactory* IKCCDNodeFactory::GetChildNode () const
  {
    return childNode;
  }

  void IKCCDNodeFactory::ClearChildNode ()
  {
    childNode = 0;
  }

  void IKCCDNodeFactory::SetMaximumIterations (size_t max)
  {
    maximumIterations = max;
  }

  size_t IKCCDNodeFactory::GetMaximumIterations ()
  {
    return maximumIterations;
  }

  void IKCCDNodeFactory::SetTargetDistance (float distance)
  {
    targetDistance = distance;
  }

  float IKCCDNodeFactory::GetTargetDistance ()
  {
    return targetDistance;
  }

  void IKCCDNodeFactory::SetMotionRatio (float ratio)
  {
    motionRatio = ratio;
  }

  float IKCCDNodeFactory::GetMotionRatio ()
  {
    return motionRatio;
  }

  void IKCCDNodeFactory::SetJointInitialization (bool initialized)
  {
    jointInitialized = initialized;
  }

  bool IKCCDNodeFactory::GetJointInitialization ()
  {
    return jointInitialized;
  }

  void IKCCDNodeFactory::SetUpwardIterations (bool upward)
  {
    upwardIterations = upward;
  }

  bool IKCCDNodeFactory::GetUpwardIterations ()
  {
    return upwardIterations;
  }

  // --------------------------  IKCCDNode  --------------------------

  CS_LEAKGUARD_IMPLEMENT(IKCCDNode);

  IKCCDNode::IKCCDNode (IKCCDNodeFactory* factory, 
			CS::Animation::iSkeleton* skeleton,
			CS::Animation::iSkeletonAnimNode* childNode)
    : scfImplementationType (this), factory (factory), sceneNode (nullptr),
    skeleton (skeleton), childNode (childNode), isActive (false)
  {
  }

  IKCCDNode::~IKCCDNode ()
  {
    Stop ();
  }

  void IKCCDNode::AddConstraint (CS::Animation::EffectorID effectorID,
				 csOrthoTransform& transform)
  {
    CS_ASSERT(factory->effectors.Contains (effectorID));

    // Record the constraint
    ConstraintData constraint;
    constraint.type = CONSTRAINT_FIXED;
    constraint.offset = transform;
    constraints.PutUnique (effectorID, constraint);
  }

  void IKCCDNode::AddConstraint (CS::Animation::EffectorID effectorID,
				 iMovable* target,
				 const csOrthoTransform& offset)
  {
    CS_ASSERT(factory->effectors.Contains (effectorID));

    // Record the constraint
    ConstraintData constraint;
    constraint.type = CONSTRAINT_MOVABLE;
    constraint.movable = target;
    constraint.offset = offset;
    constraints.PutUnique (effectorID, constraint);
  }

  void IKCCDNode::AddConstraint (CS::Animation::EffectorID effectorID,
				 iCamera* target,
				 const csOrthoTransform& offset)
  {
    CS_ASSERT(factory->effectors.Contains (effectorID));

    // Record the constraint
    ConstraintData constraint;
    constraint.type = CONSTRAINT_CAMERA;
    constraint.camera = target;
    constraint.offset = offset;
    constraints.PutUnique (effectorID, constraint);
  }

  void IKCCDNode::RemoveConstraint (CS::Animation::EffectorID effectorID)
  {
    CS_ASSERT(factory->effectors.Contains (effectorID));

    constraints.DeleteAll (effectorID);
  }

  void IKCCDNode::Play ()
  {
    CS_ASSERT (skeleton->GetSceneNode ());

    if (isActive)
      return;

    // Find the scene node
    if (!sceneNode)
      sceneNode = skeleton->GetSceneNode ();

    isActive = true;

    // Start the child node
    if (childNode)
      childNode->Play ();
  }

  void IKCCDNode::Stop ()
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

  void IKCCDNode::SetPlaybackPosition (float time)
  {
    if (childNode)
      childNode->SetPlaybackPosition (time);
  }

  float IKCCDNode::GetPlaybackPosition () const
  {
    if (childNode)
      return childNode->GetPlaybackPosition ();

    return 0.0;
  }

  float IKCCDNode::GetDuration () const
  {
    if (childNode)
      return childNode->GetDuration ();

    return 0.0;
  }

  void IKCCDNode::SetPlaybackSpeed (float speed)
  {
    if (childNode)
      childNode->SetPlaybackSpeed (speed);
  }

  float IKCCDNode::GetPlaybackSpeed () const
  {
    if (childNode)
      return childNode->GetPlaybackSpeed ();

    return 1.0;
  }

  void IKCCDNode::BlendState (CS::Animation::csSkeletalState* state, float baseWeight)
  {
    // Check that this node is active
    if (!isActive)
      return;

    // Make the child node blend the state
    if (childNode)
      childNode->BlendState (state, baseWeight);

    // Update the position of all active constraints
    csQuaternion identityRotation;
    for (csHash<ConstraintData, CS::Animation::EffectorID>::GlobalIterator it =
	   constraints.GetIterator (); it.HasNext (); )
    {
      csTuple2<ConstraintData, CS::Animation::EffectorID> tuple = it.NextTuple ();
      ConstraintData constraint = tuple.first;
      EffectorData* effector = factory->effectors[tuple.second];

      // Compute the transform of the target
      csOrthoTransform targetTransform;
      switch (constraint.type)
      {
      case CONSTRAINT_FIXED:
	{
	  targetTransform = constraint.offset;
	}
	break;

      case CONSTRAINT_MOVABLE:
	{
	  targetTransform = constraint.offset * constraint.movable->GetFullTransform ();
	}
	break;

      case CONSTRAINT_CAMERA:
	{
	  targetTransform = constraint.offset * constraint.camera->GetTransform ();
	}
	break;

      default:
	continue;
	break;
      }

      csQuaternion targetRotation;
      targetRotation.SetMatrix (targetTransform.GetO2T ());
      csVector3 targetOffset = targetTransform.GetOrigin ();

      // Compute the transform of the base of the bone chain
      csQuaternion chainRotation;
      csVector3 chainOffset (0.0f);
      CS::Animation::BoneID parentBoneID =
	skeleton->GetFactory ()->GetBoneParent (effector->chain->GetRootNode ()->GetAnimeshBone ());
      if (parentBoneID != CS::Animation::InvalidBoneID)
	skeleton->GetTransformAbsSpace (parentBoneID, chainRotation, chainOffset);

      csOrthoTransform meshTransform = skeleton->GetSceneNode ()->GetMovable ()->GetFullTransform ();
      csQuaternion meshRotation;
      meshRotation.SetMatrix (meshTransform.GetO2T ());
      chainOffset = meshTransform.GetOrigin () + meshRotation.Rotate (chainOffset);
      chainRotation = chainRotation * meshRotation;

      // Find the bone of the effector
      CS::Animation::iBodyChainNode* effectorNode = effector->chain->GetRootNode ()->FindSubChild (effector->bone);

      // Initialize the bone data
      csQuaternion beforeRotation, afterRotation;
      csVector3 beforeOffset (0.0f), afterOffset (0.0f);

      csArray<BoneData> bones;
      CS::Animation::iBodyChainNode* iteratorNode = effectorNode;
      if (!iteratorNode)
	continue;

      while (iteratorNode)
      {
	// Create an entry for the bone
	BoneData bone;
	bone.boneID = iteratorNode->GetAnimeshBone ();

	// Read the bone transform of the joint
	skeleton->GetFactory ()->GetTransformBoneSpace (bone.boneID, bone.boneRotation, bone.boneOffset);

	// Check if the rotation has to be initialized with the walue from the child node
	if (factory->jointInitialized && state->IsBoneUsed (bone.boneID))
	  bone.rotation = bone.boneRotation.GetConjugate () * state->GetQuaternion (bone.boneID) * bone.boneRotation;

	bones.Insert (0, bone);

	iteratorNode = iteratorNode->GetParent ();
      }

      size_t iterations = 0;
      float currentDistance = 1000.0f;
      while (iterations < factory->maximumIterations
	     && currentDistance > factory->targetDistance)
      {
	int boneIndex = factory->upwardIterations ? bones.GetSize () - 1 : 0;
	while ((factory->upwardIterations && boneIndex >= 0)
	       || (!factory->upwardIterations && boneIndex < (int) bones.GetSize ()))
	{
	  BoneData& bone = bones[boneIndex];

	  csQuaternion beforeRotation, afterRotation;
	  csVector3 beforeOffset (0.0f), afterOffset (0.0f);

	  // Compute the transforms of the bone and the effector
	  for (int boneIndexI = 0; boneIndexI < (int) bones.GetSize (); boneIndexI++)
	  {
	    BoneData& bone = bones[boneIndexI];

	    if (boneIndexI <= boneIndex)
	    {
	      beforeOffset = beforeOffset + beforeRotation.Rotate (bone.boneOffset);
	      beforeRotation = beforeRotation * bone.boneRotation * bone.rotation;
	    }

	    else
	    {
	      afterOffset = afterOffset + afterRotation.Rotate (bone.boneOffset);
	      afterRotation = afterRotation * bone.boneRotation * bone.rotation;
	    }
	  }

	  // Compute the transforms from the bone to the target and the effector
	  csVector3 boneOffset = chainOffset + chainRotation.Rotate (beforeOffset);
	  csQuaternion boneRotation = chainRotation * beforeRotation;

	  csVector3 bone2target = boneRotation.GetConjugate ().Rotate (targetOffset - boneOffset);
	  csVector3 bone2effector = afterOffset + afterRotation.Rotate (effector->transform.GetOrigin ());

	  // Check if the effector is close enough to the target
	  currentDistance = (bone2target - bone2effector).Norm ();
	  if (currentDistance < factory->targetDistance)
	    break;

	  // Compute the rotation needed to align the target
	  bone2target.Normalize ();
	  bone2effector.Normalize ();
	  csVector3 cross = bone2effector % bone2target;
	  cross.Normalize ();
	  float dot = bone2target * bone2effector;

	  // Check the validity of the dot product
	  // (If bone2effector and bone2target are colinear then the rotation will have no effect,
	  // so the validity of the cross product has not to be checked)
	  if (dot > 0.99999f)
	    dot = 0.99999f;
	  else if (dot < -0.99999f)
	    dot = -0.99999f;

	  csQuaternion previousRotation = bone.rotation;
	  bone.rotation.SetAxisAngle (cross, acos (dot));
	  bone.rotation = previousRotation * bone.rotation;

	  // Apply only a fraction of the rotation
	  // TODO: use a specific per bone ratio
	  bone.rotation = previousRotation.SLerp (bone.rotation, factory->motionRatio);

	  // TODO: apply bone constraints

	  if (factory->upwardIterations)
	    boneIndex--;
	  else
	    boneIndex++;
	}

	iterations++;
      }

      // Apply the bone transforms
      for (int boneIndexI = 0; boneIndexI < (int) bones.GetSize (); boneIndexI++)
      {
	BoneData& bone = bones[boneIndexI];
	state->SetBoneUsed (bone.boneID);
	state->GetVector (bone.boneID).Set (0.0f);
	state->GetQuaternion (bone.boneID) = bone.boneRotation * bone.rotation * bone.boneRotation.GetConjugate ();
      }

      // TODO: events for target lost/reached
    }
  }

  void IKCCDNode::TickAnimation (float dt)
  {
    // Update the child node
    if (childNode)
      childNode->TickAnimation (dt);
  }

  bool IKCCDNode::IsActive () const
  {
    return isActive;
  }

  CS::Animation::iSkeletonAnimNodeFactory* IKCCDNode::GetFactory () const
  {
    return factory;
  }

  CS::Animation::iSkeletonAnimNode* IKCCDNode::FindNode (const char* name)
  {
    if (factory->name == name)
      return this;

    if (childNode)
      return childNode->FindNode (name);

    return 0;
  }

  void IKCCDNode::AddAnimationCallback
    (CS::Animation::iSkeletonAnimCallback* callback)
  {
    // TODO
  }

  void IKCCDNode::RemoveAnimationCallback
    (CS::Animation::iSkeletonAnimCallback* callback)
  {
    // TODO
  }

}
CS_PLUGIN_NAMESPACE_END(IKCCD)
