/*
    Copyright (C) 2008 by Pavel Krajcevski

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

#include "oceancell.h"

using namespace CS::Plugins::WaterMesh;

csOceanCell::csOceanCell(float len, float wid, OceanLOD level)
{
  this->len = len;
  this->wid = wid;
  
  type = level;
  
  oHeight = 0;
  
  gc = csVector2(0, 0);
  
  isSetup = false;
  
  bufferHoldersNeedSetup = true;
  buffersNeedSetup = false;
}

csOceanCell::~csOceanCell()
{
  
}

void csOceanCell::SetupVertices()
{
  if(!isSetup)
  {
    float gran;
    switch(type)
    {
      default:
      case LOD_LEVEL_1:
        gran = 0.1f;
        break;
      case LOD_LEVEL_2:
        gran = 0.2f;
        break;
      case LOD_LEVEL_3:
        gran = 0.5f;
        break;
      case LOD_LEVEL_4:
        gran = 1.0f;
        break;
      case LOD_LEVEL_5:
        gran = 2.0f;
        break;
    }

    uint maxj = (uint) (len * gran);
    uint maxjd = maxj - 1;
    uint maxi = (uint) (wid * gran);
    uint maxid = maxi - 1;

    for(uint j = 0; j < maxj; ++j)
    {
      for(uint i = 0; i < maxi; ++i)
      {
        verts.Push(csVector3 ((i * wid / maxid), oHeight, (j * len / maxjd)));
        norms.Push(csVector3 (0, 1, 0));
        cols.Push(csColor (0.17f,0.27f,0.26f));
        texs.Push(csVector2((i * wid / maxid) / 1.5, (j * len / maxjd) / 1.5));
      }
    }

    for(uint j = 0; j < maxjd; ++j)
    {
      for(uint i = 0; i < maxid; ++i)
      {
        tris.Push(csTriangle ((int)(j * maxi + i), 
                    (int)((j + 1) * maxi + i), 
                    (int)(j * maxi + i + 1)));
        tris.Push(csTriangle ((int)(j * maxi + i + 1),
                    (int)((j + 1) * maxi + i),
                    (int)((j + 1) * maxi + i + 1)));
      }
    }
  
    buffersNeedSetup = true;
    isSetup = true;
  }
}

void csOceanCell::SetupBuffers()
{
  if(!buffersNeedSetup)
    return;
  
  if (!vertex_buffer)
  {
    // Create a buffer that doesn't copy the data.
    vertex_buffer = csRenderBuffer::CreateRenderBuffer (
      verts.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
      3);
  }
  vertex_buffer->CopyInto (verts.GetArray(), verts.GetSize());

  if (!texel_buffer)
  {
    // Create a buffer that doesn't copy the data.
    texel_buffer = csRenderBuffer::CreateRenderBuffer (
      verts.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
      2);
  }
  texel_buffer->CopyInto (texs.GetArray(), verts.GetSize());

  if (!index_buffer)
  {
      index_buffer = csRenderBuffer::CreateIndexRenderBuffer (
        tris.GetSize()*3,
        CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_INT,
        0, verts.GetSize()-1);
  }
  index_buffer->CopyInto (tris.GetArray(), tris.GetSize()*3);

  if (!normal_buffer)
  {            
    // Create a buffer that doesn't copy the data.
      normal_buffer = csRenderBuffer::CreateRenderBuffer (
        norms.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
        3);
  }
  normal_buffer->CopyInto (norms.GetArray(), norms.GetSize());

  if (!color_buffer)
  {            
    // Create a buffer that doesn't copy the data.
      color_buffer = csRenderBuffer::CreateRenderBuffer (
        cols.GetSize(), CS_BUF_STATIC, CS_BUFCOMP_FLOAT,
        3);
  }
  color_buffer->CopyInto (cols.GetArray(), cols.GetSize());

  buffersNeedSetup = false;
}

void csOceanCell::SetupBufferHolder()
{
  if(!bufferHoldersNeedSetup)
    return;
  
  if(bufferHolder == 0)
    bufferHolder.AttachNew(new csRenderBufferHolder);

  bufferHolder->SetRenderBuffer(CS_BUFFER_INDEX, index_buffer);
  bufferHolder->SetRenderBuffer(CS_BUFFER_POSITION, vertex_buffer);
  bufferHolder->SetRenderBuffer(CS_BUFFER_TEXCOORD0, texel_buffer);
  
  //Ocean color and normals shouldn't change..
  bufferHolder->SetRenderBuffer(CS_BUFFER_NORMAL, normal_buffer);
  bufferHolder->SetRenderBuffer(CS_BUFFER_COLOR, color_buffer);
  
  bufferHoldersNeedSetup = false;
}

////////////// csOceanNode //////////////////


csOceanNode::csOceanNode(csVector2 pos, float len, float wid)
{
  gc = pos;
  this->len = len;
  this->wid = wid;
  oHeight = 0;
  
  bbox = csBox3(gc.x, oHeight - 1.0, gc.y, gc.x + len, oHeight + 1.0, gc.y + wid);
}

csOceanNode::~csOceanNode()
{
  
}

csOceanNode csOceanNode::GetLeft() const
{
  return csOceanNode(csVector2(gc.x + len, gc.y), len, wid);
}

csOceanNode csOceanNode::GetRight() const
{
  return csOceanNode(csVector2(gc.x - len, gc.y), len, wid);
}

csOceanNode csOceanNode::GetUp() const
{
  return csOceanNode(csVector2(gc.x, gc.y + wid), len, wid);
}

csOceanNode csOceanNode::GetDown() const
{
  return csOceanNode(csVector2(gc.x, gc.y - wid), len, wid);
}

csVector3 csOceanNode::GetCenter() const
{
  return csVector3(gc.x + (len / 2), oHeight, gc.y + (wid / 2));
}
