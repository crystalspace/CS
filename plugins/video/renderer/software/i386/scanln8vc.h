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
#  include "cs3d/software/i386/mmx.h"
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

#pragma message( "draw_scanline_map" )
#define NO_draw_scanline_map
#define SCANFUNC draw_scanline_map
#define SCANMAP 1
#define SCANLOOP I386_SCANLINE_MAP8
#define SCANEND \
    do                              \
    {                               \
      *z_buffer++ = izz;            \
      izz += dzz;                   \
    }                               \
    while (z_buffer <= lastZbuf)
#include "render/software/scanline.inc"

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP

#pragma message( "draw_scanline_z_buf_map" )
#define NO_draw_scanline_z_buf_map
#define SCANFUNC draw_scanline_z_buf_map
#define SCANMAP 1
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
#include "render/software/scanline.inc"

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP

#pragma message( "draw_scanline_map_alpha1" )
#define NO_draw_scanline_map_alpha1
#define SCANFUNC draw_scanline_map_alpha1
#define SCANMAP 1
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
__asm   mov     esi, alpha_map              \
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
#include "render/software/scanline.inc"

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP

#pragma message( "draw_scanline_map_alpha2" )
#define NO_draw_scanline_map_alpha2
#define SCANFUNC draw_scanline_map_alpha2
#define SCANMAP 1
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
__asm   mov     esi, alpha_map      \
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
#include "render/software/scanline.inc"

#if defined (DO_MMX)

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#pragma message( "mmx_draw_scanline_map" )
#define SCANFUNC mmx_draw_scanline_map
#define SCANMAP 1
#define SCANLOOP I386_SCANLINE_MAP8
#define SCANEND MMX_FILLZBUFFER
#include "render/software/scanline.inc"
#define NO_mmx_draw_scanline_map

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP

#pragma message( "mmx_draw_scanline" )
#define SCANFUNC mmx_draw_scanline
#define SCANLOOP \
    do                                  \
    {                                   \
      *_dest++ = srcTex[((uu>>16)&ander_w) + ((vv>>shifter_h)&ander_h)];   \
      uu += duu;                            \
      vv += dvv;                            \
    }                                   \
    while (_dest <= _destend)
#define SCANEND MMX_FILLZBUFFER
#include "render/software/scanline.inc"
#define NO_mmx_draw_scanline

#endif // DO_MMX

#define NO_draw_pi_scanline
#pragma message( "draw_pi_scanline" )
void Scan::draw_pi_scanline (void *dest, int len, long *zbuff, long u, long du,
  long v, long dv, long z, long dz, unsigned char *bitmap, int bitmap_log2w)
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

#ifdef DO_MMX

void Scan::mmx_draw_pi_scanline (void *dest, int len, long *zbuff, long u, long du,
  long v, long dv, long z, long dz, unsigned char *bitmap, int bitmap_log2w)
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

  __asm {
        mov     oldEBP, ebp     ; Save EBP
        mov     ecx, z          ; ECX = z
        mov     edi, dest
        mov     esi, s
        mov      ax, vFrac
        mov      bx, uFrac
        add     edi, 4          ; Increment edi
        mov     ebp, sz_buffer  ; EBP = zbuff

        cmp     edi, s_destend     ; Less than 4 pixels left?
        ja  label5              ; Go by one

        ; Load MMX registers
        movd    mm1, sdzz           ; mm1 = 0 | dz
        movd    mm0, ecx            ; mm0 = 0 | z0
        punpckldq mm1, mm1          ; mm1 = dz | dz
        movq    mm2, mm0            ; mm2 = 0 | z0
        paddd   mm2, mm1            ; mm2 = dz | z1
        punpckldq mm0, mm2          ; mm0 = z1 | z0
        pslld   mm1, 1              ; mm1 = dz*2 | dz*2

; The following is somewhat hard to understand, especially without my proprietary
; GAS syntax highlight {tm} {R} :-) I've interleaved MMX instructions with
; non-MMX instructions to achieve maximal percent of paired commands; even
; commands are MMX, odd commands are non-MMX. Since MMX commands never affects
; flags register, we shouldn't care about flags and so on.
label0: movq    mm3, DWORD PTR [ebp]    ; mm3 = bz1 | bz0
        mov     ch, BYTE PTR [esi]  ; Get texel 0
        movq    mm5, mm0                ; mm5 = z1 | z0
        add     ax, dvFrac          ; v = v + dv
        movq    mm4, mm0                ; mm4 = z1 | z0
        sbb     edx, edx
        pcmpgtd mm5, mm3                ; mm5 = m1 | m0
        add     bx, duFrac          ; u = u + du
        paddd   mm0, mm1                ; mm0 = z3 | z2
        adc     esi, DWORD PTR [edx*4+dudvInt+4]  ; update texture ptr
        movq    mm6, mm5                ; mm6 = m1 | m0
        mov     cl, BYTE PTR [esi]  ; Get texel 1
        pandn   mm5, mm3                ; mm5 = ?bz1 | ?bz0
        add     ax, dvFrac          ; v = v + dv
        movq    mm3, QWORD PTR [ebp+8]  ; mm3 = bz3 | bz2
        sbb     edx, edx
        pand    mm4, mm6                ; mm4 = ?z1 | ?z0
        shl     ecx, 16             ; ECX = p1 | p0 | 0 | 0
        movq    mm7, mm0                ; mm7 = z3 | z2
        add     bx, duFrac          ; u = u + du
        por     mm5, mm4                ; mm5 = nz1 | nz0
        adc     esi, DWORD PTR [edx*4+dudvInt+4]  ; update texture ptr
        movq    mm4, mm0                ; mm4 = z3 | z2
        mov     cl, BYTE PTR [esi]  ; Get texel 2
        movq    QWORD PTR [ebp], mm5    ; put nz1 | nz0 into z-buffer
        add     ax, dvFrac          ; v = v + dv
        pcmpgtd mm7, mm3                ; mm7 = m3 | m2
        sbb     edx, edx
        movq    mm5, mm7                ; mm5 = m3 | m2
        add     bx, duFrac          ; u = u + du
        paddd   mm0, mm1                ; mm0 = z5 | z4
        adc     esi, DWORD PTR [edx*4+dudvInt+4]  ; update texture ptr
        pandn   mm5, mm3                ; mm5 = ?bz3 | ?bz2
        mov     ch, BYTE PTR [esi]  ; Get texel 3
        pand    mm4, mm7                ; mm4 = ?z3 | ?z2
        add     ax, dvFrac          ; v = v + dv
        packssdw mm7, mm6               ; mm7 = m3 | m2 | m1 | m0
        sbb     edx, edx
        por     mm5, mm4                ; mm5 = nz3 | nz2
        add     bx, duFrac          ; u = u + du
        packsswb mm7, mm7               ; mm7 = 0 | 0 | 0 | 0 | m3 | m2 | m1 | m0
        adc     esi, DWORD PTR [edx*4+dudvInt+4]  ; update texture ptr
        movq    QWORD PTR [ebp+8], mm5  ; put nz3 | nz2 into z-buffer
        rol     ecx, 16             ; ECX = p3 | p2 | p1 | p0

;----- end of interleaved code

        add     ebp, 16         ; Increment zbuf pointer

        movd    edx, mm7        ; EDX = m3 | m2 | m1 | m0
        and     ecx, edx        ; Leave only visible texels
        not     edx             ; Negate Z-mask
        and     edx, DWORD PTR [edi-4]  ; Get on-screen pixels
        or      edx, ecx        ; Merge two sets of pixels together
        add     edi, 4          ; Increment dest pointer
        mov     DWORD PTR [edi-8], edx  ; Put pixels into framebuffer
        cmp     edi, s_destend     ; dest < _destend?
        jbe label0

        ; Less than four pixels left
        movd    ecx, mm0

label5: sub     edi, 4
        cmp     edi, s_destend     ; dest >=_destend?
        jae label8

label6: cmp     ecx, DWORD PTR [ebp]
        jb  label7
        mov     dl, BYTE PTR [esi]  ; Get texel
        mov     DWORD PTR [ebp], ecx    ; *zbuff = z
        mov     BYTE PTR [edi], dl      ; Put texel

label7: add     ax, dvFrac          ; v = v + dv
        sbb     edx, edx
        add     bx, duFrac          ; u = u + du
        adc     esi, DWORD PTR [edx*4+dudvInt+4]  ; update texture ptr
        add     ecx, sdzz           ; z = z + dz
        inc     edi             ; dest++
        add     ebp, 4          ; zbuff++
        cmp     edi, s_destend     ; dest < _destend?
        jb  label6

label8: mov     ebp, oldEBP
        emms }
}

#endif // DO_MMX

#endif // __SCANLN8_H__
