/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
  
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_FPU80X86_H__
#define __CS_FPU80X86_H__

#if defined(PROC_X86) && defined(COMP_GCC)

/*
     ---- ---- --XX XXXX = MCW_EM - exception masks (1=handle exception internally, 0=fault)
     ---- ---- ---- ---X = EM_INVALID - invalid operation
     ---- ---- ---- --X- = EM_DENORMAL - denormal operand
     ---- ---- ---- -X-- = EM_ZERODIVIDE - divide by zero
     ---- ---- ---- X--- = EM_OVERFLOW - overflow
     ---- ---- ---X ---- = EM_UNDERFLOW - underflow
     ---- ---- --X- ---- = EM_INEXACT - rounding was required
     ---- --XX ---- ---- = MCW_PC - precision control
     ---- --00 ---- ---- = PC_24 - single precision
     ---- --10 ---- ---- = PC_53 - double precision
     ---- --11 ---- ---- = PC_64 - extended precision
     ---- XX-- ---- ---- = MCW_RC - rounding control
     ---- 00-- ---- ---- = RC_NEAR - round to nearest
     ---- 01-- ---- ---- = RC_DOWN - round towards -Inf
     ---- 10-- ---- ---- = RC_UP - round towards +Inf
     ---- 11-- ---- ---- = RC_CHOP - round towards zero
     ---X ---- ---- ---- = MCW_IC - infinity control (obsolete, always affine)
     ---0 ---- ---- ---- = IC_AFFINE - -Inf < +Inf
     ---1 ---- ---- ---- = IC_PROJECTIVE - -Inf == +Inf
*/

static inline unsigned int csControl87(unsigned int newcw, unsigned int mask)
{
  int oldcw;
  asm __volatile__
  (
        "       fclex                   \n"     // clear exceptions
        "       fstcw   %3              \n"     // oldcw = FPU control word
        "       andl    %3,%%eax        \n"     // eax &= oldcw;
        "       andl    %0,%%ecx        \n"     // ecx &= newcw
        "       orl     %%ecx,%%eax     \n"     // eax |= ecx
        "       movl    %%eax,%3        \n"     // tmpcw = ebx
        "       fldcw   %3              \n"     // load FPU control word
	:
	: "g" (newcw), "a" (~mask), "c" (mask), "m" (oldcw)
	: "memory"
  );
  // NOTE: oldcw is modified by the asm.  workaround for gcc 3.0
  return oldcw & 0xffff;
}


#else

#if defined(COMP_VC)
#define csControl87	_control87
#else
static inline unsigned int csControl87(unsigned int, unsigned int) { return 0; }
#endif

#endif

#endif // __CS_FPU80X86_H__
