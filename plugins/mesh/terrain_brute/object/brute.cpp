/*
Copyright (C) 1999-2001 by Jorrit Tyberghein

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
#include "csgeom/vector3.h"
#include "csgeom/segment.h"
#include "csgfx/memimage.h"
#include "csutil/util.h"
#include "iengine/camera.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "igraphic/image.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivideo/rndbuf.h"
#include "ivideo/txtmgr.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "brute.h"
#include "csqsqrt.h"

CS_LEAKGUARD_IMPLEMENT (csTerrBlock)
CS_LEAKGUARD_IMPLEMENT (csTerrainObject)
CS_LEAKGUARD_IMPLEMENT (csTerrainFactory)

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csTerrainObject)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iTerrainObjectState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTerrainObject::eiObjectModel)
  SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csTerrainObject::PolyMesh)
  SCF_IMPLEMENTS_INTERFACE (iPolygonMesh)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTerrainObject::eiTerrainObjectState)
  SCF_IMPLEMENTS_INTERFACE (iTerrainObjectState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csTerrBlock::csTerrBlock (csTerrainObject *terr)
{
  parent = 0;
  child = 0;
  children[0] = 0;
  children[1] = 0;
  children[2] = 0;
  children[3] = 0;
  neighbours[0] = 0;
  neighbours[1] = 0;
  neighbours[2] = 0;
  neighbours[3] = 0;

  vertex_data = 0;
  morphvertex_data = 0;
  normal_data = 0;
  morphnormal_data = 0;
  texcoord_data = 0;
  color_data = 0;

  built = false;

  svcontext.AttachNew (new csShaderVariableContext ());

  detach = false;

  csTerrBlock::terr = terr;
}

csTerrBlock::~csTerrBlock ()
{
  delete [] vertex_data;
  delete [] morphvertex_data;
  delete [] normal_data;
  delete [] morphnormal_data;
  delete [] texcoord_data;
}

void csTerrBlock::Detach ()
{
  detach = true;

  if (!IsLeaf ())
  {
    children[0]->Detach ();
    children[1]->Detach ();
    children[2]->Detach ();
    children[3]->Detach ();
    children[0] = 0;
    children[1] = 0;
    children[2] = 0;
    children[3] = 0;
  }

  if (parent)
  {
    if (neighbours[0])
    {
      if (child == 0 || child == 1)
      {
        neighbours[0]->neighbours[3] = parent;
      }
    }
    if (neighbours[1])
    {
      if (child == 1 || child == 3)
      {
        neighbours[1]->neighbours[2] = parent;
      }
    }
    if (neighbours[2])
    {
      if (child == 0 || child == 2)
      {
        neighbours[2]->neighbours[1] = parent;
      }
    }
    if (neighbours[3])
    {
      if (child == 2 || child == 3)
      {
        neighbours[3]->neighbours[0] = parent;
      }
    }
  }
}

void csTerrBlock::SetupMesh ()
{
  res = terr->GetBlockResolution () + 1;

  delete[] vertex_data;
  vertex_data = new csVector3[res * res];
  delete[] texcoord_data;
  texcoord_data = new csVector2[res * res];
  delete[] normal_data;
  normal_data = new csVector3[res * res];
  delete[] color_data;
  color_data = new csVector3[res * res];

  if (!terrasampler)
  {
    terrasampler = terr->terraformer->GetSampler (
      csBox2 (center.x - size / 2.0, center.z - size / 2.0, 
      center.x + size / 2.0, center.z + size / 2.0), res);
  }

  memcpy (vertex_data, terrasampler->SampleVector3 (terr->vertices_name),
    res * res * sizeof (csVector3));
  memcpy (normal_data, terrasampler->SampleVector3 (terr->normals_name),
    res * res * sizeof (csVector3));
  memcpy (texcoord_data, terrasampler->SampleVector2 (terr->texcoords_name),
    res * res * sizeof (csVector2));
  terrasampler->Cleanup ();


  bbox.Empty ();
  int i, j;
  for (j = 0; j < res; ++j)
  {
    for (i = 0; i < res; ++i)
    {
      const int pos = i + j * res;
      bbox.AddBoundingVertexSmart (vertex_data[pos]);
      color_data[pos] = csVector3(0.5, 0.5, 0.5);
    }
  }
  built = true;
}

void FillEdge (bool halfres, int res, uint16* indices, int &indexcount,
               int offs, int xadd, int zadd)
{
  int x;
  // This triangulation scheme could probably be better.
  for (x=0; x<res; x+=2)
  {
    if (x>0)
    {
      indices[indexcount++] = offs+x*xadd+zadd;
      indices[indexcount++] = offs+x*xadd;
    }
    else
    {
      indices[indexcount++] = offs+x*xadd;
      indices[indexcount++] = offs+x*xadd;
      indices[indexcount++] = offs+x*xadd;
    }

    indices[indexcount++] = offs+(x+1)*xadd+zadd;
    if (!halfres)
      indices[indexcount++] = offs+(x+1)*xadd;
    else
      indices[indexcount++] = offs+x*xadd;

    if (x<res-2)
    {
      indices[indexcount++] = offs+(x+2)*xadd+zadd;
      indices[indexcount++] = offs+(x+2)*xadd;
    }
    else
    {
      indices[indexcount++] = offs+(x+2)*xadd;
      indices[indexcount++] = offs+(x+2)*xadd;
      indices[indexcount++] = offs+(x+2)*xadd;
    }
  }
}

void csTerrBlock::Split ()
{
  int i;
  for (i=0; i<4; i++)
  {
    if (neighbours[i] && neighbours[i]->size>size && neighbours[i]->IsLeaf ())
      neighbours[i]->Split ();
  }

  children[0].AttachNew (new csTerrBlock (terr));
  children[0]->parent = this;
  children[0]->size = size/2.0;
  children[0]->center = center + csVector3 (-size/4.0, 0, -size/4.0);
  children[0]->material = material;
  children[0]->child = 0;

  children[1].AttachNew (new csTerrBlock (terr));
  children[1]->parent = this;
  children[1]->size = size/2.0;
  children[1]->center = center + csVector3 ( size/4.0, 0, -size/4.0);
  children[1]->material = material;
  children[1]->child = 1;

  children[2].AttachNew (new csTerrBlock (terr));
  children[2]->parent = this;
  children[2]->size = size/2.0;
  children[2]->center = center + csVector3 (-size/4.0, 0,  size/4.0);
  children[2]->material = material;
  children[2]->child = 2;

  children[3].AttachNew (new csTerrBlock (terr));
  children[3]->parent = this;
  children[3]->size = size/2.0;
  children[3]->center = center + csVector3 ( size/4.0, 0,  size/4.0);
  children[3]->material = material;
  children[3]->child = 3;

  if (neighbours[0])
    if (!neighbours[0]->IsLeaf ())
    {
      children[0]->neighbours[0] = neighbours[0]->children[2];
      children[1]->neighbours[0] = neighbours[0]->children[3];
      children[0]->neighbours[0]->neighbours[3] = children[0];
      children[1]->neighbours[0]->neighbours[3] = children[1];
    }
    else
    {
      children[0]->neighbours[0] = neighbours[0];
      children[1]->neighbours[0] = neighbours[0];
      neighbours[0]->neighbours[3] = this;
    }
  if (neighbours[1])
    if (!neighbours[1]->IsLeaf ())
    {
      children[1]->neighbours[1] = neighbours[1]->children[0];
      children[3]->neighbours[1] = neighbours[1]->children[2];
      children[1]->neighbours[1]->neighbours[2] = children[1];
      children[3]->neighbours[1]->neighbours[2] = children[3];
    }
    else
    {
      children[1]->neighbours[1] = neighbours[1];
      children[3]->neighbours[1] = neighbours[1];
      neighbours[1]->neighbours[2] = this;
    }
  if (neighbours[2])
    if (!neighbours[2]->IsLeaf ())
    {
      children[0]->neighbours[2] = neighbours[2]->children[1];
      children[2]->neighbours[2] = neighbours[2]->children[3];
      children[0]->neighbours[2]->neighbours[1] = children[0];
      children[2]->neighbours[2]->neighbours[1] = children[2];
    }
    else
    {
      children[0]->neighbours[2] = neighbours[2];
      children[2]->neighbours[2] = neighbours[2];
      neighbours[2]->neighbours[1] = this;
    }
  if (neighbours[3])
    if (!neighbours[3]->IsLeaf ())
    {
      children[2]->neighbours[3] = neighbours[3]->children[0];
      children[3]->neighbours[3] = neighbours[3]->children[1];
      children[2]->neighbours[3]->neighbours[0] = children[2];
      children[3]->neighbours[3]->neighbours[0] = children[3];
    }
    else
    {
      children[2]->neighbours[3] = neighbours[3];
      children[3]->neighbours[3] = neighbours[3];
      neighbours[3]->neighbours[0] = this;
    }
  children[0]->neighbours[1] = children[1];
  children[0]->neighbours[3] = children[2];

  children[1]->neighbours[2] = children[0];
  children[1]->neighbours[3] = children[3];

  children[2]->neighbours[0] = children[0];
  children[2]->neighbours[1] = children[3];

  children[3]->neighbours[0] = children[1];
  children[3]->neighbours[2] = children[2];

  /*terr->builder->BuildBlock (children[0]);
  terr->builder->BuildBlock (children[1]);
  terr->builder->BuildBlock (children[2]);
  terr->builder->BuildBlock (children[3]);*/
  children[0]->SetupMesh ();
  children[1]->SetupMesh ();
  children[2]->SetupMesh ();
  children[3]->SetupMesh ();
}

void csTerrBlock::Merge ()
{
  if (IsLeaf ())
    return;
  if (neighbours[0] && (!neighbours[0]->IsLeaf () && 
                       (!neighbours[0]->children[2]->IsLeaf () ||
                        !neighbours[0]->children[3]->IsLeaf ())))
      return;
  if (neighbours[1] && (!neighbours[1]->IsLeaf () && 
                       (!neighbours[1]->children[0]->IsLeaf () ||
                        !neighbours[1]->children[2]->IsLeaf ())))
    return;
  if (neighbours[2] && (!neighbours[2]->IsLeaf () && 
                       (!neighbours[2]->children[1]->IsLeaf () ||
                        !neighbours[2]->children[3]->IsLeaf ())))
    return;
  if (neighbours[3] && (!neighbours[3]->IsLeaf () && 
                       (!neighbours[3]->children[0]->IsLeaf () ||
                        !neighbours[3]->children[1]->IsLeaf ())))
    return;

  children[0]->Detach ();
  children[1]->Detach ();
  children[2]->Detach ();
  children[3]->Detach ();
  children[0] = 0;
  children[1] = 0;
  children[2] = 0;
  children[3] = 0;
}

void csTerrBlock::CalcLOD (iRenderView *rview)
{
  int res = terr->GetBlockResolution ();

  //csVector3 cam = rview->GetCamera ()->GetTransform ().GetOrigin ();
  csVector3 cam = terr->tr_o2c.GetOrigin ();
  csBox3 cambox (bbox.Min ()-cam, bbox.Max ()-cam);
  /*csVector3 radii = (bbox.Max ()-bbox.Min ())*0.5;
  radii *= (1.0/res)*terr->lod_lcoeff;
  float maxradius = MAX(MAX(radii.x, radii.y), radii.z);
  cam.x *= maxradius/radii.x;
  cam.y *= maxradius/radii.y;
  cam.z *= maxradius/radii.z;
  radii.x = radii.x;
  if (cam.SquaredNorm ()<maxradius*maxradius &&
      size > terr->block_minsize)*/
  float splitdist = size*terr->lod_lcoeff/res;
  if (cambox.SquaredOriginDist()<splitdist*splitdist &&
    size > terr->block_minsize)
  {
    if (IsLeaf ())
      Split ();
  }
  else
  {
    if (!IsLeaf ())
      Merge ();
  }
  if (!IsLeaf ())
    for (int i=0; i<4; i++)
      children[i]->CalcLOD (rview);
}

void csTerrBlock::DrawTest (iGraphics3D* g3d,
			    iRenderView *rview, uint32 frustum_mask,
                            csReversibleTransform &transform)
{
  if (!built)
    return;

  int res = terr->GetBlockResolution ();

  if (!IsLeaf () && children[0]->built &&
                    children[1]->built &&
                    children[2]->built &&
                    children[3]->built)
  {
    children[0]->DrawTest (g3d, rview, frustum_mask, transform);
    children[1]->DrawTest (g3d, rview, frustum_mask, transform);
    children[2]->DrawTest (g3d, rview, frustum_mask, transform);
    children[3]->DrawTest (g3d, rview, frustum_mask, transform);
    return;
  }

  if (!mesh_vertices)
  {
    int num_mesh_vertices = (res+1)*(res+1);
    mesh_vertices = 
      g3d->CreateRenderBuffer (
      sizeof(csVector3)*num_mesh_vertices, CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
      3);
    mesh_vertices->CopyToBuffer (vertex_data,
      sizeof(csVector3)*num_mesh_vertices);
    //delete[] vertex_data;@@@ CD
    //vertex_data = 0;

    /*mesh_morphvertices = 
      g3d->CreateRenderBuffer (
      sizeof(csVector3)*num_mesh_vertices, CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
      3, false);
    mesh_morphvertices->CopyToBuffer (morphvertex_data,
      sizeof(csVector3)*num_mesh_vertices);
    delete[] morphvertex_data;
    morphvertex_data = 0;*/

    mesh_normals = 
      g3d->CreateRenderBuffer (sizeof(csVector3)*num_mesh_vertices,
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
      3);
    mesh_normals->CopyToBuffer (normal_data,
      sizeof(csVector3)*num_mesh_vertices);
    delete[] normal_data;
    normal_data = 0;

    /*mesh_morphnormals = 
      g3d->CreateRenderBuffer (
      sizeof(csVector3)*num_mesh_vertices, CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
      3, false);
    mesh_morphnormals->CopyToBuffer (morphnormal_data,
      sizeof(csVector3)*num_mesh_vertices);
    delete[] morphnormal_data;
    morphnormal_data = 0;*/

    mesh_texcoords = 
    g3d->CreateRenderBuffer (sizeof(csVector2)*num_mesh_vertices,
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
      2);
    mesh_texcoords->CopyToBuffer (texcoord_data,
      sizeof(csVector2)*num_mesh_vertices);
    delete[] texcoord_data;
    texcoord_data = 0;

    mesh_colors = 
      g3d->CreateRenderBuffer (sizeof(csVector3)*num_mesh_vertices,
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
      3);
    mesh_colors->CopyToBuffer (color_data,
      sizeof(csVector3)*num_mesh_vertices);
    delete[] color_data;
    color_data = 0;

    csRef<csShaderVariable> sv;
    sv = svcontext->GetVariableAdd (terr->vertices_name);
    sv->SetValue (mesh_vertices);
    /*sv = svcontext->GetVariableAdd (terr->morphvertices_name);
    sv->SetValue (mesh_morphvertices);*/
    sv = svcontext->GetVariableAdd (terr->normals_name);
    sv->SetValue (mesh_normals);
    /*sv = svcontext->GetVariableAdd (terr->morphnormals_name);
    sv->SetValue (mesh_morphnormals);*/
    sv = svcontext->GetVariableAdd (terr->texcoords_name);
    sv->SetValue (mesh_texcoords);
    sv = svcontext->GetVariableAdd (terr->colors_name);
    sv->SetValue (mesh_colors);
  }

  //csVector3 cam = rview->GetCamera ()->GetTransform ().GetOrigin ();
  csVector3 cam = transform.GetOrigin ();

  int clip_portal, clip_plane, clip_z_plane;
  if (!rview->ClipBBox (terr->planes, frustum_mask,
    bbox, clip_portal, clip_plane, clip_z_plane))
    return;

  csBox3 cambox (bbox.Min ()-cam, bbox.Max ()-cam);
  bool baseonly = cambox.SquaredOriginDist () > 
                  terr->lod_distance*terr->lod_distance;

  int idx = ((neighbours[0] && neighbours[0]->size>size)?1:0)+
            (((neighbours[1] && neighbours[1]->size>size)?1:0)<<1)+
            (((neighbours[2] && neighbours[2]->size>size)?1:0)<<2)+
            (((neighbours[3] && neighbours[3]->size>size)?1:0)<<3);
  
  csRef<csShaderVariable> sv;
  sv = svcontext->GetVariableAdd (terr->indices_name);
  sv->SetValue (terr->mesh_indices[idx]);

  for (size_t i=0; i<=(baseonly?0:terr->palette.Length ()); ++i)
  {
    if ((i > 0) && !IsMaterialUsed (i - 1)) continue;

    bool meshCreated;
    csRenderMesh*& rm = terr->rmHolder.GetUnusedMesh (meshCreated,
      rview->GetCurrentFrameNumber ());
    rm->meshtype = CS_MESHTYPE_TRIANGLESTRIP;
    rm->variablecontext = svcontext;
    rm->clip_portal = clip_portal;
    rm->clip_plane = clip_plane;
    rm->clip_z_plane = clip_z_plane;
    rm->object2camera = transform;
    rm->indexstart = 0;
    rm->indexend = terr->numindices[idx];
    if (i==0)
      rm->material = material;
    else
      rm->material = terr->palette[i-1];
    terr->returnMeshes->Push (rm);
  }
}

bool csTerrBlock::IsMaterialUsed (int index)
{
  if ((materialsChecked.Length() <= (size_t)index) || 
    (!materialsChecked[index]))
  {
    materialsChecked.SetLength (index + 1);
    materialsChecked[index] = true;
    materialsUsed.SetLength (index + 1);

    csBox2 heightmapSpace (center.x - size / 2.0, 
      center.z - size / 2.0, center.x + size / 2.0, 
      center.z + size / 2.0);
    const csBox2& terrRegion = terr->region;

    float wm = ((float)(terr->materialMapW - 1)) /
                (terrRegion.MaxX() - terrRegion.MinX());
    float hm = ((float)(terr->materialMapH - 1)) /
                (terrRegion.MaxY() - terrRegion.MinY());
    int mmLeft = 
      (int)floor ((heightmapSpace.MinX() - terrRegion.MinX()) * wm);
    int mmTop = 
      (int)floor ((heightmapSpace.MinY() - terrRegion.MinY()) * hm);
    int mmRight = 
      (int)ceil ((heightmapSpace.MaxX() - terrRegion.MinX()) * wm);
    int mmBottom = 
      (int)ceil ((heightmapSpace.MaxY() - terrRegion.MinY()) * hm);

    // Jorrit: for some reason we must cap mmRight and mmBottom.
    if (mmRight >= terr->materialMapW) mmRight = terr->materialMapW-1;
    if (mmBottom >= terr->materialMapH) mmBottom = terr->materialMapH-1;

    bool matUsed = false;
    for (int y = mmTop; y <= mmBottom; y++)
    {
      int ofs = y * terr->materialMapW;
      for (int x = mmLeft; x <= mmRight; x++)
      {
	if (terr->materialMap[ofs + x] == index)
	{
	  matUsed = true;
	  break;
	}
      }
      if (matUsed) break;
    }

    materialsUsed[index] = matUsed;
  }
  return materialsUsed[index];
}

// ---------------------------------------------------------------

void csTerrainObject::SetupPolyMeshData ()
{
  if (polymesh_valid) return;
  SetupObject ();
  polymesh_valid = true;
  delete[] polymesh_vertices;
  delete[] polymesh_triangles;
  delete[] polymesh_polygons; polymesh_polygons = 0;

  int res = cd_resolution;
  csRef<iTerraSampler> terrasampler = terraformer->GetSampler (
      csBox2 (rootblock->center.x - rootblock->size / 2.0,
      	      rootblock->center.z - rootblock->size / 2.0, 
	      rootblock->center.x + rootblock->size / 2.0,
	      rootblock->center.z + rootblock->size / 2.0), res);
  polymesh_vertices = new csVector3 [res * res];
  polymesh_vertex_count = res * res;
  memcpy (polymesh_vertices, terrasampler->SampleVector3 (vertices_name),
    res * res * sizeof (csVector3));
  terrasampler->Cleanup ();

  polymesh_tri_count = 2 * (res-1) * (res-1);
  polymesh_triangles = new csTriangle [polymesh_tri_count];

  int x, y;
  csTriangle* tri = polymesh_triangles;
  for (y = 0 ; y < res-1 ; y++)
  {
    int yr = y * res;
    for (x = 0 ; x < res-1 ; x++)
    {
      (tri++)->Set (yr + x, yr+res + x, yr + x+1);
      (tri++)->Set (yr + x+1, yr+res + x, yr+res + x+1);
    }
  }
}

void csTerrainObject::PolyMesh::Cleanup ()
{
}

csMeshedPolygon* csTerrainObject::PolyMesh::GetPolygons ()
{
  terrain->SetupPolyMeshData ();
  if (!terrain->polymesh_polygons)
  {
    int pcnt = terrain->polymesh_tri_count;
    terrain->polymesh_polygons = new csMeshedPolygon [pcnt];
    csTriangle* tris = terrain->polymesh_triangles;
    int i;
    for (i = 0 ; i < pcnt ; i++)
    {
      terrain->polymesh_polygons[i].num_vertices = 3;
      terrain->polymesh_polygons[i].vertices = &tris[i].a;
    }
  }

  return terrain->polymesh_polygons;
}

int csTerrainObject::PolyMesh::GetVertexCount ()
{
  terrain->SetupPolyMeshData ();
  return terrain->polymesh_vertex_count;
}

csVector3* csTerrainObject::PolyMesh::GetVertices ()
{
  terrain->SetupPolyMeshData ();
  return terrain->polymesh_vertices;
}

int csTerrainObject::PolyMesh::GetPolygonCount ()
{
  terrain->SetupPolyMeshData ();
  return terrain->polymesh_tri_count;
}

int csTerrainObject::PolyMesh::GetTriangleCount ()
{
  terrain->SetupPolyMeshData ();
  return terrain->polymesh_tri_count;
}

csTriangle* csTerrainObject::PolyMesh::GetTriangles ()
{
  terrain->SetupPolyMeshData ();
  return terrain->polymesh_triangles;
}

//----------------------------------------------------------------------


csTerrainObject::csTerrainObject (iObjectRegistry* object_reg,
                                    iMeshObjectFactory *pFactory)
                                    : returnMeshesHolder (false)
{
  SCF_CONSTRUCT_IBASE (0)
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiTerrainObjectState);
  csTerrainObject::object_reg = object_reg;
  csTerrainObject::pFactory = pFactory;
  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);

  scfiObjectModel.SetPolygonMeshBase (&scfiPolygonMesh);
  scfiObjectModel.SetPolygonMeshColldet (&scfiPolygonMesh);
  scfiObjectModel.SetPolygonMeshViscull (0);
  scfiObjectModel.SetPolygonMeshShadows (0);
  scfiPolygonMesh.SetTerrain (this);

  polymesh_valid = false;
  polymesh_vertices = 0;
  polymesh_triangles = 0;
  polymesh_polygons = 0;
  cd_resolution = 256;

  logparent = 0;
  initialized = false;

  region = ((csTerrainFactory*)pFactory)->samplerRegion;
  rootblock = 0;
  vis_cb = 0;
  vbufmgr = 0;
    
  block_res = 32;

  lod_distance = 200;

  lod_lcoeff = 16;
  lod_qcoeff = 0;
  block_maxsize = region.MaxX () - region.MinX ();
  block_minsize = block_maxsize;

  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.shared.stringset", iStringSet);
  
  indices_name = strings->Request ("indices");
  vertices_name = strings->Request ("vertices");
  morphvertices_name = strings->Request ("morph source vertices");
  normals_name = strings->Request ("normals");
  morphnormals_name = strings->Request ("morph source normals");
  texcoords_name = strings->Request ("texture coordinates");
  morphval_name = strings->Request ("morph amount");
  colors_name = strings->Request ("colors");

  //terr_func = &((csTerrainFactory*)pFactory)->terr_func;
  terraformer = ((csTerrainFactory*)pFactory)->terraformer;

  /*builder = new csBlockBuilder (this);
  buildthread = csThread::Create (builder);
  buildthread->Start ();*/
}

csTerrainObject::~csTerrainObject ()
{
  //builder->Stop ();
  if (vis_cb) vis_cb->DecRef ();
  delete[] polymesh_vertices;
  delete[] polymesh_triangles;
}

void csTerrainObject::FireListeners ()
{
  size_t i;
  for (i = 0 ; i < listeners.Length () ; i++)
    listeners[i]->ObjectModelChanged (&scfiObjectModel);
}

void csTerrainObject::AddListener (iObjectModelListener *listener)
{
  RemoveListener (listener);
  listeners.Push (listener);
}

void csTerrainObject::RemoveListener (iObjectModelListener *listener)
{
  int idx = listeners.Find (listener);
  if (idx == -1) return ;
  listeners.DeleteIndex (idx);
}

void csTerrainObject::SetupObject ()
{
  if (!initialized)
  {
    initialized = true;

    csRef<iGraphics3D> g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);

    for (int t=0; t<=1; t++)
    {
      for (int r=0; r<=1; r++)
      {
        for (int l=0; l<=1; l++)
        {
          for (int b=0; b<=1; b++)
          {
            int idx = t+(r<<1)+(l<<2)+(b<<3);
            mesh_indices[idx] = 
	      g3d->CreateIndexRenderBuffer (
              sizeof(unsigned int)*block_res*block_res*2*3, 
              CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_SHORT,
              0, (block_res+1) * (block_res+1) - 1);
            uint16 *indices = 
              (uint16*)mesh_indices[idx]->Lock (CS_BUF_LOCK_NORMAL);

            numindices[idx] = 0;
            int x, z;
            for (z=1; z<(block_res-1); z++)
            {
              indices[numindices[idx]++] = 1+z*(block_res+1);
              indices[numindices[idx]++] = 1+z*(block_res+1);
              for (x=1; x<(block_res); x++)
              { 
                indices[numindices[idx]++] = x+(z+0)*(block_res+1);
                indices[numindices[idx]++] = x+(z+1)*(block_res+1);
              }
              indices[numindices[idx]++] = x-1+(z+1)*(block_res+1);
              indices[numindices[idx]++] = x-1+(z+1)*(block_res+1);
            }

            FillEdge (t==1,
              block_res, indices, numindices[idx], 
              0, 1, (block_res+1));

            FillEdge (r==1,
              block_res, indices, numindices[idx],
              block_res, (block_res+1), -1);

            FillEdge (l==1,
              block_res, indices, numindices[idx], 
              block_res*(block_res+1), -(block_res+1), 1);

            FillEdge (b==1, 
              block_res, indices, numindices[idx],
              block_res*(block_res+1)+block_res, -1, -(block_res+1));

            mesh_indices[idx]->Release ();
          }
        }
      }
    }

    rootblock.AttachNew (new csTerrBlock (this));
    rootblock->material = matwrap;
    csVector2 center = (region.Max()+region.Min())*0.5;
    rootblock->center = csVector3 (center.x, 0, center.y);
    rootblock->size = block_maxsize;
    rootblock->SetupMesh ();
    global_bbox = rootblock->bbox;
  }
}

bool csTerrainObject::SetMaterialPalette (
  const csArray<iMaterialWrapper*>& pal)
{
  palette.SetLength (pal.Length());
  for (size_t i = 0; i < pal.Length(); i++)
  {
    palette[i] = pal[i];
  }

  return true;
}

csArray<iMaterialWrapper*> csTerrainObject::GetMaterialPalette ()
{
  return palette;
}

bool csTerrainObject::SetMaterialMap (const csArray<char>& data, int w, int h)
{
  materialMap = data;
  materialMapW = w;
  materialMapH = h;

  csRef<iGraphics3D> g3d = 
    CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  csRef<iStringSet> strings = 
    CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg,
    "crystalspace.shared.stringset", iStringSet);
  csRef<iTextureManager> mgr = g3d->GetTextureManager ();

  csRef<csShaderVariable> lod_var = 
    new csShaderVariable (strings->Request ("texture lod distance"));
  lod_var->SetType (csShaderVariable::VECTOR3);
  lod_var->SetValue (csVector3 (lod_distance, lod_distance, lod_distance));
  matwrap->GetMaterial()->AddVariable (lod_var);

  for (size_t i = 0; i < palette.Length(); i ++) 
  {
    csRef<iImage> alpha = csPtr<iImage> (new csImageMemory (w, h, 
      CS_IMGFMT_ALPHA | CS_IMGFMT_TRUECOLOR));

    csRGBpixel *map = (csRGBpixel *)alpha->GetImageData ();
    int y, x;
    for (y = 0; y < h; y ++) 
    {
      for (x = 0; x < w; x ++) 
      {
        int v = ((unsigned char)data[x + y * w] == i) ? 255 : 0;
        map[x + y * w].Set (v, v, v, v);
      }
    }

    csRef<iTextureHandle> hdl = mgr->RegisterTexture (alpha, CS_TEXTURE_2D);
    csRef<csShaderVariable> var = 
      new csShaderVariable (strings->Request ("splat alpha map"));
    var->SetType (csShaderVariable::TEXTURE);
    var->SetValue (hdl);
    palette[i]->GetMaterial()->AddVariable (var);

    csRef<csShaderVariable> lod_var = 
      new csShaderVariable (strings->Request ("texture lod distance"));
    lod_var->SetType (csShaderVariable::VECTOR3);
    lod_var->SetValue (csVector3 (lod_distance, lod_distance, lod_distance));
    matwrap->GetMaterial()->AddVariable (lod_var);
    palette[i]->GetMaterial()->AddVariable (lod_var);
  }
  return true;
}

bool csTerrainObject::SetMaterialMap (iImage* map)
{
  csArray<char> image_data;
  image_data.SetLength (map->GetSize());
  if (map->GetFormat () & CS_IMGFMT_PALETTED8)
  {
    uint8 *data = (uint8 *)map->GetImageData ();
    for (int i = 0; i < map->GetSize (); i ++)
    {
      image_data[i] = data[i];
    }
  }
  else
  {
    csRGBpixel *data = (csRGBpixel *)map->GetImageData ();
    for (int i = 0; i < map->GetSize (); i ++)
    {
      image_data[i] = data[i].Intensity();
    }
  }
  return SetMaterialMap (image_data, map->GetWidth(), map->GetHeight());
}

csArray<char> csTerrainObject::GetMaterialMap ()
{
  return 0;
}

bool csTerrainObject::SetLODValue (const char* parameter, float value)
{
  if (strcmp (parameter, "splatting distance") == 0)
  {
    lod_distance = value;
    return true;
  }
  else if (strcmp (parameter, "block split distance") == 0)
  {
    lod_lcoeff = value;
    return true;
  }
  else if (strcmp (parameter, "minimum block size") == 0)
  {
    block_minsize = value;
    return true;
  }
  else if (strcmp (parameter, "block resolution") == 0)
  {
    // Make the resolution conform to n^2
    block_res = csLog2 ((int) value);
    block_res = (int) pow (2, block_res);
    return true;
  }
  else if (strcmp (parameter, "cd resolution") == 0)
  {
    cd_resolution = int (value);
    return true;
  }
  return false;
}

float csTerrainObject::GetLODValue (const char* parameter)
{
  if (strcmp (parameter, "splatting distance") == 0)
  {
    return lod_distance;
  }
  else if (strcmp (parameter, "block split distance") == 0)
  {
    return lod_lcoeff;
  }
  else if (strcmp (parameter, "minimum block size") == 0)
  {
    return block_minsize;
  }
  else if (strcmp (parameter, "block resolution") == 0)
  {
    return block_res;
  }
  else if (strcmp (parameter, "cd resolution") == 0)
  {
    return float (cd_resolution);
  }
  return 0;
}


/*static void Perspective (const csVector3& v, csVector2& p, float fov,
                         float sx, float sy)
{
  float iz = fov / v.z;
  p.x = v.x * iz + sx;
  p.y = v.y * iz + sy;
}*/

/*bool csTerrainObject::BBoxVisible (const csBox3& bbox,
                                    iRenderView* rview, iCamera* camera,
                                    int& clip_portal, 
                                    int& clip_plane, 
                                    int& clip_z_plane)
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
    return false;

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

  return rview->ClipBBox (sbox, cbox, clip_portal, 
                                      clip_plane, 
                                      clip_z_plane);
}

void csTerrainObject::TestVisibility (iRenderView* rview)
{

  //iCamera* camera = rview->GetCamera ();
  const csReversibleTransform& camtrans = camera->GetTransform ();
  const csVector3& origin = camtrans.GetOrigin ();
  //quadtree->ComputeVisibility (origin, global_bbox, horizon, CS_HORIZON_SIZE);
}*/


bool csTerrainObject::DrawTest (iRenderView* rview, iMovable* movable, 
                                uint32 frustummask)
{
  if (vis_cb) if (!vis_cb->BeforeDrawing (this, rview)) return false;

  rootblock->CalcLOD (rview);

  returnMeshes = &returnMeshesHolder.GetUnusedMeshes (
    rview->GetCurrentFrameNumber ());
  returnMeshes->Empty ();

  iCamera* cam = rview->GetCamera ();
  tr_o2c = cam->GetTransform ();
  if (!movable->IsFullTransformIdentity ())
    tr_o2c /= movable->GetFullTransform ();

  uint32 frustum_mask;
  rview->SetupClipPlanes (tr_o2c, planes, frustum_mask);

  //rendermeshes.Empty ();
  rootblock->DrawTest (g3d, rview, 0, tr_o2c);

  if (returnMeshes->Length () == 0)
    return false;

  return true;
}

void csTerrainObject::UpdateLighting (iLight**, int,
                                       iMovable*)
{
  // @@@ Can we do something more sensible here?
  return;
}

csRenderMesh** csTerrainObject::GetRenderMeshes (int &n,
                                                 iRenderView* rview,
                                                 iMovable* movable, 
                                                 uint32 frustum_mask)
{
  DrawTest (rview, movable, frustum_mask);
  n = returnMeshes->Length ();
  return returnMeshes->GetArray ();
}

void csTerrainObject::GetObjectBoundingBox (csBox3& bbox, int /*type*/)
{
  SetupObject ();
  bbox = global_bbox;
}

void csTerrainObject::GetRadius (csVector3& rad, csVector3& cent)
{
  csBox3 bbox;
  GetObjectBoundingBox (bbox);
  cent = bbox.GetCenter ();
  rad = (bbox.Max () - bbox.Min ()) * 0.5f;
}

int csTerrainObject::CollisionDetect (iMovable* m, csTransform* transform)
{
  csVector3 p = transform->GetOrigin() - m->GetPosition ();

  csVector3 d;
  terraformer->SampleVector3 (vertices_name, p.x, p.z, d);

  // @@@ The +2 seems pretty ugly, but seems to be needed, at least for
  // walktest
  d.y += 2;
  if (d.y > p.y)
  {
    transform->SetOrigin (d + m->GetPosition ());
    return 1;
  } 
  else 
  {
    return 0;
  }
}

bool csTerrainObject::HitBeam (csTerrBlock* block,
	const csSegment3& seg,
	csVector3& isect, float* pr)
{
  if (csIntersect3::BoxSegment (block->bbox, seg, isect) == -1)
  {
    return false;
  }

  // We have a hit.
  if (block->IsLeaf ())
  {
    // Check the triangles.
    csVector3* vt = block->vertex_data;
    int res = block->res;
    int x, y;
    float tot_dist = csSquaredDist::PointPoint (seg.Start (), seg.End ());
    float dist, temp;
    float itot_dist = 1 / tot_dist;
    dist = temp = tot_dist;
    csVector3 tmp;
    for (y = 0 ; y < res-1 ; y++)
    {
      int yr = y * res;
      for (x = 0 ; x < res-1 ; x++)
      {
        if (csIntersect3::IntersectTriangle (vt[yr+x],
		vt[yr+res+x], vt[yr+x+1], seg, tmp))
	{
          temp = csSquaredDist::PointPoint (seg.Start (), tmp);
          if (temp < dist)
          {
            isect = tmp;
	    dist = temp;
          }
	}
        if (csIntersect3::IntersectTriangle (vt[yr+x+1],
		vt[yr+res+x], vt[yr+res+x+1], seg, tmp))
	{
          temp = csSquaredDist::PointPoint (seg.Start (), tmp);
          if (temp < dist)
          {
            isect = tmp;
	    dist = temp;
          }
	}
      }
    }
    if (pr) *pr = csQsqrt (dist * itot_dist);
    if (dist >= tot_dist)
      return false;
    return true;
  }
  else
  {
    // Check the children.
    if (HitBeam (block->children[0], seg, isect, pr))
      return true;
    if (HitBeam (block->children[1], seg, isect, pr))
      return true;
    if (HitBeam (block->children[2], seg, isect, pr))
      return true;
    if (HitBeam (block->children[3], seg, isect, pr))
      return true;
  }
  return false;
}

bool csTerrainObject::HitBeamOutline (const csVector3& start,
                                       const csVector3& end, 
                                       csVector3& isect, float* pr)
{
  csSegment3 seg (start, end);
  return HitBeam (rootblock, seg, isect, pr);
}

bool csTerrainObject::HitBeamObject (const csVector3& start,
                                      const csVector3& end, 
                                      csVector3& isect, float* pr,
                                      int* polygon_idx)
{
  if (polygon_idx) *polygon_idx = -1;
  csSegment3 seg (start, end);
  return HitBeam (rootblock, seg, isect, pr);
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csTerrainFactory)
SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iTerrainFactoryState)
SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTerrainFactory::eiTerrainFactoryState)
SCF_IMPLEMENTS_INTERFACE (iTerrainFactoryState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTerrainFactory::eiObjectModel)
SCF_IMPLEMENTS_INTERFACE (iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END


csTerrainFactory::csTerrainFactory (iObjectRegistry* object_reg, iMeshObjectType* parent)
{
  SCF_CONSTRUCT_IBASE (0);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiTerrainFactoryState);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiObjectModel);
  csTerrainFactory::object_reg = object_reg;
  logparent = 0;
  brute_type = parent;
				
  /*terraformer = 
    CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg, "terrain", iTerraFormer);*/

  scale = csVector3(1);
}

csTerrainFactory::~csTerrainFactory ()
{
}

csPtr<iMeshObject> csTerrainFactory::NewInstance ()
{
  csTerrainObject* pTerrObj = new csTerrainObject (object_reg, this);
  return csPtr<iMeshObject> (pTerrObj);
}

void csTerrainFactory::SetTerraFormer (iTerraFormer* form)
{
  terraformer = form;
}

iTerraFormer* csTerrainFactory::GetTerraFormer ()
{
  return terraformer;
}

void csTerrainFactory::SetSamplerRegion (const csBox2& region)
{
  samplerRegion = region;
}

const csBox2& csTerrainFactory::GetSamplerRegion ()
{
  return samplerRegion;
}


//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csTerrainObjectType)
SCF_IMPLEMENTS_INTERFACE (iMeshObjectType)
SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTerrainObjectType::eiComponent)
SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csTerrainObjectType)

csTerrainObjectType::csTerrainObjectType (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csTerrainObjectType::~csTerrainObjectType ()
{
}

csPtr<iMeshObjectFactory> csTerrainObjectType::NewFactory()
{
  csTerrainFactory *pFactory = new csTerrainFactory (object_reg, this);
  return csPtr<iMeshObjectFactory> (pFactory);
}
