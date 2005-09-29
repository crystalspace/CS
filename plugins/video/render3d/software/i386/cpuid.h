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

#ifndef __CS_CPUID_H__
#define __CS_CPUID_H__

/*
    80x86 processor family feature bits.
*/
#define CPUx86_FEATURE_FPU  0x00000001	// FPU on Chip
#define CPUx86_FEATURE_VME  0x00000002	// Virtual Mode Extention
#define CPUx86_FEATURE_DE   0x00000004	// Debbuging Extentions
#define CPUx86_FEATURE_PSE  0x00000008	// 4MB size pages
#define CPUx86_FEATURE_TSC  0x00000010	// RDTSC supported
#define CPUx86_FEATURE_MSR  0x00000020	// Pentium Compatible MSRs
#define CPUx86_FEATURE_PAE  0x00000040	// Physical Address Extension (Intel)
#define CPUx86_FEATURE_PTE  0x00000040	// Support PTE (Cyrix)
#define CPUx86_FEATURE_MCE  0x00000080	// Machine Check exception
#define CPUx86_FEATURE_CX8  0x00000100	// CMPXCHG8B supported
#define CPUx86_FEATURE_APIC 0x00000200	// Local APIC on Chip (Intel)
#define CPUx86_FEATURE_SEP  0x00000800	// Fast System Call (PPro)
#define CPUx86_FEATURE_MTRR 0x00001000	// Memory Type Range Register (MTRR)
#define CPUx86_FEATURE_PGE  0x00002000	// Page Global Feature
#define CPUx86_FEATURE_MCA  0x00004000	// Machine Check Architecture
#define CPUx86_FEATURE_CMOV 0x00008000	// CMOV supported
#define CPUx86_FEATURE_MMX  0x00800000	// MMX supported
#define CPUx86_FEATURE_SSE  0x02000000	// SSE supported

/*
    Detect 80x86 CPU and its feature bits.
*/
extern "C" void csDetectCPU (int *Family, char Vendor[13], int *Features);

#endif // __CS_CPUID_H__
