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

#include "csutil/objreg.h"

CS_PLUGIN_NAMESPACE_BEGIN(ImprovedTerrain)
{

SCF_IMPLEMENT_FACTORY (csTerrainSimpleRenderer)

csTerrainSimpleCellRenderProperties::csTerrainSimpleCellRenderProperties (iBase* parent)
  : scfImplementationType (this, parent)
{
  visible = true;
}

csTerrainSimpleCellRenderProperties::~csTerrainSimpleCellRenderProperties ()
{
}

bool csTerrainSimpleCellRenderProperties::GetVisible()
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
  if (meshes.IsEmpty())
  {
    csRenderMesh* mesh = new csRenderMesh();
    
    mesh->db_mesh_name = "terrain mesh";
    
    csRef<iEngine> engine = csQueryRegistry<iEngine> (iSCF::SCF->object_reg);

    mesh->material = engine->GetMaterialList ()->FindByName ("Stone");

    mesh->meshtype = CS_MESHTYPE_TRIANGLES;
    
    mesh->indexstart = 0;
    mesh->indexend = 3;
    
    mesh->buffers.AttachNew(new csRenderBufferHolder);
    
    csRef<csRenderBuffer> vertices = csRenderBuffer::CreateRenderBuffer(3, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
    csRef<csRenderBuffer> indices = csRenderBuffer::CreateIndexRenderBuffer(3, CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_SHORT, 0, 3);
    
    csVector3* vptr = (csVector3*)vertices->Lock(CS_BUF_LOCK_NORMAL);
    vptr[0] = csVector3(0, 0, 0);
    vptr[1] = csVector3(10000, 0, 0);
    vptr[2] = csVector3(0, 0, 10000);
    vertices->Release();
    
    unsigned short* iptr = (unsigned short*)indices->Lock(CS_BUF_LOCK_NORMAL);
    iptr[0] = 0;
    iptr[1] = 2;
    iptr[2] = 1;
    indices->Release();
    
    mesh->buffers->SetRenderBuffer(CS_BUFFER_POSITION, vertices);
    mesh->buffers->SetRenderBuffer(CS_BUFFER_INDEX, indices);
    
    meshes.Push(mesh);
  }
  
  n = (int)meshes.GetSize();
  
  return meshes.GetArray();
}

}
CS_PLUGIN_NAMESPACE_END(ImprovedTerrain)
