/*
Copyright (C) 2007 by Michael Gist and Marten Svanfeldt

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

#include "cssysdef.h"

#if defined(CS_COMPILER_GCC) && defined(CS_PROCESSOR_X86) && !defined(CS_PLATFORM_WIN32)
#include "csutil/processorspecdetection.h"

namespace CS
{
  namespace Platform
  {
    namespace Implementation
    {
      uint DetectInstructionsNonWinGCCx86::CheckSupportedInstruction()
      {
	// Data from asm.
	int ecxCapFlags = 0;
	int edxCapFlags = 0;

	// 64-bit x86
#if (CS_PROCESSOR_SIZE == 64)
	// Function for cpuid to return
	const int function = 0x1;
	// Data from asm.
	int eaxCapFlags = 0;
	int ebxCapFlags = 0;
	// On 64-bit x86 cpus we know cpuid is supported.
	__asm__(
	  "  cpuid                            \n"
	  : "=a" (eaxCapFlags), "=b" (ebxCapFlags), "=c" (ecxCapFlags), "=d" (edxCapFlags)
	  : "a" (function));
#else
	__asm__(
	  //detect 386/486
	  "  pushl        %%ebx               \n"
	  "  pushfl                           \n"
	  "  popl         %%eax               \n"      //get EFLAGS
	  "  movl         %%eax, %%ebx        \n"      //save original EFLAGS
	  "  xorl         $0x40000, %%eax     \n"      //toggle AC bit
	  "  pushl        %%eax               \n"      //copy to stack
	  "  popfl                            \n"      //copy to EFLAGS
	  "  pushfl                           \n"
	  "  popl         %%eax               \n"      //get EFLAGS again
	  "  xorl         %%ebx, %%eax        \n"      //check AC bit
	  "  andl         $0x40000, %%eax     \n"
	  "  je           1f                  \n"      //is a 386, stop detection
	  "  pushl        %%ebx               \n"      //restore EFLAGS
	  "  popfl                            \n"
	  //detect 486/pentium+
	  "  pushfl                           \n"      //get EFLAGS
	  "  popl         %%eax               \n"
	  "  movl         %%eax, %%ecx        \n"
	  "  xorl         $0x200000,%%eax     \n"      //toggle ID bit in EFLAGS
	  "  pushl        %%eax               \n"      //save new EFLAGS value on stack
	  "  popfl                            \n"      //replace current EFLAGS value
	  "  pushfl                           \n"      //get new EFLAGS
	  "  popl         %%eax               \n"      //store new EFLAGS in EAX
	  "  xorl         %%eax, %%ecx        \n"      //can not toggle ID bit,
	  "  jz           1f                  \n"      //processor=80486
	  //check number of cpuid instructions
	  "  movl         $1,%%eax            \n"
	  "  cpuid                            \n"
	  "  movl         %%ecx,%0            \n"
	  "  movl         %%edx,%1            \n"
	  "1:                                 \n"
	  "  popl         %%ebx               \n"
	  : "=g" (ecxCapFlags), "=g" (edxCapFlags)
	  : 
	  : "eax", "ecx", "edx");
#endif

	uint instructionBitMask = 0U;
	// Check for instruction sets and set flags.
	if((edxCapFlags & (1<<23)) != 0)
	  instructionBitMask |= MMX;
	if((edxCapFlags & (1<<25)) != 0)
	  instructionBitMask |= SSE;
	if((edxCapFlags & (1<<26)) != 0)
	  instructionBitMask |= SSE2;
	if((ecxCapFlags & 1) != 0)
	  instructionBitMask |= SSE3;

	return instructionBitMask;
      }
    } // namespace Implementation
  } // namespace Platform
} // namespace CS

#endif // defined(CS_COMPILER_GCC) && defined(CS_PROCESSOR_X86) && !defined(CS_PLATFORM_WIN32)
