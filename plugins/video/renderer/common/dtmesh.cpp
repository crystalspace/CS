/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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
#include "ivideo/igraph3d.h"
#include "ivideo/itexture.h"
#include "iengine/itexture.h"
#include "ivideo/imater.h"
#include "iengine/imater.h"
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

#define INTERPOLATE1(component) \
  g3dpoly->vertices [i].##component## = tritexcoords [vt].##component## + \
    t * (tritexcoords [vt2].##component## - tritexcoords [vt].##component##);

#define INTERPOLATE1_FOG(component) \
  g3dpoly->fog_info [i].##component## = trifoginfo [vt].##component## + \
    t * (trifoginfo [vt2].##component## - trifoginfo [vt].##component##);

#define INTERPOLATE(component) \
{ \
  float v1 = tritexcoords [edge1 [0]].##component## + \
    t1 * (tritexcoords [edge1 [1]].##component## - \
          tritexcoords [edge1 [0]].##component##); \
  float v2 = tritexcoords [edge2 [0]].##component## + \
    t2 * (tritexcoords [edge2 [1]].##component## - \
          tritexcoords [edge2 [0]].##component##); \
  g3dpoly->vertices [i].##component## = v1 + t * (v2 - v1); \
}
#define INTERPOLATE_FOG(component) \
{ \
  float v1 = trifoginfo [edge1 [0]].##component## + \
    t1 * (trifoginfo [edge1 [1]].##component## - \
          trifoginfo [edge1 [0]].##component##); \
  float v2 = trifoginfo [edge2 [0]].##component## + \
    t2 * (trifoginfo [edge2 [1]].##component## - \
          trifoginfo [edge2 [0]].##component##); \
  g3dpoly->fog_info [i].##component## = v1 + t * (v2 - v1); \
}

static void G3DPreparePolygonFX (G3DPolygonDPFX* g3dpoly,
  csVector2* clipped_verts, int num_vertices,
  csVertexStatus* clipped_vtstats, csVector2* orig_triangle,
  bool use_fog, bool gouraud)
{
  // first we copy the first three texture coordinates to a local buffer
  // to avoid that they are overwritten when interpolating.
  G3DTexturedVertex tritexcoords [3];
  G3DFogInfo trifoginfo [3];
  int i;
  for (i = 0; i < 3; i++)
  {
    tritexcoords [i] = g3dpoly->vertices [i];
    trifoginfo [i] = g3dpoly->fog_info [i];
  }

  int vt, vt2;
  float t;
  for (i = 0 ; i < num_vertices ; i++)
  {
    g3dpoly->vertices[i].sx = clipped_verts[i].x;
    g3dpoly->vertices[i].sy = clipped_verts[i].y;
    switch (clipped_vtstats[i].Type)
    {
      case CS_VERTEX_ORIGINAL:
        vt = clipped_vtstats[i].Vertex;
        g3dpoly->vertices[i].z = tritexcoords[vt].z;
        g3dpoly->vertices[i].u = tritexcoords[vt].u;
        g3dpoly->vertices[i].v = tritexcoords[vt].v;
	if (gouraud)
	{
          g3dpoly->vertices[i].r = tritexcoords[vt].r;
          g3dpoly->vertices[i].g = tritexcoords[vt].g;
          g3dpoly->vertices[i].b = tritexcoords[vt].b;
	}
	if (use_fog) g3dpoly->fog_info[i] = trifoginfo[vt];
	break;
      case CS_VERTEX_ONEDGE:
        vt = clipped_vtstats[i].Vertex;
	vt2 = vt + 1; if (vt2 >= 3) vt2 = 0;
	t = clipped_vtstats[i].Pos;
	INTERPOLATE1 (z);
	INTERPOLATE1 (u);
	INTERPOLATE1 (v);
	if (gouraud)
	{
	  INTERPOLATE1 (r);
	  INTERPOLATE1 (g);
	  INTERPOLATE1 (b);
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
	INTERPOLATE (z);
	INTERPOLATE (u);
	INTERPOLATE (v);
	if (gouraud)
	{
	  INTERPOLATE (r);
	  INTERPOLATE (g);
	  INTERPOLATE (b);
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
static DECLARE_GROWING_ARRAY (tr_verts, csVector3);
/// Static z array.
static DECLARE_GROWING_ARRAY (z_verts, float);
/// Static uv array.
static DECLARE_GROWING_ARRAY (uv_verts, csVector2);
/// The perspective corrected vertices.
static DECLARE_GROWING_ARRAY (persp, csVector2);
/// Array which indicates which vertices are visible and which are not.
static DECLARE_GROWING_ARRAY (visible, bool);
/// Array with colors.
static DECLARE_GROWING_ARRAY (color_verts, csColor);

void DefaultDrawTriangleMesh (G3DTriangleMesh& mesh, iGraphics3D* g3d,
  csReversibleTransform& o2c, csClipper* clipper, float aspect, int width2, int height2)
{
  int i;

  // @@@ Currently we don't implement multi-texture
  // in the generic implementation. This is a todo...

  // Update work tables.
  if (mesh.num_vertices > tr_verts.Limit ())
  {
    tr_verts.SetLimit (mesh.num_vertices);
    z_verts.SetLimit (mesh.num_vertices);
    uv_verts.SetLimit (mesh.num_vertices);
    persp.SetLimit (mesh.num_vertices);
    visible.SetLimit (mesh.num_vertices);
    color_verts.SetLimit (mesh.num_vertices);
  }

  // Do vertex tweening and/or transformation to camera space
  // if any of those are needed. When this is done 'verts' will
  // point to an array of camera vertices.
  csVector3* f1 = mesh.vertices[0];
  csVector2* uv1 = mesh.texels[0][0];
  csVector3* work_verts;
  csVector2* work_uv_verts;
  csColor* work_colors;
  csColor* col1 = NULL;
  if (mesh.use_vertex_color)
    col1 = mesh.vertex_colors[0];

  if (mesh.num_vertices_pool > 1)
  {
    // Vertex morphing.
    float tween_ratio = mesh.morph_factor;
    float remainder = 1 - tween_ratio;
    csVector3* f2 = mesh.vertices[1];
    csVector2* uv2 = mesh.texels[1][0];
    csColor* col2 = NULL;
    if (mesh.use_vertex_color)
      col2 = mesh.vertex_colors[1];
    if (mesh.vertex_mode == G3DTriangleMesh::VM_WORLDSPACE)
      for (i = 0 ; i < mesh.num_vertices ; i++)
      {
        tr_verts[i] = o2c * (tween_ratio * f2[i] + remainder * f1[i]);
	if (mesh.do_morph_texels)
	  uv_verts[i] = tween_ratio * uv2[i] + remainder * uv1[i];
	if (mesh.do_morph_colors && mesh.use_vertex_color)
	{
	  color_verts[i].red = tween_ratio * col2[i].red + remainder * col1[i].red;
	  color_verts[i].green = tween_ratio * col2[i].green + remainder * col1[i].green;
	  color_verts[i].blue = tween_ratio * col2[i].blue + remainder * col1[i].blue;
	}
      }
    else
      for (i = 0 ; i < mesh.num_vertices ; i++)
      {
        tr_verts[i] = tween_ratio * f2[i] + remainder * f1[i];
	if (mesh.do_morph_texels)
	  uv_verts[i] = tween_ratio * uv2[i] + remainder * uv1[i];
	if (mesh.do_morph_colors && mesh.use_vertex_color)
	{
	  color_verts[i].red = tween_ratio * col2[i].red + remainder * col1[i].red;
	  color_verts[i].green = tween_ratio * col2[i].green + remainder * col1[i].green;
	  color_verts[i].blue = tween_ratio * col2[i].blue + remainder * col1[i].blue;
	}
      }
    work_verts = tr_verts.GetArray ();
    if (mesh.do_morph_texels)
      work_uv_verts = uv_verts.GetArray ();
    else
      work_uv_verts = uv1;
    if (mesh.do_morph_colors)
      work_colors = color_verts.GetArray ();
    else
      work_colors = col1;
  }
  else
  {
    if (mesh.vertex_mode == G3DTriangleMesh::VM_WORLDSPACE)
    {
      for (i = 0 ; i < mesh.num_vertices ; i++)
        tr_verts[i] = o2c * f1[i];
      work_verts = tr_verts.GetArray ();
    }
    else
      work_verts = f1;
    work_uv_verts = uv1;
    work_colors = col1;
  }

  // Perspective project.
  for (i = 0 ; i < mesh.num_vertices ; i++)
  {
    if (work_verts[i].z >= SMALL_Z)
    {
      z_verts[i] = 1. / work_verts[i].z;
      float iz = aspect * z_verts[i];
      persp[i].x = work_verts[i].x * iz + width2;
      persp[i].y = work_verts[i].y * iz + height2;
      visible[i] = true;
    }
    else
      visible[i] = false;
  }

  // Clipped polygon (assume it cannot have more than 64 vertices)
  G3DPolygonDPFX poly;
  memset (&poly, 0, sizeof(poly));

  // Fill flat color if renderer decide to paint it flat-shaded
  iTextureHandle* txt_handle = mesh.mat_handle[0]->GetTexture ();
  if (txt_handle)
    txt_handle->GetMeanColor (poly.flat_color_r,
      poly.flat_color_g, poly.flat_color_b);

  poly.use_fog = mesh.do_fog;

  // The triangle in question
  csVector2 triangle[3];
  csVector2 clipped_triangle[MAX_OUTPUT_VERTICES];	//@@@BAD HARCODED!
  csVertexStatus clipped_vtstats[MAX_OUTPUT_VERTICES];
  UByte clip_result;

  g3d->StartPolygonFX (mesh.mat_handle[0], mesh.fxmode);

  // Draw all triangles.
  csTriangle* triangles = mesh.triangles;
  for (i = 0 ; i < mesh.num_triangles ; i++)
  {
    int a = triangles[i].a;
    int b = triangles[i].b;
    int c = triangles[i].c;
    if (visible[a] && visible[b] && visible[c])
    {
      //-----
      // Do backface culling. Note that this depends on the
      // mirroring of the current view.
      //-----
      float area = csMath2::Area2 (persp [a], persp [b], persp [c]);
      int j, idx, dir;
      if (!area) continue;
      if (mesh.do_mirror)
      {
        if (area <= -SMALL_EPSILON) continue;
        triangle [2] = persp[a];
        triangle [1] = persp[b];
        triangle [0] = persp[c];
	// Setup loop variables for later.
        idx = 2;
	dir = -1;
      }
      else
      {
        if (area >= SMALL_EPSILON) continue;
        triangle [0] = persp[a];
        triangle [1] = persp[b];
        triangle [2] = persp[c];
	// Setup loop variables for later.
        idx = 0;
	dir = 1;
      }

      // Clip triangle. Note that the clipper doesn't care about the
      // orientation of the triangle vertices. It works just as well in
      // mirrored mode.
      int rescount = 0;
      if (mesh.do_clip && clipper)
      {
        clip_result = clipper->Clip (triangle, 3, clipped_triangle, rescount,
		clipped_vtstats);
        if (clip_result == CS_CLIP_OUTSIDE) continue;
        poly.num = rescount;
      }
      else
      {
        clip_result = CS_CLIP_INSIDE;
        poly.num = 3;
      }

      int trivert [3] = { a, b, c };
      // If mirroring we store the vertices in the other direction.
      for (j = 0; j < 3; j++)
      {
        poly.vertices [idx].z = z_verts[trivert [j]];
        poly.vertices [idx].u = work_uv_verts[trivert [j]].x;
        poly.vertices [idx].v = work_uv_verts[trivert [j]].y;
        if (work_colors)
        {
          poly.vertices [idx].r = work_colors[trivert[j]].red;
          poly.vertices [idx].g = work_colors[trivert[j]].green;
          poly.vertices [idx].b = work_colors[trivert[j]].blue;
        }
	if (poly.use_fog)
	  poly.fog_info [idx] = mesh.vertex_fog[trivert[j]];
	idx += dir;
      }
      if (clip_result != CS_CLIP_INSIDE)
	G3DPreparePolygonFX (&poly, clipped_triangle, rescount, clipped_vtstats,
		(csVector2 *)triangle, poly.use_fog, work_colors != NULL);
      else
      {
        poly.vertices [0].sx = triangle [0].x;
        poly.vertices [0].sy = triangle [0].y;
        poly.vertices [1].sx = triangle [1].x;
        poly.vertices [1].sy = triangle [1].y;
        poly.vertices [2].sx = triangle [2].x;
        poly.vertices [2].sy = triangle [2].y;
      }

      g3d->DrawPolygonFX (poly);
    }
  }

  g3d->FinishPolygonFX ();
}
