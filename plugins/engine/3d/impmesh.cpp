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
#include "iengine/portal.h"
#include "csutil/debug.h"
#include "iengine/rview.h"
#include "ivideo/graph3d.h"
#include "csgfx/renderbuffer.h"

#include "material.h"
#include "impmesh.h"
#include "sector.h"
#include "meshobj.h"
#include "light.h"
#include "engine.h"

csImposterMesh::csImposterMesh (csEngine* engine, csMeshWrapper *parent)
{
  csImposterMesh::engine = engine;
  parent_mesh = parent;

  tex = new csImposterProcTex (engine, this);
  impostermat = engine->CreateMaterial ("Imposter", tex->GetTexture ());

  //@@@ Add four to initialise array because of bug in csPoly3D
  cutout.AddVertex(csVector3());
  cutout.AddVertex(csVector3());
  cutout.AddVertex(csVector3());
  cutout.AddVertex(csVector3());

  SetImposterReady(false);
  dirty = false;
  incidence_dist = 361;
}

csImposterMesh::~csImposterMesh ()
{
  delete tex;
}

float csImposterMesh::CalcIncidenceAngleDist (iCamera *cam)
{
  // Calculate angle of incidence vs. the camera
  csReversibleTransform objt = 
    (parent_mesh->GetCsMovable()).csMovable::GetTransform ();
  csReversibleTransform camt = cam->GetTransform ();
  //csReversibleTransform seg = objt / camt;  // Matrix Math Magic!
  csVector3 m2c = (objt.GetOrigin () - camt.GetOrigin ()).Unit ();
  csVector3 objf = objt.GetT2O().Col3();
  csVector3 camf = camt.GetT2O().Col3();
  float anglecam = acosf ( camf * m2c );
  float angleobj = acosf ( objf * m2c );
  float angle = fabsf(anglecam) + fabsf(angleobj);

  //printf("angle: %f %f %f\n",angleobj,anglecam, angle);
  //csVector3 pt = seg * straight;
  //csVector3 pt = (objforward * camforward).Unit();
  //return csSquaredDist::PointPoint (straight, pt);
  return angle;
}

bool csImposterMesh::CheckIncidenceAngle (iRenderView *rview, float tolerance)
{
  float const curdist = CalcIncidenceAngleDist(rview->GetCamera ());
  float diff = curdist - incidence_dist;
  if (diff < 0) diff = -diff;
  //printf("diff: %f %f %f\n",diff ,incidence_dist, curdist);

  // If not ok, mark for redraw of imposter
  if (diff > tolerance)
  {
    //printf("redraw\n!");
    SetImposterReady (false);
    return false;
  }
  return true;
}

void csImposterMesh::SetImposterReady (bool r)
{
  ready=r;
  if (!ready)
  {
//printf("request imposter update...\n");
    engine->imposterUpdateList.Push(
      csWeakRef<csImposterProcTex>(tex));
  }
}


void csImposterMesh::FindImposterRectangle (iCamera* c)
{
  // Called from csImposterProcTex during Anim.
  //  (Recalc of texture causes recalc of imposter poly also.)

  // Save camera orientation
  const csOrthoTransform oldt = c->GetTransform ();

  // Look at mesh
  csVector3 meshcenter = parent_mesh->GetWorldBoundingBox ().GetCenter ();
  csVector3 campos = c->GetTransform ().GetOrigin ();

  c->GetTransform ().LookAt (meshcenter - campos,
     c->GetTransform ().GetT2O ().Col2 ());

  // Get screen bounding box
  res = parent_mesh->GetScreenBoundingBox (c);

  //calculate height and width of the imposter on screen
  height = (res.sbox.GetCorner(1) - res.sbox.GetCorner(0)).y;
  width = (res.sbox.GetCorner(2) - res.sbox.GetCorner(0)).x;

  // Project screen bounding box, at the returned depth to
  //  the camera transform to rotate it around where we need it
  float middle = (res.cbox.MinZ () + res.cbox.MaxZ ()) / 2;

  csVector3 v1 = c->InvPerspective (res.sbox.GetCorner(0), middle);
  csVector3 v2 = c->InvPerspective (res.sbox.GetCorner(1), middle);
  csVector3 v3 = c->InvPerspective (res.sbox.GetCorner(3), middle);
  csVector3 v4 = c->InvPerspective (res.sbox.GetCorner(2), middle);
  
  //@@@ put these into w2o and save transform
  v1 = c->GetTransform ().This2Other (v1);
  v2 = c->GetTransform ().This2Other (v2);
  v3 = c->GetTransform ().This2Other (v3);
  v4 = c->GetTransform ().This2Other (v4);

  // Save as csPoly3d for later rendering
  cutout[0] = v1;
  cutout[1] = v2;
  cutout[2] = v3;
  cutout[3] = v4;

  // Revert camera changes
  c->SetTransform (oldt);

  // save current facing for angle checking
  incidence_dist = CalcIncidenceAngleDist (c);

  dirty = true;
}

//static arrays that keep the imposterdata
CS_IMPLEMENT_STATIC_VAR (GetMeshIndices, csDirtyAccessArray<uint>, ());
CS_IMPLEMENT_STATIC_VAR (GetMeshVertices, csDirtyAccessArray<csVector3>, ());
CS_IMPLEMENT_STATIC_VAR (GetMeshTexels, csDirtyAccessArray<csVector2>, ());
CS_IMPLEMENT_STATIC_VAR (GetMeshColors, csDirtyAccessArray<csVector4>, ());

csRenderMesh** csImposterMesh::GetRenderMesh(iRenderView *rview)
{
  //Get an unused mesh
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
    mesh->flipCulling = false;
    mesh->supports_pseudoinstancing = false;
    mesh->do_mirror = rview->GetCamera ()->IsMirrored ();
    mesh->object2world = csReversibleTransform ();
    mesh->worldspace_origin = csVector3(0,0,0);
    mesh->mixmode = CS_FX_ALPHA;
    mesh->z_buf_mode = CS_ZBUF_TEST;
    mesh->material = impostermat;

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
    indexBuffer->CopyInto(mesh_indices.GetArray(), 6);

    GetMeshColors ()->Empty ();

    csVector4 c (1, 1, 1, 1.0);
    mesh_colors.Push (c);
    mesh_colors.Push (c);
    mesh_colors.Push (c);
    mesh_colors.Push (c);

    csRef<csRenderBuffer> colBuffer = csRenderBuffer::CreateRenderBuffer(
      4, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 4);
    colBuffer->CopyInto(mesh_colors.GetArray(), 4);

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
    vertBuffer->CopyInto(cutout.GetVertices (), 4);

    csRef<csRenderBuffer> texBuffer = csRenderBuffer::CreateRenderBuffer(
      4, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 2);
    texBuffer->CopyInto(mesh_texels.GetArray(), 4);

    mesh->buffers->SetRenderBuffer (CS_BUFFER_POSITION, vertBuffer);
    mesh->buffers->SetRenderBuffer (CS_BUFFER_TEXCOORD0, texBuffer);
  }

  return &mesh;
}
