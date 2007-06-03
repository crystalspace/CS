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
            /* On 64-bit x86 cpu's we know cpuid is supported.
             * Also, normal gcc asm isn't supported, but we can use
             * this macro to make use of cpuid.
             */

#if (CS_PROCESSOR_SIZE == 64)
 #define cpuid(func, eax, ebx, ecx, edx)\
    __asm__ __volatile__ ("cpuid":\
    "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx) : "a" (func));
#endif
            
            class DetectInstructionsNonWinGCCx86
            {
                bool CheckSupportedInstruction(int iSet)
                {
                    int eax, ebx, ecx, edx;
                    // 64-bit x86
                    if(CS_PROCESSOR_SIZE == 64)
                        cpuid(0x1, eax, ebx, ecx, edx);

                    switch(iSet)
                    {
                    case 0:
                        {
                            return ((edx & (1<<23)) != 0);
                        }
                    case 1:
                        {
                            return ((edx & (1<<25)) != 0);
                        }
                    case 2:
                        {
                            return ((edx & (1<<26)) != 0);
                        }
                    case 3:
                        {
                            return ((ecx & 1) != 0);
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
