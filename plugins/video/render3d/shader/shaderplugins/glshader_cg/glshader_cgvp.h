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

#include "ivideo/shader/shader.h"
#include "csutil/strhash.h"

#include <Cg/cg.h>
#include <Cg/cgGL.h>

class csShaderGLCGVP : public iShaderProgram
{
private:
  enum
  {
    XMLTOKEN_CGVP = 1,
    XMLTOKEN_DECLARE,
    XMLTOKEN_VARIABLEMAP,
    XMLTOKEN_PROGRAM
  };

  struct variablemapentry
  {
    variablemapentry() { name = csInvalidStringID; cgvarname = 0; }
    ~variablemapentry() { delete[] cgvarname; }
    csStringID name;
    char* cgvarname;
    CGparameter parameter;
  };

  struct matrixtrackerentry
  {

    CGGLenum matrix;
    CGGLenum modifier;
    CGparameter parameter;
  };

  csArray<variablemapentry> variablemap;
  csArray<matrixtrackerentry> matrixtrackers;

  csRef<iObjectRegistry> object_reg;
  CGcontext context;

  CGprogram program;

  csStringHash xmltokens;

  void BuildTokenHash();

  char* programstring;

  bool validProgram;

  csShaderVariableContextHelper svContextHelper;
  csShaderVariableList dynamicVars;
public:
  SCF_DECLARE_IBASE;

  csShaderGLCGVP(iObjectRegistry* objreg, CGcontext context)
  {
    SCF_CONSTRUCT_IBASE (0);
    validProgram = true;
    this->object_reg = objreg;
    this->context = context;
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

  bool LoadProgramStringToGL( const char* programstring );

  void SetValid(bool val) { validProgram = val; }

  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgram
  ////////////////////////////////////////////////////////////////////

  /// Sets this program to be the one used when rendering
  virtual void Activate(csRenderMesh* mesh);

  /// Deactivate program so that it's not used in next rendering
  virtual void Deactivate();

  /// Setup states needed for proper operation of the shader
  virtual void SetupState (csRenderMesh* mesh,
    const csArray<iShaderVariableContext*> &dynamicDomains);

  /// Reset states to original
  virtual void ResetState ();

  /// Check if valid
  virtual bool IsValid() { return validProgram;} 

    /// Loads shaderprogram from buffer
  virtual bool Load(iDataBuffer* program);

  /// Loads from a document-node
  virtual bool Load(iDocumentNode* node);

  /**
   * Prepares the shaderprogram for usage. Must be called before the shader
   * is assigned to a material.
   */
  virtual bool Prepare(iShaderPass *pass);

  //=================== iShaderVariableContext ================//
  /// Add a variable to this context
  virtual void AddVariable (csShaderVariable *variable)
  { svContextHelper.AddVariable (variable); }

  /// Get a named variable from this context
  virtual csShaderVariable* GetVariable (csStringID name) const
  { return svContextHelper.GetVariable (name); }

  /// Fill a csShaderVariableList
  virtual void FillVariableList (csShaderVariableList *list) const
  { svContextHelper.FillVariableList (list); }

  /// Get a named variable from this context, and any context above/outer
  virtual csShaderVariable* GetVariableRecursive (csStringID name) const
  {
    csShaderVariable* var;
    var=GetVariable (name);
    if(var) return var;
    return 0;
  }
};


#endif //__GLSHADER_CGVP_H__
