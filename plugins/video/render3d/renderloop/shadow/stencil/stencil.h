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
#include "ivideo/rendersteps/ilightiter.h"
#include "iengine/viscull.h"
#include "igeom/objmodel.h"
#include "igeom/polymesh.h"
#include "csutil/hashmap.h"
#include "csutil/strhash.h"
#include "../../common/basesteptype.h"
#include "../../common/basesteploader.h"

class csStencilShadowCacheEntry : public iObjectModelListener
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
  csRef<iGraphics3D> r3d;

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
  void ObjectModelChanged (iObjectModel* model);
  iRenderBuffer *GetRenderBuffer (csStringID name);
  void EnableShadowCaps () { enable_caps = true; }
  void DisableShadowCaps () { enable_caps = false; }
  bool ShadowCaps () { return enable_caps; }

  class RenderBufferSource : public iRenderBufferSource
  {
    SCF_DECLARE_EMBEDDED_IBASE (csStencilShadowCacheEntry);
	virtual iRenderBuffer *GetRenderBuffer (csStringID name)
	{ return scfParent->GetRenderBuffer (name); }
  } scfiRenderBufferSource;

  struct Component : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csStencilShadowCacheEntry);
	virtual bool Initialize (iObjectRegistry *objreg)
	{ return scfParent->Initialize (objreg); }
  } scfiComponent;
};

class csStencilShadowStep : public iRenderStep
{
private:
  csRef<iObjectRegistry> object_reg;
  csRef<iGraphics3D> r3d;
  csRef<iShader> shadow;

  csHashMap shadowcache;
  void DrawShadow (iRenderView* rview, iLight* light, iMeshWrapper *mesh, iShaderPass *pass);
public:
  SCF_DECLARE_IBASE;

  csStencilShadowStep (iBase *parent);
  virtual ~csStencilShadowStep ();

  bool Initialize (iObjectRegistry* objreg);
  
  void Perform (iRenderView* rview, iSector* sector);
  void Perform (iRenderView* rview, iSector* sector, iLight* light);

  struct LightRenderStep : public iLightRenderStep
  {
    SCF_DECLARE_EMBEDDED_IBASE (csStencilShadowStep);
    virtual void Perform (iRenderView* rview, iSector* sector, iLight* light) 
    { return scfParent->Perform (rview, sector, light); }
  } scfiLightRenderStep;

  struct Component : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csStencilShadowStep);
    virtual bool Initialize (iObjectRegistry *objreg)
    { return scfParent->Initialize (objreg); }
  } scfiComponent;

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
  csStringHash tokens; 
// #define CS_TOKEN_ITEM_FILE "video/render3d/renderloop/shadow/stencil/stencil.tok"
// #include "cstool/tokenlist.h"

public:
  csStencilShadowLoader (iBase *p);
  virtual ~csStencilShadowLoader ();

  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iLoaderContext* ldr_context,
	iBase* context);
};

#endif // __CS_STENCIL_H
