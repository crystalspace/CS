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

#include "../../common/shaderplugin.h"
#include "csgfx/shadervarcontext.h"
#include "ivideo/shader/shader.h"
#include "csutil/strhash.h"

class csGLShader_ARB;

class csShaderGLAVP : public iShaderProgram
{
private:
  enum
  {
    XMLTOKEN_ARBVP = 100,
    XMLTOKEN_DECLARE,
    XMLTOKEN_VARIABLEMAP,
    XMLTOKEN_PROGRAM,
    XMLTOKEN_DESCRIPTION
  };

  struct variablemapentry
  {
    variablemapentry() { name = csInvalidStringID; }
    csStringID name;
    int registernum;
    // Variables that can be resolved statically at shader load
    // or compilation is put in "statlink"
    csRef<csShaderVariable> statlink;
  };

  csArray<variablemapentry> variablemap;

  csGLShader_ARB* shaderPlug;

  GLuint program_num;

  csStringHash xmltokens;

  void BuildTokenHash();

  char* programstring;
  char* description;
  bool validProgram;

  void Report (int severity, const char* msg, ...);

  csShaderVariableContext svcontext;

public:
  SCF_DECLARE_IBASE;

  csShaderGLAVP(csGLShader_ARB* shaderPlug)
  {
    SCF_CONSTRUCT_IBASE (0);
    validProgram = true;
    this->shaderPlug = shaderPlug;
    programstring = 0;
    description = 0;
  }
  virtual ~csShaderGLAVP ()
  {
    delete[] programstring;
    delete[] description;
    SCF_DESTRUCT_IBASE ();
  }

  bool LoadProgramStringToGL( const char* programstring );

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
    const CS_SHADERVAR_STACK &stacks);

  /// Reset states to original
  virtual void ResetState ();

  /// Check if valid
  virtual bool IsValid() { return validProgram;} 

  /// Loads from a document-node
  virtual bool Load(iDocumentNode* node);

  /// Compile a program
  virtual bool Compile(csArray<iShaderVariableContext*> &staticContexts);


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
  void PushVariables (CS_SHADERVAR_STACK &stacks) const
    { svcontext.PushVariables (stacks); }

  /**
  * Pop the variables of this context off the variable stacks
  * supplied in the "stacks" argument
  */
  void PopVariables (CS_SHADERVAR_STACK &stacks) const
    { svcontext.PopVariables (stacks); }
};


#endif //__GLSHADER_AVP_H__

