/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#include <math.h>
#include <stdarg.h>

#include "cssysdef.h"
#include "ogl_polybuf.h"
#include "csutil/util.h"
#include "csgeom/transfrm.h"
#include "qint.h"
#include "ivideo/material.h"
#include "ivideo/texture.h"
#include "ivideo/polyrender.h"
#include "ogl_txtcache.h"
#include "ogl_txtmgr.h"


TrianglesList::TrianglesList ()
{
  first = 0;
  last = 0;
}

TrianglesList::~TrianglesList ()
{
  while (first)
  {
    csTrianglesPerMaterial* aux = first->next;
    delete first;
    first = aux;
  }
}

void TrianglesList::Add (csTrianglesPerMaterial* t)
{
  if (first == 0)
  {
    first = last = t;
    return;
  }
  last->next = t;
  last = t;
}

//--------------------------------------------------------------------------

csTrianglesPerMaterial::csTrianglesPerMaterial()
{
  next = 0;
}

csTrianglesPerMaterial::~csTrianglesPerMaterial ()
{
  ClearVertexArray ();
}

void csTrianglesPerMaterial::ClearVertexArray ()
{
}

//--------------------------------------------------------------------------

csTriangleArrayPolygonBuffer::csTriangleArrayPolygonBuffer (
  iVertexBufferManager* mgr, csGraphics3DOGLCommon* G3D) : 
  csPolygonBuffer (mgr)
{
  matCount = 0;
  vertices = 0;
  csTriangleArrayPolygonBuffer::G3D = G3D;
}

csTriangleArrayPolygonBuffer::~csTriangleArrayPolygonBuffer ()
{
  Clear ();
}

void csTriangleArrayPolygonBuffer::Prepare ()
{
}

void csTriangleArrayPolygonBuffer::MarkLightmapsDirty()
{
//  superLM.MarkLightmapsDirty();
}

void csTriangleArrayPolygonBuffer::SetVertexArray (
  csVector3* verts, int num_verts)
{
  delete[] vertices;
  num_vertices = num_verts;
  vertices = new csVector3 [num_verts];
  memcpy (vertices, verts, num_verts * sizeof (csVector3));
  bbox.StartBoundingBox (vertices[0]);
  int i;
  for (i = 1 ; i < num_verts ; i++)
    bbox.AddBoundingVertexSmart (vertices[i]);
}

void csTriangleArrayPolygonBuffer::AddMaterial (iMaterialHandle* mat_handle)
{
  materials.Push (mat_handle);
  matCount ++;
}

void csTriangleArrayPolygonBuffer::SetMaterial (int idx,
  iMaterialHandle* mat_handle)
{
  materials[idx] = mat_handle;
}

void csTriangleArrayPolygonBuffer::Clear ()
{
  materials.DeleteAll ();
  delete[] vertices;
//  ClearLmQueue ();
}

void csTriangleArrayPolygonBuffer::AddPolygon (int num_verts,
	int* verts,
	csPolyTextureMapping* tmapping,
	const csPlane3& poly_normal,
	int mat_index,
	iRendererLightmap* lm)
{
  /*
   * We have to:
   * Generate triangles
   * Generate uv per vertex
   * Group the triangles per materials
   * We know this:
   * - The polygons are sent to AddPolygon sorted by material
   * - m_obj2tex and v_obj2tex are the matrix and vector to obtain the uv
   * cordinates
   *
   * We can do the following:
   * For every polygon:
   * if it is the first material add the triangles to the array
   *    and calculate the uv for it's vertices
   * if preceding material is the same that polygon's material then
   *    add the triangles to the preceding material array and calculate the
   *    uv
   * else add the triangles to the next position in the array and calculate
   *    their uv's
   *
   * IMPORTANT: poly_texture is stored per triangle? and normal per vertice?
   * it would be great!
   * - Can be done in load time?, it can, but does this affect to lighting
   * part? Ask Jorrit!
   * How can we this done? Every time an AddPolygon is done you know
   * the face normal, just add it to the vertex normal and normalize
   */

  csGLSuperLightmap* slm = 0;
  if (lm)
    slm = ((csGLRendererLightmap*)lm)->slm;

  csTrianglesPerMaterial* pol;
  int last_mat_index = polygons.GetLastMaterial ();
  iSuperLightmap* last_lmh = polygons.GetLastLMHandle ();
  if ((last_mat_index != mat_index) || (last_lmh != slm))
  {
    // First polygon or material of this polygon is different from
    // last material.
    pol = new csTrianglesPerMaterial ();
    polygons.Add (pol);
  }
  else
  {
    // We can add the triangles in the last PolygonPerMaterial
    // as long they share the same material.
    pol = polygons.last;
  }

  const csMatrix3& m_obj2tex =
    tmapping ? tmapping->GetO2T () : csMatrix3 (); // @@@
  const csVector3& v_obj2tex =
    tmapping ? tmapping->GetO2TTranslation () : csVector3 (); // @@@
  csTransform obj2tex (m_obj2tex, v_obj2tex);

  csTransform tex2lm;
  if (lm && slm)
  {
    struct csPolyLMCoords
    {
      float u1, v1, u2, v2;
    };

    csPolyLMCoords lmc;
    int lm_left, lm_top, lm_w, lm_h;
    lm->GetSLMCoords (lm_left, lm_top, lm_w, lm_h);
    G3D->txtmgr->GetLightmapRendererCoords (slm->w, slm->h,
      lm_left, lm_top, lm_left + lm_w, lm_top + lm_h,  
      lmc.u1, lmc.v1, lmc.u2, lmc.v2);
    
    float lm_low_u = 0.0f, lm_low_v = 0.0f;
    float lm_high_u = 1.0f, lm_high_v = 1.0f;
    if (tmapping)
      tmapping->GetTextureBox (lm_low_u, lm_low_v, lm_high_u, lm_high_v);

    float lm_scale_u = ((lmc.u2 - lmc.u1) / (lm_high_u - lm_low_u));
    float lm_scale_v = ((lmc.v2 - lmc.v1) / (lm_high_v - lm_low_v));

    tex2lm.SetO2T (
      csMatrix3 (lm_scale_u, 0, 0,
		  0, lm_scale_v, 0,
		  0, 0, 1));
    tex2lm.SetO2TTranslation (
      csVector3 (
      (lm_scale_u != 0.0f) ? (lm_low_u - lmc.u1 / lm_scale_u) : 0,
      (lm_scale_v != 0.0f) ? (lm_low_v - lmc.v1 / lm_scale_v) : 0,
      0));
  }

  int cur_vt_num = vec_vertices.Length ();
  int new_vt_num = cur_vt_num + num_verts;
  vec_vertices.SetLength (new_vt_num);
  texels.SetLength (new_vt_num);
  lumels.SetLength (new_vt_num);

  int cur_vt_idx = cur_vt_num;
  int i;
  csTriangle triangle, orig_tri;

  csVector3 tc;
  csVector3 lmc;

  vec_vertices.Put (cur_vt_idx, vertices[verts[0]]);
  tc = obj2tex.Other2This (vertices[verts[0]]);
  texels.Put (cur_vt_idx, csVector2 (tc.x, tc.y));
  lmc = tex2lm.Other2This (tc);
  lumels.Put (cur_vt_idx, csVector2 (lmc.x, lmc.y));
  triangle.a = cur_vt_idx++;
  orig_tri.a = verts[0];

  for (i = 1; i < num_verts - 1; i++)
  {
    vec_vertices.Put (cur_vt_idx, vertices[verts[i]]);
    tc = obj2tex.Other2This (vertices[verts[i]]);
    texels.Put (cur_vt_idx, csVector2 (tc.x, tc.y));
    lmc = tex2lm.Other2This (tc);
    lumels.Put (cur_vt_idx, csVector2 (lmc.x, lmc.y));
    triangle.b = cur_vt_idx++;
    orig_tri.b = verts[i];

    vec_vertices.Put (cur_vt_idx, vertices[verts[i+1]]);
    tc = obj2tex.Other2This (vertices[verts[i+1]]);
    texels.Put (cur_vt_idx, csVector2 (tc.x, tc.y));
    lmc = tex2lm.Other2This (tc);
    lumels.Put (cur_vt_idx, csVector2 (lmc.x, lmc.y));
    triangle.c = cur_vt_idx++;
    orig_tri.c = verts[i+1];

    pol->triangles.Push (triangle);
    orig_triangles.Push (orig_tri);
  }

  pol->matIndex = mat_index;
  pol->slmh = slm;
}

csTriangleArrayVertexBufferManager::csTriangleArrayVertexBufferManager
  (iObjectRegistry* object_reg, csGraphics3DOGLCommon* g3d) :
   csVertexBufferManager (object_reg)
{
  csTriangleArrayVertexBufferManager::g3d = g3d;
}

csTriangleArrayVertexBufferManager::~csTriangleArrayVertexBufferManager()
{
}

iPolygonBuffer* csTriangleArrayVertexBufferManager::CreatePolygonBuffer ()
{
  csTriangleArrayPolygonBuffer* buf = new csTriangleArrayPolygonBuffer (this,
    g3d);
  return buf;
}

