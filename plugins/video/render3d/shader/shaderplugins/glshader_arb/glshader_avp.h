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

#ifndef __GLSHADER_AVP_H__
#define __GLSHADER_AVP_H__

#include "csplugincommon/shader/shaderplugin.h"
#include "csplugincommon/shader/shaderprogram.h"
#include "csgfx/shadervarcontext.h"
#include "ivideo/shader/shader.h"
#include "csutil/strhash.h"
#include "csutil/leakguard.h"

#include "glshader_arb.h"

class csShaderGLAVP : public csShaderProgram
{
private:
  csGLShader_ARB* shaderPlug;

  GLuint program_num;

  bool validProgram;
  csRef<iDataBuffer> programBuffer;

  void Report (int severity, const char* msg, ...);
public:
  SCF_DECLARE_IBASE_EXT (csShaderProgram);

  CS_LEAKGUARD_DECLARE (csShaderGLAVP);

  csShaderGLAVP(csGLShader_ARB* shaderPlug) : 
    csShaderProgram (shaderPlug->object_reg)
  {
    validProgram = true;
    this->shaderPlug = shaderPlug;
  }
  virtual ~csShaderGLAVP ()
  {
  }

  bool LoadProgramStringToGL ();

  void SetValid(bool val) { validProgram = val; }

  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgram
  ////////////////////////////////////////////////////////////////////

  /// Sets this program to be the one used when rendering
  virtual void Activate ();

  /// Deactivate program so that it's not used in next rendering
  virtual void Deactivate();

  /// Setup states needed for proper operation of the shader
  virtual void SetupState (const csRenderMesh* mesh,
    const csShaderVarStack &stacks);

  /// Reset states to original
  virtual void ResetState ();

  /// Check if valid
  virtual bool IsValid() { return validProgram;} 

  /// Loads from a document-node
  virtual bool Load (iShaderTUResolver*, iDocumentNode* node);

  /// Loads from raw text
  virtual bool Load (iShaderTUResolver*, const char* program, 
    csArray<csShaderVarMapping> &mappings);

  /// Compile a program
  virtual bool Compile(csArray<iShaderVariableContext*> &staticContexts);
};


#endif //__GLSHADER_AVP_H__

