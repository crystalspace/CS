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

#ifndef __CS_SUBMESHES_H__
#define __CS_SUBMESHES_H__

#include "csgeom/tri.h"
#include "cstool/rendermeshholder.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/flags.h"
#include "csutil/ref.h"
#include "csutil/scf_implementation.h"
#include "csutil/util.h"
#include "csutil/weakref.h"

#include "iengine/material.h"
#include "igeom/polymesh.h"
#include "ivideo/rndbuf.h"

CS_PLUGIN_NAMESPACE_BEGIN(Genmesh)
{
  struct SubMesh
  {
  protected:
    const char* name;
  public:
    csRef<iRenderBuffer> index_buffer;
    csRef<iMaterialWrapper> material;
    csRenderMeshHolder rmHolder;
    csRef<csRenderBufferHolder> bufferHolder;

    // Override mixmode from parent.
    uint MixMode;

    SubMesh () : name (0)
    { }
    ~SubMesh()
    {
      delete[] name;
    }
    SubMesh (const SubMesh& other) : index_buffer (other.index_buffer),
      material (other.material), rmHolder (), 
      bufferHolder (other.bufferHolder), MixMode (other.MixMode)
    {
      name = csStrNew (other.name);
    }
    const char* GetName() const { return name; }
    void SetName (const char* newName)
    {
      delete[] name;
      name = csStrNew (newName);
    }
  };

  class SubMeshesContainer
  {
    csArray<SubMesh> subMeshes;
    uint changeNum;
  public:
    SubMeshesContainer() : changeNum (0) { }

    void ClearSubMeshes ()
    { subMeshes.DeleteAll(); changeNum++; }
    bool AddSubMesh (iRenderBuffer* indices, iMaterialWrapper *material, 
      const char* name, uint mixmode = (uint)~0);
    size_t FindSubMesh (const char* name) const;
    void DeleteSubMesh (size_t index)
    { subMeshes.DeleteIndex (index); changeNum++; }
    size_t GetSubMeshCount () const
    { return subMeshes.GetSize(); }
    iRenderBuffer* GetSubMeshIndices (size_t index) const
    { return (index < subMeshes.GetSize()) ? subMeshes[index].index_buffer : 0; }
    iMaterialWrapper* GetSubMeshMaterial (size_t index) const
    { return (index < subMeshes.GetSize()) ? subMeshes[index].material : 0; }
    const char* GetSubMeshName (size_t index) const
    { return (index < subMeshes.GetSize()) ? subMeshes[index].GetName() : 0; }
    uint GetSubMeshMixmode (size_t index) const
    { return (index < subMeshes.GetSize()) ? subMeshes[index].MixMode : (uint)~0; }

    size_t GetSize() const
    { return subMeshes.GetSize(); }
    SubMesh& operator[](size_t index)
    { return subMeshes[index]; }
    uint GetChangeNum () const
    { return changeNum; }
  };

  class csGenmeshMeshObjectFactory;

  struct SubMeshesPolyMesh : 
    public scfImplementation1<SubMeshesPolyMesh, iPolygonMesh>
  {
  private:
    csWeakRef<csGenmeshMeshObjectFactory> factory;
    csFlags flags;
    const SubMeshesContainer& subMeshes;
    csDirtyAccessArray<csTriangle> triangleCache;
    csDirtyAccessArray<csMeshedPolygon> polygonCache;
    uint triChangeNum, polyChangeNum;

    void CacheTriangles ();
    void CachePolygons ();
  public:
    SubMeshesPolyMesh (csGenmeshMeshObjectFactory* Factory,
      const SubMeshesContainer& subMeshes) : 
      scfImplementationType (this), factory (Factory), subMeshes (subMeshes),
      triChangeNum (~0), polyChangeNum (~0)
    {
      flags.Set (CS_POLYMESH_TRIANGLEMESH);
    }

    virtual int GetVertexCount ();
    virtual csVector3* GetVertices ();
    virtual int GetPolygonCount ();
    virtual csMeshedPolygon* GetPolygons ();
    virtual int GetTriangleCount ();
    virtual csTriangle* GetTriangles ();
    virtual void Lock () { }
    virtual void Unlock () { }
    
    virtual csFlags& GetFlags () { return flags;  }
    virtual uint32 GetChangeNumber() const { return subMeshes.GetChangeNum(); }
  };
}
CS_PLUGIN_NAMESPACE_END(Genmesh)

#endif // __CS_SUBMESHES_H__
