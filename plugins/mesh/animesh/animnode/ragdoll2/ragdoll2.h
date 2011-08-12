/*
  Copyright (C) 2009-10 Christian Van Brussel, Communications and Remote
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
#ifndef __CS_RAGDOLL_H__
#define __CS_RAGDOLL_H__

#include "csutil/scf_implementation.h"
#include "cstool/animnodetmpl.h"
#include "csutil/leakguard.h"
#include "csutil/weakref.h"
#include "csutil/csstring.h"
#include "imesh/animnode/ragdoll2.h"
#include "iutil/comp.h"
#include "ivaria/physical2.h"

CS_PLUGIN_NAMESPACE_BEGIN(Ragdoll2)
{

class RagdollNodeManager;

struct ChainData
{
  csRef<CS::Animation::iBodyChain2> chain;
  CS::Animation::RagdollState state;
};

class RagdollNodeFactory
  : public scfImplementation2<RagdollNodeFactory,
  scfFakeInterface<CS::Animation::iSkeletonAnimNodeFactory>,
  CS::Animation::iSkeletonRagdollNodeFactory2>,
  public CS::Animation::SkeletonAnimNodeFactorySingle
{
public:
  CS_LEAKGUARD_DECLARE(RagdollNodeFactory);

  RagdollNodeFactory (RagdollNodeManager* manager, const char *name);

  //-- CS::Animation::iSkeletonRagdollNodeFactory
  virtual void SetBodySkeleton (CS::Animation::iBodySkeleton2* skeleton);
  virtual CS::Animation::iBodySkeleton2* GetBodySkeleton () const;

  virtual void AddBodyChain (CS::Animation::iBodyChain2* chain,
		       CS::Animation::RagdollState state
		       = CS::Animation::STATE_INACTIVE);
  virtual void RemoveBodyChain (CS::Animation::iBodyChain2* chain);

  inline virtual void SetChildNode (CS::Animation::iSkeletonAnimNodeFactory* factory)
  { CS::Animation::SkeletonAnimNodeFactorySingle::SetChildNode (factory); }
  inline virtual iSkeletonAnimNodeFactory* GetChildNode () const
  { return CS::Animation::SkeletonAnimNodeFactorySingle::GetChildNode (); }

  //-- CS::Animation::iSkeletonAnimNodeFactory
  csPtr<CS::Animation::SkeletonAnimNodeSingleBase> ActualCreateInstance (
    CS::Animation::iSkeletonAnimPacket* packet, CS::Animation::iSkeleton* skeleton);
protected:
  RagdollNodeManager* manager;
  csRef<CS::Animation::iBodySkeleton2> bodySkeleton;
  csArray<ChainData> chains;
  CS::Animation::BoneID ragdollRoot;

  friend class RagdollNode;
};

class RagdollNode
  : public scfImplementation2<RagdollNode,
			scfFakeInterface<CS::Animation::iSkeletonAnimNode>,
			CS::Animation::iSkeletonRagdollNode2>,
    CS::Animation::SkeletonAnimNodeSingle<RagdollNodeFactory>
{
public:
  CS_LEAKGUARD_DECLARE(RagdollNode);

  RagdollNode (RagdollNodeFactory* factory, CS::Animation::iSkeleton* skeleton);
  ~RagdollNode ();

  //-- CS::Animation::iSkeletonRagdollNode
  virtual void SetPhysicalSystem (CS::Physics2::iPhysicalSystem* system);
  virtual CS::Physics2::iPhysicalSystem* GetPhysicalSystem () const;
  virtual void SetPhysicalSector (CS::Physics2::iPhysicalSector* sector);
  virtual CS::Physics2::iPhysicalSector* GetPhysicalSector () const;
  virtual void SetBodyChainState (CS::Animation::iBodyChain2* chain, CS::Animation::RagdollState state);
  virtual CS::Animation::RagdollState GetBodyChainState (CS::Animation::iBodyChain2* chain) const;

  virtual CS::Physics2::iRigidBody* GetBoneRigidBody (CS::Animation::BoneID bone);
  virtual CS::Physics2::iJoint* GetBoneJoint (const CS::Animation::BoneID bone);

  virtual uint GetBoneCount (CS::Animation::RagdollState state) const;
  virtual CS::Animation::BoneID GetBone (CS::Animation::RagdollState state, uint index) const;

  virtual CS::Animation::BoneID GetRigidBodyBone (CS::Physics2::iRigidBody* body) const;

  virtual void ResetChainTransform (CS::Animation::iBodyChain2* chain);

  //-- CS::Animation::iSkeletonAnimNode
  virtual void Play ();
  virtual void Stop ();

  virtual void BlendState (CS::Animation::AnimatedMeshState* state,
		     float baseWeight = 1.0f);
private:
  struct BoneData
  {
    CS::Animation::BoneID boneID;
    CS::Animation::RagdollState state;
    csRef<CS::Physics2::iRigidBody> rigidBody;
    csRef<CS::Physics2::iJoint> joint;
  };

  struct ResetChainData
  {
    csRef<CS::Animation::iBodyChain2> chain;
    int frameCount;
  };

  void CreateBoneData (CS::Animation::iBodyChainNode2* chainNode,
		 CS::Animation::RagdollState state);
  void SetChainNodeState (CS::Animation::iBodyChainNode2* chainNode,
		    CS::Animation::RagdollState state);
  void UpdateBoneState (BoneData* boneData);
  void ResetChainNodeTransform (CS::Animation::iBodyChainNode2* chainNode);

private:
  csWeakRef<iSceneNode> sceneNode;
  csWeakRef<CS::Collision2::iCollisionSystem> collisionSystem;
  csWeakRef<CS::Physics2::iPhysicalSystem> physicalSystem;
  csWeakRef<CS::Physics2::iPhysicalSector> physicalSector;
  csArray<ChainData> chains;
  csHash<BoneData, CS::Animation::BoneID> bones;
  csArray<ResetChainData> resetChains;
  CS::Animation::BoneID maxBoneID;

  friend class RagdollNodeFactory;
  friend class BoneKinematicCallback;
};

class BoneKinematicCallback : public scfImplementation1
  <BoneKinematicCallback, CS::Physics2::iKinematicCallback>
{
public:
  BoneKinematicCallback (RagdollNode* ragdollNode, CS::Animation::BoneID boneID);
  ~BoneKinematicCallback ();

  virtual void GetBodyTransform (CS::Physics2::iRigidBody* body, csOrthoTransform& transform) const;

private:
  RagdollNode* ragdollNode;
  CS::Animation::BoneID boneID;
};

class RagdollNodeManager
  : public CS::Animation::AnimNodeManagerCommon<RagdollNodeManager,
					  CS::Animation::iSkeletonRagdollNodeManager2,
					  RagdollNodeFactory>
{
public:
  RagdollNodeManager (iBase* parent)
   : AnimNodeManagerCommonType (parent) {}
   
  void Report (int severity, const char* msg, ...) const;
};

}
CS_PLUGIN_NAMESPACE_END(Ragdoll2)

#endif //__CS_RAGDOLL_H__
