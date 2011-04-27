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

#include "csgeom/vector3.h"
#include "csplugincommon/opengl/glextmanager.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scanstr.h"
#include "csutil/scf.h"
#include "csutil/scfstr.h"
#include "csutil/stringreader.h"
#include "iutil/document.h"
#include "iutil/string.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"
#include "iutil/databuff.h"

#include "glshader_cgvp.h"
#include "glshader_cgfp.h"
#include "glshader_cg.h"
#include "profile_limits.h"

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{

CS_LEAKGUARD_IMPLEMENT (csShaderGLCGVP);

bool csShaderGLCGVP::Compile (iHierarchicalCache* cache, csRef<iString>* tag)
{
  if (!shaderPlug->enableVP) return false;

  csRef<iDataBuffer> programBuffer = GetProgramData();
  if (!programBuffer.IsValid())
    return false;
  csString programStr;
  programStr.Append ((char*)programBuffer->GetData(), programBuffer->GetSize());

  bool ret = DefaultLoadProgram (cgResolve, programStr, progVP,
    shaderPlug->currentLimits);

  csString tagStr (csString("CG") + shaderPlug->currentLimits.ToString());
  WriteToCache (cache, shaderPlug->currentLimits.vp, 
    shaderPlug->currentLimits, tagStr);
  tag->AttachNew (new scfString (tagStr));
  
  cacheKeepNodes.DeleteAll ();
  return ret;
}

bool csShaderGLCGVP::Precache (const ProfileLimitsPair& limits,
                               const char* tag,
                               iHierarchicalCache* cache)
{
  PrecacheClear();

  csRef<iDataBuffer> programBuffer = GetProgramData();
  if (!programBuffer.IsValid())
    return false;
  csString programStr;
  programStr.Append ((char*)programBuffer->GetData(), programBuffer->GetSize());

  ProgramObject programObj;
  bool needBuild = true;
  csString sourcePreproc;
  {
    csString programStr;
    programStr.Append ((char*)programBuffer->GetData(), programBuffer->GetSize());
    
    // Get preprocessed result of pristine source
    sourcePreproc = GetAugmentedProgram (programStr);
    if (!sourcePreproc.IsEmpty ())
    {
      // Check preprocessed source against cache
      if (shaderPlug->progCache.SearchObject (sourcePreproc, limits.fp, programObj))
        needBuild = false;
    }
  }
  
  bool ret;
  if (needBuild)
  {
    ret = DefaultLoadProgram (cgResolve, programStr, progVP, 
      limits,
      loadApplyVmap | loadFlagUnusedV2FForInit);
    WriteToCache (cache, limits.vp, limits, csString("CG") + tag);
  }
  else
  {
    ret = true;
    unusedParams = programObj.GetUnusedParams();
    WriteToCache (cache, limits.fp, limits, csString("CG") + tag,
      programObj);
  }


  return ret;
}

iShaderProgram::CacheLoadResult csShaderGLCGVP::LoadFromCache (
  iHierarchicalCache* cache, iBase* previous, iDocumentNode* programNode,
  csRef<iString>* failReason, csRef<iString>* tag)
{
  if (!shaderPlug->enableVP)
  {
    if (failReason)
      failReason->AttachNew (new scfString ("Cg VP not available or disabled"));
    /* Claim a load success, but invalid shader, to prevent loading from
	scratch (which will fail anyway) */
    return loadSuccessShaderInvalid;
  }
  return csShaderGLCGCommon::LoadFromCache (cache, previous, programNode,
    failReason, tag, 0);
}

csVertexAttrib csShaderGLCGVP::ResolveBufferDestination (const char* binding)
{
  csVertexAttrib dest = CS_VATTRIB_INVALID;
  if (program)
  {
    CGparameter parameter = cgGetNamedParameter (program, binding);
    if (parameter)
    {
      CGresource base = cgGetParameterBaseResource (parameter);
      int index = cgGetParameterResourceIndex (parameter);
      switch (base)
      {
        case CG_UNDEFINED:
          return CS_VATTRIB_UNUSED;
	case CG_TEX0: 
	case CG_TEXUNIT0:
	case CG_TEXCOORD0:
	  if ((index >= 0) && (index < 8))
            dest = (csVertexAttrib)(CS_VATTRIB_TEXCOORD0 + index);
	  break;
	case CG_ATTR0:
	  if ((index >= 0) && (index < 16))
            dest = (csVertexAttrib)(CS_VATTRIB_0 + index);
	  break;
	case CG_COL0:
	case CG_COLOR0:
	  if ((index >= 0) && (index < 2))
            dest = (csVertexAttrib)(CS_VATTRIB_PRIMARY_COLOR + index);
	  break;
	case CG_HPOS:
	case CG_POSITION0:
	  dest = CS_VATTRIB_POSITION;
	  break;
	case CG_BLENDWEIGHT0:
	  dest = CS_VATTRIB_WEIGHT;
	  break;
	case CG_NORMAL0:
	  dest = CS_VATTRIB_NORMAL;
	  break;
	case CG_FOG0:
	  dest = CS_VATTRIB_FOGCOORD;
	  break;
        default:
	  break;
      }
    }
  }

  return dest;
}

}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)
