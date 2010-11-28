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
#include "csgeom/math.h"
#include "csgeom/transfrm.h"
#include "imesh/bodymesh.h"
#include "ivaria/reporter.h"
#include "retarget.h"

CS_PLUGIN_NAMESPACE_BEGIN(Retarget)
{

  /********************
   *  RetargetNodeManager
   ********************/

  SCF_IMPLEMENT_FACTORY(RetargetNodeManager);

  CS_LEAKGUARD_IMPLEMENT(RetargetNodeManager);

  RetargetNodeManager::RetargetNodeManager (iBase* parent)
    : scfImplementationType (this, parent)
  {
  }

  CS::Animation::iSkeletonRetargetNodeFactory* RetargetNodeManager::CreateAnimNodeFactory
    (const char *name)
  {
    csRef<CS::Animation::iSkeletonRetargetNodeFactory> newFact;
    newFact.AttachNew (new RetargetAnimNodeFactory (this, name));

    return factoryHash.PutUnique (name, newFact);
  }

  CS::Animation::iSkeletonRetargetNodeFactory* RetargetNodeManager::FindAnimNodeFactory
    (const char* name) const
  {
    return factoryHash.Get (name, 0);
  }

  void RetargetNodeManager::ClearAnimNodeFactories ()
  {
    factoryHash.DeleteAll ();
  }

  bool RetargetNodeManager::Initialize (iObjectRegistry* object_reg)
  {
    this->object_reg = object_reg;
    return true;
  }

  void RetargetNodeManager::Report (int severity, const char* msg, ...) const
  {
    va_list arg;
    va_start (arg, msg);
    csRef<iReporter> rep (csQueryRegistry<iReporter> (object_reg));
    if (rep)
      rep->ReportV (severity,
		    "crystalspace.mesh.animesh.animnode.retarget",
		    msg, arg);
    else
      {
	csPrintfV (msg, arg);
	csPrintf ("\n");
      }
    va_end (arg);
  }

  /********************
   *  RetargetAnimNodeFactory
   ********************/

  CS_LEAKGUARD_IMPLEMENT(RetargetAnimNodeFactory);

  RetargetAnimNodeFactory::RetargetAnimNodeFactory (RetargetNodeManager* manager, const char *name)
    : scfImplementationType (this), manager (manager), name (name),
    retargetMode (CS::Animation::RETARGET_NAIVE)
  {
  }

  void RetargetAnimNodeFactory::SetChildNode (CS::Animation::iSkeletonAnimNodeFactory* node)
  {
    childNode = node;
  }

  CS::Animation::iSkeletonAnimNodeFactory* RetargetAnimNodeFactory::GetChildNode ()
  {
    return childNode;
  }

  void RetargetAnimNodeFactory::ClearChildNode ()
  {
    childNode = 0;
  }

  void RetargetAnimNodeFactory::SetSourceSkeleton (CS::Animation::iSkeletonFactory* skeleton)
  {
    sourceSkeleton = skeleton;
  }

  void RetargetAnimNodeFactory::SetBoneMapping (CS::Animation::BoneMapping& mapping)
  {
    boneMapping = mapping;
  }

  void RetargetAnimNodeFactory::AddBodyChain (CS::Animation::iBodyChain* chain)
  {
    // TODO: check every nodes have only one child
    bodyChains.Push (chain);
  }

  void RetargetAnimNodeFactory::RemoveBodyChain (CS::Animation::iBodyChain* chain)
  {
    bodyChains.Delete (chain);
  }

  void RetargetAnimNodeFactory::SetRetargetMode (CS::Animation::RetargetMode mode)
  {
    retargetMode = mode;
  }

 CS::Animation::RetargetMode RetargetAnimNodeFactory::GetRetargetMode () const
 {
   return retargetMode;
 }

  csPtr<CS::Animation::iSkeletonAnimNode> RetargetAnimNodeFactory::CreateInstance
    (CS::Animation::iSkeletonAnimPacket* packet, CS::Animation::iSkeleton* skeleton)
  {
    // TODO: asserts

    csRef<CS::Animation::iSkeletonAnimNode> child;
    if (childNode)
      child = childNode->CreateInstance (packet, skeleton);

    csRef<CS::Animation::iSkeletonAnimNode> newP;
    newP.AttachNew (new RetargetAnimNode (this, skeleton, child));
    return csPtr<CS::Animation::iSkeletonAnimNode> (newP);
  }

  const char* RetargetAnimNodeFactory::GetNodeName () const
  {
    return name;
  }

  CS::Animation::iSkeletonAnimNodeFactory* RetargetAnimNodeFactory::FindNode (const char* name)
  {
    if (this->name == name)
      return this;

    if (childNode)
      return childNode->FindNode (name);

    return 0;
  }

  /********************
   *  RetargetAnimNode
   ********************/

  CS_LEAKGUARD_IMPLEMENT(RetargetAnimNode);

  RetargetAnimNode::RetargetAnimNode (RetargetAnimNodeFactory* factory, 
				      CS::Animation::iSkeleton* skeleton,
				      CS::Animation::iSkeletonAnimNode* childNode)
    : scfImplementationType (this), factory (factory), skeleton (skeleton), childNode (childNode),
    isPlaying (false)
  {
    if (factory->retargetMode != CS::Animation::RETARGET_ALIGN_BONES)
      return;

    // TODO: check the validity between the chains
    // TODO: move this computation in the factory

    // Compute the base rotations to align the bones of the source and the target skeletons
    RotationCache rotationCache (skeleton->GetFactory ());
    for (csRefArray<CS::Animation::iBodyChain>::Iterator it = factory->bodyChains.GetIterator ();
	 it.HasNext (); )
    {
      csRef<CS::Animation::iBodyChain> chain = it.Next ();

      CS::Animation::iBodyChainNode* node = chain->GetRootNode ();
      while (node)
      {
	// Check if there is a child bone
	if (!node->GetChildCount ())
	  break;

	CS::Animation::BoneID targetBoneID = node->GetAnimeshBone ();
	node = node->GetChild (0);
	CS::Animation::BoneID childTargetBoneID = node->GetAnimeshBone ();

	// Find the corresponding bones in the source skeleton
	CS::Animation::BoneID sourceBoneID = factory->boneMapping.GetSourceBone (targetBoneID);
	if (sourceBoneID == CS::Animation::InvalidBoneID)
	  continue;

	CS::Animation::BoneID childSourceBoneID = factory->boneMapping.GetSourceBone (childTargetBoneID);
	if (childSourceBoneID == CS::Animation::InvalidBoneID)
	  continue;

	// Read the transforms of the source and target bones
	csQuaternion targetAbsQuaternion = rotationCache.GetBoneRotation (targetBoneID);
	csQuaternion sourceAbsQuaternion;
	csVector3 sourceOffset;
	factory->sourceSkeleton->GetTransformAbsSpace (sourceBoneID, sourceAbsQuaternion, sourceOffset);

	// Get the position of the child bones
	csQuaternion childTargetBoneQuaternion;
	csVector3 childTargetBoneOffset;
	skeleton->GetFactory ()->GetTransformBoneSpace (childTargetBoneID, childTargetBoneQuaternion,
							childTargetBoneOffset);

	csQuaternion childSourceBoneQuaternion;
	csVector3 childSourceBoneOffset;
	factory->sourceSkeleton->GetTransformBoneSpace (childSourceBoneID, childSourceBoneQuaternion,
							childSourceBoneOffset);

	// Compute the rotation to align the bones
	csVector3 mainAxis (1.0f, 0.0f, 0.0f);
	csVector3 cross;
	float dot;
	// TODO: would csQuaternion::Rotate() apply the inverse transform?
	csVector3 childSourceTargetOffset =
	  (targetAbsQuaternion.GetConjugate () * sourceAbsQuaternion).Rotate (childSourceBoneOffset);

	childTargetBoneOffset.Normalize ();
	childSourceTargetOffset.Normalize ();

	cross = childTargetBoneOffset % childSourceTargetOffset;
	cross.Normalize ();
	dot = childSourceTargetOffset * childTargetBoneOffset;

	// Check the validity of the dot product
	// (If the two vectors are colinear then the rotation will have no effect,
	// so the validity of the cross product has not to be checked)
	if (dot > 0.99999f)
	  dot = 0.99999f;
	else if (dot < -0.99999f)
	  dot = -0.99999f;

	csQuaternion target2SourceQuaternion;
	target2SourceQuaternion.SetAxisAngle (cross, acos (dot));

	// Store the rotation computed
	alignRotations.PutUnique (targetBoneID, target2SourceQuaternion);
	rotationCache.SetBoneRotation (targetBoneID, targetAbsQuaternion * target2SourceQuaternion);
      }
    }
  }

  void RetargetAnimNode::Play ()
  {
    if (isPlaying)
      return;
    isPlaying = true;

    // Start the child node
    if (childNode)
    {
      CS_ASSERT (factory->sourceSkeleton);
      childState.Setup (factory->sourceSkeleton->GetTopBoneID () + 1);
      childNode->Play ();
    }
  }

  void RetargetAnimNode::Stop ()
  {
    isPlaying = false;

    // Stop the child node
    if (childNode)
      childNode->Stop ();
  }

  void RetargetAnimNode::SetPlaybackPosition (float time)
  {
    if (childNode)
      childNode->SetPlaybackPosition (time);
  }

  float RetargetAnimNode::GetPlaybackPosition () const
  {
    if (childNode)
      return childNode->GetPlaybackPosition ();

    return 0.0f;
  }

  float RetargetAnimNode::GetDuration () const
  {
    if (childNode)
      return childNode->GetDuration ();

    return 0.0f;
  }

  void RetargetAnimNode::SetPlaybackSpeed (float speed)
  {
    if (childNode)
      childNode->SetPlaybackSpeed (speed);
  }

  float RetargetAnimNode::GetPlaybackSpeed () const
  {
    if (childNode)
      return childNode->GetPlaybackSpeed ();

    return 1.0f;
  }

  void RetargetAnimNode::BlendState (CS::Animation::csSkeletalState* state, float baseWeight)
  {
    // Check that this node is active
    if (!isPlaying || !childNode)
      return;

    // Make the child node blend the state
    childState.Reset ();
    childNode->BlendState (&childState, 1.0f);

    // Retarget the child animation into the skeletal state
    // TODO: use everywhere GetBoneOrderList
    const csArray<CS::Animation::BoneID>& boneList = skeleton->GetFactory ()->GetBoneOrderList ();
    for (csArray<CS::Animation::BoneID>::ConstIterator it = boneList.GetIterator (); it.HasNext (); )
    {
      CS::Animation::BoneID targetBoneID = it.Next ();

      // Find the corresponding bone in the source skeleton
      CS::Animation::BoneID sourceBoneID = factory->boneMapping.GetSourceBone (targetBoneID);
      if (sourceBoneID == CS::Animation::InvalidBoneID
	  || !childState.IsBoneUsed (sourceBoneID))
	continue;

      // Read the transforms of the source and target bones
      csQuaternion targetAbsQuaternion;
      csQuaternion targetBoneQuaternion;
      csVector3 targetOffset;
      skeleton->GetFactory ()->GetTransformBoneSpace (targetBoneID, targetBoneQuaternion, targetOffset);
      skeleton->GetFactory ()->GetTransformAbsSpace (targetBoneID, targetAbsQuaternion, targetOffset);

      csQuaternion sourceAbsQuaternion;
      csQuaternion sourceBoneQuaternion;
      csVector3 sourceOffset;
      factory->sourceSkeleton->GetTransformBoneSpace (sourceBoneID, sourceBoneQuaternion, sourceOffset);
      factory->sourceSkeleton->GetTransformAbsSpace (sourceBoneID, sourceAbsQuaternion, sourceOffset);

      // Naive application of the rotation on the target bone
      csQuaternion targetQuaternion;
      if (factory->retargetMode == CS::Animation::RETARGET_NAIVE
	  || (factory->retargetMode == CS::Animation::RETARGET_ALIGN_BONES
	      && !alignRotations.Contains (targetBoneID)))
	targetQuaternion = targetAbsQuaternion.GetConjugate ()
	  * sourceAbsQuaternion * sourceBoneQuaternion.GetConjugate ()
	  * childState.GetQuaternion (sourceBoneID)
	  * sourceBoneQuaternion * sourceAbsQuaternion.GetConjugate ()
	  * targetAbsQuaternion;

      // Retarget the bone by aligning the source and target child bones
      else if (factory->retargetMode == CS::Animation::RETARGET_ALIGN_BONES)
	targetQuaternion = *alignRotations[targetBoneID] * targetAbsQuaternion.GetConjugate ()
	  * sourceAbsQuaternion * sourceBoneQuaternion.GetConjugate ()
	  * childState.GetQuaternion (sourceBoneID)
	  * sourceBoneQuaternion * sourceAbsQuaternion.GetConjugate ()
	  * targetAbsQuaternion;

      // TODO: blend weight
      state->SetBoneUsed (targetBoneID);
      //state->GetQuaternion (targetBoneID) = targetQuaternion;
      state->GetQuaternion (targetBoneID) = state->GetQuaternion (targetBoneID).SLerp
	(targetBoneQuaternion * targetQuaternion * targetBoneQuaternion.GetConjugate (), baseWeight);
      state->GetVector (targetBoneID) = csLerp (state->GetVector (targetBoneID),
						childState.GetVector (sourceBoneID), baseWeight);
    }
  }

  void RetargetAnimNode::TickAnimation (float dt)
  {
    if (childNode)
      childNode->TickAnimation (dt);
  }

  bool RetargetAnimNode::IsActive () const
  {
    return isPlaying;
  }

  CS::Animation::iSkeletonAnimNodeFactory* RetargetAnimNode::GetFactory () const
  {
    return factory;
  }

  CS::Animation::iSkeletonAnimNode* RetargetAnimNode::FindNode (const char* name)
  {
    if (factory->name == name)
      return this;

    if (childNode)
      return childNode->FindNode (name);

    return 0;
  }

  void RetargetAnimNode::AddAnimationCallback
    (CS::Animation::iSkeletonAnimCallback* callback)
  {
    // TODO
  }

  void RetargetAnimNode::RemoveAnimationCallback
    (CS::Animation::iSkeletonAnimCallback* callback)
  {
    // TODO
  }

}
CS_PLUGIN_NAMESPACE_END(Retarget)
