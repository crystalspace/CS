/*
  Copyright (C) 2004 by John Harger
            (C) 2004 by Frank Richter
  
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

#include "ps1_1xto14.h"
#include "ps1_instr.h"

const char* csPS1xTo14Converter::SetLastError (const char* fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  lastError.FormatV (fmt, args);
  va_end (args);

  return lastError.GetData();
}

const char* csPS1xTo14Converter::AddInstruction (
  const csPSProgramInstruction &instr, int instrIndex)
{
  switch (instr.instruction)
  {
    case CS_PS_INS_BEM:
    case CS_PS_INS_TEXCRD:
    case CS_PS_INS_TEXDEPTH:
    case CS_PS_INS_TEXLD:
      // PS1.4 only instructions
      return SetLastError ("PS1.4 only instruction '%s'(%d)",
	GetInstructionName (instr.instruction), instrIndex);

    case CS_PS_INS_ADD:
    case CS_PS_INS_CMP:
    case CS_PS_INS_CND:
    case CS_PS_INS_DP3:
    case CS_PS_INS_DP4:
    case CS_PS_INS_LRP:
    case CS_PS_INS_MAD:
    case CS_PS_INS_MOV:
    case CS_PS_INS_MUL:
    case CS_PS_INS_NOP:
    case CS_PS_INS_SUB:
      // Arithmetic instructions 
      return AddArithmetic (instr, instrIndex);

    case CS_PS_INS_TEX:
      return AddTEX (instr, instrIndex);

    default:
      return SetLastError ("Instruction '%s'(%d) not supported yet",
	GetInstructionName (instr.instruction), instrIndex);
  }

  return 0;
}

const char* csPS1xTo14Converter::AddArithmetic (
  const csPSProgramInstruction &instr, int instrIndex)
{
  newInstructions.Push (instr);
  return 0;
}

const char* csPS1xTo14Converter::AddTEX (const csPSProgramInstruction &instr, 
					 int instrIndex)
{
  if (instr.dest_reg != CS_PS_REG_TEX)
  {
    return SetLastError ("%s (%d): Destination is not a texture register",
      GetInstructionName (instr.instruction), instrIndex);
  }

  csPSProgramInstruction newInstr;

  newInstr.instruction = CS_PS_INS_TEXLD;
  newInstr.inst_mods = instr.inst_mods;
  newInstr.dest_reg = CS_PS_REG_TEMP;
  newInstr.dest_reg_num = instr.dest_reg_num;
  newInstr.dest_reg_mods = instr.dest_reg_mods;
  newInstr.src_reg[0] = CS_PS_REG_TEX;
  newInstr.src_reg_num[0] = instr.dest_reg_num;
  newInstr.src_reg_mods[0] = instr.src_reg_mods[0];

  newInstructions.Push (newInstr);

  return 0;
}

const char* csPS1xTo14Converter::GetNewInstructions (
  const csArray<csPSProgramInstruction>*& instrs)
{
  newInstructions.Empty();

  for (int i = 0; i < instrs->Length(); i++)
  {
    const char* err;
    if ((err = AddInstruction (instrs->Get (i), i)) != 0)
      return err;
  }

  instrs = &newInstructions;
  return 0;
}
