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
#include "csutil/garray.h"
#include "csgeom/math3d.h"
#include "csgeom/transfrm.h"
#include "csgeom/polyclip.h"
#include "ivideo/polyrender.h"
#include "plugins/video/renderer/common/polybuf.h"

//------------------------------------------------------------------------
// Everything for mesh drawing.
// This is a general DrawPolygonMesh. Only use this in your
// 3D rasterizer if you can't do it better :-)
//------------------------------------------------------------------------

//@@@@@@@ DO INCREF()/DECREF() ON THESE ARRAYS!!!
/// Static vertex array.
typedef csDirtyAccessArray<csVector3> dpmesh_tr_verts;
CS_IMPLEMENT_STATIC_VAR (Get_tr_verts, dpmesh_tr_verts, ())
/// The perspective corrected vertices.
typedef csDirtyAccessArray<csVector2> dpmesh_persp;
CS_IMPLEMENT_STATIC_VAR (Get_persp, dpmesh_persp, ())
/// Array which indicates which vertices are visible and which are not.
typedef csDirtyAccessArray<bool> dpmesh_visible;
CS_IMPLEMENT_STATIC_VAR (Get_visible, dpmesh_visible, ())

static dpmesh_tr_verts *tr_verts = 0;
static dpmesh_persp *persp = 0;
static dpmesh_visible *visible = 0;

static void PlaneZ (
  const csVector3 &u,
  const csVector3 &v,
  float z,
  csVector3 &isect)
{
  float denom;
  csVector3 vu = v - u;

  denom = vu.z;
  if (denom == 0)
  {
    // they are parallel
    isect = v;
    return;
  }
  float dist = -(u.z + (-z)) / denom;
  if (dist < -SMALL_EPSILON || dist > 1 + SMALL_EPSILON) return;

  isect = u + dist * vu;
}

static bool ClipPolygonZ (
  csVector3* pverts,
  csVector3* verts,
  int &num_verts)
{
  int i, i1, num_vertices = num_verts;
  bool zs, z1s;
  bool vis[100];

  float z = 0.01f;

  for (i = 0; i < num_vertices; i++)
  {
    vis[i] = pverts[i].z >= z;
  }

  // This routine assumes clipping is needed.
  num_verts = 0;

  i1 = num_vertices - 1;

  for (i = 0; i < num_vertices; i++)
  {
    zs = vis[i];
    z1s = vis[i1];

    if (!z1s && zs)
    {
      PlaneZ (pverts[i1], pverts[i], z, verts[num_verts]);
      num_verts++;
      verts[num_verts++] = pverts[i];
    }
    else if (z1s && !zs)
    {
      PlaneZ (pverts[i1], pverts[i], z, verts[num_verts]);
      num_verts++;
    }
    else if (z1s && zs)
    {
      verts[num_verts++] = pverts[i];
    }

    i1 = i;
  }

  return true;
}


/*
 * Default implementation of DrawPolygonMesh which works with polygon
 * buffers that are equal to csPolArrayPolygonBuffer.
 * If 'clipper' == 0 then no clipping will happen.
 * If 'lazyclip' == true then only a lazy clip will happen (currently
 * not supported).
 * WARNING: This version does NOT support vertex_fog. It is supposed
 * to be used with renderers that do fog another way (like with the
 * Z-buffer like the software renderer does).
 */
void DefaultDrawPolygonMesh (G3DPolygonMesh& mesh, iGraphics3D *piG3D,
	const csReversibleTransform& o2c,
	iClipper2D* clipper, bool /*lazyclip*/, float aspect,
	int width2, int height2)
{
  csPolArrayPolygonBuffer* polbuf = (csPolArrayPolygonBuffer*)mesh.polybuf;

  if (!tr_verts)
  {
    tr_verts = Get_tr_verts ();
    persp = Get_persp ();
    visible = Get_visible ();
  }

  int num_vertices = polbuf->GetVertexCount ();
  int num_polygons = polbuf->GetPolygonCount ();

  // Update work arrays
  if (num_vertices > tr_verts->Capacity ())
  {
    tr_verts->SetCapacity (num_vertices);
    persp->SetCapacity (num_vertices);
    visible->SetCapacity (num_vertices);
  }

  csVector3 *f1 = polbuf->GetVertices ();
  csVector3 *work_verts;
  int i;

  // Perform vertex transforms if necessary
  if (mesh.vertex_mode == G3DPolygonMesh::VM_WORLDSPACE)
  {
    for (i = 0 ; i < num_vertices ; i++)
      tr_verts->Put (i, o2c * f1[i]);
    work_verts = tr_verts->GetArray();
  }
  else
  {
    work_verts = f1;
  }

  // Perspective project the vertices
  for (i = 0 ; i < num_vertices ; i++)
  {
    if (work_verts[i].z >= SMALL_Z)
    {
      float iz = aspect / work_verts[i].z;
      csVector2& p = persp->GetExtend (i);
      p.x = work_verts[i].x * iz + width2;
      p.y = work_verts[i].y * iz + height2;
      visible->Put (i, true);
    }
    else
    {
      visible->Put (i, false);
    }
  }

  csMatrix3 m_cam2tex;
  csVector3 v_cam2tex;

  G3DPolygonDP poly;
  poly.mixmode = mesh.mixmode;
  poly.use_fog = mesh.do_fog;
  poly.do_fullbright = false;
  poly.cam2tex.m_cam2tex = &m_cam2tex;
  poly.cam2tex.v_cam2tex = &v_cam2tex;

  // @@@ We need to restructure a bit here to avoid
  // unneeded copy of data.
  for (i = 0 ; i < num_polygons ; i++)
  {
    const csPolArrayPolygon& pol = polbuf->GetPolygon (i);

    float cl = pol.normal.Classify (o2c.GetOrigin ());
    if ((mesh.do_mirror && cl <= 0) || ((!mesh.do_mirror) && cl >= 0)) continue;

    poly.num = pol.num_vertices;
    CS_ASSERT (pol.mat_index >= 0 && pol.mat_index
    	< polbuf->GetMaterialCount ());
    poly.mat_handle = polbuf->GetMaterial (pol.mat_index);
  
    poly.texmap = pol.texmap;
    poly.rlm = pol.rlm;

    // Transform object to texture transform to
    // camera to texture transformation here.
    csTransform t_obj2tex (pol.texmap->GetO2T (),
    	pol.texmap->GetO2TTranslation ());
    m_cam2tex = t_obj2tex.GetO2T () * o2c.GetT2O ();
    v_cam2tex = o2c.Other2This (t_obj2tex.GetO2TTranslation ());

    //poly.poly_texture = pol.poly_texture;
    int j;
    // @@@ Support mirror here.
    int vis_cnt = 0;
    for (j = 0 ; j < poly.num ; j++)
    {
      int vidx = pol.vertices[j];
      if ((*visible)[vidx]) vis_cnt++;
    }
    if (vis_cnt == 0) continue;
    if (vis_cnt < poly.num)
    {
      csVector3 p[100];
      csVector3 clip_p[100];
      // We need some kind of clipping.
      for (j = 0 ; j < poly.num ; j++)
      {
        int vidx = pol.vertices[j];
        p[j] = work_verts[vidx];
      }
      ClipPolygonZ (p, clip_p, poly.num);
      for (j = 0 ; j < poly.num ; j++)
      {
        float iz = aspect / clip_p[j].z;
        poly.vertices[j].x = clip_p[j].x * iz + width2;
        poly.vertices[j].y = clip_p[j].y * iz + height2;
      }
    }
    else
    {
      for (j = 0 ; j < poly.num ; j++)
      {
        int vidx = pol.vertices[j];
        poly.vertices[j].x = (*persp)[vidx].x;
        poly.vertices[j].y = (*persp)[vidx].y;
      }
    }


    // Clip polygon. Note that the clipper doesn't care about the
    // orientation of the polygon vertices. It works just as well in
    // mirrored mode.
    if (clipper)
    {
      csVector2 clipped_poly[100];
      int clipped_num;
      uint8 clip_result = clipper->Clip (poly.vertices, poly.num,
      	  clipped_poly, clipped_num);
      if (clip_result == CS_CLIP_OUTSIDE) continue;
      if (clip_result != CS_CLIP_INSIDE)
      {
        // We need to copy the clipped polygon.
	memcpy (poly.vertices, clipped_poly, sizeof (csVector2)*2*clipped_num);
        poly.num = clipped_num;
      }
    }

    csVector3& vertex0 = work_verts[pol.vertices[0]];
    poly.z_value = vertex0.z;
    o2c.Other2This (pol.normal, vertex0, poly.normal);

    // Draw the polygon
    piG3D->DrawPolygon (poly);
  }
}

