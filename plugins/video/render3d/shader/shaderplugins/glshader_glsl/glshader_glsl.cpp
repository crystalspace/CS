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

#include "csplugincommon/opengl/glextmanager.h"

#include "glshader_glsl.h"
#include "glshader_glslprogram.h"
#include "glshader_glslshader.h"

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderGLSL)
{
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
      ext->InitGL_ARB_tessellation_shader ();
      if (ext->CS_GL_ARB_vertex_shader &&
        ext->CS_GL_ARB_shader_objects &&
        ext->CS_GL_ARB_shading_language_100)
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

    graph = csQueryRegistry<iGraphics3D> (object_reg);

    csRef<iFactory> f = scfQueryInterfaceSafe<iFactory> (graph);
    if (f == 0 || strcmp ("crystalspace.graphics3d.opengl", 
      f->QueryClassID ()) != 0)
      return false;

    graph->GetDriver2D()->PerformExtension ("getextmanager", &ext);
    return true;
  }
}
CS_PLUGIN_NAMESPACE_END(GLShaderGLSL)
