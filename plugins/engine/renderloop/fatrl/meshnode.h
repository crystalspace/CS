/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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

#ifndef __CS_MESHNODE_H__
#define __CS_MESHNODE_H__

#include "csutil/dirtyaccessarray.h"
#include "csutil/redblacktree.h"
#include "csutil/refarr.h"
#include "csutil/weakref.h"

#include "csgfx/shadervar.h"
#include "csgfx/shadervarcontext.h"

#include "rendernode.h"

struct ShaderTicketKey
{
  long prio;
  iShader* shader;
  size_t realTicket, sortTicket;
};

CS_SPECIALIZE_TEMPLATE
class csComparator<ShaderTicketKey, ShaderTicketKey>
{
public:
  static int Compare(ShaderTicketKey const &r1, ShaderTicketKey const &r2)
  {
    int d = r1.prio - r2.prio;
    if (d != 0) return 0;
    iShader* sh1 = (r1.sortTicket != (size_t)~0) ? r1.shader : 0;
    iShader* sh2 = (r2.sortTicket != (size_t)~0) ? r2.shader : 0;
    d = (sh1 < sh2) ? -1 : ((sh1 > sh2) ? 1 : 0);
    if (d == 0)
    {
      d = (r1.sortTicket < r2.sortTicket) ? -1 
	: ((r1.sortTicket > r2.sortTicket) ? 1 : 0);
    }
    return d;
  }
};

class csMeshRenderNodeFactory;

class csMeshRenderNode : public csRenderNode
{
  friend class csMeshRenderNodeFactory;

  struct MeshBucket
  {
    csDirtyAccessArray<csRenderMesh*> rendermeshes;
    csDirtyAccessArray<csShaderVarStack> stacks;
  };
  typedef csRedBlackTreeMap<ShaderTicketKey, MeshBucket> SortedBuckets;
  SortedBuckets buckets;
  csShaderVariableContext& shadervars;
  bool zoffset;
  class TraverseShaderBuckets
  {
    csMeshRenderNode& node;
    iGraphics3D* g3d;
  public:
    TraverseShaderBuckets (csMeshRenderNode& node, iGraphics3D* g3d) : 
      node(node), g3d(g3d) {}
    void Process (const ShaderTicketKey& key, MeshBucket& bucket);
  };
  friend class TraverseShaderBuckets;

  csMeshRenderNodeFactory* factory;
  csStringID shaderType;
  csRef<iShader> defShader;

  inline void RenderMeshes (iGraphics3D* g3d, iShader* shader, 					
    size_t ticket, csRenderMesh** meshes, size_t num,
    const csShaderVarStack* Stacks);

  csMeshRenderNode (csMeshRenderNodeFactory* factory, csStringID shaderType,
    iShader* defShader, csShaderVariableContext& shadervars, bool zoffset);
public:
  void AddMesh (csRenderMesh* rm, iShader* shader, 
    const csShaderVarStack& stacks, long prio, bool keepOrder,
    size_t ticket);
  bool HasMeshes () const { return !buckets.IsEmpty(); }

  virtual bool Preprocess (iRenderView* rview);
  virtual void Postprocess (iRenderView* rview) {}
};

class csMeshRenderNodeFactory
{
  friend class csMeshRenderNode;
  friend class csMeshRenderNode::TraverseShaderBuckets;

  csWeakRef<iShaderManager> shaderManager;
  csWeakRef<iShader> nullShader;

  static csStringID string_object2world;
public:
  csMeshRenderNodeFactory (iObjectRegistry* object_reg);

  csMeshRenderNode* CreateMeshNode (csStringID shaderType, iShader* defShader, 
    csShaderVariableContext& shadervars, bool zoffset);
};

#endif // __CS_MESHNODE_H__
