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

#ifndef __GLSHADER_PS1_PARSER_H__
#define __GLSHADER_PS1_PARSER_H__

#include "iutil/string.h"

enum csPixelShaderVersion
{
  CS_PS_INVALID = 0,
  CS_PS_1_1 = 1,
  CS_PS_1_2 = 2,
  CS_PS_1_3 = 4,
  CS_PS_1_4 = 8
};

#define CS_PS_ALLVERSIONS (CS_PS_1_1 | CS_PS_1_2 | CS_PS_1_3 | CS_PS_1_4)
#define CS_PS_OLDVERSIONS (CS_PS_1_1 | CS_PS_1_2 | CS_PS_1_3)

enum csPixelShaderInstruction
{
  CS_PS_INS_INVALID = 0,
// Arithmetic instructions
  CS_PS_INS_ADD,
  CS_PS_INS_BEM,
  CS_PS_INS_CMP,
  CS_PS_INS_CND,
  CS_PS_INS_DP3,
  CS_PS_INS_DP4,
  CS_PS_INS_LRP,
  CS_PS_INS_MAD,
  CS_PS_INS_MOV,
  CS_PS_INS_MUL,
  CS_PS_INS_NOP,
  CS_PS_INS_SUB,
// Texture instructions
  CS_PS_INS_TEX,
  CS_PS_INS_TEXBEM,
  CS_PS_INS_TEXBEML,
  CS_PS_INS_TEXCOORD,
  CS_PS_INS_TEXCRD,
  CS_PS_INS_TEXDEPTH,
  CS_PS_INS_TEXDP3,
  CS_PS_INS_TEXDP3TEX,
  CS_PS_INS_TEXKILL,
  CS_PS_INS_TEXLD,
  CS_PS_INS_TEXM3X2DEPTH,
  CS_PS_INS_TEXM3X2PAD,
  CS_PS_INS_TEXM3X2TEX,
  CS_PS_INS_TEXM3X3,
  CS_PS_INS_TEXM3X3PAD,
  CS_PS_INS_TEXM3X3SPEC,
  CS_PS_INS_TEXM3X3TEX,
  CS_PS_INS_TEXM3X3VSPEC,
  CS_PS_INS_TEXREG2AR,
  CS_PS_INS_TEXREG2GB,
  CS_PS_INS_TEXREG2RGB,
// Phase Instruction (PS 1.4 only)
  CS_PS_INS_PHASE,
// End of list (instruction count)
  CS_PS_INS_END_OF_LIST
};

enum csInstructionModifier
{
  CS_PS_IMOD_NONE = 0,
  CS_PS_IMOD_X2 = 1,
  CS_PS_IMOD_X4 = 2,
  CS_PS_IMOD_X8 = 4,
  CS_PS_IMOD_D2 = 8,
  CS_PS_IMOD_D4 = 16,
  CS_PS_IMOD_D8 = 32,
  CS_PS_IMOD_SAT = 64
};

enum csSrcRegisterModifier
{
  CS_PS_RMOD_NONE = 0x00,
  CS_PS_RMOD_BIAS = 0x01,
  CS_PS_RMOD_INVERT = 0x02,
  CS_PS_RMOD_NEGATE = 0x04,
  CS_PS_RMOD_SCALE = 0x08,
  CS_PS_RMOD_REP_RED = 0x10,
  CS_PS_RMOD_REP_GREEN = 0x20,
  CS_PS_RMOD_REP_BLUE = 0x40,
  CS_PS_RMOD_REP_ALPHA = 0x80,
  CS_PS_RMOD_XYZ = 0x100,
  CS_PS_RMOD_XYW = 0x200,
  CS_PS_RMOD_DZ = 0x400,
  CS_PS_RMOD_DW = 0x800
};

enum csDestRegisterWriteMask
{
  CS_PS_WMASK_NONE = 0x00,
  CS_PS_WMASK_RED = 0x01,
  CS_PS_WMASK_BLUE = 0x02,
  CS_PS_WMASK_GREEN = 0x04,
  CS_PS_WMASK_ALPHA = 0x08
};

enum csPSRegisterType
{
  CS_PS_REG_TEX,
  CS_PS_REG_CONSTANT,
  CS_PS_REG_TEMP,
  CS_PS_REG_COLOR,
  CS_PS_REG_NONE = 0xFF
};

struct csPSProgramInstruction
{
  int instruction;
  unsigned short inst_mods;
  csPSRegisterType dest_reg;
  int dest_reg_num;
  unsigned short dest_reg_mods;
  csPSRegisterType src_reg[3];
  int src_reg_num[3];
  unsigned short src_reg_mods[3];
};

struct csPSConstant
{
  int reg;
  csVector4 value;
};

class csPixelShaderParser
{
private:
  csRef<iObjectRegistry> object_reg;
  csStringSet strings;

  struct PS_InstructionData
  {
    csStringID id;
    unsigned versions;
    short arguments;
    bool supported;
  } PS_Instructions[CS_PS_INS_END_OF_LIST];

  csPixelShaderVersion version;
  csString version_string;
  int max_registers[4];
  csArray<csPSConstant> program_constants;
  csArray<csPSProgramInstruction> program_instructions;

  void RegisterInstructions ();
  void Report (int severity, const char* msg, ...);
  bool GetInstruction (const char *str, csPSProgramInstruction &inst);
  int GetArguments (const csString &str, csString &dest, csString &src1,
    csString &src2, csString &src3, csString &src4);
  unsigned short GetDestRegMask (const char *reg);
  unsigned short GetSrcRegMods (const char *reg);
public:
  csPixelShaderParser (iObjectRegistry *obj_reg);
  ~csPixelShaderParser ();

  bool ParseProgram (const char *program);
  const csArray<csPSProgramInstruction> &GetParsedInstructionList ()
  { return program_instructions; }
  const csArray<csPSConstant> &GetConstants ()
  { return program_constants; }

  csPixelShaderVersion GetVersion () const { return version; };
};


#endif //__GLSHADER_PS1_PARSER_H__

