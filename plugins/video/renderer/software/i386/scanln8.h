/*
    Crystal Space 8-bit software driver assembler-optimized routines
    Copyright (C) 1998 by Jorrit Tyberghein
    Contributors:
       scan_map by David N. Arnold <derek_arnold@fuse.net>
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
#define NO_scan_map_zfil
#define SCANFUNC csScan_8_scan_map_zfil
#define SCANMAP
#define SCANLOOP I386_SCANLINE_MAP8
#define SCANEND \
    do								\
    {								\
      *z_buffer++ = izz;					\
      izz += dzz;						\
    }								\
    while (z_buffer <= lastZbuf)
#include "video/renderer/software/scanln.inc"

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define NO_scan_map_zuse
#define SCANFUNC csScan_8_scan_map_zuse
#define SCANMAP
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
#include "video/renderer/software/scanln.inc"

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define NO_scan_map_fixalpha1
#define SCANFUNC csScan_8_scan_map_fixalpha1
#define SCANMAP
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
#include "video/renderer/software/scanln.inc"

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define NO_scan_map_fixalpha2
#define SCANFUNC csScan_8_scan_map_fixalpha2
#define SCANMAP
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
#include "video/renderer/software/scanln.inc"

#if defined (DO_MMX)

#undef SCANFUNC
#undef SCANEND
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define NO_mmx_scan_map_zfil
#define SCANFUNC csScan_8_mmx_scan_map_zfil
#define SCANMAP
#define SCANLOOP I386_SCANLINE_MAP8
#define SCANEND MMX_FILLZBUFFER
#include "video/renderer/software/scanln.inc"

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define NO_mmx_scan_tex_zfil
#define SCANFUNC csScan_8_mmx_scan_tex_zfil
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
#include "video/renderer/software/scanln.inc"

#endif // DO_MMX

#define NO_scan_pi_tex_zuse
void csScan_8_scan_pi_tex_zuse (void *dest, int len, unsigned long *zbuff,
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

		movl	%%ecx, (%%ebp)		# *zbuff = z
		xorl	%%ecx, %%ecx		# ecx = 0
		movl	%8, %%edx		# edx = pal_table
		movb	(%%esi), %%cl		# Get texel
		movb	(%%edx,%%ecx), %%dl	# Translate into RGB
		movl	(%%ebp), %%ecx		# Get Z back (from CPU cache)
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
    "m" (dudvInt[1]), "m" (dz), "m" (zbuff), "m" (Scan.PaletteTable)
  : "eax", "%ebx", "ecx", "edx", "esi");
}

#endif // __SCANLN8_H__
