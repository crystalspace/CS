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

#include "csutil/hashmap.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/scfstr.h"
#include "csutil/csmd5.h"
#include "csgeom/vector3.h"
#include "csutil/xmltiny.h"

#include "iutil/document.h"
#include "iutil/string.h"
#include "iutil/strset.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/rndbuf.h"
#include "ivideo/shader/shader.h"

#include "csplugincommon/opengl/glextmanager.h"

#include "glshader_ps1.h"
#include "ps1_emu_ati.h"
#include "ps1_emu_common.h"
#include "ps1_parser.h"
#include "ps1_1xto14.h"

void csShaderGLPS1_ATI::Activate ()
{
  //enable it
  shaderPlug->ext->glBindFragmentShaderATI (program_num);
  glEnable(GL_FRAGMENT_SHADER_ATI);
}

void csShaderGLPS1_ATI::Deactivate()
{
  glDisable(GL_FRAGMENT_SHADER_ATI);
}

void csShaderGLPS1_ATI::SetupState (const csRenderMesh *mesh, 
	const csShaderVarStack &stacks)
{
  csGLExtensionManager *ext = shaderPlug->ext;
  // set variables
  for (int i = 0; i < MAX_CONST_REGS; i++)
  {
    csShaderVariable* lvar = constantRegs[i].statlink;

    if (!lvar)
    {
      if (constantRegs[i].varID == csInvalidStringID) continue;

      if ((csStringID)variablemap[i].name < (csStringID)stacks.Length ()
	  && stacks[variablemap[i].name].Length () > 0)
	lvar = stacks[variablemap[i].name].Top ();
    }
    
    if(lvar)
    {
      csVector4 v4;
      if (lvar->GetValue (v4))
      {
        ext->glSetFragmentShaderConstantATI (GL_CON_0_ATI + i, &v4.x);
      }
    }
  }
}

void csShaderGLPS1_ATI::ResetState ()
{
}

bool csShaderGLPS1_ATI::GetATIShaderCommand
  (const csPSProgramInstruction &instr)
{
  if ((instr.instruction == CS_PS_INS_NOP) ||
    (instr.instruction == CS_PS_INS_PHASE))
    return true;

  csGLExtensionManager *ext = shaderPlug->ext;
  if(instr.instruction == CS_PS_INS_TEXLD
    || instr.instruction == CS_PS_INS_TEXCRD)
  {
    GLenum dest, interp, swizzle = GL_SWIZZLE_STR_ATI;
    if(instr.dest_reg != CS_PS_REG_TEMP) return false;
    dest = GL_REG_0_ATI + instr.dest_reg_num;
    if(instr.src_reg[0] == CS_PS_REG_TEX)
      interp = GL_TEXTURE0_ARB + instr.src_reg_num[0];
    else
      interp = GL_REG_0_ATI + instr.src_reg_num[0];
    if(instr.src_reg_mods[0] & CS_PS_RMOD_XYZ)
      swizzle = GL_SWIZZLE_STR_ATI;
    else if(instr.src_reg_mods[0] & CS_PS_RMOD_XYW)
      swizzle = GL_SWIZZLE_STQ_ATI;
    else if(instr.src_reg_mods[0] & CS_PS_RMOD_DZ)
      swizzle = GL_SWIZZLE_STR_DR_ATI;
    else if(instr.src_reg_mods[0] & CS_PS_RMOD_DW)
      swizzle = GL_SWIZZLE_STQ_DQ_ATI;
    if(instr.instruction == CS_PS_INS_TEXLD)
      ext->glSampleMapATI (dest, interp, swizzle);
    else 
      ext->glPassTexCoordATI (dest, interp, swizzle);
    return true;
  }

  // Channels to perform the operation on
  bool color = true;
  bool alpha = false;

  GLenum op = GL_NONE;
  GLuint dst, dstMask = GL_NONE, dstMod = GL_NONE;
  GLuint arg[3], argrep[3] = {GL_NONE, GL_NONE, GL_NONE},
    argmod[3] = {GL_NONE, GL_NONE, GL_NONE};

  if(instr.dest_reg != CS_PS_REG_TEMP) return false;
  dst = GL_REG_0_ATI + instr.dest_reg_num;

  if(instr.dest_reg_mods & CS_PS_WMASK_RED) dstMask |= GL_RED_BIT_ATI;
  if(instr.dest_reg_mods & CS_PS_WMASK_GREEN) dstMask |= GL_GREEN_BIT_ATI;
  if(instr.dest_reg_mods & CS_PS_WMASK_BLUE) dstMask |= GL_BLUE_BIT_ATI;

  if(instr.dest_reg_mods == CS_PS_WMASK_NONE ||
    (instr.dest_reg_mods & CS_PS_WMASK_ALPHA)) alpha = true;

  if(instr.dest_reg_mods == CS_PS_WMASK_ALPHA) color = false;

  if(instr.inst_mods & CS_PS_IMOD_X2) dstMod = GL_2X_BIT_ATI;
  else if(instr.inst_mods & CS_PS_IMOD_X4) dstMod = GL_4X_BIT_ATI;
  else if(instr.inst_mods & CS_PS_IMOD_X8) dstMod = GL_8X_BIT_ATI;
  else if(instr.inst_mods & CS_PS_IMOD_D2) dstMod = GL_HALF_BIT_ATI;
  else if(instr.inst_mods & CS_PS_IMOD_D4) dstMod = GL_QUARTER_BIT_ATI;
  else if(instr.inst_mods & CS_PS_IMOD_D8) dstMod = GL_EIGHTH_BIT_ATI;
  if(instr.inst_mods & CS_PS_IMOD_SAT) dstMod |= GL_SATURATE_BIT_ATI;

  int args = 0, i;
  for(i=0;i<3;i++)
  {
    if(instr.src_reg[i] == CS_PS_REG_NONE) break;
    switch(instr.src_reg[i])
    {
      default:
      case CS_PS_REG_TEMP:
        arg[i] = GL_REG_0_ATI + instr.src_reg_num[i];
        break;
      case CS_PS_REG_CONSTANT:
        arg[i] = GL_CON_0_ATI + instr.src_reg_num[i];
        break;
      case CS_PS_REG_COLOR:
        if(instr.src_reg_num[i] == 0) arg[i] = GL_PRIMARY_COLOR_ARB;
        else arg[i] = GL_SECONDARY_INTERPOLATOR_ATI;
        break;
      case CS_PS_REG_TEX:
        return false; // Not allowed in 1.4
    }
    if(instr.src_reg_mods[i] & CS_PS_RMOD_BIAS)
      argmod[i] |= GL_BIAS_BIT_ATI;
    if(instr.src_reg_mods[i] & CS_PS_RMOD_INVERT)
      argmod[i] |= GL_COMP_BIT_ATI;
    if(instr.src_reg_mods[i] & CS_PS_RMOD_NEGATE)
      argmod[i] |= GL_NEGATE_BIT_ATI;
    if(instr.src_reg_mods[i] & CS_PS_RMOD_SCALE)
      argmod[i] |= GL_2X_BIT_ATI;

    if (instr.src_reg_mods[i] & CS_PS_RMOD_REP_RED)
      argrep[i] = GL_RED;
    if (instr.src_reg_mods[i] & CS_PS_RMOD_REP_GREEN)
      argrep[i] = GL_GREEN;
    if (instr.src_reg_mods[i] & CS_PS_RMOD_REP_BLUE)
      argrep[i] = GL_BLUE;
    if (instr.src_reg_mods[i] & CS_PS_RMOD_REP_ALPHA)
      argrep[i] = GL_ALPHA;
  }
  args = i;

  switch(instr.instruction)
  {
    default:
      break;
    case CS_PS_INS_ADD: op = GL_ADD_ATI; break;
    case CS_PS_INS_CMP: op = GL_CND0_ATI; break;
    case CS_PS_INS_CND: op = GL_CND_ATI; break;
    case CS_PS_INS_DP3: op = GL_DOT3_ATI; break;
    case CS_PS_INS_DP4: op = GL_DOT4_ATI; break;
    case CS_PS_INS_LRP: op = GL_LERP_ATI; break;
    case CS_PS_INS_MAD: op = GL_MAD_ATI; break;
    case CS_PS_INS_MOV: op = GL_MOV_ATI; break;
    case CS_PS_INS_MUL: op = GL_MUL_ATI; break;
    case CS_PS_INS_SUB: op = GL_SUB_ATI; break;
  }
  switch(args)
  {
    default:
      return false;
    case 1:
      if(color) ext->glColorFragmentOp1ATI (op, dst, dstMask, dstMod,
	arg[0], argrep[0], argmod[0]);
      if(alpha) ext->glAlphaFragmentOp1ATI (op, dst, dstMod, arg[0],
	argrep[0], argmod[0]);
      break;
    case 2:
      if(color) ext->glColorFragmentOp2ATI (op, dst, dstMask, dstMod,
	arg[0], argrep[0], argmod[0], arg[1], argrep[1], argmod[1]);
      if(alpha) ext->glAlphaFragmentOp2ATI (op, dst, dstMod, arg[0],
	argrep[0], argmod[0], arg[1], argrep[1], argmod[1]);
      break;
    case 3:
      if(color) ext->glColorFragmentOp3ATI (op, dst, dstMask, dstMod,
	arg[0], argrep[0], argmod[0], arg[1], argrep[1], argmod[1],
	arg[2], argrep[2], argmod[2]);
      if(alpha) ext->glAlphaFragmentOp3ATI (op, dst, dstMod, arg[0],
	argrep[0], argmod[0], arg[1], argrep[1], argmod[1], arg[2],
	argrep[2], argmod[2]);
      break;
  }
  GLenum error;
  if((error = glGetError())) 
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "ATI_fragment_shader error %d!", error);
    return false;
  }
  return true;
}


#ifdef CS_DEBUG
//#define DUMP_CONVERTER_OUTPUT
#endif

bool csShaderGLPS1_ATI::LoadProgramStringToGL ()
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

    csRef<csShaderVariable> var;
    var.AttachNew (new csShaderVariable (csInvalidStringID));
    var->SetValue(constant.value);

    constantRegs[constant.reg].statlink = var;
    constantRegs[constant.reg].varID = csInvalidStringID;
  }

  const csArray<csPSProgramInstruction>* instrs =
    &parser.GetParsedInstructionList ();

  csPS1xTo14Converter conv;
  if(parser.GetVersion () != CS_PS_1_4)
  {
    const char* err;
    if ((err = conv.GetNewInstructions (instrs)) != 0)
    {
      Report (CS_REPORTER_SEVERITY_WARNING, 
	"Could not convert pixel shader to version 1.4: %s",
	err);
      return false;
    }
#ifdef DUMP_CONVERTER_OUTPUT
    csString prog;
    parser.WriteProgram (*instrs, prog);

    csPrintf (prog);
#endif
  }

  csGLExtensionManager *ext = shaderPlug->ext;

  program_num = ext->glGenFragmentShadersATI (1);

  ext->glBindFragmentShaderATI (program_num);

  ext->glBeginFragmentShaderATI ();

  for(i = 0; i < instrs->Length (); i++)
  {
    if(!GetATIShaderCommand (instrs->Get (i)))
    {
      ext->glEndFragmentShaderATI ();
      ext->glDeleteFragmentShaderATI (program_num);
      return false;
    }
  }

  ext->glEndFragmentShaderATI ();

  return true;
}
