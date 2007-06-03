/*
Copyright (C) 2007 by Michael Gist

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

namespace CS
{
    namespace Platform
    {
        namespace Implementation
        {
            class DetectInstructionsNonWinGCCx86
            {
            public:
                static bool CheckSupportedInstruction(int iSet)
                {
                    // Data from asm.
                    int CPUnum = 0;
                    int have_cpuid = 0;
                    int maxEax = 0;
                    int ecxCapFlags = 0;
                    int edxCapFlags = 0;
                    // 64-bit x86
#if (CS_PROCESSOR_SIZE == 64)
                    // Function for cpuid to return
                    const int function = 0x1;
                    // Data from asm.
                    int eaxCapFlags = 0;
                    int ebxCapFlags = 0;
// On 64-bit x86 cpu's we know cpuid is supported.
__asm__(
    "  cpuid                            \n"
    : "=a" (eaxCapFlags), "=b" (ebxCapFlags), "=c" (ecxCapFlags), "=d" (edxCapFlags)
    : "a" (function));
#else
char procName[16];
maxEax = 0;
__asm__(
    //detect 386/486
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
    "  movl         $386,%0             \n"      //386
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
    "  movl         $486,%0             \n"
    "  jz           1f                  \n"      //processor=80486
    "  movl         $586,%0             \n"      //586+
    "  movl         $1,%1               \n"      //we have cpuid
    //check number of cpuid instructions
    "  xorl         %%eax,%%eax         \n"      // thebolt: this was a movl $0,%eax
    "  cpuid                            \n"
    "  movl         %%eax,%2            \n"      //save the maximum eax for cpuid
    //save MFT string
    "  movl         %5,%%esi            \n"
    "  movl         %%ebx,0(%%esi)      \n"
    "  movl         %%edx,4(%%esi)      \n"
    "  movl         %%ecx,8(%%esi)      \n"
    "  movl         $0,12(%%esi)        \n"
    "  cmp          $1,%2               \n"
    "  jb           1f                  \n"
    //get flagstring
    "  movl         $1,%%eax            \n"
    "  cpuid                            \n"
    "  movl         %%ecx,%3            \n"
    "  movl         %%edx,%4            \n"
    "1:                                 \n"
    : "=g" (CPUnum), "=g" (have_cpuid), "=g" (maxEax), "=g" (ecxCapFlags), "=g" (edxCapFlags)
    : "g" (procName), "2" (maxEax)
    : "eax", "ebx", "ecx", "edx", "esi");
#endif
 
                    switch(iSet)
                    {
                    case 0:
                        {
                            return ((edxCapFlags & (1<<23)) != 0);
                        }
                    case 1:
                        {
                            return ((edxCapFlags & (1<<25)) != 0);
                        }
                    case 2:
                        {
                            return ((edxCapFlags & (1<<26)) != 0);
                        }
                    case 3:
                        {
                            return ((ecxCapFlags & 1) != 0);
                        }
                    default:
                        {
                            return false;
                        }
                    }
                }
            };
        }
    }
}
