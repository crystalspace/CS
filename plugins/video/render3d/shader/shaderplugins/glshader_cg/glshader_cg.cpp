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
#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"
#include "glshader_cgvp.h"
#include "glshader_cgfp.h"
#include "glshader_cg.h"

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{

CS_LEAKGUARD_IMPLEMENT (csGLShader_CG);

SCF_IMPLEMENT_FACTORY (csGLShader_CG)

csGLShader_CG::csGLShader_CG (iBase* parent) : 
  scfImplementationType (this, parent)
{
  context = cgCreateContext ();
  cgSetErrorHandler (ErrorHandler, 0);

  enable = false;
  isOpen = false;
  debugDump = false;
  dumpDir = 0;
  ext = 0;
}

csGLShader_CG::~csGLShader_CG()
{
  delete[] dumpDir;
  cgDestroyContext (context);
}

void csGLShader_CG::ErrorHandler (CGcontext context, CGerror error, 
				  void* appData)
{
  iObjectRegistry* object_reg = (iObjectRegistry*)appData;
  bool doVerbose;
  csRef<iVerbosityManager> verbosemgr (
    CS_QUERY_REGISTRY (object_reg, iVerbosityManager));
  if (verbosemgr) 
    doVerbose = verbosemgr->Enabled ("renderer.shader");
  else
    doVerbose = false;
  if (!doVerbose) return;

  csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
    "crystalspace.graphics3d.shader.glcg",
    "%s", cgGetErrorString (error));
  if (error == CG_COMPILER_ERROR)
  {
    const char* listing = cgGetLastListing (context);
    if (listing)
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
    case CG_PROFILE_ARBFP1:
      return 0x200;
    case CG_PROFILE_FP30:
      return 0x20a;
    case CG_PROFILE_FP40:
      return 0x300;
    default:
      return 0;
  }
}

void csGLShader_CG::GetProfileCompilerArgs (const char* type, 
                                            CGprofile profile, 
                                            ArgumentArray& args)
{
  csString profileStr (cgGetProfileString (profile));
  csConfigAccess cfg (object_reg);
  csString key ("Video.OpenGL.Shader.Cg.CompilerOptions");
  SplitArgsString (cfg->GetStr (key), args);
  key << "." << type;
  SplitArgsString (cfg->GetStr (key), args);
  key << "." << profileStr;
  SplitArgsString (cfg->GetStr (key), args);

  profileStr.Upcase();
  csString profileMacroArg ("-DPROFILE_");
  profileMacroArg += profileStr;
  args.Push (profileMacroArg);
  int profileLevel = GetProfileLevel (profile);
  if (profileLevel != 0)
  {
    profileMacroArg.Format ("-DFRAGMENT_PROGRAM_LEVEL=0x%x", profileLevel);
    args.Push (profileMacroArg);
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

csPtr<iShaderProgram> csGLShader_CG::CreateProgram(const char* type)
{
  if (!Open())
    return 0;
  if (strcasecmp(type, "vp") == 0)
    return csPtr<iShaderProgram>(new csShaderGLCGVP(this));
  else if (strcasecmp(type, "fp") == 0)
    return csPtr<iShaderProgram>(new csShaderGLCGFP(this));
  else
    return 0;
}

static CGprofile ProfileRouted (CGprofile profile)
{
  switch (profile)
  {
    case CG_PROFILE_FP20:
      return CG_PROFILE_PS_1_1;
    default:
      return profile;
  }
}

static CGprofile ProfileUnrouted (CGprofile profile)
{
  switch (profile)
  {
    case CG_PROFILE_PS_1_1:
      return CG_PROFILE_FP20;
    case CG_PROFILE_PS_1_2:
    case CG_PROFILE_PS_1_3:
      return CG_PROFILE_FP30;
    default:
      return profile;
  }
}

bool csGLShader_CG::Open()
{
  if (isOpen) return true;
  if (!object_reg) return false;

  cgSetErrorHandler (ErrorHandler, object_reg);

  csRef<iGraphics3D> r = CS_QUERY_REGISTRY(object_reg,iGraphics3D);

  csRef<iFactory> f = SCF_QUERY_INTERFACE (r, iFactory);
  if (f != 0 && strcmp ("crystalspace.graphics3d.opengl", 
	f->QueryClassID ()) == 0)
    enable = true;
  else
    return false;

  csRef<iConfigManager> config(CS_QUERY_REGISTRY (object_reg, iConfigManager));

  r->GetDriver2D()->PerformExtension ("getextmanager", &ext);
  if (ext == 0)
  {
    enable = false;
    return false;
  }

  enableVP = config->GetBool ("Video.OpenGL.Shader.Cg.Enable.Vertex", true);
  enableFP = config->GetBool ("Video.OpenGL.Shader.Cg.Enable.Fragment", true);

  if (enableVP)
  {
    if (config->KeyExists ("Video.OpenGL.Shader.Cg.MaxProfile.Vertex"))
    {
      const char* profileStr = 
        config->GetStr ("Video.OpenGL.Shader.Cg.MaxProfile.Vertex");
      CGprofile profile = cgGetProfile (profileStr);
      if (profile == CG_PROFILE_UNKNOWN)
      {
        if (doVerbose)
          Report (CS_REPORTER_SEVERITY_WARNING,
              "Unknown maximum vertex program profile '%s'", profileStr);
      }
      else
      {
        maxProfileVertex = profile;
        if (doVerbose)
          Report (CS_REPORTER_SEVERITY_NOTIFY,
            "Maximum vertex program profile: %s", profileStr);
      }
    }
    else
      maxProfileVertex = CG_PROFILE_UNKNOWN;
  }
  else
  {
    if (doVerbose)
      Report (CS_REPORTER_SEVERITY_NOTIFY,
          "Vertex program support disabled by user");
  }

  if (enableFP)
  {
    if (config->KeyExists ("Video.OpenGL.Shader.Cg.MaxProfile.Fragment"))
    {
      const char* profileStr = 
        config->GetStr ("Video.OpenGL.Shader.Cg.MaxProfile.Fragment");
      CGprofile profile = cgGetProfile (profileStr);
      if (profile == CG_PROFILE_UNKNOWN)
      {
        if (doVerbose)
          Report (CS_REPORTER_SEVERITY_WARNING,
              "Unknown maximum fragment program profile '%s'", profileStr);
      }
      else
      {
        maxProfileFragment = profile;
        if (doVerbose)
          Report (CS_REPORTER_SEVERITY_NOTIFY,
            "Maximum fragment program profile: %s", profileStr);
      }
    }
    else
      maxProfileFragment = CG_PROFILE_UNKNOWN;
  }
  else
  {
    if (doVerbose)
      Report (CS_REPORTER_SEVERITY_NOTIFY,
          "Fragment program support disabled by user");
  }

  debugDump = config->GetBool ("Video.OpenGL.Shader.Cg.DebugDump", false);
  if (debugDump)
    dumpDir = csStrNew (config->GetStr ("Video.OpenGL.Shader.Cg.DebugDumpDir",
    "/tmp/cgdump/"));
 
  // Determining what profile to use:
  //  Start off with the highest supported profile.
  psProfile = cgGLGetLatestProfile (CG_GL_FRAGMENT);
  //  Cap at the maximum profile
  if ((maxProfileFragment != CG_PROFILE_UNKNOWN) 
    && (GetProfileLevel (psProfile) > GetProfileLevel (maxProfileFragment)))
  {
    if (!ProfileNeedsRouting (maxProfileFragment))
    {
      //  maxProfileFragment is GL profile.
      //  maxProfileFragment supported natively?
      if (cgGLIsProfileSupported (maxProfileFragment))
        psProfile = maxProfileFragment;
      //  maxProfileFragment supported by routing?
      else if (IsRoutedProfileSupported (ProfileRouted (maxProfileFragment)))
        psProfile = ProfileRouted (maxProfileFragment);
      else
        //  Can't support max profile.
        psProfile = CG_PROFILE_UNKNOWN;
    }
    else 
    {
      //  maxProfileFragment is DX profile.
      //  maxProfileFragment supported via routing?
      if (IsRoutedProfileSupported (maxProfileFragment))
        psProfile = maxProfileFragment;
      //  GL equivalent of maxProfileFragment supported?
      else if (cgGLIsProfileSupported (ProfileUnrouted (maxProfileFragment)))
        psProfile = ProfileUnrouted (maxProfileFragment);
      else
        //  Can't support max profile.
        psProfile = CG_PROFILE_UNKNOWN;
    }
  }

  // Check if the requested profile needs routing.
  bool doRoute = ProfileNeedsRouting (psProfile);

  if (enableFP)
  {
    if (psProfile != CG_PROFILE_UNKNOWN)
    {
      // Load PS1 plugin, if requested and/or needed
      if (doVerbose)
        Report (CS_REPORTER_SEVERITY_NOTIFY,
          "Routing Cg fragment programs to Pixel Shader plugin %s", 
          doRoute ? "ON" : "OFF");
      if (doRoute)
      {
        psplg = csLoadPluginCheck<iShaderProgramPlugin> (object_reg,
          "crystalspace.graphics3d.shader.glps1", false);
        if(!psplg)
        {
          if (doVerbose)
            Report (CS_REPORTER_SEVERITY_WARNING,
                "Could not find crystalspace.graphics3d.shader.glps1. Cg to PS "
                "routing unavailable.");
          psProfile = cgGLGetLatestProfile (CG_GL_FRAGMENT);
        }
      }
    }
    else
    {
      if (doVerbose)
        Report (CS_REPORTER_SEVERITY_WARNING,
          "Cg fragment programs unavailable due lack of hardware support.");
      enableFP = false;
    }
  }

  isOpen = true;
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

////////////////////////////////////////////////////////////////////
//                          iComponent
////////////////////////////////////////////////////////////////////
bool csGLShader_CG::Initialize(iObjectRegistry* reg)
{
  object_reg = reg;
  csRef<iVerbosityManager> verbosemgr (
    CS_QUERY_REGISTRY (object_reg, iVerbosityManager));
  if (verbosemgr) 
    doVerbose = verbosemgr->Enabled ("renderer.shader");
  else
    doVerbose = false;
  return true;
}

}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)

