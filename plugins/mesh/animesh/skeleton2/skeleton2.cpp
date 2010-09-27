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
#include "csutil/graphalg.h"

#include "skeleton2.h"
#include "animation.h"
#include "nodes.h"
#include "utilities.h"
#include "imesh/animesh.h"
#include "imesh/object.h"
#include "iengine/mesh.h"

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{
  SCF_IMPLEMENT_FACTORY(SkeletonSystem)


  CS_LEAKGUARD_IMPLEMENT(SkeletonSystem);

  SkeletonSystem::SkeletonSystem (iBase* parent)
    : scfImplementationType (this, parent)
  {}

  CS::Animation::iSkeletonFactory* SkeletonSystem::CreateSkeletonFactory (const char* name)
  {
    // Check name uniqueness
    csRef<CS::Animation::iSkeletonFactory> newFact = csPtr<CS::Animation::iSkeletonFactory> (new SkeletonFactory);

    return factoryHash.PutUnique (name, newFact);
  }

  CS::Animation::iSkeletonFactory* SkeletonSystem::FindSkeletonFactory (const char* name)
  {
    return factoryHash.Get (name, 0);
  }

  void SkeletonSystem::ClearSkeletonFactories ()
  {
    factoryHash.DeleteAll ();
  }

  CS::Animation::iSkeletonAnimPacketFactory* SkeletonSystem::CreateAnimPacketFactory (const char* name)
  {
    // Check name uniqueness
    csRef<CS::Animation::iSkeletonAnimPacketFactory> newFact = 
      csPtr<CS::Animation::iSkeletonAnimPacketFactory> (new AnimationPacketFactory);

    return animPackets.PutUnique (name, newFact);
  }

  CS::Animation::iSkeletonAnimPacketFactory* SkeletonSystem::FindAnimPacketFactory (const char* name)
  {
    return animPackets.Get (name, 0);
  }

  void SkeletonSystem::ClearAnimPacketFactories ()
  {
    animPackets.DeleteAll ();
  }

  void SkeletonSystem::ClearAll ()
  {
    factoryHash.DeleteAll ();
    animPackets.DeleteAll ();
  }

  bool SkeletonSystem::Initialize (iObjectRegistry*)
  {
    return true;
  }



  CS_LEAKGUARD_IMPLEMENT(SkeletonFactory);

  SkeletonFactory::SkeletonFactory ()
    : scfImplementationType (this), autostart (true), cachedTransformsDirty (true), 
    orderListDirty (true)
  {}

  CS::Animation::BoneID SkeletonFactory::FindBone (const char *name) const
  {
    for (size_t i = 0; i < boneNames.GetSize (); i++)
    {
      if (!strcmp(name,boneNames[i]))
        return (CS::Animation::BoneID)i;
    }
    return CS::Animation::InvalidBoneID;
  }

  CS::Animation::BoneID SkeletonFactory::CreateBone (CS::Animation::BoneID parent)
  {
    cachedTransformsDirty = true;
    orderListDirty = true;

    // See if there are any empty slots
    for (size_t i = 0; i < allBones.GetSize (); ++i)
    {
      if (!allBones[i].created)
      {
        Bone& bone = allBones[i];
        bone.created = true;
        bone.parent = parent;

        boneNames[i] = "NEWBONE";

        return (CS::Animation::BoneID)i;
      }
    }

    Bone newBone;
    newBone.parent = parent;
    newBone.created = true;

    boneNames.Push ("NEWBONE");
    return (CS::Animation::BoneID)allBones.Push (newBone);
  }

  void SkeletonFactory::RemoveBone (CS::Animation::BoneID bone)
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);

    if (bone == allBones.GetSize ()-1)
    {
      allBones.SetSize (bone);
      boneNames.SetSize (bone);
    }
    else
    {
      allBones[bone].created = false;
      boneNames[bone] = "DELETED";
    }

    // Handle bones parented to bone...
    cachedTransformsDirty = true;
    orderListDirty = true;
  }

  CS::Animation::BoneID SkeletonFactory::GetBoneParent (CS::Animation::BoneID bone) const
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);

    return allBones[bone].parent;
  }

  bool SkeletonFactory::HasBone (CS::Animation::BoneID bone) const
  {
    return (bone < allBones.GetSize ()) && 
           allBones[bone].created;
  }

  void SkeletonFactory::SetBoneName (CS::Animation::BoneID bone, const char* name)
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);

    boneNames[bone] = name;
  }

  const char* SkeletonFactory::GetBoneName (CS::Animation::BoneID bone) const
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);

    return boneNames[bone];
  }

  CS::Animation::BoneID SkeletonFactory::GetTopBoneID () const
  {
    return (CS::Animation::BoneID)(allBones.GetSize () - 1);
  }

  void SkeletonFactory::GetTransformBoneSpace (CS::Animation::BoneID bone, csQuaternion& rot, 
    csVector3& offset) const
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);

    const Bone& boneRef = allBones[bone];
    rot = boneRef.boneRotation;
    offset = boneRef.boneOffset;
  }

  void SkeletonFactory::SetTransformBoneSpace (CS::Animation::BoneID bone, const csQuaternion& rot, 
    const csVector3& offset)
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);

    cachedTransformsDirty = true;

    Bone& boneRef = allBones[bone];
    boneRef.boneRotation = rot;
    boneRef.boneOffset = offset;
  }

  void SkeletonFactory::GetTransformAbsSpace (CS::Animation::BoneID bone, csQuaternion& rot, 
    csVector3& offset) const
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);
    
    const Bone& boneRef = allBones[bone];
    
    if (boneRef.parent == CS::Animation::InvalidBoneID)
    {
      rot = boneRef.boneRotation;
      offset = boneRef.boneOffset;
      return;
    }

    if (cachedTransformsDirty)
    {
      csQuaternion parentRot;
      csVector3 parentOffset;
      GetTransformAbsSpace (boneRef.parent, parentRot, parentOffset);

      TransformQVFrame (parentRot, parentOffset, 
        boneRef.boneRotation, boneRef.boneOffset, rot, offset);
    }
    else
    {
      rot = boneRef.absRotation;
      offset = boneRef.absOffset;
    }
  }

  void SkeletonFactory::SetTransformAbsSpace (CS::Animation::BoneID bone, const csQuaternion& rot, 
    const csVector3& offset)
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);

    Bone& boneRef = allBones[bone];

    if (boneRef.parent == CS::Animation::InvalidBoneID)
    {
      boneRef.boneRotation = rot;
      boneRef.boneOffset = offset;
      return;
    }

    csQuaternion parentRot;
    csVector3 parentOffset;
    GetTransformAbsSpace (boneRef.parent, parentRot, parentOffset);

    TransformQVFrameInv (parentRot, parentOffset,
      rot, offset, boneRef.boneRotation, boneRef.boneOffset);    

    cachedTransformsDirty = true;
  }

  csPtr<CS::Animation::iSkeleton> SkeletonFactory::CreateSkeleton ()
  {
    csRef<CS::Animation::iSkeleton> ref;
    ref.AttachNew (new Skeleton (this));
    return csPtr<CS::Animation::iSkeleton> (ref);
  }

  CS::Animation::iSkeletonAnimPacketFactory* SkeletonFactory::GetAnimationPacket () const
  {
    return animationPacket;
  }

  void SkeletonFactory::SetAnimationPacket (CS::Animation::iSkeletonAnimPacketFactory* fact)
  {
    animationPacket = fact;
  }

  void SkeletonFactory::SetAutoStart (bool autostart)
  {
    this->autostart = autostart;
  }

  bool SkeletonFactory::GetAutoStart ()
  {
    return autostart;
  }

  void SkeletonFactory::UpdateCachedTransforms ()
  {
    if (!cachedTransformsDirty)
      return;

    UpdateOrderList ();

    // We have the bone list, so go in order and we can just update current
    for (size_t i = 0; i < boneOrderList.GetSize (); ++i)
    {
      Bone& boneRef = allBones[boneOrderList[i]];
      
      if (!boneRef.created)
        continue;

      if (boneRef.parent == CS::Animation::InvalidBoneID)
      {
        boneRef.absOffset = boneRef.boneOffset;
        boneRef.absRotation = boneRef.boneRotation;
      }
      else
      {
        Bone& boneParentRef = allBones[boneRef.parent];

        TransformQVFrame (boneParentRef.absRotation, boneParentRef.absOffset, 
          boneRef.boneRotation, boneRef.boneOffset, 
          boneRef.absRotation , boneRef.absOffset);
      }
    }

    cachedTransformsDirty = false;
  }

  void SkeletonFactory::UpdateOrderList ()
  {
    if (!orderListDirty)
      return;

    // Do a topological sort on the bones to get an order list
    csArray<CS::Utility::GraphEdge> graph;

    for (size_t i = 0; i < allBones.GetSize (); ++i)
    {
      if (allBones[i].created)
      {
	CS::Utility::GraphEdge edge (allBones[i].parent, i);
        graph.Push (edge);
      }      
    }

    boneOrderList = CS::Utility::TopologicalSort (graph);
    boneOrderList.Delete (CS::Animation::InvalidBoneID);

    orderListDirty = false;
  }




  CS_LEAKGUARD_IMPLEMENT(Skeleton);

  Skeleton::Skeleton (SkeletonFactory* factory)
    : scfImplementationType (this, factory), factory (factory), 
    cachedTransformsDirty (true), version (0), animesh (nullptr)
  {
    // Setup the bones from the parent setup
    RecreateSkeletonP ();
    RecreateAnimationTreeP ();
  }

  iSceneNode* Skeleton::GetSceneNode ()
  {
    if (!animesh)
      return 0;

    csRef<iMeshObject> object = scfQueryInterface<iMeshObject> (animesh);
    return object->GetMeshWrapper ()->QuerySceneNode ();
  }

  void Skeleton::SetAnimatedMesh (CS::Mesh::iAnimatedMesh* animesh)
  {
    this->animesh = animesh;
  }

  CS::Mesh::iAnimatedMesh* Skeleton::GetAnimatedMesh ()
  {
    return animesh;
  }

  void Skeleton::GetTransformBoneSpace (CS::Animation::BoneID bone, csQuaternion& rot, 
    csVector3& offset) const
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);

    const Bone& boneRef = allBones[bone];
    rot = boneRef.boneRotation;
    offset = boneRef.boneOffset;
  }

  void Skeleton::SetTransformBoneSpace (CS::Animation::BoneID bone, const csQuaternion& rot, 
    const csVector3& offset)
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);

    cachedTransformsDirty = true;

    Bone& boneRef = allBones[bone];
    boneRef.boneRotation = rot;
    boneRef.boneOffset = offset;
  }

  void Skeleton::GetTransformAbsSpace (CS::Animation::BoneID bone, csQuaternion& rot, 
    csVector3& offset) const
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);

    const Bone& boneRef = allBones[bone];

    if (boneRef.parent == CS::Animation::InvalidBoneID)
    {
      rot = boneRef.boneRotation;
      offset = boneRef.boneOffset;
      return;
    }

    if (cachedTransformsDirty)
    {
      csQuaternion parentRot;
      csVector3 parentOffset;
      GetTransformAbsSpace (boneRef.parent, parentRot, parentOffset);

      TransformQVFrame (parentRot, parentOffset, 
        boneRef.boneRotation, boneRef.boneOffset, rot, offset);
    }
    else
    {
      rot = boneRef.absRotation;
      offset = boneRef.absOffset;
    }
  }

  void Skeleton::SetTransformAbsSpace (CS::Animation::BoneID bone, const csQuaternion& rot, 
    const csVector3& offset)
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);    

    Bone& boneRef = allBones[bone];

    if (boneRef.parent == CS::Animation::InvalidBoneID)
    {
      boneRef.boneRotation = rot;
      boneRef.boneOffset = offset;
      // TODO: no need for cachedTransformsDirty = true; ?
      return;
    }

    csQuaternion parentRot;
    csVector3 parentOffset;
    GetTransformAbsSpace (boneRef.parent, parentRot, parentOffset);

    TransformQVFrameInv (parentRot, parentOffset,
      rot, offset, boneRef.boneRotation, boneRef.boneOffset);    

    cachedTransformsDirty = true;
  }

  void Skeleton::GetTransformBindSpace (CS::Animation::BoneID bone, csQuaternion& rot, 
    csVector3& offset) const
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);    

    if (cachedTransformsDirty || factory->cachedTransformsDirty)
    {
      // Get the abs transform for factory & bone
      csQuaternion factRot;
      csVector3 factOffset;
      factory->GetTransformAbsSpace (bone, factRot, factOffset);

      csQuaternion myRot;
      csVector3 myOffset;
      GetTransformAbsSpace (bone, myRot, myOffset);

      TransformQVFrameInv (factRot, factOffset, myRot, myOffset, 
        rot, offset);
    }
    else
    {
      const Bone& boneRef = allBones[bone];

      rot = boneRef.bindRotation;
      offset = boneRef.bindOffset;
    }
  }

  void Skeleton::SetTransformBindSpace (CS::Animation::BoneID bone, const csQuaternion& rot, 
    const csVector3& offset)
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);

    csQuaternion factRot;
    csVector3 factOffset;
    factory->GetTransformAbsSpace (bone, factRot, factOffset);

    csQuaternion newAbsRot;
    csVector3 newAbsOffset;
    TransformQVFrame (factRot, factOffset, rot, offset, newAbsRot, newAbsOffset);

    SetTransformAbsSpace (bone, newAbsRot, newAbsOffset);
  }

  csPtr<CS::Animation::csSkeletalState> Skeleton::GetStateAbsSpace ()
  {
    UpdateCachedTransforms ();

    // Use a pool for these...
    csRef<CS::Animation::csSkeletalState> currState;
    currState.AttachNew (new CS::Animation::csSkeletalState);
    currState->Setup (allBones.GetSize ());

    for (CS::Animation::BoneID i = 0; i < allBones.GetSize (); ++i)
    {
      if (allBones[i].created)
      {
        currState->GetQuaternion (i) = allBones[i].absRotation;
        currState->GetVector (i) = allBones[i].absOffset;
        currState->SetBoneUsed (i);
      }
    }

    return csPtr<CS::Animation::csSkeletalState> (currState);
  }

  csPtr<CS::Animation::csSkeletalState> Skeleton::GetStateBoneSpace ()
  {
    // Use a pool for these...
    csRef<CS::Animation::csSkeletalState> currState;
    currState.AttachNew (new CS::Animation::csSkeletalState);
    currState->Setup (allBones.GetSize ());

    for (size_t i = 0; i < allBones.GetSize (); ++i)
    {
      if (allBones[i].created)
      {
        currState->GetQuaternion (i) = allBones[i].boneRotation;
        currState->GetVector (i) = allBones[i].boneOffset;
        currState->SetBoneUsed ((CS::Animation::BoneID)i);
      }
    }

    return csPtr<CS::Animation::csSkeletalState> (currState);
  }

  csPtr<CS::Animation::csSkeletalState> Skeleton::GetStateBindSpace ()
  {
    UpdateCachedTransforms ();

    // Use a pool for these...
    csRef<CS::Animation::csSkeletalState> currState;
    currState.AttachNew (new CS::Animation::csSkeletalState);
    currState->Setup (allBones.GetSize ());

    for (size_t i = 0; i < allBones.GetSize (); ++i)
    {
      if (allBones[i].created)
      {        
        currState->GetQuaternion (i) = allBones[i].bindRotation;
        currState->GetVector (i) = allBones[i].bindOffset;
        currState->SetBoneUsed ((CS::Animation::BoneID)i);
      }
    }

    return csPtr<CS::Animation::csSkeletalState> (currState);
  }

  CS::Animation::iSkeletonFactory* Skeleton::GetFactory () const
  {
    return factory;
  }


  CS::Animation::iSkeletonAnimPacket* Skeleton::GetAnimationPacket () const
  {
    return animationPacket;
  }

  void Skeleton::SetAnimationPacket (CS::Animation::iSkeletonAnimPacket* packet)
  {
    animationPacket = packet;
  }


  void Skeleton::RecreateSkeleton ()
  {
    RecreateSkeletonP ();
  }

  void Skeleton::ResetSkeletonState ()
  {
    for (size_t i = 0; i < allBones.GetSize (); ++i)
    {
      Bone& boneRef = allBones[i];

      if (boneRef.created)
      {
	boneRef.boneOffset = factory->allBones[i].boneOffset;
	boneRef.boneRotation = factory->allBones[i].boneRotation;
      }
    }

    version++;

    cachedTransformsDirty = true;    
  }

  void Skeleton::UpdateSkeleton (float dt)
  {
    if (!animationPacket || !animationPacket->GetAnimationRoot ())
      return;

    CS::Animation::iSkeletonAnimNode* rootNode = animationPacket->GetAnimationRoot ();
   
    // If the root node is active then update the skeleton
    if (rootNode->IsActive ())
    {
      // Update the root node
      rootNode->TickAnimation (dt);

      // TODO: Use a pool for these...
      csRef<CS::Animation::csSkeletalState> finalState;
      finalState.AttachNew (new CS::Animation::csSkeletalState);
      finalState->Setup (allBones.GetSize ());

      // Blend the root node into the skeleton state
      rootNode->BlendState (finalState);

      // Apply the skeleton state
      for (size_t i = 0; i < allBones.GetSize (); ++i)
      {
        Bone& boneRef = allBones[i];

	// Apply the bone state
        if (boneRef.created && finalState->IsBoneUsed ((CS::Animation::BoneID) i))
        {
	  csQuaternion skeletonRotation;
	  csVector3 skeletonOffset;
	  factory->GetTransformBoneSpace ((CS::Animation::BoneID) i, skeletonRotation,
					 skeletonOffset);


          const csQuaternion& q = finalState->GetQuaternion (i);

	  // Normalize the quaternion if needed
          if (q.Norm () > 0)
            boneRef.boneRotation = q.Unit () * skeletonRotation;

          else
            boneRef.boneRotation = q * skeletonRotation;
           
          boneRef.boneOffset = finalState->GetVector (i) + skeletonOffset;

          cachedTransformsDirty = true;
        }
      }
      
      version++;      
    }
  }

  unsigned int Skeleton::GetSkeletonStateVersion () const
  {
    return version;
  }

  void Skeleton::RecreateSkeletonP ()
  {
    allBones.DeleteAll ();

    csArray<SkeletonFactory::Bone>& factoryBones = factory->allBones;
    allBones.SetSize (factoryBones.GetSize ());

    factory->UpdateCachedTransforms ();

    // Copy the data
    for (size_t i = 0; i < allBones.GetSize (); ++i)
    {
      allBones[i].parent = factoryBones[i].parent;
      allBones[i].created = factoryBones[i].created;
      allBones[i].boneOffset = factoryBones[i].boneOffset;
      allBones[i].boneRotation = factoryBones[i].boneRotation;
      allBones[i].absOffset = factoryBones[i].absOffset;
      allBones[i].absRotation = factoryBones[i].absRotation;

      // Skeleton is same as factory
      allBones[i].bindOffset.Set (0.0f);
      allBones[i].bindRotation.SetIdentity ();
    }

    cachedTransformsDirty = false;
  }

  void Skeleton::RecreateAnimationTreeP ()
  {
    if (!factory->animationPacket)
      return;

    animationPacket = factory->animationPacket->CreateInstance (this);
  }

  void Skeleton::UpdateCachedTransforms ()
  {
    if (!factory->cachedTransformsDirty && !cachedTransformsDirty)
      return;

    // Update the factory
    factory->UpdateCachedTransforms ();

    // Update our own transforms
    const csArray<size_t>& orderList = factory->GetOrderList ();
    for (size_t i = 0; i < orderList.GetSize (); ++i)
    {
      Bone& boneRef = allBones[orderList[i]];

      if (!boneRef.created)
        continue;

      if (boneRef.parent == CS::Animation::InvalidBoneID)
      {
        boneRef.absOffset = boneRef.boneOffset;
        boneRef.absRotation = boneRef.boneRotation;
      }
      else
      {
        Bone& boneParentRef = allBones[boneRef.parent];

        TransformQVFrame (boneParentRef.absRotation, boneParentRef.absOffset, 
          boneRef.boneRotation, boneRef.boneOffset, 
          boneRef.absRotation , boneRef.absOffset);
      }

      SkeletonFactory::Bone& factoryBone = factory->allBones[orderList[i]];
      TransformQVFrameInv (factoryBone.absRotation, factoryBone.absOffset,
        boneRef.absRotation , boneRef.absOffset,
        boneRef.bindRotation, boneRef.bindOffset);
    }

    cachedTransformsDirty = false;
  }

}
CS_PLUGIN_NAMESPACE_END(Skeleton2)
