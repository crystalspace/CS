/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

#include "csutil/scf.h"
#include "csutil/csstring.h"
#include "csutil/weakref.h"
#include "csutil/leakguard.h"
#include "iengine/renderloop.h"
#include "iengine/rendersteps/irenderstep.h"
#include "iengine/rendersteps/ilightiter.h"
#include "ivideo/shader/shader.h"

#include "csplugincommon/renderstep/basesteptype.h"
#include "csplugincommon/renderstep/basesteploader.h"
#include "csplugincommon/renderstep/parserenderstep.h"

class csLightIterRSType : public csBaseRenderStepType
{
public:
  CS_LEAKGUARD_DECLARE (csLightIterRSType);

  csLightIterRSType (iBase* p);

  virtual csPtr<iRenderStepFactory> NewFactory();
};

class csLightIterRSLoader : public csBaseRenderStepLoader
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
    iLoaderContext* ldr_context, 	
    iBase* context);
};

class csLightIterRenderStepFactory : public iRenderStepFactory
{
private:
  iObjectRegistry* object_reg;

public:
  CS_LEAKGUARD_DECLARE (csLightIterRenderStepFactory);

  SCF_DECLARE_IBASE;

  csLightIterRenderStepFactory (iObjectRegistry* object_reg);
  virtual ~csLightIterRenderStepFactory ();

  virtual csPtr<iRenderStep> Create ();
};

class csLightIterRenderStep : public iRenderStep, 
			      public iLightIterRenderStep,
			      public iRenderStepContainer
{
private:
  csRefArray<iLightRenderStep> steps;

  iObjectRegistry* object_reg;
  csRef<csShaderVariable> shvar_light_0_position;
  csRef<csShaderVariable> shvar_light_0_position_world;
  csRef<csShaderVariable> shvar_light_0_diffuse;
  csRef<csShaderVariable> shvar_light_0_specular;
  csRef<csShaderVariable> shvar_light_0_attenuation;
  csRef<csShaderVariable> shvar_light_0_attenuationtex;
  bool initialized;

  class LightSVAccessor : public iLightCallback,
			  public iShaderVariableAccessor
  {
  private:
    iLight* light;
    csLightIterRenderStep* parent;

    csRef<iTextureHandle> attTex;
    int attnType;

    bool needUpdate;
  public:
    CS_LEAKGUARD_DECLARE (LightSVAccessor);
    SCF_DECLARE_IBASE;

    LightSVAccessor (iLight* light, csLightIterRenderStep* parent);
    virtual ~LightSVAccessor ();

    virtual void OnColorChange (iLight* light, const csColor& newcolor);
    virtual void OnPositionChange (iLight* light, const csVector3& newpos);
    virtual void OnSectorChange (iLight* light, iSector* newsector);
    virtual void OnRadiusChange (iLight* light, float newradius);
    virtual void OnDestroy (iLight* light);
    virtual void OnAttenuationChange (iLight* light, int newatt);

    virtual void PreGetValue (csShaderVariable *variable);
  };
  friend class LightSVAccessor;

  csHash<LightSVAccessor*, iLight*> knownLights;
  csRef<iTextureHandle> attTex;

  LightSVAccessor* GetLightAccessor (iLight* light);

public:
  csWeakRef<iGraphics3D> g3d;

  SCF_DECLARE_IBASE;

  csLightIterRenderStep (iObjectRegistry* object_reg);
  virtual ~csLightIterRenderStep ();

  void Init ();
  virtual void Perform (iRenderView* rview, iSector* sector,
    csShaderVarStack &stacks);

  virtual size_t AddStep (iRenderStep* step);
  virtual size_t GetStepCount ();

  csPtr<iTextureHandle> GetAttenuationTexture (int attnType);
  csPtr<iTextureHandle> GetAttenuationTexture (const csVector3& attnVec);
};


#endif
