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
#include "iutil/comp.h"
#include "csutil/leakguard.h"
#include "csutil/weakref.h"
#include "csutil/csstring.h"
#include "imesh/animnode/ragdoll.h"
#include "ivaria/bullet.h"

CS_PLUGIN_NAMESPACE_BEGIN(Ragdoll)
{

  class RagdollNodeManager : public scfImplementation2<RagdollNodeManager,
    CS::Animation::iSkeletonRagdollNodeManager, iComponent>
  {
  public:
    CS_LEAKGUARD_DECLARE(RagdollNodeManager);

    RagdollNodeManager (iBase* parent);

    //-- CS::Animation::iSkeletonRagdollNodeManager
    virtual CS::Animation::iSkeletonRagdollNodeFactory* CreateAnimNodeFactory (const char *name, 
               CS::Animation::iBodySkeleton* skeleton, iDynamicSystem* dynSys);

    virtual CS::Animation::iSkeletonRagdollNodeFactory* FindAnimNodeFactory (const char* name) const;
    virtual void ClearAnimNodeFactories ();

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*);

    // error reporting
    void Report (int severity, const char* msg, ...) const;

  private:
    iObjectRegistry* object_reg;
    csHash<csRef<CS::Animation::iSkeletonRagdollNodeFactory>, csString> factoryHash;
  };

  struct ChainData
  {
    csRef<CS::Animation::iBodyChain> chain;
    CS::Animation::RagdollState state;
  };

  class RagdollAnimNodeFactory : public scfImplementation2<RagdollAnimNodeFactory, 
    scfFakeInterface<CS::Animation::iSkeletonAnimNodeFactory>, CS::Animation::iSkeletonRagdollNodeFactory>
  {
    friend class RagdollAnimNode;

  public:
    CS_LEAKGUARD_DECLARE(RagdollAnimNodeFactory);

    RagdollAnimNodeFactory (RagdollNodeManager* manager, const char *name,
			    CS::Animation::iBodySkeleton* skeleton, iDynamicSystem* dynSys);

    //-- CS::Animation::iSkeletonAnimNodeFactory
    virtual csPtr<CS::Animation::iSkeletonAnimNode> CreateInstance (
	       CS::Animation::iSkeletonAnimPacket* packet, CS::Animation::iSkeleton* skeleton);

    virtual const char* GetNodeName () const;

    virtual CS::Animation::iSkeletonAnimNodeFactory* FindNode (const char* name);

    //-- CS::Animation::iSkeletonRagdollNodeFactory
    virtual void AddBodyChain (CS::Animation::iBodyChain* chain,
			       CS::Animation::RagdollState state
			       = CS::Animation::STATE_INACTIVE);
    virtual void RemoveBodyChain (CS::Animation::iBodyChain* chain);

    virtual void SetChildNode (CS::Animation::iSkeletonAnimNodeFactory* node);
    virtual CS::Animation::iSkeletonAnimNodeFactory* GetChildNode () const;
    virtual void ClearChildNode ();
    virtual iDynamicSystem* GetDynamicSystem () const;
    virtual CS::Animation::iBodySkeleton* GetBodySkeleton () const;

  protected:
    RagdollNodeManager* manager;
    csString name;
    csRef<CS::Animation::iBodySkeleton> bodySkeleton;
    csArray<ChainData> chains;
    csWeakRef<iDynamicSystem> dynSys;
    CS::Animation::BoneID ragdollRoot;
    csRef<CS::Animation::iSkeletonAnimNodeFactory> childNode;
  };

  class RagdollAnimNode : public scfImplementation2<RagdollAnimNode, 
    scfFakeInterface<CS::Animation::iSkeletonAnimNode>, CS::Animation::iSkeletonRagdollNode>
  {
  public:
    CS_LEAKGUARD_DECLARE(RagdollAnimNode);

    RagdollAnimNode (RagdollAnimNodeFactory* factory, CS::Animation::iSkeleton* skeleton,
		    CS::Animation::iSkeletonAnimNode* childNode);
    ~RagdollAnimNode ();

    //-- CS::Animation::iSkeletonRagdollNode
    virtual void SetBodyChainState (CS::Animation::iBodyChain* chain, CS::Animation::RagdollState state);
    virtual CS::Animation::RagdollState GetBodyChainState (CS::Animation::iBodyChain* chain) const;

    virtual iRigidBody* GetBoneRigidBody (CS::Animation::BoneID bone);
    virtual iJoint* GetBoneJoint (const CS::Animation::BoneID bone);

    virtual uint GetBoneCount (CS::Animation::RagdollState state) const;
    virtual CS::Animation::BoneID GetBone (CS::Animation::RagdollState state, uint index) const;

    virtual CS::Animation::BoneID GetRigidBodyBone (iRigidBody* body) const;

    virtual void ResetChainTransform (CS::Animation::iBodyChain* chain);

    //-- CS::Animation::iSkeletonAnimPacket
    virtual void Play ();

    virtual void Stop ();

    virtual void SetPlaybackPosition (float time);

    virtual float GetPlaybackPosition () const;

    virtual float GetDuration () const;

    virtual void SetPlaybackSpeed (float speed);

    virtual float GetPlaybackSpeed () const;

    virtual void BlendState (CS::Animation::csSkeletalState* state,
			     float baseWeight = 1.0f);

    virtual void TickAnimation (float dt);

    virtual bool IsActive () const;

    virtual CS::Animation::iSkeletonAnimNodeFactory* GetFactory () const;

    virtual CS::Animation::iSkeletonAnimNode* FindNode (const char* name);

    virtual void AddAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback);

    virtual void RemoveAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback);

  private:
    struct BoneData
    {
      CS::Animation::BoneID boneID;
      CS::Animation::RagdollState state;
      csRef<iRigidBody> rigidBody;
      csRef<iJoint> joint;
    };

    struct ResetChainData
    {
      csRef<CS::Animation::iBodyChain> chain;
      int frameCount;
    };

    void CreateBoneData (CS::Animation::iBodyChainNode* chainNode,
			 CS::Animation::RagdollState state);
    void SetChainNodeState (CS::Animation::iBodyChainNode* chainNode,
			    CS::Animation::RagdollState state);
    void UpdateBoneState (BoneData* boneData);
    void ResetChainNodeTransform (CS::Animation::iBodyChainNode* chainNode);

  private:
    RagdollAnimNodeFactory* factory;
    csWeakRef<iSceneNode> sceneNode;
    csWeakRef<CS::Animation::iSkeleton> skeleton;
    csArray<ChainData> chains;
    csRef<CS::Animation::iSkeletonAnimNode> childNode;
    csHash<BoneData, CS::Animation::BoneID> bones;
    csArray<ResetChainData> resetChains;
    bool isActive;
    CS::Animation::BoneID maxBoneID;

    friend class BoneKinematicCallback;
  };

  class BoneKinematicCallback : public scfImplementation1
    <BoneKinematicCallback, CS::Physics::Bullet::iKinematicCallback>
  {
  public:
    BoneKinematicCallback (RagdollAnimNode* ragdollNode, CS::Animation::BoneID boneID);
    ~BoneKinematicCallback ();

    void GetBodyTransform (iRigidBody* body, csOrthoTransform& transform) const;

  private:
    RagdollAnimNode* ragdollNode;
    CS::Animation::BoneID boneID;
  };

}
CS_PLUGIN_NAMESPACE_END(Ragdoll)

#endif //__CS_RAGDOLL_H__
