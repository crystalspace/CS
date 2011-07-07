/*
  Copyright (C) 2009 Christian Van Brussel, Institute of Information
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

#include "lookat.h"
#include "ivaria/reporter.h"
#include "imesh/bodymesh.h"
#include "csgeom/transfrm.h"
#include "iengine/scenenode.h"
#include "iengine/movable.h"
#include "iengine/camera.h"
#include "iengine/mesh.h"
#include "imesh/animesh.h"
#include "imesh/object.h"
#include "imesh/bodymesh.h"

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
  SCF_IMPLEMENT_FACTORY(LookAtNodeManager);

  // --------------------------  LookAtNodeFactory  --------------------------

  CS_LEAKGUARD_IMPLEMENT(LookAtNodeFactory);

  LookAtNodeFactory::LookAtNodeFactory (LookAtNodeManager* manager, const char *name)
    : scfImplementationType (this), CS::Animation::SkeletonAnimNodeFactorySingle (name),
    manager (manager), boneID (CS::Animation::InvalidBoneID), maximumSpeed (PI), alwaysRotate (false),
    listenerMinimumDelay (0.1f)
  {
  }

  void LookAtNodeFactory::SetBodySkeleton (CS::Animation::iBodySkeleton* skeleton)
  {
    this->skeleton = skeleton;
  }

  void LookAtNodeFactory::SetBone (CS::Animation::BoneID boneID)
  {
    this->boneID = boneID;

    // check for a CS::Animation::iBodyBoneJoint
    if (boneID == CS::Animation::InvalidBoneID
	|| !skeleton)
      bodyJoint = 0;

    else
    {
      CS::Animation::iBodyBone* bodyBone = skeleton->FindBodyBone (boneID);
      if (bodyBone)
	bodyJoint = bodyBone->GetBoneJoint ();
      else
	bodyJoint = 0;
    }
  }

  void LookAtNodeFactory::SetMaximumSpeed (float speed)
  {
    CS_ASSERT (speed >= 0.0f);
    maximumSpeed = speed;
  }

  void LookAtNodeFactory::SetAlwaysRotate (bool alwaysRotate)
  {
    this->alwaysRotate = alwaysRotate;
  }

  void LookAtNodeFactory::SetListenerDelay (float delay)
  {
    CS_ASSERT (delay >= 0.0f);
    listenerMinimumDelay = delay;
  }

  csPtr<CS::Animation::SkeletonAnimNodeSingleBase> LookAtNodeFactory::ActualCreateInstance (
    CS::Animation::iSkeletonAnimPacket* packet,
    CS::Animation::iSkeleton* skeleton)
  {
    return csPtr<CS::Animation::SkeletonAnimNodeSingleBase> (new LookAtNode (this, skeleton));
  }

  // --------------------------  LookAtNode  --------------------------

  CS_LEAKGUARD_IMPLEMENT(LookAtNode);

  LookAtNode::LookAtNode (LookAtNodeFactory* factory, 
			  CS::Animation::iSkeleton* skeleton)
    : scfImplementationType (this),
    CS::Animation::SkeletonAnimNodeSingle<LookAtNodeFactory> (factory, skeleton),
    targetMode (TARGET_NONE), trackingInitialized (true)    
  {
  }

  bool LookAtNode::HasTarget ()
  {
    return targetMode != TARGET_NONE;
  }

  void LookAtNode::SetTarget (csVector3 target)
  {
    // init tracking
    if (trackingStatus == STATUS_BASE_REACHED
	&& isPlaying)
      trackingInitialized = false;

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

  void LookAtNode::SetTarget (iMovable* target, const csVector3& offset)
  {
    CS_ASSERT (target);

    // init tracking
    if (trackingStatus == STATUS_BASE_REACHED
	&& isPlaying)
      trackingInitialized = false;

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

  void LookAtNode::SetTarget (iCamera* target, const csVector3& offset)
  {
    CS_ASSERT (target);

    // init tracking
    if (trackingStatus == STATUS_BASE_REACHED
	&& isPlaying)
      trackingInitialized = false;

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
  
  void LookAtNode::RemoveTarget ()
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

  void LookAtNode::AddListener (CS::Animation::iSkeletonLookAtListener* listener)
  {
    listeners.PushSmart (listener);
  }

  void LookAtNode::RemoveListener (CS::Animation::iSkeletonLookAtListener* listener)
  {
    listeners.Delete (listener);
  }

  void LookAtNode::Play ()
  {
    if (isPlaying)
      return;

    // init tracking
    isPlaying = true;
    trackingStatus = listenerStatus = targetMode == TARGET_NONE ?
      STATUS_BASE_REACHED : STATUS_HEADING_TARGET;
    frameDuration = 0.0f;
    listenerDelay = 0.0f;
    trackingInitialized = false;

    // start child node
    if (childNode)
      childNode->Play ();
  }

  void LookAtNode::BlendState (CS::Animation::AnimatedMeshState* state, float baseWeight)
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

    // check if we have a valid bone to control
    if (factory->boneID == CS::Animation::InvalidBoneID)
      return;

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
    CS::Animation::BoneID parentBoneID = skeleton->GetFactory ()->GetBoneParent (factory->boneID);
    csOrthoTransform parentTransform (skeleton->GetSceneNode ()->GetMovable ()->GetFullTransform ());

    if (parentBoneID != CS::Animation::InvalidBoneID)
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
    skeleton->GetFactory ()->GetTransformBoneSpace (factory->boneID, skeletonRotation,
						    skeletonOffset);

    // check if a child bone has already set this bone
    bool transformAlreadySet = state->IsBoneUsed (factory->boneID);

    // compute current transform of bone
    // (don't change position if a child node has already made it)
    csOrthoTransform boneTransform (csMatrix3 (skeletonRotation),
				    transformAlreadySet ?
				    state->GetVector (factory->boneID) : skeletonOffset);
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

    // compute needed pitch/yaw/roll to achieve the lookat
    float targetPitch, targetYaw, targetRoll = 0;
    bool wasConstrained = false;
    if (targetMode != TARGET_NONE
	&& !targetInvalid)
    {
      // compute new pitch/yaw
      target.Normalize ();
      targetPitch = -asin (target.y);

      // (take care of round errors)
      float cosPitch = cos (targetPitch);
      float fraction = fabs (cosPitch) > SMALL_EPSILON ?
	target.x / cosPitch : 1.0f;
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
      if (factory->bodyJoint)
      {
	// TODO: use spherical ellipsoid constraints
	csVector3 minimumAngle = factory->bodyJoint->GetMinimumAngle ();
	csVector3 maximumAngle = factory->bodyJoint->GetMaximumAngle ();

	// constrain X axis
	if (factory->bodyJoint->IsXRotConstrained ())
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
	if (factory->bodyJoint->IsYRotConstrained ())
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
	  && !factory->alwaysRotate)
	trackingStatus = STATUS_HEADING_BASE;
      else
	trackingStatus = STATUS_HEADING_TARGET;
    }

    // check if we head for base pose
    if (trackingStatus == STATUS_HEADING_BASE
	|| !trackingInitialized)
    {
      // take rotations from child node if any
      if (transformAlreadySet)
      {
	csQuaternion quaternion = skeletonRotation.GetConjugate ()
	  * state->GetQuaternion (factory->boneID) * skeletonRotation;
	csVector3 eulerAngles = quaternion.GetEulerAngles ();
	targetPitch = eulerAngles.x;
	targetYaw = eulerAngles.y;
	targetRoll = eulerAngles.z;
      }

      else
      {
	targetPitch = 0.0f;
	targetYaw = 0.0f;
	targetRoll = 0.0f;
      }

      if (!trackingInitialized)
      {
	previousPitch = targetPitch;
	previousYaw = targetYaw;
	previousRoll = targetRoll;
	trackingInitialized = true;
      }
    }

    // rotation speed constraint
    // (don't constraint if the child anim must simply be played)
    if (trackingStatus != STATUS_BASE_REACHED)
    {
      if (factory->maximumSpeed > SMALL_EPSILON)
      {
	// TODO: find shortest and non-blocked path to the target
	float deltaPitch = targetPitch - previousPitch;
	float deltaYaw = targetYaw - previousYaw;
	float deltaRoll = targetRoll - previousRoll;

	// compute rotational speed
	float currentSpeed = sqrt (deltaPitch * deltaPitch
				   + deltaYaw * deltaYaw
				   + deltaRoll * deltaRoll)
	  / frameDuration;

	// apply constraint
	if (currentSpeed > factory->maximumSpeed)
	{
	  float ratio = factory->maximumSpeed / currentSpeed;
	  targetPitch = previousPitch + deltaPitch * ratio;
	  targetYaw = previousYaw + deltaYaw * ratio;
	  targetRoll = previousRoll + deltaRoll * ratio;

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
    previousRoll = targetRoll;

    // check if we must simply play the child animation
    if (trackingStatus == STATUS_BASE_REACHED)
      return;

    // set new quaternion
    csQuaternion newQuaternion;
    newQuaternion.SetEulerAngles (csVector3 (targetPitch, targetYaw, targetRoll));
    newQuaternion =
      skeletonRotation * newQuaternion * skeletonRotation.GetConjugate ();

    // apply new transform
    if (!transformAlreadySet)
    {
      state->SetBoneUsed (factory->boneID);
      state->GetVector (factory->boneID) = csVector3 (0.0f);
    }
    state->GetQuaternion (factory->boneID) = newQuaternion;

    // call listeners
    if (listenerStatus == STATUS_TARGET_REACHED
	&& trackingStatus != STATUS_TARGET_REACHED)
    {
      if (listenerDelay > factory->listenerMinimumDelay)
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

  void LookAtNode::TickAnimation (float dt)
  {
    frameDuration += playbackSpeed * dt;
    listenerDelay += playbackSpeed * dt;

    if (childNode)
      childNode->TickAnimation (dt);
  }

}
CS_PLUGIN_NAMESPACE_END(LookAt)
