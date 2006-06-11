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

#include "simplerenderer.h"

#include "ivideo/rendermesh.h"
#include "csgfx/renderbuffer.h"

#include "iengine.h"

#include "iterrain/terraincell.h"

#include "csutil/objreg.h"

CS_PLUGIN_NAMESPACE_BEGIN(ImprovedTerrain)
{

SCF_IMPLEMENT_FACTORY (csTerrainSimpleRenderer)

struct csSimpleTerrainRenderData: public csRefCount
{
  csRef<csRenderBuffer> vb_pos;
  csRef<csRenderBuffer> vb_texcoord;
  csRef<csRenderBuffer> ib;

  csRef<iMaterialWrapper> material;

  unsigned int primitive_count;
};

csTerrainSimpleCellRenderProperties::csTerrainSimpleCellRenderProperties (iBase* parent)
  : scfImplementationType (this, parent)
{
  visible = true;
}

csTerrainSimpleCellRenderProperties::~csTerrainSimpleCellRenderProperties ()
{
}

bool csTerrainSimpleCellRenderProperties::GetVisible() const
{
  return visible;
}

void csTerrainSimpleCellRenderProperties::SetVisible(bool value)
{
  visible = value;
}

csTerrainSimpleRenderer::csTerrainSimpleRenderer (iBase* parent)
  : scfImplementationType (this, parent)
{
}


csTerrainSimpleRenderer::~csTerrainSimpleRenderer ()
{
}

csPtr<iTerrainCellRenderProperties> csTerrainSimpleRenderer::CreateProperties()
{
  return new csTerrainSimpleCellRenderProperties(0);
}

csRenderMesh** csTerrainSimpleRenderer::GetRenderMeshes(int& n, iRenderView* rview, iMovable* movable, uint32 frustum_mask, iTerrainCell** cells, int cell_count)
{
  meshes.Empty();
  
  for (int i = 0; i < cell_count; ++i)
  {
    csSimpleTerrainRenderData* rdata = (csSimpleTerrainRenderData*)cells[i]->GetRenderData();

    if (!rdata) continue;
    
    if (!rdata->material)
    {
      csRef<iEngine> engine = rview->GetEngine();

      rdata->material = engine->GetMaterialList ()->FindByName ("Stone");
    }

    bool created;
    
    csRenderMesh*& mesh = rm_holder.GetUnusedMesh(created, rview->GetCurrentFrameNumber());
    
    mesh->meshtype = CS_MESHTYPE_TRIANGLES;
    
    mesh->material = rdata->material;
    
    mesh->indexstart = 0;
    mesh->indexend = rdata->primitive_count * 3;
    
    mesh->buffers.AttachNew(new csRenderBufferHolder);
    
    mesh->buffers->SetRenderBuffer(CS_BUFFER_POSITION, rdata->vb_pos);
    mesh->buffers->SetRenderBuffer(CS_BUFFER_TEXCOORD0, rdata->vb_texcoord);
    mesh->buffers->SetRenderBuffer(CS_BUFFER_INDEX, rdata->ib);

    meshes.Push(mesh);
  }
  
  n = (int)meshes.GetSize();
  
  return meshes.GetArray();
}

void csTerrainSimpleRenderer::OnHeightUpdate(iTerrainCell* cell, const csRect& rectangle, float* data, unsigned int pitch)
{
  csRef<csSimpleTerrainRenderData> rdata = (csSimpleTerrainRenderData*)cell->GetRenderData();

  int grid_width = cell->GetGridWidth();
  int grid_height = cell->GetGridHeight();

  if (!rdata)
  {
    rdata.AttachNew(new csSimpleTerrainRenderData());

    cell->SetRenderData(rdata);

    rdata->primitive_count = (grid_width-1)*(grid_height-1)*2;

    rdata->vb_pos = csRenderBuffer::CreateRenderBuffer(grid_width * grid_height, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
    rdata->vb_texcoord = csRenderBuffer::CreateRenderBuffer(grid_width * grid_height, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 2);
    rdata->ib = csRenderBuffer::CreateIndexRenderBuffer(rdata->primitive_count*3, CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_SHORT, 0, grid_width * grid_height);

    // fill ib
    unsigned short* iptr = (unsigned short*)rdata->ib->Lock(CS_BUF_LOCK_NORMAL);

    for (int y = 0; y < grid_height - 1; ++y)
      for (int x = 0; x < grid_width - 1; ++x)
      {
        // tl - tr
        //  | / |
        // bl - br

        int tl = y * grid_width + x;
        int tr = y * grid_width + x + 1;
        int bl = (y + 1) * grid_width + x;
        int br = (y + 1) * grid_width + x + 1;

        *iptr++ = tl;
        *iptr++ = bl;
        *iptr++ = tr;

        *iptr++ = tr;
        *iptr++ = bl;
        *iptr++ = br;
      }

    rdata->ib->Release();

    // fill tex coords
    float* vptr = (float*)rdata->vb_texcoord->Lock(CS_BUF_LOCK_NORMAL);

    float u_offset = 1.0f / (grid_width - 1);
    float v_offset = 1.0f / (grid_height - 1);
    
    for (int y = 0; y < grid_height; ++y)
      for (int x = 0; x < grid_width; ++x)
      {
        *vptr++ = x * u_offset;
        *vptr++ = y * v_offset;
      }

    rdata->vb_texcoord->Release();
  }

  float* vptr = (float*)rdata->vb_pos->Lock(CS_BUF_LOCK_NORMAL);

  float offset_x = cell->GetPosition().x;
  float offset_y = cell->GetPosition().y;

  float scale_x = cell->GetSize().x / (grid_width - 1);
  float scale_y = cell->GetSize().y / (grid_height - 1);

   for (int y = rectangle.ymin; y < rectangle.ymax; ++y)
     for (int x = rectangle.xmin; x < rectangle.xmax; ++x)
     {
       *vptr++ = x * scale_x + offset_x;
       *vptr++ = data[y * pitch + x];
       *vptr++ = y * scale_y + offset_y;
     }

  rdata->vb_pos->Release();
}

}
CS_PLUGIN_NAMESPACE_END(ImprovedTerrain)
