/*
    Crystal Space software driver MMX-related routines and definitions
    Copyright (C) 1998 by Jorrit Tyberghein
    Contributed by Andrew Zabolotny <bit@eltech.ru>
    VC++ port by Olivier Langlois <olanglois@sympatico.ca>

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

#ifndef __MMX_H__
#define __MMX_H__

#if defined (COMP_VC) || defined (COMP_WCC)

#if _MSC_VER < 1200     // If compiler version is below 6
#define cpuid __asm _emit 0x0F __asm _emit 0xA2
#endif

#pragma warning( disable : 4035 ) // Disable warning for functions that does not have
                                  // return statement

/**
 * Detect whenever current CPU supports MMX instructions and return its ID.
 * Memory block to hold id string should be at least 13 bytes size.
 */

inline bool mmxDetect (char *id)
{
  bool retcode;                         // Watcom barfs if there is nor return value
  __asm{
                pushfd
                pop     eax
                mov     ebx, eax
                xor     eax, 200000H   // The ability to modify the ID flag of EFLAGS
                push    eax            // indicates support for the cpuid instruction
                popfd
                pushfd
                pop     eax
                cmp     eax, ebx
                setnz   al
                jz      label_1
                mov     eax, 0
                cpuid
                mov     eax,  id
                mov     DWORD PTR [eax],   ebx     // GenuineIntel
                mov     DWORD PTR [eax+4], edx
                mov     DWORD PTR [eax+8], ecx
                mov     BYTE  PTR [eax+12],0
                mov     eax, 1
                cpuid
                test    edx, 800000H  // Is MMX present ?
                setnz   al
label_1:        mov     retcode,al
  }
  return retcode;
}
#else
inline bool mmxDetect (char *id)
{
  bool detect;

  asm ("        pushfl
                popl    %%eax
                movl    %%eax, %%ebx
                xorl    $0x200000, %%eax
                pushl   %%eax
                popfl
                pushfl
                popl    %%eax
                cmpl    %%ebx, %%eax
                setnzb  %%al
                jz      1f
                movl    $0, %%eax
                cpuid
                movl    %%ebx, 0(%%esi)
                movl    %%edx, 4(%%esi)
                movl    %%ecx, 8(%%esi)
                movb    $0, 12(%%esi)
                movl    $1, %%eax
                cpuid
                testl   $0x800000, %%edx
                setnzb  %%al
1:
  ": "=a" (detect) : "S" (id) : "ebx", "ecx", "edx");
  return detect;
}
#endif

/// Call mmxEnd () before using floating-point coprocessor
inline void mmxEnd ()
{
#if defined (COMP_VC) || defined (COMP_WCC)
  __asm emms
#else
  asm ("emms");
#endif
}

/// Test routine to print current MMX state
#if defined (COMP_VC) || defined (COMP_WCC)
inline void mmxPrintState ()
{
  __int64 mm0v, mm1v, mm2v, mm3v, mm4v, mm5v, mm6v, mm7v;

  __asm{        movq    mm0v, mm0
                movq    mm1v, mm1
                movq    mm2v, mm2
                movq    mm3v, mm3
                movq    mm4v, mm4
                movq    mm5v, mm5
                movq    mm6v, mm6
                movq    mm7v, mm7
  }
 printf ("mm0 = %08X%08X  mm1 = %08X%08X\n",
   (unsigned int)(mm0v >> 32), (unsigned int)mm0v, (unsigned int)(mm1v >> 32), (unsigned int)mm1v);
 printf ("mm2 = %08X%08X  mm3 = %08X%08X\n",
   (unsigned int)(mm2v >> 32), (unsigned int)mm2v, (unsigned int)(mm3v >> 32), (unsigned int)mm3v);
 printf ("mm4 = %08x%08x  mm5 = %08x%08x\n",
   (unsigned int)(mm4v >> 32), (unsigned int)mm4v, (unsigned int)(mm5v >> 32), (unsigned int)mm5v);
 printf ("mm6 = %08x%08x  mm7 = %08x%08x\n",
   (unsigned int)(mm6v >> 32), (unsigned int)mm6v, (unsigned int)(mm7v >> 32), (unsigned int)mm7v);
}
#else
inline void mmxPrintState ()
{
  unsigned long long mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
  asm ("        movq    %%mm0, %0
                movq    %%mm1, %1
                movq    %%mm2, %2
                movq    %%mm3, %3
                movq    %%mm4, %4
                movq    %%mm5, %5
                movq    %%mm6, %6
                movq    %%mm7, %7
  " : : "m" (mm0), "m" (mm1), "m" (mm2), "m" (mm3),
        "m" (mm4), "m" (mm5), "m" (mm6), "m" (mm7) : "eax");
 printf ("mm0 = %08X%08X  mm1 = %08X%08X\n",
   (unsigned int)(mm0 >> 32), (unsigned int)mm0, (unsigned int)(mm1 >> 32), (unsigned int)mm1);
 printf ("mm2 = %08X%08X  mm3 = %08X%08X\n",
   (unsigned int)(mm2 >> 32), (unsigned int)mm2, (unsigned int)(mm3 >> 32), (unsigned int)mm3);
 printf ("mm4 = %08x%08x  mm5 = %08x%08x\n",
   (unsigned int)(mm4 >> 32), (unsigned int)mm4, (unsigned int)(mm5 >> 32), (unsigned int)mm5);
 printf ("mm6 = %08x%08x  mm7 = %08x%08x\n",
   (unsigned int)(mm6 >> 32), (unsigned int)mm6, (unsigned int)(mm7 >> 32), (unsigned int)mm7);
}
#endif

/**
 * The following macros can be used by Scan::draw_scanline_XXXX methods
 * to provide high-speed assembly implementations using MMX...
 */

/**
 * Fill Z-buffer with interpolated values ...
 * Variables expected: z_buffer (in/out), lastZbuf, izz, dzz
 */
#if defined (COMP_VC) || defined (COMP_WCC)

#define MMX_FILLZBUFFER \
    __asm{ \
    __asm       mov     ecx, lastZbuf                           \
    __asm       mov     edi, z_buffer                           \
    __asm       sub     ecx, edi                /* set ecx to offset of lastZbuf*/\
    __asm       add     ecx, 4                                  \
    __asm MMXlabel_1:                                           \
    __asm       cmp     ecx, 4*16                               \
    __asm       jb      MMXlabel_6                              \
                                                                \
    __asm       movd    mm2, dzz                /* mm2 = 0 | dz*/ \
    __asm       movd    mm0, izz                /* mm0 = 0 | z0*/  \
    __asm       punpckldq mm2, mm2              /* mm2 = dz | dz*/ \
    __asm       movq    mm1, mm0                /* mm1 = 0 | z0*/  \
    __asm       paddd   mm1, mm2                /* mm1 = dz | z1*/ \
    __asm       punpckldq mm0, mm1              /* mm0 = z1 | z0*/ \
    __asm       movq    mm1, mm0                /* mm1 = z1 | z0*/ \
    __asm       pslld   mm2, izz                /* mm2 = dz*2 | dz*2*/\
    __asm       paddd   mm1, mm2                /* mm1 = z3 | z2*/ \
    __asm       pslld   mm2, izz                /* mm2 = dz*4 | dz*4*/\
                                                                \
    __asm MMXlabel_2:                                           \
    __asm       movq    QWORD PTR [edi], mm0                    \
    __asm       paddd   mm0, mm2                                \
    __asm       movq    QWORD PTR [edi+8], mm1                  \
    __asm       paddd   mm1, mm2                                \
    __asm       movq    QWORD PTR [edi+16], mm0                 \
    __asm       paddd   mm0, mm2                                \
    __asm       movq    QWORD PTR [edi+24], mm1                 \
    __asm       paddd   mm1, mm2                                \
    __asm       sub     ecx, 4*16            /* decrement counter*/\
    __asm       movq    QWORD PTR [edi+32], mm0                 \
    __asm       paddd   mm0, mm2                                \
    __asm       movq    QWORD PTR [edi+40], mm1                 \
    __asm       paddd   mm1, mm2                                \
    __asm       movq    QWORD PTR [edi+48], mm0                 \
    __asm       paddd   mm0, mm2                                \
    __asm       movq    QWORD PTR [edi+56], mm1                 \
    __asm       paddd   mm1, mm2                                \
    __asm       add     edi, 4*16             /* increment bufptr*/\
    __asm       cmp     ecx, 4*16                               \
    __asm       jae     MMXlabel_2                              \
                                                                \
    __asm       sub     ecx, 2*4                                \
    __asm       jb      MMXlabel_3                              \
    __asm       movq    QWORD PTR [edi], mm0                    \
    __asm       add     edi, 2*4                                \
    __asm       sub     ecx, 2*4                                \
    __asm       jb      MMXlabel_3                              \
    __asm       movq    QWORD PTR [edi], mm1                    \
    __asm       add     edi, 2*4                                \
    __asm       sub     ecx, 2*4                                \
    __asm MMXlabel_3:                                           \
    __asm       add     ecx, 2*4                                \
    __asm       emms                                            \
    __asm       mov     eax, DWORD PTR [edi-4]                  \
    __asm       jmp     MMXlabel_7                              \
                                                                \
    __asm MMXlabel_6:                                           \
    __asm       mov     eax, izz                                \
    __asm MMXlabel_7:                                           \
    __asm       mov     ebx, dzz                                \
    __asm MMXlabel_8:                                           \
    __asm       sub     ecx, 4                                  \
    __asm       jc      MMXlabel_9                              \
    __asm       add     eax, ebx                                \
    __asm       mov     DWORD PTR [edi], eax                    \
    __asm       add     edi, 4                                  \
    __asm       jmp     MMXlabel_8                              \
    __asm MMXlabel_9:                                           \
    }

#else
#define MMX_FILLZBUFFER \
    asm __volatile__ (" \
                subl    %%edi, %%ecx                            \n\
                addl    $4, %%ecx                               \n\
1:              cmpl    $4*16, %%ecx                            \n\
                jb      6f                                      \n\
                                                                \n\
                movd    %2, %%mm2               # mm2 = 0 | dz  \n\
                movd    %1, %%mm0               # mm0 = 0 | z0  \n\
                punpckldq %%mm2, %%mm2          # mm2 = dz | dz \n\
                movq    %%mm0, %%mm1            # mm1 = 0 | z0  \n\
                paddd   %%mm2, %%mm1            # mm1 = dz | z1 \n\
                punpckldq %%mm1, %%mm0          # mm0 = z1 | z0 \n\
                movq    %%mm0, %%mm1            # mm1 = z1 | z0 \n\
                pslld   $1, %%mm2               # mm2 = dz*2 | dz*2\n\
                paddd   %%mm2, %%mm1            # mm1 = z3 | z2 \n\
                pslld   $1, %%mm2               # mm2 = dz*4 | dz*4\n\
                                                                \n\
2:              movq    %%mm0, (%%edi)                          \n\
                paddd   %%mm2, %%mm0                            \n\
                movq    %%mm1, 8(%%edi)                         \n\
                paddd   %%mm2, %%mm1                            \n\
                movq    %%mm0, 16(%%edi)                        \n\
                paddd   %%mm2, %%mm0                            \n\
                movq    %%mm1, 24(%%edi)                        \n\
                paddd   %%mm2, %%mm1                            \n\
                subl    $4*16, %%ecx            # decrement counter\n\
                movq    %%mm0, 32(%%edi)                        \n\
                paddd   %%mm2, %%mm0                            \n\
                movq    %%mm1, 40(%%edi)                        \n\
                paddd   %%mm2, %%mm1                            \n\
                movq    %%mm0, 48(%%edi)                        \n\
                paddd   %%mm2, %%mm0                            \n\
                movq    %%mm1, 56(%%edi)                        \n\
                paddd   %%mm2, %%mm1                            \n\
                addl    $4*16, %%edi            # increment bufptr\n\
                cmpl    $4*16, %%ecx                            \n\
                jae     2b                                      \n\
                                                                \n\
                subl    $2*4, %%ecx                             \n\
                jb      3f                                      \n\
                movq    %%mm0, (%%edi)                          \n\
                addl    $2*4, %%edi                             \n\
                subl    $2*4, %%ecx                             \n\
                jb      3f                                      \n\
                movq    %%mm1, (%%edi)                          \n\
                addl    $2*4, %%edi                             \n\
                subl    $2*4, %%ecx                             \n\
3:              addl    $2*4, %%ecx                             \n\
                emms                                            \n\
                movl    -4(%%edi), %%eax                        \n\
                jmp     7f                                      \n\
                                                                \n\
6:              movl    %1, %%eax                               \n\
7:              movl    %2, %%ebx                               \n\
8:              subl    $4, %%ecx                               \n\
                jc      9f                                      \n\
                addl    %%ebx, %%eax                            \n\
                movl    %%eax, (%%edi)                          \n\
                addl    $4, %%edi                               \n\
                jmp     8b                                      \n\
9:              movl    %%edi, %%esi"                           \
    : "=S" (z_buffer)                                           \
    : "m" (izz), "m" (dzz), "c" (lastZbuf), "D" (z_buffer)      \
    : "eax", "ebx", "ecx", "st", "st(1)", "st(2)", "st(3)",     \
      "st(4)", "st(5)", "st(6)", "st(7)"                        \
    );
#endif

#if defined (COMP_VC) || defined (COMP_WCC)
#pragma warning( default : 4035 )
#endif

#endif // __MMX_H__
