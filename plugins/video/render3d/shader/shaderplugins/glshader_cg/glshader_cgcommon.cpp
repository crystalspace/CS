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
  for(size_t i = 0; i < variablemapConstants.GetSize (); ++i)
  {
    VariableMapEntry& mapping = variablemapConstants[i];
    
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
      && (programPositionInvariant
	|| (shaderPlug->vendor ==
	  CS::PluginCommon::ShaderProgramPluginGL::ATI)))
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
	  shaderPlug->clipPlanes.AddClipPlane (plane, clip.space);
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
  /*objectCode.Empty();
  objectCodeCachePathArc.Empty();
  objectCodeCachePathItem.Empty();*/
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
  csString augmentedProgramStr = GetAugmentedProgram (programStr,
    (flags & loadFlagUnusedV2FForInit) != 0);
    
  programStr = augmentedProgramStr;
  CGprofile profile = customLimits.profile;
  CS::PluginCommon::ShaderProgramPluginGL::HardwareVendor vendor =
    customLimits.vendor;

  if (shaderPlug->doVerbose || shaderPlug->doVerbosePrecache)
  {
    shaderPlug->Report (CS_REPORTER_SEVERITY_NOTIFY,
      "Cg %s program %s: using profile %s[%d]", GetProgramType(),
      CS::Quote::Single (description.GetData ()),
      cgGetProfileString (profile), profile);
  }

  ArgumentArray args;
  shaderPlug->GetProfileCompilerArgs (GetProgramType(), profile, 
    customLimitsPair,
    vendor,
    (flags & loadIgnoreConfigProgramOpts) ? csGLShader_CG::argsNoConfig
      : csGLShader_CG::argsAll, args);
  for (i = 0; i < compilerArgs.GetSize(); i++) 
    args.Push (compilerArgs[i]);
  programPositionInvariant = false;
  for (i = 0; i < args.GetSize(); ) 
  {
    if (strcmp (args[i], "-posinv") == 0)
    {
      if (profile >= CG_PROFILE_GPU_VP)
      {
	/* Work around Cg 2.0 and above (including 3.0) bug: it emits
	   "OPTION ARB_position_invariant;" AND computes result.position in
	   the VP - doing both is verboten.
	   Affected are the GP4VP and higher profiles.
	   Remedy: remove -posinv argument 
	 */
	args.DeleteIndex (i);
	continue;
      }
      programPositionInvariant = true;
    }
    i++;
  }
  customLimits.ToCgOptions (args);
  args.Push (0);
 
  if (program)
  {
    cgDestroyProgram (program);
  }
  shaderPlug->SetCompiledSource (programStr);
  shaderPlug->SetIgnoreErrors (true);
  program = cgCreateProgram (shaderPlug->context, 
    CG_SOURCE, programStr, 
    profile, !entrypoint.IsEmpty() ? entrypoint : "main", args.GetArray());
  shaderPlug->SetIgnoreErrors (false);
  
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

  GetParamsFromVmapConstants();
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
	  && (((type == CG_GL_VERTEX) && (profile >= CG_PROFILE_ARBVP1))
	    || ((type == CG_GL_FRAGMENT) && (profile >= CG_PROFILE_ARBFP1))))
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

csString csShaderGLCGCommon::GetAugmentedProgram (const char* programStr,
                                                  bool initializeUnusedV2F)
{
  csString augmentedProgramStr;
  
  if (programPositionInvariant)
    augmentedProgramStr.Append ("/* position invariant */\n");
  
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
  
  if (initializeUnusedV2F)
    augmentedProgramStr.Append ("#define _INITIALIZE_UNUSED_V2F\n");
  
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

  output << "Program leaf parameters:\n";
  CGparameter param = cgGetFirstLeafParameter (program, CG_PROGRAM);
  while (param)
  {
    DebugDumpParam (output, param);
    param = cgGetNextLeafParameter (param);
  }
  output << "\n";

  output << "Program global parameters:\n";
  param = cgGetFirstLeafParameter (program, CG_GLOBAL);
  while (param)
  {
    DebugDumpParam (output, param);
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
      "Could not write %s", CS::Quote::Single (debugFN.GetData()));
  }
  else
  {
    debugFile->Write (output.GetData(), output.Length ());
    csReport (objectReg, CS_REPORTER_SEVERITY_NOTIFY, 
      "crystalspace.graphics3d.shader.glcg",
      "Dumped Cg program info to %s", CS::Quote::Single (debugFN.GetData()));
  }
}

void csShaderGLCGCommon::DebugDumpParam (csString& output, CGparameter param)
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
      "Could not augment %s", CS::Quote::Single (debugFN.GetData()));
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
  const char* tag, const ProgramObject& program,
  csString& failReason)
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
  CS::Utility::Checksum::MD5::Digest progHash = CS::Utility::Checksum::MD5::Encode (
    programBuffer->GetData(), programBuffer->GetSize());
  if (cacheFile.Write ((char*)&progHash, sizeof (progHash))
      != sizeof (progHash))
  {
    failReason = "write error (hash)";
    return false;
  }
  
  if (!program.IsValid())
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
    
    CS_ASSERT(!program.GetID().archive.IsEmpty());
    CS::PluginCommon::ShaderCacheHelper::WriteString (&cacheFile,
      program.GetID().archive);
    CS_ASSERT(!program.GetID().item.IsEmpty());
    CS::PluginCommon::ShaderCacheHelper::WriteString (&cacheFile,
      program.GetID().item);
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
                                       const char* tag, const ProgramObject& program)
{
  csString failReason;
  if (!WriteToCacheWorker (cache, limits, limitsPair, tag, program, failReason))
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
  
bool csShaderGLCGCommon::WriteToCache (iHierarchicalCache* cache,
                                       const ProfileLimits& limits,
                                       const ProfileLimitsPair& limitsPair, 
                                       const char* tag)
{
  csString objectCode;
  if (program != 0)
    objectCode = cgGetProgramString (program, CG_COMPILED_PROGRAM);
  ProgramObject programObj (objectCode,
    programPositionInvariant ? ProgramObject::flagPositionInvariant : 0,
    unusedParams);
  
  const char* preprocSource (cgGetProgramString (program, CG_PROGRAM_SOURCE));
  csString failReason;
  ProgramObjectID progId;
  if (!shaderPlug->progCache.WriteObject (preprocSource, limits, programObj,
    progId, failReason))
  {
    if (shaderPlug->doVerbose)
    {
      csReport (objectReg, CS_REPORTER_SEVERITY_WARNING, 
	"crystalspace.graphics3d.shader.glcg",
	"Error writing %s program for %s to compile cache: %s",
	GetProgramType(), tag,
	failReason.GetData());
    }
    return false;
  }
  
  programObj.SetID (progId);
  
  return WriteToCache (cache, limits, limitsPair, tag, programObj);
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
  csRef<iString>* failReason, csRef<iString>* tag,
  ProfileLimitsPair* cacheLimits)
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
  CS::Utility::Checksum::MD5::Digest progHash = CS::Utility::Checksum::MD5::Encode (
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
      
    CS::Utility::Checksum::MD5::Digest diskHash;
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
  ProfileLimits bestLimits (
    CS::PluginCommon::ShaderProgramPluginGL::Other,
    CG_PROFILE_UNKNOWN);
  bool bestLimitsSet = false;
  for (size_t i = cachedProgWrappers.GetSize(); i-- > 0;)
  {
    const CachedShaderWrapper& wrapper = cachedProgWrappers[i];
    const ProfileLimits& limits =
      (programType == progVP) ? wrapper.limits.vp : wrapper.limits.fp;

    if (!bestLimitsSet)
    {
      bestLimits = limits;
      bestLimitsSet = true;
    }
      
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
    
    csString objectCodeCachePathArc =
      CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile);
    if (objectCodeCachePathArc.IsEmpty()) continue;
    csString objectCodeCachePathItem =
      CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile);
    if (objectCodeCachePathItem.IsEmpty()) continue;
    
    ProgramObjectID progId (objectCodeCachePathArc, objectCodeCachePathItem);
    ProgramObject programObj;
    //if (!LoadObjectCodeFromCompileCache (limits, cache))
    if (!shaderPlug->progCache.LoadObject (progId, programObj))
      continue;
    
    oneReadCorrectly = true;
    if (program)
    {
      cgDestroyProgram (program);
      program = 0;
    }
    
    if (!programObj.IsValid()) continue;
    
    cgGetError(); // Clear error
    program = cgCreateProgram (shaderPlug->context, 
      CG_OBJECT, programObj.GetObjectCode(), limits.profile, 0, 0);
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
    programPositionInvariant = programObj.GetFlags() & ProgramObject::flagPositionInvariant;
    unusedParams = programObj.GetUnusedParams();
    
    ClipsToVmap();
    GetParamsFromVmap();

    bool doLoadToGL = !shaderPlug->ProfileNeedsRouting (programProfile);

    cgGetError(); // Clear error
    if (doLoadToGL)
    {
      cgGLLoadProgram (program);
    }
    else
    {
      cgCompileProgram (program);
    }
    
    shaderPlug->PrintAnyListing();
    err = cgGetError();
    if ((err != CG_NO_ERROR)
      || (doLoadToGL && !cgGLIsProgramLoaded (program)))
    {
      //if (shaderPlug->debugDump)
	//DoDebugDump();
	
      const char* errStr = cgGetErrorString (err);
      shaderPlug->Report (CS_REPORTER_SEVERITY_WARNING,
	"Cg error %s", errStr);
  
      if (shaderPlug->doVerbose
	  && (((programType == progVP) && (programProfile >= CG_PROFILE_ARBVP1))
	    || ((programType == progFP) && (programProfile >= CG_PROFILE_ARBFP1))))
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

    if (cacheLimits != 0)
      *cacheLimits = wrapper.limits;
    
    bool loaded = !shaderPlug->ProfileNeedsRouting (programProfile)
      || LoadProgramWithPS1 ();
    if (loaded && (bestLimits < currentLimits))
    {
      /* The best found program is worse than the current limits, so pretend
         that the shader program failed (instead just being 'invalid') -
         that will make xmlshader try to load the program from scratch,
         ie with current limits, which may just work. */
      if (failReason)
        failReason->AttachNew (new scfString ("Provoking clean load with current limits"));
      return iShaderProgram::loadFail;
    }
    return loaded ? iShaderProgram::loadSuccessShaderValid
        : iShaderProgram::loadSuccessShaderInvalid;
  }
  
  if (oneReadCorrectly)
  {
    if (bestLimits < currentLimits)
    {
      /* The 'invalid' programs may compile with the current limits -
         so again, provoke clean load */
      if (failReason)
        failReason->AttachNew (new scfString ("Provoking clean load with current limits"));
      return iShaderProgram::loadFail;
    }
    else
      return iShaderProgram::loadSuccessShaderInvalid;
  }
  else
    return iShaderProgram::loadFail;
}

bool csShaderGLCGCommon::LoadProgramWithPS1 ()
{
  pswrap = shaderPlug->psplg->CreateProgram ("fp");

  if (!pswrap)
    return false;

  const char* objectCode = cgGetProgramString (program, CG_COMPILED_PROGRAM);
  if (!objectCode || !*objectCode)
    // Program did not actually compile
    return false;

  csArray<csShaderVarMapping> mappings;
  
  for (size_t i = 0; i < variablemap.GetSize (); i++)
  {
    // Get the Cg parameter
    ShaderParameter* sparam =
      reinterpret_cast<ShaderParameter*> (variablemap[i].userVal);
    // Make sure it's a C-register
    CGresource resource = cgGetParameterResource (sparam->param);
    if (resource == CG_C)
    {
      // Get the register number, and create a mapping
      csString regnum;
      regnum.Format ("c%lu", cgGetParameterResourceIndex (sparam->param));
      mappings.Push (csShaderVarMapping (variablemap[i].name, regnum));
    }
  }

  if (pswrap->Load (0, objectCode, mappings))
  {
    bool ret = pswrap->Compile (0);
    if (shaderPlug->debugDump)
      DoDebugDump();
    return ret;
  }
  else
  {
    return false;
  }
}

}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)
