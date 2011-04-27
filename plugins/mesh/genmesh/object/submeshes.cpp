/*
  Copyright (C) 2006 by Jorrit Tyberghein
            (C) 2006 by Frank Richter

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

#include "csgeom/math3d.h"
#include "csgfx/renderbuffer.h"
#include "csgfx/vertexlistwalker.h"
#include "cstool/rbuflock.h"

#include "submeshes.h"
#include "genmesh.h"

CS_PLUGIN_NAMESPACE_BEGIN(Genmesh)
{
  SubMesh::LegacyTriangles::LegacyTriangles() : triangles_setup (false), 
    mesh_triangle_dirty_flag (false)
  {
  }

  void SubMesh::CreateLegacyBuffer()
  {
    if (!legacyTris.triangles_setup)
    {
      if (index_buffer.IsValid())
      {
	if ((index_buffer->GetComponentType() == CS_BUFCOMP_INT)
	    || (index_buffer->GetComponentType() == CS_BUFCOMP_UNSIGNED_INT))
	{
	  size_t triNum = index_buffer->GetElementCount() / 3;
	  legacyTris.mesh_triangles.SetSize (triNum);
	  csRenderBufferLock<uint8> indexLock (index_buffer, CS_BUF_LOCK_READ);
	  memcpy (legacyTris.mesh_triangles.GetArray(),
	    indexLock.Lock(), triNum * sizeof (csTriangle));
	}
	else
	{
	  size_t indexTris = index_buffer->GetElementCount() / 3;
	  legacyTris.mesh_triangles.Empty();
	  legacyTris.mesh_triangles.SetCapacity (indexTris);
	  CS::TriangleIndicesStream<int> triangles (index_buffer,
	    CS_MESHTYPE_TRIANGLES);
	  while (triangles.HasNext())
	    legacyTris.mesh_triangles.Push (triangles.Next());
	}
      }
	
      legacyTris.mesh_triangle_dirty_flag = true;
      legacyTris.triangles_setup = true;
    }
  }

  void SubMesh::ClearLegacyBuffer()
  {
    if (!legacyTris.triangles_setup) return;
    legacyTris.triangles_setup = false;
    legacyTris.mesh_triangle_dirty_flag = false;
    legacyTris.mesh_triangles.DeleteAll ();
  }

  void SubMesh::UpdateFromLegacyBuffer ()
  {
    if (legacyTris.triangles_setup)
    {
      if (legacyTris.mesh_triangle_dirty_flag)
      {
	size_t rangeMin = (size_t)~0, rangeMax = 0;
	for (size_t n = 0; n < legacyTris.mesh_triangles.GetSize(); n++)
	{
	  for (int e = 0; e < 3; e++)
	  {
	    size_t index = size_t (legacyTris.mesh_triangles[n][e]);
	    if (index < rangeMin)
	      rangeMin = index;
	    else if (index > rangeMax)
	      rangeMax = index;
	  }
	}
	csRef<iRenderBuffer> newBuffer =
	  csRenderBuffer::CreateIndexRenderBuffer (
	    legacyTris.mesh_triangles.GetSize() * 3, CS_BUF_STATIC,
	    CS_BUFCOMP_UNSIGNED_INT, rangeMin,
	    rangeMax);
	newBuffer->SetData (legacyTris.mesh_triangles.GetArray());
	index_buffer = newBuffer;
	legacyTris.mesh_triangle_dirty_flag = false;
      }
    }
  }

  iRenderBuffer* SubMesh::GetIndicesB2F (const csVector3& pos, uint frameNum,
                                         const csVector3* vertices, size_t vertNum)
  {
    UpdateFromLegacyBuffer();

    if (!b2fTree)
    {
      CS::TriangleIndicesStream<int> triangles (index_buffer,
	CS_MESHTYPE_TRIANGLES);
      b2fTree = new csBSPTree;
      b2fTree->Build (triangles, vertices);
    }
    
    const csDirtyAccessArray<int>& triidx = b2fTree->Back2Front (pos);
    
    bool bufCreated;
    csRef<iRenderBuffer>& newIndexBuffer = b2fIndices.GetUnusedData (bufCreated, frameNum);
    if (bufCreated || newIndexBuffer->GetElementCount() != triidx.GetSize()*3)
    {
      newIndexBuffer = csRenderBuffer::CreateIndexRenderBuffer (triidx.GetSize()*3,
        CS_BUF_STREAM, CS_BUFCOMP_UNSIGNED_INT, 0, vertNum-1);
    }
    csRenderBufferLock<int> indices (newIndexBuffer);
    CS::TriangleIndicesStreamRandom<int> triangles (index_buffer,
      CS_MESHTYPE_TRIANGLES);
    for (size_t t = 0; t < triidx.GetSize(); t++)
    {
      const TriangleT<int> tri (triangles[triidx[t]]);
      *(indices++) = tri.a;
      *(indices++) = tri.b;
      *(indices++) = tri.c;
    }
    return newIndexBuffer;
  }
  
  iRenderBuffer* SubMesh::GetIndices ()
  {
    UpdateFromLegacyBuffer();
    return index_buffer;
  }

  void SubMesh::SetIndices (iRenderBuffer* indices)
  {
    ClearLegacyBuffer();
    index_buffer = indices;
    if (b2fTree) 
    {
      delete b2fTree;
      b2fTree = 0;
    }
    InvalidateBoundingBox ();
  }
  
  template<typename T>
  void SubMesh::IterateAllVertices (iRenderBuffer* positions, T& functor)
  {
    UpdateFromLegacyBuffer();
    if (!index_buffer.IsValid()) return;
    
    csVertexListWalker<float, csVector3> vertices (positions);
    
    /* Set up a simple hash table to store which vertices have already
       been seen & handled, to avoid, to some degree, unnecessary
       bbox vertex adds. */
    const size_t maxVertexHashSize = 4093;
    const size_t indexOffs = index_buffer->GetRangeStart();
    const size_t vertexHashSize = csMin (maxVertexHashSize,
      index_buffer->GetRangeEnd() - indexOffs + 1);
    CS_ALLOC_STACK_ARRAY(size_t, seenVertices, vertexHashSize);
    for (size_t v = 0; v < vertexHashSize; v++) seenVertices[v] = (size_t)~0;
    
    CS::TriangleIndicesStream<size_t> triangles (index_buffer,
      CS_MESHTYPE_TRIANGLES);
    while (triangles.HasNext())
    {
      const TriangleT<size_t> tri (triangles.Next());
      for (int v = 0; v < 3; v++)
      {
        size_t vertex = tri[v];
        const size_t vertexHashIndex = (vertex-indexOffs) % vertexHashSize;
        if (seenVertices[vertexHashIndex] == vertex) continue;

        vertices.SetElement (vertex);
        const csVector3& vert = *vertices;
        functor (vert);

        seenVertices[vertexHashIndex] = vertex;
      }
    }
    
  }
  
  struct IterateBB
  {
    csBox3 bbox;
  
    void operator() (const csVector3& pos)
    { bbox.AddBoundingVertex (pos); }
  };

  const csBox3& SubMesh::GetObjectBoundingBox (iRenderBuffer* positions)
  {
    if (!bbox_valid)
    {
      IterateBB itbb;
      IterateAllVertices (positions, itbb);
      bbox = itbb.bbox;
      bbox_valid = true;
    }
    return bbox;
  }
  
  struct IterateRadius
  {
    csVector3 center;
    float sqradius;
  
    IterateRadius (const csVector3& center) : center (center), sqradius (0) {}
    void operator() (const csVector3& pos)
    { sqradius = csMax (sqradius, csSquaredDist::PointPoint (center, pos)); }
  };
  
  float SubMesh::ComputeMaxSqRadius (iRenderBuffer* positions,
                                     const csVector3& center)
  {
    IterateRadius itr (center);
    IterateAllVertices (positions, itr);
    return itr.sqradius;
  }
  
  void SubMesh::InvalidateBoundingBox ()
  {
    bbox_valid = false;
  }

  //-------------------------------------------------------------------------

  SubMeshesContainer::SubMeshesContainer() : changeNum (0)
  {
    defaultSubmesh.AttachNew (new SubMesh);
    subMeshes.Push (defaultSubmesh);
  }
  
  static int SubmeshSubmeshCompare (SubMesh* const& A, 
                                    SubMesh* const& B)
  {
    if (A < B)
    	return -1;
    if (A > B)
    	return 1;
    return 0;
  }

  iGeneralMeshSubMesh* SubMeshesContainer::AddSubMesh (
    iRenderBuffer* indices, iMaterialWrapper *material, const char* name, 
    uint mixmode)
  {
    if ((subMeshes.GetSize() == 1) && (subMeshes[0] == defaultSubmesh))
    {
      if(defaultSubmesh->GetMaterial() == 0)
        defaultSubmesh->SetMaterial(material);

      subMeshes.Empty();
    }

    csRef<SubMesh> subMesh;
    subMesh.AttachNew (new SubMesh ());
    subMesh->material = material;
    subMesh->MixMode = mixmode;
    subMesh->name = name;
    subMesh->SetIndices (indices);

    subMeshes.InsertSorted (subMesh, SubmeshSubmeshCompare);
    /* @@@ FIXME: Prolly do some error checking, like sanity of
     * indices */
    changeNum++;
    return subMesh;
  }
  
  static int SubmeshStringCompare (SubMesh* const& A, const char* const& b)
  {
    const char* a = A->GetName();
    if (a == 0) return (b == 0) ? 0 : 1;
    if (b == 0) return -1;
    return strcmp (a, b);
  }

  SubMesh* SubMeshesContainer::FindSubMesh (const char* name) const
  {
    size_t idx = subMeshes.FindKey (
      csArrayCmp<SubMesh*, const char*> (name, &SubmeshStringCompare));
    if (idx == csArrayItemNotFound) return 0;
    return subMeshes[idx];
  }

  void SubMeshesContainer::DeleteSubMesh (iGeneralMeshSubMesh* mesh)
  {
    SubMesh* subMesh = static_cast<SubMesh*> (mesh);
    size_t idx = subMeshes.FindSortedKey (
      csArrayCmp<SubMesh*, SubMesh*> (subMesh, &SubmeshSubmeshCompare));
    if (idx == csArrayItemNotFound) return;
    subMeshes.DeleteIndex (idx);
    changeNum++;

    if (subMeshes.GetSize() == 0)
      subMeshes.Push (defaultSubmesh);
  }

  void SubMeshesContainer::SetMaterialWrapper(iMaterialWrapper* mat)
  {
      defaultSubmesh->SetMaterial(mat);
      if((subMeshes.GetSize() == 1) && (subMeshes[0] != defaultSubmesh))
      {
          subMeshes[0]->SetMaterial(mat);
      }
  }

  iMaterialWrapper* SubMeshesContainer::GetMaterialWrapper() const
  {
    iMaterialWrapper* mat = nullptr;
    if((subMeshes.GetSize() == 1) && (subMeshes[0] != defaultSubmesh))
    {
        mat = subMeshes[0]->GetMaterial();
    }

    // Fallback to material of default submesh
    if (!mat) mat = defaultSubmesh->GetMaterial();
    return mat;
  }

  //-------------------------------------------------------------------------

  csRenderBufferHolder* SubMeshProxy::GetBufferHolder (
    csRenderBufferHolder* copyFrom)
  {
    if (!renderBufferAccessor.IsValid())
      renderBufferAccessor.AttachNew (new RenderBufferAccessor (this));
    if (!bufferHolder.IsValid())
      bufferHolder.AttachNew (new csRenderBufferHolder);
    *bufferHolder = *copyFrom;
    if (parentSubMesh->legacyTris.mesh_triangle_dirty_flag)
    {
      renderBufferAccessor->wrappedHolder = copyFrom->GetAccessor();
      bufferHolder->SetAccessor (renderBufferAccessor,
	copyFrom->GetAccessorMask() | CS_BUFFER_INDEX_MASK);
    }
    else
    {
      renderBufferAccessor->wrappedHolder.Invalidate();
      bufferHolder->SetAccessor (copyFrom->GetAccessor(),
	copyFrom->GetAccessorMask() & ~CS_BUFFER_INDEX_MASK);
      bufferHolder->SetRenderBuffer (CS_BUFFER_INDEX,
	parentSubMesh->GetIndices());
    }
    return bufferHolder;
  }

  //-------------------------------------------------------------------------

  void SubMeshProxy::RenderBufferAccessor::PreGetBuffer (
    csRenderBufferHolder* holder, csRenderBufferName buffer)
  {
    if ((buffer == CS_BUFFER_INDEX) && (parent.IsValid()))
      holder->SetRenderBuffer (CS_BUFFER_INDEX, parent->GetIndices());
    else
      wrappedHolder->PreGetBuffer (holder, buffer);
  }

  //-------------------------------------------------------------------------

  SubMeshProxiesContainer::SubMeshProxiesContainer ()
  {
    defaultSubmesh.AttachNew (new SubMeshProxy);
    subMeshes.Push (defaultSubmesh);
  }
    
  SubMeshProxiesContainer::SubMeshProxiesContainer (SubMeshProxy* dflt)
   : defaultSubmesh (dflt)
  {
  }
    
  static int SubmeshProxySubmeshProxyCompare (SubMeshProxy* const& A, 
                                              SubMeshProxy* const& B)
  {
    const char* a = A->GetName();
    const char* b = B->GetName();
    if (a == 0) return (b == 0) ? 0 : 1;
    if (b == 0) return -1;
    return strcmp (a, b);
  }

  void SubMeshProxiesContainer::AddSubMesh (SubMeshProxy* subMesh)
  {
    if ((subMeshes.GetSize() == 1) && (subMeshes[0] == defaultSubmesh))
    {
      if(defaultSubmesh->GetMaterial() == 0)
        defaultSubmesh->SetMaterial(subMesh->GetMaterial());

      subMeshes.Empty();
    }

    subMeshes.InsertSorted (subMesh, SubmeshProxySubmeshProxyCompare);
  }

  static int SubmeshProxyStringCompare (SubMeshProxy* const& A, const char* const& b)
  {
    const char* a = A->GetName();
    if (a == 0) return (b == 0) ? 0 : 1;
    if (b == 0) return -1;
    return strcmp (a, b);
  }

  SubMeshProxy* SubMeshProxiesContainer::FindSubMesh (const char* name) const
  {
    size_t idx = subMeshes.FindKey (
      csArrayCmp<SubMeshProxy*, const char*> (name, &SubmeshProxyStringCompare));
    if (idx == csArrayItemNotFound) return 0;
    return subMeshes[idx];
  }

  void SubMeshProxiesContainer::SetMaterialWrapper(iMaterialWrapper* mat)
  {
      defaultSubmesh->SetMaterial(mat);
      if((subMeshes.GetSize() == 1) && (subMeshes[0] != defaultSubmesh))
      {
          subMeshes[0]->SetMaterial(mat);
      }
  }

  iMaterialWrapper* SubMeshProxiesContainer::GetMaterialWrapper() const
  {
    iMaterialWrapper* mat = nullptr;
    if((subMeshes.GetSize() == 1) && (subMeshes[0] != defaultSubmesh))
    {
      mat = subMeshes[0]->GetMaterial();
    }

    // Fallback to material of default submesh
    if (!mat) mat = defaultSubmesh->GetMaterial();
    return mat;
  }

  //-------------------------------------------------------------------------

  void SubMeshesTriMesh::CacheTriangles ()
  {
    if (triChangeNum == subMeshes.GetChangeNum()) return;

    triangleCache.Empty();
    for (size_t s = 0; s < subMeshes.GetSubMeshCount(); s++)
    {
      SubMesh* subMesh = subMeshes.GetSubMesh (s);
      iRenderBuffer* buffer = subMesh->GetIndices();
      size_t offs = triangleCache.GetSize();
      size_t bufferTris = buffer->GetElementCount() / 3;
      triangleCache.SetSize (offs + bufferTris);
      void* tris = buffer->Lock (CS_BUF_LOCK_READ);
      memcpy (triangleCache.GetArray() + offs, tris, 
        bufferTris * sizeof (csTriangle));
      buffer->Release();
    }
    triangleCache.ShrinkBestFit();

    triChangeNum = subMeshes.GetChangeNum();
  }

  size_t SubMeshesTriMesh::GetVertexCount ()
  {
    if (!factory) return 0;
    return factory->GetVertexCount ();
  }
  csVector3* SubMeshesTriMesh::GetVertices ()
  {
    if (!factory) return 0;
    return factory->GetVertices ();
  }
  size_t SubMeshesTriMesh::GetTriangleCount ()
  {
    if (!factory) return 0;
    CacheTriangles ();
    return triangleCache.GetSize ();
  }
  csTriangle* SubMeshesTriMesh::GetTriangles ()
  {
    if (!factory) return 0;
    CacheTriangles ();
    return triangleCache.GetArray ();
  }

}
CS_PLUGIN_NAMESPACE_END(Genmesh)
