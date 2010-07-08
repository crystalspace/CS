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

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderPS1)
{

class csShaderGLPS1_NV : public csShaderGLPS1_Common
{
private:
  GLuint program_num;
  GLuint tex_program_num;

  enum
  {
    /// Maximum number of combiner stages supported here
    maxCombinerStages = 8,
    /// Maximum number of texture stages
    maxTextureStages = 4,
    /// Maximum number of constants per combiner stage
    maxConstsPerStage = 2
  };

  struct nv_input
  {
    nv_input () : portion (0), input (0), mapping (0), component (0) {}
    nv_input (GLenum p, GLenum v, GLenum i, GLenum m, GLenum c)
      : portion (p), variable (v), input (i), mapping (m), component (c)
    {      
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
    short stageNum;
    short numInputs;
    nv_input inputs[4];
    nv_output output;
    
    nv_combiner_stage() : numInputs (0) {}
  };
  struct nv_constant_pair
  {
    /**
     * Indices of the first and second combiner constant into the "linear"
     * PS1.x register bank
     */
    short constant[2];

    nv_constant_pair() { constant[0] = constant[1] = -1; }
  };
  nv_constant_pair constant_pairs[maxCombinerStages];
  struct nv_texture_shader_stage
  {
    short instruction;
    short stage;
    short previous;
    short param;
    bool signed_scale;
  };
  nv_texture_shader_stage texture_shader_stages[maxTextureStages];
  int numTextureStages;
  nv_combiner_stage stages[maxCombinerStages*2];
  int num_stages;
  int num_combiners;

  void ActivateTextureShaders ();
  bool ActivateRegisterCombiners ();

  bool GetTextureShaderInstructions (
    const csArray<csPSProgramInstruction> &instrs);
  bool GetNVInstructions (const csPixelShaderParser& parser,
    const csArray<csPSProgramInstruction> &instrs);
  GLenum GetTexTarget();

public:
  csShaderGLPS1_NV (csGLShader_PS1* shaderPlug)
    : csShaderGLPS1_Common(shaderPlug), program_num ((GLuint)~0),
      tex_program_num ((GLuint)~0), numTextureStages (0), 
      num_stages (0), num_combiners (0)
  {
  }
  virtual ~csShaderGLPS1_NV ()
  {
    if (program_num != (GLuint)~0)
      glDeleteLists(program_num, 2);
  }

  bool LoadProgramStringToGL (const csPixelShaderParser& parser);

  ////////////////////////////////////////////////////////////////////
  //                      iShaderProgram
  ////////////////////////////////////////////////////////////////////

  /// Sets this program to be the one used when rendering
  virtual void Activate ();

  /// Deactivate program so that it's not used in next rendering
  virtual void Deactivate();

  /// Setup states needed for proper operation of the shader
  virtual void SetupState (const CS::Graphics::RenderMesh* mesh,
    CS::Graphics::RenderMeshModes& modes,
    const csShaderVariableStack& stack);

  /// Reset states to original
  virtual void ResetState ();
};

}
CS_PLUGIN_NAMESPACE_END(GLShaderPS1)

#endif //__GLSHADER_PS1_NV_H__
