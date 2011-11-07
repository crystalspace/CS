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

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderGLSL)
{
  class csGLShader_GLSL;

  class csShaderGLSLShader : public scfImplementation0 <csShaderGLSLShader>
  {
  private:
    csGLShader_GLSL* shaderPlug;

    // OpenGL identifier
    GLhandleARB shader_id;

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

    GLhandleARB GetID () const
    {
      return shader_id;
    }

    bool Compile (const char *source);
  };
}
CS_PLUGIN_NAMESPACE_END(GLShaderGLSL)

#endif //__GLSHADER_GLSLSHADER_H__
