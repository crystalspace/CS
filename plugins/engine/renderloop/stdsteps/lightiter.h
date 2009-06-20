/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter
              (C) 2005 by Marten Svanfeldt

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

#ifndef __CS_LIGHTITER_H__
#define __CS_LIGHTITER_H__

#include "csutil/scf_implementation.h"
#include "csutil/csstring.h"
#include "csutil/weakref.h"
#include "csutil/leakguard.h"
#include "iengine/renderloop.h"
#include "iengine/rendersteps/irenderstep.h"
#include "iengine/rendersteps/ilightiter.h"
#include "iengine/lightmgr.h"
#include "ivideo/shader/shader.h"

#include "csplugincommon/rendermanager/lightsetup.h"
#include "csplugincommon/renderstep/basesteptype.h"
#include "csplugincommon/renderstep/basesteploader.h"
#include "csplugincommon/renderstep/parserenderstep.h"

class csLightIterRSType :
  public scfImplementationExt0<csLightIterRSType, csBaseRenderStepType>
{
public:
  CS_LEAKGUARD_DECLARE (csLightIterRSType);

  csLightIterRSType (iBase* p);

  virtual csPtr<iRenderStepFactory> NewFactory();
};

class csLightIterRSLoader :
  public scfImplementationExt0<csLightIterRSLoader, csBaseRenderStepLoader>
{
private:
  csRenderStepParser rsp;

  csStringHash tokens;
#define CS_TOKEN_ITEM_FILE "plugins/engine/renderloop/stdsteps/lightiter.tok"
#include "cstool/tokenlist.h"

public:
  CS_LEAKGUARD_DECLARE (csLightIterRSLoader);

  csLightIterRSLoader (iBase* p);

  virtual bool Initialize (iObjectRegistry* object_reg);

  virtual csPtr<iBase> Parse (iDocumentNode* node, 
    iStreamSource*, iLoaderContext* ldr_context, 	
    iBase* context);

  virtual bool IsThreadSafe() { return true; }
};

class csLightIterRenderStepFactory :
  public scfImplementation1<csLightIterRenderStepFactory, iRenderStepFactory>
{
private:
  iObjectRegistry* object_reg;

public:
  CS_LEAKGUARD_DECLARE (csLightIterRenderStepFactory);

  csLightIterRenderStepFactory (iObjectRegistry* object_reg);
  virtual ~csLightIterRenderStepFactory ();

  virtual csPtr<iRenderStep> Create ();
};

class csLightIterRenderStep :
  public scfImplementation3<csLightIterRenderStep,
    iRenderStep, iLightIterRenderStep, iRenderStepContainer>
{
private:
  csRefArray<iLightRenderStep> steps;

  iObjectRegistry* object_reg;

  csRef<iShaderManager> shadermgr;
  csRef<iLightManager> lightmgr;
  bool initialized;

  uint lastLSVHelperFrame;
  CS::RenderManager::LightingVariablesHelper::PersistentData
    lightSvHelperPersist;
  CS::ShaderVarStringID sv_attn_tex_name;
public:
  csWeakRef<iGraphics3D> g3d;

  csLightIterRenderStep (iObjectRegistry* object_reg);
  virtual ~csLightIterRenderStep ();

  void Init ();
  virtual void Perform (iRenderView* rview, iSector* sector,
    csShaderVariableStack& stack);

  virtual size_t AddStep (iRenderStep* step);
  virtual bool DeleteStep (iRenderStep* step);
  virtual iRenderStep* GetStep (size_t n) const;
  virtual size_t Find (iRenderStep* step) const;
  virtual size_t GetStepCount () const;
};


#endif
