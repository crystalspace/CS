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

#include "csgeom/vector4.h"
#include "csutil/csstring.h"
#include "csutil/strhash.h"
#include "iutil/databuff.h"
#include "iutil/objreg.h"
#include "iutil/string.h"

#include "ps1_instr.h"

struct csPSProgramInstruction
{
  csPixelShaderInstruction instruction;
  unsigned short inst_mods;
  csPSRegisterType dest_reg;
  int dest_reg_num;
  unsigned short dest_reg_mods;
  csPSRegisterType src_reg[3];
  int src_reg_num[3];
  unsigned short src_reg_mods[3];

  csPSProgramInstruction() 
  { 
    memset (this, 0, sizeof (*this)); 
    dest_reg = src_reg[0] = src_reg[1] = src_reg[2] = CS_PS_REG_NONE;
  }
};

struct csPSConstant
{
  int reg;
  csVector4 value;
};

class csPixelShaderParser
{
private:
  iObjectRegistry* object_reg;
  csStringHash instrStrings;

  struct PS_InstructionData
  {
    uint versions;
    int arguments;
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

  bool ParseProgram (iDataBuffer* program);
  const csArray<csPSProgramInstruction> &GetParsedInstructionList ()
  { return program_instructions; }
  const csArray<csPSConstant> &GetConstants ()
  { return program_constants; }

  void GetInstructionString (const csPSProgramInstruction& instr,
    csString& str);
#ifdef CS_DEBUG
  void WriteProgram (const csArray<csPSProgramInstruction>& instr, 
    csString& str);
#endif

  csPixelShaderVersion GetVersion () const { return version; };
};


#endif //__GLSHADER_PS1_PARSER_H__

