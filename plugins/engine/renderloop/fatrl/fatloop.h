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

#ifndef __CS_FATLOOP_H__
#define __CS_FATLOOP_H__

#include "csutil/dirtyaccessarray.h"
#include "csutil/redblacktree.h"
#include "csutil/weakref.h"
#include "csgfx/shadervarcontext.h"

#include "iengine/mesh.h"
#include "iengine/rendersteps/irenderstep.h"

#include "csplugincommon/renderstep/basesteptype.h"
#include "csplugincommon/renderstep/basesteploader.h"

class csFatLoopType : public csBaseRenderStepType
{
public:
  csFatLoopType (iBase* p);

  virtual csPtr<iRenderStepFactory> NewFactory();
};

struct RenderPass
{
  csStringID shadertype;
  csRef<iShader> defShader;

  RenderPass() : shadertype (csInvalidStringID) {}
};

class csFatLoopLoader : public csBaseRenderStepLoader
{
  csStringHash tokens;
#define CS_TOKEN_ITEM_FILE "plugins/engine/renderloop/fatrl/fatloop.tok"
#include "cstool/tokenlist.h"

  bool ParsePass (iDocumentNode* node, RenderPass& pass);
public:
  csFatLoopLoader (iBase* p);

  virtual csPtr<iBase> Parse (iDocumentNode* node, 
    iLoaderContext* ldr_context, 	
    iBase* context);
};

class csFatLoopFactory : public iRenderStepFactory
{
private:
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  csFatLoopFactory (iObjectRegistry* object_reg);
  virtual ~csFatLoopFactory ();

  virtual csPtr<iRenderStep> Create ();
};

struct ShaderTicketKey
{
  long prio;
  iShader* shader;
  size_t ticket;
};

CS_SPECIALIZE_TEMPLATE
class csComparator<ShaderTicketKey, ShaderTicketKey>
{
public:
  static int Compare(ShaderTicketKey const &r1, ShaderTicketKey const &r2)
  {
    int d = r1.prio - r2.prio;
    if (d == 0)
    {
      d = (r1.shader < r2.shader) ? -1 : ((r1.shader > r2.shader) ? 1 : 0);
    }
    if (d == 0)
    {
      d = (r1.ticket < r2.ticket) ? -1 : ((r1.ticket > r2.ticket) ? 1 : 0);
    }
    return d;
  }
};

class csFatLoopStep : public iRenderStep
{
  iObjectRegistry* object_reg;

  // This is a growing array of visible meshes. It will contain
  // the visible meshes from every recursion level appended. At
  // exit of this step the visible meshes from the current recursion
  // level are removed again.
  csDirtyAccessArray<csRenderMesh*> visible_meshes;
  csDirtyAccessArray<iMeshWrapper*> imeshes_scratch;
  csDirtyAccessArray<iShaderVariableContext*> mesh_svc;

  csArray<csShaderVariableContext> shadervars;
  csWeakRef<iShaderManager> shaderManager;
  csRef<iEngine> engine;

  static csStringID string_object2world;

  struct MeshBucket
  {
    //uint timestamp;
    csDirtyAccessArray<csRenderMesh*> rendermeshes;
    csDirtyAccessArray<iMeshWrapper*> wrappers;
    csDirtyAccessArray<iShaderVariableContext*> contexts;

    //MeshBucket() : timestamp(~0) {}
  };
  typedef csRedBlackTreeMap<ShaderTicketKey, MeshBucket> SortedBuckets;
  csArray<SortedBuckets> buckets;
  class TraverseShaderBuckets
  {
    csFatLoopStep& step;
    iGraphics3D* g3d;
    csShaderVarStack &stacks;
    const RenderPass& pass;
  public:
    TraverseShaderBuckets (csFatLoopStep& step, iGraphics3D* g3d,
      csShaderVarStack &stacks, const RenderPass& pass) : step(step), 
      g3d(g3d), stacks(stacks), pass(pass) {}
    void Process (const ShaderTicketKey& key, MeshBucket& bucket);
  };

  inline void RenderMeshes (iGraphics3D* g3d, iShader* shader, 					
    size_t ticket, iShaderVariableContext** meshContext, 
    csRenderMesh** meshes, size_t num, csShaderVarStack &stacks);

  csArray<RenderPass> passes;
  uint32 Classify (csRenderMesh* mesh);
public:
  //csStringID shadertype;
  //csRef<iShader> defShader;

  SCF_DECLARE_IBASE;

  csFatLoopStep (iObjectRegistry* object_reg);
  virtual ~csFatLoopStep ();

  bool AddPass (const RenderPass& pass)
  { if (passes.Length() >= 32) return false; passes.Push (pass); return true; }

  virtual void Perform (iRenderView* rview, iSector* sector,
    csShaderVarStack &stacks);
};

#endif // __CS_FATLOOP_H__
