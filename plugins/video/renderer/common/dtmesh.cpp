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

#include "sysdef.h"
#include "igraph3d.h"
#include "itexture.h"
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

static void G3DPreparePolygonFX (G3DPolygonDPFX* g3dpoly, csVector2* clipped_verts,
	int num_vertices, csVector2* orig_triangle, bool use_fog, bool gouraud)
{
  // Note: Assumes clockwise vertices, otherwise wouldn't be visible :).
  // 'was_clipped' will be true if the triangle was clipped.
  // This is the case if rescount != 3 (because we then don't have
  // a triangle) or else if any of the clipped vertices is different.
  bool was_clipped = (num_vertices != 3);
  int j;
  for (j = 0; j < num_vertices; j++)
  {
    g3dpoly->vertices [j].sx = clipped_verts [j].x;
    g3dpoly->vertices [j].sy = clipped_verts [j].y;
    if (!was_clipped && clipped_verts[j] != orig_triangle[j])
    	was_clipped = true;
  }

  // If it was not clipped we don't have to do anything.
  if (!was_clipped) return;

  // first we copy the first three texture coordinates to a local buffer
  // to avoid that they are overwritten when interpolating.
  G3DTexturedVertex tritexcoords[3];
  G3DFogInfo trifoginfo[3];
  for (int i = 0; i < 3; i++)
  {
    tritexcoords [i] = g3dpoly->vertices [i];
    trifoginfo [i] = g3dpoly->fog_info [i];
  }

  // Now we have to find the u,v coordinates for every
  // point in the clipped polygon. We know we started
  // from orig_triangle and that texture mapping is not perspective correct.

  // Compute U & V in vertices of the polygon
  // First find the topmost triangle vertex
  int top;
  if (orig_triangle [0].y < orig_triangle [1].y)
    if (orig_triangle [0].y < orig_triangle [2].y)
      top = 0;
    else
      top = 2;
  else
    if (orig_triangle [1].y < orig_triangle [2].y)
      top = 1;
    else
      top = 2;

  int _vbl, _vbr;
  if (top <= 0) _vbl = 2; else _vbl = top - 1;
  if (top >= 2) _vbr = 0; else _vbr = top + 1;

  // Rare special case is when triangle edge on, vertices satisfy
  //  *--------->x     a == b && (a.y < c.y) && (a.x > c.x)
  //  |  *a,b          and is clipped at c, where orig_triangle[0]
  //  | /              can be either a, b or c. In other words when
  //  |/               the single vertex is not 'top' and clipped.
  // /|*c              
  //  |                The '-= EPSILON' for both left and right 
  //  y     fig. 2     is fairly arbitrary, this probably needs to be refined.

  if (orig_triangle[top] == orig_triangle[_vbl]) 
      orig_triangle[_vbl].x -= EPSILON;
  if (orig_triangle[top] == orig_triangle[_vbr]) 
      orig_triangle[_vbr].x -= EPSILON;

  for (j = 0 ; j < g3dpoly->num ; j++)
    {
    float x = g3dpoly->vertices [j].sx;
    float y = g3dpoly->vertices [j].sy;

    // Find the original triangle top/left/bottom/right vertices
    // between which the currently examined point is located.
    // There are two possible major cases:
    // A*       A*       When DrawPolygonFX works, it will switch
    //  |\       |\      start/final values and deltas ONCE (not more)
    //  | \      | \     per triangle.On the left pictures this happens
    //  |*X\     |  \    at the point B. This means we should "emulate"
    //  |   *B   |   *B  this switch bytaking different start/final values
    //  |  /     |*X/    for interpolation if examined point X is below
    //  | /      | /     the point B.
    //  |/       |/
    // C*       C*  Fig.1 :-)
    int vtl = top, vtr = top, vbl = _vbl, vbr = _vbr;
    int ry = QRound (y); 
    if (ry > QRound (orig_triangle [vbl].y))
    {
      vtl = vbl;
      if (--vbl < 0) vbl = 2;
    }
    else if (ry > QRound (orig_triangle [vbr].y))
    {
      vtr = vbr;
      if (++vbr > 2) vbr = 0;
    }

    // Now interpolate Z,U,V,R,G,B by Y
    float tL, tR, xL, xR, tX;
    if (QRound (orig_triangle [vbl].y) != QRound (orig_triangle [vtl].y))
      tL = (y - orig_triangle [vtl].y) / (orig_triangle [vbl].y - orig_triangle [vtl].y);
    else
	tL = (x - orig_triangle [vtl].x) / (orig_triangle [vbl].x - orig_triangle [vtl].x);
    if (QRound (orig_triangle [vbr].y) != QRound (orig_triangle [vtr].y))
      tR = (y - orig_triangle [vtr].y) / (orig_triangle [vbr].y - orig_triangle [vtr].y);
    else
	tR = (x - orig_triangle [vtr].x) / (orig_triangle [vbr].x - orig_triangle [vtr].x);

    xL = orig_triangle [vtl].x + tL * (orig_triangle [vbl].x - orig_triangle [vtl].x);
    xR = orig_triangle [vtr].x + tR * (orig_triangle [vbr].x - orig_triangle [vtr].x);
    tX = xR - xL;
    if (tX) tX = (x - xL) / tX;

#   define INTERPOLATE(val,tl,bl,tr,br)	\
    {					\
      float vl,vr;				\
      if (tl != bl)				\
        vl = tl + (bl - tl) * tL;		\
      else					\
        vl = tl;				\
      if (tr != br)				\
        vr = tr + (br - tr) * tR;		\
      else					\
        vr = tr;				\
      val = vl + (vr - vl) * tX;		\
    }

    // Calculate Z
    INTERPOLATE(g3dpoly->vertices [j].z,
                tritexcoords [vtl].z, tritexcoords [vbl].z,
                tritexcoords [vtr].z, tritexcoords [vbr].z);
    if (g3dpoly->txt_handle)
    {
      // Calculate U
      INTERPOLATE(g3dpoly->vertices [j].u,
                  tritexcoords [vtl].u, tritexcoords [vbl].u,
                  tritexcoords [vtr].u, tritexcoords [vbr].u);
      // Calculate V
      INTERPOLATE(g3dpoly->vertices [j].v,
                  tritexcoords [vtl].v, tritexcoords [vbl].v,
                  tritexcoords [vtr].v, tritexcoords [vbr].v);
    }
    if (use_fog)
    {
      // Calculate R
      INTERPOLATE(g3dpoly->fog_info [j].r,
      		  trifoginfo [vtl].r, trifoginfo [vbl].r,
      		  trifoginfo [vtr].r, trifoginfo [vbr].r);
      // Calculate G
      INTERPOLATE(g3dpoly->fog_info [j].g,
      		  trifoginfo [vtl].g, trifoginfo [vbl].g,
      		  trifoginfo [vtr].g, trifoginfo [vbr].g);
      // Calculate B
      INTERPOLATE(g3dpoly->fog_info [j].b,
      		  trifoginfo [vtl].b, trifoginfo [vbl].b,
      		  trifoginfo [vtr].b, trifoginfo [vbr].b);
      // Calculate intensity
      INTERPOLATE(g3dpoly->fog_info [j].intensity,
      		  trifoginfo [vtl].intensity, trifoginfo [vbl].intensity,
      		  trifoginfo [vtr].intensity, trifoginfo [vbr].intensity);
    }
    if (gouraud)
    {
      // Calculate R
      INTERPOLATE(g3dpoly->vertices [j].r,
                  tritexcoords [vtl].r, tritexcoords [vbl].r,
                  tritexcoords [vtr].r, tritexcoords [vbr].r);
      // Calculate G
      INTERPOLATE (g3dpoly->vertices [j].g,
                  tritexcoords [vtl].g, tritexcoords [vbl].g,
                  tritexcoords [vtr].g, tritexcoords [vbr].g);
      // Calculate B
      INTERPOLATE (g3dpoly->vertices [j].b,
                  tritexcoords [vtl].b, tritexcoords [vbl].b,
                  tritexcoords [vtr].b, tritexcoords [vbr].b);
    }
    else
    {
    
      g3dpoly->vertices[j].r = 1;
      g3dpoly->vertices[j].g = 1;
      g3dpoly->vertices[j].b = 1;
      
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
  csTransform& o2c, csClipper* clipper, float aspect, int width2, int height2)
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
  csColor* col1 = mesh.vertex_colors[0];
  csVector3* work_verts;
  csVector2* work_uv_verts;
  csColor* work_colors;
  if (mesh.num_vertices_pool > 1)
  {
    // Vertex morphing.
    float tween_ratio = mesh.morph_factor;
    float remainder = 1 - tween_ratio;
    csVector3* f2 = mesh.vertices[1];
    csVector2* uv2 = mesh.texels[1][0];
    csColor* col2 = mesh.vertex_colors[1];
    if (mesh.vertex_mode == G3DTriangleMesh::VM_WORLDSPACE)
      for (i = 0 ; i < mesh.num_vertices ; i++)
      {
        tr_verts[i] = o2c * (tween_ratio * f2[i] + remainder * f1[i]);
	if (mesh.do_morph_texels)
	  uv_verts[i] = tween_ratio * uv2[i] + remainder * uv1[i];
	if (mesh.do_morph_colors)
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
	if (mesh.do_morph_colors)
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
  mesh.txt_handle[0]->GetMeanColor (poly.flat_color_r,
    poly.flat_color_g, poly.flat_color_b);

  poly.use_fog = mesh.do_fog;

  // The triangle in question
  csVector2 triangle [3];
  csVector2 clipped_triangle [10];	//@@@BAD HARCODED!

  g3d->StartPolygonFX (mesh.txt_handle[0], mesh.fxmode);

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
        if (!clipper->Clip (triangle, 3, clipped_triangle, rescount)) continue;
        poly.num = rescount;
      }
      else
        poly.num = 3;

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
      if (mesh.do_clip)
	G3DPreparePolygonFX (&poly, clipped_triangle, rescount,
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

//------------------------------------------------------------------------
