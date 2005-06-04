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

#include "rendernode.h"
#include "meshnode.h"

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

class csFatLoopStep : public iRenderStep
{
  iObjectRegistry* object_reg;

  csSet<csPtrKey<iSector> > sectorSet; // @@@ Hack.

  csWeakRef<iShaderManager> shaderManager;
  csRef<iEngine> engine;
  csWeakRef<iShader> nullShader;

  struct RenderNode
  {
    enum Type { Container, Portal };
    Type nodeType;

    iPortal* portal;
    csPoly2D poly;
    csReversibleTransform movtrans;
    csPlane3 camera_plane;

    csRenderNode* renderNode;
    iRenderView* rview;

    csArray<RenderNode*> containedNodes;

    RenderNode() : renderNode(0), rview(0) {}
    ~RenderNode() { if (renderNode) renderNode->Free(); }
  };
  csBlockAllocator<RenderNode> renderNodeAlloc;

  csArray<RenderPass> passes;
  uint32 Classify (csRenderMesh* mesh);

  csMeshRenderNodeFactory meshNodeFact;

  void BuildNodeGraph (RenderNode* node, iRenderView* rview, 
    iSector* sector, csShaderVarStack &stacks);
  void BuildPortalNodes (RenderNode* node, iMeshWrapper* meshwrapper, 
    iPortalContainer* portals, iRenderView* rview, csShaderVarStack &stacks);
  void ProcessNode (iRenderView* rview, RenderNode* node,
    csShaderVarStack &stacks);
  void RenderPortal (RenderNode* node, iRenderView* rview,
    csShaderVarStack &stacks);

  // Camera space data.
  csDirtyAccessArray<csVector3> camera_vertices;
  csArray<csPlane3> camera_planes;
  void DoPortal (RenderNode* node, iPortal* portal, const csPoly2D& poly,
    const csReversibleTransform& movtrans, const csPlane3& camera_plane, 
    iRenderView* rview, csShaderVarStack &stacks);
  bool ClipToPlane (iPortal* portal, csPlane3 *portal_plane, 
    const csVector3 &v_w2c, csVector3 * &pverts, int &num_verts);
  bool DoPerspective (csVector3 *source, int num_verts, csPoly2D *dest, 
    bool mirror, int fov, float shift_x, float shift_y, 
    const csPlane3& plane_cam);
public:
  SCF_DECLARE_IBASE;

  csFatLoopStep (iObjectRegistry* object_reg);
  virtual ~csFatLoopStep ();

  bool AddPass (const RenderPass& pass)
  { if (passes.Length() >= 32) return false; passes.Push (pass); return true; }

  virtual void Perform (iRenderView* rview, iSector* sector,
    csShaderVarStack &stacks);
};

#endif // __CS_FATLOOP_H__
