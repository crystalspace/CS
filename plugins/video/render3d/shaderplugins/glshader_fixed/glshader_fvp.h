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

#ifndef __GLSHADER_FVP_H__
#define __GLSHADER_FVP_H__

#include "ivideo/shader/shader.h"
#include "csutil/strhash.h"

class csGLShaderFVP : public iShaderProgram
{
private:
  csRef<iGraphics3D> g3d;

  enum
  {
    XMLTOKEN_FIXEDVP = 1,
    XMLTOKEN_DECLARE,
    XMLTOKEN_LIGHT,
    XMLTOKEN_AMBIENT
  };

  csPDelArray<csSymbolTable> symtabs;
  csSymbolTable *symtab;

  struct lightingentry
  {
    lightingentry() { 
      positionvar = csInvalidStringID; 
      diffusevar = csInvalidStringID; 
      specularvar = csInvalidStringID; 
      attenuationvar = csInvalidStringID;
    }
    csStringID positionvar;
    csStringID diffusevar;
    csStringID specularvar;
    csStringID attenuationvar;
    int lightnum;
  };

  csStringID ambientvar;
  csArray<lightingentry> lights;
  bool do_lighting;

  csGLExtensionManager* ext;
  csRef<iObjectRegistry> object_reg;

  csStringHash xmltokens;

  void BuildTokenHash();

  bool validProgram;

public:
  SCF_DECLARE_IBASE;

  csGLShaderFVP(iObjectRegistry* object_reg, csGLExtensionManager* ext)
  {
    validProgram = true;
    SCF_CONSTRUCT_IBASE (0);
    this->object_reg = object_reg;
    this->ext = ext;
    symtab = new csSymbolTable;
  
    g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  }
  virtual ~csGLShaderFVP ()
  {
    delete symtab;
  }

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

  virtual void AddChild(iShaderBranch *b) {}
  virtual void AddVariable(csShaderVariable* variable) {}
  virtual csShaderVariable* GetVariable(csStringID s)
    { return symtab->GetSymbol(s); }
  virtual csSymbolTable* GetSymbolTable() { return symtab; }
  virtual csSymbolTable* GetSymbolTable(int i) {
    if (symtabs.Length () <= i) symtabs.SetLength (i + 1, csSymbolTable ());
    return symtabs[i];
  }
  virtual void SelectSymbolTable(int i) {
    if (symtabs.Length () <= i) symtabs.SetLength (i + 1, csSymbolTable ());
    symtab = symtabs[i];
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

