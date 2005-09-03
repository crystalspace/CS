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
#include "csplugincommon/opengl/glextmanager.h"
#include "csplugincommon/opengl/glstates.h"
#include "csplugincommon/shader/shaderplugin.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"
#include "glshader_ffp.h"
#include "glshader_fvp.h"
#include "glshader_fixed.h"

CS_LEAKGUARD_IMPLEMENT (csGLShader_FIXED);

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csGLShader_FIXED)

SCF_IMPLEMENT_IBASE(csGLShader_FIXED)
  SCF_IMPLEMENTS_INTERFACE(iShaderProgramPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGLShader_FIXED::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END


csGLShader_FIXED::csGLShader_FIXED(iBase* parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);

  enable = false;
  enableCombine = false;
  texUnits = 0;
  isOpen = false;
}

csGLShader_FIXED::~csGLShader_FIXED()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

void csGLShader_FIXED::Report (int severity, const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  csReportV (object_reg, severity, "crystalspace.graphics3d.shader.fixed", 
    msg, args);
  va_end (args);
}

////////////////////////////////////////////////////////////////////
//                      iShaderProgramPlugin
////////////////////////////////////////////////////////////////////
bool csGLShader_FIXED::SupportType(const char* type)
{
  Open ();
  if (!enable)
    return false;
  if (strcasecmp(type, "fp") == 0)
    return true;
  else if( strcasecmp(type, "vp") == 0)
    return true;
  return false;
}

csPtr<iShaderProgram> csGLShader_FIXED::CreateProgram(const char* type)
{
  if (!enable)
    return 0;
  if( strcasecmp(type, "fp") == 0)
    return csPtr<iShaderProgram>(new csGLShaderFFP (this));
  else if( strcasecmp(type, "vp") == 0)
    return csPtr<iShaderProgram>(new csGLShaderFVP (this));
  else
    return 0;
}

#define FUNNY_TEXTURE_UNIT_COUNT

void csGLShader_FIXED::Open()
{
  if(!object_reg)
    return;
  if (isOpen) return;

  config.AddConfig (object_reg, "/config/glshader_fixed.cfg");

  if (!(enable && ext)) return;

  ext->InitGL_ARB_multitexture();

  if (ext->CS_GL_ARB_multitexture)
  {
    ext->InitGL_ARB_texture_env_combine ();
    if (!ext->CS_GL_ARB_texture_env_combine)
      ext->InitGL_EXT_texture_env_combine ();
    ext->InitGL_ARB_texture_env_dot3 ();
    if (!ext->CS_GL_ARB_texture_env_dot3)
      ext->InitGL_EXT_texture_env_dot3 ();
    glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &texUnits);

    enableCombine = ext->CS_GL_ARB_multitexture && 
      (ext->CS_GL_ARB_texture_env_combine || 
       ext->CS_GL_EXT_texture_env_combine);

  #ifdef FUNNY_TEXTURE_UNIT_COUNT
    const char* descr = 0;
    if (texUnits < 2)
      descr = "unbelievable";
    else if (texUnits == 2)
      descr = "puny";
    else if (texUnits <= 4)
      descr = "moderate";
    else if (texUnits <= 6)
      descr = "acceptable";
    else if (texUnits <= 8)
      descr = "whopping";
    else 
      descr = "unseen before";
    Report (CS_REPORTER_SEVERITY_NOTIFY, "Multitexture units: %s %d",
      descr, texUnits);
  #else
    Report (CS_REPORTER_SEVERITY_NOTIFY, "Multitexture units: %d", texUnits);
  #endif

    int useTextureUnits = 
      config->GetInt ("Video.OpenGL.Shader.Fixed.MaxTextureUnits", texUnits);
    if (useTextureUnits < texUnits)
    {
      Report (CS_REPORTER_SEVERITY_NOTIFY, 
	"Configured to use %d texture units", useTextureUnits);
      texUnits = useTextureUnits;
    }
  }
  
  csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet> (
    object_reg, "crystalspace.shared.stringset");
  lsvCache.SetStrings (strings);

  isOpen = true;
}

////////////////////////////////////////////////////////////////////
//                          iComponent
////////////////////////////////////////////////////////////////////
bool csGLShader_FIXED::Initialize(iObjectRegistry* reg)
{
  object_reg = reg;

  csRef<iGraphics3D> r = CS_QUERY_REGISTRY(object_reg, iGraphics3D);
  csRef<iShaderRenderInterface> sri = SCF_QUERY_INTERFACE(r,
    iShaderRenderInterface);

  csRef<iFactory> f = SCF_QUERY_INTERFACE (r, iFactory);
  if (f != 0 && strcmp ("crystalspace.graphics3d.opengl", 
      f->QueryClassID ()) == 0)
    enable = true;

  ext = 0;
  r->GetDriver2D()->PerformExtension ("getextmanager", &ext);

  return true;
}
