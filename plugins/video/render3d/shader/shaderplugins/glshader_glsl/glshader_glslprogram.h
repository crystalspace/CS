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

#include "csplugincommon/shader/shaderplugin.h"
#include "csplugincommon/shader/shaderprogram.h"
#include "csgfx/shadervarcontext.h"
#include "ivideo/shader/shader.h"
#include "csutil/strhash.h"
#include "csutil/leakguard.h"
#include "csutil/scf_implementation.h"

#include "glshader_glsl.h"
#include "glshader_glslshader.h"

class csShaderGLSLProgram :
  public scfImplementationExt0<csShaderGLSLProgram, csShaderProgram>
{
private:
  csGLShader_GLSL* shaderPlug;

  // GL identifier
  GLuint program_id;

  bool validProgram;                // what for

  csRef<iDataBuffer> vpSource;
  csRef<iDataBuffer> fpSource;
  // csRef<iDataBuffer> gpSource;

  csRef<csShaderGLSLShader> vp;
  csRef<csShaderGLSLShader> fp;
  // csRef<csShaderGLSLShader> gp;

  csRef<iDataBuffer> LoadSource (iDocumentNode* node);

  csPtr<csShaderGLSLShader> CreateVP () const;
  csPtr<csShaderGLSLShader> CreateFP () const;

public:
  CS_LEAKGUARD_DECLARE (csShaderGLSLProgram);

  csShaderGLSLProgram(csGLShader_GLSL* shaderPlug) : 
    scfImplementationType (this, shaderPlug->object_reg)
  {
    program_id = 0;
    validProgram = true;
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

  // stuff.
};


#endif //__GLSHADER_GLSLPROGRAM_H__

