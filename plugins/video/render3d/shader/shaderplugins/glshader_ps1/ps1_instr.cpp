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

const char* GetInstructionName (int instrID)
{
#define PS_INSTR(instr, args, psversion)	      \
  if (instrID == CS_PS_INS_ ## instr)		      \
    return #instr;				      \
  else 
#define PS_VER_INSTR(x,y)			      \
  if (instrID == CS_PS_INS_PS_ ## x ## _ ## y)	      \
    return "PS_" #x "_" #y;			      \
  else
#include "ps1_instr.inc"
  return "[invalid]";
}

const char* GetVersionString (csPixelShaderVersion ver)
{
  switch (ver)
  {
    case CS_PS_1_1: return "1.1";
    case CS_PS_1_2: return "1.2";
    case CS_PS_1_3: return "1.3";
    case CS_PS_1_4: return "1.4";
    default: return "[invalid]";
  }
}
