/*
Copyright (C) 2002 by John Harger

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
#include "csplugincommon/opengl/glstates.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"
#include "glshader_ps1.h"
#include "ps1_emu_ati.h"
#include "ps1_emu_nv.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csGLShader_PS1)

SCF_IMPLEMENT_IBASE(csGLShader_PS1)
  SCF_IMPLEMENTS_INTERFACE(iShaderProgramPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGLShader_PS1::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END


csGLShader_PS1::csGLShader_PS1(iBase* parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);

  enable = false;
  isOpen = false;
}

csGLShader_PS1::~csGLShader_PS1()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

void csGLShader_PS1::Report (int severity, const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  csReportV (object_reg, severity, 
    "crystalspace.graphics3d.shader.glps1", msg, args);
  va_end (args);
}

////////////////////////////////////////////////////////////////////
//                      iShaderProgramPlugin
////////////////////////////////////////////////////////////////////
bool csGLShader_PS1::SupportType(const char* type)
{
  if (!enable)
    return false;
  else if (strcasecmp (type, "fp") == 0)
    return true;
  return false;
}

csPtr<iShaderProgram> csGLShader_PS1::CreateProgram (const char* type)
{
  if (strcasecmp (type, "fp") == 0)
  {
    Open();

    if(ext->CS_GL_ATI_fragment_shader)
      return csPtr<iShaderProgram> (new csShaderGLPS1_ATI (this));
    else if(ext->CS_GL_NV_register_combiners)
      return csPtr<iShaderProgram> (new csShaderGLPS1_NV (this));
  }
  return 0;
}

void csGLShader_PS1::Open()
{
  if (isOpen) return;
  if(!object_reg)
    return;

  if (!ext) return;

  csRef<iConfigManager> config (CS_QUERY_REGISTRY (object_reg, iConfigManager));
  ext->InitGL_NV_register_combiners ();
  ext->InitGL_NV_register_combiners2 ();
  ext->InitGL_NV_texture_shader ();
  ext->InitGL_NV_texture_shader2 ();
  ext->InitGL_NV_texture_shader3 ();
  if(ext->CS_GL_NV_texture_shader)
  {
    if (doVerbose)
      Report(CS_REPORTER_SEVERITY_NOTIFY,
    	  "nVidia Texture Shader Extension Supported");
  }
  if(ext->CS_GL_NV_register_combiners)
  {
    if (doVerbose)
    {
      Report(CS_REPORTER_SEVERITY_NOTIFY,
    	  "nVidia Register Combiners Extension Supported");

      GLint num_combiners;
      glGetIntegerv(GL_MAX_GENERAL_COMBINERS_NV, &num_combiners);
      Report(CS_REPORTER_SEVERITY_NOTIFY,
    	  "Max General Combiners: %d", num_combiners);
    }
  }

  ext->InitGL_ATI_fragment_shader ();
  if(ext->CS_GL_ATI_fragment_shader)
  {
    if (doVerbose)
      Report(CS_REPORTER_SEVERITY_NOTIFY,
    	  "ATI Fragment Shader Extension Supported");
  }

  useLists = config->GetBool ("Video.OpenGL.Shader.PS1.UseDisplayLists", true);
  if (doVerbose)
    Report(CS_REPORTER_SEVERITY_NOTIFY,
      "Display list usage %s", useLists ? "enabled" : "disabled");

  isOpen = true;
}

////////////////////////////////////////////////////////////////////
//                          iComponent
////////////////////////////////////////////////////////////////////
bool csGLShader_PS1::Initialize(iObjectRegistry* reg)
{
  object_reg = reg;
  csRef<iGraphics3D> r = CS_QUERY_REGISTRY(object_reg,iGraphics3D);

  csRef<iFactory> f = SCF_QUERY_INTERFACE (r, iFactory);
  if (f != 0 && strcmp ("crystalspace.graphics3d.opengl", 
    f->QueryClassID ()) == 0)
    enable = true;
  else
    return false;

  r->GetDriver2D()->PerformExtension ("getextmanager", &ext);
  if(!ext) return false;
  r->GetDriver2D()->PerformExtension ("getstatecache", &stateCache);
  if(!stateCache) return false;

  csRef<iVerbosityManager> verbosemgr (
    CS_QUERY_REGISTRY (object_reg, iVerbosityManager));
  if (verbosemgr) 
    doVerbose = verbosemgr->Enabled ("renderer.shader");
  else
    doVerbose = false;

  return true;
}

