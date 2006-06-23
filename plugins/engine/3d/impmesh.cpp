/*
    Copyright (C) 2002 by Keith Fulton and Jorrit Tyberghein

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

#include "impmesh.h"
#include "sector.h"
#include "meshobj.h"
#include "light.h"
#include "engine.h"

csImposterMesh::csImposterMesh (csEngine* engine, csMeshWrapper *parent)
{
  parent_mesh = parent;
  tex = new csImposterProcTex (engine, this);
  ready	= false;
  incidence_dist = 0;
}

float csImposterMesh::CalcIncidenceAngleDist (iRenderView *rview)
{
  // Jorrit, not sure about this correctness but it compiles at least. -Keith

  // Calculate angle of incidence vs. the camera
  iCamera* camera = rview->GetCamera ();
  csReversibleTransform obj = (parent_mesh->GetCsMovable()).csMovable::GetTransform ();
  csReversibleTransform cam = camera->GetTransform ();
  csReversibleTransform seg = obj / cam;  // Matrix Math Magic!
  csVector3 straight(0,0,1);
  csVector3 pt = seg * straight;
  return csSquaredDist::PointPoint (straight, pt);
}

bool csImposterMesh::CheckIncidenceAngle (iRenderView *rview, float tolerance)
{
  float const dist2 = CalcIncidenceAngleDist(rview);
  float diff = dist2 - incidence_dist;
  if (diff < 0) diff = -diff;

  // If not ok, mark for redraw of imposter
  if (diff > tolerance)
  {
    SetImposterReady (false);
    return false;
  }
  return true;
}

void csImposterMesh::FindImposterRectangle (const iCamera* camera)
{
  // Called from csImposterProcTex during Anim.
  //  (Recalc of texture causes recalc of imposter poly also.)

  // Rotate camera to look at object directly.
  // Get screen bounding box, modified to also return depth of
  //  point of max width or height in the box.
  // Rotate camera back to original lookat
  // Project screen bounding box, at the returned depth to
  //  the camera transform to rotate it around where we need it
  // Save as csPoly3d for later rendering

  res = parent_mesh->GetScreenBoundingBox (camera);

  csVector3 v1 (0,0,0);
  csVector3 v2 (100,0,0);
  csVector3 v3 (100,100,0);
  csVector3 v4 (0,100,0);
  
  cutout.AddVertex (v1);
  cutout.AddVertex (v2);
  cutout.AddVertex (v3);
  cutout.AddVertex (v4);
}


static bool mesh_init = false;

CS_IMPLEMENT_STATIC_VAR (GetMeshIndices, csDirtyAccessArray<uint>, ());
static size_t mesh_indices_count = 0;
CS_IMPLEMENT_STATIC_VAR (GetMeshVertices, 
  csDirtyAccessArray<csVector3>, ());
CS_IMPLEMENT_STATIC_VAR (GetMeshTexels, 
  csDirtyAccessArray<csVector2>, ());
CS_IMPLEMENT_STATIC_VAR (GetMeshColors, 
  csDirtyAccessArray<csVector4>, ());

csRenderMesh** csImposterMesh::GetRenderMesh(iRenderView *rview)
{
  bool rmCreated;
  csRenderMesh*& mesh = rmHolder.GetUnusedMesh (rmCreated,
    rview->GetCurrentFrameNumber ());

  if (rmCreated)
  {
    mesh_init = true;
    mesh->meshtype = CS_MESHTYPE_QUADS;
    mesh->mixmode = CS_FX_COPY;
    mesh->z_buf_mode = CS_ZBUF_FILL;
  }
  mesh_indices_count = 0;
  GetMeshVertices ()->Empty ();
  GetMeshTexels ()->Empty ();
  GetMeshColors ()->Empty ();

  //csMatrix3 m_cam2tex;
  //csVector3 v_cam2tex;
  //  poly.mat_handle = tex->GetMaterial ();
  //m_cam2tex = poly.m_obj2tex * o2c.GetT2O ();
  //v_cam2tex = o2c.Other2This (poly.v_obj2tex);
  // project screen bounding box into poly vertex list here
  //mesh.texture = tex->GetTextureWrapper ()->GetTextureHandle ();


  csDirtyAccessArray<uint>& mesh_indices = *GetMeshIndices ();
  csDirtyAccessArray<csVector3>& mesh_vertices = *GetMeshVertices ();
  csDirtyAccessArray<csVector2>& mesh_texels = *GetMeshTexels ();
  csDirtyAccessArray<csVector4>& mesh_colors = *GetMeshColors ();
  
  mesh_indices_count += 4;
  size_t i;
  for (i = mesh_indices.Length () ; i < mesh_indices_count ; i++)
  {
    mesh_indices.Put (i, i);
  }

  csVector3 v1 (0,0,0);
  csVector3 v2 (100,0,0);
  csVector3 v3 (100,100,0);
  csVector3 v4 (0,100,0);
  
  mesh_vertices.Push (v1);
  mesh_vertices.Push (v2);
  mesh_vertices.Push (v3);
  mesh_vertices.Push (v4);

  mesh_texels.Push (csVector2 (0,0));
  mesh_texels.Push (csVector2 (1,0));
  mesh_texels.Push (csVector2 (1,1));
  mesh_texels.Push (csVector2 (0,1));

  csVector4 c (1, 1, 1, 1.0);
  mesh_colors.Push (c);
  mesh_colors.Push (c);
  mesh_colors.Push (c);
  mesh_colors.Push (c);

//  mesh.

  return &mesh;
}
