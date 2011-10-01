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

#ifndef __GLSHADER_GLSLSHADER_H__
#define __GLSHADER_GLSLSHADER_H__

#include "csplugincommon/shader/shaderplugin.h"
#include "csplugincommon/shader/shaderprogram.h"
#include "csgfx/shadervarcontext.h"
#include "ivideo/shader/shader.h"
#include "csutil/strhash.h"
#include "csutil/leakguard.h"
#include "csutil/scf_implementation.h"

#include "glshader_glsl.h"

class csShaderGLSLShader : public scfImplementation0 <csShaderGLSLShader>
{
private:
  csGLShader_GLSL* shaderPlug;

  // OpenGL identifier
  GLuint shader_id;

  // shader type description: vertex, fragment, etc.
  csString typeName;

  // shader type
  GLenum type;
public:
  CS_LEAKGUARD_DECLARE (csShaderGLSLShader);

  csShaderGLSLShader (csGLShader_GLSL* shaderPlug, const csString& name,
                      GLenum type) :
    scfImplementationType (this, shaderPlug->object_reg)
  {
    shader_id = 0;
    this->shaderPlug = shaderPlug;
    typeName = name;
    this->type = type;
  }
  ~csShaderGLSLShader ()
  {
    if (shader_id != 0)
      shaderPlug->ext->glDeleteObjectARB (shader_id);
  }

  GLuint GetID () const
  {
    return shader_id;
  }

  bool Compile (const char *source);
};


#endif //__GLSHADER_GLSLSHADER_H__

