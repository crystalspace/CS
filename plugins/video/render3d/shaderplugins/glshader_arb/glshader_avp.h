/*
Copyright (C) 2002 by Mårten Svanfeldt
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

#include "ivideo/shader/shader.h"
#include "csutil/strhash.h"

class csShaderGLAVP : public iShaderProgram
{
private:
  enum
  {
    XMLTOKEN_ARBVP = 1,
    XMLTOKEN_DECLARE,
    XMLTOKEN_VARIABLEMAP,
    XMLTOKEN_PROGRAM
  };

  struct variablemapentry
  {
    variablemapentry() { name = 0; }
    ~variablemapentry() { if(name) delete name; }
    char* name;
    int registernum;
    int namehash;
  };
  csArray<csSymbolTable> symtabs;
  csSymbolTable *symtab;

  struct streammapentry
  {
    streammapentry() { name = 0; }
    csStringID name;
    int attribnum;
  };

  csGLExtensionManager* ext;
  csRef<iObjectRegistry> object_reg;

  unsigned int program_num;

  csBasicVector variablemap;
  csBasicVector streammap;
  csStringHash xmltokens;

  void BuildTokenHash();

  char* programstring;

  bool validProgram;

public:
  SCF_DECLARE_IBASE;

  csShaderGLAVP(iObjectRegistry* objreg, csGLExtensionManager* ext)
  {
    validProgram = true;
    SCF_CONSTRUCT_IBASE (0);
    this->object_reg = objreg;
    this->ext = ext;
    programstring = 0;
  }
  virtual ~csShaderGLAVP ()
  {
    delete programstring;
  }

  bool LoadProgramStringToGL( const char* programstring );

  void SetValid(bool val) { validProgram = val; }

  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgram
  ////////////////////////////////////////////////////////////////////

  virtual csPtr<iString> GetProgramID();

  /// Sets this program to be the one used when rendering
  virtual void Activate(iShaderPass* current, csRenderMesh* mesh);

  /// Deactivate program so that it's not used in next rendering
  virtual void Deactivate(iShaderPass* current);

  /// Setup states needed for proper operation of the shader
  virtual void SetupState (iShaderPass* current, csRenderMesh* mesh);

  /// Reset states to original
  virtual void ResetState ();

  virtual void AddChild(iShaderBranch *c)
    { symtab->AddChild(c->GetSymbolTable()); }
  virtual bool AddVariable(iShaderVariable* variable) 
    { return false; } // Don't allow externals to add variables
  virtual iShaderVariable* GetVariable(csStringID name)
    { return (iShaderVariable *) symtab->GetSymbol(name); }
  virtual csSymbolTable* GetSymbolTable()
    { return symtab; }
  virtual void SelectSymbolTable(int index) {
    if (symtabs.Length() < index) symtabs.SetLength(index + 1);
    symtab = & symtabs[index];
  }

  /// Check if valid
  virtual bool IsValid() { return validProgram;} 

    /// Loads shaderprogram from buffer
  virtual bool Load(iDataBuffer* program);

  /// Loads from a document-node
  virtual bool Load(iDocumentNode* node);

  /// Prepares the shaderprogram for usage. Must be called before the shader is assigned to a material
  virtual bool Prepare();
};


#endif //__GLSHADER_AVP_H__

