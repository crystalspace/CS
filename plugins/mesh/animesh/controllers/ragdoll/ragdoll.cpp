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

#include "ragdoll.h"
#include <ivaria/reporter.h>
#include <csgeom/transfrm.h>
#include <csgeom/plane3.h>
#include <csgeom/sphere.h>
#include <iengine/scenenode.h>
#include <iengine/movable.h>
#include <iengine/mesh.h>
#include <imesh/object.h>
#include <imesh/animesh.h>

CS_PLUGIN_NAMESPACE_BEGIN(Ragdoll)
{

  /********************
   *  RagdollManager
   ********************/

  SCF_IMPLEMENT_FACTORY(RagdollManager);

  CS_LEAKGUARD_IMPLEMENT(RagdollManager);

  RagdollManager::RagdollManager (iBase* parent)
    : scfImplementationType (this, parent)
  {
  }

  iSkeletonRagdollNodeFactory2* RagdollManager::CreateAnimNodeFactory
    (const char *name, iBodySkeleton* skeleton, iDynamicSystem* dynSys)
  {
    csRef<iSkeletonRagdollNodeFactory2> newFact;
    newFact.AttachNew (new RagdollAnimNodeFactory (this, name, skeleton, dynSys));

    return factoryHash.PutUnique (name, newFact);
  }

  iSkeletonRagdollNodeFactory2* RagdollManager::FindAnimNodeFactory
    (const char* name) const
  {
    return factoryHash.Get (name, 0);
  }

  void RagdollManager::ClearAnimNodeFactories ()
  {
    factoryHash.DeleteAll ();
  }

  bool RagdollManager::Initialize (iObjectRegistry* object_reg)
  {
    this->object_reg = object_reg;
    return true;
  }

  void RagdollManager::Report (int severity, const char* msg, ...) const
  {
    va_list arg;
    va_start (arg, msg);
    csRef<iReporter> rep (csQueryRegistry<iReporter> (object_reg));
    if (rep)
      rep->ReportV (severity,
		    "crystalspace.mesh.animesh.controllers.ragdoll",
		    msg, arg);
    else
      {
	csPrintfV (msg, arg);
	csPrintf ("\n");
      }
    va_end (arg);
  }


  /********************
   *  RagdollAnimNodeFactory
   ********************/

  CS_LEAKGUARD_IMPLEMENT(RagdollAnimNodeFactory);

  RagdollAnimNodeFactory::RagdollAnimNodeFactory (RagdollManager* manager,
	    const char *name, iBodySkeleton* skeleton, iDynamicSystem* dynSys)
    : scfImplementationType (this), manager (manager), name (name),
    bodySkeleton (skeleton), dynSys (dynSys)
  {
  }

  csPtr<iSkeletonAnimNode2> RagdollAnimNodeFactory::CreateInstance (
               iSkeletonAnimPacket2* packet, iSkeleton2* skeleton)
  {
    csRef<iSkeletonAnimNode2> newP;
    newP.AttachNew (new RagdollAnimNode (this, skeleton));
    return csPtr<iSkeletonAnimNode2> (newP);
  }

  const char* RagdollAnimNodeFactory::GetNodeName () const
  {
    return name;
  }

  iSkeletonAnimNodeFactory2* RagdollAnimNodeFactory::FindNode
    (const char* name)
  {
    // TODO: add support for child nodes
    if (this->name == name)
      return this;
    else
      return 0;
  }

  void RagdollAnimNodeFactory::AddBodyChain (iBodyChain* chain,
					     csSkeletonRagdollState state)
  {
    if (chains.GetSize ())
    {
      manager->Report (CS_REPORTER_SEVERITY_WARNING,
       "The ragdoll plugin does not yet support more than one body chain");
      return;
    }

    ChainData data;
    data.chain = chain;
    data.state = state;
    chains.PutUnique (chain->GetName (), data);
  }

  void RagdollAnimNodeFactory::RemoveBodyChain (iBodyChain* chain)
  {
    chains.DeleteAll (chain->GetName ());
  }


  /********************
   *  RagdollAnimNode
   ********************/

  CS_LEAKGUARD_IMPLEMENT(RagdollAnimNode);

  RagdollAnimNode::RagdollAnimNode (RagdollAnimNodeFactory* factory, 
				    iSkeleton2* skeleton)
    : scfImplementationType (this), factory (factory), skeleton (skeleton),
    rigidBodyCreated (false)
  {
    // copy body chains
    for (csHash<ChainData, csString>::GlobalIterator it =
	   factory->chains.GetIterator (); it.HasNext (); )
    {
      ChainData data = it.Next ();
      chains.Put (data.chain->GetName (), data);
    }
  }

  void RagdollAnimNode::SetAnimatedMesh (iAnimatedMesh* mesh)
  {
    csRef<iMeshObject> animeshObject = scfQueryInterface<iMeshObject> (mesh);
    sceneNode = animeshObject->GetMeshWrapper ()->QuerySceneNode ();
  }

  void RagdollAnimNode::SetBodyChainState (iBodyChain* chain,
					   csSkeletonRagdollState state)
  {
    if (!chains.Contains (chain->GetName ()))
      return;

    // TODO: make sure there are no bones shared between the various states
    chains[chain->GetName ()]->state = state;
  }

  csSkeletonRagdollState RagdollAnimNode::GetBodyChainState (iBodyChain* chain)
  {
    if (!chains.Contains (chain->GetName ()))
      return RAGDOLL_STATE_INACTIVE;

    return chains[chain->GetName ()]->state;
  }

  iRigidBody* RagdollAnimNode::GetBoneRigidBody (BoneID bone)
  {
    if (!bones.Contains (bone))
      return 0;

    return bones[bone]->rigidBody;
  }

  iJoint* RagdollAnimNode::GetBoneJoint (const BoneID bone)
  {
    if (!bones.Contains (bone))
      return 0;

    return bones[bone]->joint;
  }

  uint RagdollAnimNode::GetBoneCount (csSkeletonRagdollState state) const
  {
    if (state != RAGDOLL_STATE_DYNAMIC)
      return 0;

    return (uint)bones.GetSize ();
  }

  BoneID RagdollAnimNode::GetBone (csSkeletonRagdollState state, uint index) const
  {
    if (state != RAGDOLL_STATE_DYNAMIC)
      return InvalidBoneID;

    uint count = 0;
    for (csHash<Bone, BoneID>::ConstGlobalIterator it = bones.GetIterator ();
	 it.HasNext (); )
    {
      Bone bone = it.Next ();
      if (count == index)
	return bone.boneID;

      count++;
    }

    return InvalidBoneID;
  }

  void RagdollAnimNode::Play ()
  {
    if (!rigidBodyCreated)
      CreateRigidBodyTree ();
  }

  void RagdollAnimNode::Stop ()
  {
    // remove all joints
    for (csHash<Bone, BoneID>::GlobalIterator it = bones.GetIterator (); it.HasNext (); )
    {
      Bone bone = it.Next ();
      factory->dynSys->RemoveJoint (bone.joint);
    }

    // remove all rigid bodies
    for (csHash<Bone, BoneID>::GlobalIterator it = bones.GetIterator (); it.HasNext (); )
    {
      Bone bone = it.Next ();
      factory->dynSys->RemoveBody (bone.rigidBody);
    }

    // remove all ragdoll's bones
    bones.DeleteAll ();

    rigidBodyCreated = false;
  }

  void RagdollAnimNode::SetPlaybackPosition (float time)
  {
    // nothing to do
  }

  float RagdollAnimNode::GetPlaybackPosition () const
  {
    return 0.0;
  }

  float RagdollAnimNode::GetDuration () const
  {
    return 0.0;
  }

  void RagdollAnimNode::SetPlaybackSpeed (float speed)
  {
    // nothing to do
  }

  float RagdollAnimNode::GetPlaybackSpeed () const
  {
    return 1.0;
  }

  void RagdollAnimNode::BlendState (csSkeletalState2* state, float baseWeight)
  {
    for (csHash<Bone, BoneID>::GlobalIterator it = bones.GetIterator ();
	 it.HasNext(); )
    {
      Bone bone = it.Next ();
      csOrthoTransform bodyTransform = bone.rigidBody->GetTransform ();

      // TODO: test for deactivation of rigid body
      BoneID parentBoneID = skeleton->GetFactory ()->GetBoneParent (bone.boneID);

      /// if this bone is the root of the skeleton
      if (parentBoneID == InvalidBoneID)
      {
	csQuaternion boneRotation;
	csVector3 boneOffset;
	skeleton->GetFactory ()->GetTransformBoneSpace (bone.boneID, boneRotation,
							boneOffset);
	csOrthoTransform boneTransform (csMatrix3 (boneRotation), boneOffset);
	csOrthoTransform newTransform = boneTransform.GetInverse () * bodyTransform;

	iMovable* movable = sceneNode->GetMovable ();
	movable->SetTransform (newTransform);
	movable->UpdateMove ();

	continue;
      }
      
      // if this bone is inside the ragdoll chain
      if (bones.Contains (parentBoneID))
      {
	Bone nullBone;
	csOrthoTransform parentTransform = bones.Get (parentBoneID, nullBone)
	  .rigidBody->GetTransform ();
	csReversibleTransform relativeTransform =
	  bodyTransform * parentTransform.GetInverse ();

	state->SetBoneUsed (bone.boneID);
	state->GetVector (bone.boneID) = relativeTransform.GetOrigin ();
	csQuaternion quaternion;
	quaternion.SetMatrix (relativeTransform.GetT2O ());
	state->GetQuaternion (bone.boneID) = quaternion;

	continue;
      }

      // else this bone is the root of the ragdoll chain, but not of the skeleton
      else
      {
	// TODO: use kinematic object if not the root of the animesh
      }
    }
  }

  void RagdollAnimNode::TickAnimation (float dt)
  {
    // TODO: update the position of the kinematic bodies
    // TODO: blend the state of the child animation nodes by converting
    //   them to forces to be applied on the rigid bodies
  }

  bool RagdollAnimNode::IsActive () const
  {
    return rigidBodyCreated;
  }

  iSkeletonAnimNodeFactory2* RagdollAnimNode::GetFactory () const
  {
    return factory;
  }

  iSkeletonAnimNode2* RagdollAnimNode::FindNode (const char* name)
  {
    // TODO: add support for child nodes
    if (factory->name == name)
      return this;
    else
      return 0;
  }

  void RagdollAnimNode::AddAnimationCallback
    (iSkeletonAnimCallback2* callback)
  {
    // TODO
  }

  void RagdollAnimNode::RemoveAnimationCallback
    (iSkeletonAnimCallback2* callback)
  {
    // TODO
  }

  bool RagdollAnimNode::CreateRigidBodyTree ()
  {
    if (rigidBodyCreated)
      return true;

    // check availability of animated mesh
    if (!sceneNode)
    {
      factory->manager->Report (CS_REPORTER_SEVERITY_ERROR,
	       "No animesh defined while creating the ragdoll rigid body tree.\n");
      return false;
    }

    // check for the dynamic system
    if (!factory->dynSys)
    {
      factory->manager->Report (CS_REPORTER_SEVERITY_ERROR,
        "No dynamic system defined while creating the ragdoll rigid body tree.\n");
      return false;
    }

    // find the dynamic body chain
    ChainData& chainData = chains.GetIterator ().Next ();
    if (chainData.state != RAGDOLL_STATE_DYNAMIC)
    {
      factory->manager->Report (CS_REPORTER_SEVERITY_ERROR,
	"No body chains in dynamic state while creating the ragdoll rigid body tree.\n");
      return false;
    }

    // create the rigid body chain
    iBodyChainNode* rootNode = chainData.chain->GetRootNode ();
    if (!CreateRigidBodyNode (rootNode, 0))
      return false;

    rigidBodyCreated = true;

    return true;
  }

  bool RagdollAnimNode::CreateRigidBodyNode (iBodyChainNode* node,
					     iRigidBody* parentBody)
  {
    iBodyBone* bodyBone = node->GetBodyBone ();

    // check availability of collider
    if (!bodyBone->GetBoneColliderCount ())
    {
      factory->manager->Report (CS_REPORTER_SEVERITY_ERROR,
			"No colliders for bone %i while creating ragdoll chain.\n",
			bodyBone->GetAnimeshBone ());
      return false;
    }

    // check availability of joint
    if (parentBody && !bodyBone->GetBoneJoint ())
    {
      factory->manager->Report (CS_REPORTER_SEVERITY_ERROR,
			"No joint for bone %i while creating ragdoll chain.\n",
			bodyBone->GetAnimeshBone ());
      return false;
    }

    // create bone reference
    Bone bone;
    bone.boneID = bodyBone->GetAnimeshBone ();

    // create rigid body
    csRef<iRigidBody> rigidBody = factory->dynSys->CreateBody ();
    bone.rigidBody = rigidBody;

    // set body position
    csQuaternion rotation;
    csVector3 offset;
    skeleton->GetTransformAbsSpace (bone.boneID, rotation, offset);
    // TODO: we shouldn't have to use the conjugate of the quaternion, isn't it?
    csOrthoTransform boneTransform (csMatrix3 (rotation.GetConjugate ()), offset);
    csOrthoTransform animeshTransform = sceneNode->GetMovable ()->GetTransform ();
    csOrthoTransform bodyTransform = boneTransform * animeshTransform;
    rigidBody->SetTransform (bodyTransform);

    // attach bone colliders
    for (uint index = 0; index < bodyBone->GetBoneColliderCount (); index++)
    {
      iBodyBoneCollider* collider = bodyBone->GetBoneCollider (index);

      switch (collider->GetGeometryType ())
	{
	case BOX_COLLIDER_GEOMETRY:
	  {
	    csVector3 boxSize;
	    collider->GetBoxGeometry (boxSize);
	    rigidBody->AttachColliderBox
	      (boxSize, collider->GetTransform (), collider->GetFriction (),
	       collider->GetDensity (), collider->GetElasticity (),
	       collider->GetSoftness ());
	    break;
	  }

	case CYLINDER_COLLIDER_GEOMETRY:
	  {
	    float length, radius;
	    collider->GetCylinderGeometry (length, radius);
	    rigidBody->AttachColliderCylinder
	      (length, radius, collider->GetTransform (), collider->GetFriction (),
	       collider->GetDensity (), collider->GetElasticity (),
	       collider->GetSoftness ());
	    break;
	  }

	case CAPSULE_COLLIDER_GEOMETRY:
	  {
	    float length, radius;
	    collider->GetCapsuleGeometry (length, radius);
	    rigidBody->AttachColliderCapsule
	      (length, radius, collider->GetTransform (), collider->GetFriction (),
	       collider->GetDensity (), collider->GetElasticity (),
	       collider->GetSoftness ());
	    break;
	  }

	case CONVEXMESH_COLLIDER_GEOMETRY:
	  {
	    iMeshWrapper* mesh;
	    collider->GetConvexMeshGeometry (mesh);
	    rigidBody->AttachColliderConvexMesh
	      (mesh, collider->GetTransform (), collider->GetFriction (),
	       collider->GetDensity (), collider->GetElasticity (),
	       collider->GetSoftness ());
	    break;
	  }

	case TRIMESH_COLLIDER_GEOMETRY:
	  {
	    iMeshWrapper* mesh;
	    collider->GetMeshGeometry (mesh);
	    rigidBody->AttachColliderMesh
	      (mesh, collider->GetTransform (), collider->GetFriction (),
	       collider->GetDensity (), collider->GetElasticity (),
	       collider->GetSoftness ());
	    break;
	  }

	case PLANE_COLLIDER_GEOMETRY:
	  {
	    csPlane3 plane;
	    collider->GetPlaneGeometry (plane);
	    // TODO: add transform
	    rigidBody->AttachColliderPlane
	      (plane, collider->GetFriction (),
	       collider->GetDensity (), collider->GetElasticity (),
	       collider->GetSoftness ());
	    break;
	  }

	case SPHERE_COLLIDER_GEOMETRY:
	  {
	    csSphere sphere;
	    collider->GetSphereGeometry (sphere);
	    rigidBody->AttachColliderSphere
	      (sphere.GetRadius (),
	       sphere.GetCenter () + collider->GetTransform ().GetOrigin (),
	       collider->GetFriction (), collider->GetDensity (),
	       collider->GetElasticity (), collider->GetSoftness ());
	    break;
	  }

	default:
	  factory->manager->Report (CS_REPORTER_SEVERITY_ERROR,
	    "No geometry for collider in bone %i while creating ragdoll chain.\n",
				    bodyBone->GetAnimeshBone ());
	  return false;
	}
    }

    // TODO: remove bodies if problem

    // set body properties if they are defined
    iBodyBoneProperties* properties = bodyBone->GetBoneProperties ();
    if (properties)
      rigidBody->SetProperties (properties->GetMass (),
				properties->GetCenter (),
				properties->GetInertia ());

    // create dynamic joint
    if (parentBody)
      bone.joint = CreateDynamicJoint (bodyBone->GetBoneJoint (),
				       parentBody, rigidBody);

    // store bone
    bones.Put (bone.boneID, bone);

    // create child nodes
    for (uint i = 0; i < node->GetChildCount (); i++)
      if (!CreateRigidBodyNode (node->GetChild (i), rigidBody))
	return false;

    return true;
  }

  csRef<iJoint> RagdollAnimNode::CreateDynamicJoint (iBodyBoneJoint* joint,
				 iRigidBody* parentBody, iRigidBody* childBody)
  {
    csRef<iJoint> dynamicJoint = factory->dynSys->CreateJoint ();
    dynamicJoint->SetBounce (joint->GetBounce (), false);
    dynamicJoint->SetRotConstraints (joint->IsXRotConstrained (),
				     joint->IsYRotConstrained (),
				     joint->IsZRotConstrained (), false);
    dynamicJoint->SetTransConstraints (joint->IsXTransConstrained (),
				       joint->IsYTransConstrained (),
				       joint->IsZTransConstrained (), false);
    dynamicJoint->SetMaximumAngle (joint->GetMaximumAngle (), false);
    dynamicJoint->SetMaximumDistance (joint->GetMaximumDistance (), false);
    dynamicJoint->SetMinimumAngle (joint->GetMinimumAngle (), false);
    dynamicJoint->SetMinimumDistance (joint->GetMinimumDistance (), false);

    // TODO: min/max angles must be set relative to the bind space,
    //   here it will be relative to the current pose

    // attach rigid bodies
    dynamicJoint->Attach (parentBody, childBody, false);
    dynamicJoint->RebuildJoint ();
    
    return dynamicJoint;
  }

}
CS_PLUGIN_NAMESPACE_END(Ragdoll)
