/*
  Copyright (C) 2002 by Marten Svanfeldt

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

#ifndef __CS_PROCESSORCAP_H__
#define __CS_PROCESSORCAP_H__

#include "csextern.h"

/**
 * This class is used to identify capabilities in the processor such as 
 * support for MMX and SSE
 */
class CS_CRYSTALSPACE_EXPORT csProcessorCapability
{
public:

  /**
   * Constructor. Does nothing
   */
  csProcessorCapability () 
  {
  }

  /**
   * Destructor. Does nothing
   */
  ~csProcessorCapability ()
  {
  }

  /**
   * Initialize the internal data. Query the processor and see what we get
   */
  static inline void Initialize ()
  {
    if (isInitialized)
      return;

#ifdef CS_PROCESSOR_X86
    CheckX86Processor ();
#else
    mmxSupported = false;
    sseSupported = false;
    processorName[0] = 0;
#endif
  }

  static inline bool HasMMX ()
  {
    Initialize ();

    return mmxSupported;
  }

  static inline bool HasSSE ()
  {
    Initialize ();

    return sseSupported;
  }

  static inline const char* GetProcessorName ()
  {
    Initialize ();

    return processorName;
  }

private:

  /// Have we been initialized yet
  static bool isInitialized;

  /// Is mmx supported
  static bool mmxSupported;

  /// Is SSE supported
  static bool sseSupported;

  /// Is 3dNow! supported
  static bool AMD3dnowSupported;
  
  /// The name of the processor
  static char processorName[16];

#if defined(CS_PROCESSOR_X86) && (CS_PROCESSOR_SIZE == 32)
  /**
  * Check for x86 features. This function is written twice due to different
  * syntax for inline assembly on MSVC and GCC
  */
  static inline void CheckX86Processor ()
  {
    int32 capFlags = 0;
    int CPUnum;
    int maxEax = 0;
    const char* procName = processorName;

    bool have_cpuid;

    #if defined(CS_COMPILER_MSVC)
    __asm
    {
      // save vars
        push        eax
        push        ebx
        push        esi

        //detect 386/486
        pushfd
        pop         eax                       //get EFLAGS
        mov         ebx, eax                  //save original EFLAGS
        xor         eax, 40000h               //toggle AC bit
        push        eax                       //copy to stack
        popfd                                 //copy to EFLAGS
        pushfd
        pop         eax                       //get EFLAGS again
        xor         eax, ebx                  //check AC bit
        mov         CPUnum, 386               //386
        je          end_detect                //is a 386, stop detection
        push        ebx                       //restore EFLAGS
        popfd

        //detect 486/pentium+
        pushfd                                //get EFLAGS
        pop         eax
        mov         ecx, eax
        xor         eax, 200000h              //toggle ID bit in EFLAGS
        push        eax                       //save new EFLAGS value on stack        									
        popfd                                 //replace current EFLAGS value
        pushfd                                //get new EFLAGS
        pop         eax                       //store new EFLAGS in EAX
        xor         eax, ecx                  //can not toggle ID bit,
        mov         CPUnum, 486
        jz          end_detect                //processor=80486
        mov         CPUnum, 586               //586+

        mov         have_cpuid, 1             //we have cpuid

        //check number of cpuid instructions
        mov         eax, 0
        cpuid         
        mov         maxEax, eax               //save the maximum eax for cpuid

        //save MFT string
        mov         esi, procName
        mov         [esi+0], ebx
        mov         [esi+4], edx
        mov         [esi+8], ecx
        mov         [esi+12], 0

        test        maxEax, 1
        jz          end_detect

        //get flagstring
        mov         eax, 1
        cpuid
        mov         capFlags, edx

end_detect:

        pop esi
        pop ebx
        pop eax
    }
    #elif defined(CS_COMPILER_GCC)
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
    "  movl         %4,%%esi            \n"
    "  movl         %%ebx,0(%%esi)      \n"
    "  movl         %%edx,4(%%esi)      \n"
    "  movl         %%ecx,8(%%esi)      \n"
    "  movl         $0,12(%%esi)        \n"
    "  testl        $1,%2               \n"
    "  jz           1f                  \n"
    //get flagstring
    "  movl         $1,%%eax            \n"
    "  cpuid                            \n"
    "  movl         %%edx,%3            \n"
    "1:                                 \n"
    : "=g" (CPUnum), "=g" (have_cpuid), "=g" (maxEax), "=g" (capFlags)
    : "g" (procName), "2" (maxEax)
    : "eax", "ebx", "ecx", "edx", "esi");

    #endif //CS_COMPILER_MSVC/GCC
    mmxSupported = capFlags & (1<<23);
    sseSupported = capFlags & (1<<25);
    //AMD3dnowSupported = capFlags & (1<<31);
  }
#else //CS_PROCESSOR_X86
  static inline void CheckX86Processor() {}
#endif //CS_PROCESSOR_X86
};

#endif //__CS_PROCESSORCAP_H__
