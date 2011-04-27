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

csImposterMesh::csImposterMesh (csEngine* engine, iImposterFactory* fact,
                                iMeshWrapper* pmesh, iRenderView* rview,
                                csArray<ImposterShader>& shaders) :
scfImplementationType (this), fact (fact), shaders (shaders),
materialUpdateNeeded (false), lastDistance (FLT_MAX), isUpdating (false),
rendered (false)
{
  // Misc inits.
  vertices.SetVertexCount (4);
  meshLocalDir.Set (0, 0, 0);
  cameraLocalDir.Set (0, 0, 0);
  camera = rview->GetCamera()->Clone();
  sector = pmesh->GetMovable()->GetSectors()->Get(0);

  // Set the initial distance.
  originalMesh = pmesh;
  realDistance = (rview->GetCamera()->GetTransform().GetOrigin()
    - pmesh->GetMovable()->GetPosition()).Norm();

  // Init the imposter mesh.
  InitMesh();

  // Register this imposter with the manager.
  impman = csQueryRegistry<iImposterManager>(engine->GetObjectRegistry());
  impman->Register(this);
}

void csImposterMesh::Destroy()
{
  impman->Unregister(this);
}

void csImposterMesh::Update(iRenderView* rview)
{
    // Update if the camera isn't valid.
    bool update = !camera.IsValid();

    // Check that this imposter mesh is valid for this meshwrapper.
    if(!WithinTolerance(rview, originalMesh))
    {
      update = true;
    }

    // Update the distance.
    float distance = (rview->GetCamera()->GetTransform().GetOrigin()
      - originalMesh->GetMovable()->GetPosition()).Norm();
    if(distance < realDistance)
    {
      realDistance = distance;
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

      if(camera.IsValid() &&
        (update || realDistance < lastDistance))
      {
        lastDistance = realDistance;
        isUpdating = impman->Update(this);
      }
    }
}

void csImposterMesh::InitMesh()
{
  // Save camera orientation
  const csOrthoTransform oldt = camera->GetTransform();

  // Look at mesh
  const csBox3& wbbox = originalMesh->GetWorldBoundingBox();
  csVector3 meshcenter = wbbox.GetCenter();
  csVector3 campos = camera->GetTransform().GetOrigin();
  camera->GetTransform ().LookAt(meshcenter-campos, camera->GetTransform().GetT2O().Col2());

  // Calculate the min and max coordinates of the bbox in camera space.
  csVector3 c = camera->GetTransform().Other2This(wbbox.GetCorner(0));
  float minX = c.x, maxX = c.x, minY = c.y, maxY = c.y, minZ = c.z;

  for (int i=1; i<7; ++i)
  {
    c = camera->GetTransform().Other2This(wbbox.GetCorner(i));

    if(c.x < minX)
      minX = c.x;

    if(c.x > maxX)
      maxX = c.x;

    if(c.y < minY)
      minY = c.y;

    if(c.y > maxY)
      maxY = c.y;

    if(c.z < minZ)
      minZ = c.z;
  }

  // Create a 2d bbox and calculate the world space vertex coordinates.
  vertices[0] = camera->GetTransform().This2Other(csVector3(maxX, maxY, minZ));
  vertices[1] = camera->GetTransform().This2Other(csVector3(maxX, minY, minZ));
  vertices[2] = camera->GetTransform().This2Other(csVector3(minX, minY, minZ));
  vertices[3] = camera->GetTransform().This2Other(csVector3(minX, maxY, minZ));

  // Get normals.
  normals = camera->GetTransform().This2Other(csVector3(0, 0, -1));

  // Restore camera orientation.
  camera->SetTransform (oldt);

  // Save current facing for angle checking
  csReversibleTransform objt = originalMesh->GetMovable()->GetFullTransform();
  csOrthoTransform& camt = camera->GetTransform();

  csVector3 relativeDir = (originalMesh->GetWorldBoundingBox().GetCenter()
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

csBatchedImposterMesh::csBatchedImposterMesh (csEngine* engine, iSector* sector) : scfImplementationType(this),
engine(engine), sector(sector), meshDirty(true), currentMesh(0)
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

  mesh->SetZBufMode (CS_ZBUF_TEST);
  mesh->GetFlags().Set(CS_ENTITY_NOLIGHTING | CS_ENTITY_NOSHADOWS | CS_ENTITY_NOSHADOWRECEIVE);
  flags.Set(CS_ENTITY_NOLIGHTING | CS_ENTITY_NOSHADOWS | CS_ENTITY_NOSHADOWRECEIVE);

  csRef<iConfigManager> cfman = csQueryRegistry<iConfigManager>(engine->GetObjectRegistry());
  updatePerFrame = cfman->GetInt("Engine.Imposters.UpdatePerFrame", 10);
}

//static arrays that keep the imposterdata
CS_IMPLEMENT_STATIC_VAR (GetMeshIndices, csDirtyAccessArray<uint>, ());
CS_IMPLEMENT_STATIC_VAR (GetMeshVertices, csDirtyAccessArray<csVector3>, ());
CS_IMPLEMENT_STATIC_VAR (GetMeshTexels, csDirtyAccessArray<csVector2>, ());
CS_IMPLEMENT_STATIC_VAR (GetMeshColors, csDirtyAccessArray<csVector4>, ());

csRenderMesh** csBatchedImposterMesh::GetRenderMeshes (int& num, iRenderView* rview, 
                                                iMovable* movable, uint32 frustum_mask)
{
  // Get an unused mesh
  bool rmCreated;
  csRenderMesh*& mesh = rmHolder.GetUnusedMesh (rmCreated,
    rview->GetCurrentFrameNumber ());

  SetupRenderMeshes(mesh, rmCreated, rview);

  // Trigger an update of some imposters for the next frame.
  {
    if(currentMesh >= imposterMeshes.GetSize())
      currentMesh = 0;

    uint i = currentMesh;
    for(; i<currentMesh+updatePerFrame && i<imposterMeshes.GetSize(); ++i)
    {
      imposterMeshes[i]->camera = rview->GetCamera();
      imposterMeshes[i]->Update(rview);
    }
    currentMesh = i;
  }

  num = 1;
  return &mesh;
}

int csBatchedImposterMesh::ImposterMeshSort(csImposterMesh* const& f, csImposterMesh* const& s)
{
  // Get distance from camera.
  csReversibleTransform tr_o2c = f->camera->GetTransform ();
  if (!f->originalMesh->GetMovable()->IsFullTransformIdentity())
    tr_o2c /= f->originalMesh->GetMovable()->GetFullTransform();
  const csBox3& cbboxf = f->originalMesh->GetTransformedBoundingBox(tr_o2c);
  float distancef = cbboxf.MinZ();

  tr_o2c = s->camera->GetTransform ();
  if (!s->originalMesh->GetMovable()->IsFullTransformIdentity())
    tr_o2c /= s->originalMesh->GetMovable()->GetFullTransform();
  const csBox3& cbboxs = s->originalMesh->GetTransformedBoundingBox(tr_o2c);
  float distances = cbboxs.MinZ();

  return (distancef > distances) ? -1 : 1;
}

void csBatchedImposterMesh::SetupRenderMeshes(csRenderMesh*& mesh, bool rmCreated, iRenderView* rview)
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
    mesh->alphaType = csAlphaMode::alphaSmooth;
    mesh->z_buf_mode = CS_ZBUF_TEST;
    mesh->renderPrio = engine->GetRenderPriority("alpha");
    mesh->material = 0;

    csRef<csRenderBufferHolder> buffer = new csRenderBufferHolder();
    mesh->buffers = buffer;
    mesh->variablecontext = new csShaderVariableContext();
  }

  // Mesh changed.
  if (meshDirty || rmCreated)
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
    uint meshCount = (uint)sortedMeshes.GetSize();

    if(numImposterMeshes != meshCount || rmCreated)
    {
      // Update mesh count.
      numImposterMeshes = meshCount;

      // Create index buffer.
      mesh_indices.Empty();

      for(uint i=0; i<meshCount; ++i)
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

      for(uint i=0; i<meshCount; ++i)
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
      for(uint i=0; i<meshCount; ++i)
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
    for(uint i=0; i<meshCount; ++i)
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
    for(uint i=0; i<meshCount; ++i)
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

    meshDirty = false;
  }
}
