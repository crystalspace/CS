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

#include "csgeom/bsptree.h"
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
    csRef<iRenderBuffer> index_buffer;
    csBox3 bbox;
    bool bbox_valid;
    
    /// Iterate over all vertices used in this submesh
    template<typename T>
    void IterateAllVertices (iRenderBuffer* positions, T& functor);
  public:
    const char* name;
    csRef<iMaterialWrapper> material;
    struct LegacyTriangles
    {
      bool triangles_setup, mesh_triangle_dirty_flag;
      csDirtyAccessArray<csTriangle> mesh_triangles;

      LegacyTriangles();
    };
    LegacyTriangles legacyTris;
    void CreateLegacyBuffer();
    void ClearLegacyBuffer();
    void UpdateFromLegacyBuffer();
  
    // Override mixmode from parent.
    uint MixMode;
    csZBufMode zmode;
    CS::Graphics::RenderPriority renderPrio;
    bool back2front;
    size_t indexStart, indexEnd;
    // Cache b2f tree for rendering
    csBSPTree* b2fTree;
    csFrameDataHolder<csRef<iRenderBuffer> > b2fIndices;

    SubMesh () : scfImplementationType (this), bbox_valid (false), name (0), 
      MixMode ((uint)~0), zmode ((csZBufMode)~0), renderPrio (-1),
      back2front (false), indexStart (0), indexEnd ((size_t)~0), b2fTree (0)
    { }
    SubMesh (const SubMesh& other) : scfImplementationType (this), 
      index_buffer (other.index_buffer), bbox (other.bbox),
      bbox_valid (other.bbox_valid), name (other.name), 
      material (other.material), MixMode (other.MixMode), zmode (other.zmode),
      renderPrio (other.renderPrio), back2front (other.back2front),
      indexStart (other.indexStart), indexEnd (other.indexEnd), b2fTree (0)
    { }
    ~SubMesh()
    {
      delete b2fTree;
    }
    const char* GetName() const { return name; }
    
    void ClearB2F()
    {
      delete b2fTree; b2fTree = 0;
      b2fIndices.Clear ();
    }    
    iRenderBuffer* GetIndicesB2F (const csVector3& pos, uint frameNum,
      const csVector3* vertices, size_t vertNum);

    //iRenderBuffer* GetIndices () const
    //{ return index_buffer; }
    iRenderBuffer* GetIndices ();
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
    void SetBack2Front (bool enable)
    { 
      if (back2front && !enable) ClearB2F();
      back2front = enable;
    }
    bool GetBack2Front () const { return back2front; }
    void SetIndexRange (size_t start, size_t end)
    { indexStart = start; indexEnd = end; }
    void GetIndexRange (size_t& start, size_t& end) const
    { start = indexStart; end = indexEnd; }

    void SetIndices (iRenderBuffer* newIndices);
    
    const csBox3& GetObjectBoundingBox (iRenderBuffer* positions);
    float ComputeMaxSqRadius (iRenderBuffer* positions,
      const csVector3& center);
    void InvalidateBoundingBox ();
  };

  class SubMeshesContainer
  {
    csRef<SubMesh> defaultSubmesh;
    csRefArray<SubMesh> subMeshes;
    uint changeNum;
  public:
    SubMeshesContainer();

    void ClearSubMeshes ()
    {
      subMeshes.DeleteAll();
      changeNum++;
      subMeshes.Push (defaultSubmesh);
    }
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

    void SetMaterialWrapper(iMaterialWrapper* mat);
    iMaterialWrapper* GetMaterialWrapper() const;
    
    SubMesh* GetDefaultSubmesh() const { return defaultSubmesh; }
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
      bitRenderPrio,
      bitBack2Front,
      bitIndexRange
    };
    csRef<iMaterialWrapper> material;
    // Override mixmode from parent.
    uint MixMode;
    csZBufMode zmode;
    CS::Graphics::RenderPriority renderPrio;
    csFlags overrideFlags;
    csRef<csRenderBufferHolder> bufferHolder;
    bool back2front;
    size_t indexStart, indexEnd;

    class RenderBufferAccessor : 
      public scfImplementation1<RenderBufferAccessor, iRenderBufferAccessor>
    {
    public:
      csWeakRef<SubMeshProxy> parent;
      csRef<iRenderBufferAccessor> wrappedHolder;

      RenderBufferAccessor (SubMeshProxy* parent) :
	scfImplementationType (this), parent (parent) { }
      void PreGetBuffer (csRenderBufferHolder* holder,
    	csRenderBufferName buffer);
    };
    csRef<RenderBufferAccessor> renderBufferAccessor;

    void SetOverrideFlag (uint bit, bool flag) 
    { overrideFlags.SetBool (1 << bit, flag); }
    bool GetOverrideFlag (uint bit) const 
    { return overrideFlags.Check (1 << bit); }
  public:
    csWeakRef<SubMesh> parentSubMesh;
    // Stuff below is used by GM for rendering
    csRenderMeshHolder rmHolder;

    SubMeshProxy () : scfImplementationType (this), overrideFlags (0)
    { }
    ~SubMeshProxy ()
    { }

    csRenderBufferHolder* GetBufferHolder (csRenderBufferHolder* copyFrom);

    const char* GetName() const 
    { 
      if (parentSubMesh) return parentSubMesh->SubMesh::GetName();
      return 0;
    }
    iRenderBuffer* GetIndices ()
    { 
      if (parentSubMesh) return parentSubMesh->SubMesh::GetIndices();
      return 0;
    }
    iRenderBuffer* GetIndicesB2F (const csVector3& pos, uint frameNum,
      const csVector3* vertices, size_t vertNum)
    {
      if (parentSubMesh) 
        return parentSubMesh->SubMesh::GetIndicesB2F (pos, frameNum, vertices,
          vertNum);
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
    void SetBack2Front (bool enable)
    {
      SetOverrideFlag (bitBack2Front, enable);
      back2front = enable; 
    }
    bool GetBack2Front () const
    {
      if (GetOverrideFlag (bitBack2Front)) return back2front;
      return parentSubMesh->SubMesh::GetBack2Front ();
    }
    void SetIndexRange (size_t start, size_t end)
    {
      SetOverrideFlag (bitIndexRange, start != 0 && end != (size_t) ~0);
      indexStart = start; indexEnd = end;
    }
    void GetIndexRange (size_t& start, size_t& end) const
    {
      if (GetOverrideFlag (bitIndexRange))
      {
	start = indexStart;
	end = indexEnd;
      }
      else parentSubMesh->SubMesh::GetIndexRange (start, end);
    }

    virtual csShaderVariable* GetVariable (CS::ShaderVarStringID name) const
    {
      csShaderVariable* var = 
        CS::ShaderVariableContextImpl::GetVariable (name);
      if (var == 0) var = parentSubMesh->SubMesh::GetVariable (name);
      return var;
    }
    virtual void PushVariables (csShaderVariableStack& stack) const
    {
      parentSubMesh->SubMesh::PushVariables (stack);
      CS::ShaderVariableContextImpl::PushVariables (stack);
    }
    virtual bool IsEmpty() const 
    { 
      return parentSubMesh->IsEmpty() 
        && CS::ShaderVariableContextImpl::IsEmpty ();
    }  
  };

  class SubMeshProxiesContainer
  {
    csRef<SubMeshProxy> defaultSubmesh;
    csRefArray<SubMeshProxy> subMeshes;
  public:
    SubMeshProxiesContainer ();
    SubMeshProxiesContainer (SubMeshProxy* deflt);
    
    void AddSubMesh (SubMeshProxy* subMesh);
    SubMeshProxy* FindSubMesh (const char* name) const;
    void Empty()
    {
      subMeshes.Empty();
    }
    void Push (SubMeshProxy* subMesh)
    {
      subMeshes.Push (subMesh);
    }

    size_t GetSize() const
    { return subMeshes.GetSize(); }
    SubMeshProxy* operator[](size_t index)
    { return subMeshes[index]; }

    void SetMaterialWrapper(iMaterialWrapper* mat);
    iMaterialWrapper* GetMaterialWrapper() const;
    
    SubMeshProxy* GetDefaultSubmesh() const { return defaultSubmesh; }
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
