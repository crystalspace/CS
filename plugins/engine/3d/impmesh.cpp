/*
    Copyright (C) 2009 by Keith Fulton and Jorrit Tyberghein
    Rewritten during Summer of Code 2006 by Christoph "Fossi" Mewes
    Rewritten during Summer of Code 2009 by Michael Gist

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
#include "csqsqrt.h"
#include "iengine/portal.h"
#include "iengine/rview.h"
#include "ivideo/graph3d.h"
#include "csgfx/renderbuffer.h"

#include "camera.h"
#include "material.h"
#include "impmesh.h"
#include "sector.h"
#include "meshobj.h"
#include "light.h"
#include "engine.h"

csImposterMesh::csImposterMesh (csEngine* engine, iSector* sector) : scfImplementationType(this),
engine(engine), sector(sector), materialUpdateNeeded(false), instance(false), removeMe(false), 
matDirty(true), meshDirty(true), numImposterMeshes(0), rendered(false), currentMesh(0)
{
  // Create meshwrapper.
  csMeshWrapper* cmesh = new csMeshWrapper(engine, this);
  cmesh->SetName("imposter");
  cmesh->SetRenderPriority(engine->GetRenderPriority("alpha"));
  mesh = csPtr<iMeshWrapper>(cmesh);

  // Set bbox so that it's never culled.
  mesh->GetMeshObject()->GetObjectModel()->SetObjectBoundingBox(
    csBox3(-CS_BOUNDINGBOX_MAXVALUE, -CS_BOUNDINGBOX_MAXVALUE, -CS_BOUNDINGBOX_MAXVALUE,
    CS_BOUNDINGBOX_MAXVALUE, CS_BOUNDINGBOX_MAXVALUE, CS_BOUNDINGBOX_MAXVALUE));

  mesh->GetMovable()->SetPosition(csVector3(0.0f));
  mesh->GetMovable()->SetSector(sector);
  mesh->GetMovable()->UpdateMove();

  mesh->GetFlags().Set(CS_ENTITY_NOLIGHTING | CS_ENTITY_NOSHADOWS | CS_ENTITY_NOSHADOWRECEIVE);
  flags.Set(CS_ENTITY_NOLIGHTING | CS_ENTITY_NOSHADOWS | CS_ENTITY_NOSHADOWRECEIVE);
}

csImposterMesh::csImposterMesh (csEngine* engine, iImposterFactory* fact,
                                iMeshWrapper* pmesh, iRenderView* rview,
                                bool instance, const char* shader) :
scfImplementationType(this), fact(fact), engine(engine), instance(instance), removeMe(false),
shader(shader), materialUpdateNeeded(false), matDirty(true), meshDirty(true),
camera(rview->GetCamera()), isUpdating(false), rendered(false)
{
  // Misc inits.
  vertices.SetVertexCount (4);
  meshLocalDir.Set (0, 0, 0);
  cameraLocalDir.Set (0, 0, 0);
  sector = pmesh->GetMovable()->GetSectors()->Get(0);
  g3d = csQueryRegistry<iGraphics3D>(engine->GetObjectRegistry());

  if(instance)
  {
    // Create meshwrapper.
    csMeshWrapper* cmesh = new csMeshWrapper(engine, this);
    csString name(pmesh->QueryObject()->GetName());
    cmesh->SetName(name + "_imposter");
    cmesh->SetRenderPriority(engine->GetRenderPriority("alpha"));
    mesh = csPtr<iMeshWrapper>(cmesh);

    // Set bbox so that it's never culled.
    mesh->GetMeshObject()->GetObjectModel()->SetObjectBoundingBox(
      csBox3(-CS_BOUNDINGBOX_MAXVALUE, -CS_BOUNDINGBOX_MAXVALUE, -CS_BOUNDINGBOX_MAXVALUE,
      CS_BOUNDINGBOX_MAXVALUE, CS_BOUNDINGBOX_MAXVALUE, CS_BOUNDINGBOX_MAXVALUE));

    // Init shadervar array.
    csRef<iShaderVarStringSet> SVstrings = csQueryRegistryTagInterface<iShaderVarStringSet>(
      engine->GetObjectRegistry(), "crystalspace.shader.variablenameset");
    CS::ShaderVarStringID varTransform = SVstrings->Request("instancing transforms");
    transformVars.AttachNew (new csShaderVariable (varTransform));
    transformVars->SetType (csShaderVariable::ARRAY);
    transformVars->SetArraySize (0);

    CS::ShaderVarStringID varFadeFactor = SVstrings->Request ("alpha factor");
    fadeFactors.AttachNew (new csShaderVariable (varFadeFactor));
    fadeFactors->SetType (csShaderVariable::ARRAY);
    fadeFactors->SetArraySize (0);

    // Add instancing shadervars.
    AddSVToMesh(mesh, transformVars);
    AddSVToMesh(mesh, fadeFactors);

    // Create first instance of the imposter.
    CreateInstance(pmesh);
  }

  // Set the initial distance.
  closestInstanceMesh = pmesh;
  closestInstance = (rview->GetCamera()->GetTransform().GetOrigin()
    - pmesh->GetMovable()->GetPosition()).Norm();

  // Init the imposter mesh.
  InitMesh();

  // Register this imposter with the manager.
  impman = csQueryRegistry<iImposterManager>(engine->GetObjectRegistry());
  impman->Register(this);
}

void csImposterMesh::Destroy()
{
  for(size_t i=0; i<instances.GetSize(); ++i)
  {
    DestroyInstance(instances[i]);
  }

  impman->Unregister(this);
}

bool csImposterMesh::Add(iMeshWrapper* mesh, iRenderView* rview)
{
  if(WithinTolerance(rview, mesh))
  {
    CreateInstance(mesh);

    // Update the range.
    float distance = (rview->GetCamera()->GetTransform().GetOrigin()
      - mesh->GetMovable()->GetPosition()).Norm();
    if(distance < closestInstance || closestInstanceMesh == mesh)
    {
      closestInstanceMesh = mesh;
      closestInstance = distance;
    }

    return true;
  }

  return false;
}

bool csImposterMesh::Update(iMeshWrapper* mesh, iRenderView* rview)
{
  for(size_t i=0; i<instances.GetSize() || !instance; ++i)
  {
    if((!instance && closestInstanceMesh == mesh) ||
      (instance && instances[i]->mesh == mesh))
    {
      // Update if the camera isn't valid.
      bool update = !camera.IsValid();

      // Check that this imposter mesh is valid for this meshwrapper.
      if(!WithinTolerance(rview, mesh))
      {
        if(instance)
        {
          DestroyInstance(instances[i]);
          instances.DeleteIndexFast(i);
          return false;
        } 
        else
        {
          update = true;
        }
      }

      // Update the distance.
      float distance = (rview->GetCamera()->GetTransform().GetOrigin()
        - mesh->GetMovable()->GetPosition()).Norm();
      if(distance < closestInstance || closestInstanceMesh == mesh)
      {
        closestInstanceMesh = mesh;
        closestInstance = distance;
      }

      // Update material!
      if(!isUpdating)
      {
        if(update)
        {
          // Update mesh
          camera = rview->GetCamera();
          InitMesh();
          materialUpdateNeeded = true;
        }

        if(camera.IsValid())
        {
          isUpdating = impman->Update(this);
        }
      }

      return true;
    }

    if(!instance)
      break;
  }

  return false;
}

bool csImposterMesh::Remove(iMeshWrapper* mesh)
{
  for(size_t i=0; i<instances.GetSize(); ++i)
  {
    if(instances[i]->mesh == mesh)
    {
      DestroyInstance(instances[i]);
      instances.DeleteIndexFast(i);
      return true;
    }
  }

  // Mark for removal.
  if(!instance && mesh == closestInstanceMesh)
  {
    removeMe = true;
    return true;
  }

  return false;
}

void csImposterMesh::AddSVToMesh(iMeshWrapper* mesh,  
                                 csShaderVariable* sv) 
{ 
  if (!mesh)
    return;

  mesh->GetSVContext()->AddVariable(sv);

  csRef<iSceneNodeArray> children = mesh->QuerySceneNode()->GetChildrenArray(); 
  for(size_t i=0; i<children->GetSize(); ++i)
  {
    AddSVToMesh (children->Get(i)->QueryMesh(), sv);
  }
}

void csImposterMesh::CreateInstance(iMeshWrapper* pmesh)
{
  Instance* instance = new Instance(pmesh);
  instance->transformVar.AttachNew(new csShaderVariable);
  instance->fadeFactor.AttachNew(new csShaderVariable);

  csMatrix3 rot;
  csReversibleTransform tr(rot, pmesh->GetMovable()->GetPosition());
  instance->transformVar->SetValue(tr);
  instance->fadeFactor->SetValue(1.0f);

  transformVars->AddVariableToArray(instance->transformVar);
  fadeFactors->AddVariableToArray(instance->fadeFactor);
  instances.Push(instance);

  // Mark original mesh as invisible.
  pmesh->GetFlags().Set(CS_ENTITY_INVISIBLEMESH);
}

void csImposterMesh::DestroyInstance(Instance* instance)
{
  // Mark original mesh as visible.
  instance->mesh->GetFlags().Reset(CS_ENTITY_INVISIBLEMESH);

  size_t idx = transformVars->FindArrayElement(instance->transformVar); 
  CS_ASSERT(idx != csArrayItemNotFound);

  transformVars->RemoveFromArray(idx);
  fadeFactors->RemoveFromArray(idx);
  delete instance;
}

void csImposterMesh::InitMesh()
{
  // Save camera orientation
  const csOrthoTransform oldt = camera->GetTransform();

  // Look at mesh
  csVector3 meshcenter = closestInstanceMesh->GetWorldBoundingBox().GetCenter();
  csVector3 campos = camera->GetTransform().GetOrigin();
  camera->GetTransform ().LookAt(meshcenter-campos, camera->GetTransform().GetT2O().Col2());

  if (instance)
  {
    // Get camera and object space size.
    csReversibleTransform tr_o2c = camera->GetTransform ();
    if (!closestInstanceMesh->GetMovable()->IsFullTransformIdentity())
      tr_o2c /= closestInstanceMesh->GetMovable()->GetFullTransform();
    const csBox3& cbbox = closestInstanceMesh->GetTransformedBoundingBox(tr_o2c);
    const csBox3& obbox = closestInstanceMesh->GetMeshObject()->GetObjectModel()->GetObjectBoundingBox();

    // Calculate object space billboard size.
    float z = (bbox.MinZ() + bbox.MaxZ()) / 2;
    vertices[0] = csVector3(cbbox.MaxX(), obbox.MaxY(), z);
    vertices[1] = csVector3(cbbox.MaxX(), obbox.MinY(), z);
    vertices[2] = csVector3(cbbox.MinX(), obbox.MinY(), z);
    vertices[3] = csVector3(cbbox.MinX(), obbox.MaxY(), z);
  }
  else
  {
    // Get screen bounding box
    csScreenBoxResult screenbox = closestInstanceMesh->GetScreenBoundingBox(camera);

    // Unproject screen bounding box into camera space.
    csVector3 v1 = camera->InvPerspective(screenbox.sbox.GetCorner(3), screenbox.cbox.MinZ());
    csVector3 v2 = camera->InvPerspective(screenbox.sbox.GetCorner(2), screenbox.cbox.MinZ());
    csVector3 v3 = camera->InvPerspective(screenbox.sbox.GetCorner(0), screenbox.cbox.MinZ());
    csVector3 v4 = camera->InvPerspective(screenbox.sbox.GetCorner(1), screenbox.cbox.MinZ());

    // Get world space vertex coordinates.
    vertices[0] = camera->GetTransform().This2Other(v1);
    vertices[1] = camera->GetTransform().This2Other(v2);
    vertices[2] = camera->GetTransform().This2Other(v3);
    vertices[3] = camera->GetTransform().This2Other(v4);

    // Get normals.
    normals = camera->GetTransform().This2Other(csVector3(0, 0, -1));
  }

  // Restore camera orientation.
  camera->SetTransform (oldt);

  // Save current facing for angle checking
  csReversibleTransform objt = closestInstanceMesh->GetMovable()->GetFullTransform();
  csOrthoTransform& camt = camera->GetTransform();

  csVector3 relativeDir = (closestInstanceMesh->GetWorldBoundingBox().GetCenter()
    - camt.GetOrigin()).Unit();

  meshLocalDir = objt.Other2ThisRelative(relativeDir);
  cameraLocalDir = camt.Other2ThisRelative(relativeDir);
}

bool csImposterMesh::WithinTolerance(iRenderView *rview, iMeshWrapper* pmesh)
{
  csReversibleTransform objt = pmesh->GetMovable()->GetFullTransform();
  csOrthoTransform& camt = rview->GetCamera()->GetTransform();

  csVector3 relativeDir = (pmesh->GetWorldBoundingBox().GetCenter() - camt.GetOrigin()).Unit();
  csVector3 meshDir = objt.This2OtherRelative(meshLocalDir);

  if(meshDir * relativeDir < cosf(fact->GetRotationTolerance()))
  {
    return false;
  }
  else
  {
    csVector3 cameraDir = camt.This2OtherRelative(cameraLocalDir);
    if(cameraDir * relativeDir < cosf(fact->GetCameraRotationTolerance()))
    {
      return false;
    }
  }

  return true;
}

//static arrays that keep the imposterdata
CS_IMPLEMENT_STATIC_VAR (GetMeshIndices, csDirtyAccessArray<uint>, ());
CS_IMPLEMENT_STATIC_VAR (GetMeshVertices, csDirtyAccessArray<csVector3>, ());
CS_IMPLEMENT_STATIC_VAR (GetMeshTexels, csDirtyAccessArray<csVector2>, ());
CS_IMPLEMENT_STATIC_VAR (GetMeshColors, csDirtyAccessArray<csVector4>, ());

csRenderMesh** csImposterMesh::GetRenderMeshes (int& num, iRenderView* rview, 
                                                iMovable* movable, uint32 frustum_mask)
{
  // Get an unused mesh
  bool rmCreated;
  csRenderMesh*& mesh = rmHolder.GetUnusedMesh (rmCreated,
    rview->GetCurrentFrameNumber ());

  if(instance)
  {
    SetupRenderMeshesInstance(mesh, rmCreated, rview->GetCamera());
  }
  else
  {
    SetupRenderMeshes(mesh, rmCreated, rview);
  }

  num = 1;
  return &mesh;
}

int csImposterMesh::ImposterMeshSort(csImposterMesh* const& f, csImposterMesh* const& s)
{
  // Get distance from camera.
  csReversibleTransform tr_o2c = f->camera->GetTransform ();
  if (!f->closestInstanceMesh->GetMovable()->IsFullTransformIdentity())
    tr_o2c /= f->closestInstanceMesh->GetMovable()->GetFullTransform();
  const csBox3& cbboxf = f->closestInstanceMesh->GetTransformedBoundingBox(tr_o2c);
  float distancef = cbboxf.MinZ();

  tr_o2c = s->camera->GetTransform ();
  if (!s->closestInstanceMesh->GetMovable()->IsFullTransformIdentity())
    tr_o2c /= s->closestInstanceMesh->GetMovable()->GetFullTransform();
  const csBox3& cbboxs = s->closestInstanceMesh->GetTransformedBoundingBox(tr_o2c);
  float distances = cbboxs.MinZ();

  return (distancef > distances) ? -1 : 1;
}

void csImposterMesh::SetupRenderMeshes(csRenderMesh*& mesh, bool rmCreated, iRenderView* rview)
{
  csDirtyAccessArray<uint>& mesh_indices = *GetMeshIndices ();
  csDirtyAccessArray<csVector3>& mesh_vertices = *GetMeshVertices ();
  csDirtyAccessArray<csVector2>& mesh_texels = *GetMeshTexels ();
  csDirtyAccessArray<csVector4>& mesh_colors = *GetMeshColors ();

  if (rmCreated)
  {
    // Initialize mesh
    mesh->meshtype = CS_MESHTYPE_TRIANGLES;
    mesh->do_mirror = rview->GetCamera()->IsMirrored ();
    mesh->object2world = csReversibleTransform ();
    mesh->worldspace_origin = csVector3 (0,0,0);
    mesh->mixmode = CS_FX_COPY;
    mesh->alphaType = csAlphaMode::alphaBinary;
    mesh->z_buf_mode = CS_ZBUF_TEST;
    mesh->renderPrio = engine->GetRenderPriority("alpha");
    mesh->material = 0;

    csRef<csRenderBufferHolder> buffer = new csRenderBufferHolder();
    mesh->buffers = buffer;
    mesh->variablecontext = new csShaderVariableContext();
  }

  // Material or mesh changed.
  if (matDirty || meshDirty || rmCreated)
  {
    // Update material pointer.
    mesh->material = mat;

    // Sort imposter meshes.
    csArray<csImposterMesh*> sortedMeshes;
    for(size_t i=0; i<imposterMeshes.GetSize(); ++i)
    {
      imposterMeshes[i]->camera = rview->GetCamera();
      sortedMeshes.InsertSorted(imposterMeshes[i], &ImposterMeshSort);
    }

    // Check for new imposter meshes.
    size_t meshCount = sortedMeshes.GetSize();

    if(numImposterMeshes != meshCount || rmCreated)
    {
      // Update mesh count.
      numImposterMeshes = meshCount;

      // Create index buffer.
      mesh_indices.Empty();

      for(size_t i=0; i<meshCount; ++i)
      {
        mesh_indices.Push (0+4*i);
        mesh_indices.Push (1+4*i);
        mesh_indices.Push (2+4*i);
        mesh_indices.Push (2+4*i);
        mesh_indices.Push (3+4*i);
        mesh_indices.Push (0+4*i);
      }

      mesh->indexstart = 0;
      mesh->indexend = 6*meshCount;

      csRef<csRenderBuffer> indexBuffer = csRenderBuffer::CreateIndexRenderBuffer(
        mesh_indices.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_INT, 0, 3*meshCount);
      indexBuffer->CopyInto (mesh_indices.GetArray(), mesh_indices.GetSize());
      mesh->buffers->SetRenderBuffer (CS_BUFFER_INDEX, indexBuffer);

      // Create colour buffer.
      static csVector4 c (1, 1, 1, 1.0);
      mesh_colors.Empty ();

      for(size_t i=0; i<meshCount; ++i)
      {
        mesh_colors.Push (c);
        mesh_colors.Push (c);
        mesh_colors.Push (c);
        mesh_colors.Push (c);
      }

      csRef<csRenderBuffer> colBuffer = csRenderBuffer::CreateRenderBuffer(
        mesh_colors.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 4);
      colBuffer->CopyInto (mesh_colors.GetArray(), mesh_colors.GetSize());
      mesh->buffers->SetRenderBuffer (CS_BUFFER_COLOR, colBuffer);

      // Create normals buffer.
      csDirtyAccessArray<csVector3> normals;
      for(size_t i=0; i<meshCount; ++i)
      {
        normals.Push(sortedMeshes[i]->normals);
        normals.Push(sortedMeshes[i]->normals);
        normals.Push(sortedMeshes[i]->normals);
        normals.Push(sortedMeshes[i]->normals);
      }

      csRef<csRenderBuffer> normalBuffer = csRenderBuffer::CreateRenderBuffer(
        normals.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
      normalBuffer->CopyInto (normals.GetArray(), normals.GetSize());
      mesh->buffers->SetRenderBuffer (CS_BUFFER_NORMAL, normalBuffer);
    }

    // Create texels buffer.
    mesh_texels.Empty ();
    for(size_t i=0; i<meshCount; ++i)
    {
      mesh_texels.Push (csVector2 (sortedMeshes[i]->texCoords.MaxX(),1-sortedMeshes[i]->texCoords.MaxY()));
      mesh_texels.Push (csVector2 (sortedMeshes[i]->texCoords.MaxX(),1-sortedMeshes[i]->texCoords.MinY()));
      mesh_texels.Push (csVector2 (sortedMeshes[i]->texCoords.MinX(),1-sortedMeshes[i]->texCoords.MinY()));
      mesh_texels.Push (csVector2 (sortedMeshes[i]->texCoords.MinX(),1-sortedMeshes[i]->texCoords.MaxY()));
    }

    csRef<csRenderBuffer> texBuffer = csRenderBuffer::CreateRenderBuffer(
      mesh_texels.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 2);
    texBuffer->CopyInto (mesh_texels.GetArray(), mesh_texels.GetSize());
    mesh->buffers->SetRenderBuffer (CS_BUFFER_TEXCOORD0, texBuffer);

    // Create vertex buffer.
    mesh_vertices.Empty ();
    for(size_t i=0; i<meshCount; ++i)
    {
      mesh_vertices.Push(sortedMeshes[i]->vertices.GetVertices()[0]);
      mesh_vertices.Push(sortedMeshes[i]->vertices.GetVertices()[1]);
      mesh_vertices.Push(sortedMeshes[i]->vertices.GetVertices()[2]);
      mesh_vertices.Push(sortedMeshes[i]->vertices.GetVertices()[3]);
    }

    csRef<csRenderBuffer> vertBuffer = csRenderBuffer::CreateRenderBuffer(
      mesh_vertices.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
    vertBuffer->CopyInto (mesh_vertices.GetArray(), mesh_vertices.GetSize());
    mesh->buffers->SetRenderBuffer (CS_BUFFER_POSITION, vertBuffer);

    matDirty = false;
    meshDirty = false;
  }
  else
  {
    // No change.. so trigger an update check.
    if(currentMesh >= imposterMeshes.GetSize())
      currentMesh = 0;

    uint i = currentMesh;
    for(; i<currentMesh+updatePerFrame && i<imposterMeshes.GetSize(); ++i)
    {
      iMeshWrapper* imesh = imposterMeshes[i]->closestInstanceMesh;
      imesh->GetFlags().Set(CS_ENTITY_INVISIBLEMESH);
      imposterMeshes[i]->camera = rview->GetCamera();
      imposterMeshes[i]->Update(imesh, rview);
    }
    currentMesh = i;
  }
}

void csImposterMesh::SetupRenderMeshesInstance(csRenderMesh*& mesh, bool rmCreated, iCamera* camera)
{
  csDirtyAccessArray<uint>& mesh_indices = *GetMeshIndices ();
  csDirtyAccessArray<csVector2>& mesh_texels = *GetMeshTexels ();
  csDirtyAccessArray<csVector4>& mesh_colors = *GetMeshColors ();

  if (rmCreated)
  {
    // Initialize mesh
    mesh->meshtype = CS_MESHTYPE_TRIANGLES;
    mesh->do_mirror = camera->IsMirrored ();
    mesh->object2world = csReversibleTransform ();
    mesh->worldspace_origin = csVector3 (0,0,0);
    mesh->mixmode = CS_FX_COPY;
    mesh->alphaType = csAlphaMode::alphaBinary;
    mesh->z_buf_mode = CS_ZBUF_TEST;
    mesh->renderPrio = engine->GetRenderPriority("alpha");
    mesh->material = 0;

    GetMeshIndices()->Empty();
    mesh_indices.Push (0);
    mesh_indices.Push (1);
    mesh_indices.Push (2);
    mesh_indices.Push (2);
    mesh_indices.Push (3);
    mesh_indices.Push (0);

    mesh->indexstart = 0;
    mesh->indexend = 6;

    csRef<csRenderBuffer> indexBuffer = 
      csRenderBuffer::CreateIndexRenderBuffer(
      6, CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_INT, 0, 3);
    indexBuffer->CopyInto (mesh_indices.GetArray(), 6);

    GetMeshColors ()->Empty ();

    csVector4 c (1, 1, 1, 1.0);
    mesh_colors.Push (c);
    mesh_colors.Push (c);
    mesh_colors.Push (c);
    mesh_colors.Push (c);

    csRef<csRenderBuffer> colBuffer = csRenderBuffer::CreateRenderBuffer(
      4, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 4);
    colBuffer->CopyInto (mesh_colors.GetArray(), 4);

    csRef<csRenderBufferHolder> buffer = new csRenderBufferHolder();
    buffer->SetRenderBuffer (CS_BUFFER_INDEX, indexBuffer);
    buffer->SetRenderBuffer (CS_BUFFER_COLOR, colBuffer);
    mesh->buffers = buffer;
    mesh->variablecontext = new csShaderVariableContext();

    // Set normals.
    csDirtyAccessArray<csVector3> normals;
    normals.Push(csVector3(0, 0, -1));
    normals.Push(csVector3(0, 0, -1));
    normals.Push(csVector3(0, 0, -1));
    normals.Push(csVector3(0, 0, -1));

    csRef<csRenderBuffer> normalBuffer = csRenderBuffer::CreateRenderBuffer(
      4, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
    normalBuffer->CopyInto (normals.GetArray(), 4);

    mesh->buffers->SetRenderBuffer (CS_BUFFER_NORMAL, normalBuffer);
  }

  // Material or mesh changed.
  if (matDirty || meshDirty)
  {
    mesh->material = mat;

    GetMeshTexels ()->Empty ();
    mesh_texels.Push (csVector2 (texCoords.MaxX(),1-texCoords.MaxY()));
    mesh_texels.Push (csVector2 (texCoords.MaxX(),1-texCoords.MinY()));
    mesh_texels.Push (csVector2 (texCoords.MinX(),1-texCoords.MinY()));
    mesh_texels.Push (csVector2 (texCoords.MinX(),1-texCoords.MaxY()));

    csRef<csRenderBuffer> texBuffer = csRenderBuffer::CreateRenderBuffer(
      4, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 2);
    texBuffer->CopyInto (mesh_texels.GetArray(), 4);

    mesh->buffers->SetRenderBuffer (CS_BUFFER_TEXCOORD0, texBuffer);

    // Recalculate vertex positions.
    InitMesh();
    GetMeshVertices ()->Empty ();
    csRef<csRenderBuffer> vertBuffer = csRenderBuffer::CreateRenderBuffer(
      4, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
    vertBuffer->CopyInto (vertices.GetVertices (), 4);
    mesh->buffers->SetRenderBuffer (CS_BUFFER_POSITION, vertBuffer);

    matDirty = false;
    meshDirty = false;
  }
}
