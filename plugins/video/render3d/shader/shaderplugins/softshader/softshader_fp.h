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

#ifndef __GLSHADER_CGFP_H__
#define __GLSHADER_CGFP_H__

#include "ivideo/shader/shader.h"
#include "csutil/strhash.h"
#include "csgfx/shadervarcontext.h"
#include "csplugincommon/shader/shaderplugin.h"

class csSoftShader_FP : public iShaderProgram
{
private:
  enum
  {
    XMLTOKEN_SOFTFP = 1
  };

  iObjectRegistry* object_reg;

  csStringHash xmltokens;

  void BuildTokenHash();

  bool validProgram;

  csShaderVariableContext svContextHelper;
public:
  SCF_DECLARE_IBASE;

  csSoftShader_FP(iObjectRegistry* objreg)
  {
    SCF_CONSTRUCT_IBASE (0);
    validProgram = true;
    this->object_reg = objreg;
  }
  virtual ~csSoftShader_FP ()
  {
    SCF_DESTRUCT_IBASE ();
  }

  void SetValid(bool val) { validProgram = val; }

  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgram
  ////////////////////////////////////////////////////////////////////

  /// Sets this program to be the one used when rendering
  virtual void Activate();

  /// Deactivate program so that it's not used in next rendering
  virtual void Deactivate();

  virtual void SetupState (const csRenderMesh* mesh,
    csRenderMeshModes& modes,
    const csShaderVarStack &stacks) {}

  virtual void ResetState () {}


  /// Check if valid
  virtual bool IsValid() { return validProgram;} 

  /// Loads shaderprogram from buffer
  virtual bool Load (iShaderTUResolver*, iDataBuffer* program);

  /// Loads from a document-node
  virtual bool Load (iShaderTUResolver*, iDocumentNode* node);

  /// Loads from raw text
  virtual bool Load (iShaderTUResolver*, const char* program, 
    csArray<csShaderVarMapping> &mappings)
  { return false; }

  /// Compile a program
  virtual bool Compile(csArray<iShaderVariableContext*> &staticContexts);
};


#endif //__GLSHADER_CGFP_H__

