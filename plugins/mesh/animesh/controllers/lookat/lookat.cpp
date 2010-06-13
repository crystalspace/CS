/*
  Copyright (C) 2009 Christian Van Brussel, Communications and Remote
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

#include "lookat.h"
#include <ivaria/reporter.h>
#include <imesh/bodymesh.h>
#include <csgeom/transfrm.h>
#include <iengine/scenenode.h>
#include <iengine/movable.h>
#include <iengine/camera.h>
#include <iengine/mesh.h>
#include <imesh/animesh.h>
#include <imesh/object.h>
#include <imesh/bodymesh.h>

#define TARGET_NONE 1
#define TARGET_POSITION 2
#define TARGET_MOVABLE 3
#define TARGET_CAMERA 4

#define STATUS_HEADING_TARGET 1
#define STATUS_TARGET_REACHED 2
#define STATUS_HEADING_BASE 3
#define STATUS_BASE_REACHED 4

CS_PLUGIN_NAMESPACE_BEGIN(LookAt)
{

  /********************
   *  LookAtManager
   ********************/

  SCF_IMPLEMENT_FACTORY(LookAtManager);

  CS_LEAKGUARD_IMPLEMENT(LookAtManager);

  LookAtManager::LookAtManager (iBase* parent)
    : scfImplementationType (this, parent)
  {
  }

  iSkeletonLookAtNodeFactory2* LookAtManager::CreateAnimNodeFactory
    (const char *name, iBodySkeleton* skeleton)
  {
    csRef<iSkeletonLookAtNodeFactory2> newFact;
    newFact.AttachNew (new LookAtAnimNodeFactory (this, name, skeleton));

    return factoryHash.PutUnique (name, newFact);
  }

  iSkeletonLookAtNodeFactory2* LookAtManager::FindAnimNodeFactory
    (const char* name) const
  {
    return factoryHash.Get (name, 0);
  }

  void LookAtManager::ClearAnimNodeFactories ()
  {
    factoryHash.DeleteAll ();
  }

  bool LookAtManager::Initialize (iObjectRegistry* object_reg)
  {
    this->object_reg = object_reg;
    return true;
  }

  void LookAtManager::Report (int severity, const char* msg, ...) const
  {
    va_list arg;
    va_start (arg, msg);
    csRef<iReporter> rep (csQueryRegistry<iReporter> (object_reg));
    if (rep)
      rep->ReportV (severity,
		    "crystalspace.mesh.animesh.controllers.lookat",
		    msg, arg);
    else
      {
	csPrintfV (msg, arg);
	csPrintf ("\n");
      }
    va_end (arg);
  }

  /********************
   *  LookAtAnimNodeFactory
   ********************/

  CS_LEAKGUARD_IMPLEMENT(LookAtAnimNodeFactory);

  LookAtAnimNodeFactory::LookAtAnimNodeFactory (LookAtManager* manager,
						const char *name,
						iBodySkeleton* skeleton)
    : scfImplementationType (this), manager (manager), name (name),
    skeleton (skeleton)
  {
  }

  void LookAtAnimNodeFactory::SetChildNode (iSkeletonAnimNodeFactory2* node)
  {
    childNode = node;
  }

  iSkeletonAnimNodeFactory2* LookAtAnimNodeFactory::GetChildNode ()
  {
    return childNode;
  }

  void LookAtAnimNodeFactory::ClearChildNode ()
  {
    childNode = 0;
  }

  csPtr<iSkeletonAnimNode2> LookAtAnimNodeFactory::CreateInstance (
               iSkeletonAnimPacket2* packet, iSkeleton2* skeleton)
  {
    csRef<iSkeletonAnimNode2> child;
    if (childNode)
      child = childNode->CreateInstance (packet, skeleton);

    csRef<iSkeletonAnimNode2> newP;
    newP.AttachNew (new LookAtAnimNode (this, skeleton, child));
    return csPtr<iSkeletonAnimNode2> (newP);
  }

  const char* LookAtAnimNodeFactory::GetNodeName () const
  {
    return name;
  }

  iSkeletonAnimNodeFactory2* LookAtAnimNodeFactory::FindNode
    (const char* name)
  {
    if (this->name == name)
      return this;

    if (childNode)
      return childNode->FindNode (name);

    return 0;
  }

  /********************
   *  LookAtAnimNode
   ********************/

  CS_LEAKGUARD_IMPLEMENT(LookAtAnimNode);

  LookAtAnimNode::LookAtAnimNode (LookAtAnimNodeFactory* factory, 
				  iSkeleton2* skeleton,
				  iSkeletonAnimNode2* childNode)
    : scfImplementationType (this), factory (factory), skeleton (skeleton),
    childNode (childNode), boneID (InvalidBoneID), targetMode (TARGET_NONE),
    isPlaying (false), maximumSpeed (PI), alwaysRotate (false),
    listenerMinimumDelay (0.1f)
  {
  }

  void LookAtAnimNode::SetAnimatedMesh (iAnimatedMesh* mesh)
  {
    CS_ASSERT (mesh);
    csRef<iMeshObject> animeshObject = scfQueryInterface<iMeshObject> (mesh);
    sceneNode = animeshObject->GetMeshWrapper ()->QuerySceneNode ();
  }

  void LookAtAnimNode::SetBone (BoneID boneID)
  {
    CS_ASSERT (boneID != InvalidBoneID
	       && skeleton
	       && skeleton->GetFactory ()->HasBone (boneID));

    // call listeners
    if (listenerStatus == STATUS_TARGET_REACHED)
      for (size_t i = 0; i < listeners.GetSize (); i++)
	listeners[i]->TargetLost ();

    // update data
    this->boneID = boneID;
    trackingStatus = STATUS_HEADING_TARGET;
    listenerStatus = STATUS_HEADING_TARGET;

    // check for a iBodyBoneJoint
    if (factory->skeleton)
    {
      iBodyBone* bodyBone = factory->skeleton->FindBodyBone (boneID);
      if (bodyBone)
	bodyJoint = bodyBone->GetBoneJoint ();
      else
	bodyJoint = 0;
    }
  }

  void LookAtAnimNode::SetTarget (csVector3 target)
  {
    // init tracking
    if (trackingStatus == STATUS_BASE_REACHED
	&& isPlaying)
      InitializeTracking ();

    // call listeners
    if (listenerStatus == STATUS_TARGET_REACHED)
      for (size_t i = 0; i < listeners.GetSize (); i++)
	listeners[i]->TargetLost ();

    // save new target
    targetMode = TARGET_POSITION;
    trackingStatus = STATUS_HEADING_TARGET;
    listenerStatus = STATUS_HEADING_TARGET;
    targetPosition = target;
    targetOffset = csVector3 (0);
  }

  void LookAtAnimNode::SetTarget (iMovable* target, const csVector3& offset)
  {
    CS_ASSERT (target);

    // init tracking
    if (trackingStatus == STATUS_BASE_REACHED
	&& isPlaying)
      InitializeTracking ();

    // call listeners
    if (listenerStatus == STATUS_TARGET_REACHED)
      for (size_t i = 0; i < listeners.GetSize (); i++)
	listeners[i]->TargetLost ();

    // save new target
    targetMode = TARGET_MOVABLE;
    trackingStatus = STATUS_HEADING_TARGET;
    listenerStatus = STATUS_HEADING_TARGET;
    targetMovable = target;
    targetOffset = offset;
  }

  void LookAtAnimNode::SetTarget (iCamera* target, const csVector3& offset)
  {
    CS_ASSERT (target);

    // init tracking
    if (trackingStatus == STATUS_BASE_REACHED
	&& isPlaying)
      InitializeTracking ();

    // call listeners
    if (listenerStatus == STATUS_TARGET_REACHED)
      for (size_t i = 0; i < listeners.GetSize (); i++)
	listeners[i]->TargetLost ();

    // save new target
    targetMode = TARGET_CAMERA;
    trackingStatus = STATUS_HEADING_TARGET;
    listenerStatus = STATUS_HEADING_TARGET;
    targetCamera = target;
    targetOffset = offset;
  }
  
  void LookAtAnimNode::RemoveTarget ()
  {
    // call listeners
    if (listenerStatus == STATUS_TARGET_REACHED)
      for (size_t i = 0; i < listeners.GetSize (); i++)
	listeners[i]->TargetLost ();

    // save null target
    targetMode = TARGET_NONE;
    trackingStatus = STATUS_HEADING_BASE;
    listenerStatus = STATUS_HEADING_BASE;
  }

  void LookAtAnimNode::SetMaximumSpeed (float speed)
  {
    CS_ASSERT (speed >= 0.0f);
    maximumSpeed = speed;
  }

  void LookAtAnimNode::SetAlwaysRotate (bool alwaysRotate)
  {
    this->alwaysRotate = alwaysRotate;
  }

  void LookAtAnimNode::SetListenerDelay (float delay)
  {
    CS_ASSERT (delay >= 0.0f);
    listenerMinimumDelay = delay;
  }

  void LookAtAnimNode::AddListener (iSkeletonLookAtListener2* listener)
  {
    listeners.PushSmart (listener);
  }

  void LookAtAnimNode::RemoveListener (iSkeletonLookAtListener2* listener)
  {
    listeners.Delete (listener);
  }

  void LookAtAnimNode::InitializeTracking ()
  {
    // save the current pitch/yaw
    csQuaternion rotation;
    csVector3 offset;
    skeleton->GetTransformBindSpace (boneID, rotation, offset);
    csQuaternion skeletonRotation;
    csVector3 skeletonOffset;
    skeleton->GetFactory ()->GetTransformBoneSpace (boneID, skeletonRotation,
						    skeletonOffset);
    csQuaternion quaternion = skeletonRotation.GetConjugate ()
      * rotation * skeletonRotation;
    csVector3 eulerAngles = rotation.GetEulerAngles ();
    previousPitch = eulerAngles.x;
    previousYaw = eulerAngles.y;
  }

  void LookAtAnimNode::Play ()
  {
    CS_ASSERT (boneID != InvalidBoneID);

    // init tracking
    isPlaying = true;
    trackingStatus = listenerStatus = targetMode == TARGET_NONE ?
      STATUS_BASE_REACHED : STATUS_HEADING_TARGET;
    frameDuration = 0.0f;
    listenerDelay = 0.0f;
    InitializeTracking ();

    // start child node
    if (childNode)
      childNode->Play ();
  }

  void LookAtAnimNode::Stop ()
  {
    isPlaying = false;

    // stop child node
    if (childNode)
      childNode->Stop ();
  }

  void LookAtAnimNode::SetPlaybackPosition (float time)
  {
    // nothing to do
  }

  float LookAtAnimNode::GetPlaybackPosition () const
  {
    return 0.0f;
  }

  float LookAtAnimNode::GetDuration () const
  {
    return 0.0f;
  }

  void LookAtAnimNode::SetPlaybackSpeed (float speed)
  {
    // TODO: nothing to do? apply on maximumSpeed? forward to child node?
  }

  float LookAtAnimNode::GetPlaybackSpeed () const
  {
    return 1.0f;
  }

  void LookAtAnimNode::BlendState (csSkeletalState2* state, float baseWeight)
  {
    // check that this node is active
    if (!isPlaying)
      return;

    // don't do anything if the frame duration is null
    if (frameDuration < EPSILON)
      return;

    // make the child node blend the state
    if (childNode)
      childNode->BlendState (state, baseWeight);

    // compute target position
    csVector3 target (0.0f);
    switch (targetMode)
    {
    case TARGET_NONE:
      // check we are not heading for base pose
      if (trackingStatus == STATUS_BASE_REACHED)
      {
	frameDuration = 0.0f;
	return;
      }
      else
	break;

    case TARGET_POSITION:
      target = targetPosition;
      break;

    case TARGET_MOVABLE:
      target = targetMovable->GetFullTransform ().This2Other (targetOffset);
      break;

    case TARGET_CAMERA:
      target = targetCamera->GetTransform ().This2Other (targetOffset);
      break;

    default:
      return;
    }

    // compute parent bone transform
    BoneID parentBoneID = skeleton->GetFactory ()->GetBoneParent (boneID);
    csOrthoTransform parentTransform (sceneNode->GetMovable ()->GetTransform ());

    if (parentBoneID != InvalidBoneID)
    {
      csQuaternion rotation;
      csVector3 offset;
      skeleton->GetTransformAbsSpace (parentBoneID, rotation, offset);
      csOrthoTransform parentAbsTransform (csMatrix3 (rotation.GetConjugate ()), offset);
      parentTransform = parentAbsTransform * parentTransform;
    }

    // Get the bind transform of the bone
    csQuaternion skeletonRotation;
    csVector3 skeletonOffset;
    skeleton->GetFactory ()->GetTransformBoneSpace (boneID, skeletonRotation,
						    skeletonOffset);

    // check if a child bone has already set this bone
    bool transformAlreadySet = state->IsBoneUsed (boneID);

    // compute current transform of bone
    // (don't change position if a child node has already made it)
    csOrthoTransform boneTransform (csMatrix3 (skeletonRotation),
				    transformAlreadySet ?
				    state->GetVector (boneID) : skeletonOffset);
    boneTransform = boneTransform * parentTransform;

    // compute target position
    bool targetInvalid = false;
    if (targetMode != TARGET_NONE)
    {
      // compute relative target transform
      target = boneTransform.Other2This (target);

      // check target vector not null length
      if (target.Norm () < EPSILON)
      {
	trackingStatus = STATUS_HEADING_BASE;
	targetInvalid = true;
      }
    }

    // compute needed pitch/yaw to achieve the lookat
    float targetPitch, targetYaw;
    bool wasConstrained = false;
    if (targetMode != TARGET_NONE
	&& !targetInvalid)
    {
      // compute new pitch/yaw
      target.Normalize ();    
      targetPitch = -asin (target.y);

      // (take care of round errors)
      float cosPitch = cos (targetPitch);
      float fraction = fabs (cosPitch) > SMALL_EPSILON ? target.x / cosPitch : 1.0f;
      if (fraction > 1.0f)
	fraction = 1.0f;
      else if (fraction < -1.0f)
	fraction = -1.0f;

      targetYaw = asin (fraction);
      bool sign1 = target.z > 0.0f ? true : false;
      bool sign2 = cosPitch > 0.0f ? true : false;
      if (sign1 ^ sign2)
	targetYaw = PI - targetYaw;

      // constrain angles from bodymesh data
      if (bodyJoint)
      {
	// TODO: use spherical ellipsoid constraints
	csVector3 minimumAngle = bodyJoint->GetMinimumAngle ();
	csVector3 maximumAngle = bodyJoint->GetMaximumAngle ();

	// constrain X axis
	if (bodyJoint->IsXRotConstrained ())
	{
	  if (targetPitch != 0.0f)
	    wasConstrained = true;
	  targetPitch = 0.0f;
	}
	else if (minimumAngle.x < maximumAngle.x)
	{
	  if (targetPitch < minimumAngle.x)
	  {
	    wasConstrained = true;
	    targetPitch = minimumAngle.x;
	  }
	  else if (targetPitch > maximumAngle.x)
	  {
	    wasConstrained = true;
	    targetPitch = maximumAngle.x;
	  }
	}

	// constrain Y axis
	if (bodyJoint->IsYRotConstrained ())
	{
	  if (targetYaw != 0.0f)
	    wasConstrained = true;
	  targetYaw = 0.0f;
	}
	else if (minimumAngle.y < maximumAngle.y)
	{
	  if (minimumAngle.y > -PI * 0.5f)
	  {
	    if (targetYaw < minimumAngle.y
		|| targetYaw > PI)
	    {
	      wasConstrained = true;
	      targetYaw = minimumAngle.y;
	    }
	  }
	  else if (minimumAngle.y <= -PI * 0.5f
		   && targetYaw > PI
		   && targetYaw < 2.0f * PI + minimumAngle.y)
	  {
	    wasConstrained = true;
	    targetYaw = 2.0f * PI + minimumAngle.y;
	  }

	  if (targetYaw < PI && targetYaw > maximumAngle.y)
	  {
	    wasConstrained = true;
	    targetYaw = maximumAngle.y;
	  }
	}
      }

      // 'always rotate' contraint
      if (wasConstrained
	  && !alwaysRotate)
	trackingStatus = STATUS_HEADING_BASE;
      else
	trackingStatus = STATUS_HEADING_TARGET;
    }

    // check if we head for base pose
    if (trackingStatus == STATUS_HEADING_BASE)
    {
      // take rotations from child node if any
      if (transformAlreadySet)
      {
	csQuaternion quaternion = skeletonRotation.GetConjugate ()
	  * state->GetQuaternion (boneID) * skeletonRotation;
	csVector3 eulerAngles = quaternion.GetEulerAngles ();
	targetPitch = eulerAngles.x;
	targetYaw = eulerAngles.y;
	// TODO: the roll param is not used, this makes a break when transitioning
	// in and out STATUS_BASE_REACHED
	// -> apply a portion of it when transitioning from this state
      }

      else
      {
	targetPitch = 0.0f;
	targetYaw = 0.0f;
      }
    }

    // rotation speed constraint
    // (don't constraint if the child anim must simply be played)
    if (trackingStatus != STATUS_BASE_REACHED)
    {
      if (maximumSpeed > 0.0f)
      {
	// TODO: find shortest and non-blocked path to the target
	float deltaPitch = targetPitch - previousPitch;
	float deltaYaw = targetYaw - previousYaw;

	// compute rotational speed
	float currentSpeed = sqrt (deltaPitch * deltaPitch + deltaYaw * deltaYaw)
	  / frameDuration;

	// apply constraint
	if (currentSpeed > maximumSpeed)
	{
	  float ratio = maximumSpeed / currentSpeed;
	  targetPitch = previousPitch + deltaPitch * ratio;
	  targetYaw = previousYaw + deltaYaw * ratio;

	  // constraint yaw between -PI/2 and 3*PI/2
	  if (targetYaw < -PI * 0.5f)
	    targetYaw = 2.0f * PI - targetYaw;
	  else if (targetYaw > PI * 1.5f)
	    targetYaw = targetYaw - 2.0f * PI;

	  wasConstrained = true;
	}

	// update state if target has been reached
	else
	{
	  if (trackingStatus == STATUS_HEADING_BASE)
	    trackingStatus = STATUS_BASE_REACHED;
	  else
	    trackingStatus = STATUS_TARGET_REACHED;
	}
      }

      else
	trackingStatus = STATUS_TARGET_REACHED;
    }

    // save state
    frameDuration = 0.0f;
    previousPitch = targetPitch;
    previousYaw = targetYaw;

    // check if we must simply play the child animation
    if (trackingStatus == STATUS_BASE_REACHED)
      return;

    // set new quaternion
    csQuaternion newQuaternion;
    newQuaternion.SetEulerAngles (csVector3 (targetPitch, targetYaw, 0));
    newQuaternion = skeletonRotation * newQuaternion * skeletonRotation.GetConjugate ();

    // apply new transform
    if (!transformAlreadySet)
    {
      state->SetBoneUsed (boneID);
      state->GetVector (boneID) = csVector3 (0.0f);
    }
    state->GetQuaternion (boneID) = newQuaternion;

    // call listeners
    if (listenerStatus == STATUS_TARGET_REACHED
	&& trackingStatus != STATUS_TARGET_REACHED)
    {
      if (listenerDelay > listenerMinimumDelay)
      {
	for (size_t i = 0; i < listeners.GetSize (); i++)
	  listeners[i]->TargetLost ();
	listenerStatus = trackingStatus;
      }
      else
	listenerDelay = 0.0f;
    }

    else if (listenerStatus != STATUS_TARGET_REACHED
	     && trackingStatus == STATUS_TARGET_REACHED)
      {
	for (size_t i = 0; i < listeners.GetSize (); i++)
	  listeners[i]->TargetReached ();
	listenerStatus = trackingStatus;
      }
  }

  void LookAtAnimNode::TickAnimation (float dt)
  {
    frameDuration += dt;
    listenerDelay += dt;

    if (childNode)
      childNode->TickAnimation (dt);
  }

  bool LookAtAnimNode::IsActive () const
  {
    return isPlaying;
  }

  iSkeletonAnimNodeFactory2* LookAtAnimNode::GetFactory () const
  {
    return factory;
  }

  iSkeletonAnimNode2* LookAtAnimNode::FindNode (const char* name)
  {
    if (factory->name == name)
      return this;

    if (childNode)
      return childNode->FindNode (name);

    return 0;
  }

  void LookAtAnimNode::AddAnimationCallback
    (iSkeletonAnimCallback2* callback)
  {
    // TODO
  }

  void LookAtAnimNode::RemoveAnimationCallback
    (iSkeletonAnimCallback2* callback)
  {
    // TODO
  }

}
CS_PLUGIN_NAMESPACE_END(LookAt)
