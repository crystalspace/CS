/*
  Copyright (C) 2008 by Marten Svanfeldt

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

#ifndef __CS_SKELETON2_H__
#define __CS_SKELETON2_H__

#include "csutil/scf_implementation.h"
#include "imesh/skeleton2.h"
#include "iutil/comp.h"
#include "csutil/array.h"
#include "csgeom/vector3.h"
#include "csgeom/quaternion.h"
#include "csutil/hash.h"
#include "csutil/leakguard.h"
#include "csutil/weakref.h"
#include "csutil/csstring.h"

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{

  class SkeletonSystem : public scfImplementation2<SkeletonSystem, 
                                                   CS::Animation::iSkeletonManager,
                                                   iComponent>
  {
  public:
    CS_LEAKGUARD_DECLARE(SkeletonSystem);
  
    SkeletonSystem (iBase* parent);

    //-- CS::Animation::iSkeletonManager
    virtual CS::Animation::iSkeletonFactory* CreateSkeletonFactory (const char* name);
    virtual CS::Animation::iSkeletonFactory* FindSkeletonFactory (const char* name);
    virtual void ClearSkeletonFactories ();

    virtual CS::Animation::iSkeletonAnimPacketFactory* CreateAnimPacketFactory (const char* name);
    virtual CS::Animation::iSkeletonAnimPacketFactory* FindAnimPacketFactory (const char* name);
    virtual void ClearAnimPacketFactories ();

    virtual void ClearAll ();

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*);
  
  private:
    csHash<csRef<CS::Animation::iSkeletonFactory>, csString> factoryHash;
    csHash<csRef<CS::Animation::iSkeletonAnimPacketFactory>, csString> animPackets;
  };




  class SkeletonFactory : public scfImplementation1<SkeletonFactory,
                                                    CS::Animation::iSkeletonFactory>
  {
  public:
    CS_LEAKGUARD_DECLARE(SkeletonFactory);
  
    SkeletonFactory ();

    //-- CS::Animation::iSkeletonFactory
    virtual CS::Animation::BoneID CreateBone (CS::Animation::BoneID parent = CS::Animation::InvalidBoneID);
    virtual CS::Animation::BoneID FindBone (const char *name) const;
    virtual void RemoveBone (CS::Animation::BoneID bone);
    virtual CS::Animation::BoneID GetBoneParent (CS::Animation::BoneID bone) const;
    virtual bool HasBone (CS::Animation::BoneID bone) const;
    virtual void SetBoneName (CS::Animation::BoneID bone, const char* name);
    virtual const char* GetBoneName (CS::Animation::BoneID bone) const;
    virtual CS::Animation::BoneID GetTopBoneID () const;

    virtual void GetTransformBoneSpace (CS::Animation::BoneID bone, csQuaternion& rot, 
      csVector3& offset) const;
    virtual void SetTransformBoneSpace (CS::Animation::BoneID bone, const csQuaternion& rot, 
      const csVector3& offset);

    virtual void GetTransformAbsSpace (CS::Animation::BoneID bone, csQuaternion& rot, 
      csVector3& offset) const;
    virtual void SetTransformAbsSpace (CS::Animation::BoneID bone, const csQuaternion& rot, 
      const csVector3& offset);

    virtual csPtr<CS::Animation::iSkeleton> CreateSkeleton ();
 
    virtual CS::Animation::iSkeletonAnimPacketFactory* GetAnimationPacket () const;
    virtual void SetAnimationPacket (CS::Animation::iSkeletonAnimPacketFactory* fact);

    virtual void SetAutoStart (bool autostart);
    virtual bool GetAutoStart ();

    virtual csString Description () const;

    virtual const csArray<CS::Animation::BoneID>& GetBoneOrderList ();

    //-- "Private"
    void UpdateCachedTransforms ();
    void UpdateOrderList ();

  private:

    void BoneDescription (CS::Animation::BoneID bone, csString& txt, size_t level) const;

    struct Bone
    {      
      CS::Animation::BoneID parent;
      bool created;
      
      // Bone space transform
      csVector3 boneOffset;
      csQuaternion boneRotation;

      // Cached absolute transforms
      csVector3 absOffset;
      csQuaternion absRotation;

      Bone () 
        : parent (~0), created (false)
      {}
    };

    csArray<Bone> allBones;
    csSafeCopyArray<csString> boneNames;

    csArray<CS::Animation::BoneID> boneOrderList;
    csRef<CS::Animation::iSkeletonAnimPacketFactory> animationPacket;

    bool autostart;

    bool cachedTransformsDirty;
    bool orderListDirty;

    friend class Skeleton;
  };


  class Skeleton : public scfImplementation1<Skeleton, CS::Animation::iSkeleton>
  {
  public:
    CS_LEAKGUARD_DECLARE(Skeleton);
  
    Skeleton (SkeletonFactory* factory);

    //-- CS::Animation::iSkeleton
    virtual iSceneNode* GetSceneNode ();
    virtual void SetAnimatedMesh (CS::Mesh::iAnimatedMesh* animesh);
    virtual CS::Mesh::iAnimatedMesh* GetAnimatedMesh ();

    virtual void GetTransformBoneSpace (CS::Animation::BoneID bone, csQuaternion& rot, 
      csVector3& offset) const;
    virtual void SetTransformBoneSpace (CS::Animation::BoneID bone, const csQuaternion& rot, 
      const csVector3& offset);

    virtual void GetTransformAbsSpace (CS::Animation::BoneID bone, csQuaternion& rot, 
      csVector3& offset) const;
    virtual void SetTransformAbsSpace (CS::Animation::BoneID bone, const csQuaternion& rot, 
      const csVector3& offset);

    virtual void GetTransformBindSpace (CS::Animation::BoneID bone, csQuaternion& rot, 
      csVector3& offset) const;
    virtual void SetTransformBindSpace (CS::Animation::BoneID bone, const csQuaternion& rot, 
      const csVector3& offset);

    virtual csPtr<CS::Animation::AnimatedMeshState> GetStateAbsSpace ();
    virtual csPtr<CS::Animation::AnimatedMeshState> GetStateBoneSpace ();
    virtual csPtr<CS::Animation::AnimatedMeshState> GetStateBindSpace ();

    virtual CS::Animation::iSkeletonFactory* GetFactory () const;

    virtual CS::Animation::iSkeletonAnimPacket* GetAnimationPacket () const;
    virtual void SetAnimationPacket (CS::Animation::iSkeletonAnimPacket* packet);

    virtual void RecreateSkeleton ();
    virtual void ResetSkeletonState ();
    virtual void UpdateSkeleton (float dt);

    virtual unsigned int GetSkeletonStateVersion () const;

  private:
    void RecreateSkeletonP ();
    void RecreateAnimationTreeP ();
    void UpdateCachedTransforms ();

    struct Bone
    {      
      CS::Animation::BoneID parent;
      bool created;

      // Bone space transform
      csVector3 boneOffset;
      csQuaternion boneRotation;

      // Cached absolute transforms
      csVector3 absOffset;
      csQuaternion absRotation;

      // Cached bind-space transforms
      csVector3 bindOffset;
      csQuaternion bindRotation;

      Bone () 
        : parent (~0), created (false)
      {}
    };

    csArray<Bone> allBones;

    SkeletonFactory* factory;
    csRef<CS::Animation::iSkeletonAnimPacket> animationPacket;
    bool cachedTransformsDirty;
    unsigned int version;
    csWeakRef<CS::Mesh::iAnimatedMesh> animesh;
  };

}
CS_PLUGIN_NAMESPACE_END(Skeleton2)


#endif
