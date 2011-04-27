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
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/scfstr.h"
#include "csutil/stringreader.h"
#include "iutil/databuff.h"
#include "iutil/document.h"
#include "iutil/hiercache.h"
#include "iutil/string.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"

#include "glshader_cgfp.h"
#include "profile_limits.h"

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{

CS_LEAKGUARD_IMPLEMENT (csShaderGLCGFP);

void csShaderGLCGFP::Activate()
{
  if (pswrap)
  {
    pswrap->Activate ();
    return;
  } 

  csShaderGLCGCommon::Activate();
}

void csShaderGLCGFP::Deactivate()
{
  if (pswrap)
  {
    pswrap->Deactivate ();
    return;
  } 

  csShaderGLCGCommon::Deactivate();
}

void csShaderGLCGFP::SetupState (const CS::Graphics::RenderMesh* mesh,
                                 CS::Graphics::RenderMeshModes& modes,
                                 const csShaderVariableStack& stack)
{
  if (pswrap)
  {
    pswrap->SetupState (mesh, modes, stack);
    return;
  } 

  csShaderGLCGCommon::SetupState (mesh, modes, stack);
}

void csShaderGLCGFP::ResetState()
{
  if (pswrap)
  {
    pswrap->ResetState ();
    return;
  } 
  csShaderGLCGCommon::ResetState();
}

bool csShaderGLCGFP::Compile (iHierarchicalCache* cache, csRef<iString>* tag)
{
  if (!shaderPlug->enableFP) return false;

  // See if we want to wrap through the PS plugin
  if (shaderPlug->ProfileNeedsRouting (shaderPlug->currentLimits.fp.profile)
      && shaderPlug->psplg)
  {
    bool ret = TryCompile (loadApplyVmap,
      shaderPlug->currentLimits);
  
    csString tagStr (csString("CG") + shaderPlug->currentLimits.ToString());
    WriteToCache (cache, shaderPlug->currentLimits.fp, 
      shaderPlug->currentLimits, tagStr);
    cacheKeepNodes.DeleteAll ();
    tag->AttachNew (new scfString (tagStr));
    
    if (!ret) return false;

    return LoadProgramWithPS1 ();
  }
  else
  {
    bool ret = TryCompile (loadLoadToGL | loadApplyVmap,
      shaderPlug->currentLimits);
  
    csString tagStr (csString("CG") + shaderPlug->currentLimits.ToString());
    WriteToCache (cache, shaderPlug->currentLimits.fp, 
      shaderPlug->currentLimits, tagStr);
    cacheKeepNodes.DeleteAll ();
    tag->AttachNew (new scfString (tagStr));
    return ret;
  }
}

bool csShaderGLCGFP::Precache (const ProfileLimitsPair& limits,
                               const char* tag,
                               iHierarchicalCache* cache)
{
  PrecacheClear();

  ProgramObject programObj;
  bool needBuild = true;
  csString sourcePreproc;
  {
    csRef<iDataBuffer> programBuffer = GetProgramData();
    if (!programBuffer.IsValid())
      return false;
    csString programStr;
    programStr.Append ((char*)programBuffer->GetData(), programBuffer->GetSize());
    
    // Get preprocessed result of pristine source
    sourcePreproc = GetAugmentedProgram (programStr);
    if (!sourcePreproc.IsEmpty ())
    {
      // Check preprocessed source against cache
      //if (TryLoadFromCompileCache (sourcePreproc, limits.fp, cache))
      if (shaderPlug->progCache.SearchObject (sourcePreproc, limits.fp, programObj))
        needBuild = false;
    }
  }
  
  bool ret;
  if (needBuild)
  {
    ret = TryCompile (loadApplyVmap, limits);
    WriteToCache (cache, limits.fp, limits, csString("CG") + tag);
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

iShaderProgram::CacheLoadResult csShaderGLCGFP::LoadFromCache (
  iHierarchicalCache* cache, iBase* previous, iDocumentNode* programNode,
  csRef<iString>* failReason, csRef<iString>* tag)
{
  if (!shaderPlug->enableFP)
  {
    if (failReason)
      failReason->AttachNew (new scfString ("Cg FP not available or disabled"));
    /* Claim a load success, but invalid shader, to prevent loading from
	scratch (which will fail anyway) */
    return loadSuccessShaderInvalid;
  }
  return csShaderGLCGCommon::LoadFromCache (cache, previous, programNode,
    failReason, tag, &cacheLimits);
}

bool csShaderGLCGFP::TryCompile (uint loadFlags,
                                 const ProfileLimitsPair& limits)
{
  csRef<iDataBuffer> programBuffer = GetProgramData();
  if (!programBuffer.IsValid())
    return false;
  csString programStr;
  programStr.Append ((char*)programBuffer->GetData(), programBuffer->GetSize());

  csStringArray testForUnused;
  csStringReader lines (programStr);
  while (lines.HasMoreLines())
  {
    csString line;
    lines.GetLine (line);
    if (line.StartsWith ("//@@UNUSED? "))
    {
      line.DeleteAt (0, 12);
      testForUnused.Push (line);
    }
  }

  if (testForUnused.GetSize() > 0)
  {
    testForUnused.Push ("PARAM__clip_out_packed_distances1_UNUSED");
    testForUnused.Push ("PARAM__clip_out_packed_distances2_UNUSED");
  }
  unusedParams.DeleteAll();
  if (!DefaultLoadProgram (0, programStr, progFP, 
      limits, (loadFlags & (~loadApplyVmap)) | loadFlagUnusedV2FForInit))
    return false;
  /* Compile twice to be able to filter out unused vertex2fragment stuff on 
    * pass 2.
    * @@@ FIXME: two passes are not always needed.
    */
  CollectUnusedParameters (unusedParams, testForUnused);
  bool ret = DefaultLoadProgram (this, programStr, progFP, 
    limits, loadFlags | loadFlagUnusedV2FForInit);
    
  return ret;
}

int csShaderGLCGFP::ResolveTU (const char* binding)
{
  int newTU = -1;
  if (program)
  {
    CGparameter parameter = cgGetNamedParameter (program, binding);
    if (parameter)
    {
      CGresource baseRes = cgGetParameterBaseResource (parameter);
      if (((baseRes == CG_TEXUNIT0) || (baseRes == CG_TEX0))
          && (cgIsParameterUsed (parameter, program)
            || cgIsParameterReferenced (parameter)))
      {
	newTU = cgGetParameterResourceIndex (parameter);
      }
    }
  }

  return newTU;
}

}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)
