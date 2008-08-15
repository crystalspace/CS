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

  size_t i;

  // See if we want to wrap through the PS plugin
  // (psplg will be 0 if wrapping isn't wanted)
  if (shaderPlug->psplg)
  {
  csRef<iDataBuffer> programBuffer = GetProgramData();
    if (!programBuffer.IsValid())
      return false;
    csString programStr;
    programStr.Append ((char*)programBuffer->GetData(), programBuffer->GetSize());
  
    ArgumentArray args;
    shaderPlug->GetProfileCompilerArgs ("fragment", shaderPlug->psProfile, 
      CS::PluginCommon::ShaderProgramPluginGL::Other, false, args);
    for (i = 0; i < compilerArgs.GetSize(); i++) 
      args.Push (compilerArgs[i]);
    args.Push (0);
    program = cgCreateProgram (shaderPlug->context, CG_SOURCE,
      programStr, shaderPlug->psProfile, 
      !entrypoint.IsEmpty() ? entrypoint : "main", args.GetArray());

    if (!program)
      return false;

    pswrap = shaderPlug->psplg->CreateProgram ("fp");

    if (!pswrap)
      return false;

    csArray<csShaderVarMapping> mappings;
    
    for (i = 0; i < variablemap.GetSize (); i++)
    {
      // Get the Cg parameter
      CGparameter parameter = cgGetNamedParameter (
	program, variablemap[i].destination);
      // Check if it's found, and just skip it if not.
      if (!parameter)
        continue;
      if (!cgIsParameterReferenced (parameter))
	continue;
      // Make sure it's a C-register
      CGresource resource = cgGetParameterResource (parameter);
      if (resource == CG_C)
      {
        // Get the register number, and create a mapping
        csString regnum;
        regnum.Format ("c%lu", cgGetParameterResourceIndex (parameter));
        mappings.Push (csShaderVarMapping (variablemap[i].name, regnum));
      }
    }

    csRef<iHierarchicalCache> ps1cache;
    if (pswrap->Load (0, cgGetProgramString (program, CG_COMPILED_PROGRAM), 
      mappings))
    {
      if (cache != 0) ps1cache = cache->GetRootedCache ("/ps1/");
      bool ret = pswrap->Compile (ps1cache);
      if (shaderPlug->debugDump)
        DoDebugDump();
      return ret;
    }
    else
    {
      return false;
    }
  }
  else
  {
    bool ret = TryCompile (shaderPlug->maxProfileFragment,
      loadLoadToGL | loadApplyVmap, 0);
  
    /* Inaccurate when VP has custom profile set */
    CGprofile vpProf = cgGLGetLatestProfile (CG_GL_VERTEX);
    if (vpProf < CG_PROFILE_ARBVP1)
      vpProf = CG_PROFILE_ARBVP1;
    if ((shaderPlug->maxProfileVertex != CG_PROFILE_UNKNOWN)
        && (vpProf > shaderPlug->maxProfileVertex))
      vpProf = shaderPlug->maxProfileVertex;
  
    ProfileLimitsPair limits;
    limits.fp = ProfileLimits (shaderPlug->vendor, programProfile);
    limits.vp = ProfileLimits (shaderPlug->vendor, vpProf);
    limits.GetCurrentLimits (shaderPlug->ext);
    cacheLimits = limits;
    WriteToCache (cache, limits.fp, limits.ToString());
    cacheKeepNodes.DeleteAll ();
    tag->AttachNew (new scfString (limits.ToString()));
    return ret;
  }

  return true;
}

bool csShaderGLCGFP::Precache (const ProfileLimits& limits, 
                               const char* tag,
                               iHierarchicalCache* cache)
{
  PrecacheClear();

  bool needBuild = true;
  csString sourcePreproc;
  {
    csRef<iDataBuffer> programBuffer = GetProgramData();
    if (!programBuffer.IsValid())
      return false;
    csString programStr;
    programStr.Append ((char*)programBuffer->GetData(), programBuffer->GetSize());
    
    ArgumentArray args;
    shaderPlug->GetProfileCompilerArgs (GetProgramType(),
      limits.profile, limits.vendor, true, args);
    for (size_t i = 0; i < compilerArgs.GetSize(); i++) 
      args.Push (compilerArgs[i]);
  
    // Get preprocessed result of pristine source
    sourcePreproc = GetPreprocessedProgram (programStr, args);
    if (!sourcePreproc.IsEmpty ())
    {
      // Check preprocessed source against cache
      if (TryLoadFromCompileCache (sourcePreproc, limits, cache))
        needBuild = false;
    }
  }
  
  bool ret;
  if (needBuild)
    ret = TryCompile (CG_PROFILE_UNKNOWN,
      loadApplyVmap | loadIgnoreConfigProgramOpts, &limits);
  else
    ret = true;

  // Store program against preprocessed source in cache
  {
    if (needBuild && !sourcePreproc.IsEmpty ())
      WriteToCompileCache (sourcePreproc, limits, cache);
  }

  WriteToCache (cache, limits, tag);
  
  return ret;
}

bool csShaderGLCGFP::TryCompile (CGprofile maxFrag, uint loadFlags,
                                 const ProfileLimits* limits)
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
  
    /* A list of unused variables to test for has been given. Test piecemeal
     * which variables are really unused */
    csSet<csString> allNewUnusedParams;
    const size_t maxSteps = 8;
    size_t step = maxSteps;
    size_t offset = 0;
    while (offset < testForUnused.GetSize())
    {
      unusedParams.DeleteAll();
      for (size_t i = 0; i < offset; i++)
	unusedParams.Add (testForUnused[i]);
      for (size_t i = offset+step; i < testForUnused.GetSize(); i++)
	unusedParams.Add (testForUnused[i]);
      bool compileSucceeded = DefaultLoadProgram (0, programStr, CG_GL_FRAGMENT, 
	maxFrag, 
	/*loadIgnoreErrors | */(loadFlags & loadIgnoreConfigProgramOpts),
	limits);
	
      if (compileSucceeded)
      {
        // Subset compiled fine, extract unused params
	csSet<csString> newUnusedParams;
	CollectUnusedParameters (newUnusedParams);
	for (size_t i = 0; i < step; i++)
	{
	  if (offset+i >= testForUnused.GetSize()) break;
	  const char* s = testForUnused[offset+i];
	  if (newUnusedParams.Contains (s))
	  {
	    allNewUnusedParams.Add (s);
	  }
	}
        offset += step;
        step = maxSteps;
      }
      else if (step > 1)
      {
        /* We might trip over a large array for whose elements can not be
           allocated. Try will less parameters. */
        step = step/2;
      }
      else
      {
        // Test for that single variable failed ... pretend it's unused
        allNewUnusedParams.Add (testForUnused[offset]);
        offset += step;
        step = maxSteps;
      }
    }
    unusedParams = allNewUnusedParams;
  }
  else
  {
    unusedParams.DeleteAll();
  }
  if (!DefaultLoadProgram (0, programStr, CG_GL_FRAGMENT, 
    maxFrag, (loadFlags & (~loadApplyVmap)) | loadFlagUnusedV2FForInit, limits))
    return false;
  /* Compile twice to be able to filter out unused vertex2fragment stuff on 
    * pass 2.
    * @@@ FIXME: two passes are not always needed.
    */
  CollectUnusedParameters (unusedParams);
  bool ret = DefaultLoadProgram (this, programStr, CG_GL_FRAGMENT, 
    maxFrag, loadFlags | loadFlagUnusedV2FForInit, limits);
    
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
