/*
    Copyright (C) 1998 by Jorrit Tyberghein
    CPU identification routine interface

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

#ifndef __CPUID_H__
#define __CPUID_H__

#define CPUx86_FEATURE_FPU	0x00000001	// FPU on Chip
#define CPUx86_FEATURE_VME	0x00000002	// Virtual Mode Extention
#define CPUx86_FEATURE_DE	0x00000004	// Debbuging Extentions
#define CPUx86_FEATURE_PSE	0x00000008	// 4MB size pages
#define CPUx86_FEATURE_TSC	0x00000010	// RDTSC supported
#define CPUx86_FEATURE_MSR	0x00000020	// Pentium Compatible MSRs
#define CPUx86_FEATURE_PAE	0x00000040	// Physical Address Extension (Intel)
#define CPUx86_FEATURE_PTE	0x00000040	// Support PTE (Cyrix)
#define CPUx86_FEATURE_MCE	0x00000080	// Machine Check exception
#define CPUx86_FEATURE_CX8	0x00000100	// CMPXCHG8B supported
#define CPUx86_FEATURE_APIC	0x00000200	// Local APIC on Chip (Intel)
#define CPUx86_FEATURE_SEP	0x00000800	// Fast System Call (PPro)
#define CPUx86_FEATURE_MTRR	0x00001000	// Memory Type Range Register (MTRR)
#define CPUx86_FEATURE_PGE	0x00002000	// Page Global Feature
#define CPUx86_FEATURE_MCA	0x00004000	// Machine Check Architecture
#define CPUx86_FEATURE_CMOV	0x00008000	// CMOV supported
#define CPUx86_FEATURE_MMX	0x00800000	// MMX supported

#ifdef DO_NASM

// This is a "processor-independent" routine that is defined
// independently for each processor type (if needed)
extern "C" void csDetectCPU (int *Family, char Vendor[13], int *Features);

#elif defined (COMP_VC) || defined (COMP_WCC)

#if _MSC_VER < 1200     // If compiler version is below 6
#define cpuid __asm _emit 0x0F __asm _emit 0xA2
#endif

/**
 * Detect whenever current CPU supports MMX instructions and return its ID.
 * Memory block to hold id string should be at least 13 bytes size.
 */

static inline void csDetectCPU (int *Family, char Vendor[13], int *Features)
{
  __asm{
		pushfd
		pop     eax
		mov     ebx,eax
		xor     eax,200000h
		push    eax
		popfd
		pushfd
		pop     eax
		cmp     eax,ebx
		jz      short notP5

		xor	eax,eax
		cpuid
		mov     eax,Vendor
		mov     dword ptr [eax],ebx
		mov     dword ptr [eax+4],edx
		mov     dword ptr [eax+8],ecx
		mov     byte ptr [eax+12],0
		mov     eax, 1
		cpuid
		mov	al,100
		mul	ah			; Compute the ??86 number
		movzx	eax,ax
		add	eax,86
		jmp	short setVal

notP5:		mov	eax,486
		mov	edx,1
		mov	ecx,Vendor
		mov	byte ptr [ecx],0
setVal:		mov     ecx,Family
		mov	[ecx],eax
		mov	ecx,Features
		mov	[ecx],edx
  }
}

#else

static inline void csDetectCPU (int *Family, char Vendor[13], int *Features)
{
  asm (
	"	pushfl			\n"
	"	popl	%%eax		\n"
	"	movl	%%eax,%%ebx	\n"
	"	xorl	$0x200000,%%eax	\n"
	"	pushl	%%eax		\n"
	"	popfl			\n"
	"	pushfl			\n"
	"	popl	%%eax		\n"
	"	cmpl	%%ebx,%%eax	\n"
	"	jz	1f		\n"
	"				\n"
	"	xorl	%%eax,%%eax	\n"
	"	cpuid			\n"
	"	movl	%%ebx,0(%%esi)	\n"
	"	movl	%%edx,4(%%esi)	\n"
	"	movl	%%ecx,8(%%esi)	\n"
	"	movb	$0,12(%%esi)	\n"
	"	movl	$1,%%eax	\n"
	"	cpuid			\n"
	"	movb	$100,%%al	\n"
	"	mulb	%%ah		\n"
	"	movzwl	%%ax,%%eax	\n"
	"	addl	$86,%%eax	\n"
	"	jmp	2f		\n"
	"				\n"
	"1:	movl	$486,%%eax	\n"
	"	movl	$1,%%edx	\n"
	"	movb	$0,0(%%esi)	\n"
	"2:	movl	%0,%%ecx	\n"
	"	movl	%%eax,(%%ecx)	\n"
	"	movl	%2,%%ecx	\n"
	"	movl	%%edx,(%%ecx)	\n"
  :
  : "m" (Family), "S" (Vendor), "m" (Features)
  : "ebx", "ecx", "edx");
}

// Temporary trick: since there is no csScan_16_mmx_draw_pi_scanline_tex_zuse
// routine in GAS mode and since this file is included only by soft_g3d.cpp,
// we define it to non-MMX version here so that it gets replaced.
#ifdef DO_MMX

// There are no MMX perspective-incorrect routines for GAS and VC assembler
#define NO_mmx_draw_pi_scanline_tex_zuse
#define csScan_16_mmx_draw_pi_scanline_tex_zuse csScan_16_draw_pi_scanline_tex_zuse

#endif // DO_MMX

#endif

#endif // __CPUID_H__
