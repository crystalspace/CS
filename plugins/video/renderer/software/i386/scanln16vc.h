/*
    Crystal Space 16-bit software driver assembler-optimized routines
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

#ifndef __SCANLN16_H__
#define __SCANLN16_H__

#if defined (DO_MMX)
#  include "cs3d/software/i386/mmx.h"
#endif

static ULong Frac, dFrac;
static unsigned int oldEBP;
static unsigned dudvInt[4];
static UShort *s;
static int sxx;
static UShort *s_destend;
static long sdzz;
static unsigned long *sz_buffer;

#define I386_SCANLINE_MAP16 \
    s = srcTex + ((vv >> 16) << shifter) + (uu >> 16);              \
                                                                    \
    dudvInt[1] = (((dvv >> 16) << shifter) + (duu>>16)) * 2;        \
    dudvInt[3] = dudvInt[1] + 2;                                    \
    dudvInt[0] = dudvInt[1] + (1 << shifter) * 2;                   \
    dudvInt[2] = dudvInt[0] + 2;                                    \
    Frac  = (uu << 16) | ((vv >> 1) & 0x7FFF);                      \
    dFrac = (duu<< 16) | ((dvv>> 1) & 0x7FFF);                      \
__asm {                                     \
__asm   mov     edi, _dest                  \
__asm   mov     esi, s                      \
__asm   mov     eax, Frac                   \
__asm   mov     ebx, dFrac                  \
__asm   xor     ecx, ecx                    \
__asm   cmp     xx, 0                       \
__asm   je  label8                          \
                                            \
__asm   label0:                             \
__asm   add     eax, ebx                    /*1 cycle*/\
__asm   setc    cl                          /*2 cycles*/\
__asm   mov     dx, WORD PTR [esi]          /*2 cycles*/\
__asm   cmp     ah, 80h                     \
__asm   adc     ecx,ecx                     /*1 cycle*/\
__asm   and     eax, 0xffff7fff             \
__asm   shl     edx,16                      /*3 cycles*/\
__asm   add     esi, DWORD PTR [ecx*4+dudvInt]\
                                            \
__asm   add     eax, ebx                    /*1 cycle*/\
__asm   setc    cl                          /*2 cycles*/\
__asm   mov     dx, WORD PTR [esi]          /*2 cycles*/\
__asm   cmp     ah, 80h                     \
__asm   adc     ecx,ecx                     /*1 cycle*/\
/*__asm   and       eax, 0xffff7fff */\
__asm   mov     DWORD PTR [edi], edx        /*3 cycles*/\
__asm   and     eax, 0xffff7fff             /*1 cycle, should ease off memory and may pair~Conor*/ \
__asm   add     esi, DWORD PTR [ecx*4+dudvInt]\
                                            \
__asm   add     eax, ebx                    /*1 cycle*/\
__asm   setc    cl                          /*2 cycles*/\
__asm   mov     dx, WORD PTR [esi]          /*2 cycles*/\
__asm   cmp     ah, 80h                     \
__asm   adc     ecx,ecx                     /*1 cycle*/\
__asm   and     eax, 0xffff7fff             \
__asm   shl     edx,16                      /*3 cycles*/\
__asm   add     esi, DWORD PTR [ecx*4+dudvInt]\
                                            \
__asm   add     eax, ebx                    /*1 cycle*/\
__asm   setc    cl                          /*2 cycles*/\
__asm   mov     dx, WORD PTR [esi]          /*2 cycles*/\
__asm   cmp     ah, 80h                     \
__asm   adc     ecx,ecx                     /*1 cycle*/\
__asm   mov     DWORD PTR [edi+4], edx      /*3 cycles*/\
__asm   and     eax, 0xffff7fff             \
__asm   add     esi, DWORD PTR [ecx*4+dudvInt]\
                                            \
__asm   add     eax, ebx                    /*1 cycle*/\
__asm   setc    cl                          /*2 cycles*/\
__asm   mov     dx, WORD PTR [esi]          /*2 cycles*/\
__asm   cmp     ah, 80h                     \
__asm   adc     ecx,ecx                     /*1 cycle*/\
__asm   and     eax, 0xffff7fff             \
__asm   shl     edx,16                      /*3 cycles*/\
__asm   add     esi, DWORD PTR [ecx*4+dudvInt]\
                                    \
__asm   add     eax, ebx                    /*1 cycle*/\
__asm   setc    cl                          /*2 cycles*/\
__asm   mov     dx, WORD PTR [esi]          /*2 cycles*/\
__asm   cmp     ah, 80h                     \
__asm   adc     ecx,ecx                     /*1 cycle*/\
__asm   mov     DWORD PTR [edi+8], edx      /*3 cycles*/\
__asm   and     eax, 0xffff7fff             \
__asm   add     esi, DWORD PTR [ecx*4+dudvInt]\
                                            \
__asm   add     eax, ebx                    /*1 cycle*/\
__asm   setc    cl                          /*2 cycles*/\
__asm   mov     dx, WORD PTR [esi]          /*2 cycles*/\
__asm   cmp     ah, 80h                     \
__asm   adc     ecx,ecx                     /*1 cycle*/\
__asm   and     eax, 0xffff7fff             \
__asm   shl     edx,16                      /*3 cycles*/\
__asm   add     esi, DWORD PTR [ecx*4+dudvInt]\
                                            \
__asm   add     eax, ebx                    /*1 cycle*/\
__asm   setc    cl                          /*2 cycles*/\
__asm   mov     dx, WORD PTR [esi]          /*2 cycles*/\
__asm   cmp     ah, 80h                     \
__asm   adc     ecx,ecx                     /*1 cycle*/\
__asm   mov     DWORD PTR [edi+12], edx     /*3 cycles*/\
__asm   and     eax, 0xffff7fff             \
__asm   add     esi, DWORD PTR [ecx*4+dudvInt]\
                                            \
__asm   add     edi,16                      \
__asm   cmp     edi,[_destend]              /*Compare from memory?*/\
__asm   jbe label0                          \
__asm   jmp     label9                      \
                                            \
__asm   label8:                             \
__asm   add     eax, ebx                    /*1 cycle*/\
__asm   setc    cl                          /*2 cycles*/\
__asm   mov     dx, WORD PTR [esi]          /*2 cycles*/\
__asm   cmp     ah, 80h                     \
__asm   adc     ecx,ecx                     /*1 cycle*/\
__asm   mov     WORD PTR [edi], dx          /*3 cycles*/\
__asm   and     eax, 0xffff7fff             \
__asm   add     esi, DWORD PTR [ecx*4+dudvInt]\
                                            \
__asm   add     edi,2                       \
__asm   cmp     edi,_destend                \
__asm   jbe label8                          \
__asm   label9:                             \
__asm   mov     _dest,edi                   \
}                                           \
    uu = uu1;                               \
    vv = vv1;

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define NO_draw_scanline_map
#define SCANFUNC draw_scanline_map
#define SCANMAP 1
#define SCANLOOP I386_SCANLINE_MAP16
#define SCANEND \
    do                              \
    {                               \
      *z_buffer++ = izz;            \
      izz += dzz;                   \
    }                               \
    while (z_buffer <= lastZbuf)

#include "cs3d/software/scanln16.inc"

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define NO_draw_scanline_z_buf_map
#define SCANFUNC draw_scanline_z_buf_map
#define SCANMAP 1
#define SCANLOOP                                            \
    s = srcTex + ((vv >> 16) << shifter) + (uu >> 16);      \
                                                            \
    dudvInt[1] = (((dvv >> 16) << shifter) + (duu>>16)) * 2;\
    dudvInt[3] = dudvInt[1] + 2;                            \
    dudvInt[0] = dudvInt[1] + (1 << shifter) * 2;           \
    dudvInt[2] = dudvInt[0] + 2;                            \
    Frac  = (uu << 16) | ((vv >> 1) & 0x7FFF);              \
    dFrac = (duu<< 16) | ((dvv>> 1) & 0x7FFF);              \
    s_destend = _destend;                         \
    sdzz   = dzz;                           \
    sz_buffer = z_buffer;                   \
                                            \
    /* Sidenote: unrolling loop gives no gain??? (because of jump?) */  \
__asm {                                     \
__asm   mov     edi, _dest                  \
__asm   mov     ecx, izz                    \
__asm   mov     esi, s                      \
__asm   mov     eax, Frac                   \
__asm   mov     ebx, dFrac                  \
__asm   mov     oldEBP, ebp                 \
__asm   mov     ebp,sz_buffer               \
                                    \
__asm   label0:                     \
__asm   cmp     ecx,DWORD PTR[ebp]      /* Check Z-buffer*/ \
__asm   jb      label1          /* We're below surface*/    \
__asm   mov     dx, WORD PTR [esi]      /* Get texel*/      \
__asm   mov     DWORD PTR [ebp], ecx    /* *zbuff = z*/     \
__asm   mov     WORD PTR [edi],dx       /* Put texel*/      \
                                    \
__asm   label1:                     \
__asm   add     eax, ebx                    /*1 cycle*/\
__asm   setc    dl                          /*2 cycles*/\
__asm   add     ecx,sdzz                /* z = z + dz*/     \
__asm   cmp     ah, 80h                     \
__asm   adc     edx,edx                     /*1 cycle*/\
__asm   and     eax, 0xffff7fff             \
__asm   add     edi,2                   /* dest++*/     \
__asm   add     ebp,4                   /* zbuff++*/        \
__asm   add     esi, DWORD PTR [edx*4+dudvInt]\
__asm   cmp     edi,s_destend              /* dest < _destend?*/      \
__asm   jbe label0                      \
__asm   mov     sz_buffer,ebp           \
__asm   mov     ebp,oldEBP              \
__asm   mov     izz,ecx                 \
__asm   mov     _dest,edi      }        \
    uu = uu1;                           \
    vv = vv1;                           \
    z_buffer = sz_buffer;

#include "cs3d/software/scanln16.inc"

#define I386_SCANLINE_MAP_ALPHA50_16                        \
    static UShort alpha = Textures::alpha_mask;             \
    s = srcTex + ((vv >> 16) << shifter) + (uu >> 16);      \
    dudvInt[1] = (((dvv >> 16) << shifter) + (duu>>16)) * 2;\
    dudvInt[3] = dudvInt[1] + 2;                            \
    dudvInt[0] = dudvInt[1] + (1 << shifter) * 2;           \
    dudvInt[2] = dudvInt[0] + 2;                            \
    Frac  = (uu << 16) | ((vv >> 1) & 0x7FFF);              \
    dFrac = (duu<< 16) | ((dvv>> 1) & 0x7FFF);              \
    sxx = xx;                               \
    s_destend = _destend;                         \
                                            \
__asm {                                     \
__asm   mov     edi, _dest                  \
__asm   mov     esi, s                      \
__asm   mov     eax, Frac                   \
__asm   mov     ebx, dFrac                  \
__asm   xor     ecx, ecx                    \
__asm   mov     oldEBP, ebp                 \
                                    \
__asm   cmp     sxx, 0              \
__asm   je  label8                  \
                                    \
__asm   label0:                     \
__asm   mov     dx, WORD PTR [esi]      /* Get texel*/      \
__asm   mov     bp, WORD PTR [edi]      /* Get pixel*/      \
__asm   and     dx,alpha                /* prepare to divide by 2*/\
__asm   and     bp,alpha                /* prepare to divide by 2*/\
__asm   shr     dx,1                    /* texel/2*/        \
__asm   shr     bp,1                    /* pixel/2*/        \
__asm   add     dx,bp                   /* texel/2 + pixel/2*/  \
__asm   add     eax, ebx                /*1 cycle*/\
__asm   setc    cl                      /*2 cycles*/\
__asm   shl     edx, 16                 \
__asm   cmp     ah, 80h                 \
__asm   adc     ecx,ecx                 /*1 cycle*/\
__asm   and     eax, 0xffff7fff         \
__asm   add     esi, DWORD PTR [ecx*4+dudvInt]\
                                    \
__asm   mov     dx, WORD PTR [esi]      /* Get texel*/      \
__asm   mov     bp, WORD PTR [edi]      /* Get pixel*/      \
__asm   and     dx,alpha                /* prepare to divide by 2*/\
__asm   and     bp,alpha                /* prepare to divide by 2*/\
__asm   shr     dx,1                /* texel/2*/        \
__asm   shr     bp,1                /* pixel/2*/        \
__asm   add     dx,bp               /* texel/2 + pixel/2*/  \
__asm   add     eax, ebx                    /*1 cycle*/\
__asm   setc    cl                          /*2 cycles*/\
__asm   mov     DWORD PTR [edi], edx/* Put pixel*/      \
__asm   cmp     ah, 80h                     \
__asm   adc     ecx,ecx                     /*1 cycle*/\
__asm   and     eax, 0xffff7fff             \
__asm   add     esi, DWORD PTR [ecx*4+dudvInt]\
                                \
__asm   mov     dx, WORD PTR [esi]      /* Get texel*/      \
__asm   mov     bp, WORD PTR [edi]      /* Get pixel*/      \
__asm   and     dx,alpha                /* prepare to divide by 2*/\
__asm   and     bp,alpha                /* prepare to divide by 2*/\
__asm   shr     dx,1                /* texel/2*/        \
__asm   shr     bp,1                /* pixel/2*/        \
__asm   add     dx,bp               /* texel/2 + pixel/2*/  \
__asm   add     eax, ebx                    /*1 cycle*/\
__asm   setc    cl                          /*2 cycles*/\
__asm   shl     edx,16                      \
__asm   cmp     ah, 80h                     \
__asm   adc     ecx,ecx                     /*1 cycle*/\
__asm   and     eax, 0xffff7fff             \
__asm   add     esi, DWORD PTR [ecx*4+dudvInt]\
                                \
__asm   mov     dx, WORD PTR [esi]      /* Get texel*/      \
__asm   mov     bp, WORD PTR [edi]      /* Get pixel*/      \
__asm   and     dx,alpha                /* prepare to divide by 2*/\
__asm   and     bp,alpha                /* prepare to divide by 2*/\
__asm   shr     dx,1                /* texel/2*/        \
__asm   shr     bp,1                /* pixel/2*/        \
__asm   add     dx,bp               /* texel/2 + pixel/2*/  \
__asm   add     eax, ebx                    /*1 cycle*/\
__asm   setc    cl                          /*2 cycles*/\
__asm   mov     DWORD PTR [edi+4], edx/* Put pixel*/        \
__asm   cmp     ah, 80h                     \
__asm   adc     ecx,ecx                     /*1 cycle*/\
__asm   and     eax, 0xffff7fff             \
__asm   add     esi, DWORD PTR [ecx*4+dudvInt]\
                                    \
__asm   mov     dx, WORD PTR [esi]      /* Get texel*/      \
__asm   mov     bp, WORD PTR [edi]      /* Get pixel*/      \
__asm   and     dx,alpha                /* prepare to divide by 2*/\
__asm   and     bp,alpha                /* prepare to divide by 2*/\
__asm   shr     dx,1                /* texel/2*/        \
__asm   shr     bp,1                /* pixel/2*/        \
__asm   add     dx,bp               /* texel/2 + pixel/2*/  \
__asm   add     eax, ebx                    /*1 cycle*/\
__asm   setc    cl                          /*2 cycles*/\
__asm   shl     edx,16                      \
__asm   cmp     ah, 80h                     \
__asm   adc     ecx,ecx                     /*1 cycle*/\
__asm   and     eax, 0xffff7fff             \
__asm   add     esi, DWORD PTR [ecx*4+dudvInt]\
                                    \
__asm   mov     dx, WORD PTR [esi]      /* Get texel*/      \
__asm   mov     bp, WORD PTR [edi]      /* Get pixel*/      \
__asm   and     dx,alpha                /* prepare to divide by 2*/\
__asm   and     bp,alpha                /* prepare to divide by 2*/\
__asm   shr     dx,1                /* texel/2*/        \
__asm   shr     bp,1                /* pixel/2*/        \
__asm   add     dx,bp               /* texel/2 + pixel/2*/  \
__asm   add     eax, ebx                    /*1 cycle*/\
__asm   setc    cl                          /*2 cycles*/\
__asm   mov     DWORD PTR [edi+8], edx/* Put pixel*/        \
__asm   cmp     ah, 80h                     \
__asm   adc     ecx,ecx                     /*1 cycle*/\
__asm   and     eax, 0xffff7fff             \
__asm   add     esi, DWORD PTR [ecx*4+dudvInt]\
                                    \
__asm   mov     dx, WORD PTR [esi]      /* Get texel*/      \
__asm   mov     bp, WORD PTR [edi]      /* Get pixel*/      \
__asm   and     dx,alpha                /* prepare to divide by 2*/\
__asm   and     bp,alpha                /* prepare to divide by 2*/\
__asm   shr     dx,1                /* texel/2*/        \
__asm   shr     bp,1                /* pixel/2*/        \
__asm   add     dx,bp               /* texel/2 + pixel/2*/  \
__asm   add     eax, ebx                    /*1 cycle*/\
__asm   setc    cl                          /*2 cycles*/\
__asm   shl     edx, 16                     \
__asm   cmp     ah, 80h                     \
__asm   adc     ecx,ecx                     /*1 cycle*/\
__asm   and     eax, 0xffff7fff             \
__asm   add     esi, DWORD PTR [ecx*4+dudvInt]\
                                    \
__asm   mov     dx, WORD PTR [esi]      /* Get texel*/      \
__asm   mov     bp, WORD PTR [edi]      /* Get pixel*/      \
__asm   and     dx,alpha                /* prepare to divide by 2*/\
__asm   and     bp,alpha                /* prepare to divide by 2*/\
__asm   shr     dx,1                /* texel/2*/        \
__asm   shr     bp,1                /* pixel/2*/        \
__asm   add     dx,bp               /* texel/2 + pixel/2*/  \
__asm   add     eax, ebx                    /*1 cycle*/\
__asm   setc    cl                          /*2 cycles*/\
__asm   mov     DWORD PTR [edi+12], edx/* Put pixel*/       \
__asm   cmp     ah, 80h                     \
__asm   adc     ecx,ecx                     /*1 cycle*/\
__asm   and     eax, 0xffff7fff             \
__asm   add     esi, DWORD PTR [ecx*4+dudvInt]\
                                    \
__asm   add     edi,16                  \
__asm   cmp     edi,s_destend              \
__asm   jbe label0                      \
__asm   jmp label9                  \
                                    \
__asm   label8:                     \
__asm   mov     dx, WORD PTR [esi]      /* Get texel*/      \
__asm   mov     bp, WORD PTR [edi]      /* Get pixel*/      \
__asm   and     dx,alpha                /* prepare to divide by 2*/\
__asm   and     bp,alpha                /* prepare to divide by 2*/\
__asm   shr     dx,1                /* texel/2*/        \
__asm   shr     bp,1                /* pixel/2*/        \
__asm   add     dx,bp               /* texel/2 + pixel/2*/  \
__asm   add     eax, ebx                    /*1 cycle*/\
__asm   setc    cl                          /*2 cycles*/\
__asm   mov     WORD PTR [edi], dx/* Put pixel*/        \
__asm   cmp     ah, 80h                     \
__asm   adc     ecx,ecx                     /*1 cycle*/\
__asm   and     eax, 0xffff7fff             \
__asm   add     esi, DWORD PTR [ecx*4+dudvInt]\
                                \
__asm   add     edi,2       /* increment dest ptr*/ \
__asm   cmp     edi,s_destend      /* we're finished?*/    \
__asm   jbe label8                      \
__asm   label9:                     \
__asm   mov     ebp,oldEBP      /* restore EBP*/        \
__asm   mov     _dest, edi         }                \
    uu = uu1;                                       \
    vv = vv1;

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP


#if 0	// commented out since it looks like Jorrit changed the things a bit
#pragma message( "draw_scanline_map_alpha25" )
#define NO_draw_scanline_map_alpha25
#define SCANFUNC draw_scanline_map_alpha25
#define SCANMAP 1
#define SCANLOOP I386_SCANLINE_MAP_ALPHA50_16
#include "cs3d/software/scanln16.inc"

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP

#pragma message( "draw_scanline_map_alpha50" )
#define NO_draw_scanline_map_alpha50
#define SCANFUNC draw_scanline_map_alpha50
#define SCANMAP 1
#define SCANLOOP I386_SCANLINE_MAP_ALPHA50_16
#include "cs3d/software/scanln16.inc"

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP

#define NO_draw_scanline_map_alpha75
#define SCANFUNC draw_scanline_map_alpha75
#define SCANMAP 1
#define SCANLOOP I386_SCANLINE_MAP_ALPHA50_16
#include "cs3d/software/scanln16.inc"
#endif

#if defined (DO_MMX)

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC mmx_draw_scanline_map
#define SCANMAP 1
#define SCANLOOP I386_SCANLINE_MAP16
#define SCANEND MMX_FILLZBUFFER
#include "cs3d/software/scanln16.inc"
#define NO_mmx_draw_scanline_map

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define SCANFUNC mmx_draw_scanline
#define SCANLOOP \
    do                                  \
    {                                   \
      *_dest++ = pal_table[srcTex[((uu>>16)&ander_w) + ((vv>>shifter_h)&ander_h)]];\
      uu += duu;                            \
      vv += dvv;                            \
    }                                   \
    while (_dest <= _destend)
#define SCANEND MMX_FILLZBUFFER
#include "cs3d/software/scanln16.inc"
#define NO_mmx_draw_scanline

#endif

#define NO_draw_pi_scanline
void Scan16::draw_pi_scanline (void *_dest, int len, long *zbuff, long uu, long duu,
  long vv, long dvv, long z, long dz, unsigned char *srcTex, int shifter)
{
  if (len <= 0)
    return;

    static UShort *tbl = pal_table;
    s_destend = ((UShort *)_dest) + len;
    s = ((UShort *)srcTex) + ((vv >> 16) << shifter) + (uu >> 16);
    dudvInt[1] = (((dvv >> 16) << shifter) + (duu>>16)) * 2;
    dudvInt[3] = dudvInt[1] + 2;
    dudvInt[0] = dudvInt[1] + (1 << shifter) * 2;
    dudvInt[2] = dudvInt[0] + 2;
    Frac  = (uu << 16) | ((vv >> 1) & 0x7FFF);
    dFrac = (duu<< 16) | ((dvv>> 1) & 0x7FFF);
    sdzz = dz;
    sz_buffer = (unsigned long *)zbuff;

  // Sidenote: unrolling loop gives no gain???
__asm {
		mov     edi, _dest
		mov     ecx, z
		mov     esi, s
		mov		eax, Frac
		mov     ebx, dFrac
		xor		edx, edx
		mov     oldEBP, ebp
		mov		ebp, sz_buffer

label0:		cmp		ecx, DWORD PTR [ebp]
		jb	label1
/*Do You thrive off cache murder?*/
/*This won't pair! At ALL!*/
//		mov 	DWORD PTR [ebp], ecx	; *zbuff = z
/*EEECHH MOVZX ECHHH!*/
//		movzx 	edx, WORD PTR [esi]		; Get texel
//		mov 	dx,WORD PTR [edx*2+tbl]	; Translate into RGB
//		mov 	WORD PTR [edi],dx		; Put texel
//Okay just an ideas
		movzx 	edx, WORD PTR [esi]		; Get texel
		mov 	DWORD PTR [ebp], ecx	; *zbuff = z
                mov 	dx,WORD PTR [edx*2+tbl]	; Translate into RGB
		mov 	WORD PTR [edi],dx		; Put texel

		xor		edx, edx

label1:	
		add		eax, ebx					/*1 cycle*/
		setc	dl							/*2 cycles*/
		add		ecx,sdzz		; z = z + dz
		cmp		ah, 80h
		adc		edx,edx						/*1 cycle*/
		and		eax, 0xffff7fff
		add		edi, 2			; dest++
		add		ebp, 4			; zbuff++
		add		esi, DWORD PTR [edx*4+dudvInt]
		cmp		edi, s_destend		; dest < lastD?
		jb	label0
		mov		ebp,oldEBP
	}
}

#ifdef DO_MMX

//Okay Please check over my changes! This was done very late at night!

#define NO_mmx_draw_pi_scanline
void Scan16::mmx_draw_pi_scanline (void *dest, int len, long *zbuff, long u, long du,
  long v, long dv, long z, long dz, unsigned char *bitmap, int bitmap_log2w)
{
  // not implemented
  abort ();
}

#endif // DO_MMX

#endif // __SCANLN16_H__
