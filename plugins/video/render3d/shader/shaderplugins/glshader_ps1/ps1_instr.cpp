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

#include "ps1_instr.h"

const csPixelShaderInstructionData csPixelShaderInstructions[CS_PS_INS_END_OF_LIST] =
{
  // Just to make sure something doesn't accidentally get
  // mapped to the INVALID instruction placeholder
  {"[invalid]", 0, 0},
#define PS_INSTR(instr, args, psversion)	\
  {#instr, psversion, args},
#include "ps1_instr.inc"
};

const char* GetInstructionName (int instrID)
{
  if ((instrID <= CS_PS_INS_INVALID) || (instrID >= CS_PS_INS_END_OF_LIST))
    return csPixelShaderInstructions[CS_PS_INS_INVALID].name;

  return csPixelShaderInstructions[instrID].name;
}
