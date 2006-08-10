/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter
              (C) 2003 by Anders Stenberg
              (C) 2006 by Hristo Hristov

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

#ifndef __CS_SHADOWMAP_H__
#define __CS_SHADOWMAP_H__

#include "csutil/scf_implementation.h"
#include "csutil/csstring.h"
#include "csutil/weakref.h"
#include "iengine/light.h"
#include "iengine/renderloop.h"
#include "iengine/rendersteps/irenderstep.h"
#include "iengine/rendersteps/ilightiter.h"
#include "ivideo/shader/shader.h"

#include "csplugincommon/renderstep/basesteptype.h"
#include "csplugincommon/renderstep/basesteploader.h"
#include "csplugincommon/renderstep/parserenderstep.h"
#include <csutil/dirtyaccessarray.h>

#include <iutil/eventh.h>
#include <iutil/eventq.h>
#include <iutil/virtclk.h>
#include <iengine/viscull.h>

class csShadowmapRSType :
  public scfImplementationExt0<csShadowmapRSType, csBaseRenderStepType>
{
public:
  csShadowmapRSType (iBase* p);

  virtual csPtr<iRenderStepFactory> NewFactory();
};

class csShadowmapRenderStepFactory :
  public scfImplementation1<csShadowmapRenderStepFactory, iRenderStepFactory>
{
private:
  iObjectRegistry* object_reg;

public:
  csShadowmapRenderStepFactory (iObjectRegistry* object_reg);
  virtual ~csShadowmapRenderStepFactory ();

  virtual csPtr<iRenderStep> Create ();
};

//-----------------------------------------------

class csShadowmapRenderStep : 
  public scfImplementation3<csShadowmapRenderStep,
    iRenderStep, iLightIterRenderStep, iVisibilityCullerListener>
{
public:
  struct DrawSettings
  {
    csStringID shadertype;
    csString shader;
    uint mixmode;
    csAlphaMode alphaMode;
    csRef<csShaderVariableContext> svContext;
  };
  csRef<iGraphics3D> g3d;
  iTextureHandle *context;
  csRefArray<iTextureHandle> depth_textures;
  csStringID depth_cubemap_name;
  csRef<iTextureHandle> depth_cubemap;

private:
  csRefArray<iLightRenderStep> steps;
  csWeakRef<iEngine> engine;
  csWeakRef<iShaderManager> shaderMgr;
  iObjectRegistry* object_reg;
  csRef<iShader> defShader;

  csDirtyAccessArray<csRenderMesh*> render_meshes;
  csDirtyAccessArray<iMeshWrapper*> mesh_wrappers;

  csStringID bones_name;
  csStringID shader_name;

  DrawSettings settings;

  csArray<iMeshWrapper*> lightMeshes;
  csRenderMeshList *mesh_list;

public:
  csShadowmapRenderStep (iObjectRegistry* object_reg);
  virtual ~csShadowmapRenderStep ();

  virtual void Perform (iRenderView* rview, iSector* sector,
    iShaderVarStack* stacks);

  DrawSettings& GetSettings () { return settings; }
  void SetDefaultShader (iShader* shader)
  { defShader = shader; }

  virtual void ObjectVisible (iVisibilityObject *visobject, 
    iMeshWrapper *mesh, uint32 frustum_mask);
};

class csShadowmapRSLoader :
  public scfImplementationExt0<csShadowmapRSLoader, csBaseRenderStepLoader>
{
  csRenderStepParser rsp;
  csStringHash tokens;
#define CS_TOKEN_ITEM_FILE "plugins/engine/renderloop/stdsteps/shadowmap.tok"

#include "cstool/tokenlist.h"

  bool ParseStep (iDocumentNode* node, 
    csShadowmapRenderStep* step, 
    csShadowmapRenderStep::DrawSettings& settings);
public:
  csShadowmapRSLoader (iBase* p);

  virtual bool Initialize (iObjectRegistry* object_reg);

  virtual csPtr<iBase> Parse (iDocumentNode* node, 
    iStreamSource*, iLoaderContext* ldr_context, 	
    iBase* context);
};

#endif // __CS_SHADOWMAP_H__
