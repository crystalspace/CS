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
          if (!s.IsEmpty()) args.Push (str);
          args.Empty();
        }
        // else fall through
      default:
        s.Append (*p);
    }
    p++;
  }
  if (!s.IsEmpty()) args.Push (s);
}

void csGLShader_CG::GetProfileCompilerArgs (const char* type, 
                                            CGprofile profile, 
                                            ArgumentArray& args)
{
  csConfigAccess cfg (object_reg);
  csString key ("Video.OpenGL.Shader.Cg.CompilerOptions");
  SplitArgsString (cfg->GetStr (key), args);
  key << "." << type;
  SplitArgsString (cfg->GetStr (key), args);
  key << "." << cgGetProfileString (profile);
  SplitArgsString (cfg->GetStr (key), args);
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

  debugDump = config->GetBool ("Video.OpenGL.Shader.Cg.DebugDump", false);
  if (debugDump)
    dumpDir = csStrNew (config->GetStr ("Video.OpenGL.Shader.Cg.DebugDumpDir",
    "/tmp/cgdump/"));

  psProfile = CG_PROFILE_UNKNOWN;
  // Check which FP profile to use...
  ext->InitGL_ARB_fragment_program ();
  if (ext->CS_GL_ARB_fragment_program)
  {
    // if AFP is supported, higher profiles are prolly supported as well
    psProfile = cgGLGetLatestProfile (CG_GL_FRAGMENT);
  }
  else
  {
    // Non-AFP is always routed through the PS1 plugin
    ext->InitGL_ATI_fragment_shader ();
    ext->InitGL_NV_texture_shader ();
    ext->InitGL_NV_texture_shader2 ();
    if (ext->CS_GL_ATI_fragment_shader)
    {
      psProfile = CG_PROFILE_PS_1_3;
    }
    else if (ext->CS_GL_NV_texture_shader && ext->CS_GL_NV_texture_shader2)
    {
      // @@@ Is that logic correct?
      ext->InitGL_NV_register_combiners2 ();
      if (ext->CS_GL_NV_register_combiners2)
      {
        ext->InitGL_NV_texture_shader3 ();
        if (ext->CS_GL_NV_texture_shader3)
        {
          psProfile = CG_PROFILE_PS_1_3;
        }
        else
        {
          psProfile = CG_PROFILE_PS_1_2;
        }
      }
      else
      {
        psProfile = CG_PROFILE_PS_1_1;
      }
    }
  }

  if (psProfile != CG_PROFILE_UNKNOWN)
  {
    // Cap profile
    if (maxProfileFragment != CG_PROFILE_UNKNOWN)
      psProfile = csMin (psProfile, maxProfileFragment);

    // Load PS1 plugin, if requested and/or needed
    bool route = ProfileNeedsRouting (psProfile);
    if (doVerbose)
      Report (CS_REPORTER_SEVERITY_NOTIFY,
        "Routing Cg fragment programs to Pixel Shader plugin %s", 
        route ? "ON" : "OFF");
    if (route)
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
    }
  }
  else
  {
    if (doVerbose)
      Report (CS_REPORTER_SEVERITY_WARNING,
        "Cg fragment programs unavailable due lack of hardware support.");
  }

  isOpen = true;
  return true;
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

