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

#ifndef __GLSHADER_GLSLPROGRAM_H__
#define __GLSHADER_GLSLPROGRAM_H__

#include "csplugincommon/shader/shaderprogram.h"

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderGLSL)
{
  class csGLShader_GLSL;
  class csShaderGLSLShader;

  class csShaderGLSLProgram :
    public scfImplementationExt0<csShaderGLSLProgram, csShaderProgram>
  {
  private:
    csRef<csGLShader_GLSL> shaderPlug;

    // GL identifier
    GLuint program_id;

    bool validProgram;                // what for
    bool useTessellation;

    csArray<csString> tuBindings;

    csRef<iDataBuffer> vpSource;
    csRef<iDataBuffer> fpSource;
    csRef<iDataBuffer> gpSource;
    csRef<iDataBuffer> epSource;
    csRef<iDataBuffer> cpSource;

    csRef<csShaderGLSLShader> vp;
    csRef<csShaderGLSLShader> fp;
    csRef<csShaderGLSLShader> gp;
    csRef<csShaderGLSLShader> ep;
    csRef<csShaderGLSLShader> cp;

    csRef<iDataBuffer> LoadSource (iDocumentNode* node);

    csPtr<csShaderGLSLShader> CreateVP () const;
    csPtr<csShaderGLSLShader> CreateFP () const;
    csPtr<csShaderGLSLShader> CreateGP () const;
    csPtr<csShaderGLSLShader> CreateEP () const;
    csPtr<csShaderGLSLShader> CreateCP () const;

    void SetupVmap ();

  public:
    CS_LEAKGUARD_DECLARE (csShaderGLSLProgram);

    csShaderGLSLProgram(csGLShader_GLSL* shaderPlug) : 
    scfImplementationType (this, shaderPlug->object_reg)
    {
      program_id = 0;
      validProgram = true;
      useTessellation = false;
      this->shaderPlug = shaderPlug;
    }
    virtual ~csShaderGLSLProgram ()
    {
      if (program_id != 0)
        shaderPlug->ext->glDeleteObjectARB (program_id);
    }

    void SetValid (bool val) { validProgram = val; }

    ////////////////////////////////////////////////////////////////////
    //                      iShaderProgram
    ////////////////////////////////////////////////////////////////////

    /// Sets this program to be the one used when rendering
    virtual void Activate ();

    /// Deactivate program so that it's not used in next rendering
    virtual void Deactivate();

    /// Setup states needed for proper operation of the shader
    virtual void SetupState (const CS::Graphics::RenderMesh* mesh,
      CS::Graphics::RenderMeshModes& modes,
      const csShaderVariableStack& stack);

    /// Reset states to original
    virtual void ResetState ();

    /// Check if valid
    virtual bool IsValid() { return validProgram;} 

    /// Loads from a document-node
    virtual bool Load (iShaderDestinationResolver*, iDocumentNode* node);

    /// Loads from raw text
    virtual bool Load (iShaderDestinationResolver*, const char*, 
      csArray<csShaderVarMapping> &);

    /// Compile a program
    virtual bool Compile (iHierarchicalCache*, csRef<iString>*);

    ////////////////////////////////////////////////////////////////////
    //                      iShaderDestinationResolver
    ////////////////////////////////////////////////////////////////////

    virtual csVertexAttrib ResolveBufferDestination (const char* binding);
    virtual int ResolveTU (const char* binding);
  };
}
CS_PLUGIN_NAMESPACE_END(GLShaderGLSL)

#endif //__GLSHADER_GLSLPROGRAM_H__

