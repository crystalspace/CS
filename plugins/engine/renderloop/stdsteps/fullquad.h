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
#include "csutil/weakref.h"
#include "iengine/light.h"
#include "iengine/renderloop.h"
#include "iengine/rendersteps/irenderstep.h"
#include "ivideo/shader/shader.h"

#include "csplugincommon/renderstep/basesteptype.h"
#include "csplugincommon/renderstep/basesteploader.h"

class csFullScreenQuadRSType : public csBaseRenderStepType
{
public:
  csFullScreenQuadRSType (iBase* p);

  virtual csPtr<iRenderStepFactory> NewFactory();
};

class csFullScreenQuadRenderStepFactory : public iRenderStepFactory
{
private:
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  csFullScreenQuadRenderStepFactory (iObjectRegistry* object_reg);
  virtual ~csFullScreenQuadRenderStepFactory ();

  virtual csPtr<iRenderStep> Create ();
};

class csFullScreenQuadRenderStep : public iRenderStep
{
public:
  struct DrawSettings
  {
    csStringID shadertype;
    csString material;
    csString shader;
    csString texture;
    uint mixmode;
    csAlphaMode alphaMode;
    csRef<csShaderVariableContext> svContext;
  };
private:
  csWeakRef<iEngine> engine;
  csWeakRef<iShaderManager> shaderMgr;
  iObjectRegistry* object_reg;

  DrawSettings firstPass;
  DrawSettings otherPasses;
  bool distinguishFirstPass;
  bool isFirstPass;

  //csFullscreenQuad* fullquad;

public:
  SCF_DECLARE_IBASE;

  csFullScreenQuadRenderStep (iObjectRegistry* object_reg);
  virtual ~csFullScreenQuadRenderStep ();

  virtual void Perform (iRenderView* rview, iSector* sector,
    csShaderVarStack &stacks);

  /*void SetMaterial (const char* m)
  { material = m; }

  void SetShaderType (csStringID s)
  { shadertype = s; }*/
  void SetDistinguishFirstPass (bool b)
  { distinguishFirstPass = b; }
  bool GetDistinguishFirstPass () { return distinguishFirstPass; }

  DrawSettings& GetFirstSettings () { return firstPass; }
  DrawSettings& GetOtherSettings () { return otherPasses; }
};

class csFullScreenQuadRSLoader : public csBaseRenderStepLoader
{
  csStringHash tokens;
#define CS_TOKEN_ITEM_FILE "plugins/engine/renderloop/stdsteps/fullquad.tok"
#include "cstool/tokenlist.h"

  bool ParseStep (iDocumentNode* node, 
    csFullScreenQuadRenderStep* step, 
    csFullScreenQuadRenderStep::DrawSettings& settings,
    bool firstPass);
public:
  csFullScreenQuadRSLoader (iBase* p);

  virtual csPtr<iBase> Parse (iDocumentNode* node, 
    iLoaderContext* ldr_context, 	
    iBase* context);
};

#endif // __CS_FULLSCREENQUAD_H__
