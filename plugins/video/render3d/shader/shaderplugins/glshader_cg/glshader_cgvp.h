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

#ifndef __GLSHADER_CGVP_H__
#define __GLSHADER_CGVP_H__

#include "../../common/shaderplugin.h"
#include "csgfx/shadervarcontext.h"
#include "ivideo/shader/shader.h"
#include "csutil/strhash.h"

#include <Cg/cg.h>
#include <Cg/cgGL.h>

#include "glshader_cg.h"

class csShaderGLCGVP : public iShaderProgram
{
private:
  enum
  {
    XMLTOKEN_CGVP = 1,
    XMLTOKEN_DECLARE,
    XMLTOKEN_VARIABLEMAP,
    XMLTOKEN_PROGRAM,
    XMLTOKEN_PROFILE
  };

  struct variablemapentry
  {
    variablemapentry() { name = csInvalidStringID; cgvarname = 0; }
    ~variablemapentry() { delete[] cgvarname; }
    csStringID name;
    char* cgvarname;
    CGparameter parameter;
    // Variables that can be resolved statically at shader load
    // or compilation is put in "statlink"
    csRef<csShaderVariable> statlink;
  };

  struct matrixtrackerentry
  {

    CGGLenum matrix;
    CGGLenum modifier;
    CGparameter parameter;
  };

  csArray<variablemapentry> variablemap;
  csArray<matrixtrackerentry> matrixtrackers;

  csGLShader_CG* shaderPlug;

  CGprogram program;
  csString cg_profile;

  csStringHash xmltokens;

  void BuildTokenHash();

  char* programstring;

  bool validProgram;

  csShaderVariableContext svcontext;
public:
  SCF_DECLARE_IBASE;

  csShaderGLCGVP(csGLShader_CG* shaderPlug)
  {
    SCF_CONSTRUCT_IBASE (0);
    validProgram = true;
    this->shaderPlug = shaderPlug;
    programstring = 0;
    program = 0;
  }
  virtual ~csShaderGLCGVP ()
  {
    delete programstring;
    if (program)
      cgDestroyProgram (program);
    SCF_DESTRUCT_IBASE ();
  }

  void SetValid(bool val) { validProgram = val; }

  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgram
  ////////////////////////////////////////////////////////////////////

  /// Sets this program to be the one used when rendering
  virtual void Activate ();

  /// Deactivate program so that it's not used in next rendering
  virtual void Deactivate();

  /// Setup states needed for proper operation of the shader
  virtual void SetupState (csRenderMesh* mesh,
    const csShaderVarStack &stacks);

  /// Reset states to original
  virtual void ResetState ();

  /// Check if valid
  virtual bool IsValid () { return validProgram;} 

  /// Loads from a document-node
  virtual bool Load (iDocumentNode* node);

  /// Loads from raw text
  virtual bool Load (const char* program, csArray<csShaderVarMapping> &mappings)
  { return false; }

  /// Compile a program
  virtual bool Compile (csArray<iShaderVariableContext*> &staticContexts);


  /**
   * Prepares the shaderprogram for usage. Must be called before the shader
   * is assigned to a material.
   */
  //virtual bool Prepare(iShaderPass *pass);

  //=================== iShaderVariableContext ================//
  /// Add a variable to this context
  void AddVariable (csShaderVariable *variable)
    { svcontext.AddVariable (variable); }

  /// Get a named variable from this context
  csShaderVariable* GetVariable (csStringID name) const
    { return svcontext.GetVariable (name); }

  /**
  * Push the variables of this context onto the variable stacks
  * supplied in the "stacks" argument
  */
  void PushVariables (csShaderVarStack &stacks) const
    { svcontext.PushVariables (stacks); }

  /**
  * Pop the variables of this context off the variable stacks
  * supplied in the "stacks" argument
  */
  void PopVariables (csShaderVarStack &stacks) const
    { svcontext.PopVariables (stacks); }
};


#endif //__GLSHADER_CGVP_H__
