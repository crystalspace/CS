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
#include "plugins/video/renderer/common/polybuf.h"

//------------------------------------------------------------------------
// Everything for mesh drawing.
// This is a general DrawPolygonMesh. Only use this in your
// 3D rasterizer if you can't do it better :-)
//------------------------------------------------------------------------

//@@@@@@@ DO INCREF()/DECREF() ON THESE ARRAYS!!!
/// Static vertex array.
static CS_DECLARE_GROWING_ARRAY (tr_verts, csVector3);
/// The perspective corrected vertices.
static CS_DECLARE_GROWING_ARRAY (persp, csVector2);
/// Array which indicates which vertices are visible and which are not.
static CS_DECLARE_GROWING_ARRAY (visible, bool);

/*
 * Default implementation of DrawPolygonMesh which works with polygon
 * buffers that are equal to csPolArrayPolygonBuffer.
 * If 'clipper' == NULL then no clipping will happen.
 * If 'lazyclip' == true then only a lazy clip will happen (currently
 * not supported).
 */
void DefaultDrawPolygonMesh (G3DPolygonMesh& mesh, iGraphics3D *piG3D,
	const csReversibleTransform& o2c,
	iClipper2D* clipper, bool lazyclip, float aspect,
	int width2, int height2)
{
  csPolArrayPolygonBuffer* polbuf = (csPolArrayPolygonBuffer*)mesh.polybuf;
  int num_vertices = polbuf->GetVertexCount ();
  int num_polygons = polbuf->GetPolygonCount ();

  // Update work arrays
  if (num_vertices > tr_verts.Limit ())
  {
    tr_verts.SetLimit (num_vertices);
    persp.SetLimit (num_vertices);
    visible.SetLimit (num_vertices);
  }

  csVector3 *f1 = polbuf->GetVertices ();
  csVector3 *work_verts;
  int i;

  // Perform vertex transforms if necessary
  if (mesh.vertex_mode == G3DPolygonMesh::VM_WORLDSPACE)
  {
    for (i = 0 ; i < num_vertices ; i++)
      tr_verts[i] = o2c * f1[i];
    work_verts = tr_verts.GetArray();
  }
  else
    work_verts = f1;

  // Perspective project the vertices
  for (i = 0 ; i < num_vertices ; i++)
  {
    if (work_verts[i].z >= SMALL_Z)
    {
      float iz = aspect / work_verts[i].z;
      persp[i].x = work_verts[i].x * iz + width2;
      persp[i].y = work_verts[i].y * iz + height2;
      visible[i] = true;
    }
    else
      visible[i] = false;
  }

  csMatrix3 m_cam2tex;
  csVector3 v_cam2tex;

  G3DPolygonDP poly;
  poly.mixmode = mesh.mixmode;
  poly.use_fog = mesh.do_fog;
  poly.plane.m_cam2tex = &m_cam2tex;
  poly.plane.v_cam2tex = &v_cam2tex;

  // @@@ We need to restructure a bit here to avoid
  // unneeded copy of data.
  for (i = 0 ; i < num_polygons ; i++)
  {
    const csPolArrayPolygon& pol = polbuf->GetPolygon (i);
    poly.num = pol.num_vertices;
    CS_ASSERT (pol.mat_index >= 0 && pol.mat_index
    	< polbuf->GetMaterialCount ());
    poly.mat_handle = polbuf->GetMaterial (pol.mat_index);

    // Transform object to texture transform to
    // camera to texture transformation here.
    m_cam2tex = pol.m_obj2tex * o2c.GetT2O ();
    v_cam2tex = o2c.Other2This (pol.v_obj2tex);

    poly.poly_texture = pol.poly_texture;
    int j;
    // @@@ Support mirror here.
    // @@@ Check for visibility of three vertices.
    for (j = 0 ; j < poly.num ; j++)
    {
      int vidx = pol.vertices[j];
      poly.vertices[j].x = persp[vidx].x;
      poly.vertices[j].y = persp[vidx].y;
      if (mesh.vertex_fog)
        poly.fog_info[j] = mesh.vertex_fog[vidx];
    }

    // Clip polygon. Note that the clipper doesn't care about the
    // orientation of the polygon vertices. It works just as well in
    // mirrored mode.
    if (clipper)
    {
      csVector2 clipped_poly[100];
      int clipped_num;
      UByte clip_result = clipper->Clip (poly.vertices, poly.num,
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

