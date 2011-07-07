/*
  Copyright (C) 2010 Christian Van Brussel, Institute of Information
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
#ifndef __CS_RETARGET_H__
#define __CS_RETARGET_H__

#include "csutil/scf_implementation.h"
#include "cstool/animnodetmpl.h"
#include "csutil/leakguard.h"
#include "csutil/weakref.h"
#include "csutil/refarr.h"
#include "csutil/csstring.h"
#include "imesh/animnode/retarget.h"
#include "iutil/comp.h"

CS_PLUGIN_NAMESPACE_BEGIN(Retarget)
{

  class RetargetNodeManager;

  class RetargetNodeFactory : public scfImplementation2<RetargetNodeFactory, 
    scfFakeInterface<CS::Animation::iSkeletonAnimNodeFactory>, CS::Animation::iSkeletonRetargetNodeFactory>
  {
    friend class RetargetNode;

  public:
    CS_LEAKGUARD_DECLARE(RetargetNodeFactory);

    RetargetNodeFactory (RetargetNodeManager* manager, const char *name);

    //-- CS::Animation::iSkeletonRetargetNodeFactory
    virtual void SetChildNode (CS::Animation::iSkeletonAnimNodeFactory* node);
    virtual CS::Animation::iSkeletonAnimNodeFactory* GetChildNode ();
    virtual void ClearChildNode ();

    virtual void SetSourceSkeleton (CS::Animation::iSkeletonFactory* skeleton);
    virtual void SetBoneMapping (CS::Animation::BoneMapping& mapping);

    virtual void AddBodyChain (CS::Animation::iBodyChain* chain);
    virtual void RemoveBodyChain (CS::Animation::iBodyChain* chain);

    //-- CS::Animation::iSkeletonAnimNodeFactory
    virtual csPtr<CS::Animation::iSkeletonAnimNode> CreateInstance (
	       CS::Animation::iSkeletonAnimPacket* packet, CS::Animation::iSkeleton* skeleton);

    virtual const char* GetNodeName () const;

    virtual CS::Animation::iSkeletonAnimNodeFactory* FindNode (const char* name);

  protected:
    RetargetNodeManager* manager;
    csString name;
    csRef<CS::Animation::iSkeletonAnimNodeFactory> childNode;
    csRef<CS::Animation::iSkeletonFactory> sourceSkeleton;
    CS::Animation::BoneMapping boneMapping;
    csRefArray<CS::Animation::iBodyChain> bodyChains;
  };

  class RetargetNode : public scfImplementation2<RetargetNode, 
    scfFakeInterface<CS::Animation::iSkeletonAnimNode>, CS::Animation::iSkeletonRetargetNode>
  {
  public:
    CS_LEAKGUARD_DECLARE(RetargetNode);

    RetargetNode (RetargetNodeFactory* factory, CS::Animation::iSkeleton* skeleton,
		      CS::Animation::iSkeletonAnimNode* childNode);

    //-- CS::Animation::iSkeletonRetargetNode

    //-- CS::Animation::iSkeletonAnimNode
    virtual void Play ();
    virtual void Stop ();

    virtual void SetPlaybackPosition (float time);
    virtual float GetPlaybackPosition () const;

    virtual float GetDuration () const;
    virtual void SetPlaybackSpeed (float speed);
    virtual float GetPlaybackSpeed () const;

    virtual void BlendState (CS::Animation::AnimatedMeshState* state,
			     float baseWeight = 1.0f);
    virtual void TickAnimation (float dt);

    virtual bool IsActive () const;
    virtual CS::Animation::iSkeletonAnimNodeFactory* GetFactory () const;
    virtual CS::Animation::iSkeletonAnimNode* FindNode (const char* name);

    virtual void AddAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback);
    virtual void RemoveAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback);

  private:
    RetargetNodeFactory* factory;
    csWeakRef<CS::Animation::iSkeleton> skeleton;
    csRef<CS::Animation::iSkeletonAnimNode> childNode;
    bool isPlaying;
    CS::Animation::AnimatedMeshState childState;
    csHash<csQuaternion, CS::Animation::BoneID> alignRotations;

    struct RotationCache
    {
      RotationCache (CS::Animation::iSkeletonFactory* skeletonFactory)
      : skeletonFactory (skeletonFactory) {}

      void SetBoneRotation (CS::Animation::BoneID bone, const csQuaternion& rotation)
      {
	rotations.PutUnique (bone, rotation);
      }

      csQuaternion GetBoneRotation (CS::Animation::BoneID bone)
      {
	csQuaternion quaternion;
	csVector3 offset;

	if (rotations.Contains (bone))
	  return *rotations[bone];

	CS::Animation::BoneID parentBone = skeletonFactory->GetBoneParent (bone);
	if (parentBone == CS::Animation::InvalidBoneID)
	  skeletonFactory->GetTransformAbsSpace (bone, quaternion, offset);

	else
	{
	  skeletonFactory->GetTransformBoneSpace (bone, quaternion, offset);
	  quaternion = GetBoneRotation (parentBone) * quaternion;
	}

	rotations.Put (bone, quaternion);
	return quaternion;
      }

    private:
      CS::Animation::iSkeletonFactory* skeletonFactory;
      csHash<csQuaternion, CS::Animation::BoneID> rotations;
    };
  };

  class RetargetNodeManager
    : public CS::Animation::AnimNodeManagerCommon<RetargetNodeManager,
						  CS::Animation::iSkeletonRetargetNodeManager,
						  RetargetNodeFactory>
  {
  public:
    RetargetNodeManager (iBase* parent)
     : AnimNodeManagerCommonType (parent) {}
  };

}
CS_PLUGIN_NAMESPACE_END(Retarget)

#endif //__CS_RETARGET_H__
