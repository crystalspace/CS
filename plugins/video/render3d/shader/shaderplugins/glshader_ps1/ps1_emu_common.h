/*
Copyright (C) 2002 by John Harger

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

#ifndef __GLSHADER_PS1_COMMON_H__
#define __GLSHADER_PS1_COMMON_H__

#include "../../common/shaderplugin.h"
#include "csgfx/shadervarcontext.h"
#include "ivideo/shader/shader.h"
#include "csutil/strhash.h"
#include "iutil/strset.h"

class csShaderGLPS1_Common : public iShaderProgram
{
protected:
  enum
  {
    XMLTOKEN_PS1FP = 100,
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

  csGLShader_PS1* shaderPlug;

  csRef<iStringSet> strings;
  csStringHash xmltokens;

  void BuildTokenHash();

  char* programstring;
  char* description;
  bool validProgram;

  void Report (int severity, const char* msg, ...);

  virtual bool LoadProgramStringToGL( const char* programstring ) = 0;

  csShaderVariableContext svcontext;
public:
  SCF_DECLARE_IBASE;

  csShaderGLPS1_Common (csGLShader_PS1* shaderplug)
  {
    SCF_CONSTRUCT_IBASE (0);
    validProgram = true;
    programstring = 0;
    description = 0;
    shaderPlug = shaderplug;
    strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
      shaderPlug->object_reg, "crystalspace.shared.stringset", iStringSet);
  }
  virtual ~csShaderGLPS1_Common ()
  {
    delete[] programstring;
    delete[] description;
    SCF_DESTRUCT_IBASE ();
  }

  void SetValid(bool val) { validProgram = val; }

  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgram
  ////////////////////////////////////////////////////////////////////

  /// Check if valid
  virtual bool IsValid() { return validProgram;} 

  /// Loads from a document-node
  virtual bool Load(iDocumentNode* node);

  /// Compile a program
  virtual bool Compile(csArray<iShaderVariableContext*> &staticDomains);

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

#endif //__GLSHADER_PS1_COMMON_H__

