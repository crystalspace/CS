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

#define TEXREG_BIT(regnum)	    (1 << regnum)
#define TEMPREG_BIT(regnum)	    (1 << (16 + regnum))

const char* csPS1xTo14Converter::SetLastError (const char* fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  lastError.FormatV (fmt, args);
  va_end (args);

  return lastError.GetData();
}

void csPS1xTo14Converter::ResetState()
{
  newInstructions.Empty();

  int i;
  for (i = 0; i < 2; i++)
    tempRegisterMap[i] = -1;
  neededRegs.Empty();
}

const char* csPS1xTo14Converter::GetTempReg (int oldReg, int instrIndex, 
					     int& newReg)
{
  newReg = -1;

  // Determine timespan the original temp reg was used.
  int firstNeeded = instrIndex + 1, lastNeeded = firstNeeded;
  while ((lastNeeded < neededRegs.Length()) && 
    (neededRegs[lastNeeded] & TEMPREG_BIT(oldReg)))
    lastNeeded++;

  int r;
  for (r = 0; r < 4; r++)
  {
    // Test if the temp reg is free for the time we need it
    // (r0-r3 double as texture registers)
    bool isFree = true;
    
    for (int i = firstNeeded; i < lastNeeded; i++)
    {
      if (neededRegs[i] & TEXREG_BIT(r))
      {
	isFree = false;
	break;
      }
    }

    if (isFree) 
    {
      newReg = r;
      break;
    }
  }

  if (newReg == -1)
  {
    for (r = 0; r < 1; r++)
    {
      bool isFree = true;
      
      for (int i = firstNeeded; i < lastNeeded; i++)
      {
	if (neededRegs[i] & TEMPREG_BIT(i))
	{
	  isFree = false;
	  break;
	}
      }

      if (isFree) 
      {
	newReg = r + 4;
	break;
      }
    }
  }

  if (newReg == -1)
    return SetLastError ("(%d): Could not find register to alias r%d",
      instrIndex, oldReg);

  tempRegisterMap[oldReg] = newReg;

  //newReg = oldReg;
  return 0;
}

const char* csPS1xTo14Converter::GetTexTempReg (int oldReg, int instrIndex, 
						int& newReg)
{
  newReg = oldReg;
  return 0;
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
  csPSProgramInstruction newInstr (instr);
  const char* err;

  int i;
  for (i = 0; i < 3; i++)
  {
    if (newInstr.src_reg[i] == CS_PS_REG_TEX)
    {
      if ((err = GetTexTempReg (newInstr.src_reg_num[i], instrIndex, 
	newInstr.src_reg_num[i])) != 0)
      {
	return err;
      }
      newInstr.src_reg[i] = CS_PS_REG_TEMP;
    } 
    else if (newInstr.src_reg[i] == CS_PS_REG_TEMP)
    {
      if (tempRegisterMap[newInstr.src_reg_num[i]] == -1)
	return SetLastError ("%s(%d): Temp register %d hasn't been "
	  "assigned yet", GetInstructionName (instr.instruction), 
	  instrIndex, newInstr.src_reg_num[i]);

      newInstr.src_reg_num[i] = tempRegisterMap[newInstr.src_reg_num[i]];
    }
  }
  
  if (newInstr.dest_reg == CS_PS_REG_TEMP)
  {
    if ((err = GetTempReg (newInstr.dest_reg_num, instrIndex, 
      newInstr.dest_reg_num)) != 0)
    {
      return err;
    }
  }

  newInstructions.Push (newInstr);

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

const char* csPS1xTo14Converter::CollectUsage (
  const csArray<csPSProgramInstruction>*& instrs)
{
  int i;
  uint currentBits = 0;
  int lastTempUse[6] = {-1, -1, -1, -1, -1, -1};

  for (i = 0; i < instrs->Length(); i++)
  {
    const csPSProgramInstruction& instr = instrs->Get (i);
    int j;

    const int dr = instr.dest_reg_num;
    uint nextBits = currentBits;
    if (instr.dest_reg == CS_PS_REG_TEMP)
    {
      currentBits &= ~TEMPREG_BIT (dr);
      nextBits |= TEMPREG_BIT (dr);

      for (j = i - 1; j > lastTempUse[dr]; j--)
      {
	neededRegs[j] &= ~TEMPREG_BIT (dr);
      }
    }
    else if (instr.dest_reg == CS_PS_REG_TEX)
    {
      currentBits &= ~TEXREG_BIT (instr.dest_reg_num);
      nextBits |= TEXREG_BIT (instr.dest_reg_num);
    }

    for (j = 0; j < 3; j++)
    {
      const int sr = instr.src_reg_num[j];
      if (instr.src_reg[j] == CS_PS_REG_TEMP)
      {
	currentBits |= TEMPREG_BIT (sr);
	lastTempUse[sr] = i;
      }
      else if (instr.src_reg[j] == CS_PS_REG_TEX)
      {
	currentBits |= TEXREG_BIT (sr);
      }
    }

    neededRegs.Push (currentBits);
    currentBits = nextBits;
  }

  return 0;
}

const char* csPS1xTo14Converter::GetNewInstructions (
  const csArray<csPSProgramInstruction>*& instrs)
{
  ResetState();

  const char* err;
  if ((err = CollectUsage (instrs)) != 0)
    return err;

  for (int i = 0; i < instrs->Length(); i++)
  {
    if ((err = AddInstruction (instrs->Get (i), i)) != 0)
      return err;
  }

  if (tempRegisterMap[0] != 0)
  {
    csPSProgramInstruction newInstr;

    newInstr.instruction = CS_PS_INS_MOV;
    newInstr.dest_reg = CS_PS_REG_TEMP;
    newInstr.dest_reg_num = 0;
    newInstr.src_reg[0] = CS_PS_REG_TEMP;
    newInstr.src_reg_num[0] = tempRegisterMap[0];

    newInstructions.Push (newInstr);
  }

  instrs = &newInstructions;
  return 0;
}
