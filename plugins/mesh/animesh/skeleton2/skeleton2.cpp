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

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{
  SCF_IMPLEMENT_FACTORY(SkeletonSystem)


  CS_LEAKGUARD_IMPLEMENT(SkeletonSystem);

  SkeletonSystem::SkeletonSystem (iBase* parent)
    : scfImplementationType (this, parent)
  {}

  iSkeletonFactory2* SkeletonSystem::CreateSkeletonFactory (const char* name)
  {
    // Check name uniqueness
    csRef<iSkeletonFactory2> newFact = csPtr<iSkeletonFactory2> (new SkeletonFactory);

    return factoryHash.PutUnique (name, newFact);
  }

  iSkeletonFactory2* SkeletonSystem::FindSkeletonFactory (const char* name)
  {
    return factoryHash.Get (name, 0);
  }

  void SkeletonSystem::ClearSkeletonFactories ()
  {
    factoryHash.DeleteAll ();
  }

  iSkeletonAnimPacketFactory2* SkeletonSystem::CreateAnimPacketFactory (const char* name)
  {
    // Check name uniqueness
    csRef<iSkeletonAnimPacketFactory2> newFact = 
      csPtr<iSkeletonAnimPacketFactory2> (new AnimationPacketFactory);

    return animPackets.PutUnique (name, newFact);
  }

  iSkeletonAnimPacketFactory2* SkeletonSystem::FindAnimPacketFactory (const char* name)
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
    : scfImplementationType (this), cachedTransformsDirty (true), 
    orderListDirty (true)
  {}

  BoneID SkeletonFactory::FindBone (const char *name) const
  {
    for (size_t i = 0; i < boneNames.GetSize (); i++)
    {
      if (!strcmp(name,boneNames[i]))
        return (BoneID)i;
    }
    return InvalidBoneID;
  }

  BoneID SkeletonFactory::CreateBone (BoneID parent)
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

        return (BoneID)i;
      }
    }

    Bone newBone;
    newBone.parent = parent;
    newBone.created = true;

    boneNames.Push ("NEWBONE");
    return (BoneID)allBones.Push (newBone);
  }

  void SkeletonFactory::RemoveBone (BoneID bone)
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

  BoneID SkeletonFactory::GetBoneParent (BoneID bone) const
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);

    return allBones[bone].parent;
  }

  bool SkeletonFactory::HasBone (BoneID bone) const
  {
    return (bone < allBones.GetSize ()) && 
           allBones[bone].created;
  }

  void SkeletonFactory::SetBoneName (BoneID bone, const char* name)
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);

    boneNames[bone] = name;
  }

  const char* SkeletonFactory::GetBoneName (BoneID bone) const
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);

    return boneNames[bone];
  }

  BoneID SkeletonFactory::GetTopBoneID () const
  {
    return (BoneID)(allBones.GetSize () - 1);
  }

  void SkeletonFactory::GetTransformBoneSpace (BoneID bone, csQuaternion& rot, 
    csVector3& offset) const
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);

    const Bone& boneRef = allBones[bone];
    rot = boneRef.boneRotation;
    offset = boneRef.boneOffset;
  }

  void SkeletonFactory::SetTransformBoneSpace (BoneID bone, const csQuaternion& rot, 
    const csVector3& offset)
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);

    cachedTransformsDirty = true;

    Bone& boneRef = allBones[bone];
    boneRef.boneRotation = rot;
    boneRef.boneOffset = offset;
  }

  void SkeletonFactory::GetTransformAbsSpace (BoneID bone, csQuaternion& rot, 
    csVector3& offset) const
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);
    
    const Bone& boneRef = allBones[bone];
    
    if (boneRef.parent == InvalidBoneID)
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

  void SkeletonFactory::SetTransformAbsSpace (BoneID bone, const csQuaternion& rot, 
    const csVector3& offset)
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);

    Bone& boneRef = allBones[bone];

    if (boneRef.parent == InvalidBoneID)
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

  csPtr<iSkeleton2> SkeletonFactory::CreateSkeleton ()
  {
    csRef<iSkeleton2> ref;
    ref.AttachNew (new Skeleton (this));
    return csPtr<iSkeleton2> (ref);
  }

  iSkeletonAnimPacketFactory2* SkeletonFactory::GetAnimationPacket () const
  {
    return animationPacket;
  }

  void SkeletonFactory::SetAnimationPacket (iSkeletonAnimPacketFactory2* fact)
  {
    animationPacket = fact;
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

      if (boneRef.parent == InvalidBoneID)
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
      if (allBones[i].created && 
          allBones[i].parent != InvalidBoneID)
      {
        CS::Utility::GraphEdge edge (allBones[i].parent, i);
        graph.Push (edge);
      }      
    }

    boneOrderList = CS::Utility::TopologicalSort (graph);

    orderListDirty = false;
  }




  CS_LEAKGUARD_IMPLEMENT(Skeleton);

  Skeleton::Skeleton (SkeletonFactory* factory)
    : scfImplementationType (this, factory), factory (factory), 
    cachedTransformsDirty (true), version (0), versionLastReset (0)
  {
    // Setup the bones from the parent setup
    RecreateSkeletonP ();
    RecreateAnimationTreeP ();
  }

  iSceneNode* Skeleton::GetSceneNode ()
  {
    return 0;
  }

  void Skeleton::GetTransformBoneSpace (BoneID bone, csQuaternion& rot, 
    csVector3& offset) const
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);

    const Bone& boneRef = allBones[bone];
    rot = boneRef.boneRotation;
    offset = boneRef.boneOffset;
  }

  void Skeleton::SetTransformBoneSpace (BoneID bone, const csQuaternion& rot, 
    const csVector3& offset)
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);

    cachedTransformsDirty = true;

    Bone& boneRef = allBones[bone];
    boneRef.boneRotation = rot;
    boneRef.boneOffset = offset;
  }

  void Skeleton::GetTransformAbsSpace (BoneID bone, csQuaternion& rot, 
    csVector3& offset) const
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);

    const Bone& boneRef = allBones[bone];

    if (boneRef.parent == InvalidBoneID)
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

  void Skeleton::SetTransformAbsSpace (BoneID bone, const csQuaternion& rot, 
    const csVector3& offset)
  {
    CS_ASSERT(bone < allBones.GetSize () && allBones[bone].created);    

    Bone& boneRef = allBones[bone];

    if (boneRef.parent == InvalidBoneID)
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

  void Skeleton::GetTransformBindSpace (BoneID bone, csQuaternion& rot, 
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

  void Skeleton::SetTransformBindSpace (BoneID bone, const csQuaternion& rot, 
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

  csPtr<csSkeletalState2> Skeleton::GetStateAbsSpace ()
  {
    UpdateCachedTransforms ();

    // Use a pool for these...
    csRef<csSkeletalState2> currState;
    currState.AttachNew (new csSkeletalState2);
    currState->Setup (allBones.GetSize ());

    for (BoneID i = 0; i < allBones.GetSize (); ++i)
    {
      if (allBones[i].created)
      {
        currState->GetQuaternion (i) = allBones[i].absRotation;
        currState->GetVector (i) = allBones[i].absOffset;
        currState->SetBoneUsed (i);
      }
    }

    return csPtr<csSkeletalState2> (currState);
  }

  csPtr<csSkeletalState2> Skeleton::GetStateBoneSpace ()
  {
    // Use a pool for these...
    csRef<csSkeletalState2> currState;
    currState.AttachNew (new csSkeletalState2);
    currState->Setup (allBones.GetSize ());

    for (size_t i = 0; i < allBones.GetSize (); ++i)
    {
      if (allBones[i].created)
      {
        currState->GetQuaternion (i) = allBones[i].boneRotation;
        currState->GetVector (i) = allBones[i].boneOffset;
        currState->SetBoneUsed ((BoneID)i);
      }
    }

    return csPtr<csSkeletalState2> (currState);
  }

  csPtr<csSkeletalState2> Skeleton::GetStateBindSpace ()
  {
    UpdateCachedTransforms ();

    // Use a pool for these...
    csRef<csSkeletalState2> currState;
    currState.AttachNew (new csSkeletalState2);
    currState->Setup (allBones.GetSize ());

    for (size_t i = 0; i < allBones.GetSize (); ++i)
    {
      if (allBones[i].created)
      {        
        currState->GetQuaternion (i) = allBones[i].bindRotation;
        currState->GetVector (i) = allBones[i].bindOffset;
        currState->SetBoneUsed ((BoneID)i);
      }
    }

    return csPtr<csSkeletalState2> (currState);
  }

  iSkeletonFactory2* Skeleton::GetFactory () const
  {
    return factory;
  }


  iSkeletonAnimPacket2* Skeleton::GetAnimationPacket () const
  {
    return animationPacket;
  }

  void Skeleton::SetAnimationPacket (iSkeletonAnimPacket2* packet)
  {
    animationPacket = packet;
  }


  void Skeleton::RecreateSkeleton ()
  {
    RecreateSkeletonP ();
  }

  void Skeleton::UpdateSkeleton (float dt)
  {
    if (!animationPacket || !animationPacket->GetAnimationRoot ())
      return;

    iSkeletonAnimNode2* rootNode = animationPacket->GetAnimationRoot ();
    // 
    rootNode->TickAnimation (dt);
   
    //
    if (rootNode->IsActive ())
    {
      // Use a pool for these...
      csRef<csSkeletalState2> finalState;
      finalState.AttachNew (new csSkeletalState2);
      finalState->Setup (allBones.GetSize ());

      rootNode->BlendState (finalState);

      // Apply the bone state
      for (size_t i = 0; i < allBones.GetSize (); ++i)
      {
        Bone& boneRef = allBones[i];

        if (boneRef.created && finalState->IsBoneUsed ((BoneID)i))
        {
          const csQuaternion& q = finalState->GetQuaternion (i);
          if (q.Norm () > 0)
          {
            boneRef.boneRotation = q.Unit ();
          }          
          else
          {
            boneRef.boneRotation = q;
          }
           
          boneRef.boneOffset = finalState->GetVector (i);

          cachedTransformsDirty = true;
        }
      }
      
      version++;      
    }    
    else if (versionLastReset != version)
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
      versionLastReset = version;

      cachedTransformsDirty = true;
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

      if (boneRef.parent == InvalidBoneID)
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
