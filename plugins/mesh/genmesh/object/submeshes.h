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
#include "igeom/trimesh.h"
#include "imesh/genmesh.h"
#include "ivideo/rendermesh.h"
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

    // Override mixmode from parent.
    uint MixMode;
    csZBufMode zmode;
    CS::Graphics::RenderPriority renderPrio;

    SubMesh () : scfImplementationType (this), name (0)
    { }
    SubMesh (const SubMesh& other) : scfImplementationType (this), 
      name (other.name), index_buffer (other.index_buffer), 
      material (other.material), MixMode (other.MixMode), zmode (other.zmode),
      renderPrio (other.renderPrio)
    { }
    const char* GetName() const { return name; }

    iRenderBuffer* GetIndices () const
    { return index_buffer; }
    iMaterialWrapper* GetMaterial () const
    { return material; }
    virtual uint GetMixmode () const
    { return MixMode; }
    void SetMaterial (iMaterialWrapper* material)
    { this->material = material; }
    csZBufMode GetZMode () const
    { return zmode; }
    void SetZMode (csZBufMode mode) { zmode = mode; }
    CS::Graphics::RenderPriority GetRenderPriority () const
    { return renderPrio; }
    void SetRenderPriority (CS::Graphics::RenderPriority prio) { renderPrio = prio; }
    void SetMixmode (uint mode) { MixMode = mode; }
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
      bitMixMode,
      bitZMode,
      bitRenderPrio
    };
    csRef<iMaterialWrapper> material;
    // Override mixmode from parent.
    uint MixMode;
    csZBufMode zmode;
    CS::Graphics::RenderPriority renderPrio;
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
      if (parentSubMesh) return parentSubMesh->SubMesh::GetName();
      return 0;
    }
    iRenderBuffer* GetIndices () const
    { 
      if (parentSubMesh) return parentSubMesh->SubMesh::GetIndices();
      return 0;
    }
    iMaterialWrapper* GetMaterial () const
    { 
      if (GetOverrideFlag (bitMaterial)) return material; 
      return parentSubMesh->SubMesh::GetMaterial ();
    }
    virtual uint GetMixmode () const
    { 
      if (GetOverrideFlag (bitMixMode)) return MixMode; 
      return parentSubMesh->SubMesh::GetMixmode ();
    }
    void SetMaterial (iMaterialWrapper* material)
    { 
      SetOverrideFlag (bitMaterial, material != 0);
      this->material = material; 
    }
    csZBufMode GetZMode () const
    { 
      if (GetOverrideFlag (bitZMode)) return zmode; 
      return parentSubMesh->SubMesh::GetZMode ();
    }
    void SetZMode (csZBufMode mode)
    { 
      SetOverrideFlag (bitZMode, mode != (csZBufMode)~0);
      zmode = mode;
    }
    CS::Graphics::RenderPriority GetRenderPriority () const
    { 
      if (GetOverrideFlag (bitRenderPrio)) return renderPrio;
      return parentSubMesh->SubMesh::GetRenderPriority (); 
    }
    void SetRenderPriority (CS::Graphics::RenderPriority prio)
    {
      SetOverrideFlag (bitRenderPrio, prio >= 0);
      renderPrio = prio; 
    }
    void SetMixmode (uint mode)
    {
      SetOverrideFlag (bitMixMode, mode != (uint)~0);
      MixMode = mode;
    }

    virtual csShaderVariable* GetVariable (csStringID name) const
    {
      csShaderVariable* var = 
        CS::ShaderVariableContextImpl::GetVariable (name);
      if (var == 0) var = parentSubMesh->SubMesh::GetVariable (name);
      return var;
    }
    virtual void PushVariables (iShaderVarStack* stacks) const
    {
      parentSubMesh->SubMesh::PushVariables (stacks);
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
    void Push (SubMeshProxy* subMesh)
    { subMeshes.Push (subMesh); }

    size_t GetSize() const
    { return subMeshes.GetSize(); }
    SubMeshProxy* operator[](size_t index)
    { return subMeshes[index]; }
  };

  class csGenmeshMeshObjectFactory;

  struct SubMeshesTriMesh : 
    public scfImplementation1<SubMeshesTriMesh, iTriangleMesh>
  {
  private:
    csWeakRef<csGenmeshMeshObjectFactory> factory;
    csFlags flags;
    const SubMeshesContainer& subMeshes;
    csDirtyAccessArray<csTriangle> triangleCache;
    uint triChangeNum;

    void CacheTriangles ();
  public:
    SubMeshesTriMesh (csGenmeshMeshObjectFactory* Factory,
      const SubMeshesContainer& subMeshes) : 
      scfImplementationType (this), factory (Factory), subMeshes (subMeshes),
      triChangeNum (~0)
    {
    }

    virtual size_t GetVertexCount ();
    virtual csVector3* GetVertices ();
    virtual size_t GetTriangleCount ();
    virtual csTriangle* GetTriangles ();
    virtual void Lock () { }
    virtual void Unlock () { }
    
    virtual csFlags& GetFlags () { return flags;  }
    virtual uint32 GetChangeNumber() const { return subMeshes.GetChangeNum(); }
  };
}
CS_PLUGIN_NAMESPACE_END(Genmesh)

#endif // __CS_SUBMESHES_H__
