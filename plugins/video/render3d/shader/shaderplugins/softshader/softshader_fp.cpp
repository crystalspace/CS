/*
Copyright (C) 2002 by Marten Svanfeldt
                      Anders Stenberg

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "csgeom/math.h"
#include "csgeom/vector3.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "imap/services.h"
#include "iutil/document.h"
#include "iutil/string.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"

#include "softshader.h"
#include "softshader_fp.h"

CS_PLUGIN_NAMESPACE_BEGIN(SoftShader)
{

void csSoftShader_FP::Activate()
{
  shaderPlug->softSRI->SetScanlineRenderer (shaderPlug->scanlineRenderer);
}

void csSoftShader_FP::Deactivate()
{
  shaderPlug->softSRI->SetScanlineRenderer (0);
}

bool csSoftShader_FP::Load (iShaderDestinationResolver*, 
			    iDocumentNode* program)
{
  if(!program)
    return false;

  csRef<iDocumentNode> variablesnode = program->GetNode("softfp");
  if(variablesnode)
  {
    csRef<iDocumentNodeIterator> it = variablesnode->GetNodes ();
    while(it->HasNext())
    {
      csRef<iDocumentNode> child = it->Next();
      if(child->GetType() != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = tokens.Request (value);
      switch (id)
      {
	case XMLTOKEN_FLATCOLOR:
	  if (!ParseProgramParam (child, flatColor, ParamVector))
	    return false;
	  break;
	case XMLTOKEN_COLORFACTOR:
	  if (!ParseProgramParam (child, cfactor, ParamFloat))
	    return false;
	  break;
	case XMLTOKEN_ALPHAFACTOR:
	  if (!ParseProgramParam (child, afactor, ParamFloat))
	    return false;
	  break;
	case XMLTOKEN_COLORSUM:
	  {
	    bool b;
	    if (!synsrv->ParseBool (child, b, true))
	      return false;
	    colorSum = b;
	  }
	  break;
        default:
	  {
	    switch (commonTokens.Request (value))
	    {
	      case XMLTOKEN_PROGRAM:
	      case XMLTOKEN_VARIABLEMAP:
		// Don't want those
		synsrv->ReportBadToken (child);
		return false;
		break;
	      default:
		if (!ParseCommon (child))
		  return false;
	    }
	  }
      }
    }
  }
  return true;
}

static inline int FactorToShift (float f)
{
  f = csClamp (f, 65536.0f, 1.0f/32768.0f);
  if (f >= 1.0)
    return csLog2 ((int)f);
  else
    return -csLog2 ((int)(1.0f/f));
}

void csSoftShader_FP::SetupState (const csRenderMesh* /*mesh*/,
				  csRenderMeshModes& /*modes*/,
				  const iShaderVarStack* stacks)
{
  csVector4 v = GetParamVectorVal (stacks, flatColor, csVector4 (1));
  shaderPlug->scanlineRenderer->SetFlatColor (v);

  shaderPlug->scanlineRenderer->SetShift (
    FactorToShift (GetParamFloatVal (stacks, cfactor, 1.0f)),
    FactorToShift (GetParamFloatVal (stacks, afactor, 1.0f)));
  shaderPlug->scanlineRenderer->SetColorSum (colorSum);
}

bool csSoftShader_FP::Compile()
{
  return true;
}

}
CS_PLUGIN_NAMESPACE_END(SoftShader)
