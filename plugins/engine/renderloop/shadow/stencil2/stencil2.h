/* 
  Copyright (C) 2003 by Jorrit Tyberghein, Daniel Duhprey
            (C) Hristo Hristov, Boyan Hristov

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

#ifndef __CS_STENCIL2_H
#define __CS_STENCIL2_H

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

class csStencil2ShadowStep;
class csStencil2ShadowType;

class csStencil2ShadowCacheEntry : public iObjectModelListener
{
private:
  csStencil2ShadowStep* parent;
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

  struct Edge
  {
    int v1, v2;
    int face_1, face_2;
  };

  csHash<Edge *> edges_hash;
  csArray<Edge *> edges;
  csArray<int> silhouette_edges;

  int vertex_count, triangle_count;
  int edge_count;
  csArray<csVector3> face_normals;
  csDirtyAccessArray<int> edge_indices;
  csArray<csVector3> edge_midpoints;
  csArray<csVector3> edge_normals;

  csStencil2PolygonMesh* closedMesh;
  bool enable_caps;
  bool meshShadows;
public:
  SCF_DECLARE_IBASE;
  csRef<csRenderBufferHolder> bufferHolder;

  csStencil2ShadowCacheEntry (csStencil2ShadowStep* parent, 
    iMeshWrapper* mesh);
  virtual ~csStencil2ShadowCacheEntry ();

  virtual void ObjectModelChanged (iObjectModel* model);
  void EnableShadowCaps () { enable_caps = true; }
  void DisableShadowCaps () { enable_caps = false; }
  bool ShadowCaps () { return enable_caps; }

  bool MeshCastsShadow () { return meshShadows; }

  void UpdateBuffers() ;

  bool CalculateEdges();
  void AddEdge(int index_v1, int index_v2, int face_index);
  bool GetShadow(csVector3 &light_pos, float shadow_length,
  	bool front_cap, bool extrusion,  bool back_cap,
    	csArray<csVector3> & shadow_vertices, csArray<int> & shadow_indeces);

  void UpdateRenderBuffers(csArray<csVector3> & shadow_vertices,
  	csArray<int> & shadow_indeces);
};

class csStencil2ShadowStep : public iRenderStep,
  public iLightRenderStep,
  public iRenderStepContainer
{
private:
  friend class csStencil2ShadowCacheEntry;

  iObjectRegistry* object_reg;
  csWeakRef<iGraphics3D> g3d;
  csWeakRef<iShaderManager> shmgr;
  csRef<csStencil2ShadowType> type;

  bool enableShadows;
  csRefArray<iLightRenderStep> steps;

  csArray<iMeshWrapper*> shadowMeshes;
  csHash<csRef<csStencil2ShadowCacheEntry>, csPtrKey<iMeshWrapper> >
  	shadowcache;

  void Report (int severity, const char* msg, ...);

  void ModelInFrustum(csVector3 &light_pos, float shadow_length,
    csPlane3* frustum_planes, 
    uint32& frustum_mask, csBox3 &model_bounding_box,
    bool & front_cap_in_frustum, 
    bool & extrusion_in_frustum,
    bool & back_cap_in_frustum);

  int CalculateShadowMethod(iRenderView *rview, csVector3 &light_pos,
    const csReversibleTransform &t, csBox3 &model_bounding_box);

  void DrawShadow(iRenderView *rview, int method,
    csStencil2ShadowCacheEntry * cache_entry, 
    iMeshWrapper *mesh, csArray<csVector3> & shadow_vertices,
    csArray<int> & shadow_indeces, 
    iShader* shader, size_t shaderTicket, size_t pass);

public:
  SCF_DECLARE_IBASE;

  csStencil2ShadowStep (csStencil2ShadowType* type);
  virtual ~csStencil2ShadowStep ();

  bool Initialize (iObjectRegistry* objreg);

  void Perform (iRenderView* rview, iSector* sector,
    csShaderVarStack &stacks);
  void Perform (iRenderView* rview, iSector* sector, iLight* light,
    csShaderVarStack &stacks);

  virtual size_t AddStep (iRenderStep* step);
  virtual bool DeleteStep (iRenderStep* step);
  virtual iRenderStep* GetStep (size_t n) const;
  virtual size_t Find (iRenderStep* step) const;
  virtual size_t GetStepCount () const;

  struct ShadowDrawVisCallback : public iVisibilityCullerListener
  {
    csStencil2ShadowStep* parent;

    SCF_DECLARE_IBASE;
    ShadowDrawVisCallback ();
    virtual ~ShadowDrawVisCallback ();

    virtual void ObjectVisible (iVisibilityObject *visobject, 
      iMeshWrapper *mesh, uint32 frustum_mask);
  } shadowDrawVisCallback;
  friend struct ShadowDrawVisCallback;
};

class csStencil2ShadowFactory : public iRenderStepFactory
{
  iObjectRegistry* object_reg;
  csRef<csStencil2ShadowType> type;
public:
  SCF_DECLARE_IBASE;

  csStencil2ShadowFactory (iObjectRegistry* object_reg,
    csStencil2ShadowType* type);
  virtual ~csStencil2ShadowFactory ();

  virtual csPtr<iRenderStep> Create ();
};

class csStencil2ShadowType : public csBaseRenderStepType
{
  csRef<iShader> shadow;
  bool shadowLoaded;

  void Report (int severity, const char* msg, ...);
public:
  csStencil2ShadowType (iBase* p);
  virtual ~csStencil2ShadowType ();

  virtual csPtr<iRenderStepFactory> NewFactory ();

  iShader* GetShadow ();
};

class csStencil2ShadowLoader : public csBaseRenderStepLoader
{
  csRenderStepParser rsp;
  csStringHash tokens; 

#define CS_TOKEN_ITEM_FILE "plugins/engine/renderloop/shadow/stencil2/stencil2.tok"
#include "cstool/tokenlist.h"

public:
  csStencil2ShadowLoader (iBase *p);
  virtual ~csStencil2ShadowLoader ();

  virtual bool Initialize (iObjectRegistry* object_reg);

  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);
};

#endif // __CS_STENCILEN_H
