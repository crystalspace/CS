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
#include "csgfx/shadervarcontext.h"
#include "cstool/rendermeshholder.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/flags.h"
#include "csutil/ref.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"
#include "csutil/util.h"
#include "csutil/weakref.h"

#include "iengine/material.h"
#include "igeom/polymesh.h"
#include "imesh/genmesh.h"
#include "ivideo/rndbuf.h"

CS_PLUGIN_NAMESPACE_BEGIN(Genmesh)
{
  class csGenmeshMeshObject;

  class SubMesh : 
    public scfImplementation2<SubMesh, 
                              iGeneralMeshSubMesh, 
                              scfFakeInterface<iShaderVariableContext> >,
    public CS::ShaderVariableContextImpl
  {
  public:
    const char* name;
    csRef<iRenderBuffer> index_buffer;
    csRef<iMaterialWrapper> material;
    //csRenderMeshHolder rmHolder;
    //csRef<csRenderBufferHolder> bufferHolder;

    // Override mixmode from parent.
    uint MixMode;

    SubMesh () : scfImplementationType (this), name (0)
    { }
    SubMesh (const SubMesh& other) : scfImplementationType (this), 
      name (other.name), index_buffer (other.index_buffer), 
      material (other.material), MixMode (other.MixMode)
    { }
    const char* GetName() const { return name; }

    iRenderBuffer* GetIndices () const
    { return index_buffer; }
    iMaterialWrapper* GetMaterial () const
    { return material; }
    virtual uint GetMixmode () const
    { return MixMode; }
  };

  class SubMeshesContainer
  {
    csRefArray<SubMesh> subMeshes;
    uint changeNum;
  public:
    SubMeshesContainer() : changeNum (0) { }

    void ClearSubMeshes ()
    { subMeshes.DeleteAll(); changeNum++; }
    iGeneralMeshSubMesh* AddSubMesh (iRenderBuffer* indices, 
      iMaterialWrapper *material, const char* name, uint mixmode = (uint)~0);
    SubMesh* FindSubMesh (const char* name) const;
    void DeleteSubMesh (iGeneralMeshSubMesh* mesh);
    size_t GetSubMeshCount () const
    { return subMeshes.GetSize(); }
    SubMesh* GetSubMesh (size_t index) const
    { return subMeshes[index]; }

    size_t GetSize() const
    { return subMeshes.GetSize(); }
    SubMesh* operator[](size_t index) const
    { return subMeshes[index]; }
    uint GetChangeNum () const
    { return changeNum; }
  };

  /**
   * SubMesh proxy - can return data from either the parent (real) SubMesh or
   * local override values.
   */
  class SubMeshProxy : 
    public scfImplementation2<SubMeshProxy, 
                              iGeneralMeshSubMesh, 
                              scfFakeInterface<iShaderVariableContext> >,
    public CS::ShaderVariableContextImpl
  {
  protected:
    enum
    {
      bitMaterial = 0,
      bitMixMode
    };
    csRef<iMaterialWrapper> material;
    // Override mixmode from parent.
    uint MixMode;
    csFlags overrideFlags;
    csRef<csRenderBufferHolder> bufferHolder;

    void SetOverrideFlag (uint bit, bool flag) 
    { overrideFlags.SetBool (1 << bit, flag); }
    bool GetOverrideFlag (uint bit) const 
    { return overrideFlags.Check (1 << bit); }
  public:
    csWeakRef<SubMesh> parentSubMesh;
    csRenderMeshHolder rmHolder;

    SubMeshProxy () : scfImplementationType (this), overrideFlags (0)
    { }
    ~SubMeshProxy ()
    { }

    csRenderBufferHolder* GetBufferHolder();

    const char* GetName() const 
    { 
      if (parentSubMesh) return parentSubMesh->GetName();
      return 0;
    }
    iRenderBuffer* GetIndices () const
    { 
      if (parentSubMesh) return parentSubMesh->GetIndices();
      return 0;
    }
    iMaterialWrapper* GetMaterial () const
    { 
      if (GetOverrideFlag (bitMaterial)) return material; 
      return parentSubMesh->GetMaterial ();
    }
    virtual uint GetMixmode () const
    { 
      if (GetOverrideFlag (bitMixMode)) return MixMode; 
      return parentSubMesh->GetMixmode ();
    }

    virtual csShaderVariable* GetVariable (csStringID name) const
    {
      csShaderVariable* var = 
        CS::ShaderVariableContextImpl::GetVariable (name);
      if (var == 0) var = parentSubMesh->GetVariable (name);
      return var;
    }
    virtual void PushVariables (iShaderVarStack* stacks) const
    {
      parentSubMesh->PushVariables (stacks);
      CS::ShaderVariableContextImpl::PushVariables (stacks);
    }
    virtual bool IsEmpty() const 
    { 
      return parentSubMesh->IsEmpty() 
        && CS::ShaderVariableContextImpl::IsEmpty ();
    }  
  };

  class SubMeshProxiesContainer
  {
    csRefArray<SubMeshProxy> subMeshes;
  public:
    void AddSubMesh (SubMeshProxy* subMesh);
    SubMeshProxy* FindSubMesh (const char* name) const;
    void Empty() { subMeshes.Empty(); }

    size_t GetSize() const
    { return subMeshes.GetSize(); }
    SubMeshProxy* operator[](size_t index)
    { return subMeshes[index]; }
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
