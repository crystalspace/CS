/*
    Crystal Space 8-bit software driver assembler-optimized routines
    Copyright (C) 1998 by Jorrit Tyberghein
    Contributors:
       draw_scanline_map by David N. Arnold <derek_arnold@fuse.net>
       MMX support and other by Andrew Zabolotny <bit@eltech.ru>
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

#ifndef __SCANLN8_H__
#define __SCANLN8_H__

#if defined (DO_MMX)
#  include "mmx.h"
#endif

static short uFrac, duFrac, vFrac, dvFrac;
static unsigned int oldEBP;
static unsigned dudvInt[2];
static unsigned char *s;
static int sxx;
static unsigned char *s_destend;
static long sdzz;
static unsigned long *sz_buffer;

#define I386_SCANLINE_MAP8 \
    s = srcTex + ((vv >> 16) << shifter) + (uu >> 16);  \
                                    \
    dudvInt[1] = ((dvv >> 16) << shifter) + (duu >> 16);        \
    dudvInt[0] = dudvInt[1] + (1 << shifter);                 \
    uFrac = (short)uu;                      \
    vFrac = (short)vv;                      \
    duFrac = (short)duu;                    \
    dvFrac = (short)dvv;                    \
    sxx = xx;                               \
    s_destend = _destend;                   \
__asm {                                     \
__asm   mov     oldEBP, ebp                 \
__asm   mov     edi, _dest                  \
__asm   mov     esi, s                      \
__asm   mov      ax, uFrac                  \
__asm   mov      bx, vFrac                  \
__asm   mov      bp, duFrac                 \
                                            \
__asm   cmp     sxx, 0              /* Less than 16 pixels?*/   \
__asm   je  label8                          \
                                            \
__asm   label0:                             \
__asm   mov     dl, BYTE PTR [esi]  /* Get texel*/      \
__asm   add      bx, dvFrac             /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi], dl  /* Put pixel*/      \
__asm   sbb     ecx, ecx            /* carry flag (vv + dvv)    (ecx is used instead of edx to avoid partial register stall on PII)*/\
__asm   add      ax, bp             /* uu += duu*/      \
__asm   adc     esi, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   mov     dl, BYTE PTR [esi]  /* Get texel*/      \
__asm   add      bx, dvFrac             /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi+1], dl/* Put pixel*/      \
__asm   sbb     ecx, ecx            /* carry flag (vv + dvv)*/  \
__asm   add      ax, bp             /* uu += duu*/      \
__asm   adc     esi, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   mov     dl, BYTE PTR [esi]  /* Get texel*/      \
__asm   add      bx, dvFrac             /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi+2], dl/* Put pixel*/      \
__asm   sbb     ecx, ecx            /* carry flag (vv + dvv)*/  \
__asm   add      ax, bp             /* uu += duu*/      \
__asm   adc     esi, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   mov     dl, BYTE PTR [esi]  /* Get texel*/      \
__asm   add      bx, dvFrac             /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi+3], dl/* Put pixel*/      \
__asm   sbb     ecx, ecx            /* carry flag (vv + dvv)*/  \
__asm   add      ax, bp             /* uu += duu*/      \
__asm   adc     esi, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   mov     dl, BYTE PTR [esi]  /* Get texel*/      \
__asm   add      bx, dvFrac             /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi+4], dl/* Put pixel*/      \
__asm   sbb     ecx, ecx            /* carry flag (vv + dvv)*/  \
__asm   add      ax, bp             /* uu += duu*/      \
__asm   adc     esi, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   mov     dl, BYTE PTR [esi]  /* Get texel*/      \
__asm   add      bx, dvFrac             /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi+5], dl/* Put pixel*/      \
__asm   sbb     ecx, ecx            /* carry flag (vv + dvv)*/  \
__asm   add      ax, bp             /* uu += duu*/      \
__asm   adc     esi, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   mov     dl, BYTE PTR [esi]  /* Get texel*/      \
__asm   add      bx, dvFrac             /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi+6], dl/* Put pixel*/      \
__asm   sbb     ecx, ecx            /* carry flag (vv + dvv)*/  \
__asm   add      ax, bp             /* uu += duu*/      \
__asm   adc     esi, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   mov     dl, BYTE PTR [esi]  /* Get texel*/      \
__asm   add      bx, dvFrac             /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi+7], dl/* Put pixel*/      \
__asm   sbb     ecx, ecx            /* carry flag (vv + dvv)*/  \
__asm   add      ax, bp             /* uu += duu*/      \
__asm   adc     esi, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   add     edi, 8              \
__asm   cmp     edi, s_destend         \
__asm   jbe label0                  \
__asm   jmp label9                  \
                                    \
__asm   label8:                     \
__asm   mov     dl, BYTE PTR [esi]  /* Get texel*/      \
__asm   add      bx, dvFrac             /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi], dl  /* Put pixel*/      \
__asm   sbb     ecx, ecx            /* carry flag (vv + dvv)*/  \
__asm   add      ax, bp             /* uu += duu*/      \
__asm   adc     esi, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   inc     edi                         \
__asm   cmp     edi, s_destend              \
__asm   jbe label8                          \
__asm   label9:                             \
__asm   mov     ebp, oldEBP                 \
__asm   mov     _dest, edi                  \
        }                                   \
    uu = uu1;                               \
    vv = vv1;

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP

#pragma message( "draw_scanline_map_zfil" )
#define NO_draw_scanline_map_zfil
#define SCANFUNC draw_scanline_map_zfil
#define SCANMAP
#define SCANLOOP I386_SCANLINE_MAP8
#define SCANEND \
    do                              \
    {                               \
      *z_buffer++ = izz;            \
      izz += dzz;                   \
    }                               \
    while (z_buffer <= lastZbuf)
#include "cs3d/software/scanline.inc"

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP

#pragma message( "draw_scanline_map_zuse" )
#define NO_draw_scanline_map_zuse
#define SCANFUNC draw_scanline_map_zuse
#define SCANMAP
#define SCANLOOP \
    s = srcTex + ((vv >> 16) << shifter) + (uu >> 16);  \
                                                        \
    dudvInt[1] = ((dvv >> 16) << shifter) + (duu >> 16);\
    dudvInt[0] = dudvInt[1] + (1 << shifter);           \
    uFrac = (short)uu;                                  \
    vFrac = (short)vv;                                  \
    duFrac = (short)duu;                                \
    dvFrac = (short)dvv;                                \
    s_destend = _destend;                                     \
    sdzz   = dzz;                                       \
    sz_buffer = z_buffer;                               \
    /* Sidenote: unrolling loop gives no gain??? (because of jump?) */  \
    __asm {                                 \
__asm   mov     oldEBP, ebp             /* Save EBP*/       \
__asm   mov     ebx, izz                    \
__asm   mov     edi, _dest                  \
__asm   mov     esi, s                      \
__asm   mov      ax, uFrac                  \
__asm   mov      cx, vFrac                  \
__asm   mov     ebp, sz_buffer              \
                                            \
__asm   label0:                             \
__asm   cmp     ebx, DWORD PTR [ebp]    /* Check Z-buffer*/ \
__asm   jb  label1                      /* We're below surface*/    \
__asm   mov     dl, BYTE PTR [esi]      /* Get texel*/      \
__asm   mov     DWORD PTR [ebp], ebx    /* *zbuff = z*/     \
__asm   mov     BYTE PTR [edi], dl      /* Put pixel*/      \
                                            \
__asm   label1:                             \
__asm   add     cx, dvFrac              /* v = v + dv*/     \
__asm   sbb     edx, edx                    \
__asm   add     ax, duFrac              /* u = u + du*/     \
__asm   adc     esi, DWORD PTR [edx*4+dudvInt+4]  /* update texture ptr*/   \
__asm   add     ebx, sdzz               /* z = z + dz*/     \
__asm   inc     edi                     /* dest++*/         \
__asm   add     ebp, 4                  /* zbuff++*/        \
__asm   cmp     edi, s_destend             /* dest < _destend?*/  \
__asm   jbe label0                      \
__asm   mov     sz_buffer, ebp          \
__asm   mov     ebp, oldEBP             \
__asm   mov     _dest,edi               \
__asm   mov     izz, ebx  }             \
    uu = uu1;                           \
    vv = vv1;                           \
    z_buffer = sz_buffer;
#include "cs3d/software/scanline.inc"

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP

#pragma message( "draw_scanline_map_alpha1" )
#define NO_draw_scanline_map_alpha1
#define SCANFUNC draw_scanline_map_alpha1
#define SCANMAP
#define SCANLOOP \
    s = srcTex + ((vv >> 16) << shifter) + (uu >> 16);  \
                                    \
    dudvInt[1] = ((dvv >> 16) << shifter) + (duu >> 16);        \
    dudvInt[0] = dudvInt[1] + (1 << shifter);                 \
    uFrac = (short)uu;                      \
    vFrac = (short)vv;                      \
    duFrac = (short)duu;                    \
    dvFrac = (short)dvv;                    \
    sxx = xx;                               \
    s_destend = _destend;                         \
                                    \
    __asm {                         \
__asm   mov     oldEBP, ebp         /* Save EBP*/       \
__asm   mov     edi, _dest                  \
__asm   mov     esi, AlphaMap              \
__asm   mov      ax, vFrac                  \
__asm   mov      dx, uFrac                  \
__asm   mov     ebp, s              /* EBP = alpha_map*/    \
__asm   xor     ebx, ebx            \
__asm   cmp     sxx, 0              /* Less than 16 pixels?*/   \
__asm   je  label8                  \
                                    \
__asm   label0:                     \
__asm   mov     bl, BYTE PTR [ebp]      /* Get texel*/      \
__asm   mov     bh, BYTE PTR [edi]      /* Get screen pixel*/   \
__asm   mov     bl, BYTE PTR [ebx+esi]  /* Get resulting pixel*/    \
__asm   add     ax, dvFrac              /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi], bl      /* Store pixel*/        \
__asm   sbb     ecx, ecx                /* carry flag (vv+dvv)*/    \
__asm   add     dx, duFrac              /* uu += duu*/      \
__asm   adc     ebp, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   mov     bl, BYTE PTR [ebp]      /* Get texel*/      \
__asm   mov     bh, BYTE PTR [edi+1]    /* Get screen pixel*/   \
__asm   mov     bl, BYTE PTR [ebx+esi]  /* Get resulting pixel*/    \
__asm   add     ax, dvFrac              /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi+1], bl    /* Store pixel*/        \
__asm   sbb     ecx, ecx                /* carry flag (vv+dvv)*/    \
__asm   add     dx, duFrac              /* uu += duu*/      \
__asm   adc     ebp, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   mov     bl, BYTE PTR [ebp]      /* Get texel*/      \
__asm   mov     bh, BYTE PTR [edi+2]    /* Get screen pixel*/   \
__asm   mov     bl, BYTE PTR [ebx+esi]  /* Get resulting pixel*/    \
__asm   add     ax, dvFrac              /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi+2], bl    /* Store pixel*/        \
__asm   sbb     ecx, ecx                /* carry flag (vv+dvv)*/    \
__asm   add     dx, duFrac              /* uu += duu*/ \
__asm   adc     ebp, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   mov     bl, BYTE PTR [ebp]      /* Get texel*/      \
__asm   mov     bh, BYTE PTR [edi+3]    /* Get screen pixel*/   \
__asm   mov     bl, BYTE PTR [ebx+esi]  /* Get resulting pixel*/    \
__asm   add     ax, dvFrac              /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi+3], bl    /* Store pixel*/        \
__asm   sbb     ecx, ecx                /* carry flag (vv+dvv)*/    \
__asm   add     dx, duFrac              /* uu += duu*/ \
__asm   adc     ebp, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   mov     bl, BYTE PTR [ebp]      /* Get texel*/      \
__asm   mov     bh, BYTE PTR [edi+4]    /* Get screen pixel*/   \
__asm   mov     bl, BYTE PTR [ebx+esi]  /* Get resulting pixel*/    \
__asm   add     ax, dvFrac              /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi+4], bl    /* Store pixel*/        \
__asm   sbb     ecx, ecx                /* carry flag (vv+dvv)*/    \
__asm   add     dx, duFrac              /* uu += duu*/ \
__asm   adc     ebp, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   mov     bl, BYTE PTR [ebp]      /* Get texel*/      \
__asm   mov     bh, BYTE PTR [edi+5]    /* Get screen pixel*/   \
__asm   mov     bl, BYTE PTR [ebx+esi]  /* Get resulting pixel*/    \
__asm   add     ax, dvFrac              /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi+5], bl    /* Store pixel*/        \
__asm   sbb     ecx, ecx                /* carry flag (vv+dvv)*/    \
__asm   add     dx, duFrac              /* uu += duu*/ \
__asm   adc     ebp, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   mov     bl, BYTE PTR [ebp]      /* Get texel*/      \
__asm   mov     bh, BYTE PTR [edi+6]    /* Get screen pixel*/   \
__asm   mov     bl, BYTE PTR [ebx+esi]  /* Get resulting pixel*/    \
__asm   add     ax, dvFrac              /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi+6], bl    /* Store pixel*/        \
__asm   sbb     ecx, ecx                /* carry flag (vv+dvv)*/    \
__asm   add     dx, duFrac              /* uu += duu*/ \
__asm   adc     ebp, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   mov     bl, BYTE PTR [ebp]      /* Get texel*/      \
__asm   mov     bh, BYTE PTR [edi+7]    /* Get screen pixel*/   \
__asm   mov     bl, BYTE PTR [ebx+esi]  /* Get resulting pixel*/    \
__asm   add     ax, dvFrac              /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi+7], bl    /* Store pixel*/        \
__asm   sbb     ecx, ecx                /* carry flag (vv+dvv)*/    \
__asm   add     dx, duFrac              /* uu += duu*/ \
__asm   adc     ebp, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   add     edi, 8                  \
__asm   cmp     edi, s_destend             \
__asm   jbe label0                      \
__asm   jmp label9                      \
                                    \
__asm   label8:                     \
__asm   mov     bl, BYTE PTR [ebp]      /* Get texel*/      \
__asm   mov     bh, BYTE PTR [edi]      /* Get screen pixel*/   \
__asm   inc     edi                     /* Increment screen ptr */ \
__asm   mov     bl, BYTE PTR [ebx+esi]  /* Get resulting pixel*/    \
__asm   add     ax, dvFrac              /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi-1], bl    /* Store pixel*/        \
__asm   sbb     ecx, ecx                /* carry flag (vv+dvv)*/    \
__asm   add     dx, duFrac              /* uu += duu*/ \
__asm   adc     ebp, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   cmp     edi, s_destend     /* We finished?*/       \
__asm   jbe label8                      \
                                    \
__asm   label9:                     \
__asm   mov     ebp, oldEBP     /* Restore EBP*/ \
__asm   mov     _dest, edi }    \
    uu = uu1;                   \
    vv = vv1;
#include "cs3d/software/scanline.inc"

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP

#pragma message( "draw_scanline_map_alpha2" )
#define NO_draw_scanline_map_alpha2
#define SCANFUNC draw_scanline_map_alpha2
#define SCANMAP
#define SCANLOOP \
    s = srcTex + ((vv >> 16) << shifter) + (uu >> 16);  \
                                    \
    dudvInt[1] = ((dvv >> 16) << shifter) + (duu >> 16);        \
    dudvInt[0] = dudvInt[1] + (1 << shifter);                 \
    uFrac = (short)uu;              \
    vFrac = (short)vv;              \
    duFrac = (short)duu;            \
    dvFrac = (short)dvv;            \
    sxx = xx;                       \
    s_destend = _destend;                 \
                                    \
    __asm {                         \
__asm   mov     oldEBP, ebp         /* Save EBP*/       \
__asm   mov     edi, _dest          \
__asm   mov     esi, AlphaMap      \
__asm   mov      ax, vFrac          \
__asm   mov      dx, uFrac          \
__asm   mov     ebp, s              /* EBP = alpha_map*/    \
__asm   cmp     sxx, 0              /* Less than 16 pixels?*/   \
__asm   xor     ebx, ebx            \
__asm   je  label8                  \
                                    \
__asm   label0:                     \
__asm   mov     bh, BYTE PTR [ebp]      /* Get texel*/      \
__asm   mov     bl, BYTE PTR [edi]      /* Get screen pixel*/   \
__asm   mov     bl, BYTE PTR [ebx+esi]  /* Get resulting pixel*/    \
__asm   add     ax, dvFrac              /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi], bl      /* Store pixel*/        \
__asm   sbb     ecx, ecx                /* carry flag (vv+dvv)*/    \
__asm   add     dx, duFrac              /* uu += duu*/      \
__asm   adc     ebp, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   mov     bh, BYTE PTR [ebp]      /* Get texel*/      \
__asm   mov     bl, BYTE PTR [edi+1]    /* Get screen pixel*/   \
__asm   mov     bl, BYTE PTR [ebx+esi]  /* Get resulting pixel*/    \
__asm   add     ax, dvFrac              /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi+1], bl    /* Store pixel*/        \
__asm   sbb     ecx, ecx                /* carry flag (vv+dvv)*/    \
__asm   add     dx, duFrac              /* uu += duu*/      \
__asm   adc     ebp, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   mov     bh, BYTE PTR [ebp]      /* Get texel*/      \
__asm   mov     bl, BYTE PTR [edi+2]    /* Get screen pixel*/   \
__asm   mov     bl, BYTE PTR [ebx+esi]  /* Get resulting pixel*/    \
__asm   add     ax, dvFrac              /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi+2], bl    /* Store pixel*/        \
__asm   sbb     ecx, ecx                /* carry flag (vv+dvv)*/    \
__asm   add     dx, duFrac              /* uu += duu*/      \
__asm   adc     ebp, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   mov     bh, BYTE PTR [ebp]      /* Get texel*/      \
__asm   mov     bl, BYTE PTR [edi+3]    /* Get screen pixel*/   \
__asm   mov     bl, BYTE PTR [ebx+esi]  /* Get resulting pixel*/    \
__asm   add     ax, dvFrac              /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi+3], bl    /* Store pixel*/        \
__asm   sbb     ecx, ecx                /* carry flag (vv+dvv)*/    \
__asm   add     dx, duFrac              /* uu += duu*/      \
__asm   adc     ebp, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   mov     bh, BYTE PTR [ebp]      /* Get texel*/      \
__asm   mov     bl, BYTE PTR [edi+4]    /* Get screen pixel*/   \
__asm   mov     bl, BYTE PTR [ebx+esi]  /* Get resulting pixel*/    \
__asm   add     ax, dvFrac              /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi+4], bl    /* Store pixel*/        \
__asm   sbb     ecx, ecx                /* carry flag (vv+dvv)*/    \
__asm   add     dx, duFrac              /* uu += duu*/      \
__asm   adc     ebp, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   mov     bh, BYTE PTR [ebp]      /* Get texel*/      \
__asm   mov     bl, BYTE PTR [edi+5]    /* Get screen pixel*/   \
__asm   mov     bl, BYTE PTR [ebx+esi]  /* Get resulting pixel*/    \
__asm   add     ax, dvFrac              /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi+5], bl    /* Store pixel*/        \
__asm   sbb     ecx, ecx                /* carry flag (vv+dvv)*/    \
__asm   add     dx, duFrac              /* uu += duu*/      \
__asm   adc     ebp, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   mov     bh, BYTE PTR [ebp]      /* Get texel*/      \
__asm   mov     bl, BYTE PTR [edi+6]    /* Get screen pixel*/   \
__asm   mov     bl, BYTE PTR [ebx+esi]  /* Get resulting pixel*/    \
__asm   add     ax, dvFrac              /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi+6], bl    /* Store pixel*/        \
__asm   sbb     ecx, ecx                /* carry flag (vv+dvv)*/    \
__asm   add     dx, duFrac              /* uu += duu*/      \
__asm   adc     ebp, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   mov     bh, BYTE PTR [ebp]      /* Get texel*/      \
__asm   mov     bl, BYTE PTR [edi+7]    /* Get screen pixel*/   \
__asm   mov     bl, BYTE PTR [ebx+esi]  /* Get resulting pixel*/    \
__asm   add     ax, dvFrac              /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi+7], bl    /* Store pixel*/        \
__asm   sbb     ecx, ecx                /* carry flag (vv+dvv)*/    \
__asm   add     dx, duFrac              /* uu += duu*/      \
__asm   adc     ebp, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   add     edi, 8              \
__asm   cmp     edi, s_destend         \
__asm   jbe label0                  \
__asm   jmp label9                  \
                                    \
__asm   label8:                     \
__asm   mov     bh, BYTE PTR [ebp]      /* Get texel*/      \
__asm   mov     bl, BYTE PTR [edi]      /* Get screen pixel*/   \
__asm   inc     edi                     /* Increment screen ptr*/   \
__asm   mov     bl, BYTE PTR [ebx+esi]  /* Get resulting pixel*/    \
__asm   add     ax, dvFrac              /* vv += dvv*/      \
__asm   mov     BYTE PTR [edi-1], bl    /* Store pixel*/        \
__asm   sbb     ecx, ecx                /* carry flag (vv+dvv)*/    \
__asm   add     dx, duFrac              /* uu += duu*/      \
__asm   adc     ebp, DWORD PTR [ecx*4+dudvInt+4]  /* update texture ptr*/   \
                                    \
__asm   cmp     edi, s_destend     /* We finished?*/       \
__asm   jbe label8              \
                                \
__asm   label9:                 \
__asm   mov     ebp, oldEBP     /* Restore EBP*/ \
__asm   mov     _dest, edi }    \
    uu = uu1;                   \
    vv = vv1;
#include "cs3d/software/scanline.inc"

#if defined (DO_MMX)

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#pragma message( "mmx_draw_scanline_map_zfil" )
#define NO_mmx_draw_scanline_map_zfil
#define SCANFUNC mmx_draw_scanline_map_zfil
#define SCANMAP
#define SCANLOOP I386_SCANLINE_MAP8
#define SCANEND MMX_FILLZBUFFER
#include "cs3d/software/scanline.inc"

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP

#pragma message( "mmx_draw_scanline_tex_zfil" )
#define NO_mmx_draw_scanline_tex_zfil
#define SCANFUNC mmx_draw_scanline_tex_zfil
#define SCANLOOP \
    do									\
    {									\
      *_dest++ = COLORMAP [srcTex [((uu >> 16) & ander_w) +		\
        ((vv >> shifter_h) & ander_h)]];				\
      uu += duu;							\
      vv += dvv;							\
    }									\
    while (_dest <= _destend)
#define SCANEND MMX_FILLZBUFFER
#include "cs3d/software/scanline.inc"

#endif // DO_MMX

#pragma message( "draw_pi_scanline_tex_zuse" )
#define NO_draw_pi_scanline_tex_zuse
void csScan_8_draw_pi_scanline_tex_zuse (void *dest, int len,
  unsigned long *zbuff, long u, long du, long v, long dv,
  unsigned long z, long dz, unsigned char *bitmap, int bitmap_log2w)
{
  if (len <= 0)
    return;

  s_destend = ((unsigned char *)dest) + len;
  s = bitmap + ((v >> 16) << bitmap_log2w) + (u >> 16);

  dudvInt[1] = ((dv >> 16) << bitmap_log2w) + (du >> 16);
  dudvInt[0] = dudvInt [1] + (1 << bitmap_log2w);
  uFrac = (short)u;
  vFrac = (short)v;
  duFrac = (short)du;
  dvFrac = (short)dv;
  sz_buffer = (unsigned long *)zbuff;
  sdzz = dz;

  // Sidenote: unrolling loop gives no gain??? (because of jump?)
  __asm {
        mov     oldEBP, ebp     ; Save EBP
        mov     edi, dest
        mov     ebx, z
        mov     esi, s
        mov      ax, vFrac
        mov      cx, uFrac

        mov     ebp, sz_buffer

label0: cmp     ebx, DWORD PTR [ebp]        ; Check Z-buffer
        jb  label1                          ; We're below surface
        mov     dl, BYTE PTR [esi]          ; Get texel
        mov     DWORD PTR [ebp], ebx        ; *zbuff = z
        mov     BYTE PTR [edi], dl          ; Put pixel

label1: add     ax, dvFrac                  ; v = v + dv
        sbb     edx, edx
        add     cx, duFrac                  ; u = u + du
        adc     esi, DWORD PTR [edx*4+dudvInt+4]  ; update texture ptr
        add     ebx, sdzz                   ; z = z + dz
        inc     edi                         ; dest++
        add     ebp, 4                      ; zbuff++
        cmp     edi, s_destend                 ; dest < _destend?
        jb  label0
        mov     ebp, oldEBP
        }
}

#endif // __SCANLN8_H__
