/*
    Copyright (C) 2002 by Jorrit Tyberghein and Ryan Surkamp

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
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/polyclip.h"
#include "csgeom/poly2d.h"
#include "csgeom/vector3.h"
#include "csutil/garray.h"
#include "iengine/camera.h"
#include "iengine/engine.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivideo/vbufmgr.h"
#include "iengine/material.h"
#include "iengine/rview.h"
#include "ivideo/txtmgr.h"
#include "igraphic/image.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "imap/ldrctxt.h"
#include "csgfx/rgbpixel.h"
#include "bcterr.h"
#include "quadtree.h"
#include "qint.h"
#include "qsqrt.h"
#include "ivaria/reporter.h"
#include "igeom/polymesh.h"

CS_IMPLEMENT_PLUGIN


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Shared Functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

csVector3 BezierCompute ( float u, csVector3* temp)
{
  csVector3 pt;
  float u1, a, b ,c, d;
  u1 = 1.0f - u;
  a = u1 * u1 * u1;
  b = 3 * u1 * u1 * u;
  c = 3 * u1 * u * u;
  d = u * u * u;
  pt.x = (a * temp[0].x) +
    (b * temp[1].x) +
    (c * temp[2].x) +
    (d * temp[3].x);
  pt.y = (a * temp[0].y) +
    (b * temp[1].y) +
    (c * temp[2].y) +
    (d * temp[3].y);
  pt.z = (a * temp[0].z) +
    (b * temp[1].z) +
    (c * temp[2].z) +
    (d * temp[3].z);
  return pt;
}

/*
 * BezierControlCompute :
 * Basically creates a new point from a set of horizontal control points
 *
 */

csVector3 BezierControlCompute (float u, csVector3* cntrl, int width)
{
  csVector3 pt;
  float u1, a, b ,c, d;
  int  width2, width3;
  width2 = width * 2;
  width3 = width * 3;
  u1 = 1.0f - u;
  a = u1 * u1 * u1;
  b = 3 * u1 * u1 * u;
  c = 3 * u1 * u * u;
  d = u * u * u;
  pt.x = (a * cntrl[0].x) +
    (b * cntrl[width].x) +
    (c * cntrl[width2].x) +
    (d * cntrl[width3].x);
  pt.y = (a * cntrl[0].y) +
    (b * cntrl[width].y) +
    (c * cntrl[width2].y) +
    (d * cntrl[width3].y);
  pt.z = (a * cntrl[0].z) +
    (b * cntrl[width].z) +
    (c * cntrl[width2].z) +
    (d * cntrl[width3].z);
  return pt;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// csSharedLODMesh
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// creates arrays and sets free to true

void csSharedLODMesh::CreateMesh (int new_x_verts, int new_z_verts, int edge_res, unsigned int nlevel )
{
  int longest, num_tris;
  x_verts = new_x_verts;
  z_verts = new_z_verts;
  // free = true;
  num_verts = (x_verts * z_verts) + ( (edge_res + 2) * 4) ;
  normals = new csVector3[num_verts];
  color = new csColor[num_verts];
  texels = new csVector2[num_verts];
  verts = new csVector3[num_verts];
  mesh = new G3DTriangleMesh;

  // setup mesh
  if (x_verts > z_verts) longest = x_verts; else longest = z_verts;
  num_tris = (x_verts - 1) * (z_verts - 1) * 2;
  num_tris += (edge_res + longest) * 8;
  mesh->triangles = NULL;
  mesh->triangles = new csTriangle[num_tris];
  level = (unsigned char)nlevel;
  buf = NULL;
  mesh->vertex_fog = NULL;
  mesh->num_vertices_pool = 1;
  mesh->morph_factor = 0;
  mesh->use_vertex_color = true;
  mesh->do_morph_texels = false;
  mesh->do_morph_colors = false;
  mesh->do_fog = true;
  mesh->vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
  mesh->mixmode = CS_FX_GOURAUD;
  mesh->clip_portal = CS_CLIP_NOT;
  mesh->clip_plane = CS_CLIP_NOT;
  mesh->clip_z_plane = CS_CLIP_NOT;

  //for (int i = 0; i < num_verts; i++)
  //	verts[i].Set (0, 0 ,0);
}

csSharedLODMesh::~csSharedLODMesh ()
{
  delete [] normals;
  delete [] color;
  delete [] texels;
  delete [] verts;
  delete [] mesh->triangles;
  delete mesh;
}

csSharedLODMesh::csSharedLODMesh ()
{
  buf = NULL;
  verts = NULL;
  normals = NULL;
  texels = NULL;
  mesh = NULL;
  color = NULL;
  level = 0;
  x_verts = 0;
  z_verts = 0;
  num_verts = 0;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// csBCLODOwner
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

csBCLODOwner::csBCLODOwner (int newsize)
{
  int i;
  size = newsize;
  owners = new csBCTerrBlock*[newsize];
  for ( i = 0; i < newsize; i++)
  {
    owners[i] = NULL;
  }

}

csBCLODOwner::~csBCLODOwner ()
{
  int i;
  for ( i = 0; i < size; i++)
  {
    owners[i] = NULL;
  }
  delete [] owners;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// csBCTerrBlock
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// freeing from itself
void csBCTerrBlock::FreeLOD ()
{
  if (current_lod == NULL) return;
  // current_lod->free = true;
  if (current_lod->buf)
  {
    if (current_lod->buf->IsLocked ())
      owner->vbufmgr->UnlockBuffer (current_lod->buf);
    current_lod->buf->DecRef ();
    current_lod->buf = NULL;
  }
  if (current_lod->mesh->vertex_fog) delete current_lod->mesh->vertex_fog;
  owner->factory_state->FreeShared (this, current_lod->level);
  current_lod = NULL;
}
// freed from outside
bool csBCTerrBlock::FreeSharedLOD ()
{
  if (current_lod == NULL) return true;
  // current_lod->free = true;
  if (current_lod->buf)
  {
    if (current_lod->buf->IsLocked ())
      owner->vbufmgr->UnlockBuffer (current_lod->buf);
    current_lod->buf->DecRef ();
    current_lod->buf = NULL;
  }
  if (current_lod->mesh->vertex_fog) delete current_lod->mesh->vertex_fog;
  current_lod = NULL;
  return true;
}

void SetNil (G3DTriangleMesh *mesh)
{
  mesh->vertex_fog = NULL;
  mesh->num_vertices_pool = 1;
  mesh->morph_factor = 0;
  mesh->use_vertex_color = false;
  mesh->do_morph_texels = false;
  mesh->do_morph_colors = false;
  mesh->do_fog = true;
  mesh->vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
  mesh->mixmode = CS_FX_GOURAUD;
  mesh->clip_portal = CS_CLIP_NOT;
  mesh->clip_plane = CS_CLIP_NOT;
  mesh->clip_z_plane = CS_CLIP_NOT;
  mesh->triangles = NULL;
  mesh->buffers[0] = NULL;
  mesh->mat_handle = NULL;
}

csBCTerrBlock::csBCTerrBlock ()
{
  controlpoint = NULL;
  current_lod = NULL;
  default_lod = NULL;
  material = NULL;
  //lod_level = 0;
  max_LOD = 0;
  owner = NULL;
  //vis = false;
  x1_end = 0;
  x2_end = 0;
  z1_end = 0;
  z2_end = 0;
  SetNil (&draw_mesh);
  buf = NULL;
}
csBCTerrBlock::~csBCTerrBlock ()
{
  if (default_lod)
  {
    if (default_lod->buf)
      default_lod->buf->DecRef ();
    if (default_lod->mesh->vertex_fog)
      delete default_lod->mesh->vertex_fog;
  }
  if (default_lod) delete default_lod;
  FreeLOD ();
  if (buf)
    buf->DecRef ();
  if (draw_mesh.vertex_fog) delete draw_mesh.vertex_fog;
  if (large_tri) delete [] large_tri;
  if (verts) delete [] verts;
  if (normals) delete [] normals;
  if (texels) delete [] texels;
  if (color) delete [] color;
  if (material) material->DecRef ();
}

void csBCTerrBlock::AddMaterial (iMaterialWrapper* mat)
{
  if (material) material->DecRef ();
  mat->IncRef ();
  material = mat;
  if (default_lod)
    default_lod->mesh->mat_handle = material->GetMaterialHandle ();
  if (current_lod)
    current_lod->mesh->mat_handle = material->GetMaterialHandle ();
  draw_mesh.mat_handle = material->GetMaterialHandle ();  
}

void csBCTerrBlock::RebuildBlock (csBCTerrBlock* up_neighbor, csBCTerrBlock* left_neighbor)
{
  Build (controlpoint, up_neighbor, left_neighbor); 
  if (draw_mesh.vertex_fog) delete draw_mesh.vertex_fog;
  if (large_tri) delete [] large_tri;
  if (verts) delete [] verts;
  if (normals) delete [] normals;
  if (texels) delete [] texels;
  if (color) delete [] color;
  SetupBaseMesh ();
}

void csBCTerrBlock::SetupBaseMesh ()
{
  if (!controlpoint) return;
  if (!owner) return;
  int i, j, hor_length, pos, size, a;
  float u, v, divu, divv;
  csVector3 temp[4];
  size = owner->sys_inc * owner->sys_inc;
  verts = new csVector3[size];
  normals = new csVector3[size];
  texels = new csVector2[size];
  color = new csColor[size];
  hor_length = owner->hor_length;
  pos = 0;

  divu = owner->sys_inc - 1;
  divv = divu;
  for (i = 0; i < owner->sys_inc; i++)
  {
    v = (float)i / divv;
    temp[0] = BezierControlCompute (v, controlpoint, owner->hor_length);
    temp[1] = BezierControlCompute (v, &controlpoint[1], owner->hor_length);
    temp[2] = BezierControlCompute (v, &controlpoint[2], owner->hor_length);
    temp[3] = BezierControlCompute (v, &controlpoint[3], owner->hor_length);
    for (j = 0; j < owner->sys_inc; j++)
    {
      u = (float)j / divu;
      verts[pos] = BezierCompute (u, temp);
      if (j == divu) u = 1.0f;
      if (i == divv) v = 1.0f;
      texels[pos].Set (u,v);
      color[pos].Set (1,1,1);
      normals[pos].Set(1,1,1);
      pos++;      
    }    
  }
  for (i = 0; i < size; i++)
  {
    texels[i].x = texels[i].x * owner->correct_du +
      owner->correct_su;
    texels[i].y = texels[i].y * owner->correct_dv +
      owner->correct_sv;
  }
  draw_mesh.mat_handle = material->GetMaterialHandle ();
  owner->SetupVertexBuffer (buf, buf);
  /*if (buf)
  {
    if (owner->vbufmgr)
      owner->vbufmgr->LockBuffer(buf,
        verts, texels, color,
        size, 0);
  }*/
  small_tri[0].a = 0;
  small_tri[0].b = (owner->sys_inc - 1) * owner->sys_inc;
  small_tri[0].c = owner->sys_inc - 1;
  small_tri[1].a = (owner->sys_inc - 1) * owner->sys_inc;
  small_tri[1].b = size - 1;
  small_tri[1].c = owner->sys_inc - 1;
  pos = 0;
  size = (owner->sys_inc - 1);
  size *= (size * 2);  
  large_tri = new csTriangle[size];
  for (i = 0; i < (owner->sys_inc - 1); i++)
  {
    for (j = 0; j < (owner->sys_inc - 1); j++)
    {
      a = (owner->sys_inc * i) + j;
      large_tri[pos].a = a;
      large_tri[pos].b = a + owner->sys_inc;
      large_tri[pos].c = a + 1;
      pos++;
      large_tri[pos].a = a + owner->sys_inc;
      large_tri[pos].b = a + owner->sys_inc + 1;
      large_tri[pos].c = a + 1;
      pos++;
    }
  }
}

void csBCTerrBlock::Build (csVector3* cntrl,
  	csBCTerrBlock* up_neighbor, csBCTerrBlock* left_neighbor)
{
  csVector3 *work;
  int end, i, j;
  if (!owner->ComputeSharedMesh ( default_lod, cntrl)) exit(0);
  /* edge creation */
  /*
   * Todo:
   * normal / texel / color creation for edges
   */
  //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo: Edge Creation");
  csSharedLODMesh *work_mesh;
  csVector2* size;
  float u_add, u, edges;
  int edge_res, pos, work_end;
  size = owner->factory_state->GetSize ();
  edge_res = owner->factory_state->GetMaxEdgeResolution ();
  end = default_lod->x_verts * default_lod->z_verts;
  edges = edge_res;
  work = cntrl + (owner->hor_length * 3); // lower control points

  // up
  //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo: Edge Creation up");
  if (up_neighbor)
  {
    float count = 0.0;
    work_mesh = up_neighbor->default_lod;
    pos = end;
    x1_end = 0;
    work_end = work_mesh->x_verts * work_mesh->z_verts;
    u_add = (float)(up_neighbor->x2_end - up_neighbor->z1_end) - 1.0;
    for (i = (work_end + up_neighbor->z1_end); i < (work_end + up_neighbor->x2_end); i++)
    {
      default_lod->verts[pos] = work_mesh->verts[i];
      default_lod->normals[pos] = work_mesh->normals[i];
      u = i - (work_end + up_neighbor->z1_end);
      u = u / u_add;
      if (u < 0.0f) u = 0.0f;
      if (u > 1.0f) u = 1.0f;
      default_lod->texels[pos].Set ( 0.0, u );
      //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo u : 0.0 v: %f", u );
      default_lod->color[pos] = work_mesh->color[i];
      pos++;
      x1_end++;
      count += 1.0f;
    }
    //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo u_add %f", u_add );
  } else
  {
    u_add = 1.0 / edges;
    pos = end;
    default_lod->verts[pos] = cntrl[0];
    default_lod->normals[pos].Set (0,1,0);
    default_lod->texels[pos].Set ( 0, 0);
    default_lod->color[pos].Set (1.,1.,1.);
    u = u_add;
    pos += 1;
    x1_end = 1;
    for (i = 0; i < edges; i++)
    {
      default_lod->verts[pos] = BezierCompute (u, cntrl);
      //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo: %f %f %f", default_lod->verts[pos].x, default_lod->verts[pos].y, default_lod->verts[pos].z);
      default_lod->normals[pos].Set (0,1,0);
      default_lod->texels[pos].Set ( 0, u);
      default_lod->color[pos].Set (1.,1.,1.);
      x1_end++;
      pos++;
      u += u_add;
    }
    default_lod->verts[pos] = cntrl[3];
    default_lod->normals[pos].Set (0,1,0);
    default_lod->texels[pos].Set ( 0, 1);
    default_lod->color[pos].Set (1.,1.,1.);
    x1_end++;
  }  
  // right
  //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo: Edge Creation right");
  u_add = 1.0 / (edges + 1.0);
  pos = end + x1_end;
  default_lod->verts[pos] = cntrl[3];
  default_lod->normals[pos].Set (0,1,0);
  default_lod->texels[pos].Set ( 0, 1);
  default_lod->color[pos].Set (1,1,1);
  u = u_add;
  pos += 1;
  z1_end = x1_end + 1;
  for (i = 0; i < edges; i++)
  {
    default_lod->verts[pos] = BezierControlCompute (u, &cntrl[3], owner->hor_length) ;
    //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo: %f %f %f", default_lod->verts[pos].x, default_lod->verts[pos].y, default_lod->verts[pos].z);
    default_lod->normals[pos].Set (0,1,0);
    default_lod->texels[pos].Set ( u, 1);
    default_lod->color[pos].Set (1,1,1);
    z1_end++;
    pos++;
    u += u_add;
  }
  default_lod->verts[pos] = work[3];
  default_lod->normals[pos].Set (0,1,0);
  default_lod->texels[pos].Set ( 1, 1);
  default_lod->color[pos].Set (1,1,1);
  z1_end++;

  // down
  //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo: Edge Creation down");
  u_add = 1.0 / (edges + 1.0);
  pos = end + z1_end;
  default_lod->verts[pos] = work[0];
  default_lod->normals[pos].Set (0,1,0);
  default_lod->texels[pos].Set ( 1, 0);
  default_lod->color[pos].Set (1,1,1);
  //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","Down u : 1.0 v: 0.0" );
  u = u_add;
  pos += 1;
  x2_end = z1_end + 1;
  for (i = 0; i < edges; i++)
  {
    default_lod->verts[pos] = BezierCompute (u, work) ;
    //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo: %f %f %f", default_lod->verts[pos].x, default_lod->verts[pos].y, default_lod->verts[pos].z);

    default_lod->normals[pos].Set (0,1,0);
    default_lod->texels[pos].Set (1, u);
    //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo u : 1.0 v: %f", u );
    default_lod->color[pos].Set (1,1,1);
    x2_end++;
    pos++;
    u += u_add;
  }
  default_lod->verts[pos] = work[3];
  default_lod->normals[pos].Set (0,1,0);
  default_lod->texels[pos].Set ( 1, 1);
  default_lod->color[pos].Set (1,1,1);
  //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","Down u : 1.0 v: 1.0" );
  x2_end++;

  // left
  //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo: Edge Creation left");
  if (left_neighbor)
  {
    work_mesh = left_neighbor->default_lod;
    pos = end + x2_end;
    z2_end = x2_end;
    work_end = work_mesh->x_verts * work_mesh->z_verts; // length
    u_add = (float)(left_neighbor->z1_end - left_neighbor->x1_end) - 1.0;
    for (i = (work_end + left_neighbor->x1_end); i < (work_end + left_neighbor->z1_end); i++)
    {
      default_lod->verts[pos] = work_mesh->verts[i];
      default_lod->normals[pos] = work_mesh->normals[i];
      u = i - (work_end + left_neighbor->x1_end);
      u = u / u_add;
      if (u < 0.0f) u = 0.0f;
      if (u > 1.0f) u = 1.0f;
      default_lod->texels[pos].Set ( u, 0.0);
      //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo u : %f v: 0.0", u );
      default_lod->color[pos] = work_mesh->color[i];
      pos++;
      z2_end++;
    }
  } else
  {
    u_add = 1.0 / edges;
    pos = end + x2_end;
    default_lod->verts[pos] = cntrl[0];
    default_lod->normals[pos].Set (0,1,0);
    default_lod->texels[pos].Set ( 0.0, 0.0);
    default_lod->color[pos].Set (1,1,1);
    u = u_add;
    pos += 1;
    z2_end = x2_end + 1;
    for (i = 0; i < edges; i++)
    {
      default_lod->verts[pos] = BezierControlCompute (u, cntrl, owner->hor_length);
      //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo: %f %f %f", default_lod->verts[pos].x, default_lod->verts[pos].y, default_lod->verts[pos].z);
      default_lod->normals[pos].Set (0,1,0);
      default_lod->texels[pos].Set ( u, 0.0);
      default_lod->color[pos].Set (1,1,1);
      z2_end++;
      pos++;
      u += u_add;
    }
    default_lod->verts[pos] = work[0];
    default_lod->normals[pos].Set (0,1,0);
    default_lod->texels[pos].Set ( 1, 0);
    default_lod->color[pos].Set (1,1,1);
    z2_end++;
  }
  
  for (i = end; i < (end + z2_end); i++)
  {
    default_lod->texels[i].x = (default_lod->texels[i].x * owner->correct_du) +
      owner->correct_su;
    default_lod->texels[i].y = (default_lod->texels[i].y * owner->correct_dv) +
      owner->correct_sv;
    //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","edge u: %f v: %f",
      //default_lod->texels[i].x, default_lod->texels[i].y);
  }
  /*for (i = end; i < (end + z2_end); i++)
  {
    default_lod->texels[i].x = default_lod->texels[i].x * owner->correct_dv +
      owner->correct_sv;
    default_lod->texels[i].y = default_lod->texels[i].y * owner->correct_du +
      owner->correct_su;
  }*/
  
  //csReport (nowner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo: Edge Addition");

  AddEdgeTriangles (default_lod);
  // Setup vertex buffer / g3dTriangleMesh
  //if (default_lod->buf) csReport (nowner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo: Vertex Buffer Error");
  //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo: Vertex Buffer");
  owner->SetupVertexBuffer ( default_lod->buf, default_lod->mesh->buffers[0] );
  //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo: Finished");
  // lock mesh?
  //if (owner->vbufmgr)
  //  owner->vbufmgr->LockBuffer(default_lod->mesh->buffers[0],
  //      default_lod->verts, default_lod->texels, default_lod->color,
  //      (end + z2_end), 0);
  default_lod->mesh->mat_handle = material->GetMaterialHandle ();

  /*for (i= 0; i < end; i++)
  {
    csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo: Verts # %f %f %f", default_lod->verts[i].x, default_lod->verts[i].y, default_lod->verts[i].z);
  }*/
  //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block", "Triangles %d", default_lod->mesh->num_triangles);
  //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block", "x_Verts %d Z_Verts %d", default_lod->x_verts, default_lod->z_verts);
  //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","end %d x1_end %d z1_end %d x2_end %d z2_end %d", end, x1_end, z1_end, x2_end, z2_end);
  /*csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","up");
  for (i= end; i < x1_end; i++)
  {
    csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo: Verts # %f %f %f", default_lod->verts[i].x, default_lod->verts[i].y, default_lod->verts[i].z);
  }
  csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","right");
  for (i= x1_end; i < z1_end; i++)
  {
    csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo: Verts # %f %f %f", default_lod->verts[i].x, default_lod->verts[i].y, default_lod->verts[i].z);
  }
  csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","down");
  for (i= z1_end; i < x2_end; i++)
  {
    csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo: Verts # %f %f %f", default_lod->verts[i].x, default_lod->verts[i].y, default_lod->verts[i].z);
  }
  csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","left");
  for (i= x2_end; i < z2_end; i++)
  {
    csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo: Verts # %f %f %f", default_lod->verts[i].x, default_lod->verts[i].y, default_lod->verts[i].z);
  }
  csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","interior");
  for (i= 0; i < end; i++)
  {
    csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo: Verts # %f %f %f", default_lod->verts[i].x, default_lod->verts[i].y, default_lod->verts[i].z);
  }
  int p;
  for (i= 0; i < default_lod->mesh->num_triangles; i++)
  {
    p = (default_lod->x_verts - 1) * (default_lod->z_verts - 1) * 2;
    if ( i == p ) csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","After end");
    csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo: Triangle # %d %d %d", default_lod->mesh->triangles[i].a, default_lod->mesh->triangles[i].b, default_lod->mesh->triangles[i].c);
  }*/
  work = controlpoint;
  bbox.StartBoundingBox ();
  for ( j = 0; j < 4; j++)
  {
    for ( i = 0; i < 4; i++)
    {
      bbox.AddBoundingVertex (work[i]);
    }
    work += owner->hor_length; // move work to next row
  }
  if (current_lod) 
  {
    AddEdgesToCurrent ();
    owner->ComputeSharedMesh ( current_lod, cntrl);
    AddEdgeTriangles (current_lod);
  }
}

/*
 * SetInfo
 * Sets owner, control point pointer;
 * Finds Max LOD and Default LOD, creates default_lod with shared edges
 *
 */


/*
Currently repeats edge control vertices : could share them :/
*/
void csBCTerrBlock::SetInfo ( csBCTerrObject* nowner, csVector3* cntrl, csBCTerrBlock* up_neighbor,
    csBCTerrBlock* left_neighbor)
{
  float dy, low, high; // change from lowest to heightest height
  bool wavy;
  int i, j;
  int width1, width2, width3, levels;
  csVector3 *work;
  AddMaterial (nowner->factory_state->GetDefaultMaterial ());
  owner = nowner;
  controlpoint = cntrl;
  /* max_LOD & bounding box */
  //csReport (nowner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo: max_lod / bbox");
  low = cntrl[0].y;
  high = cntrl[0].y;
  work = cntrl;
  for ( j = 0; j < 4; j++)
  {
    for ( i = 0; i < 4; i++)
    {
      if (work[i].y < low) low = work[i].y;
      if (work[i].y > high) high = work[i].y;
    }
    work += nowner->hor_length; // move work to next row
  }
  dy = high - low;
  levels = nowner->factory_state->GetUserLOD ();
  high = nowner->factory_state->GetMultiplier ();
  high = high * (256*3);
  if (high != 0) low = dy / high; else low = 0; // change / max_change
  max_LOD = (int)(low * levels); // ratio * levels
  max_LOD -= 1;
  if (max_LOD >= levels) max_LOD = levels - 1;
  if (max_LOD < 0) max_LOD = 0;
  /* default_lod creation */
  wavy = false;
  // horizontal wavy check
  work = cntrl;
  for (j = 0; j < 4; j++)
  {
    if ( (work[1] > work[0]) && (work[1] > work[2]) ) wavy = true;
    if ( (work[2] > work[1]) && (work[2] > work[3]) ) wavy = true;
    work += nowner->hor_length;
  }
  // vertical wavy check
  width1 = nowner->hor_length * 2;
  width2 = nowner->hor_length * 3;
  width3 = nowner->hor_length * 4;
  work = cntrl;
  for (j = 0; j < 4; j++)
  {
    if ( (work[width1] > work[0]) && (work[width1] > work[width2]) ) wavy = true;
    if ( (work[width2] > work[width1]) && (work[width2] > work[width3]) ) wavy = true;
    work += 1;
  }
  //csReport (nowner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo: Create Free Mesh");
  //iBCFa
  default_lod = nowner->factory_state->CreateFreeMesh (wavy);
  if (!default_lod) csReport (nowner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo: Unable to Create Free Mesh");
  
  if (max_LOD > default_lod->level) max_LOD = default_lod->level;
  //csReport (nowner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","SetInfo: Compute Free Mesh");
  Build (cntrl, up_neighbor, left_neighbor);
  
  SetupBaseMesh ();
}
// split for future suport of 3+ lod levels ie: pass mesh
/*Speed this up in the future?*/
void csBCTerrBlock::AddEdgeTriangles ( csSharedLODMesh *lod)
{
  int end, i, diff, edge_pos, x_verts, z_verts, count, num_tri, last_vert, total;
  /* Triangle will be passed down and always points to next available triangle*/
  csTriangle *tri;

  x_verts = lod->x_verts;
  z_verts = lod->z_verts;
  // up
  //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","Add Edge: up");
  num_tri = (x_verts - 1) * (z_verts - 1) * 2;
  tri = lod->mesh->triangles; // adding tris 2 end of triangle array
  end = x_verts * z_verts;
  if ( x1_end < x_verts )
  {
    // edge side has less vertices than the interior square edge
    diff = x_verts / x1_end;
    edge_pos = end; // shared edge pos
    last_vert = 0;
    total = end + x1_end;
    for (i = 0; i < (x_verts - 1); i++)
    {
      tri[num_tri].a = edge_pos;
      tri[num_tri].b = i;
      tri[num_tri].c = i + 1;
      num_tri++;
      if ( (i - last_vert) >= diff )
      {
        edge_pos++;
        last_vert = i;
        if ( edge_pos == total )
        {
          edge_pos--;
        }
        else
        {
          // connecting triangle
          tri[num_tri].a = edge_pos - 1;
          tri[num_tri].b = i + 1;
          tri[num_tri].c = edge_pos;
          num_tri++;
        }
      }
    }
    total -= 1;
    while ( edge_pos < total )
    {
      tri[num_tri].a = edge_pos;
      tri[num_tri].b = x_verts - 1;
      tri[num_tri].c = edge_pos + 1;
      num_tri++;
      edge_pos++;
    }
  } else
  {
    // edge side has more vertices than the interior square edge
    diff = x1_end / x_verts;
    edge_pos = 0; // x_verts upper edge pos
    last_vert = end;
    total = end + x1_end - 1;
    for (i = end; i < total; i++)
    {
      tri[num_tri].a = i;
      tri[num_tri].b = edge_pos;
      tri[num_tri].c = i + 1;
      num_tri++;
      if ( (i - last_vert ) >= diff)
      {
        edge_pos++;
        last_vert = i;
        if ( edge_pos == x_verts ) edge_pos--;
        else
        {
          // connecting triangle
          tri[num_tri].a = i + 1;
          tri[num_tri].b = edge_pos - 1;
          tri[num_tri].c = edge_pos;
          num_tri++;
        }
      }

    }
    while (edge_pos < (x_verts - 1) )
    {
      tri[num_tri].a = total;
      tri[num_tri].b = edge_pos;
      tri[num_tri].c = edge_pos + 1;
      num_tri++;
      edge_pos++;
    }
  }

  // right
  //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","Add Edge: right");
  if ( (z1_end - x1_end) < z_verts)
  {
    // edge side has less vertices than the interior square edge
    diff = z_verts / (z1_end - x1_end);
    edge_pos = end + x1_end; // shared edge pos
    count = 1;
    last_vert = count;
    total = end + z1_end;
    for (i = (x_verts - 1); i < (end - x_verts); i += x_verts)
    {
      tri[num_tri].a = i;
      tri[num_tri].b = i + x_verts;
      tri[num_tri].c = edge_pos;
      num_tri++;
      count++;
      if ( ( count - last_vert) >= diff )
      {
        edge_pos++;
        last_vert = count;
        if ( edge_pos == total ) edge_pos--;
        else
        {
          tri[num_tri].a = i + x_verts;
          tri[num_tri].b = edge_pos;
          tri[num_tri].c = edge_pos - 1;
          num_tri++;
        }
      }
    }
    total -= 1;
    while ( edge_pos < total )
    {
      tri[num_tri].a = edge_pos;
      tri[num_tri].b = end - 1;
      tri[num_tri].c = edge_pos + 1;
      num_tri++;
      edge_pos++;
    }

  } else
  {
    // edge side has more vertices than the interior square edge
    diff = (z1_end - x1_end) / z_verts;
    edge_pos = x_verts - 1; // x_verts right edge start pos
    count = 1;
    last_vert = end + x1_end;
    total = end + z1_end - 1;
    for (i = (end + x1_end); i < total; i++)
    {
      tri[num_tri].a = edge_pos;
      tri[num_tri].b = i + 1;
      tri[num_tri].c = i;
      num_tri++;
      if ( ( i - last_vert) >= diff)
      {
        edge_pos += x_verts;
        last_vert = i;
        count++;
        if ( edge_pos >= end ) edge_pos = end - 1 ;
        else
        {
          tri[num_tri].a = edge_pos - x_verts;
          tri[num_tri].b = edge_pos;
          tri[num_tri].c = i + 1;
          num_tri++;
        }
      }

    }
    while (edge_pos < (end - 1) )
    {
      tri[num_tri].a = edge_pos;
      tri[num_tri].b = edge_pos + x_verts;
      tri[num_tri].c = total;
      num_tri++;
      edge_pos += x_verts;
    }
  }

  // down
  //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","Add Edge: down");
  if ( (x2_end - z1_end) < x_verts )
  {
    // edge side has less vertices than the interior square edge
    diff = x_verts / (x2_end - z1_end);
    edge_pos = end + z1_end; // shared edge pos
    total = end + x2_end;
    count= (z_verts - 1) * x_verts;
    last_vert = count;
    for (i = count; i < (end - 1); i++)
    {
      tri[num_tri].a = i;
      tri[num_tri].b = edge_pos;
      tri[num_tri].c = i + 1;
      num_tri++;
      if ( (i - last_vert) >= diff )
      {
        edge_pos++;
        last_vert = i;
        if ( edge_pos == total ) edge_pos--;
        else
        {
          // connecting triangle
          tri[num_tri].a = edge_pos - 1;
          tri[num_tri].b = edge_pos;
          tri[num_tri].c = i + 1;
          num_tri++;
        }
      }
    }
    total -= 1;
    while ( edge_pos < total )
    {
      tri[num_tri].a = edge_pos;
      tri[num_tri].b = edge_pos + 1;
      tri[num_tri].c = end - 1;
      num_tri++;
      edge_pos++;
    }
  } else
  {
    // edge side has more vertices than the interior square edge
    diff = (x2_end - z1_end) / x_verts;
    edge_pos = (z_verts - 1) * x_verts; // x_verts bottum left edge pos
    //count = edge_pos;
    last_vert = end + z1_end;
    total = end + x2_end - 1;
    for (i = (end + z1_end); i < total; i++)
    {
      tri[num_tri].a = i;
      tri[num_tri].b = i + 1;
      tri[num_tri].c = edge_pos;
      num_tri++;
      if ( (i - last_vert) >= diff)
      {
        edge_pos++;
        last_vert = i;
        if ( edge_pos == end) edge_pos--;
        else
        {
          // connecting triangle
          tri[num_tri].a = edge_pos - 1;
          tri[num_tri].b = i + 1;
          tri[num_tri].c = edge_pos;
          num_tri++;
        }
      }

    }
    while (edge_pos < (end - 1) )
    {
      tri[num_tri].a = edge_pos;
      tri[num_tri].b = total;
      tri[num_tri].c = edge_pos + 1;
      num_tri++;
      edge_pos++;
    }
  }

  // left
  //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","Add Edge: left");
  if ( (z2_end - x2_end) < z_verts)
  {
    // edge side has less vertices than the interior square edge
    diff = z_verts / (z2_end - x2_end);
    edge_pos = end + x2_end; // shared edge pos
    count = 1;
    last_vert = count;
    total = end + z2_end;
    for (i = 0; i < ( (z_verts - 1) * x_verts); i += x_verts)
    {
      tri[num_tri].a = edge_pos;
      tri[num_tri].b = i + x_verts;
      tri[num_tri].c = i;
      num_tri++;
      count++;
      if ( (count - last_vert) >= diff )
      {
        edge_pos++;
        last_vert = count;
        if ( edge_pos == total ) edge_pos--;
        else
        {
          tri[num_tri].a = edge_pos - 1;
          tri[num_tri].b = edge_pos;
          tri[num_tri].c = i + x_verts;
          num_tri++;
        }
      }
    }
    count = (z_verts - 1) * x_verts;
    total -= 1;
    while ( edge_pos < total )
    {
      tri[num_tri].a = edge_pos;
      tri[num_tri].b = edge_pos + 1;
      tri[num_tri].c = count;
      num_tri++;
      edge_pos++;
    }

  } else
  {
    // edge side has more vertices than the interior square edge
    int work;
    diff = (z2_end - x2_end) / z_verts;
    edge_pos = 0; // x_verts right edge start pos
    count = 1;
    work = (z_verts - 1) * x_verts;
    last_vert = end + x2_end;
    total = end + z2_end - 1;
    for (i = (end + x2_end); i < total; i++)
    {
      tri[num_tri].a = i;
      tri[num_tri].b = i + 1;
      tri[num_tri].c = edge_pos;
      num_tri++;
      if ( (i - last_vert) >= diff)
      {
        edge_pos += x_verts;
        count++;
        last_vert = i;
        if ( edge_pos > work ) edge_pos = work;
        else
        {
          tri[num_tri].a = i + 1;
          tri[num_tri].b = edge_pos;
          tri[num_tri].c = edge_pos - x_verts;
          num_tri++;
        }
      }

    }
    while (edge_pos < work )
    {
      tri[num_tri].a = total;
      tri[num_tri].b = edge_pos + x_verts;
      tri[num_tri].c = edge_pos;
      num_tri++;
      edge_pos += x_verts;
    }
  }
  //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","X_Verts: %d", lod->x_verts);
  //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","Z_Verts: %d", lod->z_verts);
  lod->mesh->num_triangles = num_tri;
  //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","num_triangles: %d", lod->mesh->num_triangles);
  //lod->mesh->num_triangles = (x_verts - 1) * (z_verts - 1) * 2;
  //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block","num_triangles: %d", lod->mesh->num_triangles);

}

void csBCTerrBlock::ManagerClosed ()
{
  if (current_lod)
  {
    if (current_lod->buf) current_lod->buf->DecRef ();
    current_lod->buf = NULL;
  }
  if (default_lod)
  {
    if (default_lod->buf) default_lod->buf->DecRef ();
    default_lod->buf = NULL;
  }
}
/*
 * CreateNewMesh :
 * Creates a new curent_lod
 * Checks if able to compute a new mesh
 * then frees old current_lod and computes new mesh
 * includes edge info copying and edge triangle creation
 */

void csBCTerrBlock::CreateNewMesh (int level)
{
  csSharedLODMesh* newmesh;
  int end;
  
  newmesh = owner->factory_state->GetSharedMesh (level, this);
  if (!newmesh) 
  {
    int levels;
    levels = owner->factory_state->GetUserLOD ();
    levels--;
    level++;
    if ( level < levels )
      newmesh = owner->factory_state->GetSharedMesh (level, this);
    if (!newmesh)
      return;
  }
  FreeLOD ();
  current_lod = newmesh;
  AddEdgesToCurrent ();
  owner->ComputeSharedMesh (newmesh, controlpoint);
  AddEdgeTriangles (current_lod);
  current_lod->buf = NULL; // make sure, can cause memory leak?
  owner->SetupVertexBuffer ( current_lod->buf, current_lod->mesh->buffers[0] );
  // lock buffer?
  end = current_lod->x_verts * current_lod->z_verts;
  //if (owner->vbufmgr)
  //  owner->vbufmgr->LockBuffer(current_lod->mesh->buffers[0],
  //      current_lod->verts, current_lod->texels, current_lod->color,
  //      (end + z2_end), 0);
  current_lod->mesh->mat_handle = material->GetMaterialHandle ();
}

void csBCTerrBlock::AddEdgesToCurrent ()
{
  int i, end, stop, pos;
  if (!current_lod) return;
  end = current_lod->x_verts * current_lod->z_verts;
  stop = end + x1_end;
  pos = default_lod->x_verts * default_lod->z_verts;
  for (i = end; i < stop; i++)
  {
    current_lod->verts[i] = default_lod->verts[pos];
    current_lod->color[i] = default_lod->color[pos];
    current_lod->normals[i] = default_lod->normals[pos];
    current_lod->texels[i] = default_lod->texels[pos];
    pos++;
  }
  stop = end + z1_end;
  for (i = (end + x1_end); i < stop; i++)
  {
    current_lod->verts[i] = default_lod->verts[pos];
    current_lod->color[i] = default_lod->color[pos];
    current_lod->normals[i] = default_lod->normals[pos];
    current_lod->texels[i] = default_lod->texels[pos];
    pos++;
  }
  stop = end + x2_end;
  for (i = (end + z1_end); i < stop; i++)
  {
    current_lod->verts[i] = default_lod->verts[pos];
    current_lod->color[i] = default_lod->color[pos];
    current_lod->normals[i] = default_lod->normals[pos];
    current_lod->texels[i] = default_lod->texels[pos];
    pos++;
  }
  stop = end + z2_end;
  for (i = (end + x2_end); i < stop; i++)
  {
    current_lod->verts[i] = default_lod->verts[pos];
    current_lod->color[i] = default_lod->color[pos];
    current_lod->normals[i] = default_lod->normals[pos];
    current_lod->texels[i] = default_lod->texels[pos];
    pos++;
  }
}

static void Perspective (const csVector3& v, csVector2& p, float fov,
      float sx, float sy)
{
  float iz = fov / v.z;
  p.x = v.x * iz + sx;
  p.y = v.y * iz + sy;
}
/*
    Draw
    0 = current
    1 = default
    2 = largepolygons
    3 = small polygon
*/

void csBCTerrBlock::Draw (iRenderView *rview, iCamera* camera, int level)
{
  csReversibleTransform& camtrans = camera->GetTransform ();
  float fov = camera->GetFOV ();
  float sx = camera->GetShiftX ();
  float sy = camera->GetShiftY ();

  // first compute camera and screen space bounding boxes.
  csBox3 cbox;
  cbox.StartBoundingBox (camtrans * bbox.GetCorner (0));
  cbox.AddBoundingVertexSmart (camtrans * bbox.GetCorner (1));
  cbox.AddBoundingVertexSmart (camtrans * bbox.GetCorner (2));
  cbox.AddBoundingVertexSmart (camtrans * bbox.GetCorner (3));
  cbox.AddBoundingVertexSmart (camtrans * bbox.GetCorner (4));
  cbox.AddBoundingVertexSmart (camtrans * bbox.GetCorner (5));
  cbox.AddBoundingVertexSmart (camtrans * bbox.GetCorner (6));
  cbox.AddBoundingVertexSmart (camtrans * bbox.GetCorner (7));

  // if the entire bounding box is behind the camera, we're done.
  if ((cbox.MinZ () < 0) && (cbox.MaxZ () < 0))
    return;

  // Transform from camera to screen space.
  csBox2 sbox;
  if (cbox.MinZ () <= 0)
  {
    // Bbox is very close to camera.
    // Just return a maximum bounding box.
    sbox.Set (-10000, -10000, 10000, 10000);
  }
  else
  {
    csVector2 oneCorner;
    Perspective (cbox.Max (), oneCorner, fov, sx, sy);
    sbox.StartBoundingBox (oneCorner);
    csVector3 v (cbox.MinX (), cbox.MinY (), cbox.MaxZ ());
    Perspective (v, oneCorner, fov, sx, sy);
    sbox.AddBoundingVertexSmart (oneCorner);
    Perspective (cbox.Min (), oneCorner, fov, sx, sy);
    sbox.AddBoundingVertexSmart (oneCorner);
    v.Set (cbox.MaxX (), cbox.MaxY (), cbox.MinZ ());
    Perspective (v, oneCorner, fov, sx, sy);
    sbox.AddBoundingVertexSmart (oneCorner);
  }
  int clip_portal, clip_plane, clip_z_plane;
  if (rview->ClipBBox (sbox, cbox, clip_portal, clip_plane, clip_z_plane))
  {
    iGraphics3D *pG3D = rview->GetGraphics3D();
    int end;
    if ((!current_lod) && (level == 0)) level = 1;
    //if ((!current_lod) || (level > 1)) level = 1;
    //if (level == 2) level = 3;
    if (level == 0)
    {
      if (!current_lod->mesh->mat_handle)
        current_lod->mesh->mat_handle = material->GetMaterialHandle ();
      if (!current_lod->buf)
        owner->SetupVertexBuffer ( current_lod->buf, current_lod->mesh->buffers[0] );
      //if (!current_lod->buf->IsLocked ())
      //{
        end = current_lod->x_verts * current_lod->z_verts;
        owner->vbufmgr->LockBuffer(current_lod->mesh->buffers[0],
          current_lod->verts, current_lod->texels, current_lod->color,
          (end + z2_end), 0);
      //}
      current_lod->mesh->clip_portal = clip_portal;
      current_lod->mesh->clip_portal = clip_plane;
      current_lod->mesh->clip_portal = clip_z_plane;

      rview->CalculateFogMesh(camtrans, current_lod->mesh[0]);
      //current_lod->mesh->do_mirror = true;
      pG3D->DrawTriangleMesh(current_lod->mesh[0]);
      owner->vbufmgr->UnlockBuffer (current_lod->mesh->buffers[0]);
      //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block", "Drawing current_lod");
    } else if (level == 1)
    {
      if (!default_lod->mesh->mat_handle)
        default_lod->mesh->mat_handle = material->GetMaterialHandle ();
      if (!default_lod->buf)
        owner->SetupVertexBuffer ( default_lod->buf, default_lod->mesh->buffers[0] );
      //if (!default_lod->buf->IsLocked ())
      //{
        end = default_lod->x_verts * default_lod->z_verts;
        owner->vbufmgr->LockBuffer(default_lod->mesh->buffers[0],
          default_lod->verts, default_lod->texels, default_lod->color,
          (end + z2_end), 0);
      //}
      default_lod->mesh->clip_portal = clip_portal;
      default_lod->mesh->clip_portal = clip_plane;
      default_lod->mesh->clip_portal = clip_z_plane;
      rview->CalculateFogMesh(camtrans, default_lod->mesh[0]);
      //default_lod->mesh->do_mirror = true;
      pG3D->DrawTriangleMesh(default_lod->mesh[0]);
      owner->vbufmgr->UnlockBuffer (default_lod->mesh->buffers[0]);
      //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block", "Drawing default_lod");
    } else if (level == 2)
    {
      //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block", "Draw #3");
      if (!draw_mesh.mat_handle)
        draw_mesh.mat_handle = material->GetMaterialHandle ();
      if (!buf)
        owner->SetupVertexBuffer (buf , draw_mesh.buffers[0]);
      else
        draw_mesh.buffers[0] = buf;
      //if (!buf->IsLocked ())
      //{        
        end = owner->sys_inc * owner->sys_inc;  
        owner->vbufmgr->LockBuffer(draw_mesh.buffers[0],
          verts, texels, color,
          end, 0);
      //}
      draw_mesh.clip_portal = clip_portal;
      draw_mesh.clip_portal = clip_plane;
      draw_mesh.clip_portal = clip_z_plane;
      draw_mesh.triangles = large_tri;
      end = owner->sys_inc - 1;
      end *= (end * 2);
      draw_mesh.num_triangles = end;
      rview->CalculateFogMesh(camtrans, draw_mesh);
      pG3D->DrawTriangleMesh(draw_mesh);
      owner->vbufmgr->UnlockBuffer (draw_mesh.buffers[0]);
    } else
    {
      //csReport (owner->object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Block", "Draw #4");
      if (!draw_mesh.mat_handle)
        draw_mesh.mat_handle = material->GetMaterialHandle ();
      if (!buf)
        owner->SetupVertexBuffer (buf , draw_mesh.buffers[0]);
      else
        draw_mesh.buffers[0] = buf;
      //if (!buf->IsLocked ())
      //{
        owner->vbufmgr->LockBuffer(draw_mesh.buffers[0],
          verts, texels, color,
          16, 0);
      //}
      draw_mesh.clip_portal = clip_portal;
      draw_mesh.clip_portal = clip_plane;
      draw_mesh.clip_portal = clip_z_plane;
      draw_mesh.triangles = &small_tri[0];
      draw_mesh.num_triangles = 2;
      rview->CalculateFogMesh(camtrans, draw_mesh);
      pG3D->DrawTriangleMesh(draw_mesh);
      owner->vbufmgr->UnlockBuffer (draw_mesh.buffers[0]);
    }
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// csBCTerrObject
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
SCF_IMPLEMENT_IBASE(BCPolyMesh)
  SCF_IMPLEMENTS_INTERFACE(iPolygonMesh)
SCF_IMPLEMENT_IBASE_END

BCPolyMesh::BCPolyMesh ()
{
  SCF_CONSTRUCT_IBASE (NULL)
  culling = false;
  square_verts = NULL;
  culling_mesh = NULL;
}
BCPolyMesh::~BCPolyMesh ()
{
  if (culling)
  {
    delete [] square_verts;
    delete [] culling_mesh[0].vertices;
    delete culling_mesh;
  }    
} 


SCF_IMPLEMENT_IBASE (csBCTerrObject)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iBCTerrState)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iTerrFuncState)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iVertexBufferManagerClient)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBCTerrObject::BCTerrState)
  SCF_IMPLEMENTS_INTERFACE (iBCTerrState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBCTerrObject::TerrFuncState)
  SCF_IMPLEMENTS_INTERFACE (iTerrFuncState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBCTerrObject::BCTerrModel)
  SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBCTerrObject::eiVertexBufferManagerClient)
  SCF_IMPLEMENTS_INTERFACE (iVertexBufferManagerClient)
SCF_IMPLEMENT_EMBEDDED_IBASE_END


csBCTerrObject::csBCTerrObject (iObjectRegistry* object_reg,
  iMeshObjectFactory *newpFactory)
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiBCTerrState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiTerrFuncState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiVertexBufferManagerClient);
  csBCTerrObject::object_reg = object_reg;
  pFactory = newpFactory;
  logparent = NULL;
  vbufmgr = NULL;
  factory_state = SCF_QUERY_INTERFACE (pFactory, iBCTerrFactoryState);
  initialized = false;
  topleft.Set (0,0,0);
  x_blocks = z_blocks = 0;
  vis_cb = NULL;
  collision = NULL;
  initheight = false;
  toph = 0.0;
  righth = 0.0;
  downh = 0.0;
  lefth = 0.0;
  vis = false;
  sys_inc = 4;
  prebuilt = false;
  CorrectSeams (0,0);
}

csBCTerrObject::~csBCTerrObject ()
{
  // int x, z;
  if (control_points) delete [] control_points;
  if (blocks) delete [] blocks;
  if (vis_cb) vis_cb->DecRef ();
}

void csBCTerrObject::Build ()
{
  if (!initialized)
  {
    if (prebuilt)
    {
      SetupMesh ();
      BuildCullMesh ();
      initialized = true;
    }
  }
}

void csBCTerrObject::RebuildBlocks ()
{
  int end, i, j;
  csBCTerrBlock *left, *up;
  if (!initialized) return;
  end = x_blocks * z_blocks;    
  for ( i = 0; i < x_blocks; i++)
  {
    for (j = 0; j < z_blocks; j++)
    {      
      end = i * (x_blocks) + j;
      if (i != 0) up = &blocks[end - x_blocks]; else up = NULL;
      if (j == 0) left = NULL; else left = &blocks[end - 1];
      blocks[end].RebuildBlock (up, left);
    }
  }
}

void csBCTerrObject::CorrectSeams (int tw, int th)
{
  //if (object_reg)
    //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Terr","x %d, y %d", tw, th);
  correct_tw = tw;
  correct_th = th;
  if (tw)
  {
    correct_du = 1. - 2. / float (tw);
    correct_su = 1. / float (tw);
  }
  else
  {
    correct_du = 1;
    correct_su = 0;
  }
  if (th)
  {
    correct_dv = 1. - 2. / float (th);
    correct_sv = 1. / float (th);
  }
  else
  {
    correct_dv = 1;
    correct_sv = 0;
  }
}


int csBCTerrObject::HeightTest (csVector3 *point)
{
	//csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","Height Test");
  if (initialized)
  {
    if (collision)
      return collision->HeightTestExact (point);
  }
  return 0;
}

int csBCTerrObject::HeightTestExt (csVector3 *point)
{
	//csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","Height Test");
  if (initialized)
  {
    if (collision)
      return collision->HeightTestExt (point);
  }
  return 0;
}

void csBCTerrObject::PreBuild ()
{
  csVector2* size;
  csVector3* work;
  float x, z, last_x, last_z, rat_x, rat_z;
  // rat = ratio
  int x_size, z_size, i, j, new_size;
  if ( (x_blocks <= 0) || (z_blocks <= 0) ) return;
  size = factory_state->GetSize (); // get block size
  x = size->x / 3.0; // x axis increments
  z = size->y / 3.0; // z axis increments
  x_size = (3 * x_blocks) + 1; // # of x control points
  z_size = (3 * z_blocks) + 1; // # of z control points
  new_size = x_size * z_size;
  control_points = new csVector3[new_size];
  hor_length = x_size;  
  control_points[0] = topleft;

  last_z = topleft.z;
  work = control_points;
  for (i = 0; i < z_size; i++)
  {
    last_x = topleft.x;
    rat_z = (topleft.z - last_z) / (z_blocks * size->y);
    //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","Control Points: New Set");
    for (j = 0; j < x_size; j++)
    {
      work->x = last_x;
      work->z = last_z;
      rat_x = (last_x - topleft.x) / (x_blocks * size->x);
      // get y value
      work->y = topleft.y;
      //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","Control Points: %f %f %f", work->x, work->y, work->z);
      last_x += x;
      work += 1;
    }
    last_z -= z;
  }  
  prebuilt = true;
}

void csBCTerrObject::SetControlPoint (const csVector3 point,
    const int iter)
{
  int size;
  if (!prebuilt) return;
  size = ((3 * x_blocks) + 1) * ((3 * z_blocks) + 1);
  if ((size <= iter) && (iter >=0) ) 
  {
    control_points[iter] = point;    
    if (initialized)
    {
      FlattenSides ();
      SetupCollisionQuads ();
      BuildCullMesh ();
      RebuildBlocks ();
    }
  }
}

void csBCTerrObject::SetControlPointHeight (const float height,
    const int iter)
{
  int size;
  if (!prebuilt) return;
  //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","Set Control Point Height: %f iter: %d",
    //height, iter);
  size = ((3 * x_blocks) + 1) * ((3 * z_blocks) + 1);
  if ((iter < size) && (iter >= 0) )
  {
    control_points[iter].y = height;    
    if (initialized)
    {
      FlattenSides ();
      SetupCollisionQuads ();
      BuildCullMesh ();
      RebuildBlocks ();
    }
  }
}


bool csBCTerrObject::DrawTest (iRenderView* rview, iMovable* movable)
{
  return true;
}

void csBCTerrObject::UpdateLighting (iLight** lights, int num_lights,
               iMovable* movable)
{
}

/*
 * todo:
 * make 2 seperate for loops
 * 1 -> sys dist
 * 2 -> normal checks
 * remove visibilty system for terrblock
 * add list of checking objects to factory
*/

bool csBCTerrObject::Draw (iRenderView* rview, iMovable* movable,
    csZBufMode zbufMode)
{
  if ( !initialized) return true;
  int i, n, j, lod_levels, level;
  bool do_sys_dist, sys_far;
  do_sys_dist = false;
  sys_far = false;
  float distance, start_sys, sys_dist ;
  csVector3 dist;
  //SetAllVisible (); // for now
  n = x_blocks * z_blocks;
  iGraphics3D *pG3D = rview->GetGraphics3D();
  iCamera* pCamera = rview->GetCamera();
  csReversibleTransform& camtrans = pCamera->GetTransform ();
  // csBCTerrBlock *cur_block;
  const csVector3& cam_origin = camtrans.GetOrigin ();
  pG3D->SetObjectToCamera (&camtrans);
  pG3D->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, zbufMode );
  float* Distances = factory_state->GetLODDistances ();
  factory_state->GetSystemDistance (start_sys, sys_dist);
  lod_levels = factory_state->GetUserLOD ();

  dist = bbox.GetCenter () - cam_origin;
  distance = dist.x * dist.x + dist.y * dist.y + dist.z * dist.z;
  vis = true;
  distance -= (radius * radius);
  if (distance > start_sys)
  {
    do_sys_dist = true;
    if (distance > sys_dist)
      sys_far = true;      
  }
  // csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","Drawing");
  for (i = 0; i < n; i++)
  {
    //if (blocks[i].vis )
    //{
      dist = blocks[i].bbox.GetCenter () - cam_origin;
      distance = dist.x * dist.x + dist.y * dist.y + dist.z * dist.z;
      if (blocks[i].current_lod && (do_sys_dist == false))
      {
        csSharedLODMesh* current_lod = blocks[i].current_lod;
        if ( current_lod->level == 0 )
        {
          // front of lod spectrum
          if (distance > (Distances[0]) )
          {
            blocks[i].FreeLOD ();
            if (lod_levels > 1)
            {
              level = -1;
              for (j = 1; j < (lod_levels - 1); j++)
              {
                if (distance < Distances[j] )
                {
                  level = j;
                  break;
                }
              }
              if (level > 0)
                blocks[i].CreateNewMesh (level);
            }
          }
        } else
        {
          level = -1;
          for (j = 0; j < (lod_levels - 1); j++)
          {
            if (distance < Distances[j])
            {
              level = j;
              break; // get out of for loop
            }
          }
          if (level > -1)
          {
            if (level < current_lod->level)
            {
              blocks[i].CreateNewMesh (level);
            } else if (level > current_lod->level)
            {
              blocks[i].FreeLOD ();
              blocks[i].CreateNewMesh (level);
            }
          } else
            blocks[i].FreeLOD ();
        }
        blocks[i].Draw (rview, pCamera, 0);
      } else
      {
        if (do_sys_dist)
        {
          if (sys_far)
            blocks[i].Draw (rview, pCamera, 3);
          else
            blocks[i].Draw (rview, pCamera, 2);          
        } else
        {
          // get new current_lod, draw current_lod
          if (distance < Distances[lod_levels - 1])
          {
            if (lod_levels > 1)
            {
              level = -1;
              for (j = 0; j < (lod_levels - 1); j++)
              {
                if (distance < Distances[j])
                {
                  level = j;
                  break;
                }
              }
              if (level > -1)
                blocks[i].CreateNewMesh (level);
            }
            blocks[i].Draw (rview, pCamera, 0);
          } else
          {
            blocks[i].Draw (rview, pCamera, 1);
          }
        }
      }
    /*} else
    {
      if (blocks[i].current_lod)
      {
        // free lod if distance is correct
        // currently unused and incomplete
        dist = blocks[i].bbox.GetCenter () - cam_origin;
        distance = dist.x * dist.x + dist.y * dist.y + dist.z * dist.z;
        if (distance > (Distances[blocks[i].current_lod->level + 1]) )
          blocks[i].FreeLOD ();
      }
    }*/
  }
  factory_state->SetFocusPoint (cam_origin);
  return true;
}


void csBCTerrObject::GetRadius (csVector3& rad, csVector3& cent)
{
  rad = radius;
  cent = bbox.GetCenter ();
}

bool csBCTerrObject::HitBeamOutline (const csVector3& start, const csVector3& end,
    csVector3& isect, float* pr)
{
  if (collision)
    return collision->HitBeamOutline (start, end, isect, pr);
  else
    return false;
}

bool csBCTerrObject::HitBeamObject (const csVector3& start, const csVector3& end,
    csVector3& isect, float* pr)
{
  if (collision)
    return collision->HitBeamObject (start, end, isect, pr);
  else
    return false;
}

/*
 *Set Height Map
 *Where creation starts for this mesh, SetupMesh is called here :P
 */
void csBCTerrObject::SetHeightMap (iImage* im)
{
  if ( (x_blocks < 1) || (z_blocks < 1) ) return;
  if (!prebuilt) return;
  //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","Setup: Control Points");
  SetupControlPoints (im);
  //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","Setup: Mesh");
  SetupMesh ();
  BuildCullMesh ();
  initialized = true;
  //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","Setup: Complete");
}

void csBCTerrObject::SetupMesh ()
{
  int size, i, j;
  float t;
  csVector3 v, center;
  csVector3 *cntrl_pt;
  csBCTerrBlock *up, *left;
  size = x_blocks * z_blocks;
  blocks = new csBCTerrBlock[size];
  cntrl_pt = control_points;  
  FlattenSides ();
  bbox.StartBoundingBox (); // empty bounding box
  for (i = 0; i < z_blocks; i++)
  {
    for (j = 0; j < x_blocks; j++)
    {
      size = (i * x_blocks) + j;
      if (i != 0) up = &blocks[size - x_blocks]; else up = NULL;
      if (j == 0) left = NULL; else left = &blocks[size - 1];
      //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","Mesh Setup: SetInfo");
      blocks[size].SetInfo ( this, cntrl_pt, up, left);
      //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","Mesh Setup: End SetInfo");
      bbox += blocks[size].bbox; // create a global bounding box
      cntrl_pt += 3;
    }
    cntrl_pt += (hor_length * 2) + 1;
  }
  center = bbox.GetCenter();
  v = bbox.Max();
  t = (v.x - center.x)*(v.x - center.x) +
    (v.y - center.y)*(v.y - center.y) +
    (v.z - center.z)*(v.z - center.z);
  t = qsqrt (t);
  radius = csVector3 (t,t,t);
  SetupCollisionQuads ();
}

void csBCTerrObject::SetupCollisionQuads ()
{
  float shortest;
  csVector2* size;
  int end, i;
  size = factory_state->GetSize ();
  shortest = 0.0f;
  if (x_blocks > z_blocks)
    shortest = size->y;
  else
    shortest = size->x;
  if (collision) delete collision;
  collision = new csBCCollisionQuad (control_points, x_blocks, 
      z_blocks, shortest, object_reg );
  end = x_blocks * z_blocks;
  for (i = 0; i < end; i++)
  {
    collision->AddBlock (&blocks[i]);
  }
  collision->RebuildBoundingBoxes ();
}

int csBCTerrObject::GetHeightFromImage (iImage* im, float x, float z)
{
  int height, width, pos, rows;
  csRGBpixel* pix;
  width = im->GetWidth ();
  height = im->GetHeight ();
  if (x < 0) x = 0;
  if (x > 1) x = 1.0;
  if (z < 0) z = 0;
  if (z > 1) z = 1.0;
  pix = (csRGBpixel*)im->GetImageData ();
  rows = QInt (z * height - 1);
  if (rows < 0) rows = 0;
  pos = QInt ((rows * width) + (x * width));
  if (pos > (width * height)) pos = width * height;
  return ( QInt (pix[pos].red) + QInt (pix[pos].green) + QInt (pix[pos].blue) );

}

void csBCTerrObject::FlattenSides ()
{
  int x_size, z_size, i, end;
  float y;
  if (!prebuilt) return;
  x_size = (3 * x_blocks) + 1; // # of x control points
  z_size = (3 * z_blocks) + 1;
  end = x_size * z_size;
  
  /*control_points[0].y = topleft.y;
  control_points[x_size - 1].y = topleft.y; 
  control_points[(z_size -1) * x_size].y = topleft.y; 
  control_points[end - 1].y = topleft.y;*/ 
  if (!initheight)
  {
    toph = topleft.y;
    righth = topleft.y;
    downh = topleft.y;
    lefth = topleft.y;    
  } else
  {    
    float max;
    max = toph;
    if (righth > max ) max = righth;
    if (downh > max ) max = downh;
    if (lefth > max ) max = lefth;
    if (btop && (bright || bleft) )
    {
      toph = max;
    }
    if (bright && (btop || bdown))
    {
      righth = max;
    }
    if (bdown && (bright || bleft))
    {  
      downh = max;  
    }
    if (bleft && (btop || bdown))
    {
      lefth = max;
    }
  }

  // top
  if (btop)
  {  
    y = toph;    
    for (i = 0; i < (x_size); i++)
    {
      control_points[i].y = y;
    }
  }
  // right
  if (bright)
  {    
    y = righth;
    for (i = (x_size - 1 ); i < end; i += x_size)
    {
      control_points[i].y = y;
    }
  }
  // down
  if (bdown)
  { 
    y = downh;   
    for (i = ((z_size - 1) * x_size); i < end; i++)
    {
      control_points[i].y = y;
    }
  }
  // left
  if (bleft)
  {
    y = lefth;    
    for (i = 0; i < end; i += x_size)
    {
      control_points[i].y = y;
    }
  }
}

void csBCTerrObject::BuildCullMesh ()
{
  int x_size, z_size, end;
  x_size = (3 * x_blocks) + 1;
  z_size = (3 * z_blocks) + 1;
  end = x_size * z_size;
  int *verts;
  if (culling_mesh.culling_mesh)
  {
    if (culling_mesh.culling_mesh[0].vertices)
      delete [] culling_mesh.culling_mesh[0].vertices;
    if (culling_mesh.square_verts) delete culling_mesh.square_verts;
    delete culling_mesh.culling_mesh;
  }
  culling_mesh.culling_mesh = new csMeshedPolygon;  
  verts = new int[4];  
  culling_mesh.square_verts = new csVector3[4];
  culling_mesh.square_verts[0] = control_points[0];
  culling_mesh.square_verts[1] = control_points[x_size - 1];
  culling_mesh.square_verts[2] = control_points[end - 1];
  culling_mesh.square_verts[3] = control_points[end - x_size];
  verts[0] = 0;
  verts[1] = 1;
  verts[2] = 2;
  verts[3] = 3;
  culling_mesh.culling_mesh[0].vertices = verts;
  culling_mesh.culling_mesh[0].num_vertices = 4; 
  culling_mesh.culling = true;   
}

void csBCTerrObject::SetupControlPoints (iImage* im)
{
  csVector2* size;
  csVector3* work;
  float x, z, last_x, last_z, rat_x, rat_z;
  // rat = ratio
  int x_size, z_size, i, j;
  control_points[0] = topleft;
  size = factory_state->GetSize (); // get block size
  x = size->x / 3.0; // x axis increments
  z = size->y / 3.0; // z axis increments
  x_size = (3 * x_blocks) + 1; // # of x control points
  z_size = (3 * z_blocks) + 1; // # of z control points

  last_z = topleft.z;
  work = control_points;
  for (i = 0; i < z_size; i++)
  {
    last_x = topleft.x;
    rat_z = (topleft.z - last_z) / (z_blocks * size->y);
    //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","Control Points: New Set");
    for (j = 0; j < x_size; j++)
    {
      work->x = last_x;
      work->z = last_z;
      rat_x = (last_x - topleft.x) / (x_blocks * size->x);
      // get y value
      work->y = topleft.y + ( GetHeightFromImage(im, rat_x, rat_z) * factory_state->GetMultiplier()) ;
      //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","Control Points: %f %f %f", work->x, work->y, work->z);
      last_x += x;
      work += 1;
    }
    last_z -= z;
  }
  prebuilt = true;
}
/*
 * FreeSharedLOD
 *
 * free lods of non- visible objects
 */
void csBCTerrObject::FreeSharedLOD (const csVector3 point)
{
  if (vis == false)
  {
    int i, n;
    csVector3 dist;
    float distance, checkdistance;
    n = x_blocks * z_blocks;
    float* Distances = factory_state->GetLODDistances ();
    for (i = 0; i < n; i++)
    {
      if (blocks[i].current_lod)
      {
        // free lod if distance is correct
        // currently unused and incomplete
        dist = blocks[i].bbox.GetCenter () - point;
        distance = dist.x * dist.x + dist.y * dist.y + dist.z * dist.z;
        checkdistance = Distances[blocks[i].current_lod->level];
        if (distance > checkdistance )
          blocks[i].FreeLOD ();
      }
    }
  }
}


/*
 * ComputeSharedMesh
 *
 * Creates interior mesh with triangles
 * Does not create edge triangles, those are handled by the block class
 */

bool csBCTerrObject::ComputeSharedMesh (csSharedLODMesh* mesh, csVector3* cntrl_pts)
{
  csVector2* uv;
  csVector3 temp[4];
  // csVector3 *vert;
  float v, u;
  int count, x_verts, z_verts, i, j, count2;
  uv = factory_state->GetLODUV ( mesh->level );
  if ( (uv->x <= 0) || ( uv->y <= 0) ) return false; // deal wif the error
  v = uv->y;
  count = 0;
  //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","Interior Vertexes: Setup");
  //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","X_Verts %d Z_Verts %d", mesh->x_verts, mesh->z_verts);
  while ( v < 1.0 )
  {
    temp[0] = BezierControlCompute ( v, cntrl_pts , hor_length);
    temp[1] = BezierControlCompute ( v, (cntrl_pts + 1) , hor_length);
    temp[2] = BezierControlCompute ( v, (cntrl_pts + 2) , hor_length);
    temp[3] = BezierControlCompute ( v, (cntrl_pts + 3) , hor_length);
    u = uv->x;
    //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","Interior Vertexes: New Set");
    //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","Temp: %f %f %f", temp[0].x, temp[0].y, temp[0].z);
    //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","Temp: %f %f %f", temp[1].x, temp[1].y, temp[1].z);
    //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","Temp: %f %f %f", temp[2].x, temp[2].y, temp[2].z);
    //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","Temp: %f %f %f", temp[3].x, temp[3].y, temp[3].z);
    //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","Hor_Length %d", hor_length);
    //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","v %f", v);
    while ( u < 1.0)
    {
      mesh->verts[count] = BezierCompute (u, temp);
      //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","Interior Vertexes: %f %f %f", mesh->verts[count].x, mesh->verts[count].y, mesh->verts[count].z);
      //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","u %f", u);
      mesh->texels[count].Set (v, u);
      mesh->normals[count].Set (0, 1, 0); // ? just for now...
      mesh->color[count].Set ( 1, 1, 1);
      count++;
      u += uv->x;
    }
    v += uv->y;
  }
  int end = mesh->x_verts * mesh->z_verts;
  for (i = 0; i < end; i++)
  {
    mesh->texels[i].x = mesh->texels[i].x * correct_du +
      correct_su;
    mesh->texels[i].y = mesh->texels[i].y * correct_dv +
      correct_sv;
  }
  //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Object","!!!!Interior Vertexes: END!!!!");
  //CS_ASSERT( count == (mesh->x_verts * mesh->z_verts));
  /* triangle computation */
  // get x / z verts
  /*u = uv->x;
  count = 0;
  while (u < 1.0)
  {
    count++;
    u += uv->x;
  }
  x_verts = count;

  v = uv->y;
  count = 0;
  while (v < 1.0)
  {
    count++;
    v += uv->y;
  }
  z_verts = count;*/
  x_verts = mesh->x_verts;
  z_verts = mesh->z_verts;
  // create tris
  csTriangle* tri;
  tri = mesh->mesh->triangles;
  for (i = 0; i < (z_verts - 1); i++)
  {
    count = i * x_verts; // current row
    count2 = count + x_verts; // next row
    for ( j = 0; j < (x_verts - 1); j++)
    {
      tri[0].a = count + j;
      tri[0].b = count2 + j;
      tri[0].c = count + j + 1;
      tri[1].a = count2 + j;
      tri[1].b = count2 + j + 1;
      tri[1].c = count + j + 1;
      tri += 2;
    }
  }
  return true;
}

void csBCTerrObject::eiVertexBufferManagerClient::ManagerClosing ()
{
  if (scfParent->vbufmgr)
  {
    int n = scfParent->x_blocks * scfParent->z_blocks;
    for (int i = 0 ; i < n; i++)
    {
      csBCTerrBlock& block = scfParent->blocks[i];
      block.ManagerClosed ();
    }
    scfParent->vbufmgr = NULL;
  }
}

void csBCTerrObject::SetupVertexBuffer (iVertexBuffer *&vbuf1, iVertexBuffer *&vbuf2)
{
  if (!vbuf1)
  {
    if (!vbufmgr)
    {
      iGraphics3D* g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
      vbufmgr = g3d->GetVertexBufferManager ();
      g3d->DecRef ();
      vbufmgr->AddClient (&scfiVertexBufferManagerClient);
    }
    vbuf1 = vbufmgr->CreateBuffer (1);
    vbuf2 = vbuf1;
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// csBCTerrObjectFactory
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
SCF_IMPLEMENT_IBASE (csBCTerrObjectFactory)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iBCTerrFactoryState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBCTerrObjectFactory::BCTerrFactoryState)
  SCF_IMPLEMENTS_INTERFACE (iBCTerrFactoryState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csBCTerrObjectFactory::csBCTerrObjectFactory (iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiBCTerrFactoryState);
  csBCTerrObjectFactory::object_reg = object_reg;
  logparent = NULL;
  owners = 0;

  LOD_Levels = 0;
  blocksize.Set (0,0);
  focus.Set (0,0,0);
  edge_res = 4;
  height_multiplier = 0;
  initialized = false;
  LOD_Distance = NULL;
  LOD_UV = NULL;
  LOD_Mesh_Numbers = NULL;
  Shared_Meshes = NULL;
  owners = NULL;
  ResetOwner ();
  time = 0;
  default_mat = NULL;
  sys_distance = 20000;
  start_sys = 10000;
  object_list = NULL;
  num_objects = 0;
  time_count = 0;
  free_lods = false;
}

csBCTerrObjectFactory::~csBCTerrObjectFactory ()
{
  int i;
  if (LOD_Levels)
  {
    for (i = 0; i < LOD_Levels; i++)
    {
      delete [] Shared_Meshes[i];
      delete owners[i];
      Shared_Meshes[i] = NULL;
      owners[i] = NULL;
    }
    delete [] Shared_Meshes;
    delete [] LOD_Distance;
    delete [] LOD_UV;
    delete [] LOD_Mesh_Numbers;
    delete [] owners;
  }
  if (default_mat) default_mat->DecRef ();
  for (i = 0; i < num_objects; i++)
  {
    object_list[i] = NULL;
  }
  if (object_list) delete [] object_list;
}

void csBCTerrObjectFactory::AddTerrObject (csBCTerrObject* obj)
{
  int i;
  num_objects++;
  csBCTerrObject** new_list;
  new_list = new csBCTerrObject*[num_objects];
  for (i = 0; i < (num_objects - 1); i++)
  {    
    new_list[i] = object_list[i];
    object_list[i] = NULL;
  }
  new_list[num_objects - 1] = obj;
  delete [] object_list;
  object_list = new_list;
}

void csBCTerrObjectFactory::GetXZFromLOD ( int level, int &x_vert, int &z_vert)
{
  float x;
  int count;
  x = LOD_UV[level].x;
  count = 0;
  if (x > 0)
  {
    while (x < 1.0)
    {
      count++;
      x += LOD_UV[level].x;
    }
  }
  x_vert = count;

  x = LOD_UV[level].y;
  count = 0;
  if (x > 0)
  {
    while (x < 1.0)
    {
      count++;
      x += LOD_UV[level].y;
    }
  }
  z_vert = count;
}

void csBCTerrObjectFactory::AddLOD (float distance, int inc)
{
  csVector2* new_LOD_UV;
  float* new_LOD_Distance;
  float d;
  csVector2 uv;
  LOD_Levels++;
  int i,j;
  bool added;
  if ( (blocksize.x <= 0) || (blocksize.y <= 0) ) return;
  // create new arrays
  new_LOD_UV = new csVector2[LOD_Levels];
  new_LOD_Distance = new float[LOD_Levels];

  // Determine new values
  // y in vectors is actually along z axis
  if (inc < 0) inc = -inc;
  d = 1.0 / inc;
  if (d > 0.25) d = 0.25f;
  if (d <= 0.0) d = 0.1f; // floating point error?
  //d = 0.25f;
  uv.x = d;
  uv.y = d;
  if (distance > max_lod_distance)
    distance = max_lod_distance;
  d = distance * distance;
  added = false;
  // Copy old + newvalues into new arrays
  //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Factory", "LOD_Distance %f", distance);
  if (LOD_Levels == 1)
  {
    new_LOD_UV[0] = uv;
    new_LOD_Distance[0] = d;
  } else
  {
    i = 0;
    while ( (LOD_Distance[i] < d) && (i < (LOD_Levels - 1))  )
    {
      i++;
    }
    for ( j = 0; j < i; j++)
    {
      new_LOD_UV[j] = LOD_UV[j];
      new_LOD_Distance[j] = LOD_Distance[j];
    }

    new_LOD_UV[i] = uv;
    new_LOD_Distance[i] = d;

    for (j = i + 1; j < (LOD_Levels); j++)
    {
      new_LOD_UV[j] = LOD_UV[i];
      new_LOD_Distance[j] = LOD_Distance[i];
      i++;
    }
  }
  delete [] LOD_Distance;
  delete [] LOD_UV;
  LOD_Distance = new_LOD_Distance;
  LOD_UV = new_LOD_UV;
  /*for (i = 0; i < LOD_Levels; i++)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Factory", "LOD_Distance %f", LOD_Distance[i]);
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Factory", "LOD_U %f V %f", LOD_UV[i].x, LOD_UV[i].y);

  }*/
}

/*
 * New Instance
 * Creates a new Bezier Curve Terrain Object if: UV & Distances are defined;
 * Also is responsable for setting iniitialized & creating shared meshes
 */
csPtr<iMeshObject> csBCTerrObjectFactory::NewInstance ()
{
  if ( (LOD_Levels > 0) && (LOD_Distance && LOD_UV) && (default_mat) )
  {
    if (initialized == false)
    {
      //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Factory","New Instance Creation");
      int i, x, j, x_verts, z_verts;
      float longest;
      int coverage;
      if (blocksize.x > blocksize.y) longest = blocksize.x; else longest = blocksize.y;
      if (longest <= 0) return NULL;
      longest = longest * longest;
      Shared_Meshes = new csSharedLODMesh*[LOD_Levels];
      LOD_Mesh_Numbers = new int[LOD_Levels];
      owners = new csBCLODOwner*[LOD_Levels];
      for ( i = 0; i < LOD_Levels; i++)
      {
        coverage = (int)(LOD_Distance[i] / (float)longest);
        if (coverage < 1) coverage = 1;
        coverage = coverage  + coverage - 1;
        // # of meshes = total area - inside area
        x = coverage - 2;
        if (x > 0) x = x * x; else x = 0;
        LOD_Mesh_Numbers[i] = (coverage * coverage) - x;
        csSharedLODMesh* list;
        
        GetXZFromLOD (i, x_verts, z_verts);
        //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Factory","LOD_MEsh Number %d", LOD_Mesh_Numbers[i]);
        if (LOD_Mesh_Numbers[i] > 30) LOD_Mesh_Numbers[i] = 30;
        if (LOD_Mesh_Numbers[i] < 4) LOD_Mesh_Numbers[i] = 4;
        list = new csSharedLODMesh[ LOD_Mesh_Numbers[i] ];
        for (j = 0; j < LOD_Mesh_Numbers[i]; j++)
        {
          //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Factory","create mesh call");
          list[j].CreateMesh (x_verts, z_verts, edge_res, i);
          //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Factory","after create mesh call");
        }
        Shared_Meshes[i] = list;
        owners[i] = new csBCLODOwner (LOD_Mesh_Numbers[i]);
      }
      initialized = true;
    }
    //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Factory","Passed New Instance Creation");

    csBCTerrObject* pTerrObj = new csBCTerrObject (object_reg, this);
    AddTerrObject (pTerrObj);
    return csPtr<iMeshObject> (pTerrObj);
  }
  else
  {
    return NULL;
  }
}

csSharedLODMesh* csBCTerrObjectFactory::GetSharedMesh ( int level, csBCTerrBlock *owner)
{
  // cpu considerations
  if (time < cpu_time_limit) return NULL;
  time = 0;
  int i;
  // find an free LOD mesh
  if (free_lods)
  {
    for (i = 0; i < num_objects; i++)
    {
      object_list[i]->FreeSharedLOD (focus);
    }
    free_lods = false;
  }
  //level -= user_lod_start;
  if (level >= 0)
  {    
    csBCLODOwner* list = owners[level];
    for ( i = 0; i < LOD_Mesh_Numbers[level]; i++)
    {
      if (!list->owners[i])
      {
        list->owners[i] = owner;
        csSharedLODMesh* mesh = Shared_Meshes[level];
        return &mesh[i];
      }
    }
  }
  return NULL;
}

void csBCTerrObjectFactory::FreeShared (csBCTerrBlock *owner, int level  )
{
  if ( (level < 0) || (level >= LOD_Levels) ) return;
  csBCTerrBlock** list;
  int i;
  list = owners[level]->owners;
  for (i = 0; i < LOD_Mesh_Numbers[level]; i++)
  {
    if (list[i] == owner) list[i] = NULL;
  }
}

/*
 * CheckShared :
 * interates through owners (1 per frame) checks distance,
 * frees shared LOD if it isn't needed
 */

void csBCTerrObjectFactory::CheckShared ()
{
}

csSharedLODMesh* csBCTerrObjectFactory::CreateFreeMesh ()
{
  int x_verts, z_verts, lod_level;
  csSharedLODMesh* mesh;
  mesh = NULL;
  if (LOD_Levels < 1) return NULL;
  lod_level = LOD_Levels - 1;
  GetXZFromLOD (lod_level, x_verts, z_verts);
  mesh = new csSharedLODMesh;
  mesh->CreateMesh ( x_verts, z_verts, edge_res, lod_level);
  // mesh->free = false;
  //csVector3 *trial = new csVector3[500];
  //delete [] trial;
  return mesh;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// csBCTerrObjectType
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------



SCF_IMPLEMENT_IBASE (csBCTerrObjectType)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBCTerrObjectType::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csBCTerrObjectType)

SCF_EXPORT_CLASS_TABLE (bcterr)
  SCF_EXPORT_CLASS (csBCTerrObjectType, "crystalspace.mesh.object.bcterr",
    "Crystal Space Bezier Curve Terrain Mesh Type")
SCF_EXPORT_CLASS_TABLE_END

csBCTerrObjectType::csBCTerrObjectType (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csBCTerrObjectType::~csBCTerrObjectType ()
{
}

csPtr<iMeshObjectFactory> csBCTerrObjectType::NewFactory()
{
  csBCTerrObjectFactory *pFactory = new csBCTerrObjectFactory (object_reg);
  return csPtr<iMeshObjectFactory> (pFactory);
}

