/*
Copyright (C) 2002 by Anders Stenberg
                      Marten Svanfeldt

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
#include "csplugincommon/opengl/glextmanager.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/scfstr.h"
#include "csutil/scfstringarray.h"
#include "csutil/stringquote.h"
#include "csutil/xmltiny.h"
#include "iutil/comp.h"
#include "iutil/hiercache.h"
#include "iutil/plugin.h"
#include "iutil/stringarray.h"
#include "ivaria/reporter.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"

#include "glshader_cgvp.h"
#include "glshader_cgfp.h"
#include "glshader_cg.h"
#include "profile_limits.h"
#include "stringstore.h"



CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{

CS_LEAKGUARD_IMPLEMENT (csGLShader_CG);

SCF_IMPLEMENT_FACTORY (csGLShader_CG)

csGLShader_CG::csGLShader_CG (iBase* parent) : 
  scfImplementationType (this, parent), compiledProgram (0),
  stringStore (0)
{
  context = cgCreateContext ();
  cgSetErrorHandler (ErrorHandlerObjReg, 0);

  enable = false;
  debugDump = false;
  dumpDir = 0;
}

csGLShader_CG::~csGLShader_CG()
{
  cs_free (dumpDir);
  cgDestroyContext (context);
  cgSetErrorHandler (ErrorHandlerObjReg, object_reg);
  delete stringStore;
}

void csGLShader_CG::ErrorHandler (CGcontext context, CGerror error, 
				  void* appData)
{
  csGLShader_CG* This = reinterpret_cast<csGLShader_CG*> (appData);
  if (This->doIgnoreErrors) return;

  bool doVerbose;
  csRef<iVerbosityManager> verbosemgr (
    csQueryRegistry<iVerbosityManager> (This->object_reg));
  if (verbosemgr) 
    doVerbose = verbosemgr->Enabled ("renderer.shader");
  else
    doVerbose = false;
  if (!doVerbose) return;

  csReport (This->object_reg, CS_REPORTER_SEVERITY_WARNING,
    "crystalspace.graphics3d.shader.glcg",
    "%s", cgGetErrorString (error));
  if (error == CG_COMPILER_ERROR)
  {
    const char* listing = cgGetLastListing (context);
    This->PrintCgListing (listing);
  }
}

void csGLShader_CG::ErrorHandlerObjReg (CGcontext context, CGerror error, 
				        void* appData)
{
  iObjectRegistry* object_reg = reinterpret_cast<iObjectRegistry*> (appData);

  if (object_reg)
  {
    bool doVerbose;
    csRef<iVerbosityManager> verbosemgr (
      csQueryRegistry<iVerbosityManager> (object_reg));
    if (verbosemgr) 
      doVerbose = verbosemgr->Enabled ("renderer.shader");
    else
      doVerbose = false;
    if (!doVerbose) return;
  }

  csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
    "crystalspace.graphics3d.shader.glcg",
    "%s", cgGetErrorString (error));
}

void csGLShader_CG::PrintCgListing (const char* listing)
{
  if (listing && *listing)
  {
    if (compiledProgram != 0)
    {
      /* There is a listing - we split up up the source and parse the 
       * listing string in order to be able to write out the source lines
       * causing errors. */
      csStringArray sourceSplit;
      sourceSplit.SplitString (compiledProgram, "\n");
      csStringArray listingSplit;
      listingSplit.SplitString (listing, "\n");
      
      for (size_t l = 0; l < listingSplit.GetSize(); l++)
      {
        const char* listingLine = listingSplit[l];
        if (!listingLine || !*listingLine) continue;
        
	csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
	  "crystalspace.graphics3d.shader.glcg",
	  "%s", listingLine);
	  
	while (*listingLine && (*listingLine == ' ')) listingLine++;
	uint lineNum = 0;
	if (sscanf (listingLine, "(%u)", &lineNum) == 1)
	{
	  if ((lineNum > 0) && (lineNum <= sourceSplit.GetSize()))
	    csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
	      "crystalspace.graphics3d.shader.glcg",
	      "%s", sourceSplit[lineNum-1]);
	}
      }
    }
    else
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
	"crystalspace.graphics3d.shader.glcg",
	"%s", listing);
    }
  }
}

void csGLShader_CG::SplitArgsString (const char* str, ArgumentArray& args)
{
  if ((str == 0) || (*str == 0)) return;

  csString s;
  const char* p = str;
  bool quote = false;
  while (*p)
  {
    switch (*p)
    {
      case '"':
        quote = !quote;
        break;
      case '\\':
        if (quote)
        {
          p++;
          s.Append (*p);
          break;
        }
        // else fall through
      case '\r':
      case '\n':
      case ' ':
        if (!quote)
        {
          if (!s.IsEmpty()) args.Push (s);
          s.Empty();
          break;
        }
        // else fall through
      default:
        s.Append (*p);
    }
    p++;
  }
  if (!s.IsEmpty()) args.Push (s);
}

/// Assign a number to a Cg profile so DX and GL profiles can be compared.
static int GetProfileLevel (CGprofile profile)
{
  switch (profile)
  {
    case CG_PROFILE_FP20:
    case CG_PROFILE_PS_1_1:
      return 0x101;
    case CG_PROFILE_PS_1_2:
      return 0x102;
    case CG_PROFILE_PS_1_3:
      return 0x103;
    case CG_PROFILE_GLSLF:
    case CG_PROFILE_GLSLC:
      // Actually GLSL can be anything from 0x200 upwards...
    case CG_PROFILE_ARBFP1:
      return 0x200;
    case CG_PROFILE_FP30:
      return 0x20a;
    case CG_PROFILE_FP40:
      return 0x300;
    case CG_PROFILE_GPU_FP:
      return 0x400;
    default:
      // Some "future proofing"
      return (profile > CG_PROFILE_GPU_FP) ? 0x500 : 0;
  }
}

void csGLShader_CG::GetProfileCompilerArgs (const char* type, 
                                            CGprofile profile,
                                            const ProfileLimitsPair& limitsPair,
                                            HardwareVendor vendor,
                                            uint argsMask,
                                            ArgumentArray& args)
{
  csString profileStr (cgGetProfileString (profile));
  if (!(argsMask & argsNoConfig))
  {
    csConfigAccess cfg (object_reg);
    csString key ("Video.OpenGL.Shader.Cg.CompilerOptions");
    SplitArgsString (cfg->GetStr (key), args);
    key << "." << type;
    SplitArgsString (cfg->GetStr (key), args);
    key << "." << profileStr;
    SplitArgsString (cfg->GetStr (key), args);
  }

  if (!(argsMask & argsNoProfileLimits))
  {
    profileStr.Upcase();
    csString profileMacroArg ("-DPROFILE_");
    profileMacroArg += profileStr;
    args.Push (profileMacroArg);
    
    profileStr = cgGetProfileString (limitsPair.vp.profile);
    if (!profileStr.IsEmpty())
    {
      profileStr.Upcase();
      profileMacroArg = "-DVERT_PROFILE_";
      profileMacroArg += profileStr;
      args.Push (profileMacroArg);
    }
    
    profileStr = cgGetProfileString (limitsPair.fp.profile);
    if (!profileStr.IsEmpty())
    {
      profileStr.Upcase();
      profileMacroArg = "-DFRAG_PROFILE_";
      profileMacroArg += profileStr;
      args.Push (profileMacroArg);
    }
    
    int profileLevel = GetProfileLevel (limitsPair.fp.profile);
    if (profileLevel != 0)
    {
      profileMacroArg.Format ("-DFRAGMENT_PROGRAM_LEVEL=0x%x", profileLevel);
      args.Push (profileMacroArg);
    }
  
    if (vendor != Invalid)
    {
      csString vendorStr;
      switch (vendor)
      {
	case ATI:     vendorStr = "ATI"; break;
	case NVIDIA:  vendorStr = "NVIDIA"; break;
	case Other:   vendorStr = "OTHER"; break;
	default:      CS_ASSERT(false);
      }
      vendorStr = "-DVENDOR_" + vendorStr;
      args.Push (vendorStr);
    }
  }
  
  if (!(argsMask & argsNoProgramType))
  {
    csString typeStr (type);
    typeStr.Upcase();
    typeStr = "-DPROGRAM_TYPE_" + typeStr;
    args.Push (typeStr);
  }
}

////////////////////////////////////////////////////////////////////
//                      iShaderProgramPlugin
////////////////////////////////////////////////////////////////////
bool csGLShader_CG::SupportType(const char* type)
{
  if (!Open())
    return false;
  if (!enable)
    return false;
  if( strcasecmp(type, "vp") == 0)
    return true;
  if( strcasecmp(type, "fp") == 0)
    return true;
  return false;
}

void csGLShader_CG::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csReportV (object_reg, severity,
    "crystalspace.graphics3d.shader.glcg", msg, arg);
  va_end (arg);
}

csPtr<iShaderProgram> csGLShader_CG::CreateProgram (const char* type)
{
  Open();
  if (!enable) return 0;
  
  if (strcasecmp(type, "vp") == 0)
    return csPtr<iShaderProgram> (new csShaderGLCGVP(this));
  else if (strcasecmp(type, "fp") == 0)
  {
    return csPtr<iShaderProgram> (new csShaderGLCGFP (this,
      currentLimits));
  }
  else
    return 0;
}

static CGprofile NextBestFakeProfile (CGprofile profile)
{
  switch (profile)
  {
    case CG_PROFILE_PS_1_1:
      return CG_PROFILE_FP20;
    case CG_PROFILE_PS_1_2:
    case CG_PROFILE_PS_1_3:
      return CG_PROFILE_FP30;
      
    case CG_PROFILE_FP20:
      return CG_PROFILE_PS_1_1;
    case CG_PROFILE_FP30:
    case CG_PROFILE_FP40:
    case CG_PROFILE_GPU_FP:
      return CG_PROFILE_ARBFP1;
    
    case CG_PROFILE_VP30:
    case CG_PROFILE_VP40:
    case CG_PROFILE_GPU_VP:
      return CG_PROFILE_ARBVP1;
    default:
      return CG_PROFILE_UNKNOWN;
  }
}

void csGLShader_CG::ParsePrecacheLimits (iConfigFile* config, 
                                         ProfileLimitsArray& out)
{
  csSet<csString> seenKeys;
  csRef<iConfigIterator> vertexPrecache = config->Enumerate (
    csString().Format ("Video.OpenGL.Shader.Cg.Precache."));
  while (vertexPrecache->HasNext())
  {
    vertexPrecache->Next();
    csString key (vertexPrecache->GetKey (true));
    size_t dot = key.FindFirst ('.');
    if (dot != (size_t)-1) key.Truncate (dot);
    if (!seenKeys.Contains (key))
    {
      ProfileLimitsPair newPair;
      const char* profile = config->GetStr (
	csString().Format ("Video.OpenGL.Shader.Cg.Precache.%s.Vertex.Profile",
	  key.GetData()), 0);
      if (profile != 0)
      {
	CGprofile profile_cg = cgGetProfile (profile);
	if (profile_cg != CG_PROFILE_UNKNOWN)
	{
	  newPair.vp = ProfileLimits (
	   Other, profile_cg);
	  newPair.vp.GetCgDefaults();
	  newPair.vp.ReadFromConfig (config, 
	    csString().Format ("Video.OpenGL.Shader.Cg.Precache.%s.Vertex",
	      key.GetData()));
	}
      }
      profile = config->GetStr (
	csString().Format ("Video.OpenGL.Shader.Cg.Precache.%s.Fragment.Profile",
	  key.GetData()), 0);
      if (profile != 0)
      {
	CGprofile profile_cg = cgGetProfile (profile);
	if (profile_cg != CG_PROFILE_UNKNOWN)
	{
	  newPair.fp = ProfileLimits (Other, profile_cg);
	  newPair.fp.GetCgDefaults();
	  newPair.fp.ReadFromConfig (config, 
	    csString().Format ("Video.OpenGL.Shader.Cg.Precache.%s.Fragment",
	      key.GetData()));
	}
      }
      out.Push (newPair);
      seenKeys.AddNoTest (key);
    }
  }
  /* Dirty trick: put highest profile last; most likely to compile; in the
     case of VPs means bindings are correct.
   */
  out.Sort();
}

bool csGLShader_CG::Open()
{
  if (isOpen) return true;
  if (!object_reg) return false;
  
  csRef<iConfigManager> config (csQueryRegistry<iConfigManager> (object_reg));

  ParsePrecacheLimits (config, precacheLimits);
  
  debugDump = config->GetBool ("Video.OpenGL.Shader.Cg.DebugDump", false);
  if (debugDump)
    dumpDir = CS::StrDup (config->GetStr ("Video.OpenGL.Shader.Cg.DebugDumpDir",
    "/tmp/cgdump/"));
    
  cgSetAutoCompile (context, CG_COMPILE_MANUAL);
  
  cgSetErrorHandler (ErrorHandler, this);

  if (!CS::PluginCommon::ShaderProgramPluginGL::Open ())
    return true;

  enable = true;

  ext->InitGL_ARB_color_buffer_float();
  ext->InitGL_ARB_vertex_program();
  ext->InitGL_ARB_fragment_program();
  ext->InitGL_NV_gpu_program4();
  ext->InitGL_EXT_gpu_program_parameters();

  enableVP = config->GetBool ("Video.OpenGL.Shader.Cg.Enable.Vertex", true);
  enableFP = config->GetBool ("Video.OpenGL.Shader.Cg.Enable.Fragment", true);

  bool forceBestProfile =
    config->GetBool ("Video.OpenGL.Shader.Cg.ForceBestProfile", false);

  strictMatchVP = false;
  if (enableVP)
  {
    if (config->KeyExists ("Video.OpenGL.Shader.Cg.Fake.Vertex.Profile"))
    {
      const char* profileStr = 
        config->GetStr ("Video.OpenGL.Shader.Cg.Fake.Vertex.Profile");
      CGprofile profile = cgGetProfile (profileStr);
      /* @@@ Hack: Make sure at least ARB_v_p is used.
       * This is done because we don't completely support NV_vertex_program based
       * profiles - those require "manual" binding of state matrices via 
       * glTrackMatrixNV() which we don't support right now.
       */
      if (profile == CG_PROFILE_UNKNOWN)
      {
        if (doVerbose)
          Report (CS_REPORTER_SEVERITY_WARNING,
              "Unknown fake vertex program profile %s", CS::Quote::Single (profileStr));
      }
      else
      {
        if (profile < CG_PROFILE_ARBVP1) profile = CG_PROFILE_ARBVP1;
        CGprofile profile2;
        if (cgGLIsProfileSupported (profile2 = profile)
            || cgGLIsProfileSupported (profile2 = NextBestFakeProfile (profile)))
        {
	  ProfileLimits limits (vendor, profile2);
	  limits.ReadFromConfig (config, "Video.OpenGL.Shader.Cg.Fake.Vertex");
	  currentLimits.vp = limits;
          strictMatchVP = true;
	}
	else
	{
	  if (doVerbose)
	    Report (CS_REPORTER_SEVERITY_WARNING,
	      "Fake vertex program profile %s unsupported",
	      CS::Quote::Single (cgGetProfileString (profile)));
	}
      }
    }
    if (currentLimits.vp.profile == CG_PROFILE_UNKNOWN)
    {
      CGprofile profile = cgGLGetLatestProfile (CG_GL_VERTEX);
      if (profile < CG_PROFILE_ARBVP1)
      {
        if (cgGLIsProfileSupported (CG_PROFILE_ARBVP1))
          profile = CG_PROFILE_ARBVP1;
        else
          profile = CG_PROFILE_UNKNOWN;
      }
      ProfileLimits limits (vendor, profile);
      limits.GetCurrentLimits (ext);
      currentLimits.vp = limits;
      strictMatchVP = forceBestProfile;
    }
    enableVP = currentLimits.vp.profile != CG_PROFILE_UNKNOWN;
    if (doVerbose)
    {
      if (enableVP)
	Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "Using vertex program limits: %s",
	  currentLimits.vp.ToStringForPunyHumans().GetData());
      else
	Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "Vertex programs not supported");
    }
  }
  else
  {
    if (doVerbose)
      Report (CS_REPORTER_SEVERITY_NOTIFY,
          "Vertex program support disabled by user");
  }

  strictMatchFP = false;
  if (enableFP)
  {
    psplg = csLoadPluginCheck<iShaderProgramPlugin> (object_reg,
      "crystalspace.graphics3d.shader.glps1", false);
    if(!psplg)
    {
      if (doVerbose)
        Report (CS_REPORTER_SEVERITY_WARNING,
            "Could not find crystalspace.graphics3d.shader.glps1. Cg to PS "
            "routing unavailable.");
    }

    if (config->KeyExists ("Video.OpenGL.Shader.Cg.Fake.Fragment.Profile"))
    {
      const char* profileStr = 
        config->GetStr ("Video.OpenGL.Shader.Cg.Fake.Fragment.Profile");
      CGprofile profile = cgGetProfile (profileStr);
      if (profile == CG_PROFILE_UNKNOWN)
      {
        if (doVerbose)
          Report (CS_REPORTER_SEVERITY_WARNING,
              "Unknown fake fragment program profile %s", CS::Quote::Single (profileStr));
      }
      else
      {
        CGprofile profile2;
	// Check if hardware supports requested profile, directly or by routing
        if (cgGLIsProfileSupported (profile2 = profile)
            || IsRoutedProfileSupported (profile2))
        {
	  ProfileLimits limits (vendor, profile2);
	  limits.ReadFromConfig (config, "Video.OpenGL.Shader.Cg.Fake.Fragment");
	  currentLimits.fp = limits;
          strictMatchFP = true;
	}
	// Check if hardware supports a profile "close enough" to the requested one,
	// directly or by routing
	else if (cgGLIsProfileSupported (profile2 = NextBestFakeProfile (profile))
            || IsRoutedProfileSupported (profile2))
	{
	  ProfileLimits limits (
	    CS::PluginCommon::ShaderProgramPluginGL::Other,
	    profile2);
	  limits.GetCurrentLimits (ext);
	  currentLimits.fp = limits;
          strictMatchFP = true;
	}
	else
	{
	  if (doVerbose)
	    Report (CS_REPORTER_SEVERITY_WARNING,
	      "Fake fragment program profile %s unsupported", CS::Quote::Single (profileStr));
	}
      }
    }
    if (currentLimits.fp.profile == CG_PROFILE_UNKNOWN)
    {
      CGprofile latestProfile = cgGLGetLatestProfile (CG_GL_FRAGMENT);
      if (((latestProfile == CG_PROFILE_FP20) /* (1) below */
	  || (latestProfile == CG_PROFILE_UNKNOWN)) /* (2) below */
	&& psplg.IsValid())
      {
        /* (1) Use CG_PROFILE_PS_1_1 or CG_PROFILE_PS_1_3 instead of FP20.
           NVidia's FP20 backend apparently doesn't handle constants that are
           needed in two different combiner stages very well - it seems to
           only set one of the constants. However, the PS1.x code generated
           by Cg is semantically correct; our PS1.x 'emulation' plugin correctly
           handles the required constant mapping, so prefer indirection through
           PS1.x even if the hardware could support FP20 natively. 
	   (2) If there is no Cg-supported profile we might still support a
	   routed PS1.x profile.
	   */
        if (IsRoutedProfileSupported (CG_PROFILE_PS_1_3))
          latestProfile = CG_PROFILE_PS_1_3;
        else if (IsRoutedProfileSupported (CG_PROFILE_PS_1_1))
          latestProfile = CG_PROFILE_PS_1_1;
      }
      ProfileLimits limits (vendor, latestProfile);
      limits.GetCurrentLimits (ext);
      currentLimits.fp = limits;
      strictMatchFP = forceBestProfile;
    }
    enableFP = currentLimits.fp.profile != CG_PROFILE_UNKNOWN;
    if (doVerbose)
    {
      if (enableFP)
	Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "Using fragment program limits: %s",
	  currentLimits.fp.ToStringForPunyHumans().GetData());
      else
	Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "Fragment programs not supported");
    }
  }
  else
  {
    if (doVerbose)
      Report (CS_REPORTER_SEVERITY_NOTIFY,
          "Fragment program support disabled by user");
  }

  cgGLSetDebugMode (config->GetBool ("Video.OpenGL.Shader.Cg.CgDebugMode",
    false));
 
  return true;
}

bool csGLShader_CG::IsRoutedProfileSupported (CGprofile profile)
{
  ext->InitGL_ATI_fragment_shader();
  ext->InitGL_NV_texture_shader();
  ext->InitGL_NV_texture_shader3();
  switch (profile)
  {
    case CG_PROFILE_PS_1_1:
      return ext->CS_GL_ATI_fragment_shader || ext->CS_GL_NV_texture_shader;
    case CG_PROFILE_PS_1_2:
    case CG_PROFILE_PS_1_3:
      return ext->CS_GL_ATI_fragment_shader || ext->CS_GL_NV_texture_shader3;
    default:
      return false;
  }
}

csPtr<iStringArray> csGLShader_CG::QueryPrecacheTags (const char* type)
{
  if (!Open()) return (iStringArray*)nullptr;
  
  scfStringArray* tags = new scfStringArray;
  for (size_t i = 0; i < precacheLimits.GetSize(); i++)
  {
    tags->Push (csString("CG") + precacheLimits[i].ToString());
  }
  return csPtr<iStringArray> (tags);
}

bool csGLShader_CG::Precache (const char* type, const char* tag,
                              iBase* previous,
                              iDocumentNode* node, iHierarchicalCache* cacheTo,
                              csRef<iBase>* outObj)
{
  if (!Open()) return false;
  
  csRef<iShaderProgramCG> prevCG (scfQueryInterfaceSafe<iShaderProgramCG> (
    previous));
  csRef<iShaderDestinationResolver> resolve (
    scfQueryInterfaceSafe<iShaderDestinationResolver> (previous));
  
  if ((tag == 0) || (tag[0] != 'C') || (tag[1] != 'G'))
    return false;
  tag += 2;

  ProfileLimitsPair limits;
  if (!limits.FromString (tag))
    return false;
  csRef<csShaderGLCGCommon> prog;
  bool result = false;
  if (strcasecmp(type, "fp") == 0)
  {
    prog.AttachNew (new csShaderGLCGFP (this, limits));
    /* Precache for 'tag' */
    if (!prog->Load (resolve, node)) return false;
    result = Precache (prog, limits, tag, cacheTo);
  }
  else if (strcasecmp(type, "vp") == 0)
  {
    prog.AttachNew (new csShaderGLCGVP (this));
    /* Get tag from prevCG */
    if (prevCG.IsValid())
    {
      /* If previous tag and current tag match, precache */
      csShaderGLCGFP* prevFP = static_cast<csShaderGLCGFP*> (
        (iShaderProgramCG*)prevCG);
      ProfileLimitsPair prevLimits = prevFP->cacheLimits;
      if (limits != prevLimits) return false;
      if (!prevFP->IsValid()) return false;
    }
    /* Precache for 'tag' */
    if (!prog->Load (resolve, node)) return false;
    result = Precache (prog, limits, tag, cacheTo);
  }
  else
    return false;

  if (outObj) *outObj = prog;
  return result;

}

bool csGLShader_CG::Precache (csShaderGLCGCommon* prog, 
                              ProfileLimitsPair& limits,
                              const char* tag,
                              iHierarchicalCache* cacheTo)
{
  bool result = false;
  result = prog->Precache (limits, tag, cacheTo);
  return result;
}

////////////////////////////////////////////////////////////////////
//                          iComponent
////////////////////////////////////////////////////////////////////
bool csGLShader_CG::Initialize(iObjectRegistry* reg)
{
  if (!CS::PluginCommon::ShaderProgramPluginGL::Initialize (reg))
    return false;
    
  csRef<iPluginManager> plugin_mgr = 
    csQueryRegistry<iPluginManager> (object_reg);

  binDocSys = csLoadPluginCheck<iDocumentSystem> (plugin_mgr,
    "crystalspace.documentsystem.binary");
  xmlDocSys.AttachNew (new csTinyDocumentSystem);
  
  csRef<iShaderManager> shaderMgr =
    csQueryRegistry<iShaderManager> (object_reg);
  if (!shaderMgr.IsValid())
  {
    Report (CS_REPORTER_SEVERITY_WARNING,
      "Could not obtain iShaderManager");
  }
  else
  {
    iHierarchicalCache* shaderCache = shaderMgr->GetShaderCache();
    if (shaderCache != 0)
    {
      csRef<iHierarchicalCache> progCache =
        shaderCache->GetRootedCache ("/CgProgCache");
      stringStore = new StringStore (progCache);
      this->progCache.SetCache (progCache);
    }
  }
  
  return true;
}

}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)

