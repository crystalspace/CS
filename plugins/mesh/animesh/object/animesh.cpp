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

#include "csgeom/math3d.h"
#include "csgfx/renderbuffer.h"
#include "csgfx/vertexlistwalker.h"
#include "csgfx/trianglestream.h"
#include "cstool/rbuflock.h"
#include "cstool/rviewclipper.h"
#include "csutil/objreg.h"
#include "csutil/scf.h"
#include "csutil/scfarray.h"
#include "csutil/sysfunc.h"
#include "iengine/camera.h"
#include "iengine/material.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "imesh/skeleton2.h"
#include "imesh/skeleton2anim.h"
#include "iutil/strset.h"
#include "ivideo/rendermesh.h"

#include "animesh.h"



CS_PLUGIN_NAMESPACE_BEGIN(Animesh)
{

  static CS::ShaderVarStringID svNameVertexUnskinned = CS::InvalidShaderVarStringID;
  static CS::ShaderVarStringID svNameNormalUnskinned = CS::InvalidShaderVarStringID;
  static CS::ShaderVarStringID svNameTangentUnskinned = CS::InvalidShaderVarStringID;
  static CS::ShaderVarStringID svNameBinormalUnskinned = CS::InvalidShaderVarStringID;

  static CS::ShaderVarStringID svNameBoneIndex = CS::InvalidShaderVarStringID;
  static CS::ShaderVarStringID svNameBoneWeight = CS::InvalidShaderVarStringID;
  static CS::ShaderVarStringID svNameBoneTransforms = CS::InvalidShaderVarStringID;

  static CS::ShaderVarStringID svNameBoneTransformsReal = CS::InvalidShaderVarStringID;
  static CS::ShaderVarStringID svNameBoneTransformsDual = CS::InvalidShaderVarStringID;


  SCF_IMPLEMENT_FACTORY(AnimeshObjectType);

  AnimeshObjectType::AnimeshObjectType (iBase* parent)
    : scfImplementationType (this, parent)
  {
  }

  csPtr<iMeshObjectFactory> AnimeshObjectType::NewFactory ()
  {
    csRef<iMeshObjectFactory> ref;
    ref.AttachNew (new AnimeshObjectFactory (this));
    return csPtr<iMeshObjectFactory> (ref);
  }

  bool AnimeshObjectType::Initialize (iObjectRegistry* object_reg)
  {
    csRef<iShaderVarStringSet> strset =
      csQueryRegistryTagInterface<iShaderVarStringSet> (
        object_reg, "crystalspace.shader.variablenameset");

    // Get the SV names
    svNameVertexUnskinned = strset->Request ("position unskinned");
    svNameNormalUnskinned = strset->Request ("normal unskinned");
    svNameTangentUnskinned = strset->Request ("tangent unskinned");
    svNameBinormalUnskinned = strset->Request ("binormal unskinned");

    svNameBoneIndex = strset->Request ("bone index");
    svNameBoneWeight = strset->Request ("bone weight");
    svNameBoneTransforms = strset->Request ("bone transform");

    svNameBoneTransforms = strset->Request ("bone transform real");
    svNameBoneTransforms = strset->Request ("bone transform dual");


    return true;
  }




  AnimeshObjectFactory::AnimeshObjectFactory (AnimeshObjectType* objType)
    : scfImplementationType (this), objectType (objType), logParent (0), material (0),
    vertexCount (0)
  {
  }

  CS::Mesh::iAnimatedMeshSubMeshFactory* AnimeshObjectFactory::CreateSubMesh (iRenderBuffer* indices,
    const char* name, bool visible)
  {
    csRef<FactorySubmesh> newSubmesh;

    newSubmesh.AttachNew (new FactorySubmesh(name));
    newSubmesh->indexBuffers.Push (indices);  
    newSubmesh->visible = visible;
    submeshes.Push (newSubmesh);

    return newSubmesh;
  }

  CS::Mesh::iAnimatedMeshSubMeshFactory* AnimeshObjectFactory::CreateSubMesh (
    const csArray<iRenderBuffer*>& indices, 
    const csArray<csArray<unsigned int> >& boneIndices,
    const char* name,
    bool visible)
  {
    csRef<FactorySubmesh> newSubmesh;

    newSubmesh.AttachNew (new FactorySubmesh(name));
    newSubmesh->visible = visible;
    
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

  CS::Mesh::iAnimatedMeshSubMeshFactory* AnimeshObjectFactory::GetSubMesh (size_t index) const
  {
    CS_ASSERT (index < submeshes.GetSize ());
    return submeshes[index];
  }

  size_t AnimeshObjectFactory::FindSubMesh (const char* name) const
  {
    for (size_t i=0; i < submeshes.GetSize (); ++i)
    {
      const char* meshName = submeshes[i]->GetName();
      if (meshName)
      {
        if (!strcmp(meshName, name))
        {
          return i;
        }
      }
    }

    return (size_t)-1;
  }

  size_t AnimeshObjectFactory::GetSubMeshCount () const
  {
    return submeshes.GetSize ();
  }

  void AnimeshObjectFactory::DeleteSubMesh (CS::Mesh::iAnimatedMeshSubMeshFactory* mesh)
  {
    submeshes.Delete (static_cast<FactorySubmesh*> (mesh));
    Invalidate ();
  }

  uint AnimeshObjectFactory::GetVertexCount () const
  {
    return vertexCount;
  }

  iRenderBuffer* AnimeshObjectFactory::GetVertices ()
  {
    return vertexBuffer;
  }

  bool AnimeshObjectFactory::SetVertices (iRenderBuffer *renderBuffer)
  {
    if (renderBuffer->GetComponentCount () < 3)
      return false;

    vertexBuffer = renderBuffer;
    vertexCount = (uint)vertexBuffer->GetElementCount ();

    //Update the number of bone influences
    boneInfluences.SetSize (vertexCount*4);//@@TODO handle

    return true;
  }

  iRenderBuffer* AnimeshObjectFactory::GetTexCoords ()
  {
    return texcoordBuffer;
  }

  bool AnimeshObjectFactory::SetTexCoords (iRenderBuffer *renderBuffer)
  {
    if (renderBuffer->GetElementCount () < vertexCount)
      return false;

    texcoordBuffer = renderBuffer;    
    return true;
  }

  iRenderBuffer* AnimeshObjectFactory::GetNormals ()
  {
    return normalBuffer;
  }

  bool AnimeshObjectFactory::SetNormals (iRenderBuffer *renderBuffer)
  {
    if (renderBuffer->GetElementCount () < vertexCount)
      return false;

    normalBuffer = renderBuffer;    
    return true;
  }

  iRenderBuffer* AnimeshObjectFactory::GetTangents ()
  {
    return tangentBuffer;
  }

  bool AnimeshObjectFactory::SetTangents (iRenderBuffer *renderBuffer)
  {
    if (renderBuffer->GetElementCount () < vertexCount)
      return false;

    tangentBuffer = renderBuffer;    
    return true;
  }

  iRenderBuffer* AnimeshObjectFactory::GetBinormals ()
  {
    return binormalBuffer;
  }

  bool AnimeshObjectFactory::SetBinormals (iRenderBuffer *renderBuffer)
  {
    if (renderBuffer->GetElementCount () < vertexCount)
      return false;

    binormalBuffer = renderBuffer;    
    return true;
  }

  iRenderBuffer* AnimeshObjectFactory::GetColors ()
  {
    return colorBuffer;
  }

  bool AnimeshObjectFactory::SetColors (iRenderBuffer *renderBuffer)
  {
    if (renderBuffer->GetElementCount () < vertexCount)
      return false;

    colorBuffer = renderBuffer;    
    return true;
  }

  void AnimeshObjectFactory::Invalidate ()
  {
    // Create the weight & influence renderbuffers
    static csInterleavedSubBufferOptions bufSettings[] = 
    {
      {CS_BUFCOMP_UNSIGNED_INT, 0},
      {CS_BUFCOMP_FLOAT, 0}
    };

    bufSettings[0].componentCount = bufSettings[1].componentCount = 4;//@@TODO

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
          mappingHash.PutUnique (bm.boneRemappingTable[i], (uint)i);
        }
       
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
        bufferholder->SetRenderBuffer (CS_BUFFER_TEXCOORD0, texcoordBuffer);
        bufferholder->SetRenderBuffer (CS_BUFFER_COLOR_UNLIT, colorBuffer);
        bufferholder->SetRenderBuffer (CS_BUFFER_COLOR, colorBuffer);
        sm->bufferHolders.Push (bufferholder);
      }
    }


    // Setup the bone weight & index buffers for cases not covered above
    masterBWBuffer = csRenderBuffer::CreateInterleavedRenderBuffers (
      vertexCount, CS_BUF_STATIC, 2, bufSettings, boneWeightAndIndexBuffer);
    masterBWBuffer->CopyInto (boneInfluences.GetArray (), 
      csMin((size_t)vertexCount, (size_t)boneInfluences.GetSize ()/4));
    
    // Fix the bb
    factoryBB.StartBoundingBox ();
    csVertexListWalker<float, csVector3> vbuf (vertexBuffer);
    for (size_t i = 0; i < vertexCount; ++i)
    {
      factoryBB.AddBoundingVertex (*vbuf);
      ++vbuf;
    }

    // Normalize the bone weights
    for (size_t i = 0; i < vertexCount; ++i)
    {
      float sumWeight = 0;
      for (size_t j = 0; j < 4; ++j)
      {
        sumWeight += boneInfluences[i*4+j].influenceWeight;
      }

      for (size_t j = 0; j < 4; ++j)
      {
        boneInfluences[i*4+j].influenceWeight /= sumWeight;
      }
    }
  }

  void AnimeshObjectFactory::SetSkeletonFactory (CS::Animation::iSkeletonFactory2* skeletonFactory)
  {
    this->skeletonFactory = skeletonFactory;
  }

  CS::Animation::iSkeletonFactory2* AnimeshObjectFactory::GetSkeletonFactory () const
  {
    return skeletonFactory;
  }

  void AnimeshObjectFactory::SetBoneInfluencesPerVertex (uint num)
  {
    // TODO
  }

  uint AnimeshObjectFactory::GetBoneInfluencesPerVertex () const
  {
    return 4;
  }

  CS::Mesh::csAnimatedMeshBoneInfluence* AnimeshObjectFactory::GetBoneInfluences ()
  {
    return boneInfluences.GetArray ();
  }

  CS::Mesh::iAnimatedMeshMorphTarget* AnimeshObjectFactory::CreateMorphTarget (
    const char* name)
  {
    csRef<MorphTarget> newTarget;
    newTarget.AttachNew (new MorphTarget (this, name));
    size_t targetNum = morphTargets.Push (newTarget);
    morphTargetNames.Put (name, (uint)targetNum);
    return newTarget;
  }

  CS::Mesh::iAnimatedMeshMorphTarget* AnimeshObjectFactory::GetMorphTarget (uint target)
  {
    CS_ASSERT (target < morphTargets.GetSize ());
    return morphTargets[target];
  }

  uint AnimeshObjectFactory::GetMorphTargetCount () const
  {
    return (uint)morphTargets.GetSize();
  }

  void AnimeshObjectFactory::ClearMorphTargets ()
  {
    morphTargets.DeleteAll ();
    morphTargetNames.DeleteAll ();
  }

  uint AnimeshObjectFactory::FindMorphTarget (const char* name) const
  {
    return morphTargetNames.Get (name, (uint)~0);
  }

  void AnimeshObjectFactory::CreateSocket (CS::Animation::BoneID bone, 
    const csReversibleTransform& transform, const char* name)
  {
    csRef<FactorySocket> socket;
    socket.AttachNew (new FactorySocket (this, bone, name, transform));

    sockets.Push (socket);
  }

  size_t AnimeshObjectFactory::GetSocketCount () const
  {
    return sockets.GetSize ();
  }

  CS::Mesh::iAnimatedMeshSocketFactory* AnimeshObjectFactory::GetSocket (size_t index) const
  {
    CS_ASSERT (index < sockets.GetSize ());
    return sockets[index];
  }

  uint AnimeshObjectFactory::FindSocket (const char* name) const
  {
    for(size_t i=0; i<sockets.GetSize(); ++i)
    {
      if(!strcmp(name, sockets[i]->GetName()))
      {
        return (uint)i;
      }
    }

    return (uint)~0;
  }

  csFlags& AnimeshObjectFactory::GetFlags ()
  {
    return factoryFlags;
  }

  csPtr<iMeshObject> AnimeshObjectFactory::NewInstance ()
  {
    csRef<iMeshObject> ref;
    ref.AttachNew (new AnimeshObject (this));
    return csPtr<iMeshObject> (ref);
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

  FactorySocket::FactorySocket (AnimeshObjectFactory* factory, CS::Animation::BoneID bone, 
    const char* name, csReversibleTransform transform)
    : scfImplementationType (this), factory (factory), bone (bone), name (name),
    transform (transform)
  {}

  const char* FactorySocket::GetName () const
  {
    return name.GetData ();
  }

  void FactorySocket::SetName (const char* value)
  {
    name = value;
  }

  const csReversibleTransform& FactorySocket::GetTransform () const
  {
    return transform;
  }

  void FactorySocket::SetTransform (csReversibleTransform& tf)
  {
    transform = tf;
  }

  CS::Animation::BoneID FactorySocket::GetBone () const
  {
    return bone;
  }
  
  void FactorySocket::SetBone (CS::Animation::BoneID bone)
  {
    this->bone = bone;
  }

  CS::Mesh::iAnimatedMeshFactory* FactorySocket::GetFactory ()
  {
    return factory;
  }



  AnimeshObject::AnimeshObject (AnimeshObjectFactory* factory)
    : scfImplementationType (this), factory (factory), logParent (0),
    material (0), mixMode (0), skeleton (0), morphVersion (0), morphStateChanged (false),
    skinVertexVersion (~0), skinNormalVersion (~0), skinTangentBinormalVersion (~0),
    morphVertexVersion (0), skinVertexLF (false), skinNormalLF (false), skinTangentBinormalLF (false)
  {
    postMorphVertices = factory->vertexBuffer;
    SetupSubmeshes ();
    SetupSockets ();

    if (factory->skeletonFactory)
    {
      skeleton = factory->skeletonFactory->CreateSkeleton ();
      skeleton->SetAnimatedMesh (this);
      skeletonVersion = skeleton->GetSkeletonStateVersion() - 1;
    }
  }

  void AnimeshObject::SetSkeleton (CS::Animation::iSkeleton2* newskel)
  {
    skeleton = newskel;
    if (skeleton)
    {
      skeleton->SetAnimatedMesh (this);
      skeletonVersion = skeleton->GetSkeletonStateVersion() - 1;
    }
    else
    {
      skeletonVersion = ~0;
    }
  }

  CS::Animation::iSkeleton2* AnimeshObject::GetSkeleton () const
  {
    return skeleton;
  }

  CS::Mesh::iAnimatedMeshSubMesh* AnimeshObject::GetSubMesh (size_t index) const
  {
    CS_ASSERT (index < submeshes.GetSize ());
    return submeshes[index];
  }

  size_t AnimeshObject::GetSubMeshCount () const
  {
    return submeshes.GetSize();
  }

  void AnimeshObject::SetMorphTargetWeight (uint target, float weight)
  {
    CS_ASSERT (target < factory->morphTargets.GetSize ());

    // allocating array now saves some tiny memory and some flops at each
    // frame until morph targets are used
    morphTargetWeights.SetSize (factory->morphTargets.GetSize(), 0.0f);

    if (morphTargetWeights[target] != weight)
    {
      morphTargetWeights[target] = weight;
      morphStateChanged = true;
    }
  }

  float AnimeshObject::GetMorphTargetWeight (uint target) const
  {
    if (morphTargetWeights.GetSize() > target)
      return morphTargetWeights[target];
    else
      return 0.0;
  }

  size_t AnimeshObject::GetSocketCount () const
  {
    return sockets.GetSize ();
  }

  CS::Mesh::iAnimatedMeshSocket* AnimeshObject::GetSocket (size_t index) const
  {
    CS_ASSERT (index < sockets.GetSize ());
    return sockets[index];
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
    //const csVector3& wo = o2wt.GetOrigin ();

    uint frameNum = rview->GetCurrentFrameNumber ();

    // Iterate all submeshes...
    for (size_t i = 0; i < submeshes.GetSize (); ++i)
    {
      if (!submeshes[i]->isRendering)
        continue;
      
      Submesh* sm = submeshes[i];
      FactorySubmesh* fsm = factory->submeshes[i];
      
      // Fetch the material
      iMaterialWrapper* submat = sm->material;
      if (!submat) submat = fsm->material;
      if (!submat) submat = material;
      if (!submat) submat = factory->material;

      if (!submat)
      {
        csPrintf ("INTERNAL ERROR: mesh used without material!\n");
        num = 0;
        return 0;
      }

      if (submat->IsVisitRequired ()) 
        submat->Visit ();

      for (size_t j = 0; j < fsm->indexBuffers.GetSize (); ++j)
      {
        bool rmCreated;
        CS::Graphics::RenderMesh*& meshPtr = rmHolder.GetUnusedMesh (rmCreated,
          frameNum);

        // Setup the render mesh
        meshPtr->clip_portal = clip_portal;
        meshPtr->clip_plane = clip_plane;
        meshPtr->clip_z_plane = clip_z_plane;
        meshPtr->do_mirror = camera->IsMirrored ();
        meshPtr->meshtype = CS_MESHTYPE_TRIANGLES;
        meshPtr->indexstart = 0;
        meshPtr->indexend = (unsigned int)fsm->indexBuffers[j]->GetElementCount ();
        meshPtr->material = submat;

        meshPtr->mixmode = mixMode;
        meshPtr->buffers = sm->bufferHolders[j];
        meshPtr->renderPrio = fsm->renderPriority;
        meshPtr->z_buf_mode = fsm->zbufMode;

        meshPtr->object2world = o2wt;
        meshPtr->bbox = GetObjectBoundingBox();
        meshPtr->geometryInstance = factory;
        meshPtr->variablecontext = sm->svContexts[j];

        renderMeshList.Push (meshPtr);
      }
    }

    MorphVertices ();
    PreskinLF ();

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
      skeleton->UpdateSkeleton ((current_time - lastTick) / 1000.0f);

      // Copy the skeletal state into our buffers
      UpdateLocalBoneTransforms ();
      UpdateSocketTransforms ();
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
    // TODO: use a pre-test on the collision boxes of each bones

    csSegment3 seg (start, end);
    csRenderBufferLock<csVector3> vrt (skeleton ? skinnedVertices : postMorphVertices);

    // Iterate all submeshes...
    for (size_t i = 0; i < submeshes.GetSize (); ++i)
    {
      if (!submeshes[i]->isRendering)
        continue;

      FactorySubmesh* fsm = factory->submeshes[i];
      for (size_t j = 0; j < fsm->indexBuffers.GetSize (); ++j)
      {
	iRenderBuffer* indexBuffer = submeshes[i]->GetFactorySubMesh ()->GetIndices (j);
	CS::TriangleIndicesStream<uint> triangles (indexBuffer,
						   CS_MESHTYPE_TRIANGLES);
	while (triangles.HasNext())
	{
	  CS::TriangleT<uint> t (triangles.Next());
	  if (csIntersect3::SegmentTriangle (seg, 
					     vrt[t.a], vrt[t.b], vrt[t.c], 
					     isect))
	  {
	    if (pr) *pr = csQsqrt (csSquaredDist::PointPoint (start, isect) /
				   csSquaredDist::PointPoint (start, end));
	    return true;
	  }
	}
      }
    }

    return false;
  }

  bool AnimeshObject::HitBeamObject (const csVector3& start, const csVector3& end,
    csVector3& isect, float* pr, int* polygon_idx,
    iMaterialWrapper** material, iMaterialArray* materials)
  {
    // TODO: use a pre-test on the collision boxes of each bones

    csSegment3 seg (start, end);
    float tot_dist = csSquaredDist::PointPoint (start, end);
    float dist, temp;
    float itot_dist = 1 / tot_dist;
    dist = temp = tot_dist;
    csVector3 tmp;
    iMaterialWrapper* mat = 0;
    csRenderBufferLock<csVector3> vrt (skeleton ? skinnedVertices : postMorphVertices);

    // Iterate all submeshes...
    for (size_t i = 0; i < submeshes.GetSize (); ++i)
    {
      if (!submeshes[i]->isRendering)
        continue;

      FactorySubmesh* fsm = factory->submeshes[i];
      for (size_t j = 0; j < fsm->indexBuffers.GetSize (); ++j)
      {
	iRenderBuffer* indexBuffer = submeshes[i]->GetFactorySubMesh ()->GetIndices (j);
	CS::TriangleIndicesStream<uint> triangles (indexBuffer,
						   CS_MESHTYPE_TRIANGLES);
	while (triangles.HasNext())
	{
	  CS::TriangleT<uint> t (triangles.Next());
	  if (csIntersect3::SegmentTriangle (seg, 
					     vrt[t.a], vrt[t.b], vrt[t.c], 
					     tmp))
	  {
	    temp = csSquaredDist::PointPoint (start, tmp);
	    if (temp < dist)
	    {
	      isect = tmp;
	      dist = temp;
	      //if (polygon_idx) *polygon_idx = i; // @@@ Uh, how to handle?
	      mat = submeshes[i]->GetMaterial ();
	    }
	  }
	}
      }
    }

    if (pr) *pr = csQsqrt (dist * itot_dist);
    if (dist >= tot_dist)
      return false;

    if (material) *material = mat;
    if (materials) materials->Push (mat);

    return true;
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

  void AnimeshObject::PositionChild (iMeshObject* child,csTicks current_time)
  {
    // TODO
  }

  void AnimeshObject::BuildDecal(const csVector3* pos, float decalRadius,
    iDecalBuilder* decalBuilder)
  {
    // TODO
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
        
        sv = svContext->GetVariableAdd (svNameVertexUnskinned);
        sv->SetValue (postMorphVertices);

        if (factory->normalBuffer)
        {
          sv = svContext->GetVariableAdd (svNameNormalUnskinned);
          sv->SetValue (factory->normalBuffer);
        }        

        if (factory->tangentBuffer)
        {
          sv = svContext->GetVariableAdd (svNameTangentUnskinned);
          sv->SetValue (factory->tangentBuffer);
        }        

        if (factory->binormalBuffer)
        {
          sv = svContext->GetVariableAdd (svNameBinormalUnskinned);
          sv->SetValue (factory->binormalBuffer);
        }

        
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
        
        if (subsm)
        {          
          sv = svContext->GetVariableAdd (svNameBoneTransforms);          
          sm->boneTransformArray.Push (sv);
        }
        else
        {
          if (!boneTransformArray)
          {
            boneTransformArray.AttachNew (new csShaderVariable(svNameBoneTransforms));
          }
          svContext->AddVariable (boneTransformArray);
        }

        sm->svContexts.Push (svContext);
      }

      csRef<RenderBufferAccessor> rba;
      rba.AttachNew (new RenderBufferAccessor (this));
      for (size_t j = 0; j < fsm->bufferHolders.GetSize (); ++j)
      {
        csRef<csRenderBufferHolder> bufferHolder;
        bufferHolder.AttachNew (new csRenderBufferHolder (*fsm->bufferHolders[j]));

        // Setup the accessor to this mesh
        bufferHolder->SetAccessor (rba, 
          CS_BUFFER_POSITION_MASK | CS_BUFFER_NORMAL_MASK | 
          CS_BUFFER_TANGENT_MASK | CS_BUFFER_BINORMAL_MASK);

        sm->bufferHolders.Push (bufferHolder);
      }

    }
  }

  void AnimeshObject::SetupSockets ()
  {
    sockets.DeleteAll ();

    for (size_t i = 0; i < factory->sockets.GetSize (); ++i)
    {
      csRef<Socket> newSocket;
      newSocket.AttachNew (new Socket(this, factory->sockets[i]));
      sockets.Push (newSocket);
    }
  }

  void AnimeshObject::UpdateLocalBoneTransforms ()
  {
    if (!skeleton)
      return; // nothing to update

    lastSkeletonState = skeleton->GetStateBindSpace ();
    skeletonVersion = skeleton->GetSkeletonStateVersion ();

    if (boneTransformArray)
    {
      // Update the global one
      boneTransformArray->SetArraySize (lastSkeletonState->GetBoneCount ()*2);
      
      csRef<csShaderVariable> sv;
      for (size_t i = 0, j = 0; i < lastSkeletonState->GetBoneCount (); ++i, j+=2)
      {
        const csVector3& v = lastSkeletonState->GetVector (i);
        const csQuaternion& q = lastSkeletonState->GetQuaternion (i);

        const csDualQuaternion dq (q, v);

        sv = boneTransformArray->GetArrayElement (j);
        if (!sv)
        {
          sv.AttachNew (new csShaderVariable (svNameBoneTransformsReal));
          boneTransformArray->SetArrayElement (j, sv);
        }
        sv->SetValue (dq.real);

        sv = boneTransformArray->GetArrayElement (j+1);
        if (!sv)
        {
          sv.AttachNew (new csShaderVariable (svNameBoneTransformsDual));
          boneTransformArray->SetArrayElement (j+1, sv);
        }
        sv->SetValue (dq.dual);
      }
    }

    // Iterate all submeshes...
    for (size_t i = 0; i < submeshes.GetSize (); ++i)
    {
      Submesh* sm = submeshes[i];
      FactorySubmesh* fsm = sm->factorySubmesh;
      
      if (!sm->isRendering || sm->boneTransformArray.GetSize () == 0)
        continue;
      
      // Iterate over index-buffers
      for (size_t j = 0; j < sm->boneTransformArray.GetSize (); ++j)
      {
        csShaderVariable* boneTransformArray = sm->boneTransformArray[j];
        const FactorySubmesh::RemappedBones& remap = fsm->boneMapping[j];

        boneTransformArray->SetArraySize (remap.boneRemappingTable.GetSize ());

        csRef<csShaderVariable> sv;
        for (size_t bi = 0, k = 0; bi < remap.boneRemappingTable.GetSize (); ++bi, k+=2)
        {
          //unsigned int realBi = remap.boneRemappingTable[bi];

          // bi is the "virtual" bone index, realBi the real one
          const csVector3& v = lastSkeletonState->GetVector (i);
          const csQuaternion& q = lastSkeletonState->GetQuaternion (i);

          const csDualQuaternion dq (q, v);

          sv = boneTransformArray->GetArrayElement (k);
          if (!sv)
          {
            sv.AttachNew (new csShaderVariable (svNameBoneTransformsReal));
            boneTransformArray->SetArrayElement (k, sv);
          }
          sv->SetValue (dq.real);

          sv = boneTransformArray->GetArrayElement (k+1);
          if (!sv)
          {
            sv.AttachNew (new csShaderVariable (svNameBoneTransformsDual));
            boneTransformArray->SetArrayElement (k+1, sv);
          }
          sv->SetValue (dq.dual);
        }
      }

    }
  }

  void AnimeshObject::UpdateSocketTransforms ()
  {
    if (!skeleton)
      return;

    for (size_t i = 0; i < sockets.GetSize (); ++i)
    {
      CS::Animation::BoneID bone = sockets[i]->bone;

      csQuaternion q;
      csVector3 v;

      skeleton->GetTransformAbsSpace(bone, q, v);

      sockets[i]->socketBoneTransform.SetO2T (csMatrix3 (q));
      sockets[i]->socketBoneTransform.SetOrigin (v);
      sockets[i]->UpdateSceneNode ();
    }
  }

  void AnimeshObject::PreGetBuffer (csRenderBufferHolder* holder, 
    csRenderBufferName buffer)
  {  
    switch (buffer)
    {
      // Vertices render buffer
    case CS_BUFFER_POSITION:
      {
	// If there is no skeleton then simply use the morphed vertices
        if (!skeleton)
        {
          holder->SetRenderBuffer (CS_BUFFER_POSITION, postMorphVertices);
          return;
        }

	// Allocate a new render buffer if needed
        if (!skinnedVertices ||
	    skinnedVertices->GetElementCount () < factory->GetVertexCountP ())
        {
          skinnedVertices = csRenderBuffer::CreateRenderBuffer (factory->GetVertexCountP (),
            CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 3);

          skinVertexVersion = skeletonVersion - 1;
        }

        holder->SetRenderBuffer (CS_BUFFER_POSITION, skinnedVertices);

	// Update the skinning of the vertices if needed
	if (skeletonVersion != skinVertexVersion
	    || morphVersion != morphVertexVersion)
	{
	  SkinVertices ();
	  skinVertexVersion = skeletonVersion;
	  morphVertexVersion = morphVersion;
	}
        skinVertexLF = true;
      }
      break;

      // Normals render buffer
    case CS_BUFFER_NORMAL:
      {
	// If there is no skeleton then simply use the factory's buffer
        if (!skeleton)
        {
          holder->SetRenderBuffer (CS_BUFFER_NORMAL, factory->normalBuffer);
          return;
        }

	// Allocate a new render buffer if needed
        if (!skinnedNormals ||
	    skinnedNormals->GetElementCount () < factory->GetVertexCountP ())
        {
          skinnedNormals = csRenderBuffer::CreateRenderBuffer (factory->GetVertexCountP (),
            CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 3);
         
          skinNormalVersion = skeletonVersion - 1;
        }

        holder->SetRenderBuffer (CS_BUFFER_NORMAL, skinnedNormals);

	// Update the skinning of the normals if needed
        if (skeletonVersion != skinNormalVersion)
        {
	  SkinNormals ();
          skinNormalVersion = skeletonVersion;
        }
        skinNormalLF = true;
      }
      break;

      // Tangents and binormals render buffers
    case CS_BUFFER_TANGENT:
    case CS_BUFFER_BINORMAL:
      {
	// If there is no skeleton then simply use the factory's buffers
        if (!skeleton)
        {
          holder->SetRenderBuffer (CS_BUFFER_TANGENT, factory->tangentBuffer);
          holder->SetRenderBuffer (CS_BUFFER_BINORMAL, factory->binormalBuffer);
          return;
        }

	// Allocate new render buffers if needed
        if (!skinnedTangents ||
	    skinnedTangents->GetElementCount () < factory->GetVertexCountP ())
        {
          skinnedTangents = csRenderBuffer::CreateRenderBuffer (factory->GetVertexCountP (),
            CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 3);

          skinTangentBinormalVersion = skeletonVersion - 1;
        }
      
        if (!skinnedBinormals ||
	    skinnedBinormals->GetElementCount () < factory->GetVertexCountP ())
        {
          skinnedBinormals = csRenderBuffer::CreateRenderBuffer (factory->GetVertexCountP (),
            CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 3);
          
          skinTangentBinormalVersion = skeletonVersion - 1;
        }

        holder->SetRenderBuffer (CS_BUFFER_TANGENT, skinnedTangents);
        holder->SetRenderBuffer (CS_BUFFER_BINORMAL, skinnedBinormals);

	// Update the skinning of the buffers if needed
        if (skeletonVersion != skinTangentBinormalVersion)
        {
	  SkinTangentAndBinormal ();
          skinTangentBinormalVersion = skeletonVersion;
        }
        skinTangentBinormalLF = true;
      }
      break;

    default: //Empty..
      break;
    }
  }

  void AnimeshObject::PreskinLF ()
  {
    // Pre-skin the buffers if they were needed last frame
    bool reSkinVertex = skinVertexLF
      && (skinVertexVersion != skeletonVersion
	  || morphVertexVersion != morphVersion);

    bool reSkinNormal = skinNormalLF
      && skinNormalVersion != skeletonVersion;

    bool reSkinTangentBinormal = skinTangentBinormalLF
      && skinTangentBinormalVersion != skeletonVersion;

    if (reSkinVertex)
    {
      if (reSkinNormal)
      {
	if (reSkinTangentBinormal)
	  SkinAll ();

	else
	  SkinVerticesAndNormals ();
      }

      else
      {
	SkinVertices ();

	if (reSkinTangentBinormal)
	  SkinTangentAndBinormal ();
      }
    }

    else
    {
      if (reSkinNormal)
	SkinNormals ();

      if (reSkinTangentBinormal)
	SkinTangentAndBinormal ();
    }


    if (reSkinVertex)
    {
      skinVertexVersion = skeletonVersion;
      morphVertexVersion = morphVersion;
    }

    if (reSkinNormal)
      skinNormalVersion = skeletonVersion;

    if (reSkinTangentBinormal)
      skinTangentBinormalVersion = skeletonVersion;

    skinVertexLF = skinNormalLF = skinTangentBinormalLF = false;
  }

  AnimeshObject::Socket::Socket (AnimeshObject* object, FactorySocket* factorySocket)
    : scfImplementationType (this), object (object), factorySocket (factorySocket),
    bone (factorySocket->bone), transform (factorySocket->transform), sceneNode (0)
  {
  }

  const char* AnimeshObject::Socket::GetName () const
  {
    return factorySocket->GetName ();
  }

  CS::Mesh::iAnimatedMeshSocketFactory* AnimeshObject::Socket::GetFactory ()
  {
    return factorySocket;
  }

  const csReversibleTransform& AnimeshObject::Socket::GetTransform () const
  {
    return transform;
  }

  void AnimeshObject::Socket::SetTransform (csReversibleTransform& tf)
  {
    transform = tf;
  }

  const csReversibleTransform AnimeshObject::Socket::GetFullTransform () const
  {
    return socketBoneTransform*transform;
  }

  CS::Animation::BoneID AnimeshObject::Socket::GetBone () const
  {
    return bone;
  }

  CS::Mesh::iAnimatedMesh* AnimeshObject::Socket::GetMesh () const
  {
    return object;
  }

  iSceneNode* AnimeshObject::Socket::GetSceneNode () const
  {
    return sceneNode;
  }

  void AnimeshObject::Socket::SetSceneNode (iSceneNode* sn)
  {
    sceneNode = sn;
  }

  void AnimeshObject::Socket::UpdateSceneNode ()
  {
    if (!sceneNode)
      return;

    iMovable* mov = sceneNode->GetMovable ();
    mov->SetTransform (GetFullTransform ());
    mov->UpdateMove ();
  }

}
CS_PLUGIN_NAMESPACE_END(Animesh)

