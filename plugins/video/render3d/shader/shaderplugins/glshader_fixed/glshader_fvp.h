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

#ifndef __GLSHADER_FVP_H__
#define __GLSHADER_FVP_H__

#include "ivideo/shader/shader.h"
#include "csgfx/shadervar.h"
#include "csutil/strhash.h"
#include "../../common/shaderplugin.h"

class csGLShader_FIXED;

class csGLShaderFVP : public iShaderProgram
{
private:
  csRef<iGraphics3D> g3d;
  csGLShader_FIXED* shaderPlug;

  enum
  {
    XMLTOKEN_FIXEDVP = 1,
    XMLTOKEN_DECLARE,
    XMLTOKEN_LIGHT,
    XMLTOKEN_AMBIENT,
    XMLTOKEN_ENVIRONMENT,
    XMLTOKEN_REFLECT
  };

  enum ENVMODE
  {
    ENVIRON_NONE = 0,
    ENVIRON_REFLECT_CUBE
  };

  struct lightingentry
  {
    lightingentry()
    { 
      positionvar = csInvalidStringID; 
      diffusevar = csInvalidStringID; 
      specularvar = csInvalidStringID; 
      attenuationvar = csInvalidStringID;
    }
    csStringID positionvar;     csRef<csShaderVariable> positionVarRef;
    csStringID diffusevar;      csRef<csShaderVariable> diffuseVarRef;
    csStringID specularvar;     csRef<csShaderVariable> specularVarRef;
    csStringID attenuationvar;  csRef<csShaderVariable> attenuationVarRef;
    int lightnum;
    csShaderVariableProxyList dynVars;
  };

  

  csStringID ambientvar;
  csRef<csShaderVariable> ambientVarRef;
  csArray<lightingentry> lights;
  bool do_lighting;

  ENVMODE environment;

  csRef<iObjectRegistry> object_reg;

  csStringHash xmltokens;

  void BuildTokenHash();

  bool validProgram;

  csShaderVariableContextHelper svContextHelper;
  csShaderVariableProxyList dynamicVars;
public:
  SCF_DECLARE_IBASE;

  csGLShaderFVP (csGLShader_FIXED* shaderPlug);
  virtual ~csGLShaderFVP ()
  {
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
    csArray<iShaderVariableContext*> &dynamicDomains);

  /// Reset states to original
  virtual void ResetState ();

  /// Loads from a document-node
  virtual bool Load(iDocumentNode* node);

  /// Compile a program
  virtual bool Compile(csArray<iShaderVariableContext*> &staticDomains);

  //=================== iShaderVariableContext ================//
  /// Add a variable to this context
  virtual void AddVariable (csShaderVariable *variable)
  { svContextHelper.AddVariable (variable); }

  /// Get a named variable from this context
  virtual csShaderVariable* GetVariable (csStringID name) const
  { return svContextHelper.GetVariable (name); }

  /// Fill a csShaderVariableList
  virtual unsigned int FillVariableList (csShaderVariableProxyList*list) const
  { return svContextHelper.FillVariableList (list); }

  /// Get a named variable from this context, and any context above/outer
  virtual csShaderVariable* GetVariableRecursive (csStringID name) const
  {
    csShaderVariable* var;
    var=GetVariable (name);
    if(var) return var;
    return 0;
  }
};


#endif //__GLSHADER_AVP_H__

