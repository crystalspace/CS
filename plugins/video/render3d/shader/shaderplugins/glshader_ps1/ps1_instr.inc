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

PS_INSTR(ADD,           3, CS_PS_ALLVERSIONS)
PS_INSTR(BEM,           3, CS_PS_1_4)
PS_INSTR(CMP,           4, CS_PS_1_2 | CS_PS_1_3 | CS_PS_1_4)
PS_INSTR(CND,           4, CS_PS_ALLVERSIONS)
PS_INSTR(DP3,           3, CS_PS_ALLVERSIONS)
PS_INSTR(DP4,           3, CS_PS_ALLVERSIONS)
PS_INSTR(LRP,           4, CS_PS_ALLVERSIONS)
PS_INSTR(MAD,           4, CS_PS_ALLVERSIONS)
PS_INSTR(MOV,           2, CS_PS_ALLVERSIONS)
PS_INSTR(MUL,           3, CS_PS_ALLVERSIONS)
PS_INSTR(NOP,           0, CS_PS_ALLVERSIONS)
PS_INSTR(SUB,           3, CS_PS_ALLVERSIONS)
// Texture instructions
PS_INSTR(TEX,           1, CS_PS_OLDVERSIONS)
PS_INSTR(TEXBEM,        2, CS_PS_OLDVERSIONS)
PS_INSTR(TEXBEML,       2, CS_PS_OLDVERSIONS)
PS_INSTR(TEXCOORD,      1, CS_PS_OLDVERSIONS)
PS_INSTR(TEXCRD,        2, CS_PS_1_4)
PS_INSTR(TEXDEPTH,      1, CS_PS_1_4)
PS_INSTR(TEXDP3,        2, CS_PS_1_2 | CS_PS_1_3)
PS_INSTR(TEXDP3TEX,     2, CS_PS_1_2 | CS_PS_1_3)
PS_INSTR(TEXKILL,       1, CS_PS_ALLVERSIONS)
PS_INSTR(TEXLD,         2, CS_PS_1_4)
PS_INSTR(TEXM3X2DEPTH,  2, CS_PS_1_3)
PS_INSTR(TEXM3X2PAD,    2, CS_PS_OLDVERSIONS)
PS_INSTR(TEXM3X2TEX,    2, CS_PS_OLDVERSIONS)
PS_INSTR(TEXM3X3,       2, CS_PS_1_2 | CS_PS_1_3)
PS_INSTR(TEXM3X3PAD,    2, CS_PS_OLDVERSIONS)
PS_INSTR(TEXM3X3SPEC,   2, CS_PS_OLDVERSIONS)
PS_INSTR(TEXM3X3TEX,    2, CS_PS_OLDVERSIONS)
PS_INSTR(TEXM3X3VSPEC,  1, CS_PS_OLDVERSIONS)
PS_INSTR(TEXREG2AR,     2, CS_PS_1_2 | CS_PS_1_3)
PS_INSTR(TEXREG2GB,     2, CS_PS_1_2 | CS_PS_1_3)
PS_INSTR(TEXREG2RGB,    2, CS_PS_1_2 | CS_PS_1_3)
// Phase Instruction (PS 1.4 only)
PS_INSTR(PHASE,         0, CS_PS_1_4)
// Version instructions
PS_VER_INSTR(1,1)
PS_VER_INSTR(1,2)
PS_VER_INSTR(1,3)
PS_VER_INSTR(1,4)

#undef PS_VER_INSTR
#undef PS_INSTR
