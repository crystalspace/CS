/* 
    Copyright (C) 2003 by Jorrit Tyberghein, Daniel Duhprey

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

#ifndef __CS_STENCIL_H
#define __CS_STENCIL_H

#include "csutil/ref.h"

#include "iutil/objreg.h"
#include "iutil/comp.h"
#include "ivideo/graph3d.h"
#include "ivideo/rndbuf.h"
#include "ivideo/shader/shader.h"
#include "iengine/rendersteps/irenderstep.h"
#include "iengine/rendersteps/icontainer.h"
#include "iengine/rendersteps/ilightiter.h"
#include "iengine/viscull.h"
#include "igeom/objmodel.h"
#include "igeom/polymesh.h"
#include "csutil/hash.h"
#include "csutil/csstring.h"
#include "csutil/strhash.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/weakref.h"
#include "csgfx/shadervarcontext.h"
#include "csplugincommon/renderstep/basesteptype.h"
#include "csplugincommon/renderstep/basesteploader.h"
#include "csplugincommon/renderstep/parserenderstep.h"

class csStencilShadowStep;
class csStencilShadowType;

class csStencilShadowCacheEntry : public iObjectModelListener				  
{
private:
  csStencilShadowStep* parent;
  iObjectModel* model;
  iMeshWrapper* meshWrapper;

  struct csLightCacheEntry 
  {
    iLight* light;
    csVector3 meshLightPos;
    csRef<iRenderBuffer> shadow_index_buffer;
    int edge_start, index_range;
  };
  csHash<csLightCacheEntry*, iLight*> lightcache;

  csRef<iRenderBuffer> shadow_vertex_buffer;
  csRef<iRenderBuffer> shadow_normal_buffer;
  csRef<iRenderBuffer> active_index_buffer;

  struct EdgeInfo 
  {
    csVector3 a, b;
    csVector3 norm;
    int ind_a, ind_b;
  };

  int vertex_count, triangle_count;
  int edge_count;
  csDirtyAccessArray<csVector3> face_normals;
  csDirtyAccessArray<int> edge_indices;
  csArray<csVector3> edge_midpoints;
  csArray<csVector3> edge_normals;

  // Mesh that was created when the original shadow mesh was auto-closed.
  // Kept so that a new one isn't alloced every time.
  csStencilPolygonMesh* closedMesh;

  bool enable_caps;

  bool meshShadows;

  inline void HandlePoly (const csVector3* vertices, const int* polyVertices, 
    const int numVerts, csArray<EdgeInfo>& edge_array, 
    csHash<EdgeInfo*>& edge_stack, int& NextEdge, int& TriIndex);
  void HandleEdge (EdgeInfo* e, csHash<EdgeInfo*>& edge_stack);
public:
  SCF_DECLARE_IBASE;
  csRef<csRenderBufferHolder> bufferHolder;

  csStencilShadowCacheEntry (csStencilShadowStep* parent, 
    iMeshWrapper* mesh);
  virtual ~csStencilShadowCacheEntry ();

  void SetActiveLight (iLight *light, csVector3 meshlightpos, 
    int& active_index_range, int& active_edge_start);
  virtual void ObjectModelChanged (iObjectModel* model);
  void EnableShadowCaps () { enable_caps = true; }
  void DisableShadowCaps () { enable_caps = false; }
  bool ShadowCaps () { return enable_caps; }

  bool MeshCastsShadow () { return meshShadows; }

  void UpdateBuffers() ;
};

class csStencilShadowStep : public iRenderStep,
			    public iLightRenderStep,
			    public iRenderStepContainer
{
private:
  friend class csStencilShadowCacheEntry;

  iObjectRegistry* object_reg;
  csWeakRef<iGraphics3D> g3d;
  csWeakRef<iShaderManager> shmgr;
  csRef<csStencilShadowType> type;

  bool enableShadows;
  csRefArray<iLightRenderStep> steps;

  csArray<iMeshWrapper*> shadowMeshes;
  csHash< csRef<csStencilShadowCacheEntry>, iMeshWrapper*> shadowcache;

  void DrawShadow (iRenderView* rview, iLight* light, iMeshWrapper *mesh, 
    iShader *shader, size_t shaderTicket, size_t pass);

  void Report (int severity, const char* msg, ...);

public:
  SCF_DECLARE_IBASE;

  csStencilShadowStep (csStencilShadowType* type);
  virtual ~csStencilShadowStep ();

  bool Initialize (iObjectRegistry* objreg);
  
  void Perform (iRenderView* rview, iSector* sector,
    csShaderVarStack &stacks);
  void Perform (iRenderView* rview, iSector* sector, iLight* light,
    csShaderVarStack &stacks);

  virtual size_t AddStep (iRenderStep* step);
  virtual size_t GetStepCount ();

  struct ShadowDrawVisCallback : public iVisibilityCullerListener
  {
    csStencilShadowStep* parent;

    SCF_DECLARE_IBASE;
    ShadowDrawVisCallback ();
    virtual ~ShadowDrawVisCallback ();

    virtual void ObjectVisible (iVisibilityObject *visobject, 
      iMeshWrapper *mesh, uint32 frustum_mask);
  } shadowDrawVisCallback;
  friend struct ShadowDrawVisCallback;
};

class csStencilShadowFactory : public iRenderStepFactory
{
  iObjectRegistry* object_reg;
  csRef<csStencilShadowType> type;
public:
  SCF_DECLARE_IBASE;

  csStencilShadowFactory (iObjectRegistry* object_reg,
      csStencilShadowType* type);
  virtual ~csStencilShadowFactory ();

  virtual csPtr<iRenderStep> Create ();

};

class csStencilShadowType : public csBaseRenderStepType
{
  csRef<iShader> shadow;
  bool shadowLoaded;

  void Report (int severity, const char* msg, ...);
public:
  csStencilShadowType (iBase* p);
  virtual ~csStencilShadowType ();

  virtual csPtr<iRenderStepFactory> NewFactory ();

  iShader* GetShadow ();
};

class csStencilShadowLoader : public csBaseRenderStepLoader
{
  csRenderStepParser rsp;

  csStringHash tokens; 
 #define CS_TOKEN_ITEM_FILE "plugins/engine/renderloop/shadow/stencil/stencil.tok"
 #include "cstool/tokenlist.h"

public:
  csStencilShadowLoader (iBase *p);
  virtual ~csStencilShadowLoader ();

  virtual bool Initialize (iObjectRegistry* object_reg);

  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iLoaderContext* ldr_context,
	iBase* context);
};

#endif // __CS_STENCIL_H
