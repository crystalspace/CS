/*
    OUTDATED: to be removed after NASM routines are debugged - A.Z.

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

/// Call mmxEnd () before using floating-point coprocessor
inline void mmxEnd ()
{
#if defined (COMP_VC) || defined (COMP_WCC)
  __asm emms
#else
  asm ("emms");
#endif
}

/**
 * The following macros can be used by draw_scanline_XXXX methods
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
    : "eax", "ebx", "st", "st(1)", "st(2)", "st(3)",            \
      "st(4)", "st(5)", "st(6)", "st(7)"                        \
    );
#endif

#endif // __MMX_H__
