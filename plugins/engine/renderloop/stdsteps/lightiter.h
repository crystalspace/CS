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
#include "iengine/renderloop.h"
#include "ivideo/rendersteps/irenderstep.h"
#include "ivideo/rendersteps/ilightiter.h"
#include "ivideo/shader/shader.h"

#include "../common/basesteptype.h"
#include "../common/basesteploader.h"
#include "../common/parserenderstep.h"

class csLightIterRSType : public csBaseRenderStepType
{
public:
  csLightIterRSType (iBase* p);

  virtual csPtr<iRenderStepFactory> NewFactory();
};

class csLightIterRSLoader : public csBaseRenderStepLoader
{
  csRenderStepParser rsp;

  csStringHash tokens;
#define CS_TOKEN_ITEM_FILE "engine/renderloop/stdsteps/lightiter.tok"
#include "cstool/tokenlist.h"

public:
  csLightIterRSLoader (iBase* p);

  virtual bool Initialize (iObjectRegistry* object_reg);

  virtual csPtr<iBase> Parse (iDocumentNode* node, 
    iLoaderContext* ldr_context, 	
    iBase* context);
};

class csLightIterRenderStepFactory : public iRenderStepFactory
{
  csRef<iObjectRegistry> object_reg;
public:
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

  csRef<iObjectRegistry> object_reg;
  csRef<csShaderVariable> shvar_light_0_position;
  csRef<csShaderVariable> shvar_light_0_diffuse;
  csRef<csShaderVariable> shvar_light_0_specular;
  csRef<csShaderVariable> shvar_light_0_attenuation;
  bool initialized;
public:
  SCF_DECLARE_IBASE;

  csLightIterRenderStep (iObjectRegistry* object_reg);
  virtual ~csLightIterRenderStep ();

  void InitVariables ();
  virtual void Perform (iRenderView* rview, iSector* sector);

  virtual int AddStep (iRenderStep* step);
  virtual int GetStepCount ();
};


#endif
