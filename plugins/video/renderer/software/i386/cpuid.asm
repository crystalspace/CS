;--------=========xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx=========--------
;
;   Copyright (C) 1998 by Jorrit Tyberghein
;   Written by Andrew Zabolotny
;   CPU identification code
;
;   This library is free software; you can redistribute it and/or
;   modify it under the terms of the GNU Library General Public
;   License as published by the Free Software Foundation; either
;   version 2 of the License, or (at your option) any later version.
;
;   This library is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;   Library General Public License for more details.
;
;   You should have received a copy of the GNU Library General Public
;   License along with this library; if not, write to the Free
;   Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
;
;--------=========xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx=========--------

%include "cs.ash"

;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
; Summary:
;   Detect whenever current CPU supports MMX instructions and return its ID.
;   Memory block to hold id string should be at least 13 bytes size.
; Arguments:
;   none
; Example:
;   extern "C" void csDetectCPU (int *Family, char *Vendor[13], int *Features)
;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
proc		csDetectCPU,0,ebx
		targ	Family			; int *
		targ	Vendor			; char *
		targ	Features		; int *

		pushfd
		pop	eax
		mov	ebx,eax			; The ability to modify the
		xor	eax,200000h		; ID bit of EFLAGS indicates
		push	eax			; support for cpuid instruction
		popfd
		pushfd
		pop	eax
		cmp	eax,ebx
		jz	%$notP5

		mov	eax,0			; Query vendor id
		cpuid
		mov	eax,Vendor
		mov	[eax],ebx		; "GenuineIntel"
		mov	[eax+4],edx
		mov	[eax+8],ecx
		mov	byte [eax+12],0		; trailing zero

		mov	eax,1			; Query processor family
		cpuid
		mov	al,100
		mul	ah			; Compute the ??86 number
		movzx	eax,ax
		add	eax,86
		jmp	%$save

%$notP5:	mov	eax,486			; Not a P5
		mov	edx,1			; feature bits - assume FPU on chip
		mov	ecx,Vendor
		mov	byte [ecx],0		; empty vendor id

%$save:		mov	ecx,Family
		mov	[ecx],eax
		mov	ecx,Features
		mov	[ecx],edx
endproc
