/*
    Copyright (C) 2002 by Keith Fulton and Jorrit Tyberghein
    Rewritten during Sommer of Code 2006 by Christoph "Fossi" Mewes

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
#include "iengine/impman.h"
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
                                iMeshWrapper* pmesh, iRenderView* rview) :
  scfImplementationType(this), engine(engine), fact(fact)
{
  // Misc inits.
  cutout.SetVertexCount (4);
  meshLocalDir.Set (0, 0, 0);
  cameraLocalDir.Set (0, 0, 0);
  sector = pmesh->GetMovable()->GetSectors()->Get(0);
  g3d = csQueryRegistry<iGraphics3D>(engine->GetObjectRegistry());

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

  // Create meshwrapper.
  csMeshWrapper* cmesh = new csMeshWrapper(engine, this);
  csString name(pmesh->QueryObject()->GetName());
  cmesh->SetName(name + "_imposter");
  mesh = static_cast<iMeshWrapper*>(cmesh);

  // Add instancing shadervars.
  AddSVToMesh(mesh, transformVars);
  AddSVToMesh(mesh, fadeFactors);

  // Set bbox so that it's never culled.
  mesh->GetMeshObject()->GetObjectModel()->SetObjectBoundingBox(
      csBox3(-CS_BOUNDINGBOX_MAXVALUE, -CS_BOUNDINGBOX_MAXVALUE, -CS_BOUNDINGBOX_MAXVALUE,
      CS_BOUNDINGBOX_MAXVALUE, CS_BOUNDINGBOX_MAXVALUE, CS_BOUNDINGBOX_MAXVALUE));

  // Create first instance of the imposter.
  CreateInstance(pmesh);

  // Init the imposter mesh.
  InitMesh(rview->GetCamera());

  // Register this imposter with the manager.
  impman = csQueryRegistry<iImposterManager>(engine->GetObjectRegistry());
  //impman->Register(this, rview);

  // Move imposter mesh to correct sector.
  mesh->GetMovable()->SetPosition(csVector3(0.0f));
  mesh->GetMovable()->SetSector(sector);
  mesh->GetMovable()->UpdateMove();
}

csImposterMesh::~csImposterMesh ()
{
  for(size_t i=0; i<instances.GetSize(); ++i)
  {
    DestroyInstance(instances[i]);
  }

  impman->Unregister(this);

  mesh->GetMovable()->SetSector(0);
  mesh->GetMovable()->UpdateMove();
  delete mesh;
}

bool csImposterMesh::Update(iMeshWrapper* mesh, iRenderView* rview)
{
  for(size_t i=0; i<instances.GetSize(); ++i)
  {
    if(instances[i]->mesh == mesh)
    {
      if(!WithinTolerance(rview, mesh))
      {
        DestroyInstance(instances[i]);
        instances.DeleteIndexFast(i);
        return false;
      }

      return true;
    }
  }

  if(WithinTolerance(rview, mesh))
  {
    CreateInstance(mesh);
    return true;
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
}

void csImposterMesh::DestroyInstance(Instance* instance)
{
  size_t idx = transformVars->FindArrayElement(instance->transformVar); 
  CS_ASSERT(idx != csArrayItemNotFound);

  transformVars->RemoveFromArray(idx);
  delete instance;
}

void csImposterMesh::InitMesh(iCamera* camera)
{
  // Save camera orientation
  const csOrthoTransform oldt = camera->GetTransform ();

  // Look at mesh
  csVector3 meshcenter = instances[0]->mesh->GetWorldBoundingBox().GetCenter();
  csVector3 campos = camera->GetTransform ().GetOrigin ();
  camera->GetTransform ().LookAt(meshcenter-campos, camera->GetTransform().GetT2O().Col2());

  // Get screen bounding box
  csScreenBoxResult res = instances[0]->mesh->GetScreenBoundingBox (camera);

  // Calculate height and width of the imposter on screen
  height = (res.sbox.GetCorner (1) - res.sbox.GetCorner (0)).y;
  width = (res.sbox.GetCorner (2) - res.sbox.GetCorner (0)).x;

  // Project screen bounding box, at the returned depth to
  //  the camera transform to rotate it around where we need it
  float middle = (res.cbox.MinZ () + res.cbox.MaxZ ()) / 2;

  csVector3 v1 = camera->InvPerspective (res.sbox.GetCorner (0), middle);
  csVector3 v2 = camera->InvPerspective (res.sbox.GetCorner (1), middle);
  csVector3 v3 = camera->InvPerspective (res.sbox.GetCorner (3), middle);
  csVector3 v4 = camera->InvPerspective (res.sbox.GetCorner (2), middle);

  // Convert to object space vertex positions.
  csReversibleTransform w2c = camera->GetTransform ();
  csReversibleTransform o2w = instances[0]->mesh->GetMovable ()->GetFullTransform ();
  cutout[0] = o2w.Other2This (w2c.This2Other (v1));
  cutout[1] = o2w.Other2This (w2c.This2Other (v2));
  cutout[2] = o2w.Other2This (w2c.This2Other (v3));
  cutout[3] = o2w.Other2This (w2c.This2Other (v4));

  // Calculate texture resolution.
  /*
  float den = 2.0f*res.distance*tanf(camera->GetFOV()/2.0f);
  int oTexHeight = g3d->GetHeight()*height/den;
  int oTexWidth = g3d->GetWidth()*width/den;
  texHeight = csFindNearestPowerOf2(oTexHeight);
  texWidth = csFindNearestPowerOf2(oTexWidth);

  if((oTexHeight - (texHeight >> 1)) < (texHeight - oTexHeight))
    texHeight >>= 1;
  if((oTexWidth - (texWidth >> 1)) < (texWidth - oTexWidth))
    texWidth >>= 1;
    */
  texHeight = 256;
  texWidth = 256;

  // Revert camera changes
  camera->SetTransform (oldt);

  // Save current facing for angle checking
  csReversibleTransform objt = instances[0]->mesh->GetMovable()->GetFullTransform();
  csOrthoTransform& camt = camera->GetTransform();

  csVector3 relativeDir = (instances[0]->mesh->GetWorldBoundingBox().GetCenter()
    - camt.GetOrigin()).Unit();

  meshLocalDir = objt.Other2ThisRelative(relativeDir);
  cameraLocalDir = camt.Other2ThisRelative(relativeDir);

  dirty = true;
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

  csDirtyAccessArray<uint>& mesh_indices = *GetMeshIndices ();
  csDirtyAccessArray<csVector2>& mesh_texels = *GetMeshTexels ();
  csDirtyAccessArray<csVector4>& mesh_colors = *GetMeshColors ();

  if (rmCreated)
  {
    //Initialize mesh
    mesh->meshtype = CS_MESHTYPE_TRIANGLES;
    mesh->do_mirror = rview->GetCamera ()->IsMirrored ();
    mesh->object2world = csReversibleTransform ();
    mesh->worldspace_origin = csVector3 (0,0,0);
    mesh->mixmode = CS_FX_ALPHA;
    mesh->alphaType = csAlphaMode::alphaBinary;
    mesh->z_buf_mode = CS_ZBUF_TEST;

    iTextureWrapper* tex = engine->CreateBlackTexture("impostertex", 256, 512, 0, CS_TEXTURE_3D);
    tex->Register(engine->G3D->GetTextureManager());
    mat = engine->CreateMaterial("impostermat", tex);

    csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet>(
    engine->GetObjectRegistry(), "crystalspace.shared.stringset");
    csStringID shadertype = strings->Request("base");
    csRef<iShaderManager> shman = csQueryRegistry<iShaderManager>(engine->objectRegistry);
    iShader* shader = shman->GetShader("lighting_default_instance");
    mat->GetMaterial()->SetShader(shadertype, shader);

    mesh->material = mat;

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
  }

  //mesh changed
  if (dirty)
  {
    GetMeshVertices ()->Empty ();
    GetMeshTexels ()->Empty ();

    float x = 1;
    float y = 1;

    // correct textels for imposter heigth/width ratio
    // since r2t texture is square, but billboard might not
    if (height > width)
    {
      x -= (1 - width/height)/2;
    } else {
      y -= (1 - height/width)/2;
    }

    mesh_texels.Push (csVector2 (1-x,y));  //0 1
    mesh_texels.Push (csVector2 (1-x,1-y));  //0 0
    mesh_texels.Push (csVector2 (x,1-y));  //1 0
    mesh_texels.Push (csVector2 (x,y));    //1 1

    csRef<csRenderBuffer> vertBuffer = csRenderBuffer::CreateRenderBuffer(
      4, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
    vertBuffer->CopyInto (cutout.GetVertices (), 4);

    csRef<csRenderBuffer> texBuffer = csRenderBuffer::CreateRenderBuffer(
      4, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 2);
    texBuffer->CopyInto (mesh_texels.GetArray(), 4);

    csDirtyAccessArray<csVector3> normals;
    normals.Push(csVector3(0, 0, -1));
    normals.Push(csVector3(0, 0, -1));
    normals.Push(csVector3(0, 0, -1));
    normals.Push(csVector3(0, 0, -1));

    csRef<csRenderBuffer> normalBuffer = csRenderBuffer::CreateRenderBuffer(
      4, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 2);
    normalBuffer->CopyInto (normals.GetArray(), 4);

    mesh->buffers->SetRenderBuffer (CS_BUFFER_POSITION, vertBuffer);
    mesh->buffers->SetRenderBuffer (CS_BUFFER_TEXCOORD0, texBuffer);
    mesh->buffers->SetRenderBuffer (CS_BUFFER_NORMAL, normalBuffer);

    dirty = false;
  }

  num = 1;
  return &mesh;
}
