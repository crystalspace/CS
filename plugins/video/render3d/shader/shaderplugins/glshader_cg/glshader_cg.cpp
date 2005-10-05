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

iObjectRegistry* csGLShader_CG::object_reg = 0;
CGcontext csGLShader_CG::context;

CS_LEAKGUARD_IMPLEMENT (csGLShader_CG);

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csGLShader_CG)

SCF_IMPLEMENT_IBASE(csGLShader_CG)
  SCF_IMPLEMENTS_INTERFACE(iShaderProgramPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGLShader_CG::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END


csGLShader_CG::csGLShader_CG(iBase* parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);

  context = cgCreateContext ();
  cgSetErrorCallback (ErrorCallback);

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
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

void csGLShader_CG::ErrorCallback ()
{
  bool doVerbose;
  csRef<iVerbosityManager> verbosemgr (
    CS_QUERY_REGISTRY (object_reg, iVerbosityManager));
  if (verbosemgr) 
    doVerbose = verbosemgr->Enabled ("renderer.shader");
  else
    doVerbose = false;
  if (!doVerbose) return;

  CGerror error = cgGetError();
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

  csRef<iGraphics3D> r = CS_QUERY_REGISTRY(object_reg,iGraphics3D);

  csRef<iFactory> f = SCF_QUERY_INTERFACE (r, iFactory);
  if (f != 0 && strcmp ("crystalspace.graphics3d.opengl", 
	f->QueryClassID ()) == 0)
    enable = true;
  else
    return false;

  r->GetDriver2D()->PerformExtension ("getextmanager", &ext);
  if (ext == 0)
  {
    enable = false;
    return false;
  }

  bool route = false;
  csRef<iConfigManager> config(CS_QUERY_REGISTRY (object_reg, iConfigManager));
  if (config->KeyExists ("Video.OpenGL.Shader.Cg.PSRouting"))
  {
    route = config->GetBool ("Video.OpenGL.Shader.Cg.PSRouting");
    if (doVerbose)
      Report (CS_REPORTER_SEVERITY_NOTIFY,
        "Routing Cg fragment programs to Pixel Shader plugin %s (forced).", 
        route ? "ON" : "OFF");
  }
  else
  {
    ext->InitGL_ARB_fragment_program ();
    ext->InitGL_ATI_fragment_shader ();

    // If the hardware's got ATI_f_p, but isn't new enough to have ARB_f_p
    // we default to routing Cg fragment programs to the Pixel Shader plugin
    route = !ext->CS_GL_ARB_fragment_program && 
            ext->CS_GL_ATI_fragment_shader;
    if (doVerbose)
      Report (CS_REPORTER_SEVERITY_NOTIFY,
        "Routing Cg fragment programs to Pixel Shader plugin %s (default).", 
        route ? "ON" : "OFF");
  }
  ext->InitGL_ARB_vertex_program ();

  debugDump = config->GetBool ("Video.OpenGL.Shader.Cg.DebugDump", false);
  if (debugDump)
    dumpDir = csStrNew (config->GetStr ("Video.OpenGL.Shader.Cg.DebugDumpDir",
    "/tmp/cgdump/"));

  csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY (object_reg,
    iPluginManager);
  if (route)
  {
    psplg = CS_QUERY_PLUGIN_CLASS(plugin_mgr, 
      "crystalspace.graphics3d.shader.glps1", iShaderProgramPlugin);
    if(!psplg)
    {
      psplg = CS_LOAD_PLUGIN(plugin_mgr, 
        "crystalspace.graphics3d.shader.glps1", iShaderProgramPlugin);
      if (!psplg)
      {
        if (doVerbose)
          Report (CS_REPORTER_SEVERITY_WARNING,
            "Could not find crystalspace.graphics3d.shader.glps1. Cg to PS "
            "routing unavailable.");
      }
    }
  }
  // Check which FP profile to use...
  if (psplg)
  {
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
    else
    {
      if (doVerbose)
        Report (CS_REPORTER_SEVERITY_WARNING,
          "Cg to PS routing disabled due lack of hardware support.");
      psplg = 0;
    }
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
