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
#include "ivideo/rendersteps/irenderstep.h"
#include "ivideo/rendersteps/icontainer.h"
#include "ivideo/rendersteps/ilightiter.h"
#include "iengine/viscull.h"
#include "igeom/objmodel.h"
#include "igeom/polymesh.h"
#include "csutil/hashmap.h"
#include "csutil/csstring.h"
#include "csutil/strhash.h"
#include "../../common/basesteptype.h"
#include "../../common/basesteploader.h"
#include "../../common/parserenderstep.h"

class csStencilShadowCacheEntry : public iObjectModelListener,
				  public iRenderBufferSource
{
private:
  iObjectModel *model;

  struct csLightCacheEntry 
  {
    iLight* light;
    csVector3 meshLightPos;
    csRef<iRenderBuffer> shadow_index_buffer;
    int edge_start, index_range;
  };
  csHashMap lightcache;

  csStringID shadow_vertex_name, shadow_normal_name, shadow_index_name;
  csRef<iRenderBuffer> shadow_vertex_buffer;
  csRef<iRenderBuffer> shadow_normal_buffer;
  csRef<iRenderBuffer> active_index_buffer;
  csRef<iGraphics3D> g3d;

  struct EdgeInfo {
    csVector3 a, b;
    csVector3 norm;
    int ind_a, ind_b;
  };

  int vertex_count, triangle_count;
  int edge_count;
  int *edge_indices;
  csVector3 *edge_midpoints, *edge_normals;

  bool enable_caps;

  void HandleEdge (EdgeInfo* e, csHashMap& edge_stack);
public:
  SCF_DECLARE_IBASE;

  csStencilShadowCacheEntry (iBase* parent);
  virtual ~csStencilShadowCacheEntry ();

  bool Initialize (iObjectRegistry *objreg);

  void SetActiveLight (iLight *light, csVector3 meshlightpos, int& active_index_range, int& active_edge_start);
  virtual void ObjectModelChanged (iObjectModel* model);
  virtual iRenderBuffer *GetRenderBuffer (csStringID name);
  void EnableShadowCaps () { enable_caps = true; }
  void DisableShadowCaps () { enable_caps = false; }
  bool ShadowCaps () { return enable_caps; }
};

class csStencilShadowStep : public iRenderStep,
			    public iLightRenderStep,
			    public iRenderStepContainer
{
private:
  csRef<iObjectRegistry> object_reg;
  csRef<iGraphics3D> g3d;
  csRef<iShader> shadow;

  csRefArray<iLightRenderStep> steps;

  csHashMap shadowcache;
  void DrawShadow (iRenderView* rview, iLight* light, iMeshWrapper *mesh, iShaderPass *pass);
public:
  SCF_DECLARE_IBASE;

  csStencilShadowStep (iBase *parent);
  virtual ~csStencilShadowStep ();

  bool Initialize (iObjectRegistry* objreg);
  
  void Perform (iRenderView* rview, iSector* sector);
  void Perform (iRenderView* rview, iSector* sector, iLight* light);

  virtual int AddStep (iRenderStep* step);
  virtual int GetStepCount ();

/*  struct ShadowDrawVisCallback : public iVisibilityCullerListener
  {    
    csStencilShadowStep* parent;

    SCF_DECLARE_IBASE;
    ShadowDrawVisCallback (csStencilShadowStep* parent);
    virtual ~ShadowDrawVisCallback ();

    virtual void ObjectVisible (iVisibilityObject *visobject, 
      iMeshWrapper *mesh);
  } shadowDrawVisCallback;*/
};

class csStencilShadowFactory : public iRenderStepFactory
{
  csRef<iObjectRegistry> object_reg;
public:
  SCF_DECLARE_IBASE;

  csStencilShadowFactory (iBase *p, iObjectRegistry* object_reg);
  virtual ~csStencilShadowFactory ();

  virtual csPtr<iRenderStep> Create ();

};

class csStencilShadowType : public csBaseRenderStepType
{
public:
  csStencilShadowType (iBase* p);
  virtual ~csStencilShadowType ();

  virtual csPtr<iRenderStepFactory> NewFactory ();
};

class csStencilShadowLoader : public csBaseRenderStepLoader
{
  csRenderStepParser rsp;

  csStringHash tokens; 
 #define CS_TOKEN_ITEM_FILE "video/render3d/renderloop/shadow/stencil/stencil.tok"
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
