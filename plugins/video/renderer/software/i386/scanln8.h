/*
    Crystal Space 8-bit software driver assembler-optimized routines
    Copyright (C) 1998 by Jorrit Tyberghein
    Contributors:
       draw_scanline_map by David N. Arnold <derek_arnold@fuse.net>
       MMX support and other by Andrew Zabolotny <bit@eltech.ru>

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

#define I386_SCANLINE_MAP8 \
    int uFrac, duFrac, vFrac, dvFrac;					\
    unsigned int oldEBP;						\
    static int dudvInt[2];						\
    unsigned char *s = srcTex + ((vv >> 16) << shifter) + (uu >> 16);	\
									\
    dudvInt[1] = ((dvv >> 16) << shifter) + (duu >> 16);		\
    dudvInt[0] = dudvInt[1] + (1 << shifter); 				\
    uFrac = uu << 16;							\
    vFrac = vv << 16;							\
    duFrac = duu << 16;							\
    dvFrac = dvv << 16;							\
									\
    asm __volatile__ ("" : : "a" (uFrac), "b" (vFrac), "c" (dvFrac),	\
                      "S" (s), "D" (_dest));				\
    asm __volatile__ ("							\
		movl	%%ebp, %3					\n\
		movb	(%%edi), %%dl					\n\
		movl	%1, %%ebp					\n\
									\n\
		cmpl	$0, %5						\n\
		je	8f						\n\
									\n\
0:		movb	(%%esi), %%dl		# Get texel		\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, (%%edi)		# Put pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv + dvv)	\n\
		addl	%%ebp, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		movb	(%%esi), %%dl		# Get texel		\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, 1(%%edi)		# Put pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv + dvv)	\n\
		addl	%%ebp, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		movb	(%%esi), %%dl		# Get texel		\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, 2(%%edi)		# Put pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv + dvv)	\n\
		addl	%%ebp, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		movb	(%%esi), %%dl		# Get texel		\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, 3(%%edi)		# Put pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv + dvv)	\n\
		addl	%%ebp, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		movb	(%%esi), %%dl		# Get texel		\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, 4(%%edi)		# Put pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv + dvv)	\n\
		addl	%%ebp, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		movb	(%%esi), %%dl		# Get texel		\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, 5(%%edi)		# Put pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv + dvv)	\n\
		addl	%%ebp, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		movb	(%%esi), %%dl		# Get texel		\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, 6(%%edi)		# Put pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv + dvv)	\n\
		addl	%%ebp, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		movb	(%%esi), %%dl		# Get texel		\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, 7(%%edi)		# Put pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv + dvv)	\n\
		addl	%%ebp, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		addl	$8, %%edi					\n\
		cmpl	%2, %%edi					\n\
		jbe	0b 						\n\
		jmp	9f						\n\
									\n\
8:		movb	(%%esi), %%dl		# Get texel		\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, (%%edi)		# Put pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv + dvv)	\n\
		addl	%%ebp, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		incl	%%edi						\n\
		cmpl	%2, %%edi					\n\
		jbe	8b 						\n\
9:		movl	%3, %%ebp"					\
    : "=D" (_dest) 							\
    : "m" (duFrac), "m" (_destend), "m" (oldEBP), "m" (dudvInt[1]),	\
      "m" (xx)								\
    : "%edx");								\
    uu = uu1;								\
    vv = vv1;

#undef SCANFUNC
#undef SCANEND
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define NO_draw_scanline_map_zfil
#define SCANFUNC csScan_8_draw_scanline_map_zfil
#define SCANMAP 1
#define SCANLOOP I386_SCANLINE_MAP8
#define SCANEND \
    do								\
    {								\
      *z_buffer++ = izz;					\
      izz += dzz;						\
    }								\
    while (z_buffer <= lastZbuf)
#include "cs3d/software/scanln.inc"

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define NO_draw_scanline_map_zuse
#define SCANFUNC csScan_8_draw_scanline_map_zuse
#define SCANMAP 1
#define SCANLOOP \
    int uFrac, duFrac, vFrac, dvFrac;					\
    static unsigned int oldEBP;						\
    static int dudvInt[2];						\
    unsigned char *src = srcTex + ((vv >> 16) << shifter) + (uu >> 16);	\
									\
    dudvInt[1] = ((dvv >> 16) << shifter) + (duu >> 16);		\
    dudvInt[0] = dudvInt [1] + (1 << shifter);				\
    uFrac = uu << 16;							\
    vFrac = vv << 16;							\
    duFrac = duu << 16;							\
    dvFrac = dvv << 16;							\
									\
    asm __volatile__ ("" : : "a" (uFrac), "b" (vFrac), "c" (izz),	\
                      "S" (src), "D" (_dest));				\
    /* Sidenote: unrolling loop gives no gain??? (because of jump?) */	\
    asm __volatile__ ("							\
  		movl	%%ebp, %4		# Save EBP		\n\
  		movl	%7, %%ebp					\n\
									\n\
0:		cmpl	(%%ebp), %%ecx		# Check Z-buffer	\n\
  		jb	1f			# We're below surface	\n\
  		movb	(%%esi), %%dl		# Get texel		\n\
  		movl	%%ecx, (%%ebp)		# *zbuff = z		\n\
  		movb	%%dl, (%%edi)		# Put texel		\n\
									\n\
1:		addl	%2, %%ebx		# v = v + dv		\n\
  		sbbl	%%edx, %%edx					\n\
  		addl	%1, %%eax		# u = u + du		\n\
  		adcl	%5(,%%edx,4), %%esi	# Update source texture pointer\n\
  		addl	%6, %%ecx		# z = z + dz		\n\
  		incl	%%edi			# dest++		\n\
  		addl	$4, %%ebp		# zbuff++		\n\
  		cmpl	%3, %%edi		# dest < _destend?	\n\
  		jbe	0b						\n\
  		movl	%%ebp, %7					\n\
  		movl	%%ecx, %8
  		movl	%4, %%ebp"					\
    : "=D" (_dest)							\
    : "m" (duFrac), "m" (dvFrac), "m" (_destend), "m" (oldEBP),		\
      "m" (dudvInt[1]), "m" (dzz), "m" (z_buffer), "m" (izz)		\
    : "eax", "%ebx", "ecx", "edx", "esi" );				\
    uu = uu1;								\
    vv = vv1;
#include "cs3d/software/scanln.inc"

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define NO_draw_scanline_map_alpha1
#define SCANFUNC csScan_8_draw_scanline_map_alpha1
#define SCANMAP 1
#define SCANLOOP \
    int uFrac, duFrac, vFrac, dvFrac;					\
    static unsigned int oldEBP;						\
    static int dudvInt[2];						\
    unsigned char *s = srcTex + ((vv>>16)<<shifter) + (uu>>16);		\
									\
    dudvInt[1] = ((dvv >> 16) << shifter) + (duu>>16);			\
    dudvInt[0] = dudvInt[1] + (1 << shifter); 				\
    uFrac = uu << 16;							\
    vFrac = vv << 16;							\
    duFrac = duu << 16;							\
    dvFrac = dvv << 16;							\
									\
    asm __volatile__ ("" : : "a" (uFrac), "b" (vFrac),			\
                      "c" (dvFrac), "S" (s), "D" (_dest));		\
    asm __volatile__ ("							\
		movl	%%ebp, %3		# Save EBP		\n\
		movl	%6, %%ebp		# EBP = alpha_map	\n\
		cmpl	$0, %5			# Less than 16 pixels?	\n\
		je	8f						\n\
									\n\
0:		movzbl	(%%esi), %%edx		# Get texel		\n\
		movb	(%%edi), %%dh		# Get screen pixel	\n\
		movb	(%%ebp, %%edx), %%dl	# Get resulting pixel	\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, (%%edi)		# Store pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv+dvv)	\n\
		addl	%1, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		movzbl	(%%esi), %%edx		# Get texel		\n\
		movb	1(%%edi), %%dh		# Get screen pixel	\n\
		movb	(%%ebp, %%edx), %%dl	# Get resulting pixel	\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, 1(%%edi)		# Store pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv+dvv)	\n\
		addl	%1, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		movzbl	(%%esi), %%edx		# Get texel		\n\
		movb	2(%%edi), %%dh		# Get screen pixel	\n\
		movb	(%%ebp, %%edx), %%dl	# Get resulting pixel	\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, 2(%%edi)		# Store pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv+dvv)	\n\
		addl	%1, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		movzbl	(%%esi), %%edx		# Get texel		\n\
		movb	3(%%edi), %%dh		# Get screen pixel	\n\
		movb	(%%ebp, %%edx), %%dl	# Get resulting pixel	\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, 3(%%edi)		# Store pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv+dvv)	\n\
		addl	%1, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		movzbl	(%%esi), %%edx		# Get texel		\n\
		movb	4(%%edi), %%dh		# Get screen pixel	\n\
		movb	(%%ebp, %%edx), %%dl	# Get resulting pixel	\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, 4(%%edi)		# Store pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv+dvv)	\n\
		addl	%1, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		movzbl	(%%esi), %%edx		# Get texel		\n\
		movb	5(%%edi), %%dh		# Get screen pixel	\n\
		movb	(%%ebp, %%edx), %%dl	# Get resulting pixel	\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, 5(%%edi)		# Store pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv+dvv)	\n\
		addl	%1, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		movzbl	(%%esi), %%edx		# Get texel		\n\
		movb	6(%%edi), %%dh		# Get screen pixel	\n\
		movb	(%%ebp, %%edx), %%dl	# Get resulting pixel	\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, 6(%%edi)		# Store pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv+dvv)	\n\
		addl	%1, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		movzbl	(%%esi), %%edx		# Get texel		\n\
		movb	7(%%edi), %%dh		# Get screen pixel	\n\
		movb	(%%ebp, %%edx), %%dl	# Get resulting pixel	\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, 7(%%edi)		# Store pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv+dvv)	\n\
		addl	%1, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		addl	$8, %%edi					\n\
		cmpl	%2, %%edi					\n\
		jbe	0b 						\n\
		jmp	9f						\n\
									\n\
8:		movzbl	(%%esi), %%edx		# Get texel		\n\
		movb	(%%edi), %%dh		# Get screen pixel	\n\
		incl	%%edi			# Increment screen ptr	\n\
		movb	(%%ebp, %%edx), %%dl	# Get resulting pixel	\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, -1(%%edi)		# Store pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv+dvv)	\n\
		addl	%1, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		cmpl	%2, %%edi		# We finished?		\n\
		jbe	8b 						\n\
									\n\
9:		movl	%3, %%ebp		# Restore EBP"		\
    : "=D" (_dest) 							\
    : "m" (duFrac), "m" (_destend), "m" (oldEBP),			\
      "m" (dudvInt[1]), "m" (xx), "m" (Scan.AlphaMap)			\
    : "%edx", "%ebp" );							\
    uu = uu1;								\
    vv = vv1;
#include "cs3d/software/scanln.inc"

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define NO_draw_scanline_map_alpha2
#define SCANFUNC csScan_8_draw_scanline_map_alpha2
#define SCANMAP 1
#define SCANLOOP \
    int uFrac, duFrac, vFrac, dvFrac;					\
    static unsigned int oldEBP;						\
    static int dudvInt[2];						\
    unsigned char *s = srcTex + ((vv>>16)<<shifter) + (uu>>16);		\
									\
    dudvInt[1] = ((dvv >> 16) << shifter) + (duu>>16);			\
    dudvInt[0] = dudvInt[1] + (1 << shifter); 				\
    uFrac = uu << 16;							\
    vFrac = vv << 16;							\
    duFrac = duu << 16;							\
    dvFrac = dvv << 16;							\
									\
    asm __volatile__ ("" : : "a" (uFrac), "b" (vFrac),			\
                      "c" (dvFrac), "S" (s), "D" (_dest));		\
    asm __volatile__ ("							\
		movl	%%ebp, %3		# Save EBP		\n\
		movl	%6, %%ebp		# EBP = alpha_map	\n\
		cmpl	$0, %5			# Less than 16 pixels?	\n\
		je	8f						\n\
									\n\
0:		movzbl	(%%edi), %%edx		# Get texel		\n\
		movb	(%%esi), %%dh		# Get screen pixel	\n\
		movb	(%%ebp, %%edx), %%dl	# Get resulting pixel	\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, (%%edi)		# Store pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv+dvv)	\n\
		addl	%1, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		movzbl	1(%%edi), %%edx		# Get texel		\n\
		movb	(%%esi), %%dh		# Get screen pixel	\n\
		movb	(%%ebp, %%edx), %%dl	# Get resulting pixel	\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, 1(%%edi)		# Store pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv+dvv)	\n\
		addl	%1, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		movzbl	2(%%edi), %%edx		# Get texel		\n\
		movb	(%%esi), %%dh		# Get screen pixel	\n\
		movb	(%%ebp, %%edx), %%dl	# Get resulting pixel	\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, 2(%%edi)		# Store pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv+dvv)	\n\
		addl	%1, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		movzbl	3(%%edi), %%edx		# Get texel		\n\
		movb	(%%esi), %%dh		# Get screen pixel	\n\
		movb	(%%ebp, %%edx), %%dl	# Get resulting pixel	\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, 3(%%edi)		# Store pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv+dvv)	\n\
		addl	%1, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		movzbl	4(%%edi), %%edx		# Get texel		\n\
		movb	(%%esi), %%dh		# Get screen pixel	\n\
		movb	(%%ebp, %%edx), %%dl	# Get resulting pixel	\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, 4(%%edi)		# Store pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv+dvv)	\n\
		addl	%1, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		movzbl	5(%%edi), %%edx		# Get texel		\n\
		movb	(%%esi), %%dh		# Get screen pixel	\n\
		movb	(%%ebp, %%edx), %%dl	# Get resulting pixel	\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, 5(%%edi)		# Store pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv+dvv)	\n\
		addl	%1, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		movzbl	6(%%edi), %%edx		# Get texel		\n\
		movb	(%%esi), %%dh		# Get screen pixel	\n\
		movb	(%%ebp, %%edx), %%dl	# Get resulting pixel	\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, 6(%%edi)		# Store pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv+dvv)	\n\
		addl	%1, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		movzbl	7(%%edi), %%edx		# Get texel		\n\
		movb	(%%esi), %%dh		# Get screen pixel	\n\
		movb	(%%ebp, %%edx), %%dl	# Get resulting pixel	\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, 7(%%edi)		# Store pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv+dvv)	\n\
		addl	%1, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		addl	$8, %%edi					\n\
		cmpl	%2, %%edi					\n\
		jbe	0b 						\n\
		jmp	9f						\n\
									\n\
8:		movzbl	(%%edi), %%edx		# Get texel		\n\
		movb	(%%esi), %%dh		# Get screen pixel	\n\
		incl	%%edi			# Increment screen ptr	\n\
		movb	(%%ebp, %%edx), %%dl	# Get resulting pixel	\n\
		addl	%%ecx, %%ebx		# vv += dvv		\n\
		movb	%%dl, -1(%%edi)		# Store pixel		\n\
		sbbl	%%edx, %%edx		# carry flag (vv+dvv)	\n\
		addl	%1, %%eax		# uu += duu		\n\
		adcl	%4(,%%edx,4), %%esi	# update texture ptr	\n\
									\n\
		cmpl	%2, %%edi		# We finished?		\n\
		jbe	8b 						\n\
									\n\
9:		movl	%3, %%ebp		# Restore EBP"		\
    : "=D" (_dest) 							\
    : "m" (duFrac), "m" (_destend), "m" (oldEBP),			\
      "m" (dudvInt[1]), "m" (xx), "m" (Scan.AlphaMap)			\
    : "%edx", "%ebp" );							\
    uu = uu1;								\
    vv = vv1;
#include "cs3d/software/scanln.inc"

#if defined (DO_MMX)

#undef SCANFUNC
#undef SCANEND
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define NO_mmx_draw_scanline_map_zfil
#define SCANFUNC csScan_8_mmx_draw_scanline_map_zfil
#define SCANMAP 1
#define SCANLOOP I386_SCANLINE_MAP8
#define SCANEND MMX_FILLZBUFFER
#include "cs3d/software/scanln.inc"

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define NO_mmx_draw_scanline_tex_zfil
#define SCANFUNC csScan_8_mmx_draw_scanline_tex_zfil
#define SCANLOOP \
    do									\
    {									\
      *_dest++ = srcTex[((uu>>16)&ander_w) + ((vv>>shifter_h)&ander_h)];	\
      uu += duu;							\
      vv += dvv;							\
    }									\
    while (_dest <= _destend)
#define SCANEND MMX_FILLZBUFFER
#include "cs3d/software/scanln.inc"

#endif // DO_MMX

#define NO_draw_pi_scanline_tex_zuse
void csScan_8_draw_pi_scanline_tex_zuse (void *dest, int len, unsigned long *zbuff,
  long u, long du, long v, long dv, unsigned long z, long dz,
  unsigned char *bitmap, int bitmap_log2w)
{
  if (len <= 0)
    return;

  int uFrac, duFrac, vFrac, dvFrac;
  static unsigned int oldEBP;
  static int dudvInt[2];
  char *destend = ((char *)dest) + len;
  unsigned char *src = bitmap + ((v >> 16) << bitmap_log2w) + (u >> 16);

  dudvInt[1] = ((dv >> 16) << bitmap_log2w) + (du >> 16);
  dudvInt[0] = dudvInt [1] + (1 << bitmap_log2w);
  uFrac = u << 16;
  vFrac = v << 16;
  duFrac = du << 16;
  dvFrac = dv << 16;

  asm __volatile__ ("" : : "a" (uFrac), "b" (vFrac), "c" (z),
                    "S" (src), "D" (dest));
  // Sidenote: unrolling loop gives no gain??? (because of jump?)
  asm __volatile__ ("
		movl	%%ebp, %4		# Save EBP
		movl	%7, %%ebp

0:		cmpl	(%%ebp), %%ecx		# Check Z-buffer
		jb	1f			# We're below surface
		movb	(%%esi), %%dl		# Get texel
		movl	%%ecx, (%%ebp)		# *zbuff = z
		movb	%%dl, (%%edi)		# Put texel

1:		addl	%2, %%ebx		# v = v + dv
		sbbl	%%edx, %%edx
		addl	%1, %%eax		# u = u + du
		adcl	%5(,%%edx,4), %%esi	# Update source texture pointer
		addl	%6, %%ecx		# z = z + dz
		incl	%%edi			# dest++
		addl	$4, %%ebp		# zbuff++
		cmpl	%3, %%edi		# dest < _destend?
		jb	0b
		movl	%4, %%ebp"
  : "=D" (dest)
  : "m" (duFrac), "m" (dvFrac), "m" (destend), "m" (oldEBP),
    "m" (dudvInt[1]), "m" (dz), "m" (zbuff)
  : "eax", "%ebx", "ecx", "edx", "esi" );
}

#ifdef DO_MMX

#define NO_mmx_draw_pi_scanline_tex_zuse
void csScan_8_mmx_draw_pi_scanline_tex_zuse (void *dest, int len, unsigned long *zbuff,
  long u, long du, long v, long dv, unsigned long z, long dz,
  unsigned char *bitmap, int bitmap_log2w)
{
  if (len <= 0)
    return;

  int uFrac, duFrac, vFrac, dvFrac;
  static unsigned int oldEBP;
  static int dudvInt[2];
  char *destend = ((char *)dest) + len;
  unsigned char *src = bitmap + ((v >> 16) << bitmap_log2w) + (u >> 16);

  dudvInt[1] = ((dv >> 16) << bitmap_log2w) + (du >> 16);
  dudvInt[0] = dudvInt [1] + (1 << bitmap_log2w);
  uFrac = u << 16;
  vFrac = v << 16;
  duFrac = du << 16;
  dvFrac = dv << 16;
  asm __volatile__ ("" : : "a" (uFrac), "b" (vFrac), "S" (src), "D" (dest));
  asm __volatile__ ("
		movl	%%ebp, %4		# Save EBP
		addl	$4, %%edi		# Increment edi
		movl	%8, %%ebp		# EBP = zbuff
                movl	%6, %%ecx		# ECX = z

		cmpl	%3, %%edi		# Less than 4 pixels left?
		ja	5f			# Go by one

		# Load MMX registers
		movd	%7, %%mm1		# mm1 = 0 | dz
		movd	%%ecx, %%mm0		# mm0 = 0 | z0
		punpckldq %%mm1, %%mm1		# mm1 = dz | dz
		movq	%%mm0, %%mm2		# mm2 = 0 | z0
		paddd	%%mm1, %%mm2		# mm2 = dz | z1
		punpckldq %%mm2, %%mm0		# mm0 = z1 | z0
		pslld	$1, %%mm1		# mm1 = dz*2 | dz*2

# The following is somewhat hard to understand, especially without my proprietary
# GAS syntax highlight {tm} {R} :-) I've interleaved MMX instructions with
# non-MMX instructions to achieve maximal percent of paired commands; even
# commands are MMX, odd commands are non-MMX. Since MMX commands never affects
# flags register, we shouldn't care about flags and so on.
0:		movq	(%%ebp), %%mm3		# mm3 = bz1 | bz0
		movb	(%%esi), %%cl		# Get texel 0
		movq	%%mm0, %%mm5		# mm5 = z1 | z0
		addl	%2, %%ebx		# v = v + dv
		movq	%%mm0, %%mm4		# mm4 = z1 | z0
		sbbl	%%edx, %%edx
		pcmpgtd	%%mm3, %%mm5		# mm5 = m1 | m0
		addl	%1, %%eax		# u = u + du
		paddd	%%mm1, %%mm0		# mm0 = z3 | z2
		adcl	%5(,%%edx,4), %%esi	# Update source texture pointer
		movq	%%mm5, %%mm6		# mm6 = m1 | m0
		movb	(%%esi), %%ch		# Get texel 1
		pandn	%%mm3, %%mm5		# mm5 = ?bz1 | ?bz0
		addl	%2, %%ebx		# v = v + dv
		movq	8(%%ebp), %%mm3		# mm3 = bz3 | bz2
		sbbl	%%edx, %%edx
		pand	%%mm6, %%mm4		# mm4 = ?z1 | ?z0
		shll	$16, %%ecx		# ECX = p1 | p0 | 0 | 0
		movq	%%mm0, %%mm7		# mm7 = z3 | z2
		addl	%1, %%eax		# u = u + du
		por	%%mm4, %%mm5		# mm5 = nz1 | nz0
		adcl	%5(,%%edx,4), %%esi	# Update source texture pointer
		movq	%%mm0, %%mm4		# mm4 = z3 | z2
		movb	(%%esi), %%cl		# Get texel 2
		movq	%%mm5, (%%ebp)		# put nz1 | nz0 into z-buffer
		addl	%2, %%ebx		# v = v + dv
		pcmpgtd	%%mm3, %%mm7		# mm7 = m3 | m2
		sbbl	%%edx, %%edx
		movq	%%mm7, %%mm5		# mm5 = m3 | m2
		addl	%1, %%eax		# u = u + du
		paddd	%%mm1, %%mm0		# mm0 = z3 | z2
		adcl	%5(,%%edx,4), %%esi	# Update source texture pointer
		pandn	%%mm3, %%mm5		# mm5 = ?bz3 | ?bz2
		movb	(%%esi), %%ch		# Get texel 3
		pand	%%mm7, %%mm4		# mm4 = ?z3 | ?z2
                addl	%2, %%ebx		# v = v + dv
		packssdw %%mm6, %%mm7		# mm7 = m3 | m2 | m1 | m0
		sbbl	%%edx, %%edx
		por	%%mm4, %%mm5		# mm5 = nz3 | nz2
                roll	$16, %%ecx		# ECX = p3 | p2 | p1 | p0
		packsswb %%mm7, %%mm7		# mm7 = 0 | 0 | 0 | 0 | m3 | m2 | m1 | m0
		addl	%1, %%eax		# u = u + du
		movq	%%mm5, 8(%%ebp)		# put nz3 | nz2 into z-buffer
                adcl	%5(,%%edx,4), %%esi	# Update source texture pointer

#----- end of interleaved code

		addl	$16, %%ebp		# Increment zbuf pointer

		movd	%%mm7, %%edx		# EDX = m3 | m2 | m1 | m0
		andl	%%edx, %%ecx		# Leave only visible texels
		notl	%%edx			# Negate Z-mask
		andl	-4(%%edi), %%edx	# Get on-screen pixels
		orl	%%ecx, %%edx		# Merge two sets of pixels together
		addl	$4, %%edi		# Increment dest pointer
		movl	%%edx, -8(%%edi)	# Put pixels into framebuffer
		cmpl	%3, %%edi		# dest < _destend?
		jbe	0b

                # Less than four pixels left
		movd	%%mm0, %%ecx

5:              subl	$4, %%edi
		cmpl	%3, %%edi		# dest >=_destend?
		jae	8f

6:		cmpl	(%%ebp), %%ecx
		jb	7f
		movb	(%%esi), %%dl		# Get texel
		movl	%%ecx, (%%ebp)		# *zbuff = z
		movb	%%dl, (%%edi)		# Put texel

7:		addl	%2, %%ebx		# v = v + dv
		sbbl	%%edx, %%edx
		addl	%1, %%eax		# u = u + du
		adcl	%5(,%%edx,4), %%esi	# Update source texture pointer
		addl	%7, %%ecx		# z = z + dz
		incl	%%edi			# dest++
		addl	$4, %%ebp		# zbuff++
		cmpl	%3, %%edi		# dest < _destend?
		jb	6b

8:		movl	%4, %%ebp
                emms"
  : "=D" (dest)
  : "m" (duFrac), "m" (dvFrac), "m" (destend), "m" (oldEBP),
    "m" (dudvInt[1]), "m" (z), "m" (dz), "m" (zbuff)
  : "eax", "%ebx", "ecx", "edx", "esi" );
}

#endif // DO_MMX

#endif // __SCANLN8_H__
