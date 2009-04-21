/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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

#include "csgfx/renderbuffer.h"
#include "csgfx/vertexlistwalker.h"
#include "cstool/rbuflock.h"

#include "genmesh.h"

CS_PLUGIN_NAMESPACE_BEGIN(Genmesh)
{
  csGenmeshMeshObjectFactory::LegacyBuffers::LegacyBuffers()
   : buffersSetup (0), mesh_vertices_dirty_flag (false),
     mesh_texels_dirty_flag (false),
     mesh_normals_dirty_flag (false),
     mesh_colors_dirty_flag (false)/*,
     mesh_triangle_dirty_flag (false)*/
  {
  }

  void csGenmeshMeshObjectFactory::CreateLegacyBuffers()
  {
    if (!(legacyBuffers.buffersSetup & CS_BUFFER_POSITION_MASK))
    {
      if (knownBuffers.position.IsValid())
      {
	csVertexListWalker<float, csVector3> vertices (knownBuffers.position);
	legacyBuffers.mesh_vertices.SetCapacity (vertices.GetSize());
	
	for (size_t v = 0; v < vertices.GetSize(); v++)
	{
	  legacyBuffers.mesh_vertices.Push (*vertices);
	  ++vertices;
	}
      }
      
      legacyBuffers.mesh_vertices_dirty_flag = true;
      legacyBuffers.buffersSetup |= CS_BUFFER_POSITION_MASK;
    }
    
    if (!(legacyBuffers.buffersSetup & CS_BUFFER_TEXCOORD0_MASK))
    {
      if (knownBuffers.texcoord.IsValid())
      {
	csVertexListWalker<float, csVector2> texcoords (knownBuffers.texcoord);
	legacyBuffers.mesh_texels.SetCapacity (texcoords.GetSize());
	
	for (size_t v = 0; v < texcoords.GetSize(); v++)
	{
	  legacyBuffers.mesh_texels.Push (*texcoords);
	  ++texcoords;
	}
      }
      
      legacyBuffers.mesh_texels_dirty_flag = true;
      legacyBuffers.buffersSetup |= CS_BUFFER_TEXCOORD0_MASK;
    }
    
    if (!(legacyBuffers.buffersSetup & CS_BUFFER_NORMAL_MASK))
    {
      if (knownBuffers.normal.IsValid())
      {
	csVertexListWalker<float, csVector3> normals (knownBuffers.normal);
	legacyBuffers.mesh_normals.SetCapacity (normals.GetSize());
	
	for (size_t v = 0; v < normals.GetSize(); v++)
	{
	  legacyBuffers.mesh_normals.Push (*normals);
	  ++normals;
	}
      }
      
      legacyBuffers.mesh_normals_dirty_flag = true;
      legacyBuffers.buffersSetup |= CS_BUFFER_NORMAL_MASK;
    }
    
    if (!(legacyBuffers.buffersSetup & CS_BUFFER_COLOR_MASK))
    {
      if (knownBuffers.color.IsValid())
      {
	csVertexListWalker<float, csColor4> colors (knownBuffers.color);
	legacyBuffers.mesh_colors.SetCapacity (colors.GetSize());
	
	for (size_t v = 0; v < colors.GetSize(); v++)
	{
	  legacyBuffers.mesh_colors.Push (*colors);
	  ++colors;
	}
      }
      
      legacyBuffers.mesh_colors_dirty_flag = true;
      legacyBuffers.buffersSetup |= CS_BUFFER_COLOR_MASK;
    }
    
    /*if (!(legacyBuffers.buffersSetup & CS_BUFFER_INDEX_MASK))
    {
      if (knownBuffers.index.IsValid())
      {
	if ((knownBuffers.index->GetComponentType() == CS_BUFCOMP_INT)
	    || (knownBuffers.index->GetComponentType() == CS_BUFCOMP_UNSIGNED_INT))
	{
	  size_t triNum = knownBuffers.index->GetElementCount() / 3;
	  legacyBuffers.mesh_triangles.SetSize (triNum);
	  csRenderBufferLock<uint8> indexLock (knownBuffers.index, CS_BUF_LOCK_READ);
	  memcpy (legacyBuffers.mesh_triangles.GetArray(),
	    indexLock.Lock(), triNum * sizeof (csTriangle));
	}
	else
	{
	  size_t indexTris = knownBuffers.index->GetElementCount() / 3;
	  legacyBuffers.mesh_triangles.Empty();
	  legacyBuffers.mesh_triangles.SetCapacity (indexTris);
	  CS::TriangleIndicesStream<int> triangles (knownBuffers.index,
	    CS_MESHTYPE_TRIANGLES);
	  while (triangles.HasNext())
	    legacyBuffers.mesh_triangles.Push (triangles.Next());
	}
      }
	
      legacyBuffers.mesh_triangle_dirty_flag = true;
      legacyBuffers.buffersSetup |= CS_BUFFER_INDEX_MASK;
    }*/
  }
  
  void csGenmeshMeshObjectFactory::ClearLegacyBuffers (uint mask)
  {
    mask &= legacyBuffers.buffersSetup;
    if (mask & CS_BUFFER_POSITION_MASK)
      legacyBuffers.mesh_vertices.Empty();
    if (mask & CS_BUFFER_TEXCOORD0_MASK)
      legacyBuffers.mesh_texels.Empty();
    if (mask & CS_BUFFER_NORMAL_MASK)
      legacyBuffers.mesh_normals.Empty();
    if (mask & CS_BUFFER_COLOR_MASK)
      legacyBuffers.mesh_colors.Empty();
    //if (mask & CS_BUFFER_INDEX_MASK)
      //legacyBuffers.mesh_triangles.Empty();
    legacyBuffers.buffersSetup &= ~mask;
  }
  
  void csGenmeshMeshObjectFactory::UpdateFromLegacyBuffers()
  {
    if (legacyBuffers.buffersSetup & CS_BUFFER_POSITION_MASK)
    {
      if (legacyBuffers.mesh_vertices_dirty_flag)
      {
	csRef<iRenderBuffer> newBuffer =
	  csRenderBuffer::CreateRenderBuffer (
	    legacyBuffers.mesh_vertices.GetSize(), CS_BUF_STATIC,
	    CS_BUFCOMP_FLOAT, 3);
	newBuffer->SetData (legacyBuffers.mesh_vertices.GetArray());
	InternalSetBuffer (CS_BUFFER_POSITION, newBuffer);
	legacyBuffers.mesh_vertices_dirty_flag = false;
      }
    }
    if (legacyBuffers.buffersSetup & CS_BUFFER_TEXCOORD0_MASK)
    {
      if (legacyBuffers.mesh_texels_dirty_flag)
      {
	csRef<iRenderBuffer> newBuffer =
	  csRenderBuffer::CreateRenderBuffer (
	    legacyBuffers.mesh_texels.GetSize(), CS_BUF_STATIC,
	    CS_BUFCOMP_FLOAT, 2);
	newBuffer->SetData (legacyBuffers.mesh_texels.GetArray());
	InternalSetBuffer (CS_BUFFER_TEXCOORD0, newBuffer);
	legacyBuffers.mesh_texels_dirty_flag = false;
      }
    }
    if (legacyBuffers.buffersSetup & CS_BUFFER_NORMAL_MASK)
    {
      if (legacyBuffers.mesh_normals_dirty_flag)
      {
	csRef<iRenderBuffer> newBuffer =
	  csRenderBuffer::CreateRenderBuffer (
	    legacyBuffers.mesh_normals.GetSize(), CS_BUF_STATIC,
	    CS_BUFCOMP_FLOAT, 3);
	newBuffer->SetData (legacyBuffers.mesh_normals.GetArray());
	InternalSetBuffer (CS_BUFFER_NORMAL, newBuffer);
	legacyBuffers.mesh_normals_dirty_flag = false;
      }
    }
    if (legacyBuffers.buffersSetup & CS_BUFFER_COLOR_MASK)
    {
      if (legacyBuffers.mesh_colors_dirty_flag)
      {
        if(legacyBuffers.mesh_colors.GetSize() > 0)
        {
          csRef<iRenderBuffer> newBuffer =
            csRenderBuffer::CreateRenderBuffer (
            legacyBuffers.mesh_colors.GetSize(), CS_BUF_STATIC,
            CS_BUFCOMP_FLOAT, 4);
          newBuffer->SetData (legacyBuffers.mesh_colors.GetArray());
          InternalSetBuffer (CS_BUFFER_COLOR, newBuffer);
          legacyBuffers.mesh_colors_dirty_flag = false;
        }
        else
        {
          RemoveRenderBuffer(CS_BUFFER_COLOR);
        }
      }
    }
    /*if (legacyBuffers.buffersSetup & CS_BUFFER_INDEX_MASK)
    {
      if (legacyBuffers.mesh_triangle_dirty_flag)
      {
	csRef<iRenderBuffer> newBuffer =
	  csRenderBuffer::CreateIndexRenderBuffer (
	    legacyBuffers.mesh_triangles.GetSize() * 3, CS_BUF_STATIC,
	    CS_BUFCOMP_UNSIGNED_INT, 0,
	    legacyBuffers.mesh_vertices.GetSize());
	newBuffer->SetData (legacyBuffers.mesh_triangles.GetArray());
	InternalSetBuffer (CS_BUFFER_INDEX, newBuffer);
	legacyBuffers.mesh_triangle_dirty_flag = false;
      }
    }*/
  }

  //-------------------------------------------------------------------------

  csColor4* csGenmeshMeshObjectFactory::GetColors (bool ensureValid)
  {
    if (ensureValid && (legacyBuffers.mesh_colors.GetSize() == 0))
    {
      legacyBuffers.mesh_colors.SetCapacity (legacyBuffers.mesh_vertices.Capacity());
      legacyBuffers.mesh_colors.SetSize (legacyBuffers.mesh_vertices.GetSize(),
        csColor4 (0, 0, 0, 1));
    }
    return legacyBuffers.mesh_colors.GetArray();
  }
  
  csColor4* csGenmeshMeshObjectFactory::GetColors ()
  {
    SetupFactory ();
    CreateLegacyBuffers();
    return GetColors (true);
  }

  csVector3* csGenmeshMeshObjectFactory::GetVertices ()
  {
    SetupFactory ();
    CreateLegacyBuffers();
    return legacyBuffers.mesh_vertices.GetArray ();
  }
  
  csVector2* csGenmeshMeshObjectFactory::GetTexels ()
  {
    SetupFactory ();
    CreateLegacyBuffers();
    return legacyBuffers.mesh_texels.GetArray ();
  }
  
  csVector3* csGenmeshMeshObjectFactory::GetNormals ()
  {
    SetupFactory ();
    CreateLegacyBuffers();
    return legacyBuffers.mesh_normals.GetArray ();
  }

  int csGenmeshMeshObjectFactory::GetTriangleCount () const
  {
    if (subMeshes.GetDefaultSubmesh()->legacyTris.triangles_setup)
      return (int)subMeshes.GetDefaultSubmesh()->legacyTris.mesh_triangles.GetSize(); 
    iRenderBuffer* indices = subMeshes.GetDefaultSubmesh()->GetIndices();
    if (indices != 0)
      return (int)(indices->GetElementCount() / 3);
    else
      return 0;
  }
  
  csTriangle* csGenmeshMeshObjectFactory::GetTriangles ()
  {
    SetupFactory ();
    subMeshes.GetDefaultSubmesh()->CreateLegacyBuffer();
    return subMeshes.GetDefaultSubmesh()->legacyTris.mesh_triangles.GetArray ();
  }

  int csGenmeshMeshObjectFactory::GetVertexCount () const
  { 
    if (legacyBuffers.buffersSetup & CS_BUFFER_POSITION_MASK)
      return (int)legacyBuffers.mesh_vertices.GetSize(); 
    else if (knownBuffers.position.IsValid())
      return (int)knownBuffers.position->GetElementCount();
    else
      return 0;
  }
}
CS_PLUGIN_NAMESPACE_END(Genmesh)
