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

#ifndef __CS_GENERIC_H__
#define __CS_GENERIC_H__

#include "csutil/scf.h"
#include "csutil/csstring.h"
#include "iengine/light.h"
#include "iengine/renderloop.h"
#include "iengine/viscull.h"
#include "ivideo/rendersteps/irenderstep.h"
#include "ivideo/rendersteps/igeneric.h"
#include "ivideo/rendersteps/ilightiter.h"
#include "ivideo/shader/shader.h"

#include "../common/basesteptype.h"
#include "../common/basesteploader.h"

class csGenericRSType : public csBaseRenderStepType
{
public:
  csGenericRSType (iBase* p);

  virtual csPtr<iRenderStepFactory> NewFactory();
};

class csGenericRSLoader : public csBaseRenderStepLoader
{
  csStringHash tokens;
#define CS_TOKEN_ITEM_FILE "engine/renderloop/stdsteps/generic.tok"
#include "cstool/tokenlist.h"

public:
  csGenericRSLoader (iBase* p);

  virtual csPtr<iBase> Parse (iDocumentNode* node, 
    iLoaderContext* ldr_context, 	
    iBase* context);
};

class csGenericRenderStepFactory : public iRenderStepFactory
{
  csRef<iObjectRegistry> object_reg;
public:
  SCF_DECLARE_IBASE;

  csGenericRenderStepFactory (iObjectRegistry* object_reg);
  virtual ~csGenericRenderStepFactory ();

  virtual csPtr<iRenderStep> Create ();
};

class csGenericRenderStep : public iRenderStep, 
			    public iGenericRenderStep,
			    public iLightRenderStep
{
private:
//  csRenderLoop* rl;
  csStringID shadertype;
  bool zOffset;
  csZBufMode zmode;
  csRef<iStringSet> strings;

  // @@@ Veeery rough
  class ViscullCallback : public iVisibilityCullerListener
  {
    iGraphics3D* g3d;
    csStringID shadertype;
    iRenderView* rview;
  public:
    SCF_DECLARE_IBASE;

    ViscullCallback (iGraphics3D* g3d, csStringID shadertype, 
      iRenderView* rview);
    virtual ~ViscullCallback () {}

    void ObjectVisible (iVisibilityObject *visobject, 
      iMeshWrapper *mesh);
  };

public:
  SCF_DECLARE_IBASE;

  csGenericRenderStep (iObjectRegistry* object_reg);
  virtual ~csGenericRenderStep ();

  virtual void Perform (iRenderView* rview, iSector* sector);
  virtual void Perform (iRenderView* rview, iSector* sector,
    iLight* light);

  virtual void SetShaderType (const char* type);
  virtual const char* GetShaderType ();

  virtual void SetZOffset (bool zOffset);
  virtual bool GetZOffset ();

  virtual void SetZBufMode (csZBufMode zmode);
  virtual csZBufMode GetZBufMode ();

  inline static void RenderMeshes (iGraphics3D* g3d, iShaderWrapper* shader, 
    csRenderMesh** meshes, int num);
};


#endif
