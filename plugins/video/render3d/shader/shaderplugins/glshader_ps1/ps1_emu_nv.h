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

#ifndef __GLSHADER_PS1_NV_H__
#define __GLSHADER_PS1_NV_H__

#include "csplugincommon/shader/shaderplugin.h"
#include "ivideo/shader/shader.h"
#include "csutil/strhash.h"

#include "ps1_emu_common.h"
#include "ps1_parser.h"

class csShaderGLPS1_NV : public csShaderGLPS1_Common
{
private:
  GLuint program_num;
  GLuint tex_program_num;

  struct nv_input
  {
    nv_input (GLenum p, GLenum v, GLenum i, GLenum m, GLenum c)
    {
      portion = p; variable = v; input = i; mapping = m; component = c;
    }
    GLenum portion;
    GLenum variable;
    GLenum input;
    GLenum mapping;
    GLenum component;
  };
  struct nv_output
  {
    GLenum portion;
    GLenum abOutput;
    GLenum cdOutput;
    GLenum sumOutput;
    GLenum scale;
    GLenum bias;
    GLboolean abDotProduct;
    GLboolean cdDotProduct;
    GLboolean muxSum;
  };
  struct nv_combiner_stage
  {
    short con_first;
    short con_second;
    csArray<nv_input> inputs;
    nv_output output;
  };
  struct nv_constant_pair
  {
    short stage;
    short first;
    short second;
  };
  csArray<nv_constant_pair> constant_pairs;
  struct nv_texture_shader_stage
  {
    short instruction;
    short stage;
    short previous;
    short param;
    bool signed_scale;
  };
  csArray<nv_texture_shader_stage> texture_shader_stages;

  void ActivateTextureShaders ();
  csVector4 GetConstantRegisterValue (int reg);

  bool GetTextureShaderInstructions (
    const csArray<csPSProgramInstruction> &instrs);
  bool GetNVInstructions (csPixelShaderParser& parser,
    csArray<nv_combiner_stage> &stages,
    const csArray<csPSProgramInstruction> &instrs);
  GLenum GetTexTarget();
public:
  csShaderGLPS1_NV (csGLShader_PS1* shaderPlug)
    : csShaderGLPS1_Common(shaderPlug) 
  {
    tex_program_num = ~0;
  }
  virtual ~csShaderGLPS1_NV ()
  {
    glDeleteLists(program_num, 2);
  }

  bool LoadProgramStringToGL ();

  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgram
  ////////////////////////////////////////////////////////////////////

  /// Sets this program to be the one used when rendering
  virtual void Activate ();

  /// Deactivate program so that it's not used in next rendering
  virtual void Deactivate();

  /// Setup states needed for proper operation of the shader
  virtual void SetupState (const csRenderMesh* mesh,
    csRenderMeshModes& modes,
    const csShaderVarStack &stacks);

  /// Reset states to original
  virtual void ResetState ();
};


#endif //__GLSHADER_PS1_NV_H__

