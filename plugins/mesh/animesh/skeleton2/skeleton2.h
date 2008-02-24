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

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{

  class SkeletonSystem : public scfImplementation2<SkeletonSystem, 
                                                   iSkeletonSystem2,
                                                   iComponent>
  {
  public:
    SkeletonSystem (iBase* parent);

    //-- iSkeletonSystem2
    virtual csPtr<iSkeletonFactory2> CreateSkeletonFactory ();
    virtual csPtr<iSkeletonAnimationFactory2> CreateAnimationNodeFactory ();
    virtual csPtr<iSkeletonBlendNodeFactory2> CreateBlendNodeFactory ();

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*);
  
  private:
  };


  class SkeletonFactory : public scfImplementation1<SkeletonFactory,
                                                    iSkeletonFactory2>
  {
  public:
    SkeletonFactory ();

    //-- iSkeletonFactory2
    virtual BoneID CreateBone (BoneID parent = (BoneID)~0);
    virtual void RemoveBone (BoneID bone);
    virtual BoneID GetBoneParent (BoneID bone) const;
    virtual bool HasBone (BoneID bone) const;

    virtual void GetTransformBoneSpace (BoneID bone, csQuaternion& rot, 
      csVector3& offset) const;
    virtual void SetTransformBoneSpace (BoneID bone, const csQuaternion& rot, 
      const csVector3& offset);

    virtual void GetTransformAbsSpace (BoneID bone, csQuaternion& rot, 
      csVector3& offset) const;
    virtual void SetTransformAbsSpace (BoneID bone, const csQuaternion& rot, 
      const csVector3& offset);

    virtual csPtr<iSkeleton2> CreateSkeleton ();
 
    virtual iSkeletonAnimationNodeFactory2* GetAnimationRoot () const;

    virtual void SetAnimationRoot (iSkeletonAnimationNodeFactory2* fact);

  private:
  };


  class Skeleton : public scfImplementation1<Skeleton, iSkeleton2>
  {
  public:
    Skeleton ();


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

    virtual iSkeletonFactory2* GetFactory () const;

    virtual iSkeletonAnimationNode2* GetAnimationRoot () const;

    virtual void RecreateAnimationTree ();

    virtual void UpdateSkeleton (float dt);

  private:
  };

}
CS_PLUGIN_NAMESPACE_END(Skeleton2)


#endif
