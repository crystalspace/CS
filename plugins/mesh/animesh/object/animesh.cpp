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

#include "csgfx/renderbuffer.h"
#include "csgfx/vertexlistwalker.h"
#include "cstool/rviewclipper.h"
#include "csutil/scf.h"
#include "csutil/sysfunc.h"
#include "iengine/camera.h"
#include "iengine/material.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "ivideo/rendermesh.h"
#include "iutil/strset.h"
#include "csutil/objreg.h"

#include "animesh.h"

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(Animesh)
{

  static csStringID svNameVertexSkinned = csInvalidStringID;
  static csStringID svNameNormalSkinned = csInvalidStringID;
  static csStringID svNameTangentSkinned = csInvalidStringID;
  static csStringID svNameBinormalSkinned = csInvalidStringID;
  static csStringID svNameBoneIndex = csInvalidStringID;
  static csStringID svNameBoneWeight = csInvalidStringID;
  static csStringID svNameBoneTransforms = csInvalidStringID;


  SCF_IMPLEMENT_FACTORY(AnimeshObjectType);

  AnimeshObjectType::AnimeshObjectType (iBase* parent)
    : scfImplementationType (this, parent)
  {
  }

  csPtr<iMeshObjectFactory> AnimeshObjectType::NewFactory ()
  {
    return new AnimeshObjectFactory (this);
  }

  bool AnimeshObjectType::Initialize (iObjectRegistry* object_reg)
  {
    csRef<iStringSet> strset = csQueryRegistryTagInterface<iStringSet> (
      object_reg, "crystalspace.shared.stringset");

    // Get the SV names
    svNameVertexSkinned = strset->Request ("position skinned");
    svNameNormalSkinned = strset->Request ("normal skinned");
    svNameTangentSkinned = strset->Request ("tangent skinned");
    svNameBinormalSkinned = strset->Request ("binormal skinned");
    svNameBoneIndex = strset->Request ("bone index");
    svNameBoneWeight = strset->Request ("bone weight");
    svNameBoneTransforms = strset->Request ("bone transform");


    return true;
  }




  AnimeshObjectFactory::AnimeshObjectFactory (AnimeshObjectType* objType)
    : scfImplementationType (this), objectType (objType), logParent (0), material (0),
    vertexCount (0), bonesPerBatch (~0)
  {
  }

  iAnimatedMeshFactorySubMesh* AnimeshObjectFactory::CreateSubMesh (iRenderBuffer* indices)
  {
    csRef<FactorySubmesh> newSubmesh;

    newSubmesh.AttachNew (new FactorySubmesh);
    newSubmesh->indexBuffers.Push (indices);    
    submeshes.Push (newSubmesh);

    return newSubmesh;
  }

  iAnimatedMeshFactorySubMesh* AnimeshObjectFactory::CreateSubMesh (
    const csArray<iRenderBuffer*>& indices, 
    const csArray<csArray<unsigned int> >& boneIndices)
  {
    csRef<FactorySubmesh> newSubmesh;

    newSubmesh.AttachNew (new FactorySubmesh);
    
    for (size_t i = 0; i < indices.GetSize (); ++i)
    {
      newSubmesh->indexBuffers.Push (indices[i]);
    }    
    
    // Setup the bone mappings
    for (size_t i = 0; i < boneIndices.GetSize (); ++i)
    {
      FactorySubmesh::RemappedBones rb;
      rb.boneRemappingTable = boneIndices[i];
      newSubmesh->boneMapping.Push (rb);
    }
    
    submeshes.Push (newSubmesh);

    return newSubmesh;
  }

  iAnimatedMeshFactorySubMesh* AnimeshObjectFactory::GetSubMesh (size_t index) const
  {
    CS_ASSERT (index < submeshes.GetSize ());
    return submeshes[index];
  }

  size_t AnimeshObjectFactory::GetSubMeshCount () const
  {
    return submeshes.GetSize ();
  }

  void AnimeshObjectFactory::DeleteSubMesh (iAnimatedMeshFactorySubMesh* mesh)
  {
    submeshes.Delete (static_cast<FactorySubmesh*> (mesh));
    Invalidate ();
  }

  void AnimeshObjectFactory::SetVertexCount (uint count)
  {
    if (count != vertexCount)
    {
      vertexCount = count;

      // Recreate the buffers
      vertexBuffer = csRenderBuffer::CreateRenderBuffer (vertexCount, CS_BUF_STATIC, 
        CS_BUFCOMP_FLOAT, 3);

      texcoordBuffer = csRenderBuffer::CreateRenderBuffer (vertexCount, CS_BUF_STATIC, 
        CS_BUFCOMP_FLOAT, 2);

      normalBuffer = csRenderBuffer::CreateRenderBuffer (vertexCount, CS_BUF_STATIC, 
        CS_BUFCOMP_FLOAT, 3);

      tangentBuffer = csRenderBuffer::CreateRenderBuffer (vertexCount, CS_BUF_STATIC, 
        CS_BUFCOMP_FLOAT, 3);

      binormalBuffer = csRenderBuffer::CreateRenderBuffer (vertexCount, CS_BUF_STATIC, 
        CS_BUFCOMP_FLOAT, 3);

      colorBuffer = csRenderBuffer::CreateRenderBuffer (vertexCount, CS_BUF_STATIC, 
        CS_BUFCOMP_FLOAT, 4);  

      // Reallocate the bone influence table
      boneInfluences.DeleteAll ();
      boneInfluences.SetSize (count*4);

      static csInterleavedSubBufferOptions bufSettings[] = 
      {
        {CS_BUFCOMP_UNSIGNED_INT, 0},
        {CS_BUFCOMP_FLOAT, 0}
      };

      bufSettings[0].componentCount = bufSettings[1].componentCount = 4;//@@TODO

      masterBWBuffer = csRenderBuffer::CreateInterleavedRenderBuffers (
        vertexCount, CS_BUF_STATIC, 2, bufSettings, boneWeightAndIndexBuffer);
    }
  }

  uint AnimeshObjectFactory::GetVertexCount () const
  {
    return vertexCount;
  }

  iRenderBuffer* AnimeshObjectFactory::GetVertices ()
  {
    return vertexBuffer;
  }

  iRenderBuffer* AnimeshObjectFactory::GetTexCoords ()
  {
    return texcoordBuffer;
  }

  iRenderBuffer* AnimeshObjectFactory::GetNormals ()
  {
    return normalBuffer;
  }

  iRenderBuffer* AnimeshObjectFactory::GetTangents ()
  {
    return tangentBuffer;
  }

  iRenderBuffer* AnimeshObjectFactory::GetBinormals ()
  {
    return binormalBuffer;
  }

  iRenderBuffer* AnimeshObjectFactory::GetColors ()
  {
    return colorBuffer;
  }

  void AnimeshObjectFactory::Invalidate ()
  {
    // Traverse the submeshes, in cases where there is a remapping, create
    // remapped bone influence tables

    csHash<unsigned int, unsigned int> mappingHash; //Map from real bone index to virtual bone index

    for (size_t si = 0; si < submeshes.GetSize (); ++si)
    {
      FactorySubmesh* sm = submeshes[si];
      for (size_t bmi = 0; bmi < sm->boneMapping.GetSize (); ++bmi)
      {
        FactorySubmesh::RemappedBones& bm = sm->boneMapping[bmi];
        mappingHash.DeleteAll ();
      
        // Need remapping, setup the hash
        for (size_t i = 0; i < bm.boneRemappingTable.GetSize (); ++i)
        {
          mappingHash.PutUnique (bm.boneRemappingTable[i], i);
        }

        // Create the weight & influence renderbuffers
        static csInterleavedSubBufferOptions bufSettings[] = 
        {
          {CS_BUFCOMP_UNSIGNED_INT, 0},
          {CS_BUFCOMP_FLOAT, 0}
        };

        bufSettings[0].componentCount = bufSettings[1].componentCount = 4;//@@TODO

        bm.masterBWBuffer = csRenderBuffer::CreateInterleavedRenderBuffers (
          vertexCount, CS_BUF_STATIC, 2, bufSettings, bm.boneWeightAndIndexBuffer);

        // Copy the data and remap the bones
        csRenderBufferLock<unsigned int> biLock (bm.boneWeightAndIndexBuffer[0]);
        csRenderBufferLock<float> bwLock (bm.boneWeightAndIndexBuffer[1]);

        for (size_t i = 0; i < boneInfluences.GetSize (); ++i)
        {
          *biLock++ = mappingHash.Get (boneInfluences[i].bone, 0);
          *bwLock++ = boneInfluences[i].influenceWeight;
        }
        
      }

      // Setup buffer holders
      sm->bufferHolders.DeleteAll ();
      for (size_t i = 0; i < sm->indexBuffers.GetSize (); ++i)
      {      
        csRef<csRenderBufferHolder> bufferholder;
        bufferholder.AttachNew (new csRenderBufferHolder);
        bufferholder->SetRenderBuffer (CS_BUFFER_INDEX, sm->indexBuffers[i]);
        bufferholder->SetRenderBuffer (CS_BUFFER_POSITION, vertexBuffer);
        bufferholder->SetRenderBuffer (CS_BUFFER_TEXCOORD0, texcoordBuffer);
        bufferholder->SetRenderBuffer (CS_BUFFER_NORMAL, normalBuffer);
        bufferholder->SetRenderBuffer (CS_BUFFER_TANGENT, tangentBuffer);
        bufferholder->SetRenderBuffer (CS_BUFFER_BINORMAL, binormalBuffer);
        bufferholder->SetRenderBuffer (CS_BUFFER_COLOR_UNLIT, colorBuffer);
        sm->bufferHolders.Push (bufferholder);
      }
    }


    // Setup the bone weight & index buffers for cases not covered above
    masterBWBuffer->CopyInto (boneInfluences.GetArray (), vertexCount);
    
    // Fix the bb
    factoryBB.StartBoundingBox ();
    csRenderBufferLock<csVector3> vbuf (vertexBuffer);
    for (size_t i = 0; i < vertexCount; ++i)
    {
      factoryBB.AddBoundingVertex (*vbuf++);
    }
  }

  void AnimeshObjectFactory::SetBoneInfluencesPerVertex (uint num)
  {

  }

  uint AnimeshObjectFactory::GetBoneInfluencesPerVertex () const
  {
    return 4;
  }

  csAnimatedMeshBoneInfluence* AnimeshObjectFactory::GetBoneInfluences ()
  {
    return boneInfluences.GetArray ();
  }

  void AnimeshObjectFactory::SetMaxBonesPerBatch (uint count)
  {
    bonesPerBatch = count;
  }

  uint AnimeshObjectFactory::GetMaxBonesPerBatch () const
  {
    return bonesPerBatch;
  }

  iAnimatedMeshMorphTarget* AnimeshObjectFactory::CreateMorphTarget ()
  {
    return 0;
  }

  iAnimatedMeshMorphTarget* AnimeshObjectFactory::GetMorphTarget (uint target)
  {
    return 0;
  }

  uint AnimeshObjectFactory::GetMorphTargetCount () const
  {
    return 0;
  }

  void AnimeshObjectFactory::ClearMorphTargets ()
  {    
  }

  csFlags& AnimeshObjectFactory::GetFlags ()
  {
    return factoryFlags;
  }

  csPtr<iMeshObject> AnimeshObjectFactory::NewInstance ()
  {
    return new AnimeshObject (this);
  }

  csPtr<iMeshObjectFactory> AnimeshObjectFactory::Clone ()
  {
    return 0;
  }

  void AnimeshObjectFactory::HardTransform (const csReversibleTransform& t)
  {    
  }

  bool AnimeshObjectFactory::SupportsHardTransform () const
  {
    return false;
  }

  void AnimeshObjectFactory::SetMeshFactoryWrapper (iMeshFactoryWrapper* lp)
  {
    logParent = lp;
  }

  iMeshFactoryWrapper* AnimeshObjectFactory::GetMeshFactoryWrapper () const
  {
    return logParent;
  }

  iMeshObjectType* AnimeshObjectFactory::GetMeshObjectType () const
  {
    return objectType;
  }

  iObjectModel* AnimeshObjectFactory::GetObjectModel ()
  {
    return 0;
  }

  bool AnimeshObjectFactory::SetMaterialWrapper (iMaterialWrapper* material)
  {
    this->material = material;
    return true;
  }

  iMaterialWrapper* AnimeshObjectFactory::GetMaterialWrapper () const
  {
    return material;
  }

  void AnimeshObjectFactory::SetMixMode (uint mode)
  {
    mixMode = mode;
  }
  
  uint AnimeshObjectFactory::GetMixMode () const
  {
    return mixMode;
  }




  AnimeshObject::AnimeshObject (AnimeshObjectFactory* factory)
    : scfImplementationType (this), factory (factory), logParent (0),
    material (0), mixMode (0), skeleton (0)
  {
    SetupSubmeshes ();
  }

  void AnimeshObject::SetSkeleton (iSkeleton2* newskel)
  {
    skeleton = newskel;
  }

  iAnimatedMeshSubMesh* AnimeshObject::GetSubMesh (size_t index) const
  {
    return 0;
  }

  size_t AnimeshObject::GetSubMeshCount () const
  {
    return 0;
  }

  void AnimeshObject::SetMorphTargetWeight (uint target, float weight)
  {
  }

  float AnimeshObject::GetMorphTargetWeight (uint target) const
  {
    return 0;
  }

  iMeshObjectFactory* AnimeshObject::GetFactory () const
  {
    return factory;
  }

  csFlags& AnimeshObject::GetFlags ()
  {
    return meshObjectFlags;
  }

  csPtr<iMeshObject> AnimeshObject::Clone ()
  {
    return 0;
  }

  CS::Graphics::RenderMesh** AnimeshObject::GetRenderMeshes (int& num, iRenderView* rview, 
    iMovable* movable, uint32 frustum_mask)
  {
    renderMeshList.DeleteAll ();

    // Boiler-plate stuff...
    iCamera* camera = rview->GetCamera ();

    int clip_portal, clip_plane, clip_z_plane;
    CS::RenderViewClipper::CalculateClipSettings (rview->GetRenderContext (),
      frustum_mask, clip_portal, clip_plane, clip_z_plane);

    const csReversibleTransform o2wt = movable->GetFullTransform ();
    const csVector3& wo = o2wt.GetOrigin ();
    
    
    // Fetch the material
    iMaterialWrapper* mat = material;
    if (!mat)
      mat = factory->material;

    if (!mat)
    {
      csPrintf ("INTERNAL ERROR: mesh used without material!\n");
      return 0;
    }

    if (mat->IsVisitRequired ()) 
      mat->Visit ();

    uint frameNum = rview->GetCurrentFrameNumber ();

    // Iterate all submeshes...
    for (size_t i = 0; i < submeshes.GetSize (); ++i)
    {
      if (!submeshes[i]->isRendering)
        continue;
      
      Submesh* sm = submeshes[i];
      FactorySubmesh* fsm = factory->submeshes[i];

      for (size_t j = 0; j < fsm->indexBuffers.GetSize (); ++j)
      {
        bool rmCreated;
        CS::Graphics::RenderMesh*& meshPtr = rmHolder.GetUnusedMesh (rmCreated,
          frameNum);

        // Setup the RM
        meshPtr->clip_portal = clip_portal;
        meshPtr->clip_plane = clip_plane;
        meshPtr->clip_z_plane = clip_z_plane;
        meshPtr->do_mirror = camera->IsMirrored ();
        meshPtr->meshtype = CS_MESHTYPE_TRIANGLES;
        meshPtr->indexstart = 0;
        meshPtr->indexend = (unsigned int)fsm->indexBuffers[j]->GetElementCount ();
        meshPtr->material = mat;

        meshPtr->mixmode = mixMode;
        meshPtr->buffers = fsm->bufferHolders[j];

        meshPtr->object2world = o2wt;
        meshPtr->geometryInstance = factory;
        meshPtr->variablecontext = sm->svContexts[j];

        renderMeshList.Push (meshPtr);
      }
    }


    num = (int)renderMeshList.GetSize ();
    return renderMeshList.GetArray ();
  }

  void AnimeshObject::SetVisibleCallback (iMeshObjectDrawCallback* cb)
  {
  }

  iMeshObjectDrawCallback* AnimeshObject::GetVisibleCallback () const
  {
    return 0;
  }

  void AnimeshObject::NextFrame (csTicks current_time,const csVector3& pos,
    uint currentFrame)
  {
    if (skeleton)
    {
      iSkeletonAnimationNode2* root = skeleton->GetAnimationRoot ();
      if (root)
      {
        root->TickAnimation ((current_time - lastTick) / 1000.0f);
      }
    }
    lastTick = current_time;
  }

  void AnimeshObject::HardTransform (const csReversibleTransform& t)
  {
  }

  bool AnimeshObject::SupportsHardTransform () const
  {
    return false;
  }

  bool AnimeshObject::HitBeamOutline (const csVector3& start,
    const csVector3& end, csVector3& isect, float* pr)
  {
    return false;
  }

  bool AnimeshObject::HitBeamObject (const csVector3& start, const csVector3& end,
    csVector3& isect, float* pr, int* polygon_idx,
    iMaterialWrapper** material)
  {
    return false;
  }

  void AnimeshObject::SetMeshWrapper (iMeshWrapper* lp)
  {
    logParent = lp;
  }

  iMeshWrapper* AnimeshObject::GetMeshWrapper () const
  {
    return logParent;
  }

  iObjectModel* AnimeshObject::GetObjectModel ()
  {
    return this;
  }

  bool AnimeshObject::SetColor (const csColor& color)
  {
    return false;
  }

  bool AnimeshObject::GetColor (csColor& color) const
  {
    return false;
  }

  bool AnimeshObject::SetMaterialWrapper (iMaterialWrapper* mat)
  {
    material = mat;
    return true;
  }

  iMaterialWrapper* AnimeshObject::GetMaterialWrapper () const
  {
    return material;
  }

  void AnimeshObject::SetMixMode (uint mode)
  {
    mixMode = mode;
  }

  uint AnimeshObject::GetMixMode () const
  {
    return mixMode;
  }

  void AnimeshObject::InvalidateMaterialHandles ()
  {
  }

  void AnimeshObject::PositionChild (iMeshObject* child,csTicks current_time)
  {
  }

  void AnimeshObject::BuildDecal(const csVector3* pos, float decalRadius,
    iDecalBuilder* decalBuilder)
  {
  }

  const csBox3& AnimeshObject::GetObjectBoundingBox ()
  {
    return factory->factoryBB; //@@TODO: later
  }
  
  void AnimeshObject::SetObjectBoundingBox (const csBox3& bbox)
  {
    //??
  }

  void AnimeshObject::GetRadius (float& radius, csVector3& center)
  {
    center = factory->factoryBB.GetCenter ();
    radius = factory->factoryBB.GetSize ().Norm ();
  }

  void AnimeshObject::SetupSubmeshes ()
  {
    submeshes.DeleteAll ();

    for (size_t i = 0; i < factory->submeshes.GetSize (); ++i)
    {
      FactorySubmesh* fsm = factory->submeshes[i];

      csRef<Submesh> sm; 
      sm.AttachNew (new Submesh (this, fsm));
      submeshes.Push (sm);

      bool subsm = fsm->boneMapping.GetSize () > 0;

      for (size_t j = 0; j < fsm->indexBuffers.GetSize (); ++j)
      {
        // SV context
        csRef<csShaderVariableContext> svContext;
        svContext.AttachNew (new csShaderVariableContext);
        csShaderVariable* sv;
        sv = svContext->GetVariableAdd (svNameVertexSkinned);
        sv->SetAccessor (sm, j);

        sv = svContext->GetVariableAdd (svNameNormalSkinned);
        sv->SetAccessor (sm, j);

        sv = svContext->GetVariableAdd (svNameTangentSkinned);
        sv->SetAccessor (sm, j);

        sv = svContext->GetVariableAdd (svNameBinormalSkinned);
        sv->SetAccessor (sm, j);

        
        sv = svContext->GetVariableAdd (svNameBoneIndex);
        if (subsm)
          sv->SetValue (fsm->boneMapping[j].boneWeightAndIndexBuffer[0]);
        else        
          sv->SetValue (factory->boneWeightAndIndexBuffer[0]);        

        sv = svContext->GetVariableAdd (svNameBoneWeight);
        if (subsm)
          sv->SetValue (fsm->boneMapping[j].boneWeightAndIndexBuffer[1]);
        else        
          sv->SetValue (factory->boneWeightAndIndexBuffer[1]);

        sv = svContext->GetVariableAdd (svNameBoneTransforms);
        sv->SetAccessor (sm, j);

        sm->svContexts.Push (svContext);
      }

    }
  }

  void AnimeshObject::Submesh::PreGetValue (csShaderVariable *variable)
  {

  }

}
CS_PLUGIN_NAMESPACE_END(Animesh)

