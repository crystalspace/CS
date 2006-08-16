/*
    Copyright (C) 2006 by Kapoulkine Arseny

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

#include "bruteblockrenderer.h"

#include "ivideo/rendermesh.h"
#include "csgfx/renderbuffer.h"

#include "csgfx/shadervarcontext.h"

#include "iengine.h"

#include "iterrain/terraincell.h"

#include "csutil/objreg.h"

#include "csgfx/shadervar.h"

#include "csgfx/rgbpixel.h"
#include "csgfx/imagememory.h"

#include "csutil/refarr.h"

#include "ivideo/txtmgr.h"

#include "cstool/rbuflock.h"

#include <windows.h>

CS_PLUGIN_NAMESPACE_BEGIN(ImprovedTerrain)
{

SCF_IMPLEMENT_FACTORY (csTerrainBruteBlockRenderer)

struct csBruteBlockTerrainRenderData;

/**
 * This is one block in the terrain.
 */
class csTerrBlock : public csRefCount
{
public:
  CS_LEAKGUARD_DECLARE (csTerrBlock);

  csRef<iRenderBuffer> mesh_vertices;
  csRef<iRenderBuffer> mesh_texcoords;
  csRef<iRenderBuffer> mesh_heights;
  csRef<csRenderBufferHolder> bufferHolder;

  csVector2 center;
  csVector2 size;
  int res;

  // for this kind of grid:
  // . _ .
  // _ _ _
  // . _ .
  // left = top = 0, right = bottom = 2,
  // step = 2
  int left, top, right, bottom;
  int step;

  bool built;

  csBruteBlockTerrainRenderData *rdata;

  //          0
  //      ---------
  //      | 0 | 1 |
  //    2 |-------| 1
  //      | 2 | 3 |
  //      ---------
  //          3

  csTerrBlock* parent;
  csRef<csTerrBlock> children[4];
  csTerrBlock* neighbours[4];
  int child;

  static int tris;

  csBox3 bbox;

public:

  csTerrBlock (csBruteBlockTerrainRenderData *data);
  ~csTerrBlock ();

  /// Load data from Former
  void LoadData ();

  /// Generate mesh
  void SetupMesh ();

  /// Detach the node from the tree
  void Detach ();

  /// Split block in 4 children
  void Split ();

  /// Merge block
  void Merge ();

  /// Checks if something needs to be merged or split
  void CalcLOD ();

  /// Returns true if this node is a leaf
  bool IsLeaf ()  const
  { return children[0] == 0; }

  void DrawTest (iGraphics3D* g3d, iRenderView *rview, uint32 frustum_mask,
                 csReversibleTransform &transform, iMovable *movable,
                 csDirtyAccessArray<csRenderMesh*>& meshes);

  bool detach;
};

static void FillEdge (bool halfres, int res, uint16* indices, int &indexcount,
               int offs, int add)
{
  if (!halfres) return;
  
  int x;
  // This triangulation scheme could probably be better.
  for (x=0; x<res; x+=2)
  {
    // 
    // a - c - e - g - i
    //   b   d   f   h
    // ok, having 2 triangles for each crease is not cool
    // though without that I doubt that it's possible to
    // make correct orientation. Is it?
    indices[indexcount++] = offs + x * add;
    indices[indexcount++] = offs + (x+1) * add;
    indices[indexcount++] = offs + (x+2) * add;
    indices[indexcount++] = offs + x * add;
    indices[indexcount++] = offs + (x+2) * add;
    indices[indexcount++] = offs + (x+1) * add;
  }
}
/*static void FillEdge (bool halfres, int res, uint16* indices, int &indexcount,
               int offs, int xadd, int zadd)
{
  int x;
  // This triangulation scheme could probably be better.
  for (x=0; x<res; x+=2)
  {
    // 
    // a   c   e   g   i
    //   b   d   f   h
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
}*/

struct csBruteBlockTerrainRenderData: public csRefCount
{
  csRef<csTerrBlock> rootblock;

  csRef<iMaterialWrapper> material;
  
  csRenderMeshHolder* rm_holder;

  csPlane3* planes;

  csRefArray<iMaterialWrapper>* material_palette;
  csArray<csRef<csShaderVariableContext> > sv_context;
  
  csArray<csRef<iTextureHandle> > alpha_map;

  unsigned int primitive_count;

  int block_res, block_minsize;
  float lod_lcoeff;

  csRef<iRenderBuffer> mesh_indices[16];
  int numindices[16];
  
  csReversibleTransform tr_o2c;

  bool initialized;

  iTerrainCell* cell;

  csBruteBlockTerrainRenderData(iTerrainCell* cell)
  {
    this->cell = cell;

    initialized = false;

//        <lodvalue name="block resolution">16</lodvalue>
    block_res = 16;

//        <lodvalue name="minimum block size">32</lodvalue>
    block_minsize = 32;
    
    lod_lcoeff = 16;

//    <variable name="LodM" value="-0.00666667" />
//    <variable name="LodA" value="1.33333" />
//        <lodvalue name="splatting distance">200</lodvalue>
//        <lodvalue name="block split distance">8</lodvalue>
//        <lodvalue name="cd resolution">256</lodvalue>
  }

  void SetupObject(iGraphics3D* g3d)
  {
    if (!initialized)
    {
      initialized = true;
  
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
                (block_res*block_res*2 + 4*block_res)*3, 
                CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_SHORT,
                0, (block_res+1) * (block_res+1) - 1);
              uint16 *indices = 
                (uint16*)mesh_indices[idx]->Lock (CS_BUF_LOCK_NORMAL);
  
              numindices[idx] = 0;
              int x, z;
/*              for (z=1; z<(block_res-1); z++)
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
              }*/
              for (z=0; z<block_res; z++)
              {
                  for (x=0; x<block_res; x++)
                  { 
                      indices[numindices[idx]++] = x+(z+0)*(block_res+1);
                      indices[numindices[idx]++] = x+(z+1)*(block_res+1);
                      indices[numindices[idx]++] = x+1+(z+0)*(block_res+1);
                      
                      indices[numindices[idx]++] = x+1+(z+0)*(block_res+1);
                      indices[numindices[idx]++] = x+(z+1)*(block_res+1);
                      indices[numindices[idx]++] = x+1+(z+1)*(block_res+1);
                  }
              }
  
              /*FillEdge (t==1,
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
                block_res*(block_res+1)+block_res, -1, -(block_res+1));*/
                
              FillEdge (t==1,
                  block_res, indices, numindices[idx], 
                  0, 1);

              FillEdge (r==1,
                  block_res, indices, numindices[idx],
                  block_res, block_res + 1);

              FillEdge (l==1,
                  block_res, indices, numindices[idx], 
                  0, block_res + 1);

              FillEdge (b==1, 
                  block_res, indices, numindices[idx],
                  block_res*(block_res+1), 1);

              mesh_indices[idx]->Release ();
            }
          }
        }
      }

      rootblock.AttachNew (new csTerrBlock (this));
      csVector2 center = cell->GetPosition () + csVector2 (cell->GetSize ().x,
        cell->GetSize ().y)/2;
      rootblock->center = center;
      rootblock->size = csVector2(cell->GetSize ().x, cell->GetSize ().y);
      
      rootblock->left = rootblock->top = 0;
      rootblock->right = cell->GetGridWidth () - 1;
      rootblock->bottom = cell->GetGridHeight () - 1;

      // so, we're going to take block_res steps of size step to reach 
      // from left to right, that is
      // left + step * block_res = right
      // beware! block_res and right should be 2^n !
      rootblock->step = rootblock->right / block_res;

      rootblock->SetupMesh ();
    }
  }
};

csTerrBlock::csTerrBlock (csBruteBlockTerrainRenderData* data)
{
  parent = 0;
  child = 0;
  children[0] = children[1] = children[2] = children[3] = 0;
  neighbours[0] =  neighbours[1] = neighbours[2] = neighbours[3] = 0;

  built = false;

  bufferHolder.AttachNew (new csRenderBufferHolder);

  detach = false;

  rdata = data;
}

csTerrBlock::~csTerrBlock ()
{
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
    children[0] = children[1] = children[2] = children[3] = 0;
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

void csTerrBlock::LoadData ()
{
  res = rdata->block_res + 1;

  mesh_vertices = 
      csRenderBuffer::CreateRenderBuffer (
      res*res, CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
      3);

  mesh_texcoords = 
      csRenderBuffer::CreateRenderBuffer (res*res,
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
      2);

  bufferHolder->SetRenderBuffer (CS_BUFFER_POSITION, mesh_vertices);
  bufferHolder->SetRenderBuffer (CS_BUFFER_TEXCOORD0, mesh_texcoords);
  
  const csVector2& pos = rdata->cell->GetPosition();
  const csVector3& cell_size = rdata->cell->GetSize();
  
  csLockedHeightData data = rdata->cell->GetHeightData ();
  
  if (!built)
    bbox.Empty();
  
  {
    csRenderBufferLock<csVector3> vertex_data(mesh_vertices);
    
    unsigned int index = 0;

    float min_x = center.x - size.x/2;
    float max_x = center.x + size.x/2;

    float min_z = center.y - size.y/2;
    float max_z = center.y + size.y/2;

    float x_offset = (max_x - min_x) / (float)(res - 1);
    float z_offset = (max_z - min_z) / (float)(res - 1);

    float c_z = min_z;
    
    float min_height = 1e+20f;
    float max_height = -1e+20f;
    
    for (int y = 0, grid_y = top; y < res; ++y, grid_y += step)
    {
      float c_x = min_x;
      float* row = data.data + data.pitch * grid_y;
      
      for (int x = 0, grid_x = left; x < res; ++x, grid_x += step)
      {
        float height = row[grid_x];
        
        vertex_data[index] = csVector3(c_x, height, c_z);
        
        if (min_height > height) min_height = height;
        if (max_height < height) max_height = height;
        
        c_x += x_offset;
        
        ++index;
      }
      
      c_z += z_offset;
    }
    
    if (!built)
    {
      bbox.Set(min_x, min_height, min_z,
               max_x, max_height, max_z);
    }
  }  
  
  {
    csRenderBufferLock<csVector2> texcoord_data(mesh_texcoords);
    
    unsigned int index = 0;
    
    float min_u = (center.x - size.x/2 - pos.x) / cell_size.x;
    float max_u = (center.x + size.x/2 - pos.x) / cell_size.x;
    
    float min_v = (center.y - size.y/2 - pos.y) / cell_size.y;
    float max_v = (center.y + size.y/2 - pos.y) / cell_size.y;
    
    float u_offset = (max_u - min_u) / (float)(res - 1);
    float v_offset = (max_v - min_v) / (float)(res - 1);
    
    float v = min_v;

    for (int y = 0; y < res; ++y)
    {
      float u = min_u;
      
      for (int x = 0; x < res; ++x)
      {
        texcoord_data[index].x = u;
        texcoord_data[index].y = v;
      
        u += u_offset;
        
        ++index;
      }
      
      v += v_offset;
    }
  }
}

void csTerrBlock::SetupMesh ()
{
  
  //@@@ have a method to set this from world
  res = rdata->block_res + 1;
  
  LoadData ();
  
  built = true;
}

void csTerrBlock::Split ()
{
  int i;
  for (i=0; i<4; i++)
  {
    if (neighbours[i] && neighbours[i]->step>step && neighbours[i]->IsLeaf ())
      neighbours[i]->Split ();
  }

  int half_right = left + (right - left) / 2;
  int half_bottom = top + (bottom - top) / 2;

  children[0].AttachNew (new csTerrBlock (rdata));
  children[0]->parent = this;
  children[0]->size = size/2.0;
  children[0]->center = center + csVector2 (-size.x/4.0, -size.y/4.0);
  children[0]->child = 0;
  children[0]->step = step / 2;
  children[0]->left = left;
  children[0]->right = half_right;
  children[0]->top = top;
  children[0]->bottom = half_bottom;

  children[1].AttachNew (new csTerrBlock (rdata));
  children[1]->parent = this;
  children[1]->size = size/2.0;
  children[1]->center = center + csVector2 ( size.x/4.0, -size.y/4.0);
  children[1]->child = 1;
  children[1]->step = step / 2;
  children[1]->left = half_right;
  children[1]->right = right;
  children[1]->top = top;
  children[1]->bottom = half_bottom;

  children[2].AttachNew (new csTerrBlock (rdata));
  children[2]->parent = this;
  children[2]->size = size/2.0;
  children[2]->center = center + csVector2 (-size.x/4.0, size.y/4.0);
  children[2]->child = 2;
  children[2]->step = step / 2;
  children[2]->left = left;
  children[2]->right = half_right;
  children[2]->top = half_bottom;
  children[2]->bottom = bottom;

  children[3].AttachNew (new csTerrBlock (rdata));
  children[3]->parent = this;
  children[3]->size = size/2.0;
  children[3]->center = center + csVector2 ( size.x/4.0, size.y/4.0);
  children[3]->child = 3;
  children[3]->step = step / 2;
  children[3]->left = half_right;
  children[3]->right = right;
  children[3]->top = half_bottom;
  children[3]->bottom = bottom;

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
  children[0] = children[1] = children[2] = children[3] = 0;
}

void csTerrBlock::CalcLOD ()
{
  int res = rdata->block_res;

  csVector3 cam = rdata->tr_o2c.GetOrigin ();
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
  
  float size = MAX(this->size.x, this->size.y);
      
  float splitdist = size*rdata->lod_lcoeff / float (res);
  
  if (GetAsyncKeyState(VK_F2) >= 0) return;
  
  if (cambox.SquaredOriginDist()<splitdist*splitdist &&
    step > 1)
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
      children[i]->CalcLOD ();
}

void csTerrBlock::DrawTest (iGraphics3D* g3d,
			    iRenderView *rview, uint32 frustum_mask,
                            csReversibleTransform &transform,
                            iMovable *movable,
        csDirtyAccessArray<csRenderMesh*>& meshes)
{
  if (!IsLeaf () && children[0]->built &&
                    children[1]->built &&
                    children[2]->built &&
                    children[3]->built)
  {
    children[0]->DrawTest (g3d, rview, frustum_mask, transform, movable,
      meshes);
    children[1]->DrawTest (g3d, rview, frustum_mask, transform, movable,
      meshes);
    children[2]->DrawTest (g3d, rview, frustum_mask, transform, movable,
      meshes);
    children[3]->DrawTest (g3d, rview, frustum_mask, transform, movable,
      meshes);
    return;
  }

  if (!built)
    return;

  int res = rdata->block_res;

  //csVector3 cam = rview->GetCamera ()->GetTransform ().GetOrigin ();
  csVector3 cam = transform.GetOrigin ();

  int clip_portal, clip_plane, clip_z_plane;
  if (!rview->ClipBBox (rdata->planes, frustum_mask,
      bbox, clip_portal, clip_plane, clip_z_plane))
    return;

  int idx = ((!neighbours[0] || neighbours[0]->step>step)?1:0)+
            (((!neighbours[1] || neighbours[1]->step>step)?1:0)<<1)+
            (((neighbours[2] && neighbours[2]->step>step)?1:0)<<2)+
            (((neighbours[3] && neighbours[3]->step>step)?1:0)<<3);
            
  bufferHolder->SetRenderBuffer (CS_BUFFER_INDEX, rdata->mesh_indices[idx]);
  
  const csReversibleTransform o2wt = movable->GetFullTransform ();
  const csVector3& wo = o2wt.GetOrigin ();
  bool isMirrored = rview->GetCamera()->IsMirrored();

  for (unsigned int j = 0; j < rdata->material_palette->GetSize (); ++j)
  {
    bool created;
    
    csRenderMesh*& mesh = rdata->rm_holder->GetUnusedMesh (created,
                                       rview->GetCurrentFrameNumber ());
    
    mesh->meshtype = CS_MESHTYPE_TRIANGLES;
    
    mesh->buffers = bufferHolder;
    mesh->clip_portal = clip_portal;
    mesh->clip_plane = clip_plane;
    mesh->clip_z_plane = clip_z_plane;
    mesh->indexstart = 0;
    mesh->indexend = rdata->numindices[idx];
    mesh->material = rdata->material_palette->Get (j);
    mesh->variablecontext = rdata->sv_context[j];

    mesh->object2world = o2wt;
    mesh->worldspace_origin = wo;
    mesh->do_mirror = isMirrored;
      
    meshes.Push (mesh);
  }
}

csTerrainBruteBlockCellRenderProperties::
 csTerrainBruteBlockCellRenderProperties(iBase* parent)
  : scfImplementationType (this, parent)
{
  visible = true;
}

csTerrainBruteBlockCellRenderProperties::
 ~csTerrainBruteBlockCellRenderProperties ()
{
}

bool csTerrainBruteBlockCellRenderProperties::GetVisible() const
{
  return visible;
}

void csTerrainBruteBlockCellRenderProperties::SetVisible(bool value)
{
  visible = value;
}

csTerrainBruteBlockRenderer::csTerrainBruteBlockRenderer (iBase* parent)
  : scfImplementationType (this, parent)
{
}


csTerrainBruteBlockRenderer::~csTerrainBruteBlockRenderer ()
{
}

csPtr<iTerrainCellRenderProperties> csTerrainBruteBlockRenderer::
 CreateProperties()
{
  return new csTerrainBruteBlockCellRenderProperties(NULL);
}

csRenderMesh** csTerrainBruteBlockRenderer::GetRenderMeshes (int& n,
      iRenderView* rview, iMovable* movable, uint32 frustum_mask,
      iTerrainCell** cells, int cell_count)
{
  meshes.Empty ();
  
  bool do_mirroring = rview->GetCamera ()->IsMirrored ();
  
  const csReversibleTransform& o2wt = movable->GetFullTransform ();
  const csVector3& wo = o2wt.GetOrigin ();
  
  iCamera* cam = rview->GetCamera ();
  csReversibleTransform tr_o2c = cam->GetTransform ();
  if (!movable->IsFullTransformIdentity ())
      tr_o2c /= movable->GetFullTransform ();

  csPlane3 planes[10];
  rview->SetupClipPlanes (tr_o2c, planes, frustum_mask);

  for (int i = 0; i < cell_count; ++i)
  {
    csBruteBlockTerrainRenderData* rdata = (csBruteBlockTerrainRenderData*)
                                           cells[i]->GetRenderData ();

    if (!rdata) continue;
    
    rdata->material_palette = &material_palette;
    rdata->rm_holder = &rm_holder;
    rdata->planes = planes;

	rdata->rootblock->CalcLOD ();
    
    rdata->tr_o2c = tr_o2c;

    rdata->rootblock->DrawTest (g3d, rview, 0, tr_o2c, movable, meshes);
  }
  
  n = (int)meshes.GetSize ();
  
  return meshes.GetArray ();
}

void csTerrainBruteBlockRenderer::OnMaterialPaletteUpdate (
const csRefArray<iMaterialWrapper>& material_palette)
{
  this->material_palette = material_palette;
}

void csTerrainBruteBlockRenderer::OnHeightUpdate (iTerrainCell* cell,
const csRect& rectangle, const float* data, unsigned int pitch)
{
  csRef<csBruteBlockTerrainRenderData> rdata = (csBruteBlockTerrainRenderData*)
  cell->GetRenderData ();

  int grid_width = cell->GetGridWidth ();
  int grid_height = cell->GetGridHeight ();
  
  int mm_width = cell->GetMaterialMapWidth();
  int mm_height = cell->GetMaterialMapHeight();

  if (!rdata)
  {
    rdata.AttachNew (new csBruteBlockTerrainRenderData(cell));

    cell->SetRenderData (rdata);

    rdata->SetupObject(g3d);
  }
}

void csTerrainBruteBlockRenderer::OnMaterialMaskUpdate (iTerrainCell* cell,
unsigned int material, const csRect& rectangle, const unsigned char* data,
unsigned int pitch)
{
  csRef<csBruteBlockTerrainRenderData> rdata = (csBruteBlockTerrainRenderData*)
  cell->GetRenderData ();

  if (!rdata)
  {
    rdata.AttachNew (new csBruteBlockTerrainRenderData(cell));

    cell->SetRenderData (rdata);

    rdata->SetupObject(g3d);
  }

  if (rdata->sv_context.GetSize () <= material)
  {
    rdata->sv_context.SetSize (material + 1);
  }
  
  if (!rdata->sv_context[material])
  {
    rdata->sv_context[material].AttachNew (new csShaderVariableContext);
   
    csRef<csShaderVariable> var;
    var.AttachNew (new csShaderVariable(strings->Request ("splat base pass")));
    var->SetType (csShaderVariable::INT);
    var->SetValue (material == 0);
    rdata->sv_context[material]->AddVariable (var);
  }
    
  if (rdata->alpha_map.GetSize () <= material)
  {
    rdata->alpha_map.SetSize (material + 1);
  }

  if (!rdata->alpha_map[material])
  {
    csRef<iImage> image;
    image.AttachNew (new csImageMemory (cell->GetMaterialMapWidth (),
    cell->GetMaterialMapHeight (), CS_IMGFMT_TRUECOLOR));

    rdata->alpha_map[material] = g3d->GetTextureManager()->RegisterTexture
    (image, CS_TEXTURE_2D | CS_TEXTURE_3D | CS_TEXTURE_CLAMP);
    
    csRef<csShaderVariable> var;
    var.AttachNew (new csShaderVariable(strings->Request ("splat alpha map")));
    var->SetType (csShaderVariable::TEXTURE);
    var->SetValue (rdata->alpha_map[material]);
    rdata->sv_context[material]->AddVariable (var);
  }
  
  csDirtyAccessArray<csRGBpixel> image_data;
  image_data.SetSize (rectangle.Width () * rectangle.Height ());
  
  for (int y = 0; y < rectangle.Width (); ++y)
    for (int x = 0; x < rectangle.Height (); ++x)
      image_data[y * rectangle.Width () + x].Set (
      data[y * pitch + x], data[y * pitch + x],
      data[y * pitch + x], data[y * pitch + x]);
      
  rdata->alpha_map[material]->Blit (rectangle.xmin, rectangle.ymin,
  rectangle.Width (), rectangle.Height (), (unsigned char*)
  image_data.GetArray (), iTextureHandle::RGBA8888);
}

bool csTerrainBruteBlockRenderer::Initialize (iObjectRegistry* object_reg)
{
  this->object_reg = object_reg;
   
  g3d = csQueryRegistry<iGraphics3D> (object_reg);
  
  strings = csQueryRegistryTagInterface<iStringSet> (object_reg,
  "crystalspace.shared.stringset");
    
  return true;
}

}
CS_PLUGIN_NAMESPACE_END(ImprovedTerrain)
