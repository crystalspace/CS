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

#include "cssysdef.h"

#include "csutil/scf.h"

#include "skeleton2.h"

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{
  SCF_IMPLEMENT_FACTORY(SkeletonSystem)


  SkeletonSystem::SkeletonSystem (iBase* parent)
    : scfImplementationType (this, parent)
  {}

  csPtr<iSkeletonFactory2> SkeletonSystem::CreateSkeletonFactory ()
  {
    return new SkeletonFactory;
  }

  csPtr<iSkeletonAnimationFactory2> SkeletonSystem::CreateAnimationNodeFactory ()
  {
    return 0;
  }

  csPtr<iSkeletonBlendNodeFactory2> SkeletonSystem::CreateBlendNodeFactory ()
  {
    return 0;
  }

  bool SkeletonSystem::Initialize (iObjectRegistry*)
  {
    return true;
  }



  SkeletonFactory::SkeletonFactory ()
    : scfImplementationType (this)
  {}

  BoneID SkeletonFactory::CreateBone (BoneID parent)
  {
    return 0;
  }

  void SkeletonFactory::RemoveBone (BoneID bone)
  {

  }

  BoneID SkeletonFactory::GetBoneParent (BoneID bone) const
  {
    return 0;
  }

  bool SkeletonFactory::HasBone (BoneID bone) const
  {
    return false;
  }

  void SkeletonFactory::GetTransformBoneSpace (BoneID bone, csQuaternion& rot, 
    csVector3& offset) const
  {

  }
  void SkeletonFactory::SetTransformBoneSpace (BoneID bone, const csQuaternion& rot, 
    const csVector3& offset)
  {

  }

  void SkeletonFactory::GetTransformAbsSpace (BoneID bone, csQuaternion& rot, 
    csVector3& offset) const
  {

  }

  void SkeletonFactory::SetTransformAbsSpace (BoneID bone, const csQuaternion& rot, 
    const csVector3& offset)
  {

  }

  csPtr<iSkeleton2> SkeletonFactory::CreateSkeleton ()
  {
    return 0;
  }

  iSkeletonAnimationNodeFactory2* SkeletonFactory::GetAnimationRoot () const
  {
    return 0;
  }

  void SkeletonFactory::SetAnimationRoot (iSkeletonAnimationNodeFactory2* fact)
  {

  }




  Skeleton::Skeleton ()
    : scfImplementationType (this)
  {
  }

  iSceneNode* Skeleton::GetSceneNode ()
  {
    return 0;
  }

  void Skeleton::GetTransformBoneSpace (BoneID bone, csQuaternion& rot, 
    csVector3& offset) const
  {
  }

  void Skeleton::SetTransformBoneSpace (BoneID bone, const csQuaternion& rot, 
    const csVector3& offset)
  {
  }

  void Skeleton::GetTransformAbsSpace (BoneID bone, csQuaternion& rot, 
    csVector3& offset) const
  {
  }

  void Skeleton::SetTransformAbsSpace (BoneID bone, const csQuaternion& rot, 
    const csVector3& offset)
  {
  }

  void Skeleton::GetTransformBindSpace (BoneID bone, csQuaternion& rot, 
    csVector3& offset) const
  {
  }

  void Skeleton::SetTransformBindSpace (BoneID bone, const csQuaternion& rot, 
    const csVector3& offset)
  {
  }

  iSkeletonFactory2* Skeleton::GetFactory () const
  {
    return 0;
  }

  iSkeletonAnimationNode2* Skeleton::GetAnimationRoot () const
  {
    return 0;
  }

  void Skeleton::RecreateAnimationTree ()
  {
  }

  void Skeleton::UpdateSkeleton (float dt)
  {
  }


}
CS_PLUGIN_NAMESPACE_END(Skeleton2)
