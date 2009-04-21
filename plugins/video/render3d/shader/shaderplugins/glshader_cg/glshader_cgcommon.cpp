/*
  Copyright (C) 2002-2005 by Marten Svanfeldt
			     Anders Stenberg
			     Frank Richter

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

#include "csplugincommon/opengl/glextmanager.h"
#include "csplugincommon/opengl/glstates.h"
#include "csplugincommon/shader/shadercachehelper.h"
#include "csutil/csendian.h"
#include "csutil/documenthelper.h"
#include "csutil/memfile.h"
#include "csutil/scfstr.h"
#include "csutil/scfstringarray.h"
#include "csutil/stringreader.h"
#include "iutil/hiercache.h"
#include "iutil/stringarray.h"
#include "ivaria/reporter.h"

#include "glshader_cg.h"
#include "glshader_cgfp.h"
#include "glshader_cgcommon.h"
#include "profile_limits.h"
#include "stringstore.h"

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{

CS_LEAKGUARD_IMPLEMENT (csShaderGLCGCommon);

csShaderGLCGCommon::csShaderGLCGCommon (csGLShader_CG* shaderPlug, 
					ProgramType type) :
  scfImplementationType (this, shaderPlug->object_reg), programType (type)
{
  this->shaderPlug = shaderPlug;
  program = 0;

  InitTokenTable (xmltokens);
}

csShaderGLCGCommon::~csShaderGLCGCommon ()
{
  if (program)
    cgDestroyProgram (program);
    
  for(size_t i = 0; i < variablemap.GetSize (); ++i)
  {
    VariableMapEntry& mapping = variablemap[i];
    
    ShaderParameter* param =
      reinterpret_cast<ShaderParameter*> (mapping.userVal);
    
    FreeShaderParam (param);
  }
}

void csShaderGLCGCommon::Activate()
{
  cgGLEnableProfile (programProfile);
  #if 0
  if (!cgGLIsProgramLoaded (program))
  {
    cgGLLoadProgram (program);
    PostCompileVmapProcess ();
  }
  #endif
  cgGLBindProgram (program);
  
  if (shaderPlug->ext->CS_GL_ARB_color_buffer_float)
  {
    shaderPlug->statecache->SetClampColor (GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE);
  }
}

void csShaderGLCGCommon::Deactivate()
{
  cgGLDisableProfile (programProfile);

  if (shaderPlug->ext->CS_GL_ARB_color_buffer_float)
  {
    shaderPlug->statecache->SetClampColor (GL_CLAMP_VERTEX_COLOR_ARB, GL_TRUE);
  }
}

void csShaderGLCGCommon::SetupState (const CS::Graphics::RenderMesh* /*mesh*/,
                                     CS::Graphics::RenderMeshModes& /*modes*/,
                                     const csShaderVariableStack& stack)
{
  if ((clips.GetSize() > 0) && (programType == progVP))
  {
    uint clipFlags = 0;
    shaderPlug->clipPlanes.SetShaderVars (stack);
    
    csVector4 packDist[2];
    clipPackedDists[0]->GetValue (packDist[0]);
    clipPackedDists[1]->GetValue (packDist[1]);
    for (size_t c = 0; c < clips.GetSize(); c++)
    {
      const Clip& clip = clips[c];
	
      bool hasClipDist = false;
      if (!clip.distance.IsConstant())
      {
	csVector4 v;
	if (GetParamVectorVal (stack, clip.distance, &v))
	{
	  float distVal = v[clip.distComp];
	  if (clip.distNeg) distVal = -distVal;
	  packDist[c/4][c%4] = distVal;
	  hasClipDist = true;
	}
	else
	{
	  // Force clipping to have no effect
	  packDist[c/4][c%4] = -FLT_MAX;
	}
      }
      
      bool doClipping = false;
      /* Enable a clip plane if:
         - both plane and distance are constant
         - one is not constant and the SV value is present
         - both are not constant and the SV values are present
       */
      csVector4 v;
      bool constPlane = clip.plane.IsConstant();
      bool hasPlaneSV = !constPlane
        && GetParamVectorVal (stack, clip.plane, &v);
      bool constDist = clip.distance.IsConstant();
      bool hasDistSV = !constDist && hasClipDist;
      doClipping = (constPlane && constDist)
        || (constPlane && hasDistSV)
        || (hasPlaneSV && constDist)
        || (hasPlaneSV && hasDistSV);
      if (doClipping)
      {
        clipFlags |= 1 << c;
        if (!constPlane)
        {
          csVector4 v = GetParamVectorVal (stack, clip.plane, csVector4 (0));
          clipPlane[c]->SetValue (v);
        }
      }
      else
      {
        if (!constPlane)
        {
          // Force clip plane to not clip
          csVector4 v (0);
          clipPlane[c]->SetValue (v);
        }
      }
    }
    clipPackedDists[0]->SetValue (packDist[0]);
    clipPackedDists[1]->SetValue (packDist[1]);
  
    if ((programProfile == CG_PROFILE_VP40)
        || (programProfile == CG_PROFILE_GPU_VP))
    {
      for (size_t c = 0; c < clips.GetSize(); c++)
      {
	if (clipFlags & (1 << c))
          shaderPlug->clipPlanes.EnableClipPlane ((uint)c);
      }
    }
    else if ((programProfile == CG_PROFILE_ARBVP1)
      && (shaderPlug->vendor ==
        CS::PluginCommon::ShaderProgramPluginGL::ATI))
    {
      for (size_t c = 0; c < clips.GetSize(); c++)
      {
	const Clip& clip = clips[c];
	
	if (clipFlags & (1 << c))
	{
	  csVector4 v (
	   GetParamVectorVal (stack, clip.plane, csVector4 (0, 0, 0, 0)));
	  csPlane3 plane (v.x, v.y, v.z, v.w);
	  float distVal = packDist[c/4][c%4];
	  plane.DD -= distVal / plane.Normal().Norm();
	  shaderPlug->clipPlanes.AddClipPlane (v, clip.space);
	}
      }
    }
  }
  
  ApplyVariableMapArrays (stack);
}

void csShaderGLCGCommon::ResetState()
{
  if ((clips.GetSize() > 0) && (programType == progVP))
  {
    shaderPlug->clipPlanes.DisableClipPlanes();
  }
}
  
void csShaderGLCGCommon::EnsureDumpFile()
{
  if (debugFN.IsEmpty())
  {
    static int programCounter = 0;
    
    const char* progTypeStr ="";
    switch (programType)
    {
      case progVP: progTypeStr = "cgvp"; break;
      case progFP: progTypeStr = "cgfp"; break;
    }
    
    csRef<iVFS> vfs = csQueryRegistry<iVFS> (objectReg);
    csString filename;
    filename << shaderPlug->dumpDir << (programCounter++) << 
      progTypeStr << ".txt";
    debugFN = filename;
    vfs->DeleteFile (debugFN);
  }
}

void csShaderGLCGCommon::PrecacheClear()
{
  if (program != 0)
  {
    cgDestroyProgram (program);
    program = 0;
  }
  objectCode.Empty();
  objectCodeCachePathArc.Empty();
  objectCodeCachePathItem.Empty();
}

bool csShaderGLCGCommon::DefaultLoadProgram (iShaderProgramCG* cgResolve,
  const char* programStr, ProgramType _type,
  const ProfileLimitsPair& customLimitsPair, uint flags)
{
  if (!programStr || !*programStr) return false;

  const ProfileLimits& customLimits = (_type == progVP) ?
    customLimitsPair.vp : customLimitsPair.fp;
  CGGLenum type = (_type == progVP) ? CG_GL_VERTEX : CG_GL_FRAGMENT;

  size_t i;
  csString augmentedProgramStr;
  const csSet<csString>* unusedParams = &this->unusedParams;
  if (cgResolve != 0)
  {
    unusedParams = &cgResolve->GetUnusedParameters ();
  }
  csSet<csString>::GlobalIterator unusedIt (unusedParams->GetIterator());
  while (unusedIt.HasNext())
  {
    const csString& param = unusedIt.Next ();
    augmentedProgramStr.AppendFmt ("#define %s\n",
      param.GetData());
  }
  
  if (flags & loadFlagUnusedV2FForInit)
    augmentedProgramStr.Append ("#define _INITIALIZE_UNUSED_V2F\n");
  
  OutputClipPreamble (augmentedProgramStr);
  WriteClipApplications (augmentedProgramStr);
  
  augmentedProgramStr.Append (programStr);
  programStr = augmentedProgramStr;
  CGprofile profile = customLimits.profile;
  CS::PluginCommon::ShaderProgramPluginGL::HardwareVendor vendor =
    customLimits.vendor;

  if (shaderPlug->doVerbose || shaderPlug->doVerbosePrecache)
  {
    shaderPlug->Report (CS_REPORTER_SEVERITY_NOTIFY,
      "Cg %s program '%s': using profile %s[%d]", GetProgramType(),
      description.GetData (), cgGetProfileString (profile), profile);
  }

  ArgumentArray args;
  shaderPlug->GetProfileCompilerArgs (GetProgramType(), profile, 
    customLimitsPair,
    vendor,
    (flags & loadIgnoreConfigProgramOpts) ? csGLShader_CG::argsNoConfig
      : csGLShader_CG::argsAll, args);
  for (i = 0; i < compilerArgs.GetSize(); i++) 
    args.Push (compilerArgs[i]);
  /* Work around Cg 2.0 bug: it emits "OPTION ARB_position_invariant;"
     AND computes result.position in the VP - doing both is verboten.
     Remedy: remove -posinv argument 
     (cgc version 2.0.0010)
   */
  if (profile == CG_PROFILE_GPU_VP)
  {
    for (i = 0; i < args.GetSize(); ) 
    {
      if (strcmp (args[i], "-posinv") == 0)
	args.DeleteIndex (i);
      else
	i++;
    }
  }
  customLimits.ToCgOptions (args);
  args.Push (0);
 
  if (program)
  {
    cgDestroyProgram (program);
  }
  shaderPlug->SetCompiledSource (programStr);
  program = cgCreateProgram (shaderPlug->context, 
    CG_SOURCE, programStr, 
    profile, !entrypoint.IsEmpty() ? entrypoint : "main", args.GetArray());
  
  if (!(flags & loadIgnoreErrors)) shaderPlug->PrintAnyListing();

  if (!program)
  {
    shaderPlug->SetCompiledSource (0);
    /*if (shaderPlug->debugDump)
    {
      EnsureDumpFile();
      WriteAdditionalDumpInfo ("Failed program source", programStr);
    }*/
    return false;
  }
  programProfile = cgGetProgramProfile (program);

  if (flags & loadApplyVmap)
    GetParamsFromVmap();

  if (flags & loadIgnoreErrors) shaderPlug->SetIgnoreErrors (true);
  cgCompileProgram (program);
  if (flags & loadIgnoreErrors)
    shaderPlug->SetIgnoreErrors (false);
  else
    shaderPlug->PrintAnyListing();
  
  if (flags & loadApplyVmap)
    GetPostCompileParamProps ();

  if (flags & loadLoadToGL)
  {
    cgGetError(); // Clear error
    cgGLLoadProgram (program);
    if (!(flags & loadIgnoreErrors)) shaderPlug->PrintAnyListing();
    if ((cgGetError() != CG_NO_ERROR)
      || !cgGLIsProgramLoaded (program)) 
    {
      if (shaderPlug->debugDump)
	DoDebugDump();

      if (shaderPlug->doVerbose
	  && ((type == CG_GL_VERTEX) && (profile >= CG_PROFILE_ARBVP1))
	    || ((type == CG_GL_FRAGMENT) && (profile >= CG_PROFILE_ARBFP1)))
      {
	csString err = (char*)glGetString (GL_PROGRAM_ERROR_STRING_ARB);
	if (!err.IsEmpty())
	  shaderPlug->Report (CS_REPORTER_SEVERITY_WARNING,
	    "OpenGL error string: %s", err.GetData());
      }

      shaderPlug->SetCompiledSource (0);
      return false;
    }
  }

  if (shaderPlug->debugDump)
    DoDebugDump();
  
  shaderPlug->SetCompiledSource (0);

  bool result = true;
  if (programType == progFP)
  {
    int numVaryings = 0;
    CGparameter param = cgGetFirstLeafParameter (program, CG_PROGRAM);
    while (param)
    {
      if (cgIsParameterUsed (param, program)
	&& cgIsParameterReferenced (param))
      {
	const CGenum var = cgGetParameterVariability (param);
	if (var == CG_VARYING)
	  numVaryings++;
      }
  
      param = cgGetNextLeafParameter (param);
    }
    
    /* WORKAROUND: Even NVs G80 doesn't support passing more than 16 attribs
       into an FP, yet Cg happily generates code that uses more (and GL falls 
       back to SW).
       So manually check the number of varying inputs and reject more than 16.
       
       @@@ This should be at least configurable
     */
    const int maxNumVaryings = 16;
    if (numVaryings > maxNumVaryings)
    {
      if (shaderPlug->doVerbose || shaderPlug->doVerbosePrecache)
      {
	shaderPlug->Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "Discarding compiled program for having too much varyings "
	  "(%d, limit is %d)",
	  numVaryings, maxNumVaryings);
      }
      cgDestroyProgram (program);
      program = 0;
      result = false;
    }
  }
  if (!result && !debugFN.IsEmpty())
  {
    csRef<iVFS> vfs = csQueryRegistry<iVFS> (objectReg);
    vfs->DeleteFile (debugFN);
  }
  return result;
}

csString csShaderGLCGCommon::GetPreprocessedProgram (const char* programStr)
{
  csString augmentedProgramStr;
  const csSet<csString>* unusedParams = &this->unusedParams;
  if (cgResolve != 0)
  {
    unusedParams = &cgResolve->GetUnusedParameters ();
  }
  csSet<csString>::GlobalIterator unusedIt (unusedParams->GetIterator());
  while (unusedIt.HasNext())
  {
    const csString& param = unusedIt.Next ();
    augmentedProgramStr.AppendFmt ("#define %s\n",
      param.GetData());
  }
  
  OutputClipPreamble (augmentedProgramStr);
  WriteClipApplications (augmentedProgramStr);
  
  augmentedProgramStr.Append (programStr);
  
  return augmentedProgramStr;
}

void csShaderGLCGCommon::DoDebugDump ()
{
  csString output;
  DumpProgramInfo (output);
  output << "CG program type: " << programType << "\n";
  output << "CG profile: " << cgGetProgramString (program, 
    CG_PROGRAM_PROFILE) << "\n";
  output << "CG entry point: " << (entrypoint ? entrypoint : "main") << 
    "\n";
  output << "CG program valid: " << IsValid() << "\n";
  output << "\n";

  output << "Variable mappings:\n";
  for (size_t v = 0; v < variablemap.GetSize (); v++)
  {
    const VariableMapEntry& vme = variablemap[v];
    ShaderParameter* sparam =
      reinterpret_cast<ShaderParameter*> (vme.userVal);

    output << stringsSvName->Request (vme.name);
    output << '(' << vme.name << ") -> ";
    output << vme.destination << ' ';
    if (sparam == 0)
    {
      output << "(null)";
    }
    else
    {
      if (sparam->paramType != 0) output << cgGetTypeString (sparam->paramType) << ' ';
      if (sparam->param != 0) output << cgGetParameterName (sparam->param) << "  ";
      output << "baseslot " << sparam->baseSlot;
      if (sparam->assumeConstant) output << "  assumed constant";
    }
    output << '\n'; 
  }
  output << "\n";

  CGparameter param = cgGetFirstLeafParameter (program, CG_PROGRAM);
  while (param)
  {
    output << "Parameter: " << cgGetParameterName (param) << "\n";
    output << " Type: " << 
      cgGetTypeString (cgGetParameterNamedType (param)) << "\n";
    output << " Direction: " <<
      cgGetEnumString (cgGetParameterDirection (param)) << "\n";
    output << " Semantic: " << cgGetParameterSemantic (param) << "\n";
    const CGenum var = cgGetParameterVariability (param);
    output << " Variability: " << cgGetEnumString (var) << "\n";
    output << " Resource: " <<
      cgGetResourceString (cgGetParameterResource (param)) << "\n";
    output << " Resource index: " <<
      cgGetParameterResourceIndex (param) << "\n";
    // Cg 2.0 seems to not like CG_DEFAULT for uniforms
    if (/*(var == CG_UNIFORM) || */(var == CG_CONSTANT))
    {
      int nValues;
      const double* values = cgGetParameterValues (param, 
	(var == CG_UNIFORM) ? CG_DEFAULT : CG_CONSTANT, &nValues);
      if (nValues != 0)
      {
	output << " Values:";
	for (int v = 0; v < nValues; v++)
	{
	  output << ' ' << values[v];
	}
	output << "\n";
      }
    }
    if (!cgIsParameterUsed (param, program)) output << "  not used\n";
    if (!cgIsParameterReferenced (param)) output << "  not referenced\n";

    param = cgGetNextLeafParameter (param);
  }
  output << "\n";

  output << "Program source:\n";
  output << cgGetProgramString (program, CG_PROGRAM_SOURCE);
  output << "\n";

  output << "Compiled program:\n";
  output << cgGetProgramString (program, CG_COMPILED_PROGRAM);
  output << "\n";

  csRef<iVFS> vfs = csQueryRegistry<iVFS> (objectReg);
  EnsureDumpFile();

  csRef<iFile> debugFile = vfs->Open (debugFN, VFS_FILE_APPEND);
  if (!debugFile)
  {
    csReport (objectReg, CS_REPORTER_SEVERITY_WARNING, 
      "crystalspace.graphics3d.shader.glcg",
      "Could not write '%s'", debugFN.GetData());
  }
  else
  {
    debugFile->Write (output.GetData(), output.Length ());
    csReport (objectReg, CS_REPORTER_SEVERITY_NOTIFY, 
      "crystalspace.graphics3d.shader.glcg",
      "Dumped Cg program info to '%s'", debugFN.GetData());
  }
}

void csShaderGLCGCommon::WriteAdditionalDumpInfo (const char* description, 
						  const char* content)
{
  if (!shaderPlug->debugDump || !debugFN) return;

  csRef<iVFS> vfs = csQueryRegistry<iVFS> (objectReg);
  csRef<iDataBuffer> oldDump = vfs->ReadFile (debugFN, true);

  csString output (oldDump ? (char*)oldDump->GetData() : 0);
  output << description << ":\n";
  output << content;
  output << "\n";
  if (!vfs->WriteFile (debugFN, output.GetData(), output.Length ()))
  {
    csReport (objectReg, CS_REPORTER_SEVERITY_WARNING, 
      "crystalspace.graphics3d.shader.glcg",
      "Could not augment '%s'", debugFN.GetData());
  }
}

/* Magic value for cg program files.
 * The most significant byte serves as a "version", increase when the
 * cache file format changes. */
static const uint32 cacheFileMagic = 0x06706763;

enum
{
  cpsValid = 0x6b726f77,
  cpsInvalid = 0x6b723062
};

bool csShaderGLCGCommon::WriteToCacheWorker (iHierarchicalCache* cache,
  const ProfileLimits& limits, const ProfileLimitsPair& limitsPair, 
  const char* tag, csString& failReason)
{
  if (!cache) return false;

  csMemFile cacheFile;
  
  uint32 diskMagic = csLittleEndian::UInt32 (cacheFileMagic);
  if (cacheFile.Write ((char*)&diskMagic, sizeof (diskMagic))
      != sizeof (diskMagic))
  {
    failReason = "write error (magic)";
    return false;
  }
  
  csRef<iDataBuffer> programBuffer = GetProgramData();
  csMD5::Digest progHash = csMD5::Encode (
    programBuffer->GetData(), programBuffer->GetSize());
  if (cacheFile.Write ((char*)&progHash, sizeof (progHash))
      != sizeof (progHash))
  {
    failReason = "write error (hash)";
    return false;
  }
  
  csString objectCode (this->objectCode);
  if ((program != 0) && objectCode.IsEmpty())
    objectCode = cgGetProgramString (program, CG_COMPILED_PROGRAM);
  /*const char* object;
  if ((program == 0)
    || ((object = cgGetProgramString (program, CG_COMPILED_PROGRAM)) == 0)
    || (*object == 0))*/
  if ((program == 0) || objectCode.IsEmpty())
  {
    uint32 diskState = csLittleEndian::UInt32 (cpsInvalid);
    if (cacheFile.Write ((char*)&diskState, sizeof (diskState))
	!= sizeof (diskState))
    {
      failReason = "write error (state-invalid)";
      return false;
    }
  }
  else
  {
    {
      uint32 diskState = csLittleEndian::UInt32 (cpsValid);
      if (cacheFile.Write ((char*)&diskState, sizeof (diskState))
	  != sizeof (diskState))
      {
	failReason = "write error (state-valid)";
	return false;
      }
    }
    
    CS::PluginCommon::ShaderCacheHelper::WriteString (&cacheFile, description);
    
    if (objectCodeCachePathArc.IsEmpty() || objectCodeCachePathItem.IsEmpty())
    {
      csString preprocSource (GetPreprocessedProgram (
        cgGetProgramString (program, CG_PROGRAM_SOURCE)));
      csString failReason2;
      if (!WriteToCompileCache (preprocSource, limits, cache, failReason2))
      {
	failReason.Format ("failed compile cache writing: %s",
	  failReason2.GetData());
	return false;
      }
    }
    
    CS_ASSERT(!objectCodeCachePathArc.IsEmpty());
    CS::PluginCommon::ShaderCacheHelper::WriteString (&cacheFile,
      objectCodeCachePathArc);
    CS_ASSERT(!objectCodeCachePathItem.IsEmpty());
    CS::PluginCommon::ShaderCacheHelper::WriteString (&cacheFile,
      objectCodeCachePathItem);
  }
  
  csString cacheName ("/");
  cacheName += tag;
  if (!cache->CacheData (cacheFile.GetData(), cacheFile.GetSize(),
    cacheName))
  {
    failReason = "failed writing to cache";
    return false;
  }
  return true;
}

bool csShaderGLCGCommon::WriteToCache (iHierarchicalCache* cache,
                                       const ProfileLimits& limits,
                                       const ProfileLimitsPair& limitsPair, 
                                       const char* tag)
{
  csString failReason;
  if (!WriteToCacheWorker (cache, limits, limitsPair, tag, failReason))
  {
    if (shaderPlug->doVerbose)
    {
      csReport (objectReg, CS_REPORTER_SEVERITY_WARNING, 
	"crystalspace.graphics3d.shader.glcg",
	"Error writing %s program for %s to cache: %s",
	GetProgramType(), tag,
	failReason.GetData());
    }
    return false;
  }
  return true;
}

struct CachedShaderWrapper
{
  csString name;
  csRef<iFile> cacheFile;
  ProfileLimitsPair limits;
  
  bool operator< (const CachedShaderWrapper& other) const
  { return limits < other.limits; }
};

iShaderProgram::CacheLoadResult csShaderGLCGCommon::LoadFromCache (
  iHierarchicalCache* cache, iBase* previous, iDocumentNode* node,
  csRef<iString>* failReason, csRef<iString>* tag)
{
  if (!cache) return iShaderProgram::loadFail;

  csRef<iShaderProgramCG> prevCG (scfQueryInterfaceSafe<iShaderProgramCG> (
    previous));

  csRef<iStringArray> allCachedPrograms;
  if ((programType == progVP) && prevCG.IsValid())
  {
    csShaderGLCGFP* prevFP = static_cast<csShaderGLCGFP*> (
      (iShaderProgramCG*)prevCG);
    csString tagStr ("CG");
    tagStr += prevFP->cacheLimits.ToString();
    if (failReason) failReason->AttachNew (
      new scfString ("paired cached programs not found"));
    allCachedPrograms.AttachNew (new scfStringArray);
    allCachedPrograms->Push (tagStr);
  }
  else
    allCachedPrograms = cache->GetSubItems ("/");

  if (!allCachedPrograms.IsValid() || (allCachedPrograms->GetSize() == 0))
  {
    if (failReason) failReason->AttachNew (
      new scfString ("no cached programs found"));
    return iShaderProgram::loadFail;
  }
  
  if (!GetProgramNode (node)) return iShaderProgram::loadFail;
  csRef<iDataBuffer> programBuffer = GetProgramData();
  csMD5::Digest progHash = csMD5::Encode (
    programBuffer->GetData(), programBuffer->GetSize());
  
  csArray<CachedShaderWrapper> cachedProgWrappers;
  for (size_t i = 0; i < allCachedPrograms->GetSize(); i++)
  {
    const char* tag = allCachedPrograms->Get (i);
    if ((tag[0] != 'C') || (tag[1] != 'G')) continue;

    CachedShaderWrapper wrapper;
    if (!wrapper.limits.FromString (tag+2)) continue;
    wrapper.name = tag;

    csString cachePath ("/");
    cachePath.Append (tag);
    csRef<iDataBuffer> cacheBuf = cache->ReadCache (cachePath);
    if (!cacheBuf.IsValid()) continue;
    
    csRef<iFile> cacheFile;
    cacheFile.AttachNew (new csMemFile (cacheBuf, true));
    wrapper.cacheFile = cacheFile;
  
    uint32 diskMagic;
    if (cacheFile->Read ((char*)&diskMagic, sizeof (diskMagic))
	!= sizeof (diskMagic)) continue;
    if (csLittleEndian::UInt32 (diskMagic) != cacheFileMagic)
      continue;
      
    csMD5::Digest diskHash;
    if (cacheFile->Read ((char*)&diskHash, sizeof (diskHash))
	!= sizeof (diskHash)) continue;
    if (diskHash != progHash) continue;
    
    cachedProgWrappers.Push (wrapper);
  }
  
  if (cachedProgWrappers.GetSize() == 0)
  {
    if (failReason && !*failReason) failReason->AttachNew (
      new scfString ("all cached programs failed to read"));
    return iShaderProgram::loadFail;
  }
  
  cachedProgWrappers.Sort ();

  ProfileLimits currentLimits (
    (programType == progVP) ? shaderPlug->currentLimits.vp 
      : shaderPlug->currentLimits.fp);
  bool strictMatch = (programType == progVP) ? shaderPlug->strictMatchVP 
      : shaderPlug->strictMatchFP;
  const char* progTypeNode = 0;
  switch (programType)
  {
    case progVP: progTypeNode = "cgvp"; break;
    case progFP: progTypeNode = "cgfp"; break;
  }
  
  csString allReasons;
  bool oneReadCorrectly = false;
  for (size_t i = cachedProgWrappers.GetSize(); i-- > 0;)
  {
    const CachedShaderWrapper& wrapper = cachedProgWrappers[i];
    const ProfileLimits& limits =
      (programType == progVP) ? wrapper.limits.vp : wrapper.limits.fp;
      
    if (strictMatch && (limits != currentLimits))
    {
      allReasons += wrapper.name;
      allReasons += ": strict mismatch; ";
      continue;
    }
  
    bool profileSupported =
      (shaderPlug->ProfileNeedsRouting (limits.profile)
        && shaderPlug->IsRoutedProfileSupported (limits.profile))
      || cgGLIsProfileSupported (limits.profile);
    if (!profileSupported)
    {
      allReasons += wrapper.name;
      allReasons += ": Profile unsupported; ";
      continue;
    }
    
    if ((limits.vendor != currentLimits.vendor)
        && (limits.vendor != CS::PluginCommon::ShaderProgramPluginGL::Other))
    {
      allReasons += wrapper.name;
      allReasons += ": vendor mismatch; ";
      continue;
    }
    
    bool limitsSupported = currentLimits >= limits;
    if (!limitsSupported)
    {
      allReasons += wrapper.name;
      allReasons += ": Limits exceeded; ";
      continue;
    }
    
    iFile* cacheFile = wrapper.cacheFile;
    
    {
      uint32 diskState;
      if (cacheFile->Read ((char*)&diskState, sizeof (diskState))
	  != sizeof (diskState)) continue;
      if (csLittleEndian::UInt32 (diskState) != cpsValid)
      {
        oneReadCorrectly = true;
        continue;
      }
    }
  
    
    description = CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile);
    
    bool breakFail = false;
    csRef<iDocumentNode> cgNode = node->GetNode (progTypeNode);
    if (!cgNode.IsValid()) continue;
    csRef<iDocumentNodeIterator> nodes = cgNode->GetNodes();
    while(nodes->HasNext() && !breakFail)
    {
      csRef<iDocumentNode> child = nodes->Next();
      if(child->GetType() != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch(id)
      {
	case XMLTOKEN_VARIABLEMAP:
	  if (!ParseVmap (child))
	    breakFail = true;
	  break;
	case XMLTOKEN_CLIP:
	  if (!ParseClip (child))
	    breakFail = true;
	  break;
	default:
	  /* Ignore unknown nodes. Invalid nodes would have been caught
	     by the first (not from cache) parsing */
	  break;
      }
    }
    if (breakFail) continue;
    
    objectCodeCachePathArc =
      CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile);
    if (objectCodeCachePathArc.IsEmpty()) continue;
    objectCodeCachePathItem =
      CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile);
    if (objectCodeCachePathItem.IsEmpty()) continue;
    
    if (!LoadObjectCodeFromCompileCache (limits, cache))
      continue;
    
    oneReadCorrectly = true;
    if (program)
    {
      cgDestroyProgram (program);
      program = 0;
    }
    
    if (objectCode.IsEmpty()) continue;
    
    cgGetError(); // Clear error
    program = cgCreateProgram (shaderPlug->context, 
      CG_OBJECT, objectCode, limits.profile, 0, 0);
    if (!program) continue;
    CGerror err = cgGetError();
    if (err != CG_NO_ERROR)
    {
      const char* errStr = cgGetErrorString (err);
      shaderPlug->Report (CS_REPORTER_SEVERITY_WARNING,
	"Cg error %s", errStr);
      continue;
    }
    programProfile = limits.profile;
    
    ClipsToVmap();
    GetParamsFromVmap();
    
    cgGetError(); // Clear error
    cgGLLoadProgram (program);
    shaderPlug->PrintAnyListing();
    err = cgGetError();
    if ((err != CG_NO_ERROR)
      || !cgGLIsProgramLoaded (program)) 
    {
      //if (shaderPlug->debugDump)
	//DoDebugDump();
	
      const char* errStr = cgGetErrorString (err);
      shaderPlug->Report (CS_REPORTER_SEVERITY_WARNING,
	"Cg error %s", errStr);
  
      if (shaderPlug->doVerbose
	  && ((programType == progVP) && (programProfile >= CG_PROFILE_ARBVP1))
	    || ((programType == progFP) && (programProfile >= CG_PROFILE_ARBFP1)))
      {
	const char* err = (char*)glGetString (GL_PROGRAM_ERROR_STRING_ARB);
	shaderPlug->Report (CS_REPORTER_SEVERITY_WARNING,
	  "OpenGL error string: %s", err);
      }
  
      shaderPlug->SetCompiledSource (0);
      continue;
    }
    
    GetPostCompileParamProps ();
    
    if (shaderPlug->debugDump)
      DoDebugDump();
      
    tag->AttachNew (new scfString (wrapper.name));
    
    return iShaderProgram::loadSuccessShaderValid;
  }
  
  if (failReason) failReason->AttachNew (
    new scfString (allReasons));
  return oneReadCorrectly ? iShaderProgram::loadSuccessShaderInvalid : iShaderProgram::loadFail;
}

/* This define specifies whether the original source should be stored in
   the program cache. Otherwise, cache items are only identified by the
   hash of the source. Obviously, this runs the risk of collisions - otoh,
   this has not been observed as an issue so far. Enable source storing once
   that does.
 */
#define PROG_CACHE_STORE_SOURCE 0

static const uint32 cacheFileMagicCC = 0x04435043
                                       ^ (0x00202020 * PROG_CACHE_STORE_SOURCE);

bool csShaderGLCGCommon::TryLoadFromCompileCache (const char* source, 
                                                  const ProfileLimits& limits,
                                                  iHierarchicalCache* cache)
{
  csString objectCodeCachePathArc, objectCodeCachePathItem;
  
#if PROG_CACHE_STORE_SOURCE
  csMemFile stringIDs;
  {
    csStringReader reader (source);
    csString line;
    while (reader.GetLine (line))
    {
      StringStore::ID stringID = shaderPlug->stringStore->GetIDForString (line);
      uint64 diskID = csLittleEndian::UInt64 (stringID);
      stringIDs.Write ((char*)&diskID, sizeof (diskID));
    }
  }
#endif // PROG_CACHE_STORE_SOURCE
  
  iHierarchicalCache* rootCache = cache->GetTopCache();
  csMD5::Digest sourceMD5 = csMD5::Encode (source);
  csString cacheArcPath;
  cacheArcPath.Format ("/CgProgCache/%s",
    sourceMD5.HexString().GetData());
    
  csRef<iDataBuffer> cacheArcBuf = rootCache->ReadCache (cacheArcPath);
  if (!cacheArcBuf.IsValid()) return false;
  CS::PluginCommon::ShaderCacheHelper::MicroArchive cacheArc;
  {
    csMemFile cacheArcFile (cacheArcBuf, true);
    if (!cacheArc.Read (&cacheArcFile)) return false;
  }
  
  csRef<iFile> foundFile;
#if PROG_CACHE_STORE_SOURCE
  csString entryPrefix (limits.ToString().GetData());
  itemPrefix.Append ("/");
  for (size_t i = 0; i < cacheArc.GetEntriesNum(); i++)
  {
    const char* arcEntry = cacheArc.GetEntryName (i);
    if (strncmp (arcEntry, entryPrefix, entryPrefix.Length()) != 0) continue;
    csRef<iDataBuffer> cacheBuf = cacheArc.ReadEntry (arcEntry);
    if (!cacheBuf.IsValid()) continue;
    
    csRef<iFile> cacheFile;
    cacheFile.AttachNew (new csMemFile (cacheBuf, true));
  
    uint32 diskMagic;
    if (cacheFile->Read ((char*)&diskMagic, sizeof (diskMagic))
	!= sizeof (diskMagic)) continue;
    if (csLittleEndian::UInt32 (diskMagic) != cacheFileMagicCC)
      continue;

    csRef<iDataBuffer> cachedIDs =
      CS::PluginCommon::ShaderCacheHelper::ReadDataBuffer (cacheFile);
    
    if (cachedIDs->GetSize() != stringIDs.GetSize())
      continue;
    if (memcmp (cachedIDs->GetData(), stringIDs.GetData(),
        stringIDs.GetSize()) != 0)
      continue;
      
    foundFile = cacheFile;
    objectCodeCachePathArc.Format ("/%s",
      sourceMD5.HexString().GetData());
    objectCodeCachePathItem = arcEntry;
    break;
  }
#else
  {
    csRef<iDataBuffer> cacheBuf =
      cacheArc.ReadEntry (limits.ToString().GetData());
    if (!cacheBuf.IsValid()) return false;
    
    csRef<iFile> cacheFile;
    cacheFile.AttachNew (new csMemFile (cacheBuf, true));
  
    uint32 diskMagic;
    if (cacheFile->Read ((char*)&diskMagic, sizeof (diskMagic))
	!= sizeof (diskMagic)) return false;;
    if (csLittleEndian::UInt32 (diskMagic) != cacheFileMagicCC)
      return false;;
      
    uint32 diskSize;
    if (cacheFile->Read ((char*)&diskSize, sizeof (diskSize))
	!= sizeof (diskSize)) return false;;
    if (csLittleEndian::UInt32 (diskSize) != strlen (source))
      return false;;
      
    foundFile = cacheFile;
    objectCodeCachePathArc.Format ("/%s",
      sourceMD5.HexString().GetData());
    objectCodeCachePathItem = limits.ToString().GetData();
  }
#endif

  if (!foundFile.IsValid()) return false;
  
  {
    uint32 diskState;
    if (foundFile->Read ((char*)&diskState, sizeof (diskState))
	!= sizeof (diskState)) return false;
    if (csLittleEndian::UInt32 (diskState) != cpsValid)
    {
      if (program) cgDestroyProgram (program);
      program = 0;
      return true;
    }
  }
  
  objectCode =
    CS::PluginCommon::ShaderCacheHelper::ReadString (foundFile);
  if (objectCode.IsEmpty()) return false;
  
  unusedParams.DeleteAll();
  {
    csString p;
    while (!(p = CS::PluginCommon::ShaderCacheHelper::ReadString (foundFile))
      .IsEmpty())
    {
      unusedParams.Add (p);
    }
  }

  if (program)
  {
    cgDestroyProgram (program);
  }
  cgGetError(); // Clear error
  program = cgCreateProgram (shaderPlug->context, 
    CG_OBJECT, objectCode, limits.profile, 0, 0);
  if (!program) return false;
  CGerror err = cgGetError();
  if (err != CG_NO_ERROR)
  {
    const char* errStr = cgGetErrorString (err);
    shaderPlug->Report (CS_REPORTER_SEVERITY_WARNING,
      "Cg error %s", errStr);
    return false;
  }
  
  this->objectCodeCachePathArc = objectCodeCachePathArc;
  this->objectCodeCachePathItem = objectCodeCachePathItem;
  
  return true;
}

bool csShaderGLCGCommon::LoadObjectCodeFromCompileCache (
  const ProfileLimits& limits, iHierarchicalCache* cache)
{
  CS_ASSERT(!objectCodeCachePathArc.IsEmpty());
  CS_ASSERT(!objectCodeCachePathItem.IsEmpty());

  iHierarchicalCache* rootCache = cache->GetTopCache();
  csRef<iHierarchicalCache> progCache =
    rootCache->GetRootedCache ("/CgProgCache");
  
  csRef<iDataBuffer> cacheArcBuf = progCache->ReadCache (objectCodeCachePathArc);
  if (!cacheArcBuf.IsValid()) return false;
  CS::PluginCommon::ShaderCacheHelper::MicroArchive cacheArc;
  {
    csMemFile cacheArcFile (cacheArcBuf, true);
    if (!cacheArc.Read (&cacheArcFile)) return false;
  }
    
  csRef<iDataBuffer> cacheBuf = cacheArc.ReadEntry (objectCodeCachePathItem);
  if (!cacheBuf.IsValid()) return false;
  
  csRef<iFile> cacheFile;
  cacheFile.AttachNew (new csMemFile (cacheBuf, true));
  
  uint32 diskMagic;
  if (cacheFile->Read ((char*)&diskMagic, sizeof (diskMagic))
      != sizeof (diskMagic)) return false;
  if (csLittleEndian::UInt32 (diskMagic) != cacheFileMagicCC)
    return false;
    
  {
#if PROG_CACHE_STORE_SOURCE
    csRef<iDataBuffer> skipBuf = 
      CS::PluginCommon::ShaderCacheHelper::ReadDataBuffer (cacheFile);
#else
    uint32 dummy;
    cacheFile->Read ((char*)&dummy, sizeof (dummy));
#endif
  }

  
  if (!cacheFile.IsValid()) return false;
  
  {
    uint32 diskState;
    if (cacheFile->Read ((char*)&diskState, sizeof (diskState))
	!= sizeof (diskState)) return false;
    if (csLittleEndian::UInt32 (diskState) != cpsValid)
    {
      objectCode.Empty();
      return true;
    }
  }
  
  objectCode =
    CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile);
    
  unusedParams.DeleteAll();
  {
    csString p;
    while (!(p = CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile))
      .IsEmpty())
    {
      unusedParams.Add (p);
    }
  }
  
  return true;
}

bool csShaderGLCGCommon::WriteToCompileCache (const char* source,
                                              const ProfileLimits& limits,
                                              iHierarchicalCache* cache,
                                              csString& failReason)
{
  CS_ASSERT(objectCodeCachePathArc.IsEmpty());
  CS_ASSERT(objectCodeCachePathItem.IsEmpty());
  
#if PROG_CACHE_STORE_SOURCE
  csMemFile stringIDs;
  {
    csStringReader reader (source);
    csString line;
    while (reader.GetLine (line))
    {
      StringStore::ID stringID = shaderPlug->stringStore->GetIDForString (line);
      uint64 diskID = csLittleEndian::UInt64 (stringID);
      stringIDs.Write ((char*)&diskID, sizeof (diskID));
    }
  }
#endif
  
  iHierarchicalCache* rootCache = cache->GetTopCache();
  csMD5::Digest sourceMD5 = csMD5::Encode (source);
  csString cacheArcPath;
  cacheArcPath.Format ("/CgProgCache/%s",
    sourceMD5.HexString().GetData());
  
  CS::PluginCommon::ShaderCacheHelper::MicroArchive cacheArc;
  {
    csRef<iDataBuffer> cacheArcBuf = rootCache->ReadCache (cacheArcPath);
    if (cacheArcBuf.IsValid())
    {
      csMemFile cacheArcFile (cacheArcBuf, true);
      cacheArc.Read (&cacheArcFile);
    }
  }
  
#if PROG_CACHE_STORE_SOURCE
  csString subItem;
  csString entryPrefix (limits.ToString().GetData());
  itemPrefix.Append ("/");
  for (size_t i = 0; i < cacheArc.GetEntriesNum(); i++)
  {
    const char* arcEntry = cacheArc.GetEntryName (i);
    if (strncmp (arcEntry, entryPrefix, entryPrefix.Length()) != 0) continue;
    csRef<iDataBuffer> cacheBuf = cacheArc.ReadEntry (arcEntry);
    if (!cacheBuf.IsValid()) continue;
    
    csRef<iFile> cacheFile;
    cacheFile.AttachNew (new csMemFile (cacheBuf, true));
  
    uint32 diskMagic;
    if (cacheFile->Read ((char*)&diskMagic, sizeof (diskMagic))
	!= sizeof (diskMagic)) continue;
    if (csLittleEndian::UInt32 (diskMagic) != cacheFileMagicCC)
      continue;
    
    csRef<iDataBuffer> cachedIDs =
      CS::PluginCommon::ShaderCacheHelper::ReadDataBuffer (cacheFile);
    
    if (cachedIDs->GetSize() != stringIDs.GetSize())
      continue;
    if (memcmp (cachedIDs->GetData(), stringIDs.GetData(),
        stringIDs.GetSize()) != 0)
      continue;
      
    subItem = arcEntry;
    break;
  }
  
  if (subItem.IsEmpty())
  {
    uint n = 0;
    csRef<iDataBuffer> item;
    do
    {
      subItem.Format ("%s%u", entryPrefix.GetData(), n++);
      item = cacheArc.ReadEntry (subItem);
    }
    while (item.IsValid());
  }
#endif
  
  csMemFile cacheFile;
  
  uint32 diskMagic = csLittleEndian::UInt32 (cacheFileMagicCC);
  if (cacheFile.Write ((char*)&diskMagic, sizeof (diskMagic))
      != sizeof (diskMagic))
  {
    failReason = "write error (magic)";
    return false;
  }
  
#if PROG_CACHE_STORE_SOURCE
  {
    csRef<iDataBuffer> idsBuffer (stringIDs.GetAllData());
    if (!CS::PluginCommon::ShaderCacheHelper::WriteDataBuffer (&cacheFile,
        idsBuffer))
    {
      failReason = "write error (source)";
      return false;
    }
  }
#else
  uint32 diskSize = csLittleEndian::UInt32 (strlen (source));
  if (cacheFile.Write ((char*)&diskSize, sizeof (diskSize))
      != sizeof (diskSize))
  {
    failReason = "write error (source size)";
    return false;
  }
#endif
  
  const char* object;
  if ((program == 0)
    || ((object = cgGetProgramString (program, CG_COMPILED_PROGRAM)) == 0)
    || (*object == 0))
  {
    uint32 diskState = csLittleEndian::UInt32 (cpsInvalid);
    if (cacheFile.Write ((char*)&diskState, sizeof (diskState))
	!= sizeof (diskState))
    {
      failReason = "write error (state-invalid)";
      return false;
    }
  }
  else
  {
    {
      uint32 diskState = csLittleEndian::UInt32 (cpsValid);
      if (cacheFile.Write ((char*)&diskState, sizeof (diskState))
	  != sizeof (diskState))
      {
	failReason = "write error (state-valid)";
	return false;
      }
    }
    
    if (!CS::PluginCommon::ShaderCacheHelper::WriteString (&cacheFile, object))
    {
      failReason = "write error (object code)";
      return false;
    }
    
    csSet<csString>::GlobalIterator iter (unusedParams.GetIterator());
    while (iter.HasNext())
    {
      const csString& s = iter.Next();
      if (!CS::PluginCommon::ShaderCacheHelper::WriteString (&cacheFile, s))
      {
	failReason = "write error (unused param)";
	return false;
      }
    }

    if (!CS::PluginCommon::ShaderCacheHelper::WriteString (&cacheFile, 0))
    {
      failReason = "write error (empty string)";
      return false;
    }
  }
  
  csRef<iDataBuffer> cacheData = cacheFile.GetAllData();
#if PROG_CACHE_STORE_SOURCE
  objectCodeCachePathItem = subItem;
#else
  objectCodeCachePathItem = limits.ToString().GetData();
#endif
  if (!cacheArc.WriteEntry (objectCodeCachePathItem, cacheData))
  {
    objectCodeCachePathItem.Empty();
    failReason = "failed writing cache entry";
    return false;
  }
  objectCodeCachePathArc.Format ("/%s",
    sourceMD5.HexString().GetData());
  
  csMemFile cacheArcFile;
  bool cacheArcWrite;
  if ((!(cacheArcWrite = cacheArc.Write (&cacheArcFile)))
    || !(rootCache->CacheData (cacheArcFile.GetData(), cacheArcFile.GetSize(),
      cacheArcPath)))
  {
    objectCodeCachePathArc.Empty();
    objectCodeCachePathItem.Empty();

    if (!cacheArcWrite)
      failReason = "failed writing archive";
    else
      failReason = "failed caching archive";
    return false;
  }
  
  return true;
}

}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)
