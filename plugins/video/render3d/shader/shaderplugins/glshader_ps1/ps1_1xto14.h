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

#ifndef __GLSHADER_PS1_PS1XTO14_H__
#define __GLSHADER_PS1_PS1XTO14_H__

#include "csutil/array.h"

#include "ps1_parser.h"

class csPS1xTo14Converter
{
protected:
  csArray<csPSProgramInstruction> newInstructions;
  csString lastError;

  const char* SetLastError (const char* fmt, ...);

  const char* AddInstruction (const csPSProgramInstruction &instr,
    int instrIndex);

  const char* AddArithmetic (const csPSProgramInstruction &instr,
    int instrIndex);
  const char* AddTEX (const csPSProgramInstruction &instr,
    int instrIndex);
public:
  const char* GetNewInstructions (
    const csArray<csPSProgramInstruction>*& instrs);
};

#endif // __GLSHADER_PS1_PS1XTO14_H__
