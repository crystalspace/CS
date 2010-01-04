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

CS_PLUGIN_NAMESPACE_BEGIN(Ragdoll)
{

  class RagdollManager : public scfImplementation2<RagdollManager,
    iRagdollManager, iComponent>
  {
  public:
    CS_LEAKGUARD_DECLARE(RagdollManager);

    RagdollManager (iBase* parent);

    //-- iRagdollManager
    virtual iRagdollAnimNodeFactory* CreateAnimNodeFactory (const char *name, 
               iBodySkeleton* skeleton, iDynamicSystem* dynSys);

    virtual iRagdollAnimNodeFactory* FindAnimNodeFactory (const char* name) const;
    virtual void ClearAnimNodeFactories ();

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*);

    // error reporting
    void Report (int severity, const char* msg, ...) const;

  private:
    iObjectRegistry* object_reg;
    csHash<csRef<iRagdollAnimNodeFactory>, csString> factoryHash;
  };

  struct ChainData
  {
    csRef<iBodyChain> chain;
    csChainStateType state;
  };

  class RagdollAnimNodeFactory : public scfImplementation2<RagdollAnimNodeFactory, 
    scfFakeInterface<iSkeletonAnimNodeFactory2>, iRagdollAnimNodeFactory>
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

    //-- iRagdollAnimNodeFactory
    virtual void AddBodyChain (iBodyChain* chain,
			       csChainStateType state = RAGDOLL_STATE_INACTIVE);
    virtual void RemoveBodyChain (iBodyChain* chain);

  protected:
    RagdollManager* manager;
    csString name;
    csRef<iBodySkeleton> bodySkeleton;
    csHash<ChainData, csString> chains;
    csWeakRef<iDynamicSystem> dynSys;
    BoneID ragdollRoot;
  };


  class RagdollAnimNode : public scfImplementation2<RagdollAnimNode, 
    scfFakeInterface<iSkeletonAnimNode2>, iRagdollAnimNode>
  {
  public:
    CS_LEAKGUARD_DECLARE(RagdollAnimNode);

    RagdollAnimNode (RagdollAnimNodeFactory* factory, iSkeleton2* skeleton);

    //-- iRagdollAnimNode
    virtual void SetAnimatedMesh (iAnimatedMesh* mesh);

    virtual void SetBodyChainState (iBodyChain* chain, csChainStateType state);
    virtual csChainStateType GetBodyChainState (iBodyChain* chain);

    virtual iRigidBody* GetBoneRigidBody (BoneID bone);
    virtual iJoint* GetBoneJoint (const BoneID bone);

    virtual uint GetBoneCount (csChainStateType state) const;
    virtual BoneID GetBone (csChainStateType state, uint index) const;

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
    bool CreateRigidBodyTree ();
    bool CreateRigidBodyNode (iBodyChainNode* chainNode, iRigidBody* parentBody);
    csRef<iJoint> CreateDynamicJoint (iBodyBoneJoint* joint,
				      iRigidBody* parentBody,
				      iRigidBody* childBody);

  private:
    struct Bone
    {
      csRef<iRigidBody> rigidBody;
      csRef<iJoint> joint;
      BoneID boneID;
    };

    RagdollAnimNodeFactory* factory;
    csWeakRef<iSceneNode> sceneNode;
    csWeakRef<iSkeleton2> skeleton;
    csHash<ChainData, csString> chains;
    csHash<Bone, BoneID> bones;
    bool rigidBodyCreated;
  };

}
CS_PLUGIN_NAMESPACE_END(Ragdoll)

#endif //__CS_RAGDOLL_H__
