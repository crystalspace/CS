/*
Copyright (C) 2002 by Anders Stenberg
                      Mårten Svanfeldt

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

  csPDelArray<csSymbolTable> symtabs;
  csSymbolTable *symtab;

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

public:
  SCF_DECLARE_IBASE;

  csShaderGLCGVP(iObjectRegistry* objreg, CGcontext context)
  {
    validProgram = true;
    SCF_CONSTRUCT_IBASE (0);
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
  }

  bool LoadProgramStringToGL( const char* programstring );

  void SetValid(bool val) { validProgram = val; }

  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgram
  ////////////////////////////////////////////////////////////////////

  /// Sets this program to be the one used when rendering
  virtual void Activate(iShaderPass* current, csRenderMesh* mesh);

  /// Deactivate program so that it's not used in next rendering
  virtual void Deactivate(iShaderPass* current);

  virtual void SetupState (iShaderPass *current, csRenderMesh *mesh) {}

  virtual void ResetState () {}

  virtual void AddChild(iShaderBranch *b) {}
  virtual void AddVariable(csShaderVariable* variable) {}
  virtual csShaderVariable* GetVariable(csStringID s)
  { 
    return symtab->GetSymbol(s); 
  }
  virtual csSymbolTable* GetSymbolTable() { return symtab; }
  virtual csSymbolTable* GetSymbolTable(int i)
  {
    if (symtabs.Length () <= i) 
      symtabs.SetLength (i + 1, csSymbolTable ());
    return symtabs[i];
  }
  virtual void SelectSymbolTable(int i)
  {
    if (symtabs.Length () <= i) 
      symtabs.SetLength (i + 1, csSymbolTable ());
    symtab = symtabs[i];
  }

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
  virtual bool Prepare();
};


#endif //__GLSHADER_CGVP_H__
