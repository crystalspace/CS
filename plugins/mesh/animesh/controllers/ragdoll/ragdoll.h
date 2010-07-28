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
    iSkeletonRagdollManager2, iComponent>
  {
  public:
    CS_LEAKGUARD_DECLARE(RagdollManager);

    RagdollManager (iBase* parent);

    //-- iSkeletonRagdollManager2
    virtual iSkeletonRagdollNodeFactory2* CreateAnimNodeFactory (const char *name, 
               iBodySkeleton* skeleton, iDynamicSystem* dynSys);

    virtual iSkeletonRagdollNodeFactory2* FindAnimNodeFactory (const char* name) const;
    virtual void ClearAnimNodeFactories ();

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*);

    // error reporting
    void Report (int severity, const char* msg, ...) const;

  private:
    iObjectRegistry* object_reg;
    csHash<csRef<iSkeletonRagdollNodeFactory2>, csString> factoryHash;
  };

  struct ChainData
  {
    csRef<iBodyChain> chain;
    CS::Animation::RagdollState state;
  };

  class RagdollAnimNodeFactory : public scfImplementation2<RagdollAnimNodeFactory, 
    scfFakeInterface<iSkeletonAnimNodeFactory2>, iSkeletonRagdollNodeFactory2>
  {
    friend class RagdollAnimNode;

  public:
    CS_LEAKGUARD_DECLARE(RagdollAnimNodeFactory);

    RagdollAnimNodeFactory (RagdollManager* manager, const char *name,
			    iBodySkeleton* skeleton, iDynamicSystem* dynSys);

    //-- iSkeletonAnimNodeFactory2
    virtual csPtr<iSkeletonAnimNode2> CreateInstance (
	       iSkeletonAnimPacket2* packet, iSkeleton2* skeleton);

    virtual const char* GetNodeName () const;

    virtual iSkeletonAnimNodeFactory2* FindNode (const char* name);

    //-- iSkeletonRagdollNodeFactory2
    virtual void AddBodyChain (iBodyChain* chain,
			       CS::Animation::RagdollState state
			       = CS::Animation::STATE_INACTIVE);
    virtual void RemoveBodyChain (iBodyChain* chain);

    virtual void SetChildNode (iSkeletonAnimNodeFactory2* node);
    virtual iSkeletonAnimNodeFactory2* GetChildNode ();
    virtual void ClearChildNode ();

  protected:
    RagdollManager* manager;
    csString name;
    csRef<iBodySkeleton> bodySkeleton;
    csHash<ChainData, csString> chains;
    csWeakRef<iDynamicSystem> dynSys;
    BoneID ragdollRoot;
    csRef<iSkeletonAnimNodeFactory2> childNode;
  };

  class RagdollAnimNode : public scfImplementation2<RagdollAnimNode, 
    scfFakeInterface<iSkeletonAnimNode2>, iSkeletonRagdollNode2>
  {
  public:
    CS_LEAKGUARD_DECLARE(RagdollAnimNode);

    RagdollAnimNode (RagdollAnimNodeFactory* factory, iSkeleton2* skeleton,
		    iSkeletonAnimNode2* childNode);
    ~RagdollAnimNode ();

    //-- iSkeletonRagdollNode2
    virtual void SetAnimatedMesh (CS::Mesh::iAnimatedMesh* mesh);

    virtual void SetBodyChainState (iBodyChain* chain, CS::Animation::RagdollState state);
    virtual CS::Animation::RagdollState GetBodyChainState (iBodyChain* chain);

    virtual iRigidBody* GetBoneRigidBody (BoneID bone);
    virtual iJoint* GetBoneJoint (const BoneID bone);

    virtual uint GetBoneCount (CS::Animation::RagdollState state) const;
    virtual BoneID GetBone (CS::Animation::RagdollState state, uint index) const;

    virtual void ResetChainTransform (iBodyChain* chain);

    //-- iSkeletonAnimPacket2
    virtual void Play ();

    virtual void Stop ();

    virtual void SetPlaybackPosition (float time);

    virtual float GetPlaybackPosition () const;

    virtual float GetDuration () const;

    virtual void SetPlaybackSpeed (float speed);

    virtual float GetPlaybackSpeed () const;

    virtual void BlendState (csSkeletalState2* state,
			     float baseWeight = 1.0f);

    virtual void TickAnimation (float dt);

    virtual bool IsActive () const;

    virtual iSkeletonAnimNodeFactory2* GetFactory () const;

    virtual iSkeletonAnimNode2* FindNode (const char* name);

    virtual void AddAnimationCallback (iSkeletonAnimCallback2* callback);

    virtual void RemoveAnimationCallback (iSkeletonAnimCallback2* callback);

  private:
    struct BoneData
    {
      BoneID boneID;
      CS::Animation::RagdollState state;
      csRef<iRigidBody> rigidBody;
      csRef<iJoint> joint;
    };

    struct ResetChainData
    {
      csRef<iBodyChain> chain;
      int frameCount;
    };

    void CreateBoneData (iBodyChainNode* chainNode,
			 CS::Animation::RagdollState state);
    void SetChainNodeState (iBodyChainNode* chainNode,
			    CS::Animation::RagdollState state);
    void UpdateBoneState (BoneData* boneData);
    void ResetChainNodeTransform (iBodyChainNode* chainNode);

  private:
    RagdollAnimNodeFactory* factory;
    csWeakRef<iSceneNode> sceneNode;
    csWeakRef<iSkeleton2> skeleton;
    csHash<ChainData, csString> chains;
    csRef<iSkeletonAnimNode2> childNode;
    csHash<BoneData, BoneID> bones;
    csArray<ResetChainData> resetChains;
    bool isActive;
    BoneID maxBoneID;

    friend class BoneKinematicCallback;
  };

  class BoneKinematicCallback : public scfImplementation1
    <BoneKinematicCallback, CS::Physics::Bullet::iKinematicCallback>
  {
  public:
    BoneKinematicCallback (RagdollAnimNode* ragdollNode, BoneID boneID);
    ~BoneKinematicCallback ();

    void GetBodyTransform (iRigidBody* body, csOrthoTransform& transform) const;

  private:
    RagdollAnimNode* ragdollNode;
    BoneID boneID;
  };

}
CS_PLUGIN_NAMESPACE_END(Ragdoll)

#endif //__CS_RAGDOLL_H__
