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

#include "cssysdef.h"
#include "csgeom/vector3.h"
#include "csplugincommon/opengl/glextmanager.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "iutil/document.h"
#include "iutil/string.h"
#include "iutil/strset.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"
#include "glshader_ps1.h"
#include "ps1_emu_nv.h"
#include "ps1_emu_common.h"
#include "ps1_parser.h"

void csShaderGLPS1_NV::Activate ()
{
  // enable it
  glEnable (GL_TEXTURE_SHADER_NV);

  glEnable (GL_REGISTER_COMBINERS_NV);
  glEnable (GL_PER_STAGE_CONSTANTS_NV);
  glCallList (program_num);
}

void csShaderGLPS1_NV::Deactivate()
{
  // disable it
  glDisable (GL_PER_STAGE_CONSTANTS_NV);
  glDisable (GL_REGISTER_COMBINERS_NV);
  glDisable (GL_TEXTURE_SHADER_NV);
}

void csShaderGLPS1_NV::SetupState (const csRenderMesh *mesh, 
                                   csRenderMeshModes& modes,
	                           const csShaderVarStack &stacks)
{
  csGLExtensionManager *ext = shaderPlug->ext;

  // set variables
  for (int i = 0; i < MAX_CONST_REGS; i++)
  {
    if (RetrieveParamValue (constantRegs[i], stacks))
    {
      const float* vptr = &constantRegs[i].vectorValue.x;
      for(size_t j = 0; j < constant_pairs.Length (); j++)
      {
        nv_constant_pair &pair = constant_pairs.Get (j);
        if (pair.first == i)
        {
          ext->glCombinerStageParameterfvNV (
            GL_COMBINER0_NV + pair.stage, GL_CONSTANT_COLOR0_NV,
            vptr);
        }
        else if (pair.second == i)
        {
          ext->glCombinerStageParameterfvNV (
            GL_COMBINER0_NV + pair.stage, GL_CONSTANT_COLOR1_NV,
            vptr);
        }
      }
    }
  }

  // Has to go here at least the first time so that we can find
  // the correct texture targets
  ActivateTextureShaders ();
}

void csShaderGLPS1_NV::ResetState ()
{
}

void csShaderGLPS1_NV::ActivateTextureShaders ()
{
  if (tex_program_num != (GLuint)~0)
  {
    glCallList(tex_program_num);
    return;
  }

  tex_program_num = program_num + 1;
  glNewList (tex_program_num, GL_COMPILE);

  csGLExtensionManager *ext = shaderPlug->ext;

  size_t i;
  for(i = 0; i < 4; i++)
  {
    ext->glActiveTextureARB (GL_TEXTURE0_ARB + (int)i);
    glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV,
      GL_NONE);
  }


  for(i = 0; i < texture_shader_stages.Length (); i++)
  {
    const nv_texture_shader_stage &shader = texture_shader_stages.Get(i);

    ext->glActiveTextureARB (GL_TEXTURE0_ARB + shader.stage);

    switch(shader.instruction)
    {
      default:
        break;
      case CS_PS_INS_TEX:
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV,
          GetTexTarget ());
        break;
      case CS_PS_INS_TEXBEM:
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV,
          GL_OFFSET_TEXTURE_2D_NV);
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV,
          GL_TEXTURE0_ARB + shader.previous);
        break;
      case CS_PS_INS_TEXBEML:
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV,
          GL_OFFSET_TEXTURE_SCALE_NV);
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV,
          GL_TEXTURE0_ARB + shader.previous);
        break;
      case CS_PS_INS_TEXCOORD:
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV,
          GL_PASS_THROUGH_NV);
        break;
      case CS_PS_INS_TEXKILL:
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV,
          GL_CULL_FRAGMENT_NV);
        break;
      // 3x2 and 3x3 pad instructions are equivalent here
      case CS_PS_INS_TEXM3X3PAD:
      case CS_PS_INS_TEXM3X2PAD:
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV,
          GL_DOT_PRODUCT_NV);
        if(shader.signed_scale)
        {
          glTexEnvi(GL_TEXTURE_SHADER_NV,
            GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV, GL_EXPAND_NORMAL_NV);
        }
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV,
          GL_TEXTURE0_ARB + shader.previous);
        break;
      case CS_PS_INS_TEXM3X2TEX:
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV,
          GL_DOT_PRODUCT_TEXTURE_2D_NV);
        if(shader.signed_scale)
        {
          glTexEnvi(GL_TEXTURE_SHADER_NV,
            GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV, GL_EXPAND_NORMAL_NV);
        }
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV,
          GL_TEXTURE0_ARB + shader.previous);
        break;
      case CS_PS_INS_TEXM3X3TEX:
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV,
          GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV);
        if(shader.signed_scale)
        {
          glTexEnvi(GL_TEXTURE_SHADER_NV,
            GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV, GL_EXPAND_NORMAL_NV);
        }
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV,
          GL_TEXTURE0_ARB + shader.previous);
        break;
      case CS_PS_INS_TEXM3X3SPEC:
      {
        csVector4 eye = GetConstantRegisterValue(shader.param);
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV,
          GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV);
        glTexEnvfv(GL_TEXTURE_SHADER_NV, GL_CONST_EYE_NV, &eye.x);
        if(shader.signed_scale)
        {
          glTexEnvi(GL_TEXTURE_SHADER_NV,
            GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV, GL_EXPAND_NORMAL_NV);
        }
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV,
          GL_TEXTURE0_ARB + shader.previous);
        break;
      }
      case CS_PS_INS_TEXM3X3VSPEC:
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV,
          GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV);
        if(shader.signed_scale)
        {
          glTexEnvi(GL_TEXTURE_SHADER_NV,
            GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV, GL_EXPAND_NORMAL_NV);
        }
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV,
          GL_TEXTURE0_ARB + shader.previous);
        break;
      case CS_PS_INS_TEXREG2AR:
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV,
          GL_DEPENDENT_AR_TEXTURE_2D_NV);
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV,
          GL_TEXTURE0_ARB + shader.previous);
        break;
      case CS_PS_INS_TEXREG2GB:
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV,
          GL_DEPENDENT_GB_TEXTURE_2D_NV);
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV,
          GL_TEXTURE0_ARB + shader.previous);
        break;
    }
  }

  glEndList();
}

GLenum csShaderGLPS1_NV::GetTexTarget()
{
  if(glIsEnabled(GL_TEXTURE_CUBE_MAP_ARB))
    return GL_TEXTURE_CUBE_MAP_ARB;
  if(glIsEnabled(GL_TEXTURE_3D))
    return GL_TEXTURE_3D;
  if(glIsEnabled(GL_TEXTURE_RECTANGLE_NV))
    return GL_TEXTURE_RECTANGLE_NV;
  if(glIsEnabled(GL_TEXTURE_2D))
    return GL_TEXTURE_2D;
  if(glIsEnabled(GL_TEXTURE_1D))
    return GL_TEXTURE_1D;

  return GL_NONE;
}


bool csShaderGLPS1_NV::GetTextureShaderInstructions (
  const csArray<csPSProgramInstruction> &instrs)
{
  for(size_t i = 0; i < instrs.Length (); i++)
  {
    const csPSProgramInstruction &inst = instrs.Get (i);

    // End of texture stages
    if(inst.instruction < CS_PS_INS_TEX) break;

    int stage = inst.dest_reg_num;
    if(stage < 0 || stage > 3) return false;

    int previous = inst.src_reg_num[0];
    if(inst.src_reg[0] == CS_PS_REG_NONE) previous = 0;
    if(previous < 0 || previous > 3) return false;

    int param = inst.src_reg_num[1];
    if(inst.src_reg[1] == CS_PS_REG_NONE) param = 0;
    if(param < 0 || param > 3) return false;

    nv_texture_shader_stage texshader;
    texshader.instruction = inst.instruction;
    if(inst.src_reg_mods[0] & CS_PS_RMOD_BIAS && 
        inst.src_reg_mods[0] & CS_PS_RMOD_SCALE)
      texshader.signed_scale = true;
    texshader.stage = stage;
    texshader.previous = previous;
    texshader.param = param;
    texture_shader_stages.Push (texshader);
  }

  return true;
}

csVector4 csShaderGLPS1_NV::GetConstantRegisterValue (int reg)
{
  if (constantRegs[reg].valid && (constantRegs[reg].var 
    || (constantRegs[reg].name != csInvalidStringID)))
  {
    if (constantRegs[reg].var)
    {
      csVector4 v;
      constantRegs[reg].var->GetValue (v);
      return v;
    }
    return constantRegs[reg].vectorValue;
  }
  return csVector4 (0.0f,0.0f,0.0f,0.0f);
}

bool csShaderGLPS1_NV::GetNVInstructions (csPixelShaderParser& parser,
					  csArray<nv_combiner_stage> &stages,
					  const csArray<csPSProgramInstruction> &instrs)
{
  for(size_t i = 0; i < instrs.Length (); i++)
  {
    const csPSProgramInstruction &inst = instrs.Get(i);

    /* Skip over any tex instructions, they are used in the texture
     * shader portion */
    if(inst.instruction >= CS_PS_INS_TEX) continue;

    GLenum portion = GL_RGB;
    bool both_pipelines = true;

    if(inst.instruction == CS_PS_INS_DP3)
      both_pipelines = false;

    if(inst.dest_reg_mods == CS_PS_WMASK_ALPHA)
    {
      // Write to alpha (scalar) pipeline only
      portion = GL_ALPHA;
      both_pipelines = false;
    }
    else if(inst.dest_reg_mods == (CS_PS_WMASK_RED | CS_PS_WMASK_BLUE
      | CS_PS_WMASK_GREEN))
    {
      // Write to color (vector) pipeline only
      both_pipelines = false;
    }

    // NV_r_c based register targets
    GLenum dest = GL_DISCARD_NV, src[3] = {GL_ZERO, GL_ZERO, GL_ZERO};
    GLenum mapping[3] = {GL_SIGNED_IDENTITY_NV,
      GL_SIGNED_IDENTITY_NV, GL_SIGNED_IDENTITY_NV};
    GLenum component[3] = {GL_RGB, GL_RGB, GL_RGB};
    GLenum scale = GL_NONE;
 
    // Convert the instruction modifiers to NV_r_c equivalents
    if(inst.inst_mods & CS_PS_IMOD_X2) scale = GL_SCALE_BY_TWO_NV;
    else if(inst.inst_mods & CS_PS_IMOD_X4) scale = GL_SCALE_BY_FOUR_NV;
    else if(inst.inst_mods & CS_PS_IMOD_D2) scale = GL_SCALE_BY_ONE_HALF_NV;
    else if(inst.inst_mods)
    {
      csString instrStr;
      parser.GetInstructionString (inst, instrStr);
      Report(CS_REPORTER_SEVERITY_WARNING,
        "Register Combiners doesn't support one or more modifiers for '%s' (%d).",
	instrStr.GetData(), i);
      return false;
    }

    int con_first = -1, con_second = -1;

    // Convert the PS1.x registers to NV_r_c equivalents
    for(int j=0;j<4;j++)
    {
      csPSRegisterType in_reg = CS_PS_REG_NONE;
      int in_num = 0;
      GLenum *out = 0;
      switch(j)
      {
        case 0:
          in_reg = inst.dest_reg;
          in_num = inst.dest_reg_num;
          out = &dest;
          break;
        case 1:
        case 2:
        case 3:
          in_reg = inst.src_reg[j-1];
          in_num = inst.src_reg_num[j-1];
          out = &src[j-1];
          break;
      }
      if(in_reg == CS_PS_REG_NONE) break;
      switch(in_reg)
      {
        case CS_PS_REG_TEX:
          *out = GL_TEXTURE0_ARB + in_num;
          break;
        case CS_PS_REG_CONSTANT:
          if(con_first < 0)
          {
            *out = GL_CONSTANT_COLOR0_NV;
            con_first = in_num;
          }
          else
          {
            *out = GL_CONSTANT_COLOR1_NV;
            con_second = in_num;
          }
          break;
        case CS_PS_REG_TEMP:
          if(in_num == 0) *out = GL_SPARE0_NV;
          else *out = GL_SPARE1_NV;
          break;
        case CS_PS_REG_COLOR:
          if(in_num == 0) *out = GL_PRIMARY_COLOR_NV;
          else *out = GL_SECONDARY_COLOR_NV;
          break;
        default:
	  break;
      }

      // Get the src register modifiers
      if(j>0)
      {
        switch(inst.src_reg_mods[j-1])
        {
          case (CS_PS_RMOD_NEGATE | CS_PS_RMOD_BIAS):
            mapping[j-1] = GL_HALF_BIAS_NEGATE_NV;
            break;
          case (CS_PS_RMOD_NEGATE | CS_PS_RMOD_SCALE):
            mapping[j-1] = GL_HALF_BIAS_NEGATE_NV;
            break;
          case (CS_PS_RMOD_BIAS | CS_PS_RMOD_SCALE):
            mapping[j-1] = GL_EXPAND_NORMAL_NV;
            break;
          case (CS_PS_RMOD_BIAS | CS_PS_RMOD_SCALE | CS_PS_RMOD_NEGATE):
            mapping[j-1] = GL_EXPAND_NEGATE_NV;
            break;
          case CS_PS_RMOD_SCALE:
            Report(CS_REPORTER_SEVERITY_ERROR,
              "Register Combiners doesn't support the _x2 register modifier.");
            return false;
          case CS_PS_RMOD_NEGATE:
            mapping[j-1] = GL_SIGNED_NEGATE_NV;
            break;
          case CS_PS_RMOD_BIAS:
            mapping[j-1] = GL_HALF_BIAS_NORMAL_NV;
            break;
          case CS_PS_RMOD_INVERT:
            mapping[j-1] = GL_UNSIGNED_INVERT_NV;
            break;
          default:
            mapping[j-1] = GL_SIGNED_IDENTITY_NV;
            break;
        }
      }
    }

    for(int i=0;i<(both_pipelines ? 2 : 1);i++)
    {
      if(i==1) // alpha half
      {
        portion = GL_ALPHA;
        for(int j=0;j<3;j++)
        {
          component[j] = GL_ALPHA;
        }
      }
      // NV register combiner stage
      nv_combiner_stage combiner;

      combiner.con_first = con_first;
      combiner.con_second = con_second;
      combiner.output.portion = portion;
      combiner.output.abOutput = GL_DISCARD_NV;
      combiner.output.cdOutput = GL_DISCARD_NV;
      combiner.output.sumOutput = GL_DISCARD_NV;
      combiner.output.scale = scale;
      combiner.output.bias = GL_NONE;
      combiner.output.abDotProduct = GL_FALSE;
      combiner.output.cdDotProduct = GL_FALSE;
      combiner.output.muxSum = GL_FALSE;

      switch(inst.instruction)
      {
        case CS_PS_INS_ADD:
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_A_NV,
            src[0], mapping[0], component[0]));
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_B_NV,
            GL_ZERO, GL_UNSIGNED_INVERT_NV, component[0]));
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_C_NV,
            src[1], mapping[1], component[1]));
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_D_NV,
            GL_ZERO, GL_UNSIGNED_INVERT_NV, component[1]));
          combiner.output.sumOutput = dest;
          break;
        case CS_PS_INS_BEM:
          Report(CS_REPORTER_SEVERITY_WARNING,
            "NV_register_combiners does not support the 'bem' instruction");
	  return false;
        case CS_PS_INS_CMP:
          Report(CS_REPORTER_SEVERITY_WARNING,
            "NV_register_combiners does not support the 'cmp' instruction");
	  return false;
        case CS_PS_INS_CND:
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_A_NV,
            src[0], mapping[0], component[0]));
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_B_NV,
            GL_ZERO, GL_UNSIGNED_INVERT_NV, component[0]));
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_C_NV,
            src[1], mapping[1], component[1]));
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_D_NV,
            GL_ZERO, GL_UNSIGNED_INVERT_NV, component[1]));
          combiner.output.sumOutput = dest;
          combiner.output.muxSum = GL_TRUE;
          break;
        case CS_PS_INS_DP3:
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_A_NV,
            src[0], mapping[0], component[0]));
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_B_NV,
            src[1], mapping[1], component[1]));
          combiner.output.abOutput = dest;
          combiner.output.abDotProduct = GL_TRUE;
          break;
        case CS_PS_INS_DP4:
          Report(CS_REPORTER_SEVERITY_WARNING,
            "NV_register_combiners does not support four component dot \
            products.");
          return false;
        case CS_PS_INS_LRP:
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_A_NV,
            src[0], GL_UNSIGNED_IDENTITY_NV, component[0]));
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_B_NV,
            src[1], mapping[1], component[1]));
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_C_NV,
            src[0], GL_UNSIGNED_INVERT_NV, component[0]));
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_D_NV,
            src[2], mapping[2], component[2]));
          combiner.output.sumOutput = dest;
          break;
        case CS_PS_INS_MAD:
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_A_NV,
            src[0], mapping[0], component[0]));
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_B_NV,
            src[1], mapping[1], component[1]));
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_C_NV,
            src[2], mapping[2], component[2]));
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_D_NV,
            GL_ZERO, GL_UNSIGNED_INVERT_NV, component[2]));
          combiner.output.sumOutput = dest;
          break;
        case CS_PS_INS_MOV:
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_A_NV,
            src[0], mapping[0], component[0]));
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_B_NV,
            GL_ZERO, GL_UNSIGNED_INVERT_NV, component[0]));
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_C_NV,
            GL_ZERO, GL_UNSIGNED_IDENTITY_NV, component[0]));
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_D_NV,
            GL_ZERO, GL_UNSIGNED_IDENTITY_NV, component[0]));
          combiner.output.sumOutput = dest;
          break;
        case CS_PS_INS_MUL:
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_A_NV,
            src[0], mapping[0], component[0]));
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_B_NV,
            src[1], mapping[1], component[1]));
          combiner.output.abOutput = dest;
          break;
        case CS_PS_INS_SUB:
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_A_NV,
            src[0], mapping[0], component[0]));
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_B_NV,
            GL_ZERO, GL_UNSIGNED_INVERT_NV, component[0]));
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_C_NV,
            src[1], GL_SIGNED_NEGATE_NV, component[1]));
          combiner.inputs.Push(nv_input(portion, GL_VARIABLE_D_NV,
            GL_ZERO, GL_UNSIGNED_INVERT_NV, component[1]));
          combiner.output.sumOutput = dest;
          break;
        default:
          break;
      }
      stages.Push (combiner);
    }
  }
  return true;
}

bool csShaderGLPS1_NV::LoadProgramStringToGL ()
{
  if (!programBuffer.IsValid())
    programBuffer = GetProgramData();
  if(!programBuffer.IsValid())
    return false;

  csPixelShaderParser parser (shaderPlug->object_reg);

  if(!parser.ParseProgram (programBuffer)) return false;

  const csArray<csPSConstant> &constants = parser.GetConstants ();

  size_t i;

  for(i = 0; i < constants.Length(); i++)
  {
    const csPSConstant& constant = constants.Get (i);

    constantRegs[constant.reg].vectorValue = constant.value;
    constantRegs[constant.reg].valid = true;
  }

  const csArray<csPSProgramInstruction> &instrs =
    parser.GetParsedInstructionList ();

  csArray<nv_combiner_stage> stages;

  // Get all requested texture shader functions first
  if(!GetTextureShaderInstructions(instrs)) return false;

  // Then translate PS instructions into NV_register_combiners info
  if(!GetNVInstructions (parser, stages, instrs)) return false;

  if(stages.Length () < 1) return false;

  program_num = glGenLists (2);
  if(program_num < 1) return false;

  csGLExtensionManager *ext = shaderPlug->ext;

  glNewList(program_num, GL_COMPILE);

  int num_combiners = 1;
  int prev_combiner = 0;
  for(i = 0; i < stages.Length (); i++)
  {
    const nv_combiner_stage &stage = stages.Get (i);
    if(prev_combiner != num_combiners)
    {
      nv_constant_pair constant_pair;
      constant_pair.first = stage.con_first;
      constant_pair.second = stage.con_second;
      constant_pair.stage = num_combiners - 1;
      constant_pairs.Push(constant_pair);
      prev_combiner = num_combiners;
    }
    if(i+1 < stages.Length ())
    {
      nv_combiner_stage &next = stages.Get (i+1);
      /* If there is any difference in outputs, or it's not
       * using the alpha channel, increment the register
       * combiner stage */
      if(!(next.output.abOutput == stage.output.abOutput
        && next.output.cdOutput == stage.output.cdOutput
        && next.output.sumOutput == stage.output.sumOutput
        && next.output.portion == GL_ALPHA))
      {
        num_combiners ++;
      }
    }
  }

  ext->glCombinerParameteriNV (GL_NUM_GENERAL_COMBINERS_NV,
    num_combiners);

  GLenum glstage = GL_COMBINER0_NV;

  for(i = 0; i < stages.Length (); i++)
  {
    const nv_combiner_stage &stage = stages.Get (i);
    for(size_t j = 0; j < stage.inputs.Length(); j++)
    {
      const nv_input &input = stage.inputs.Get (j);
      ext->glCombinerInputNV (glstage, input.portion, input.variable,
        input.input, input.mapping, input.component);
      if(glGetError() == GL_INVALID_OPERATION)
      {
        Report(CS_REPORTER_SEVERITY_ERROR,
          "glCombinerInputNV #%d returned GL_INVALID_OPERATION on stage %d!",
          j, i);
        return false;
      }
    }
    ext->glCombinerOutputNV (glstage, stage.output.portion,
      stage.output.abOutput, stage.output.cdOutput, stage.output.sumOutput,
      stage.output.scale, stage.output.bias, stage.output.abDotProduct,
      stage.output.cdDotProduct, stage.output.muxSum);
    if(glGetError() == GL_INVALID_OPERATION)
    {
      Report(CS_REPORTER_SEVERITY_ERROR,
        "glCombinerOutputNV returned GL_INVALID_OPERATION on stage %d!",
        i);
      return false;
    }
    if(i+1 < stages.Length ())
    {
      nv_combiner_stage &next = stages.Get (i+1);
      /* If there is any difference in outputs, or it's not
       * using the alpha channel, increment the register
       * combiner stage */
      if(!(next.output.abOutput == stage.output.abOutput
        && next.output.cdOutput == stage.output.cdOutput
        && next.output.sumOutput == stage.output.sumOutput
        && next.output.portion == GL_ALPHA))
      {
        glstage ++;
      }
    }
  }

  ext->glFinalCombinerInputNV (GL_VARIABLE_B_NV, GL_SPARE0_NV,
    GL_UNSIGNED_IDENTITY_NV, GL_RGB);

  glEndList();

  return true;
}
