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
//#include "qint.h"
#include "csutil/garray.h"
//#include "csutil/cscolor.h"
//#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/transfrm.h"
#include "csgeom/polyclip.h"

//------------------------------------------------------------------------
// Everything for mesh drawing.
// This is a general DrawPolygonMesh. Only use this in your
// 3D rasterizer if you can't do it better :-)
//------------------------------------------------------------------------

//@@@@@@@ DO INCREF()/DECREF() ON THESE ARRAYS!!!
/// Static vertex array.
static DECLARE_GROWING_ARRAY (tr_verts, csVector3);
/// The perspective corrected vertices.
static DECLARE_GROWING_ARRAY (persp, csVector2);
/// Array which indicates which vertices are visible and which are not.
static DECLARE_GROWING_ARRAY (visible, bool);
/// Static z array.
static DECLARE_GROWING_ARRAY (z_verts, float);

/*
 * [01:00:34] <Jorrit> 1. Transform the vertices from world to camera space.
 * [01:00:40] <Jorrit> 2. Perspective project them.
 * [01:00:48] <Jorrit> 3. Call DrawPolygon for every polygon.
 *
 * [01:31:23] <Jorrit> As to the transform. Every graphics3D implementation holds a o2c transform.
 * [01:31:27] <Jorrit> (object to camera).
 * [01:31:47] <Jorrit> See how the soft_g3d.h DrawTriangleMesh gives o2c to DefaultDrawTriangleMesh.
 * [01:31:57] <Jorrit> You also need to give the clipper and so on.
 * [01:32:00] <link> ah I see
 * [01:32:08] <Jorrit> (initially you can ignore the clipper but you'll have to support it in the end).
 */
void DefaultDrawPolygonMesh(G3DPolygonMesh& mesh, iGraphics3D *piG3D, csTransform &o2c,
			    csClipper */*clipper*/, float aspect, int width2, int height2)
{

  // Update work arrays
  if(mesh.num_vertices > tr_verts.Limit()) {
    tr_verts.SetLimit(mesh.num_vertices);
    persp.SetLimit(mesh.num_vertices);
    visible.SetLimit(mesh.num_vertices);
  }

  csVector3 *f1 = &mesh.vertices[0];
  csVector3 *work_verts;
  int i;

  // Perform vertex transforms if necessary
  if (mesh.vertex_mode == G3DPolygonMesh::VM_WORLDSPACE) {
    for (i = 0 ; i < mesh.num_vertices ; i++)
      tr_verts[i] = o2c * f1[i];
    work_verts = tr_verts.GetArray();
  } else
    work_verts = f1;

  // Perspective project the vertices
  for(i = 0; i < mesh.num_vertices; i++) {
    if (work_verts[i].z >= SMALL_Z) {
      z_verts[i] = 1. / work_verts[i].z;
      float iz = aspect * z_verts[i];
      persp[i].x = work_verts[i].x * iz + width2;
      persp[i].y = work_verts[i].y * iz + height2;
      visible[i] = true;
    }
    else
      visible[i] = false;
  }

  for(i = 0; i < mesh.num_vertices; i++) {
    G3DPolygonDP poly;
    poly.num = mesh.polygons[i].vertices;
    poly.use_fog = mesh.do_fog;
    if(mesh.master_txt_handle)
      poly.txt_handle = mesh.master_txt_handle;
    else
      poly.txt_handle = &mesh.txt_handle[i];
    poly.plane = mesh.plane[i];
    poly.normal = mesh.normal[i];
    poly.z_value = z_verts[0];
    poly.inv_aspect = aspect * z_verts[0];  //@@@ Is this right?
#ifdef DO_HW_UVZ
    //csVector3* uvz; @@@ Where do I get this?
    poly.mirror = mesh.do_mirror;
#endif
    int j;
    for(j = 0; j < mesh.num_vertices; j++) {
      poly.vertices[j].sx = work_verts[j].x;
      poly.vertices[j].sy = work_verts[j].y;
      poly.fog_info[j] = mesh.vertex_fog[j];
    }

    //@@@ How to handle these?
    //iPolygonTexture* poly_texture;
    //int alpha;

    // Draw the polygon
    piG3D->DrawPolygon(poly);

  }

}
