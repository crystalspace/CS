/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "ivideo/material.h"
#include "ivideo/vbufmgr.h"
#include "iengine/material.h"
#include "qint.h"
#include "csutil/garray.h"
#include "csutil/cscolor.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/transfrm.h"
#include "csgeom/polyclip.h"

//------------------------------------------------------------------------
// Everything for mesh drawing.
// This is a general DrawTriangleMesh. Only use this in your
// 3D rasterizer if you can't do it better :-)
//------------------------------------------------------------------------

#define INTERPOLATE1_S(var) \
  g3dpoly->var [i] = tritexcoords_##var [vt]+ \
    t * (tritexcoords_##var [vt2] - tritexcoords_##var [vt]);

#define INTERPOLATE1(var,component) \
  g3dpoly->var [i].component = tritexcoords_##var [vt].component + \
    t * (tritexcoords_##var [vt2].component - tritexcoords_##var [vt].component);

#define INTERPOLATE1_FOG(component) \
  g3dpoly->fog_info [i].component = trifoginfo [vt].component + \
    t * (trifoginfo [vt2].component - trifoginfo [vt].component);

#define INTERPOLATE_S(var) \
{ \
  float v1 = tritexcoords_##var [edge1 [0]] + \
    t1 * (tritexcoords_##var [edge1 [1]] - \
          tritexcoords_##var [edge1 [0]]); \
  float v2 = tritexcoords_##var [edge2 [0]] + \
    t2 * (tritexcoords_##var [edge2 [1]] - \
          tritexcoords_##var [edge2 [0]]); \
  g3dpoly->var [i] = v1 + t * (v2 - v1); \
}
#define INTERPOLATE(var,component) \
{ \
  float v1 = tritexcoords_##var [edge1 [0]].component + \
    t1 * (tritexcoords_##var [edge1 [1]].component - \
          tritexcoords_##var [edge1 [0]].component); \
  float v2 = tritexcoords_##var [edge2 [0]].component + \
    t2 * (tritexcoords_##var [edge2 [1]].component - \
          tritexcoords_##var [edge2 [0]].component); \
  g3dpoly->var [i].component = v1 + t * (v2 - v1); \
}
#define INTERPOLATE_FOG(component) \
{ \
  float v1 = trifoginfo [edge1 [0]].component + \
    t1 * (trifoginfo [edge1 [1]].component - \
          trifoginfo [edge1 [0]].component); \
  float v2 = trifoginfo [edge2 [0]].component + \
    t2 * (trifoginfo [edge2 [1]].component - \
          trifoginfo [edge2 [0]].component); \
  g3dpoly->fog_info [i].component = v1 + t * (v2 - v1); \
}

static void G3DPreparePolygonFX (G3DPolygonDPFX* g3dpoly,
  csVector2* clipped_verts, int num_vertices,
  csVertexStatus* clipped_vtstats, csVector2* orig_triangle,
  bool use_fog, bool gouraud)
{
  // first we copy the first three texture coordinates to a local buffer
  // to avoid that they are overwritten when interpolating.
  csVector2 tritexcoords_vertices[3];
  csVector2 tritexcoords_texels[3];
  csColor tritexcoords_colors[3];
  float tritexcoords_z[3];
  G3DFogInfo trifoginfo [3];
  int i;
  memcpy (tritexcoords_vertices, g3dpoly->vertices, 3 * sizeof (csVector2));
  memcpy (tritexcoords_texels, g3dpoly->texels, 3 * sizeof (csVector2));
  memcpy (tritexcoords_colors, g3dpoly->colors, 3 * sizeof (csColor));
  memcpy (tritexcoords_z, g3dpoly->z, 3 * sizeof (float));

  int vt, vt2;
  float t;
  for (i = 0 ; i < num_vertices ; i++)
  {
    g3dpoly->vertices[i] = clipped_verts[i];
    switch (clipped_vtstats[i].Type)
    {
      case CS_VERTEX_ORIGINAL:
        vt = clipped_vtstats[i].Vertex;
        g3dpoly->z[i] = tritexcoords_z[vt];
        g3dpoly->texels[i] = tritexcoords_texels[vt];
	if (gouraud)
          g3dpoly->colors[i] = tritexcoords_colors[vt];
	if (use_fog) g3dpoly->fog_info[i] = trifoginfo[vt];
	break;
      case CS_VERTEX_ONEDGE:
        vt = clipped_vtstats[i].Vertex;
	vt2 = vt + 1; if (vt2 >= 3) vt2 = 0;
	t = clipped_vtstats[i].Pos;
	INTERPOLATE1_S (z);
	INTERPOLATE1 (texels,x);
	INTERPOLATE1 (texels,y);
	if (gouraud)
	{
	  INTERPOLATE1 (colors,red);
	  INTERPOLATE1 (colors,green);
	  INTERPOLATE1 (colors,blue);
	}
	if (use_fog)
	{
	  INTERPOLATE1_FOG (r);
	  INTERPOLATE1_FOG (g);
	  INTERPOLATE1_FOG (b);
	  INTERPOLATE1_FOG (intensity);
	}
	break;
      case CS_VERTEX_INSIDE:
        float x = clipped_verts [i].x;
        float y = clipped_verts [i].y;
        int edge1 [2], edge2 [2];
        if ((y >= orig_triangle [0].y && y <= orig_triangle [1].y) ||
	    (y <= orig_triangle [0].y && y >= orig_triangle [1].y))
	{
	  edge1[0] = 0;
	  edge1[1] = 1;
          if ((y >= orig_triangle [1].y && y <= orig_triangle [2].y) ||
	      (y <= orig_triangle [1].y && y >= orig_triangle [2].y))
	  {
	    edge2[0] = 1;
	    edge2[1] = 2;
	  }
	  else
	  {
	    edge2[0] = 0;
	    edge2[1] = 2;
	  }
	}
	else
	{
	  edge1[0] = 1;
	  edge1[1] = 2;
	  edge2[0] = 0;
	  edge2[1] = 2;
	}
	csVector2& A = orig_triangle [edge1 [0]];
	csVector2& B = orig_triangle [edge1 [1]];
	csVector2& C = orig_triangle [edge2 [0]];
	csVector2& D = orig_triangle [edge2 [1]];
	float t1 = (y - A.y) / (B.y - A.y);
	float t2 = (y - C.y) / (D.y - C.y);
	float x1 = A.x + t1 * (B.x - A.x);
	float x2 = C.x + t2 * (D.x - C.x);
	t = (x - x1) / (x2 - x1);
	INTERPOLATE_S (z);
	INTERPOLATE (texels,x);
	INTERPOLATE (texels,y);
	if (gouraud)
	{
	  INTERPOLATE (colors,red);
	  INTERPOLATE (colors,green);
	  INTERPOLATE (colors,blue);
	}
	if (use_fog)
	{
	  INTERPOLATE_FOG (r);
	  INTERPOLATE_FOG (g);
	  INTERPOLATE_FOG (b);
	  INTERPOLATE_FOG (intensity);
	}
	break;
    }
  }
}

//@@@@@@@ DO INCREF()/DECREF() ON THESE ARRAYS!!!
/// Static vertex array.
typedef csDirtyAccessArray<csVector3> dtmesh_tr_verts;
CS_IMPLEMENT_STATIC_VAR (Get_tr_verts, dtmesh_tr_verts, ())
/// Static z array.
typedef csDirtyAccessArray<float> dtmesh_z_verts;
CS_IMPLEMENT_STATIC_VAR (Get_z_verts, dtmesh_z_verts, ())
/// Static uv array.
typedef csDirtyAccessArray<csVector2> dtmesh_uv_verts;
CS_IMPLEMENT_STATIC_VAR (Get_uv_verts, dtmesh_uv_verts, ())
/// The perspective corrected vertices.
typedef csDirtyAccessArray<csVector2> dtmesh_persp;
CS_IMPLEMENT_STATIC_VAR (Get_persp, dtmesh_persp, ())
/// Array with colors.
typedef csDirtyAccessArray<csColor> dtmesh_color_verts;
CS_IMPLEMENT_STATIC_VAR (Get_color_verts, dtmesh_color_verts, ())

static dtmesh_tr_verts *tr_verts = 0;
static dtmesh_z_verts *z_verts = 0;
static dtmesh_uv_verts *uv_verts = 0;
static dtmesh_persp *persp = 0;
static dtmesh_color_verts *color_verts = 0;


static void DrawTriangle (
	iGraphics3D* g3d,
	iClipper2D* clipper,
	G3DTriangleMesh& mesh,
	G3DPolygonDPFX& poly,
	const csVector2& pa, const csVector2& pb, const csVector2& pc,
        int* trivert,
	float* z_verts,
	csVector2* uv_verts,
	csColor* colors,
	G3DFogInfo* fog)
{
  // The triangle in question
  csVector2 triangle[3];
  csVector2 clipped_triangle[MAX_OUTPUT_VERTICES];	//@@@BAD HARCODED!
  csVertexStatus clipped_vtstats[MAX_OUTPUT_VERTICES];
  uint8 clip_result;

  if (!tr_verts)
  {
    tr_verts = Get_tr_verts ();
    persp= Get_persp();
    color_verts = Get_color_verts ();
  }

  //-----
  // Do backface culling. Note that this depends on the
  // mirroring of the current view.
  //-----
  float area = csMath2::Area2 (pa, pb, pc);
  int j, idx, dir;
  if (!area) return;
  if (mesh.do_mirror)
  {
    if (area <= -SMALL_EPSILON) return;
    triangle[2] = pa; triangle[1] = pb; triangle[0] = pc;
    // Setup loop variables for later.
    idx = 2; dir = -1;
  }
  else
  {
    if (area >= SMALL_EPSILON) return;
    triangle[0] = pa; triangle[1] = pb; triangle[2] = pc;
    // Setup loop variables for later.
    idx = 0; dir = 1;
  }

  // Clip triangle. Note that the clipper doesn't care about the
  // orientation of the triangle vertices. It works just as well in
  // mirrored mode.
  int rescount = 0;
  if (clipper)
  {
    clip_result = clipper->Clip (triangle, 3, clipped_triangle, rescount,
		  clipped_vtstats);
    if (clip_result == CS_CLIP_OUTSIDE) return;
    poly.num = rescount;
  }
  else
  {
    clip_result = CS_CLIP_INSIDE;
    poly.num = 3;
  }

  // If mirroring we store the vertices in the other direction.
  for (j = 0; j < 3; j++)
  {
    poly.z [idx] = z_verts[trivert [j]];
    poly.texels [idx] = uv_verts[trivert [j]];
    if (colors)
    {
      poly.colors [idx] = colors[trivert[j]];
    }
    if (poly.use_fog) poly.fog_info [idx] = fog[trivert[j]];
    idx += dir;
  }
  if (clip_result != CS_CLIP_INSIDE)
    G3DPreparePolygonFX (&poly, clipped_triangle, rescount, clipped_vtstats,
		(csVector2 *)triangle, poly.use_fog, colors != 0);
  else
  {
    poly.vertices [0].x = triangle [0].x;
    poly.vertices [0].y = triangle [0].y;
    poly.vertices [1].x = triangle [1].x;
    poly.vertices [1].y = triangle [1].y;
    poly.vertices [2].x = triangle [2].x;
    poly.vertices [2].y = triangle [2].y;
  }
  g3d->DrawPolygonFX (poly);
}

void DefaultDrawTriangleMesh (G3DTriangleMesh& mesh, iGraphics3D* g3d,
  csReversibleTransform& o2c, iClipper2D* clipper, bool lazyclip, float aspect,
  int width2, int height2)
{
  size_t i;

  if (!z_verts)
  {
    tr_verts = Get_tr_verts ();
    z_verts = Get_z_verts ();
    uv_verts = Get_uv_verts ();
    persp= Get_persp();
    color_verts = Get_color_verts ();
  }

#ifdef CS_DEBUG
  // Check if the vertex buffers are locked.
  CS_ASSERT (mesh.buffers[0]->IsLocked ());
  if (mesh.num_vertices_pool > 1)
  {
    CS_ASSERT (mesh.buffers[1]->IsLocked ());
  }
#endif

  // @@@ Currently we don't implement multi-texture
  // in the generic implementation. This is a todo...
  size_t num_vertices = mesh.buffers[0]->GetVertexCount ();

  // Update work tables.
  if (num_vertices > tr_verts->Capacity ())
  {
    tr_verts->SetCapacity (num_vertices);
    z_verts->SetCapacity (num_vertices);
    uv_verts->SetCapacity (num_vertices);
    persp->SetCapacity (num_vertices);
    color_verts->SetCapacity (num_vertices);
  }

  // Do vertex tweening and/or transformation to camera space
  // if any of those are needed. When this is done 'verts' will
  // point to an array of camera vertices.
  csVector3* f1 = mesh.buffers[0]->GetVertices ();
  csVector2* uv1 = mesh.buffers[0]->GetTexels ();
  csVector3* work_verts;
  csVector2* work_uv_verts;
  csColor* work_col;
  csColor* col1 = 0;
  if (mesh.use_vertex_color)
    col1 = mesh.buffers[0]->GetColors ();

  if (mesh.num_vertices_pool > 1)
  {
    // Vertex morphing.
    float tween_ratio = mesh.morph_factor;
    float remainder = 1 - tween_ratio;
    csVector3* f2 = mesh.buffers[1]->GetVertices ();
    csVector2* uv2 = mesh.buffers[1]->GetTexels ();
    csColor* col2 = 0;
    if (mesh.use_vertex_color)
      col2 = mesh.buffers[1]->GetColors ();
    if (mesh.vertex_mode == G3DTriangleMesh::VM_WORLDSPACE)
      for (i = 0 ; i < num_vertices ; i++)
      {
        (*tr_verts)[i] = o2c * (tween_ratio * f2[i] + remainder * f1[i]);
	if (mesh.do_morph_texels)
	  (*uv_verts)[i] = tween_ratio * uv2[i] + remainder * uv1[i];
	if (mesh.do_morph_colors && mesh.use_vertex_color)
	{
	  (*color_verts)[i].red = tween_ratio * col2[i].red
	  	+ remainder * col1[i].red;
	  (*color_verts)[i].green = tween_ratio * col2[i].green
	  	+ remainder * col1[i].green;
	  (*color_verts)[i].blue = tween_ratio * col2[i].blue
	  	+ remainder * col1[i].blue;
	}
      }
    else
      for (i = 0 ; i < num_vertices ; i++)
      {
        (*tr_verts)[i] = tween_ratio * f2[i] + remainder * f1[i];
	if (mesh.do_morph_texels)
	  (*uv_verts)[i] = tween_ratio * uv2[i] + remainder * uv1[i];
	if (mesh.do_morph_colors && mesh.use_vertex_color)
	{
	  (*color_verts)[i].red = tween_ratio * col2[i].red
	  	+ remainder * col1[i].red;
	  (*color_verts)[i].green = tween_ratio * col2[i].green
	  	+ remainder * col1[i].green;
	  (*color_verts)[i].blue = tween_ratio * col2[i].blue
	  	+ remainder * col1[i].blue;
	}
      }
    work_verts = tr_verts->GetArray ();
    if (mesh.do_morph_texels)
      work_uv_verts = uv_verts->GetArray ();
    else
      work_uv_verts = uv1;
    if (mesh.do_morph_colors)
      work_col = color_verts->GetArray ();
    else
      work_col = col1;
  }
  else
  {
    if (mesh.vertex_mode == G3DTriangleMesh::VM_WORLDSPACE)
    {
      for (i = 0 ; i < num_vertices ; i++)
        tr_verts->Put (i, o2c * f1[i]);
      work_verts = tr_verts->GetArray ();
    }
    else
      work_verts = f1;
    work_uv_verts = uv1;
    work_col = col1;
  }

  // Perspective project.
  for (i = 0 ; i < num_vertices ; i++)
  {
    if (work_verts[i].z >= SMALL_Z)
    {
      z_verts->Put (i, 1. / work_verts[i].z);
      float iz = aspect * (*z_verts)[i];
      csVector2& p = persp->GetExtend (i);
      p.x = work_verts[i].x * iz + width2;
      p.y = work_verts[i].y * iz + height2;
    }
  }

  // Clipped polygon (assume it cannot have more than 64 vertices)
  G3DPolygonDPFX poly;
  memset (&poly, 0, sizeof(poly));
  poly.mat_handle = mesh.mat_handle;
  poly.mixmode = mesh.mixmode;

  // Fill flat color if renderer decide to paint it flat-shaded
  iTextureHandle* txt_handle = mesh.mat_handle->GetTexture ();
  if (txt_handle)
    txt_handle->GetMeanColor (poly.flat_color_r,
      poly.flat_color_g, poly.flat_color_b);

  poly.use_fog = mesh.do_fog;

  // Draw all triangles.
  csTriangle* triangles = mesh.triangles;
  for (i = 0 ; i < (size_t)mesh.num_triangles ; i++)
  {
    int a = triangles[i].a;
    int b = triangles[i].b;
    int c = triangles[i].c;
    int cnt_vis = int (work_verts[a].z >= SMALL_Z) +
    		  int (work_verts[b].z >= SMALL_Z) +
    		  int (work_verts[c].z >= SMALL_Z);
    if (cnt_vis == 0)
    {
      //=====
      // Easy case: the triangle is completely not visible.
      //=====
      continue;
    }
    else if (cnt_vis == 3 || lazyclip)
    {
      //=====
      // Another easy case: all vertices are visible or we are using
      // lazy clipping in which case we draw the triangle completely.
      //=====
      int trivert [3] = { a, b, c };
      DrawTriangle (g3d, clipper, mesh, poly,
      	(*persp)[a], (*persp)[b], (*persp)[c], trivert,
	z_verts->GetArray (), work_uv_verts, work_col, mesh.vertex_fog);
    }
    else if (cnt_vis == 1)
    {
      //=====
      // A reasonably complex case: one vertex is visible. We need
      // to clip to the Z-plane but fortunatelly this will result in
      // another triangle.
      //=====

      // The following com_iz is valid for all points on Z-plane.
      float com_zv = 1.0f / (SMALL_Z*10);
      float com_iz = aspect * (1.0f / (SMALL_Z*10));

      csVector3 va = work_verts[a];
      csVector3 vb = work_verts[b];
      csVector3 vc = work_verts[c];
      csVector3 v;
      csVector2 pa, pb, pc;
      float zv[3];
      csVector2 uv[3];
      csColor col[3];
      G3DFogInfo fog[3];
#undef COPYVT
#define COPYVT(id,idl,i) \
	p##idl = (*persp)[i]; zv[id] = (*z_verts)[i]; uv[id] = work_uv_verts[i]; \
	if (work_col) col[id] = work_col[i]; \
	if (poly.use_fog) fog[id] = mesh.vertex_fog[i];
#undef INTERPOL
#define INTERPOL(id,idl,i1,i2) \
	uv[id] = work_uv_verts[i1] + r*(work_uv_verts[i2]-work_uv_verts[i1]); \
        zv[id] = com_zv; \
	p##idl.x = v.x * com_iz + width2; p##idl.y = v.y * com_iz + width2; \
	if (work_col) \
	{ \
	  col[id].red = work_col[i1].red+r*(work_col[i2].red-work_col[i1].red); \
	  col[id].green = work_col[i1].green+r*(work_col[i2].green-work_col[i1].green); \
	  col[id].blue = work_col[i1].blue+r*(work_col[i2].blue-work_col[i1].blue); \
	} \
	if (poly.use_fog) \
	{ \
	  fog[id].r = mesh.vertex_fog[i1].r + r*(mesh.vertex_fog[i2].r-mesh.vertex_fog[i1].r); \
	  fog[id].g = mesh.vertex_fog[i1].g + r*(mesh.vertex_fog[i2].g-mesh.vertex_fog[i1].g); \
	  fog[id].b = mesh.vertex_fog[i1].b + r*(mesh.vertex_fog[i2].b-mesh.vertex_fog[i1].b); \
	  fog[id].intensity = mesh.vertex_fog[i1].intensity + r*(mesh.vertex_fog[i2].intensity-mesh.vertex_fog[i1].intensity); \
	}

      if (va.z >= SMALL_Z)
      {
	// Point a is visible.
	COPYVT(0,a,a)

	// Calculate intersection between a-b and Z=SMALL_Z*10.
	// p = a + r * (b-a) (parametric line equation between a and b).
	float r = (SMALL_Z*10-va.z)/(vb.z-va.z);
	v.Set (va.x + r * (vb.x-va.x), va.y + r * (vb.y-va.y), SMALL_Z*10);
	INTERPOL(1,b,a,b)
	// Calculate intersection between a-c and Z=SMALL_Z*10.
	r = (SMALL_Z*10-va.z)/(vc.z-va.z);
	v.Set (va.x + r * (vc.x-va.x), va.y + r * (vc.y-va.y), SMALL_Z*10);
	INTERPOL(2,c,a,c)
      }
      else if (vb.z >= SMALL_Z)
      {
	// Point b is visible.
	COPYVT(1,b,b)

	// Calculate intersection between b-a and Z=SMALL_Z*10.
	float r = (SMALL_Z*10-vb.z)/(va.z-vb.z);
	v.Set (vb.x + r * (va.x-vb.x), vb.y + r * (va.y-vb.y), SMALL_Z*10);
	INTERPOL(0,a,b,a)
	// Calculate intersection between b-c and Z=SMALL_Z*10.
	r = (SMALL_Z*10-vb.z)/(vc.z-vb.z);
	v.Set (vb.x + r * (vc.x-vb.x), vb.y + r * (vc.y-vb.y), SMALL_Z*10);
	INTERPOL(2,c,b,c)
      }
      else
      {
	// Point c is visible.
	COPYVT(2,c,c)

	// Calculate intersection between c-a and Z=SMALL_Z*10.
	float r = (SMALL_Z*10-vc.z)/(va.z-vc.z);
	v.Set (vc.x + r * (va.x-vc.x), vc.y + r * (va.y-vc.y), SMALL_Z*10);
	INTERPOL(0,a,c,a)
	// Calculate intersection between c-b and Z=SMALL_Z*10.
	r = (SMALL_Z*10-vc.z)/(vb.z-vc.z);
	v.Set (vc.x + r * (vb.x-vc.x), vc.y + r * (vb.y-vc.y), SMALL_Z*10);
	INTERPOL(1,b,c,b)
      }

      // Now pa, pb, pc will be a triangle that is completely visible.
      // uv[0..2] contains the texture coordinates.
      // zv[0..2] contains 1/z
      // col[0..2] contains color information
      // fog[0..2] contains fog information

      int trivert [3] = { 0, 1, 2 };
      DrawTriangle (g3d, clipper, mesh, poly,
      	pa, pb, pc, trivert,
	zv, uv, work_col ? col : 0, fog);
    }
    else
    {
      //=====
      // The most complicated case: two vertices are visible. In this
      // case clipping to the Z-plane does not result in a triangle.
      // So we have to triangulate.
      // We will triangulate to triangles a,b,c, and a,c,d.
      //=====

      // The following com_iz is valid for all points on Z-plane.
      float com_zv = 1.0f / (SMALL_Z*10);
      float com_iz = aspect * (1.0f / (SMALL_Z*10));

      csVector3 va = work_verts[a];
      csVector3 vb = work_verts[b];
      csVector3 vc = work_verts[c];
      csVector3 v;
      csVector2 pa, pb, pc, pd;
      float zv[4];
      csVector2 uv[4];
      csColor col[4];
      G3DFogInfo fog[4];
      if (va.z < SMALL_Z)
      {
	// Point a is not visible.
	COPYVT(1,b,b)
	COPYVT(2,c,c)

	// Calculate intersection between a-b and Z=SMALL_Z*10.
	// p = a + r * (b-a) (parametric line equation between a and b).
	float r = (SMALL_Z*10-va.z)/(vb.z-va.z);
	v.Set (va.x + r * (vb.x-va.x), va.y + r * (vb.y-va.y), SMALL_Z*10);
	INTERPOL(0,a,a,b)
	// Calculate intersection between a-c and Z=SMALL_Z*10.
	r = (SMALL_Z*10-va.z)/(vc.z-va.z);
	v.Set (va.x + r * (vc.x-va.x), va.y + r * (vc.y-va.y), SMALL_Z*10);
	INTERPOL(3,d,a,c)
      }
      else if (vb.z < SMALL_Z)
      {
	// Point b is not visible.
	COPYVT(0,a,a)
	COPYVT(3,d,c)

	// Calculate intersection between b-a and Z=SMALL_Z*10.
	float r = (SMALL_Z*10-vb.z)/(va.z-vb.z);
	v.Set (vb.x + r * (va.x-vb.x), vb.y + r * (va.y-vb.y), SMALL_Z*10);
	INTERPOL(1,b,b,a)
	// Calculate intersection between b-c and Z=SMALL_Z*10.
	r = (SMALL_Z*10-vb.z)/(vc.z-vb.z);
	v.Set (vb.x + r * (vc.x-vb.x), vb.y + r * (vc.y-vb.y), SMALL_Z*10);
	INTERPOL(2,c,b,c)
      }
      else
      {
	// Point c is not visible.
	COPYVT(0,a,a)
	COPYVT(1,b,b)

	// Calculate intersection between c-a and Z=SMALL_Z*10.
	float r = (SMALL_Z*10-vc.z)/(va.z-vc.z);
	v.Set (vc.x + r * (va.x-vc.x), vc.y + r * (va.y-vc.y), SMALL_Z*10);
	INTERPOL(3,d,c,a)
	// Calculate intersection between c-b and Z=SMALL_Z*10.
	r = (SMALL_Z*10-vc.z)/(vb.z-vc.z);
	v.Set (vc.x + r * (vb.x-vc.x), vc.y + r * (vb.y-vc.y), SMALL_Z*10);
	INTERPOL(2,c,c,b)
      }

      // Now pa,pb,pc and pa,pc,pd will be triangles that are visible.
      // uv[0..3] contains the texture coordinates.
      // zv[0..3] contains 1/z
      // col[0..3] contains color information
      // fog[0..3] contains fog information

      int trivert1[3] = { 0, 1, 2 };
      DrawTriangle (g3d, clipper, mesh, poly,
      	pa, pb, pc, trivert1,
	zv, uv, work_col ? col : 0, fog);
      int trivert2[3] = { 0, 2, 3 };
      DrawTriangle (g3d, clipper, mesh, poly,
      	pa, pc, pd, trivert2,
	zv, uv, work_col ? col : 0, fog);
    }
  }
}
