/*
    Crystal Space 16-bit software driver assembler-optimized routines
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

#ifndef __SCANLN16_H__
#define __SCANLN16_H__

#if defined (DO_MMX)
#  include "mmx.h"
#endif

#define I386_SCANLINE_MAP16 \
    int uFrac, duFrac, vFrac, dvFrac;					\
    static unsigned int oldEBP;						\
    static int dudvInt[4];						\
    UShort *s = srcTex + ((vv >> 16) << shifter) + (uu >> 16);		\
									\
    dudvInt[2] = (((dvv >> 16) << shifter) + (duu>>16)) * 2;		\
    dudvInt[3] = dudvInt[2] + 2;					\
    dudvInt[0] = dudvInt[2] + (1 << shifter) * 2;			\
    dudvInt[1] = dudvInt[0] + 2;					\
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
									\n\
		cmpl	$0, %5						\n\
		je	8f						\n\
									\n\
0:		addl	%%ecx, %%ebx					\n\
		sbbl	%%ebp, %%ebp					\n\
		movw	(%%esi), %%dx					\n\
		shll	$1, %%ebp					\n\
		addl	%1, %%eax					\n\
		movw	%%dx, (%%edi)					\n\
		adcl	$0, %%ebp					\n\
		addl	%4(,%%ebp,4), %%esi				\n\
									\n\
		addl	%%ecx, %%ebx 					\n\
		sbbl	%%ebp, %%ebp					\n\
		movw	(%%esi), %%dx					\n\
		shll	$1, %%ebp					\n\
		addl	%1, %%eax 					\n\
		movw	%%dx, 2(%%edi)					\n\
		adcl	$0, %%ebp					\n\
		addl	%4(,%%ebp,4), %%esi				\n\
									\n\
		addl	%%ecx, %%ebx 					\n\
		sbbl	%%ebp, %%ebp					\n\
		movw	(%%esi), %%dx					\n\
		shll	$1, %%ebp					\n\
		addl	%1, %%eax 					\n\
		movw	%%dx, 4(%%edi)					\n\
		adcl	$0, %%ebp					\n\
		addl	%4(,%%ebp,4), %%esi				\n\
									\n\
		addl	%%ecx, %%ebx 					\n\
		sbbl	%%ebp, %%ebp					\n\
		movw	(%%esi), %%dx					\n\
		shll	$1, %%ebp					\n\
		addl	%1, %%eax 					\n\
		movw	%%dx, 6(%%edi)					\n\
		adcl	$0, %%ebp					\n\
		addl	%4(,%%ebp,4), %%esi				\n\
									\n\
		addl	%%ecx, %%ebx 					\n\
		sbbl	%%ebp, %%ebp					\n\
		movw	(%%esi), %%dx					\n\
		shll	$1, %%ebp					\n\
		addl	%1, %%eax 					\n\
		movw	%%dx, 8(%%edi)					\n\
		adcl	$0, %%ebp					\n\
		addl	%4(,%%ebp,4), %%esi				\n\
									\n\
		addl	%%ecx, %%ebx 					\n\
		sbbl	%%ebp, %%ebp					\n\
		movw	(%%esi), %%dx					\n\
		shll	$1, %%ebp					\n\
		addl	%1, %%eax 					\n\
		movw	%%dx, 10(%%edi)					\n\
		adcl	$0, %%ebp					\n\
		addl	%4(,%%ebp,4), %%esi				\n\
									\n\
		addl	%%ecx, %%ebx 					\n\
		sbbl	%%ebp, %%ebp					\n\
		movw	(%%esi), %%dx					\n\
		shll	$1, %%ebp					\n\
		addl	%1, %%eax 					\n\
		movw	%%dx, 12(%%edi)					\n\
		adcl	$0, %%ebp					\n\
		addl	%4(,%%ebp,4), %%esi				\n\
									\n\
		addl	%%ecx, %%ebx 					\n\
		sbbl	%%ebp, %%ebp					\n\
		movw	(%%esi), %%dx					\n\
		shll	$1, %%ebp					\n\
		addl	%1, %%eax 					\n\
		movw	%%dx, 14(%%edi)					\n\
		adcl	$0, %%ebp					\n\
		addl	%4(,%%ebp,4), %%esi				\n\
									\n\
		addl	$16, %%edi					\n\
		cmpl	%2, %%edi					\n\
		jbe	0b 						\n\
		jmp     9f 						\n\
			 						\n\
8:		addl	%%ecx, %%ebx					\n\
		sbbl	%%ebp, %%ebp					\n\
		movw	(%%esi), %%dx					\n\
		shll	$1, %%ebp					\n\
		addl	%1, %%eax					\n\
		movw	%%dx, (%%edi)					\n\
		adcl	$0, %%ebp					\n\
		addl	%4(,%%ebp,4), %%esi				\n\
									\n\
		addl	$2,%%edi					\n\
		cmpl	%2, %%edi					\n\
		jbe	8b						\n\
9:		movl	%3, %%ebp"					\
    : "=D" (_dest) 							\
    : "m" (duFrac), "m" (_destend), "m" (oldEBP), "m" (dudvInt[2]),	\
      "m" (xx)								\
    : "%edx");								\
    uu = uu1;								\
    vv = vv1;

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define NO_draw_scanline_map_zfil
#define SCANFUNC csScan_16_draw_scanline_map_zfil
#define SCANMAP 1
#define SCANLOOP I386_SCANLINE_MAP16
#define SCANEND \
    do									\
    {									\
      *z_buffer++ = izz;						\
      izz += dzz;							\
    }									\
    while (z_buffer <= lastZbuf)
#include "cs3d/software/scanln.inc"

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define NO_draw_scanline_map_zuse
#define SCANFUNC csScan_16_draw_scanline_map_zuse
#define SCANMAP 1
#define SCANLOOP \
    int uFrac, duFrac, vFrac, dvFrac;					\
    static unsigned int oldEBP;						\
    static int dudvInt[4];						\
    UShort *src = srcTex + ((vv >> 16) << shifter) + (uu >> 16);	\
									\
    dudvInt[2] = (((dvv >> 16) << shifter) + (duu>>16)) * 2;		\
    dudvInt[3] = dudvInt[2] + 2;					\
    dudvInt[0] = dudvInt[2] + (1 << shifter) * 2;			\
    dudvInt[1] = dudvInt[0] + 2;					\
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
		movw	(%%esi), %%dx		# Get texel		\n\
		movl	%%ecx, (%%ebp)		# *zbuff = z		\n\
  		movw	%%dx, (%%edi)		# Put texel		\n\
									\n\
1:		addl	%2, %%ebx 		# vv += dvv		\n\
		sbbl	%%edx, %%edx		# carry (vv + dvv)	\n\
		shll	$1, %%edx		# *2			\n\
		addl	%1, %%eax 		# uu += duu		\n\
		adcl	$0, %%edx		# +carry (uu + duu)	\n\
		addl	%5(,%%edx,4), %%esi	# Source texture ptr	\n\
									\n\
		addl	%6, %%ecx		# z = z + dz		\n\
		addl	$2, %%edi		# dest++		\n\
		addl	$4, %%ebp		# zbuff++		\n\
		cmpl	%3, %%edi		# dest < _destend?	\n\
		jbe	0b						\n\
		movl	%%ebp, %7					\n\
		movl	%%ecx, %8					\n\
		movl	%4, %%ebp"					\
    : "=D" (_dest)							\
    : "m" (duFrac), "m" (dvFrac), "m" (_destend), "m" (oldEBP),		\
      "m" (dudvInt[2]), "m" (dzz), "m" (z_buffer), "m" (izz)		\
    : "eax", "%ebx", "ecx", "edx", "esi" );				\
    uu = uu1;								\
    vv = vv1;
#include "cs3d/software/scanln.inc"

#define I386_SCANLINE_MAP_ALPHA50_16 \
    int uFrac, duFrac, vFrac, dvFrac;					\
    static unsigned int oldEBP;						\
    static int dudvInt[4];						\
    UShort *s = srcTex + ((vv >> 16) << shifter) + (uu >> 16);		\
									\
    dudvInt[2] = (((dvv >> 16) << shifter) + (duu>>16)) * 2;		\
    dudvInt[3] = dudvInt[2] + 2;					\
    dudvInt[0] = dudvInt[2] + (1 << shifter) * 2;			\
    dudvInt[1] = dudvInt[0] + 2;					\
    uFrac = uu << 16;							\
    vFrac = vv << 16;							\
    duFrac = duu << 16;							\
    dvFrac = dvv << 16;							\
									\
    asm __volatile__ ("" : : "a" (uFrac), "b" (vFrac),			\
                      "S" (s), "D" (_dest));				\
    asm __volatile__ ("							\
		movl	%%ebp, %4		# save EBP		\n\
		movb	(%%edi), %%dl		# fetch into cache	\n\
		movl	%2, %%ebp		# ebp = ddv		\n\
									\n\
		cmpl	$0, %6						\n\
		je	8f						\n\
									\n\
0:		movw	(%%esi), %%dx		# Get texel		\n\
		movw	(%%edi), %%cx		# Get pixel		\n\
		andl	%7, %%edx		# prepare to divide by 2\n\
		andl	%7, %%ecx		# prepare to divide by 2\n\
		shrl	$1, %%edx		# texel/2		\n\
		shrl	$1, %%ecx		# pixel/2		\n\
		addl	%%ecx, %%edx		# texel/2 + pixel/2	\n\
		addl	%%ebp, %%ebx		# vv += dvv		\n\
		sbbl	%%ecx, %%ecx		# carry flag		\n\
		movw	%%dx, (%%edi)		# Put pixel		\n\
		shll	$1, %%ecx		# *2			\n\
		addl	%1, %%eax		# uu += duu		\n\
		adcl	$0, %%ecx		# + carry (uu + duu)	\n\
		addl	%5(,%%ecx,4), %%esi	# update texture ptr	\n\
									\n\
		movw	(%%esi), %%dx		# Get texel		\n\
		movw	2(%%edi), %%cx		# Get pixel		\n\
		andl	%7, %%edx		# prepare to divide by 2\n\
		andl	%7, %%ecx		# prepare to divide by 2\n\
		shrl	$1, %%edx		# texel/2		\n\
		shrl	$1, %%ecx		# pixel/2		\n\
		addl	%%ecx, %%edx		# texel/2 + pixel/2	\n\
		addl	%%ebp, %%ebx		# vv += dvv		\n\
		sbbl	%%ecx, %%ecx		# carry flag		\n\
		movw	%%dx, 2(%%edi)		# Put pixel		\n\
		shll	$1, %%ecx		# *2			\n\
		addl	%1, %%eax		# uu += duu		\n\
		adcl	$0, %%ecx		# + carry (uu + duu)	\n\
		addl	%5(,%%ecx,4), %%esi	# update texture ptr	\n\
									\n\
		movw	(%%esi), %%dx		# Get texel		\n\
		movw	4(%%edi), %%cx		# Get pixel		\n\
		andl	%7, %%edx		# prepare to divide by 2\n\
		andl	%7, %%ecx		# prepare to divide by 2\n\
		shrl	$1, %%edx		# texel/2		\n\
		shrl	$1, %%ecx		# pixel/2		\n\
		addl	%%ecx, %%edx		# texel/2 + pixel/2	\n\
		addl	%%ebp, %%ebx		# vv += dvv		\n\
		sbbl	%%ecx, %%ecx		# carry flag		\n\
		movw	%%dx, 4(%%edi)		# Put pixel		\n\
		shll	$1, %%ecx		# *2			\n\
		addl	%1, %%eax		# uu += duu		\n\
		adcl	$0, %%ecx		# + carry (uu + duu)	\n\
		addl	%5(,%%ecx,4), %%esi	# update texture ptr	\n\
									\n\
		movw	(%%esi), %%dx		# Get texel		\n\
		movw	6(%%edi), %%cx		# Get pixel		\n\
		andl	%7, %%edx		# prepare to divide by 2\n\
		andl	%7, %%ecx		# prepare to divide by 2\n\
		shrl	$1, %%edx		# texel/2		\n\
		shrl	$1, %%ecx		# pixel/2		\n\
		addl	%%ecx, %%edx		# texel/2 + pixel/2	\n\
		addl	%%ebp, %%ebx		# vv += dvv		\n\
		sbbl	%%ecx, %%ecx		# carry flag		\n\
		movw	%%dx, 6(%%edi)		# Put pixel		\n\
		shll	$1, %%ecx		# *2			\n\
		addl	%1, %%eax		# uu += duu		\n\
		adcl	$0, %%ecx		# + carry (uu + duu)	\n\
		addl	%5(,%%ecx,4), %%esi	# update texture ptr	\n\
									\n\
		addl	$8, %%edi					\n\
		cmpl	%3, %%edi					\n\
		jbe	0b 						\n\
		jmp     9f 						\n\
			 						\n\
8:		movw	(%%esi), %%dx		# Get texel		\n\
		movw	(%%edi), %%cx		# Get pixel		\n\
		andl	%7, %%edx		# prepare to divide by 2\n\
		andl	%7, %%ecx		# prepare to divide by 2\n\
		shrl	$1, %%edx		# texel/2		\n\
		shrl	$1, %%ecx		# pixel/2		\n\
		addl	%%ecx, %%edx		# texel/2 + pixel/2	\n\
		addl	%%ebp, %%ebx		# vv += dvv		\n\
		sbbl	%%ecx, %%ecx		# carry flag		\n\
		movw	%%dx, (%%edi)		# Put pixel		\n\
		shll	$1, %%ecx		# *2			\n\
		addl	%1, %%eax		# uu += duu		\n\
		adcl	$0, %%ecx		# + carry (uu + duu)	\n\
		addl	%5(,%%ecx,4), %%esi	# update texture ptr	\n\
									\n\
		addl	$2,%%edi		# increment dest ptr	\n\
		cmpl	%3, %%edi		# we're finished?	\n\
		jbe	8b						\n\
9:		movl	%4, %%ebp		# restore EBP"		\
    : "=D" (_dest) 							\
    : "m" (duFrac), "m" (dvFrac), "m" (_destend), "m" (oldEBP),		\
      "m" (dudvInt[2]), "m" (xx), "m" (Scan.AlphaMask)			\
    : "%edx");								\
    uu = uu1;								\
    vv = vv1;

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define NO_draw_scanline_map_alpha50
#define SCANFUNC csScan_16_draw_scanline_map_alpha50
#define SCANMAP 1
#define SCANLOOP I386_SCANLINE_MAP_ALPHA50_16
#include "cs3d/software/scanln.inc"

#if defined (DO_MMX)

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define NO_mmx_draw_scanline_map_zfil
#define SCANFUNC csScan_16_mmx_draw_scanline_map_zfil
#define SCANMAP 1
#define SCANLOOP I386_SCANLINE_MAP16
#define SCANEND MMX_FILLZBUFFER
#include "cs3d/software/scanln.inc"

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#define NO_mmx_draw_scanline_tex_zfil
#define SCANFUNC csScan_16_mmx_draw_scanline_tex_zfil
#define SCANLOOP \
    do									\
    {									\
      *_dest++ = Scan.PaletteTable[srcTex[((uu>>16)&ander_w) + ((vv>>shifter_h)&ander_h)]];\
      uu += duu;							\
      vv += dvv;							\
    }									\
    while (_dest <= _destend)
#define SCANEND MMX_FILLZBUFFER
#include "cs3d/software/scanln.inc"

#endif

#define NO_draw_pi_scanline_tex_zuse
void csScan_16_draw_pi_scanline_tex_zuse (void *dest, int len,
  unsigned long *zbuff, long u, long du, long v, long dv,
  unsigned long z, long dz, unsigned char *bitmap, int bitmap_log2w)
{
  if (len <= 0)
    return;

  static int uFrac, duFrac, vFrac, dvFrac;
  static unsigned int oldEBP;
  static int dudvInt[2];
  UShort *destend = ((UShort *)dest) + len;
  unsigned char *src = bitmap + ((v >> 16) << bitmap_log2w) + (u >> 16);

  dudvInt[1] = ((dv >> 16) << bitmap_log2w) + (du >> 16);
  dudvInt[0] = dudvInt [1] + (1 << bitmap_log2w);
  uFrac = u << 16;
  vFrac = v << 16;
  duFrac = du << 16;
  dvFrac = dv << 16;

  asm __volatile__ ("" : : "a" (uFrac), "b" (vFrac), "c" (z),
                    "S" (src), "D" (dest));
  // Sidenote: unrolling loop gives no gain???
  asm __volatile__ ("
		movl	%%ebp, %4		# Save EBP
		movl	%7, %%ebp

0:		cmpl	(%%ebp), %%ecx
		jb	1f
		movl	%8, %%edx		# edx = pal_table
		movl	%%ecx, (%%ebp)		# *zbuff = z
		movzbl	(%%esi), %%ecx		# Get texel
		movw	(%%edx,%%ecx,2), %%dx	# Translate into RGB
		movl	(%%ebp), %%ecx		# Get Z back (from CPU cache)
		movw	%%dx, (%%edi)		# Put texel

1:		addl	%2, %%ebx		# v = v + dv
		sbbl	%%edx, %%edx
		addl	%1, %%eax		# u = u + du
		adcl	%5(,%%edx,4), %%esi	# Update source texture pointer
		addl	%6, %%ecx		# z = z + dz
		addl	$2, %%edi		# dest++
		addl	$4, %%ebp		# zbuff++
		cmpl	%3, %%edi		# dest < _destend?
		jb	0b
		movl	%4, %%ebp"
  : "=D" (dest)
  : "m" (duFrac), "m" (dvFrac), "m" (destend), "m" (oldEBP),
    "m" (dudvInt[1]), "m" (dz), "m" (zbuff), "m" (Scan.PaletteTable)
  : "eax", "%ebx", "ecx", "edx", "esi" );
}

#endif // __SCANLN16_H__
