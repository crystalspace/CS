/*
  Copyright (C) 2011 by Antony Martin

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
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"
#include "iutil/databuff.h"

#include "glshader_glslprogram.h"
#include "glshader_glsl.h"


CS_LEAKGUARD_IMPLEMENT (csGLShader_GLSL);

SCF_IMPLEMENT_FACTORY (csGLShader_GLSL)

csGLShader_GLSL::csGLShader_GLSL(iBase* parent) : 
  scfImplementationType (this, parent), ext (0)
{
  enable = false;
  isOpen = false;
}

csGLShader_GLSL::~csGLShader_GLSL()
{
}

void csGLShader_GLSL::Report (int severity, const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  csReportV (object_reg, severity,
             "crystalspace.graphics3d.shader.glsl", msg, args);
  va_end (args);
}

////////////////////////////////////////////////////////////////////
//                      iShaderProgramPlugin
////////////////////////////////////////////////////////////////////
bool csGLShader_GLSL::SupportType(const char* type)
{
  Open ();
  if (!enable)
    return false;
  if (strcasecmp (type, "shader") == 0)
    return true;
  return false;
}

csPtr<iShaderProgram> csGLShader_GLSL::CreateProgram (const char *type)
{
  Open ();
  if (!enable)
    return 0;

  if (strcasecmp (type, "shader") == 0)
    return csPtr<iShaderProgram> (new csShaderGLSLProgram (this));

  return 0;
}

void csGLShader_GLSL::Open()
{
  if (isOpen) return;
  if(!object_reg)
    return;

  if (ext)
  {
    ext->InitGL_ARB_vertex_shader ();
    ext->InitGL_ARB_fragment_shader ();
    ext->InitGL_ARB_shader_objects ();
    ext->InitGL_ARB_shading_language_100 ();
    ext->InitGL_EXT_geometry_shader4 ();
    if (ext->CS_GL_ARB_vertex_shader &&
        ext->CS_GL_ARB_fragment_shader &&
        ext->CS_GL_ARB_shader_objects &&
        ext->CS_GL_ARB_shading_language_100 &&
        ext->CS_GL_EXT_geometry_shader4)
      enable = true;
  } else return;

  isOpen = true;
}

////////////////////////////////////////////////////////////////////
//                          iComponent
////////////////////////////////////////////////////////////////////
bool csGLShader_GLSL::Initialize(iObjectRegistry* reg)
{
  object_reg = reg;

  csRef<iGraphics3D> r = csQueryRegistry<iGraphics3D> (object_reg);

  csRef<iFactory> f = scfQueryInterfaceSafe<iFactory> (r);
  if (f == 0 || strcmp ("crystalspace.graphics3d.opengl", 
      f->QueryClassID ()) != 0)
    return false;

  r->GetDriver2D()->PerformExtension ("getextmanager", &ext);
  return true;
}

