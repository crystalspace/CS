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
#include "csutil/weakref.h"
#include "csutil/strhash.h"
#include "csutil/leakguard.h"
#include "csplugincommon/shader/shaderplugin.h"
#include "csplugincommon/shader/shaderprogram.h"

class csGLShader_FIXED;

class csGLShaderFVP : public csShaderProgram
{
public:
  CS_LEAKGUARD_DECLARE (csGLShaderFVP);

  enum TEXGENMODE
  {
    TEXGEN_NONE = 0,
    TEXGEN_REFLECT_SPHERE,
    TEXGEN_REFLECT_CUBE,
    TEXGEN_FOG
  };
  enum TexMatrixOpType
  {
    TexMatrixScale,
    TexMatrixRotate,
    TexMatrixTranslate,
    TexMatrixMatrix
  };
  struct TexMatrixOp
  {
    TexMatrixOpType type;
    ProgramParam param;

    TexMatrixOp (float def) : param (def) { }
  };
private:
  csStringHash tokens;
#define CS_TOKEN_ITEM_FILE \
  "plugins/video/render3d/shader/shaderplugins/glshader_fixed/glshader_fvp.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE

  csWeakRef<iGraphics3D> g3d;
  csGLShader_FIXED* shaderPlug;

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
  };

  csGLStateCache* statecache;  

  csStringID ambientvar;
  csRef<csShaderVariable> ambientVarRef;
  csArray<lightingentry> lights;
  bool do_lighting;

  struct layerentry
  {
    TEXGENMODE texgen;
    ProgramParam constcolor;
    csArray<TexMatrixOp> texMatrixOps;
    
    csStringID fogplane;
    csStringID fogdensity;

    layerentry () : texgen(TEXGEN_NONE) {}
  };

  csArray<layerentry> layers;

  csStringID primcolvar;
  csRef<csShaderVariable> primcolVarRef;

  bool validProgram;

  bool ParseTexMatrixOp (iDocumentNode* node, 
    TexMatrixOp& op, bool matrix = false);
  bool ParseTexMatrix (iDocumentNode* node, 
    csArray<TexMatrixOp>& matrixOps);
public:
  csGLShaderFVP (csGLShader_FIXED* shaderPlug);
  virtual ~csGLShaderFVP ();

  void SetValid(bool val) { validProgram = val; }

  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgram
  ////////////////////////////////////////////////////////////////////

  /// Sets this program to be the one used when rendering
  virtual void Activate ();

  /// Deactivate program so that it's not used in next rendering
  virtual void Deactivate ();

  /// Setup states needed for proper operation of the shader
  virtual void SetupState (const csRenderMesh* mesh,
    const csShaderVarStack &stacks);

  /// Reset states to original
  virtual void ResetState ();

  /// Loads from a document-node
  virtual bool Load(iShaderTUResolver* tuResolve, iDocumentNode* node);

  /// Loads from raw text
  virtual bool Load (iShaderTUResolver*,const char* program, 
    csArray<csShaderVarMapping> &mappings)
  { return false; }

  /// Compile a program
  virtual bool Compile(csArray<iShaderVariableContext*> &staticContexts);
};


#endif //__GLSHADER_AVP_H__

