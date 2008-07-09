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
#include "csplugincommon/shader/shadercachehelper.h"
#include "csutil/csendian.h"
#include "csutil/documenthelper.h"
#include "csutil/memfile.h"
#include "csutil/scfstr.h"
#include "iutil/hiercache.h"
#include "iutil/stringarray.h"
#include "ivaria/reporter.h"

#include "glshader_cg.h"
#include "glshader_cgcommon.h"
#include "profile_limits.h"

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{

CS_LEAKGUARD_IMPLEMENT (csShaderGLCGCommon);

csShaderGLCGCommon::csShaderGLCGCommon (csGLShader_CG* shaderPlug, 
					ProgramType type) :
  scfImplementationType (this, shaderPlug->object_reg), programType (type)
{
  validProgram = true;
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
}

void csShaderGLCGCommon::Deactivate()
{
  cgGLDisableProfile (programProfile);
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
	
      if (!clip.distance.IsConstant())
      {
	csVector4 v (GetParamVectorVal (stack, clip.distance,
	  csVector4 (0, 0, 0, 0)));
	float distVal = v[clip.distComp];
	if (clip.distNeg) distVal = -distVal;
	packDist[c/4][c%4] = distVal;
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
      bool hasDistSV = !constDist
        && GetParamVectorVal (stack, clip.distance, &v);
      doClipping = (constPlane && constDist)
        || (constPlane && hasDistSV)
        || (hasPlaneSV && constDist)
        || (hasPlaneSV && hasDistSV);
      if (doClipping) clipFlags |= 1 << c;
    }
    clipPackedDists[0]->SetValue (packDist[0]);
    clipPackedDists[1]->SetValue (packDist[1]);
  
    if ((programProfile == CG_PROFILE_VP30)
        || (programProfile == CG_PROFILE_VP40)
        || (programProfile == CG_PROFILE_GPU_VP))
    {
      for (size_t c = 0; c < clips.GetSize(); c++)
      {
	if (clipFlags & (1 << c))
          shaderPlug->clipPlanes.EnableClipPlane (c);
      }
    }
    else if (programProfile == CG_PROFILE_ARBVP1)
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

bool csShaderGLCGCommon::DefaultLoadProgram (
  iShaderDestinationResolverCG* cgResolve,
  const char* programStr, CGGLenum type, CGprofile maxProfile, 
  uint flags, const ProfileLimits* customLimits)
{
  if (!programStr || !*programStr) return false;

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
  CGprofile profile = CG_PROFILE_UNKNOWN;

  if (customLimits != 0)
  {
    profile = customLimits->profile;
  }
  else
  {
    if (!cg_profile.IsEmpty())
      profile = cgGetProfile (cg_profile);
  
    if(profile == CG_PROFILE_UNKNOWN)
      profile = cgGLGetLatestProfile (type);
  
    if (maxProfile != CG_PROFILE_UNKNOWN)
      profile = csMin (profile, maxProfile);
  }

  if (shaderPlug->doVerbose)
  {
    shaderPlug->Report (CS_REPORTER_SEVERITY_NOTIFY,
      "Cg %s program '%s': using profile %s[%d]", GetProgramType(),
      description.GetData (), cgGetProfileString (profile), profile);
  }

  ArgumentArray args;
  shaderPlug->GetProfileCompilerArgs (GetProgramType(), profile, 
    flags & loadIgnoreConfigProgramOpts, args);
  for (i = 0; i < compilerArgs.GetSize(); i++) 
    args.Push (compilerArgs[i]);
  if (customLimits != 0) customLimits->ToCgOptions (args);
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
      //if (shaderPlug->debugDump)
	//DoDebugDump();

      if (shaderPlug->doVerbose
	  && ((type == CG_GL_VERTEX) && (profile >= CG_PROFILE_ARBVP1))
	    || ((type == CG_GL_FRAGMENT) && (profile >= CG_PROFILE_ARBFP1)))
      {
	//const char* err = (char*)glGetString (GL_PROGRAM_ERROR_STRING_ARB);
	const char* err = "";
	shaderPlug->Report (CS_REPORTER_SEVERITY_WARNING,
	  "OpenGL error string: %s", err);
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
    result = numVaryings <= 16;
  }
  if (!result && !debugFN.IsEmpty())
  {
    csRef<iVFS> vfs = csQueryRegistry<iVFS> (objectReg);
    vfs->DeleteFile (debugFN);
  }
  return result;
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
  output << "CG program valid: " << validProgram << "\n";
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

/* Magic value for cpg program files.
 * The most significant byte serves as a "version", increase when the
 * cache file format changes. */
static const uint32 cacheFileMagic = 0x02706763;

enum
{
  cpsValid = 0x6b726f77,
  cpsInvalid = 0x6b723062
};

bool csShaderGLCGCommon::WriteToCache (iHierarchicalCache* cache,
                                       const ProfileLimits& limits)
{
  if (!cache) return false;

  csRefArray<iDocumentNode> cacheKeepNodes (this->cacheKeepNodes);
  this->cacheKeepNodes.DeleteAll ();

  csMemFile cacheFile;
  
  uint32 diskMagic = csLittleEndian::UInt32 (cacheFileMagic);
  if (cacheFile.Write ((char*)&diskMagic, sizeof (diskMagic))
      != sizeof (diskMagic)) return false;
  
  {
    uint32 diskProfile = csLittleEndian::UInt32 (programProfile);
    if (cacheFile.Write ((char*)&diskProfile, sizeof (diskProfile))
	!= sizeof (diskProfile)) return false;
  }
  
  if (!limits.Write (&cacheFile)) return false;
  
  const char* object;
  if ((program == 0)
    || ((object = cgGetProgramString (program, CG_COMPILED_PROGRAM)) == 0)
    || (*object == 0))
  {
    uint32 diskState = csLittleEndian::UInt32 (cpsInvalid);
    if (cacheFile.Write ((char*)&diskState, sizeof (diskState))
	!= sizeof (diskState)) return false;
  }
  else
  {
    {
      uint32 diskState = csLittleEndian::UInt32 (cpsValid);
      if (cacheFile.Write ((char*)&diskState, sizeof (diskState))
	  != sizeof (diskState)) return false;
    }
    
    CS::PluginCommon::ShaderCacheHelper::WriteString (&cacheFile, description);
    
    csRef<iDocumentSystem> docsys = shaderPlug->binDocSys;
    if (!docsys.IsValid()) docsys = shaderPlug->xmlDocSys;
    csRef<iDocument> doc = docsys->CreateDocument();
    csRef<iDocumentNode> root = doc->CreateRoot();
    for (size_t i = 0; i < cacheKeepNodes.GetSize(); i++)
    {
      csRef<iDocumentNode> newNode = root->CreateNodeBefore (
	cacheKeepNodes[i]->GetType());
      CS::DocSystem::CloneNode (cacheKeepNodes[i], newNode);
    }
    
    {
      csMemFile docFile;
      const char* err = doc->Write (&docFile);
      if (err != 0)
      {
	csReport (objectReg, CS_REPORTER_SEVERITY_WARNING, 
	  "crystalspace.graphics3d.shader.glcg",
	  "Error writing document: %s", err);
      }
      csRef<iDataBuffer> docBuf = docFile.GetAllData (false);
      if (!CS::PluginCommon::ShaderCacheHelper::WriteDataBuffer (&cacheFile,
	  docBuf))
	return false;
    }
    
    CS::PluginCommon::ShaderCacheHelper::WriteString (&cacheFile, object);
  }
  
  csString cacheName ("/");
  cacheName += limits.ToString ();
  return cache->CacheData (cacheFile.GetData(), cacheFile.GetSize(),
    cacheName);
}

struct CachedShaderWrapper
{
  csString name;
  csRef<iFile> cacheFile;
  ProfileLimits limits;
  
  CachedShaderWrapper (iFile* file, CGprofile profile) : cacheFile (file),
    limits (profile) {}
    
  bool operator< (const CachedShaderWrapper& other) const
  { return limits < other.limits; }
};

iShaderProgram::CacheLoadResult csShaderGLCGCommon::LoadFromCache (
  iHierarchicalCache* cache, csRef<iString>* failReason)
{
  if (!cache) return iShaderProgram::loadFail;

  csRef<iStringArray> allCachedPrograms = cache->GetSubItems ("/");
  if (!allCachedPrograms.IsValid())
  {
    if (failReason) failReason->AttachNew (
      new scfString ("no cached programs found"));
    return iShaderProgram::loadFail;
  }
  
  csArray<CachedShaderWrapper> cachedProgWrappers;
  for (size_t i = 0; i < allCachedPrograms->GetSize(); i++)
  {
    csString cachePath ("/");
    cachePath.Append (allCachedPrograms->Get (i));
    csRef<iDataBuffer> cacheBuf = cache->ReadCache (cachePath);
    if (!cacheBuf.IsValid()) continue;
    
    csRef<iFile> cacheFile;
    cacheFile.AttachNew (new csMemFile (cacheBuf, true));
  
    uint32 diskMagic;
    if (cacheFile->Read ((char*)&diskMagic, sizeof (diskMagic))
	!= sizeof (diskMagic)) continue;
    if (csLittleEndian::UInt32 (diskMagic) != cacheFileMagic)
      continue;
      
    CGprofile profile;
    {
      uint32 diskProfile;
      if (cacheFile->Read ((char*)&diskProfile, sizeof (diskProfile))
	  != sizeof (diskProfile)) continue;
      profile = (CGprofile)csLittleEndian::UInt32 (diskProfile);
    }
    
    CachedShaderWrapper wrapper (cacheFile, profile);
    if (!wrapper.limits.Read (cacheFile)) continue;
    
    wrapper.name = allCachedPrograms->Get (i);
    
    cachedProgWrappers.Push (wrapper);
  }
  
  if (cachedProgWrappers.GetSize() == 0)
  {
    if (failReason) failReason->AttachNew (
      new scfString ("all cached programs failed to read"));
    return iShaderProgram::loadFail;
  }
  
  cachedProgWrappers.Sort ();

  csString allReasons;
  bool oneReadCorrectly = false;
  for (size_t i = cachedProgWrappers.GetSize(); i-- > 0;)
  {
    const CachedShaderWrapper& wrapper = cachedProgWrappers[i];
  
    bool profileSupported =
      (shaderPlug->ProfileNeedsRouting (wrapper.limits.profile)
        && shaderPlug->IsRoutedProfileSupported (wrapper.limits.profile))
      || cgGLIsProfileSupported (wrapper.limits.profile);
    if (!profileSupported)
    {
      allReasons += wrapper.name;
      allReasons += ": Profile unsupported; ";
      continue;
    }
    
    ProfileLimits currentLimits (wrapper.limits.profile);
    currentLimits.GetCurrentLimits ();
    bool limitsSupported = currentLimits >= wrapper.limits;
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
    
    csRef<iDataBuffer> docBuf =
      CS::PluginCommon::ShaderCacheHelper::ReadDataBuffer (cacheFile);
    if (!docBuf.IsValid()) continue;
    
    csRef<iDocument> doc;
    if (shaderPlug->binDocSys.IsValid())
    {
      doc = shaderPlug->binDocSys->CreateDocument ();
      const char* err = doc->Parse (docBuf);
      if (err != 0)
      {
	csReport (objectReg, CS_REPORTER_SEVERITY_WARNING, 
	  "crystalspace.graphics3d.shader.glcg",
	  "Error reading document: %s", err);
      }
    }
    if (!doc.IsValid() && shaderPlug->xmlDocSys.IsValid())
    {
      doc = shaderPlug->xmlDocSys->CreateDocument ();
      const char* err = doc->Parse (docBuf);
      if (err != 0)
      {
	csReport (objectReg, CS_REPORTER_SEVERITY_WARNING, 
	  "crystalspace.graphics3d.shader.glcg",
	  "Error reading document: %s", err);
      }
    }
    if (!doc.IsValid()) continue;
    
    csRef<iDocumentNode> root = doc->GetRoot();
    if (!root.IsValid()) continue;
    
    bool breakFail = false;
    csRef<iDocumentNodeIterator> nodes = root->GetNodes();
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
	  breakFail = true;
      }
    }
    if (breakFail) continue;
    
    objectCode =
      CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile);
    if (objectCode.IsEmpty()) continue;
    
    oneReadCorrectly = true;
    program = cgCreateProgram (shaderPlug->context, 
      CG_OBJECT, objectCode, wrapper.limits.profile, 0, 0);
    if (!program) continue;
    CGerror err = cgGetError();
    if (err != CG_NO_ERROR)
    {
      const char* errStr = cgGetErrorString (err);
      shaderPlug->Report (CS_REPORTER_SEVERITY_WARNING,
	"Cg error %s", errStr);
      continue;
    }
    programProfile = wrapper.limits.profile;
    
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
    
    return iShaderProgram::loadSuccessShaderValid;
  }
  
  if (failReason) failReason->AttachNew (
    new scfString (allReasons));
  return oneReadCorrectly ? iShaderProgram::loadSuccessShaderInvalid : iShaderProgram::loadFail;
}

}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)
