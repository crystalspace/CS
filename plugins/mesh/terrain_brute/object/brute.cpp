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
#include "csgeom/math.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/vector3.h"
#include "csgeom/segment.h"
#include "csgeom/trimesh.h"
#include "csgeom/trimeshlod.h"
#include "csgfx/memimage.h"
#include "csgfx/renderbuffer.h"
#include "csutil/util.h"
#include "csutil/memfile.h"
#include "csutil/csendian.h"
#include "csutil/csmd5.h"
#include "iengine/camera.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/movable.h"
#include "iengine/sector.h"
#include "iengine/rview.h"
#include "igraphic/image.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivideo/rndbuf.h"
#include "ivideo/txtmgr.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "iutil/object.h"
#include "iutil/cmdline.h"
#include "iutil/verbositymanager.h"
#include "iutil/cache.h"
#include "ivaria/reporter.h"
#include "brute.h"
#include "csqsqrt.h"

CS_LEAKGUARD_IMPLEMENT (csTerrBlock);
CS_LEAKGUARD_IMPLEMENT (csTerrainObject);
CS_LEAKGUARD_IMPLEMENT (csTerrainFactory);

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csTerrainObject)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iObjectModel)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iTerrainObjectState)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iShadowReceiver)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iLightingInfo)
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

SCF_IMPLEMENT_EMBEDDED_IBASE (csTerrainObject::ShadowReceiver)
  SCF_IMPLEMENTS_INTERFACE (iShadowReceiver)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTerrainObject::LightingInfo)
  SCF_IMPLEMENTS_INTERFACE (iLightingInfo)
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
  normal_data = 0;
  texcoord_data = 0;
  color_data = 0;
  last_colorVersion = ~0;

  built = false;

  bufferHolder.AttachNew (new csRenderBufferHolder);

  detach = false;

  csTerrBlock::terr = terr;
}

csTerrBlock::~csTerrBlock ()
{
  delete [] vertex_data;
  delete [] normal_data;
  delete [] texcoord_data;
  delete [] color_data;
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
  color_data = new csColor[res * res];

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
  int i;
  int totres = res * res;
  bbox.StartBoundingBox (vertex_data[0]);
  color_data[0].Set (0.5, 0.5, 0.5);
  for (i = 1 ; i < totres ; i++)
  {
    bbox.AddBoundingVertexSmart (vertex_data[i]);
    color_data[i].Set (0.5, 0.5, 0.5);
  }
  built = true;
  last_colorVersion = ~0;
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
  float splitdist = size*terr->lod_lcoeff / float (res);
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

void csTerrBlock::UpdateBlockColors ()
{
  const csDirtyAccessArray<csColor>& colors = terr->GetStaticColors ();
  int lmres = terr->GetLightMapResolution ();
  int res = terr->GetBlockResolution ();
  const csBox3& gb = terr->global_bbox;
  // @@@
  // Opt by moving some precalced fields in object?
  float lm_minx = ((bbox.MinX ()-gb.MinX ()) / (gb.MaxX ()-gb.MinX ())) * float (lmres);
  float lm_miny = ((bbox.MinZ ()-gb.MinZ ()) / (gb.MaxZ ()-gb.MinZ ())) * float (lmres);
  float lm_maxx = ((bbox.MaxX ()-gb.MinX ()) / (gb.MaxX ()-gb.MinX ())) * float (lmres);
  float lm_maxy = ((bbox.MaxZ ()-gb.MinZ ()) / (gb.MaxZ ()-gb.MinZ ())) * float (lmres);
  if (lm_minx < 0) lm_minx = 0;
  else if (lm_minx > lmres-1) lm_minx = lmres-1;
  if (lm_maxx < lm_minx) lm_maxx = lm_minx;
  else if (lm_maxx > lmres-1) lm_maxx = lmres-1;
  if (lm_miny < 0) lm_miny = 0;
  else if (lm_miny > lmres-1) lm_miny = lmres-1;
  if (lm_maxy < lm_miny) lm_maxy = lm_miny;
  else if (lm_maxy > lmres-1) lm_maxy = lmres-1;
  int x, y;
  csColor* c = color_data;
  for (y = 0 ; y <= res ; y++)
  {
    int lmy = int ((float (y) / float (res)) * (lm_maxy - lm_miny)
      	+ lm_miny);
    lmy *= lmres;
    for (x = 0 ; x <= res ; x++)
    {
      int lmx = int ((float (x) / float (res)) * (lm_maxx - lm_minx)
      	+ lm_minx);
      *c++ = colors[lmy + lmx];
    }
  }
}

void csTerrBlock::UpdateStaticLighting ()
{
  if (IsLeaf ())
  {
    if (last_colorVersion == terr->colorVersion)
      return;
    last_colorVersion = terr->colorVersion;

    int res = terr->GetBlockResolution ();
    int num_mesh_vertices = (res+1)*(res+1);
    if (!color_data)
      color_data = new csColor[num_mesh_vertices];
    UpdateBlockColors ();
    if (mesh_colors)
      mesh_colors->CopyInto (color_data, num_mesh_vertices);
  }
  else
  {
    if (children[0]->built) children[0]->UpdateStaticLighting ();
    if (children[1]->built) children[1]->UpdateStaticLighting ();
    if (children[2]->built) children[2]->UpdateStaticLighting ();
    if (children[3]->built) children[3]->UpdateStaticLighting ();
  }
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
      csRenderBuffer::CreateRenderBuffer (
      num_mesh_vertices, CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
      3);
    mesh_vertices->CopyInto (vertex_data, num_mesh_vertices);
    //delete[] vertex_data;@@@ CD
    //vertex_data = 0;

    mesh_normals = 
      csRenderBuffer::CreateRenderBuffer (num_mesh_vertices,
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
      3);
    mesh_normals->CopyInto (normal_data, num_mesh_vertices);
    delete[] normal_data;
    normal_data = 0;

    mesh_texcoords = 
    csRenderBuffer::CreateRenderBuffer (num_mesh_vertices,
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
      2);
    mesh_texcoords->CopyInto (texcoord_data, num_mesh_vertices);
    delete[] texcoord_data;
    texcoord_data = 0;

    mesh_colors = 
      csRenderBuffer::CreateRenderBuffer (num_mesh_vertices,
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
      3);
    mesh_colors->CopyInto (color_data,num_mesh_vertices);
    if (!terr->staticlighting)
    {
      delete[] color_data;
      color_data = 0;
    }
    bufferHolder->SetRenderBuffer (CS_BUFFER_POSITION, mesh_vertices);
    bufferHolder->SetRenderBuffer (CS_BUFFER_NORMAL, mesh_normals);
    bufferHolder->SetRenderBuffer (CS_BUFFER_TEXCOORD0, mesh_texcoords);
    bufferHolder->SetRenderBuffer (CS_BUFFER_COLOR, mesh_colors);
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
  
  bufferHolder->SetRenderBuffer (CS_BUFFER_INDEX, terr->mesh_indices[idx]);

  for (size_t i=0; i<=(baseonly?0:terr->palette.Length ()); ++i)
  {
    if ((i > 0) && !IsMaterialUsed (i - 1)) continue;

    bool meshCreated;
    csRenderMesh*& rm = terr->rmHolder.GetUnusedMesh (meshCreated,
      rview->GetCurrentFrameNumber ());
    rm->meshtype = CS_MESHTYPE_TRIANGLESTRIP;
    rm->buffers = bufferHolder;
    rm->clip_portal = clip_portal;
    rm->clip_plane = clip_plane;
    rm->clip_z_plane = clip_z_plane;
    rm->object2camera = transform;
    rm->indexstart = 0;
    rm->indexend = terr->numindices[idx];
    if (i==0)
    {
      rm->material = material;
      rm->variablecontext = terr->baseContext;
    } else {
      rm->material = terr->palette[i-1];
      rm->variablecontext = terr->paletteContexts[i-1];
    }
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

    // Slightly overexagerate the heightmap Space so that
    // we don't miss materials due to just being at the border
    // of a block.
    csBox2 heightmapSpace (
    	center.x - size / 2.0 -1.0,
	center.z - size / 2.0 -1.0,
	center.x + size / 2.0 + 1.0,
	center.z + size / 2.0 + 1.0);
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
    // res: and mmLeft/mmTop... :)
    if (mmLeft < 0) mmLeft = 0;
    if (mmTop < 0) mmTop = 0;

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

class csTriangleLODAlgoHM : public csTriangleLODAlgo
{
public:
  csVector3* normals;
  // Per vertex: -1 for corner, 0 for center, 1, 2, 3, 4 for specific edge.
  int* edgedata;
  float min_max_cost;	// 1-max_cost
  csTriangleMesh* mesh;

public:
  virtual ~csTriangleLODAlgoHM () { }

  virtual void CalculateCost (csTriangleVerticesCost* vertices,
  	csTriangleVertexCost* vertex)
  {
    size_t i, j;
    vertex->to_vertex = -1;
    if (vertex->deleted)
    {
      // If the vertex is deleted we have a very high cost.
      // The cost is higher than the maximum cost you can get for
      // a non-deleted vertex. This is to make sure that we get
      // the last non-deleted vertex at the end of the LOD algorithm.
      vertex->cost = 1000000.0;
      return;
    }
    int idx = vertex->idx;
    int edge = edgedata[idx];
    if (edge == -1)
    {
      // If we are on a corner then we can't collapse this vertex
      // at all. Very high cost.
      vertex->cost = 1000000.0;
      return;
    }

    // Calculate the minimum cos(angle) for all normals adjacent to this
    // vertex. That's the worst value for cos(angle) and so that will be
    // the cost of removing this vertex.
    const csVector3& n = normals[idx];
    const csVector3& this_pos = vertex->pos;
    float min_cosa = 1000.0;
    float min_sq_dist = 1000000.;
    for (i = 0 ; i < vertex->con_vertices.Length () ; i++)
    {
      int connected_i = vertex->con_vertices[i];
      float cur_cosa = n * normals[connected_i];
      if (cur_cosa < min_cosa)
      {
        if (cur_cosa < min_max_cost)
	{
	  // We can already stop here. We are too bad.
          vertex->cost = 1000000.0;
          return;
	}
        min_cosa = cur_cosa;
      }

      // Check if we can collapse this edge. We can collapse this
      // edge if this vertex is a center vertex or if it is on the same edge
      // as the other vertex.
      if (edge == 0 || edge == edgedata[connected_i])
      {
	const csVector3& other_pos = vertices->GetVertex (connected_i).pos;
	csTriangle* tris = mesh->GetTriangles ();

        // We will not collapse an edge if it means that the
	// 'y' orientation of the triangle changes (i.e. the
	// normals of the adjacent triangles no longer point
	// upwards). We test this by checking all connected triangles
	// that are not empty, then it projects them to 2D (with x
	// and z) and it will see if their direction changes.
	bool bad = false;
	csVector3 v3[3];
	csVector2 v[3];
	for (j = 0 ; j < vertex->con_triangles.Length () ; j++)
	{
	  csTriangle& tri = tris[vertex->con_triangles[j]];
	  v3[0] = vertices->GetVertex (tri.a).pos; v[0].Set (v3[0].x, v3[0].z);
	  v3[1] = vertices->GetVertex (tri.b).pos; v[1].Set (v3[1].x, v3[1].z);
	  v3[2] = vertices->GetVertex (tri.c).pos; v[2].Set (v3[2].x, v3[2].z);
	  int sameidx;
	  if (tri.a == idx) sameidx = 0;
	  else if (tri.b == idx) sameidx = 1;
	  else sameidx = 2;
	  float dir_before = csMath2::Area2 (v[0], v[1], v[2]);
	  v[sameidx].Set (other_pos.x, other_pos.z);
	  float dir_after = csMath2::Area2 (v[0], v[1], v[2]);
	  bad = (dir_before < -.0001 && dir_after > .0001) ||
	    	(dir_before > .0001 && dir_after < -.0001);
	  if (bad) break;
	}

	if (!bad)
	{
          // We prefer collapsing along the shortest edge.
          float sq_dist = csSquaredDist::PointPoint (this_pos, other_pos);
	  if (sq_dist < min_sq_dist)
	  {
            min_sq_dist = sq_dist;
            vertex->to_vertex = connected_i;
          }
        }
      }
    }

    // If we found no edge to collapse then we can't collapse and we
    // pick a high cost.
    if (vertex->to_vertex == -1)
    {
      vertex->cost = 1000000.0;
      return;
    }

    vertex->cost = 1 - min_cosa;
  }
};

#define CDLODMAGIC	    "CD01" // must be 4 chars!

bool csTerrainObject::ReadCDLODFromCache ()
{
  int i;

  bool verbose = false;
  csRef<iVerbosityManager> verbosemgr (
    CS_QUERY_REGISTRY (object_reg, iVerbosityManager));
  if (verbosemgr) 
    verbose = verbosemgr->CheckFlag ("bruteblock");
  
  csRef<iCommandLineParser> cmdline = CS_QUERY_REGISTRY (
  	object_reg, iCommandLineParser);
  if (cmdline->GetOption ("recalc"))
  {
    static bool reportit = true;
    if (reportit)
    {
      reportit = false;
      csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
	  "crystalspace.mesh.bruteblock",
	  "Forced recalculation of terrain LOD!");
    }
    return false;
  }

  csRef<iEngine> engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!engine) return false;
  iCacheManager* cache_mgr = engine->GetCacheManager ();

  char* cachename = GenerateCacheName ();
  csRef<iDataBuffer> db = cache_mgr->ReadCache ("bruteblock_lod",
      	cachename, 0);
  delete[] cachename;
  if (!db) return false;

  csRef<csMemFile> cf;
  cf.AttachNew (new csMemFile ((char*)db->GetData (), db->GetSize (),
  	    csMemFile::DISPOSITION_IGNORE));

  char header[5];
  cf->Read (header, 4);
  header[4] = 0;
  if (strcmp (header, CDLODMAGIC))
  {
    if (verbose)
      csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
	  "crystalspace.mesh.bruteblock",
    	  "Forced recalculation of terrain LOD: magic number mismatch!");
    return false;	// Mismatch.
  }

  uint32 cache_cd_res;
  cf->Read ((char*)&cache_cd_res, 4);
  cache_cd_res = csConvertEndian (cache_cd_res);
  if ((int)cache_cd_res != cd_resolution)
  {
    if (verbose)
      csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
	  "crystalspace.mesh.bruteblock",
    	  "Forced recalculation of terrain LOD: resolution mismatch!");
    return false;	// Mismatch.
  }

  uint32 ptc;
  cf->Read ((char*)&ptc, 4);
  polymesh_tri_count = (int)csConvertEndian (ptc);
  polymesh_triangles = new csTriangle [polymesh_tri_count];

  for (i = 0 ; i < polymesh_tri_count ; i++)
  {
    uint32 a, b, c;
    cf->Read ((char*)&a, 4); a = csConvertEndian (a);
    cf->Read ((char*)&b, 4); b = csConvertEndian (b);
    cf->Read ((char*)&c, 4); c = csConvertEndian (c);
    polymesh_triangles[i].a = a;
    polymesh_triangles[i].b = b;
    polymesh_triangles[i].c = c;
  }
  return true;
}

void csTerrainObject::WriteCDLODToCache ()
{
  csRef<iEngine> engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!engine) return;
  iCacheManager* cache_mgr = engine->GetCacheManager ();
  if (!cache_mgr) return;

  char* cachename = GenerateCacheName ();

  csMemFile m;
  csRef<iFile> mf = SCF_QUERY_INTERFACE ((&m), iFile);

  char header[5];
  strcpy (header, CDLODMAGIC);
  mf->Write ((char const*) header, 4);

  uint32 cd_res = (uint32)cd_resolution;
  cd_res = csConvertEndian (cd_res);
  mf->Write ((char const*) &cd_res, 4);

  uint32 tri_count = (uint32)polymesh_tri_count;
  tri_count = csConvertEndian (tri_count);
  mf->Write ((char const*) &tri_count, 4);

  int i;
  for (i = 0 ; i < polymesh_tri_count ; i++)
  {
    uint32 a, b, c;
    a = (uint32)polymesh_triangles[i].a; a = csConvertEndian (a);
    b = (uint32)polymesh_triangles[i].b; b = csConvertEndian (b);
    c = (uint32)polymesh_triangles[i].c; c = csConvertEndian (c);
    mf->Write ((char const*) &a, 4);
    mf->Write ((char const*) &b, 4);
    mf->Write ((char const*) &c, 4);
  }

  cache_mgr->CacheData ((void*)(m.GetData ()), m.GetSize (),
  	  "bruteblock_lod", cachename, 0);
  delete[] cachename;
  cache_mgr->Flush ();
}

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

  // We get the vertices and normals from the sampler. We will
  // use the normals for Level of Detail reduction on the collision
  // detection mesh.
  polymesh_vertices = new csVector3 [res * res];
  polymesh_vertex_count = res * res;
  memcpy (polymesh_vertices, terrasampler->SampleVector3 (vertices_name),
    res * res * sizeof (csVector3));

  if (cd_lod_cost > 0.00001)
  {
    // We use LOD. First see if we can get it from the cache.
    if (ReadCDLODFromCache ())
      return;
  }

  // First we make the base triangle mesh with highest detail.
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

  if (cd_lod_cost <= 0.00001)
  {
    // We do no lod on the CD mesh.
    return;
  }

  csVector3* normals = new csVector3[res * res];
  memcpy (normals, terrasampler->SampleVector3 (normals_name),
    res * res * sizeof (csVector3));

  csRef<iCommandLineParser> cmdline = CS_QUERY_REGISTRY (pFactory->object_reg,
  	iCommandLineParser);
  bool verbose = cmdline->GetOption ("verbose") != 0;
  if (verbose)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
	  "crystalspace.mesh.bruteblock",
    	  "Optimizing CD Mesh for Terrain: tris %d ...",
    	  polymesh_tri_count);
  }

  // Set up the base mesh which will be used in the LOD
  // reduction algorithm. After setting up this mesh we
  // can discard our triangle array since a copy is made.
  csTriangleMesh mesh;
  mesh.SetTriangles (polymesh_triangles, polymesh_tri_count);
  delete[] polymesh_triangles;	// SetTriangles() makes a copy.

  // Set up the LOD algorithm that we will use.
  // This is basically the cost calculation function.
  csTriangleLODAlgoHM lodalgo;
  lodalgo.normals = normals;
  lodalgo.edgedata = new int[res*res];	//@@@ Delete in csTriangleLODAlgoHM?
  lodalgo.min_max_cost = 1.0 - cd_lod_cost;
  lodalgo.mesh = &mesh;

  // Set up edge data. We have to be careful when collapsing
  // vertices that are on a border. For example, a vertex on a left
  // border can only be collapsed to another vertex on the left
  // border. To detect that quickly in our lod algorithm we fill
  // the edgedata table with the relevant information here.
  int i = 0;
  for (y = 0 ; y < res ; y++)
  {
    bool u = y == 0;
    bool d = y == res-1;
    for (x = 0 ; x < res ; x++)
    {
      bool l = x == 0;
      bool r = x == res-1;
      lodalgo.edgedata[i] =
        ((l && u) || (l && d) || (r && u) || (r && d)) ? -1 :
      	l ? 1 : u ? 2 : r ? 3 : d ? 4 : 0;
      i++;
    }
  }

  // This class will maintain the cost of all vertices.
  // It is used by CalculateLODFast() below.
  csTriangleVerticesCost mesh_verts (&mesh, polymesh_vertices,
      polymesh_vertex_count);

  // Do the triangle reduction.
  polymesh_tri_count = 0;
  polymesh_triangles = csTriangleMeshLOD::CalculateLODFast (&mesh,
  	&mesh_verts, cd_lod_cost, polymesh_tri_count,
	&lodalgo);

  if (verbose)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
	  "crystalspace.mesh.bruteblock",
    	  "Optimizing done: result %d", polymesh_tri_count);
  }

  WriteCDLODToCache ();

  delete[] lodalgo.edgedata;
  delete[] normals;
  terrasampler->Cleanup ();
}

void csTerrainObject::CleanPolyMeshData ()
{
  delete[] polymesh_vertices;
  polymesh_vertices = 0;
  delete[] polymesh_triangles;
  polymesh_triangles = 0;
  delete[] polymesh_polygons;
  polymesh_polygons = 0;
}

void csTerrainObject::PolyMesh::Cleanup ()
{
  terrain->CleanPolyMeshData ();
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
                                    csTerrainFactory *pFactory)
                                    : returnMeshesHolder (false)
{
  SCF_CONSTRUCT_IBASE (0);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiTerrainObjectState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiShadowReceiver);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiLightingInfo);
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
  cd_lod_cost = -1; //0.02;

  logparent = 0;
  initialized = false;

  region = ((csTerrainFactory*)pFactory)->samplerRegion;
  rootblock = 0;
  vis_cb = 0;
    
  block_res = 32;

  lod_distance = 200;

  lod_lcoeff = 16;
  lod_qcoeff = 0;
  block_maxsize = region.MaxX () - region.MinX ();
  block_minsize = block_maxsize;

  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.shared.stringset", iStringSet);
  
  vertices_name = strings->Request ("vertices");
  normals_name = strings->Request ("normals");
  texcoords_name = strings->Request ("texture coordinates");
  colors_name = strings->Request ("colors");

  //terr_func = &((csTerrainFactory*)pFactory)->terr_func;
  terraformer = ((csTerrainFactory*)pFactory)->terraformer;

  /*builder = new csBlockBuilder (this);
  buildthread = csThread::Create (builder);
  buildthread->Start ();*/

  staticlighting = false;
  castshadows = false;
  lmres = 257;

  colorVersion = 0;
  last_colorVersion = ~0;
  dynamic_ambient.Set (0.0f, 0.0f, 0.0f);

  baseContext = new csShaderVariableContext();
}

csTerrainObject::~csTerrainObject ()
{
  //builder->Stop ();
  if (vis_cb) vis_cb->DecRef ();
  delete[] polymesh_vertices;
  delete[] polymesh_triangles;
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiTerrainObjectState);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiShadowReceiver);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiLightingInfo);
  SCF_DESTRUCT_IBASE ();
}

void csTerrainObject::SetStaticLighting (bool enable)
{
  staticlighting = enable;
  if (staticlighting)
  {
    staticLights.SetLength (lmres * lmres);
  }
  else
  {
    staticLights.DeleteAll ();
  }
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
	      csRenderBuffer::CreateIndexRenderBuffer (
              block_res*block_res*2*3, 
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

iMeshObjectFactory* csTerrainObject::GetFactory () const
{
  return pFactory;
}

void csTerrainObject::InitializeDefault (bool clear)
{
  if (!staticlighting) return;

  if (clear)
  {
    csColor amb;
    float lightScale = CS_NORMAL_LIGHT_LEVEL / 256.0f;
    pFactory->engine->GetAmbientLight (amb);
    amb *= lightScale;
    for (size_t i = 0 ; i < staticLights.Length(); i++)
    {
      staticLights[i] = amb;
    }
  }
  colorVersion++;
}

char* csTerrainObject::GenerateCacheName ()
{
  csBox3 b;
  GetObjectBoundingBox (b);

  csMemFile mf;
  mf.Write ("bruteblock", 8);
  uint32 l;
  l = csConvertEndian ((uint32)pFactory->hm_x);
  mf.Write ((char*)&l, 4);
  l = csConvertEndian ((uint32)pFactory->hm_y);
  mf.Write ((char*)&l, 4);

  if (logparent)
  {
    csRef<iMeshWrapper> mw (SCF_QUERY_INTERFACE (logparent, iMeshWrapper));
    if (mw)
    {
      if (mw->QueryObject ()->GetName ())
        mf.Write (mw->QueryObject ()->GetName (),
        strlen (mw->QueryObject ()->GetName ()));
      iMovable* movable = mw->GetMovable ();
      iSector* sect = movable->GetSectors ()->Get (0);
      if (sect && sect->QueryObject ()->GetName ())
        mf.Write (sect->QueryObject ()->GetName (),
        strlen (sect->QueryObject ()->GetName ()));
      csVector3 pos = movable->GetFullPosition ();
      l = csConvertEndian ((int32)csQint ((pos.x * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = csConvertEndian ((int32)csQint ((pos.y * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = csConvertEndian ((int32)csQint ((pos.z * 1000)+.5));
      mf.Write ((char*)&l, 4);
      csReversibleTransform tr = movable->GetFullTransform ();
      const csMatrix3& o2t = tr.GetO2T ();
      l = csConvertEndian ((int32)csQint ((o2t.m11 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = csConvertEndian ((int32)csQint ((o2t.m12 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = csConvertEndian ((int32)csQint ((o2t.m13 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = csConvertEndian ((int32)csQint ((o2t.m21 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = csConvertEndian ((int32)csQint ((o2t.m22 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = csConvertEndian ((int32)csQint ((o2t.m23 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = csConvertEndian ((int32)csQint ((o2t.m31 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = csConvertEndian ((int32)csQint ((o2t.m32 * 1000)+.5));
      mf.Write ((char*)&l, 4);
      l = csConvertEndian ((int32)csQint ((o2t.m33 * 1000)+.5));
      mf.Write ((char*)&l, 4);
    }
  }

  l = csConvertEndian ((int32)csQint ((b.MinX () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = csConvertEndian ((int32)csQint ((b.MinY () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = csConvertEndian ((int32)csQint ((b.MinZ () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = csConvertEndian ((int32)csQint ((b.MaxX () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = csConvertEndian ((int32)csQint ((b.MaxY () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = csConvertEndian ((int32)csQint ((b.MaxZ () * 1000)+.5));
  mf.Write ((char*)&l, 4);

  csMD5::Digest digest = csMD5::Encode (mf.GetData (), mf.GetSize ());
  csString hex(digest.HexString());
  return hex.Detach();
}

static const char CachedLightingMagic[] = "brute";
static const size_t CachedLightingMagicSize = sizeof (CachedLightingMagic) - 1;

#define STATIC_LIGHT_SCALE	255.0f

bool csTerrainObject::ReadFromCache (iCacheManager* cache_mgr)
{
  if (!staticlighting) return true;

  colorVersion++;
  char* cachename = GenerateCacheName ();
  cache_mgr->SetCurrentScope (cachename);
  delete[] cachename;

  bool rc = false;
  csRef<iDataBuffer> db = cache_mgr->ReadCache ("bruteblock_lm", 0, ~0);
  if (db)
  {
    csMemFile mf ((const char*)(db->GetData ()), db->GetSize ());
    char magic[CachedLightingMagicSize + 1];
    if (mf.Read (magic, CachedLightingMagicSize) != CachedLightingMagicSize) 
      goto stop;
    magic[CachedLightingMagicSize] = 0;
    if (strcmp (magic, CachedLightingMagic) == 0)
    {
      size_t v;
      for (v = 0; v < staticLights.Length(); v++)
      {
	csColor& c = staticLights[v];
	uint8 b;
	if (mf.Read ((char*)&b, sizeof (b)) != sizeof (b)) goto stop;
	c.red = (float)b / STATIC_LIGHT_SCALE;
	if (mf.Read ((char*)&b, sizeof (b)) != sizeof (b)) goto stop;
	c.green = (float)b / STATIC_LIGHT_SCALE;
	if (mf.Read ((char*)&b, sizeof (b)) != sizeof (b)) goto stop;
	c.blue = (float)b / STATIC_LIGHT_SCALE;
      }

      uint8 c;
      if (mf.Read ((char*)&c, sizeof (c)) != sizeof (c)) goto stop;
      while (c != 0)
      {
	char lid[16];
	if (mf.Read (lid, 16) != 16) goto stop;
	iLight *l = pFactory->engine->FindLightID (lid);
	if (!l) goto stop;
	l->AddAffectedLightingInfo (&scfiLightingInfo);

	csShadowArray* shadowArr = new csShadowArray();
	float* intensities = new float[staticLights.Length()];
	shadowArr->shadowmap = intensities;
	for (size_t n = 0; n < staticLights.Length(); n++)
	{
          uint8 b;
          if (mf.Read ((char*)&b, sizeof (b)) != sizeof (b))
          {
            delete shadowArr;
            goto stop;
          }
          intensities[n] = (float)b / STATIC_LIGHT_SCALE;
	}
	pseudoDynInfo.Put (l, shadowArr);

        if (mf.Read ((char*)&c, sizeof (c)) != sizeof (c)) goto stop;
      }
      rc = true;
    }
  }

stop:
  cache_mgr->SetCurrentScope (0);
  return rc;
}

bool csTerrainObject::WriteToCache (iCacheManager* cache_mgr)
{
  if (!staticlighting) return true;
  char* cachename = GenerateCacheName ();
  cache_mgr->SetCurrentScope (cachename);
  delete[] cachename;

  bool rc = false;
  csMemFile mf;
  mf.Write (CachedLightingMagic, CachedLightingMagicSize);
  for (size_t v = 0; v < staticLights.Length(); v++)
  {
    const csColor& c = staticLights[v];
    int i; uint8 b;

    i = csQint (c.red * STATIC_LIGHT_SCALE);
    if (i < 0) i = 0; if (i > 255) i = 255; b = i;
    mf.Write ((char*)&b, sizeof (b));

    i = csQint (c.green * STATIC_LIGHT_SCALE);
    if (i < 0) i = 0; if (i > 255) i = 255; b = i;
    mf.Write ((char*)&b, sizeof (b));

    i = csQint (c.blue * STATIC_LIGHT_SCALE);
    if (i < 0) i = 0; if (i > 255) i = 255; b = i;
    mf.Write ((char*)&b, sizeof (b));
  }
  uint8 c = 1;

  csHash<csShadowArray*, iLight*>::GlobalIterator pdlIt (
    pseudoDynInfo.GetIterator ());
  while (pdlIt.HasNext ())
  {
    mf.Write ((char*)&c, sizeof (c));

    iLight* l;
    csShadowArray* shadowArr = pdlIt.Next (l);
    const char* lid = l->GetLightID ();
    mf.Write ((char*)lid, 16);

    float* intensities = shadowArr->shadowmap;
    for (size_t n = 0; n < staticLights.Length(); n++)
    {
      int i; uint8 b;
      i = csQint (intensities[n] * STATIC_LIGHT_SCALE);
      if (i < 0) i = 0; if (i > 255) i = 255; b = i;
      mf.Write ((char*)&b, sizeof (b));
    }
  }
  c = 0;
  mf.Write ((char*)&c, sizeof (c));


  rc = cache_mgr->CacheData ((void*)(mf.GetData ()), mf.GetSize (),
    "bruteblock_lm", 0, ~0);
  cache_mgr->SetCurrentScope (0);
  return rc;
}

void csTerrainObject::PrepareLighting ()
{
  if (!staticlighting && pFactory->light_mgr)
  {
    const csArray<iLight*>& relevant_lights = pFactory->light_mgr
      ->GetRelevantLights (logparent, -1, false);
    for (size_t i = 0; i < relevant_lights.Length(); i++)
      affecting_lights.Add (relevant_lights[i]);
  }
}

void csTerrainObject::SetDynamicAmbientLight (const csColor& color)
{
  dynamic_ambient = color;
  colorVersion++;
}

const csColor& csTerrainObject::GetDynamicAmbientLight ()
{
  static csColor col;
  return col;
}

void csTerrainObject::LightChanged (iLight* light)
{
  colorVersion++;
}

void csTerrainObject::LightDisconnect (iLight* light)
{
  affecting_lights.Delete (light);
  colorVersion++;
}

void csTerrainObject::UpdateColors ()
{
  if (!staticlighting) return;
  if (colorVersion == last_colorVersion) return;
  last_colorVersion = colorVersion;

  csColor baseColor (dynamic_ambient);
      
  staticColors.SetLength (staticLights.Length ());
  size_t i;
  for (i = 0; i < staticLights.Length(); i++)
  {
    staticColors[i] = staticLights[i] + baseColor;
  }

  csHash<csShadowArray*, iLight*>::GlobalIterator pdlIt =
	pseudoDynInfo.GetIterator ();
  while (pdlIt.HasNext ())
  {
    iLight* light;
    csShadowArray* shadowArr = pdlIt.Next (light);
    float* intensities = shadowArr->shadowmap;
    const csColor& lightcol = light->GetColor ();

    if (lightcol.red > EPSILON || lightcol.green > EPSILON
        || lightcol.blue > EPSILON)
    {
      for (i = 0; i < staticLights.Length(); i++)
      {
        staticColors[i] += lightcol * intensities[i];
      }
    }
  }
}

#define VERTEX_OFFSET       (10.0f * SMALL_EPSILON)

/*
  Lighting w/o local shadows:
  - Contribution from all affecting lights is calculated and summed up
    at runtime.
  Lighting with local shadows:
  - Contribution from static lights is calculated, summed and stored.
  - For every static pseudo-dynamic lights, the intensity of contribution
    is stored.
  - At runtime, the static lighting colors are copied to the actual used
    colors, the intensities of the pseudo-dynamic lights are multiplied
    with the actual colors of that lights and added as well, and finally,
    dynamic lighst are calculated.
 */
void csTerrainObject::CastShadows (iMovable* movable, iFrustumView* fview)
{
  SetupObject ();
  iBase* b = (iBase *)fview->GetUserdata ();
  iLightingProcessInfo* lpi = (iLightingProcessInfo*)b;
  CS_ASSERT (lpi != 0);

  iLight* li = lpi->GetLight ();
  bool dyn = lpi->IsDynamic ();

  if (!dyn)
  {
    if (!staticlighting || 
      li->GetDynamicType () == CS_LIGHT_DYNAMICTYPE_PSEUDO)
    {
      li->AddAffectedLightingInfo (&scfiLightingInfo);
      if (li->GetDynamicType () != CS_LIGHT_DYNAMICTYPE_PSEUDO)
        affecting_lights.Add (li);
    }
  }
  else
  {
    if (!affecting_lights.In (li))
    {
      li->AddAffectedLightingInfo (&scfiLightingInfo);
      affecting_lights.Add (li);
    }
    if (staticlighting) return;
  }

  if (!staticlighting) return;

  csReversibleTransform o2w (movable->GetFullTransform ());

  csFrustum *light_frustum = fview->GetFrustumContext ()->GetLightFrustum ();
  iShadowBlockList* shadows = fview->GetFrustumContext ()->GetShadows ();
  iShadowIterator* shadowIt = shadows->GetShadowIterator ();

  // Compute light position in object coordinates
  csVector3 wor_light_pos = li->GetCenter ();
  csVector3 obj_light_pos = o2w.Other2This (wor_light_pos);

  bool pseudoDyn = li->GetDynamicType () == CS_LIGHT_DYNAMICTYPE_PSEUDO;
  csShadowArray* shadowArr;
  if (pseudoDyn)
  {
    shadowArr = new csShadowArray ();
    pseudoDynInfo.Put (li, shadowArr);
    shadowArr->shadowmap = new float[staticLights.Length()];
    memset(shadowArr->shadowmap, 0, staticLights.Length() * sizeof(float));
  }

  float lightScale = CS_NORMAL_LIGHT_LEVEL / 256.0f;
  csColor light_color =
    li->GetColor () * lightScale /* * (256. / CS_NORMAL_LIGHT_LEVEL)*/;

  csRef<iTerraSampler> terrasampler = terraformer->GetSampler (
      csBox2 (rootblock->center.x - rootblock->size / 2.0,
      	      rootblock->center.z - rootblock->size / 2.0, 
	      rootblock->center.x + rootblock->size / 2.0,
	      rootblock->center.z + rootblock->size / 2.0), lmres);
  const csVector3* lm_vertices = terrasampler->SampleVector3 (vertices_name);
  const csVector3* lm_normals = terrasampler->SampleVector3 (normals_name);

  csColor col;
  csRef<iCommandLineParser> cmdline = CS_QUERY_REGISTRY (pFactory->object_reg,
  	iCommandLineParser);
  bool verbose = cmdline->GetOption ("verbose") != 0;
  size_t i;
  float light_radiussq = csSquare (li->GetCutoffDistance ());
  for (i = 0 ; i < staticLights.Length() ; i++)
  {
    if (verbose)
    {
      if (i % 10000 == 0)
      {
	printf ("%lu out of %lu\n", (unsigned long)i,
	  (unsigned long)staticLights.Length ());
	fflush (stdout);
      }
    }
    /*
      A small fraction of the normal is added to prevent unwanted
      self-shadowing (due small inaccuracies, the tri(s) this vertex
      lies on may shadow it.)
     */
    csVector3 v = o2w.This2Other (lm_vertices[i] +
    	(lm_normals[i] * VERTEX_OFFSET)) - wor_light_pos;

    if (!light_frustum->Contains (v))
    {
      continue;
    }

    float vrt_sq_dist = csSquaredDist::PointPoint (obj_light_pos,
      lm_vertices[i]);
    if (vrt_sq_dist >= light_radiussq) continue;

    bool inShadow = false;
    shadowIt->Reset ();
    while (shadowIt->HasNext ())
    {
      csFrustum* shadowFrust = shadowIt->Next ();
      if (shadowFrust->Contains (v))
      {
	inShadow = true;
	break;
      }
    }
    if (inShadow) continue;

    float cosinus;
    if (vrt_sq_dist < SMALL_EPSILON) cosinus = 1;
    else cosinus = (obj_light_pos - lm_vertices[i]) * lm_normals[i];
    // because the vector from the object center to the light center
    // in object space is equal to the position of the light

    if (cosinus > 0)
    {
      float vrt_dist = csQsqrt (vrt_sq_dist);
      if (vrt_sq_dist >= SMALL_EPSILON) cosinus /= vrt_dist;
      float bright = li->GetBrightnessAtDistance (vrt_dist);
      if (cosinus < 1) bright *= cosinus;
      if (pseudoDyn)
      {
	// Pseudo-dynamic
	bright *= lightScale;
	if (bright > 1.0f) bright = 1.0f; // @@@ clamp here?
	shadowArr->shadowmap[i] = bright;
      }
      else
      {
	col = light_color * bright;
	staticLights[i] += col;
      }
    }
  }
  terrasampler->Cleanup ();
}

bool csTerrainObject::SetMaterialPalette (
  const csArray<iMaterialWrapper*>& pal)
{
  palette.SetLength (pal.Length());
  paletteContexts.SetLength (pal.Length());
  for (size_t i = 0; i < pal.Length(); i++)
  {
    palette[i] = pal[i];
    paletteContexts[i] = new csShaderVariableContext();
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
  baseContext->AddVariable (lod_var);

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
    paletteContexts[i]->AddVariable (var);

    csRef<csShaderVariable> lod_var = 
      new csShaderVariable (strings->Request ("texture lod distance"));
    lod_var->SetType (csShaderVariable::VECTOR3);
    lod_var->SetValue (csVector3 (lod_distance, lod_distance, lod_distance));
    paletteContexts[i]->AddVariable (lod_var);
  }
  return true;
}

bool csTerrainObject::SetMaterialMap (iImage* map)
{
  const size_t mapSize = map->GetWidth() * map->GetHeight();
  csArray<char> image_data;
  image_data.SetLength (mapSize);
  if (map->GetFormat () & CS_IMGFMT_PALETTED8)
  {
    uint8 *data = (uint8 *)map->GetImageData ();
    for (size_t i = 0; i < mapSize; i ++)
    {
      image_data[i] = data[i];
    }
  }
  else
  {
    csRGBpixel *data = (csRGBpixel *)map->GetImageData ();
    for (size_t i = 0; i < mapSize; i ++)
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
    block_res = (int) ((float)pow (2.0f, block_res));
    return true;
  }
  else if (strcmp (parameter, "cd resolution") == 0)
  {
    cd_resolution = int (value);
    return true;
  }
  else if (strcmp (parameter, "cd lod cost") == 0)
  {
    cd_lod_cost = value;
    return true;
  }
  else if (strcmp (parameter, "lightmap resolution") == 0)
  {
    lmres = int (value);
    if (staticlighting)
      staticLights.SetLength (lmres * lmres);
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
  else if (strcmp (parameter, "cd lod cost") == 0)
  {
    return cd_lod_cost;
  }
  else if (strcmp (parameter, "lightmap resolution") == 0)
  {
    return float (lmres);
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

  UpdateColors ();

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
  if (staticlighting)
    rootblock->UpdateStaticLighting ();

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
  SetupObject();
  DrawTest (rview, movable, frustum_mask);
  n = returnMeshes->Length ();
  return returnMeshes->GetArray ();
}

void csTerrainObject::GetObjectBoundingBox (csBox3& bbox)
{
  SetupObject ();
  bbox = global_bbox;
}

void csTerrainObject::SetObjectBoundingBox (const csBox3& bbox)
{
  global_bbox = bbox;
  scfiObjectModel.ShapeChanged ();
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


csTerrainFactory::csTerrainFactory (iObjectRegistry* object_reg,
	iMeshObjectType* parent)
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
  light_mgr = CS_QUERY_REGISTRY (object_reg, iLightManager);
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
}

csTerrainFactory::~csTerrainFactory ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiTerrainFactoryState);
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiObjectModel);
  SCF_DESTRUCT_IBASE ();
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
  // @@@ Add better control over resolution?
  int resolution = (int)(region.MaxX () - region.MinX ());
  // Make the resolution conform to n^2+1
  resolution = csLog2 (resolution);
  resolution = (int)(pow(2.0f, resolution)) + 1;

  hm_x = hm_y = resolution;
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
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

csPtr<iMeshObjectFactory> csTerrainObjectType::NewFactory()
{
  csTerrainFactory *pFactory = new csTerrainFactory (object_reg, this);
  return csPtr<iMeshObjectFactory> (pFactory);
}

