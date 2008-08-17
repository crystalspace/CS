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
#include "csutil/csstring.h"

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{

  class SkeletonSystem : public scfImplementation2<SkeletonSystem, 
                                                   iSkeletonManager2,
                                                   iComponent>
  {
  public:
    CS_LEAKGUARD_DECLARE(SkeletonSystem);
  
    SkeletonSystem (iBase* parent);

    //-- iSkeletonManager2
    virtual iSkeletonFactory2* CreateSkeletonFactory (const char* name);
    virtual iSkeletonFactory2* FindSkeletonFactory (const char* name);
    virtual void ClearSkeletonFactories ();

    virtual iSkeletonAnimPacketFactory2* CreateAnimPacketFactory (const char* name);
    virtual iSkeletonAnimPacketFactory2* FindAnimPacketFactory (const char* name);
    virtual void ClearAnimPacketFactories ();

    virtual void ClearAll ();

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*);
  
  private:
    csHash<csRef<iSkeletonFactory2>, csString> factoryHash;
    csHash<csRef<iSkeletonAnimPacketFactory2>, csString> animPackets;
  };




  class SkeletonFactory : public scfImplementation1<SkeletonFactory,
                                                    iSkeletonFactory2>
  {
  public:
    CS_LEAKGUARD_DECLARE(SkeletonFactory);
  
    SkeletonFactory ();

    //-- iSkeletonFactory2
    virtual BoneID CreateBone (BoneID parent = InvalidBoneID);
    virtual BoneID FindBone (const char *name) const;
    virtual void RemoveBone (BoneID bone);
    virtual BoneID GetBoneParent (BoneID bone) const;
    virtual bool HasBone (BoneID bone) const;
    virtual void SetBoneName (BoneID bone, const char* name);
    virtual const char* GetBoneName (BoneID bone) const;
    virtual BoneID GetTopBoneID () const;

    virtual void GetTransformBoneSpace (BoneID bone, csQuaternion& rot, 
      csVector3& offset) const;
    virtual void SetTransformBoneSpace (BoneID bone, const csQuaternion& rot, 
      const csVector3& offset);

    virtual void GetTransformAbsSpace (BoneID bone, csQuaternion& rot, 
      csVector3& offset) const;
    virtual void SetTransformAbsSpace (BoneID bone, const csQuaternion& rot, 
      const csVector3& offset);

    virtual csPtr<iSkeleton2> CreateSkeleton ();
 
    virtual iSkeletonAnimPacketFactory2* GetAnimationPacket () const;
    virtual void SetAnimationPacket (iSkeletonAnimPacketFactory2* fact);

    //-- "Private"
    inline const csArray<size_t>& GetOrderList () const
    {
      return boneOrderList;
    }

    void UpdateCachedTransforms ();
    void UpdateOrderList ();

  private:    

    struct Bone
    {      
      BoneID parent;
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

    csArray<size_t> boneOrderList;
    csRef<iSkeletonAnimPacketFactory2> animationPacket;    

    bool cachedTransformsDirty;
    bool orderListDirty;

    friend class Skeleton;
  };


  class Skeleton : public scfImplementation1<Skeleton, iSkeleton2>
  {
  public:
    CS_LEAKGUARD_DECLARE(Skeleton);
  
    Skeleton (SkeletonFactory* factory);


    //-- iSkeleton2
    virtual iSceneNode* GetSceneNode ();

    virtual void GetTransformBoneSpace (BoneID bone, csQuaternion& rot, 
      csVector3& offset) const;
    virtual void SetTransformBoneSpace (BoneID bone, const csQuaternion& rot, 
      const csVector3& offset);

    virtual void GetTransformAbsSpace (BoneID bone, csQuaternion& rot, 
      csVector3& offset) const;
    virtual void SetTransformAbsSpace (BoneID bone, const csQuaternion& rot, 
      const csVector3& offset);

    virtual void GetTransformBindSpace (BoneID bone, csQuaternion& rot, 
      csVector3& offset) const;
    virtual void SetTransformBindSpace (BoneID bone, const csQuaternion& rot, 
      const csVector3& offset);

    virtual csPtr<csSkeletalState2> GetStateAbsSpace ();
    virtual csPtr<csSkeletalState2> GetStateBoneSpace ();
    virtual csPtr<csSkeletalState2> GetStateBindSpace ();

    virtual iSkeletonFactory2* GetFactory () const;

    virtual iSkeletonAnimPacket2* GetAnimationPacket () const;
    virtual void SetAnimationPacket (iSkeletonAnimPacket2* packet);

    virtual void RecreateSkeleton ();
    virtual void UpdateSkeleton (float dt);

    virtual unsigned int GetSkeletonStateVersion () const;

  private:
    void RecreateSkeletonP ();
    void RecreateAnimationTreeP ();
    void UpdateCachedTransforms ();

    struct Bone
    {      
      BoneID parent;
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
    csRef<iSkeletonAnimPacket2> animationPacket;
    bool cachedTransformsDirty;
    unsigned int version, versionLastReset;
  };

}
CS_PLUGIN_NAMESPACE_END(Skeleton2)


#endif
