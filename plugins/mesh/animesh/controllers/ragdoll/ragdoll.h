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
#ifndef __CS_RAGDOLL_H__
#define __CS_RAGDOLL_H__

#include "csutil/scf_implementation.h"
#include "iutil/comp.h"
#include "csutil/leakguard.h"
#include "csutil/weakref.h"
#include "csutil/csstring.h"
#include "imesh/ragdoll.h"
#include "ivaria/bullet.h"

CS_PLUGIN_NAMESPACE_BEGIN(Ragdoll)
{

  class RagdollManager : public scfImplementation2<RagdollManager,
    CS::Animation::iSkeletonRagdollManager2, iComponent>
  {
  public:
    CS_LEAKGUARD_DECLARE(RagdollManager);

    RagdollManager (iBase* parent);

    //-- CS::Animation::iSkeletonRagdollManager2
    virtual CS::Animation::iSkeletonRagdollNodeFactory2* CreateAnimNodeFactory (const char *name, 
               CS::Animation::iBodySkeleton* skeleton, iDynamicSystem* dynSys);

    virtual CS::Animation::iSkeletonRagdollNodeFactory2* FindAnimNodeFactory (const char* name) const;
    virtual void ClearAnimNodeFactories ();

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*);

    // error reporting
    void Report (int severity, const char* msg, ...) const;

  private:
    iObjectRegistry* object_reg;
    csHash<csRef<CS::Animation::iSkeletonRagdollNodeFactory2>, csString> factoryHash;
  };

  struct ChainData
  {
    csRef<CS::Animation::iBodyChain> chain;
    CS::Animation::RagdollState state;
  };

  class RagdollAnimNodeFactory : public scfImplementation2<RagdollAnimNodeFactory, 
    scfFakeInterface<CS::Animation::iSkeletonAnimNodeFactory2>, CS::Animation::iSkeletonRagdollNodeFactory2>
  {
    friend class RagdollAnimNode;

  public:
    CS_LEAKGUARD_DECLARE(RagdollAnimNodeFactory);

    RagdollAnimNodeFactory (RagdollManager* manager, const char *name,
			    CS::Animation::iBodySkeleton* skeleton, iDynamicSystem* dynSys);

    //-- CS::Animation::iSkeletonAnimNodeFactory2
    virtual csPtr<CS::Animation::iSkeletonAnimNode2> CreateInstance (
	       CS::Animation::iSkeletonAnimPacket2* packet, CS::Animation::iSkeleton2* skeleton);

    virtual const char* GetNodeName () const;

    virtual CS::Animation::iSkeletonAnimNodeFactory2* FindNode (const char* name);

    //-- CS::Animation::iSkeletonRagdollNodeFactory2
    virtual void AddBodyChain (CS::Animation::iBodyChain* chain,
			       CS::Animation::RagdollState state
			       = CS::Animation::STATE_INACTIVE);
    virtual void RemoveBodyChain (CS::Animation::iBodyChain* chain);

    virtual void SetChildNode (CS::Animation::iSkeletonAnimNodeFactory2* node);
    virtual CS::Animation::iSkeletonAnimNodeFactory2* GetChildNode ();
    virtual void ClearChildNode ();

  protected:
    RagdollManager* manager;
    csString name;
    csRef<CS::Animation::iBodySkeleton> bodySkeleton;
    csHash<ChainData, csString> chains;
    csWeakRef<iDynamicSystem> dynSys;
    CS::Animation::BoneID ragdollRoot;
    csRef<CS::Animation::iSkeletonAnimNodeFactory2> childNode;
  };

  class RagdollAnimNode : public scfImplementation2<RagdollAnimNode, 
    scfFakeInterface<CS::Animation::iSkeletonAnimNode2>, CS::Animation::iSkeletonRagdollNode2>
  {
  public:
    CS_LEAKGUARD_DECLARE(RagdollAnimNode);

    RagdollAnimNode (RagdollAnimNodeFactory* factory, CS::Animation::iSkeleton2* skeleton,
		    CS::Animation::iSkeletonAnimNode2* childNode);
    ~RagdollAnimNode ();

    //-- CS::Animation::iSkeletonRagdollNode2
    virtual void SetBodyChainState (CS::Animation::iBodyChain* chain, CS::Animation::RagdollState state);
    virtual CS::Animation::RagdollState GetBodyChainState (CS::Animation::iBodyChain* chain);

    virtual iRigidBody* GetBoneRigidBody (CS::Animation::BoneID bone);
    virtual iJoint* GetBoneJoint (const CS::Animation::BoneID bone);

    virtual uint GetBoneCount (CS::Animation::RagdollState state) const;
    virtual CS::Animation::BoneID GetBone (CS::Animation::RagdollState state, uint index) const;

    virtual void ResetChainTransform (CS::Animation::iBodyChain* chain);

    //-- CS::Animation::iSkeletonAnimPacket2
    virtual void Play ();

    virtual void Stop ();

    virtual void SetPlaybackPosition (float time);

    virtual float GetPlaybackPosition () const;

    virtual float GetDuration () const;

    virtual void SetPlaybackSpeed (float speed);

    virtual float GetPlaybackSpeed () const;

    virtual void BlendState (CS::Animation::csSkeletalState2* state,
			     float baseWeight = 1.0f);

    virtual void TickAnimation (float dt);

    virtual bool IsActive () const;

    virtual CS::Animation::iSkeletonAnimNodeFactory2* GetFactory () const;

    virtual CS::Animation::iSkeletonAnimNode2* FindNode (const char* name);

    virtual void AddAnimationCallback (CS::Animation::iSkeletonAnimCallback2* callback);

    virtual void RemoveAnimationCallback (CS::Animation::iSkeletonAnimCallback2* callback);

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
    csWeakRef<CS::Animation::iSkeleton2> skeleton;
    csHash<ChainData, csString> chains;
    csRef<CS::Animation::iSkeletonAnimNode2> childNode;
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
