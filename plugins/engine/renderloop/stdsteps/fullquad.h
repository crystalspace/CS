/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter
              (C) 2003 by Anders Stenberg

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

#ifndef __CS_FULLSCREENQUAD_H__
#define __CS_FULLSCREENQUAD_H__

#include "csutil/scf.h"
#include "csutil/csstring.h"
#include "iengine/light.h"
#include "iengine/renderloop.h"
#include "iengine/rendersteps/irenderstep.h"
#include "ivideo/shader/shader.h"

#include "../common/basesteptype.h"
#include "../common/basesteploader.h"

class csFullscreenQuad;

class csFullScreenQuadRSType : public csBaseRenderStepType
{
public:
  csFullScreenQuadRSType (iBase* p);

  virtual csPtr<iRenderStepFactory> NewFactory();
};

class csFullScreenQuadRSLoader : public csBaseRenderStepLoader
{
  csStringHash tokens;
#define CS_TOKEN_ITEM_FILE "engine/renderloop/stdsteps/fullquad.tok"
#include "cstool/tokenlist.h"

public:
  csFullScreenQuadRSLoader (iBase* p);

  virtual csPtr<iBase> Parse (iDocumentNode* node, 
    iLoaderContext* ldr_context, 	
    iBase* context);
};

class csFullScreenQuadRenderStepFactory : public iRenderStepFactory
{
  csRef<iObjectRegistry> object_reg;
public:
  SCF_DECLARE_IBASE;

  csFullScreenQuadRenderStepFactory (iObjectRegistry* object_reg);
  virtual ~csFullScreenQuadRenderStepFactory ();

  virtual csPtr<iRenderStep> Create ();
};

class csFullScreenQuadRenderStep : public iRenderStep
{
private:
  csStringID shadertype;
  csString material;
  csRef<iEngine> engine;
  csFullscreenQuad* fullquad;

public:
  SCF_DECLARE_IBASE;

  csFullScreenQuadRenderStep (iObjectRegistry* object_reg);
  virtual ~csFullScreenQuadRenderStep ();

  virtual void Perform (iRenderView* rview, iSector* sector,
    csShaderVarStack &stacks);

  void SetMaterial (const char* m)
  { material = m; }

  void SetShaderType (csStringID s)
  { shadertype = s; }
};


#endif // __CS_FULLSCREENQUAD_H__
