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

#include "cstool/rbuflock.h"

#include "submeshes.h"
#include "genmesh.h"

CS_PLUGIN_NAMESPACE_BEGIN(Genmesh)
{
  static int SubmeshSubmeshCompare (const SubMesh& A, const SubMesh& B)
  {
    const char* a = A.GetName();
    const char* b = B.GetName();
    if (a == 0) return (b == 0) ? 0 : 1;
    if (b == 0) return -1;
    return strcmp (a, b);
  }

  bool SubMeshesContainer::AddSubMesh (iRenderBuffer* indices, 
    iMaterialWrapper *material, const char* name, uint mixmode)
  {
    SubMesh subMesh;
    subMesh.material = material;
    subMesh.MixMode = mixmode;
    subMesh.index_buffer = indices;
    subMesh.SetName (name);

    subMesh.bufferHolder.AttachNew (new csRenderBufferHolder);
    subMesh.bufferHolder->SetRenderBuffer(CS_BUFFER_INDEX,
      indices);

    subMeshes.InsertSorted (subMesh, SubmeshSubmeshCompare);
    /* @@@ FIXME: Prolly do some error checking, like sanity of
     * indices */
    changeNum++;
    return true;
  }

  static int SubmeshStringCompare (const SubMesh& A, const char* const& b)
  {
    const char* a = A.GetName();
    if (a == 0) return (b == 0) ? 0 : 1;
    if (b == 0) return -1;
    return strcmp (a, b);
  }

  size_t SubMeshesContainer::FindSubMesh (const char* name) const
  {
    return subMeshes.FindSortedKey (
      csArrayCmp<SubMesh, const char*> (name, &SubmeshStringCompare));
  }

  //-------------------------------------------------------------------------

  void SubMeshesPolyMesh::CacheTriangles ()
  {
    if (triChangeNum == subMeshes.GetChangeNum()) return;

    triangleCache.Empty();
    for (size_t s = 0; s < subMeshes.GetSubMeshCount(); s++)
    {
      iRenderBuffer* buffer = subMeshes.GetSubMeshIndices (s);
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
  void SubMeshesPolyMesh::CachePolygons ()
  {
    if (polyChangeNum == subMeshes.GetChangeNum()) return;

    CacheTriangles ();

    polygonCache.Empty();
    for (size_t s = 0; s < subMeshes.GetSubMeshCount(); s++)
    {
      iRenderBuffer* buffer = subMeshes.GetSubMeshIndices (s);
      size_t bufferTris = buffer->GetElementCount() / 3;
      csRenderBufferLock<uint, iRenderBuffer*> indices (buffer);
      for (size_t t = 0; t < bufferTris; t++)
      {
        csMeshedPolygon poly;
        poly.num_vertices = 3;
        poly.vertices = (int*)(indices.Get (t*3));
        polygonCache.Push (poly);
      }
    }
    polygonCache.ShrinkBestFit();

    polyChangeNum = subMeshes.GetChangeNum();
  }

  int SubMeshesPolyMesh::GetVertexCount ()
  {
    if (!factory) return 0;
    return factory->GetVertexCount ();
  }
  csVector3* SubMeshesPolyMesh::GetVertices ()
  {
    if (!factory) return 0;
    return factory->GetVertices ();
  }
  int SubMeshesPolyMesh::GetPolygonCount ()
  {
    if (!factory) return 0;
    CachePolygons ();
    return (int)polygonCache.GetSize ();
  }
  csMeshedPolygon* SubMeshesPolyMesh::GetPolygons ()
  {
    if (!factory) return 0;
    CachePolygons ();
    return polygonCache.GetArray ();
  }
  int SubMeshesPolyMesh::GetTriangleCount ()
  {
    if (!factory) return 0;
    CacheTriangles ();
    return (int)triangleCache.GetSize ();
  }
  csTriangle* SubMeshesPolyMesh::GetTriangles ()
  {
    if (!factory) return 0;
    CacheTriangles ();
    return triangleCache.GetArray ();
  }

}
CS_PLUGIN_NAMESPACE_END(Genmesh)
