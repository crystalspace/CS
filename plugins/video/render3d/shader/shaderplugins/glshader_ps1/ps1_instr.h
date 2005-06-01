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

#ifndef __GLSHADER_PS1_INSTR_H__
#define __GLSHADER_PS1_INSTR_H__

#include "cstool/bitmasktostr.h"

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
#define PS_INSTR(instr, args, psversion)	CS_PS_INS_ ## instr,
#define PS_VER_INSTR(x,y)			\
  CS_PS_INS_PS_ ## x ## _ ## y,			
#include "ps1_instr.inc"
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

CS_BITMASKTOSTR_MASK_TABLE_BEGIN(srcRegisterMods)
  CS_BITMASKTOSTR_MASK_TABLE_DEFINE(CS_PS_RMOD_NONE)
  CS_BITMASKTOSTR_MASK_TABLE_DEFINE(CS_PS_RMOD_BIAS)
  CS_BITMASKTOSTR_MASK_TABLE_DEFINE(CS_PS_RMOD_INVERT)
  CS_BITMASKTOSTR_MASK_TABLE_DEFINE(CS_PS_RMOD_NEGATE)
  CS_BITMASKTOSTR_MASK_TABLE_DEFINE(CS_PS_RMOD_SCALE)
  CS_BITMASKTOSTR_MASK_TABLE_DEFINE(CS_PS_RMOD_REP_RED)
  CS_BITMASKTOSTR_MASK_TABLE_DEFINE(CS_PS_RMOD_REP_GREEN)
  CS_BITMASKTOSTR_MASK_TABLE_DEFINE(CS_PS_RMOD_REP_BLUE)
  CS_BITMASKTOSTR_MASK_TABLE_DEFINE(CS_PS_RMOD_REP_ALPHA)
  CS_BITMASKTOSTR_MASK_TABLE_DEFINE(CS_PS_RMOD_XYZ)
  CS_BITMASKTOSTR_MASK_TABLE_DEFINE(CS_PS_RMOD_XYW)
  CS_BITMASKTOSTR_MASK_TABLE_DEFINE(CS_PS_RMOD_DZ)
  CS_BITMASKTOSTR_MASK_TABLE_DEFINE(CS_PS_RMOD_DW)
CS_BITMASKTOSTR_MASK_TABLE_END;

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
  CS_PS_REG_TEX = 0,
  CS_PS_REG_CONSTANT,
  CS_PS_REG_TEMP,
  CS_PS_REG_COLOR,
  CS_PS_REG_NONE = ~0,
};

extern const char* GetInstructionName (int instrID);
extern const char* GetVersionString (csPixelShaderVersion ver);

#endif // __GLSHADER_PS1_INSTR_H__
