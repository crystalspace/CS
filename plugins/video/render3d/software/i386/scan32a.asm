;
;   Copyright (C) 1998 by Jorrit Tyberghein
;   Written by Andrew Zabolotny
;   32-bit scanline drawing routines
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

;   PLEASE read first the comment at the start of scan.ash

%include "cs.ash"
%include "scan.ash"

;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
; Summary:
;   The internal scanloop for scan_map_zfil
;   Draw one horizontal scanline (with lighting)
; Arguments:
;   none
; Example:
;   scanproc 32,scan_map_zfil,SCANPROC_MAP,scanloop_map
;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
%define		scanloop_map_args 24
%macro		scanloop_map_init 0
		loc	%$dudvFP,16	; fixed-point value of (dv * tex_w + du)
		tloc	%$duFP		; fixed-point duu
		tloc	%$destend8	; dest+(destend-dest) & ~7
%endmacro
%macro		scanloop_map_body 0
	%ifdef PIC
		push	ebx
	%endif
; dudvInt[1] = ((dvv >> 16) << shifter) + (duu >> 16);
		mov	eax,%$dvv		; eax = dvv		; 0
		mov	edx,%$duu		; edx = duu		; 0
		sar	eax,16			; eax = dvv >> 16	; 1
		mov	esi,bitmap2		; esi = texture ptr	; 1
		sar	edx,16			; edx = duu >> 16	; 2
		mov	ecx,shf_u		; ecx = shifter		; 2
		add	ecx,2			; ecx += log2(pixsize)	; 3
		shl	eax,cl			; eax = (dvv >> 16) << s; 4/4
; dudvInt[0] = dudvInt[1] + (1 << shifter);
		mov	ebx,%$vv		; ebx = vv		; 4
		lea	eax,[eax+edx*4]		; eax = dudvInt[1]	; 8
		mov	edx,1			; edx = 1		; 8
		shl	edx,cl			; edx = 1 << shifter	; 9/4
		mov	[%$dudvFP_+8],eax	; dudvInt[2] = eax	; 9
		add	eax,4			; eax += pixsize	; 13
		sar	ebx,16			; ebx = vv >> 16	; 14
		mov	[%$dudvFP_+12],eax	; dudvInt[3] = eax+2	; 14
		add	eax,edx			; eax = dudvInt[1]	; 15
		mov	edx,%$uu		; edx = uu		; 15
		sar	edx,16			; edx = uu >> 16	; 16
		mov	[%$dudvFP_+4],eax	; dudvInt[1] = eax	; 16
		sub	eax,4			; eax = dudvInt[0]	; 17
		mov	[%$dudvFP_+0],eax	; dudvInt[0] = eax	; 18
; unsigned char *s = srcTex + ((vv >> 16) << shifter) + (uu >> 16);
		mov	eax,%$uu					; 18
		shl	ebx,cl						; 19/4
		test	[edi],eax		; fetch dest into cache	; 19
		lea	esi,[esi+edx*4]		; esi = source texture	; 23
		mov	edx,%$destend					; 23
		add	esi,ebx						; 24

; uFrac = uu << 16;
; vFrac = vv << 16;
		mov	ebx,%$vv					; 24
		shl	eax,16			; eax = uu << 16	; 25
		mov	ebp,%$dvv					; 25
		shl	ebx,16			; ebx = vv << 16	; 26
; duFrac = duu << 16;
; dvFrac = dvv << 16;
		mov	ecx,%$duu					; 26
		shl	ebp,16			; ebp = dvv << 16	; 27
		sub	edx,edi						; 27
		shl	ecx,16			; ecx = duu << 16	; 28
		and	edx,~31			; (destend-dest) & ~7	; 28
		mov	%$duFP,ecx					; 29
		jz	near %$sloop1					; 29

		add	edx,edi						; 30
		mov	%$destend8,edx					; 30

; --------------// Register usage //--------------
; EAX	frac(uu)	mem	frac(duu)
; EBX	frac(vv)	EBP	frac(dvv)
; ECX	--//scratch//--	EDX	--//scratch//--
; ESI	texture		EDI	dest

%$sloop8:	add	ebx,ebp			; vv += dvv		; 0
		mov	edx,%$duFP		; edx = duu		; 0
		sbb	ecx,ecx			; ecx = carry(vv+dvv)	; 1
		add	eax,edx			; uu += duu		; 1
		adc	ecx,ecx			; ebx = (ebx*2)+carry	; 2
		mov	edx,[esi]		; dx = texel		; 2
		add	esi,[%$dudvFP_+8+ecx*4]	; update texture ptr	; 4/3
		mov	[edi],edx		; put texel		; 4/2

		add	ebx,ebp			; vv += dvv		; 0
		mov	edx,%$duFP		; edx = duu		; 0
		sbb	ecx,ecx			; ecx = carry(vv+dvv)	; 1
		add	eax,edx			; uu += duu		; 1
		adc	ecx,ecx			; ebx = (ebx*2)+carry	; 2
		mov	edx,[esi]		; dx = texel		; 2
		add	esi,[%$dudvFP_+8+ecx*4]	; update texture ptr	; 4/3
		mov	[edi+4],edx		; put texel		; 4/2

		add	ebx,ebp			; vv += dvv		; 0
		mov	edx,%$duFP		; edx = duu		; 0
		sbb	ecx,ecx			; ecx = carry(vv+dvv)	; 1
		add	eax,edx			; uu += duu		; 1
		adc	ecx,ecx			; ebx = (ebx*2)+carry	; 2
		mov	edx,[esi]		; dx = texel		; 2
		add	esi,[%$dudvFP_+8+ecx*4]	; update texture ptr	; 4/3
		mov	[edi+8],edx		; put texel		; 4/2

		add	ebx,ebp			; vv += dvv		; 0
		mov	edx,%$duFP		; edx = duu		; 0
		sbb	ecx,ecx			; ecx = carry(vv+dvv)	; 1
		add	eax,edx			; uu += duu		; 1
		adc	ecx,ecx			; ebx = (ebx*2)+carry	; 2
		mov	edx,[esi]		; dx = texel		; 2
		add	esi,[%$dudvFP_+8+ecx*4]	; update texture ptr	; 4/3
		mov	[edi+12],edx		; put texel		; 4/2

		add	ebx,ebp			; vv += dvv		; 0
		mov	edx,%$duFP		; edx = duu		; 0
		sbb	ecx,ecx			; ecx = carry(vv+dvv)	; 1
		add	eax,edx			; uu += duu		; 1
		adc	ecx,ecx			; ebx = (ebx*2)+carry	; 2
		mov	edx,[esi]		; dx = texel		; 2
		add	esi,[%$dudvFP_+8+ecx*4]	; update texture ptr	; 4/3
		mov	[edi+16],edx		; put texel		; 4/2

		add	ebx,ebp			; vv += dvv		; 0
		mov	edx,%$duFP		; edx = duu		; 0
		sbb	ecx,ecx			; ecx = carry(vv+dvv)	; 1
		add	eax,edx			; uu += duu		; 1
		adc	ecx,ecx			; ebx = (ebx*2)+carry	; 2
		mov	edx,[esi]		; dx = texel		; 2
		add	esi,[%$dudvFP_+8+ecx*4]	; update texture ptr	; 4/3
		mov	[edi+20],edx		; put texel		; 4/2

		add	ebx,ebp			; vv += dvv		; 0
		mov	edx,%$duFP		; edx = duu		; 0
		sbb	ecx,ecx			; ecx = carry(vv+dvv)	; 1
		add	eax,edx			; uu += duu		; 1
		adc	ecx,ecx			; ebx = (ebx*2)+carry	; 2
		mov	edx,[esi]		; dx = texel		; 2
		add	esi,[%$dudvFP_+8+ecx*4]	; update texture ptr	; 4/3
		mov	[edi+24],edx		; put texel		; 4/2

		add	ebx,ebp			; vv += dvv		; 0
		mov	edx,%$duFP		; edx = duu		; 0
		sbb	ecx,ecx			; ecx = carry(vv+dvv)	; 1
		add	eax,edx			; uu += duu		; 1
		adc	ecx,ecx			; ebx = (ebx*2)+carry	; 2
		mov	edx,[esi]		; dx = texel		; 2
		add	esi,[%$dudvFP_+8+ecx*4]	; update texture ptr	; 4/3
		mov	[edi+28],edx		; put texel		; 4/2

		add	edi,32
		cmp	edi,%$destend8
		jb	near %$sloop8
		cmp	edi,%$destend
		jae	%$sexit

%$sloop1:       add	ebx,ebp			; vv += dvv		; 0
		mov	edx,%$duFP		; edx = duu		; 0
		sbb	ecx,ecx			; ecx = carry(vv+dvv)	; 1
		add	eax,edx			; uu += duu		; 1
		adc	ecx,ecx			; ebx = (ebx*2)+carry	; 2
		mov	edx,[esi]		; dx = texel		; 2
		add	esi,[%$dudvFP_+8+ecx*4]	; update texture ptr	; 4/3
		mov	[edi],edx		; put texel		; 4/2

		add	edi,4
		cmp	edi,%$destend
		jb	%$sloop1

%$sexit:
	%ifdef PIC
		pop	ebx
	%endif
%endmacro
%define		scanloop_map_fini zfill

;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
; Summary:
;   The internal scanloop for mmx_scan_map_zfil
;   Draw one horizontal scanline (with lighting) using MMX
; Arguments:
;   none
; Example:
;   scanproc 32,mmx_scan_map_zfil,SCANPROC_MAP|SCANPROC_MMX,mmx_scanloop_map
;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
%define		mmx_scanloop_map_args scanloop_map_args
%define		mmx_scanloop_map_init scanloop_map_init
%define		mmx_scanloop_map_body scanloop_map_body
%define		mmx_scanloop_map_fini mmx_zfill

;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
; Summary:
;   The internal scanloop for scan_map_zuse
;   Draw one horizontal scanline (Z buffer and lighting).
; Arguments:
;   none
; Example:
;   scanproc 32,scan_map_zuse,SCANPROC_MAP,scanloop_map_z
;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
%define		scanloop_map_z_args 24
%macro		scanloop_map_z_init 0
		loc	%$dudvFP,16	; fixed-point value of (dv * tex_w + du)
		tloc	%$duFP		; fixed-point duu
		tloc	%$dvFP		; fixed-point dvv
%endmacro
%macro		scanloop_map_z_body 0
	%ifdef PIC
		push	ebx
	%endif

; dudvInt[1] = ((dvv >> 16) << shifter) + (duu >> 16);
		mov	eax,%$dvv		; eax = dvv		; 0
		mov	edx,%$duu		; edx = duu		; 0
		sar	eax,16			; eax = dvv >> 16	; 1
		mov	esi,bitmap2		; esi = texture ptr	; 1
		sar	edx,16			; edx = duu >> 16	; 2
		mov	ecx,shf_u		; ecx = shifter		; 2
		add	ecx,2			; ecx += log2(pixsize)	; 3
		shl	eax,cl			; eax = (dvv >> 16) << s; 4/4
; dudvInt[0] = dudvInt[1] + (1 << shifter);
		mov	ebx,%$vv		; ebx = vv		; 4
		lea	eax,[eax+edx*4]		; eax = dudvInt[1]	; 8
		mov	edx,1			; edx = 1		; 8
		shl	edx,cl			; edx = 1 << shifter	; 9/4
		mov	[%$dudvFP_+8],eax	; dudvInt[2] = eax	; 9
		add	eax,4			; eax += pixsize	; 13
		sar	ebx,16			; ebx = vv >> 16	; 14
		mov	[%$dudvFP_+12],eax	; dudvInt[3] = eax+2	; 14
		add	eax,edx			; eax = dudvInt[1]	; 15
		mov	edx,%$uu		; edx = uu		; 15
		sar	edx,16			; edx = uu >> 16	; 16
		mov	[%$dudvFP_+4],eax	; dudvInt[1] = eax	; 16
		sub	eax,4			; eax = dudvInt[0]	; 17
		mov	[%$dudvFP_+0],eax	; dudvInt[0] = eax	; 18
; unsigned char *s = srcTex + ((vv >> 16) << shifter) + (uu >> 16);
		mov	eax,%$uu					; 18
		shl	ebx,cl						; 19/4
		test	[edi],eax		; fetch dest into cache	; 19
		lea	esi,[esi+edx*4]		; esi = source texture	; 23
		add	esi,ebx						; 24

; uFrac = uu << 16;
; vFrac = vv << 16;
		mov	ebx,%$vv					; 24
		shl	eax,16			; eax = uu << 16	; 25
		mov	ebp,%$dvv					; 25
		shl	ebx,16			; ebx = vv << 16	; 26
; duFrac = duu << 16;
; dvFrac = dvv << 16;
		mov	ecx,%$duu					; 26
		shl	ebp,16			; ebp = dvv << 16	; 27
		shl	ecx,16			; ecx = duu << 16	; 28
		mov	%$dvFP,ebp					; 29
		mov	%$duFP,ecx					; 29

; --------------// Register usage //--------------
; EAX	frac(uu)	mem	frac(duu)
; EBX	frac(vv)	mem	frac(dvv)
; ECX	1/z		EDX	--//scratch//--
; ESI	texture		EDI	dest
; EBP	Z-buffer

		mov	ebp,%$zbuff
		mov	ecx,%$izz

%$zloop:	mov	edx,[ebp]		; Get Z value		; 0
		cmp	ecx,edx			; Check Z-buffer	; 1
		jb	%$zbelow		; We're below surface	; 1
		mov	edx,[esi]		; Get texel		; 2
		mov	[ebp],ecx		; *zbuff = z		; 2
  		mov	[edi],edx		; Put texel		; 3
%$zbelow:	add	ebx,%$dvFP 		; vv += dvv		; 3?
		sbb	edx,edx			; carry (vv + dvv)	; 4
		add	eax,%$duFP		; uu += duu		; 4/2
		adc	edx,edx			; *2+carry (uu + duu)	; 6
		add	edi,4			; dest++		; 6
		adc	esi,[%$dudvFP_+8+edx*4]	; Source texture ptr	; 7/2
		add	ecx,%$dzz		; z = z + dz		; 7/2
		add	ebp,4			; zbuff++		; 9
		cmp	edi,%$destend		; dest < destend?	; 9
		jb	%$zloop						; 10

		mov	%$zbuff,ebp
		mov	%$izz,ecx

	%ifdef PIC
		pop	ebx
	%endif
%endmacro
%define		scanloop_map_z_fini

;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
; Summary:
;   The internal scanloop for scan_map_fixalpha50
;   Draw one horizontal scanline (transparency and lighting).
; Arguments:
;   none
; Example:
;   scanproc 32,scan_map_fixalpha50,SCANPROC_MAP,scanloop_map_a50
;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
%define		scanloop_map_a50_args 28
%macro		scanloop_map_a50_init 0
		loc	%$dudvFP,16	; fixed-point value of (dv * tex_w + du)
		tloc	%$duFP		; fixed-point duu
		tloc	%$dvFP		; fixed-point dvv
		tloc	%$destend4	; dest+(destend-dest) & ~3
%endmacro
%macro		scanloop_map_a50_body 0
	%ifdef PIC
		push	ebx
	%endif
; dudvInt[1] = ((dvv >> 16) << shifter) + (duu >> 16);
		mov	eax,%$dvv		; eax = dvv		; 0
		mov	edx,%$duu		; edx = duu		; 0
		sar	eax,16			; eax = dvv >> 16	; 1
		mov	esi,bitmap2		; esi = texture ptr	; 1
		sar	edx,16			; edx = duu >> 16	; 2
		mov	ecx,shf_u		; ecx = shifter		; 2
		add	ecx,2			; ecx += log2(pixsize)	; 3
		shl	eax,cl			; eax = (dvv >> 16) << s; 4/4
; dudvInt[0] = dudvInt[1] + (1 << shifter);
		mov	ebx,%$vv		; ebx = vv		; 4
		lea	eax,[eax+edx*4]		; eax = dudvInt[1]	; 8
		mov	edx,1			; edx = 1		; 8
		shl	edx,cl			; edx = 1 << shifter	; 9/4
		mov	[%$dudvFP_+8],eax	; dudvInt[2] = eax	; 9
		add	eax,4			; eax += pixsize	; 13
		sar	ebx,16			; ebx = vv >> 16	; 14
		mov	[%$dudvFP_+12],eax	; dudvInt[3] = eax+2	; 14
		add	eax,edx			; eax = dudvInt[1]	; 15
		mov	edx,%$uu		; edx = uu		; 15
		sar	edx,16			; edx = uu >> 16	; 16
		mov	[%$dudvFP_+4],eax	; dudvInt[1] = eax	; 16
		sub	eax,4			; eax = dudvInt[0]	; 17
		mov	[%$dudvFP_+0],eax	; dudvInt[0] = eax	; 18
; unsigned char *s = srcTex + ((vv >> 16) << shifter) + (uu >> 16);
		mov	eax,%$uu					; 18
		shl	ebx,cl						; 19/4
		test	[edi],eax		; fetch dest into cache	; 19
		lea	esi,[esi+edx*4]		; esi = source texture	; 23
		mov	edx,%$destend					; 23
		add	esi,ebx						; 24

; uFrac = uu << 16;
; vFrac = vv << 16;
		mov	ebx,%$vv					; 24
		shl	eax,16			; eax = uu << 16	; 25
		mov	ebp,%$dvv					; 25
		shl	ebx,16			; ebx = vv << 16	; 26
; duFrac = duu << 16;
; dvFrac = dvv << 16;
		mov	ecx,%$duu					; 26
		shl	ebp,16			; ebp = dvv << 16	; 27
		sub	edx,edi						; 27
		shl	ecx,16			; ecx = duu << 16	; 28
		and	edx,~15			; (destend-dest) & ~3	; 28
		mov	%$duFP,ecx					; 29
		mov	%$dvFP,ebp					; 29
		jz	near %$sloop1					; 29

		add	edx,edi						; 30
		mov	%$destend4,edx					; 30

; --------------// Register usage //--------------
; EAX	frac(uu)	mem	frac(duu)
; EBX	frac(vv)	mem	frac(dvv)
; ECX	--//scratch//--	EDX	--//scratch//--
; ESI	texture		EDI	dest
; EBP	work

%$sloop4:       mov	ecx,[edi]		; Get pixel		; 0
		mov	edx,[esi]		; Get texel		; 0
		and	ecx,0xfefefe		; remove lower bits	; 1
		and	edx,0xfefefe		; remove lower bits	; 1
		mov	ebp,%$dvFP		; ebp = dvv		; 2
		add	ecx,edx			; tex' + pix'		; 2
		add	ebx,ebp			; vv += dvv		; 3
		mov	ebp,%$duFP		; ebp = duu		; 3
		sbb	edx,edx			; carry flag		; 4
		add	eax,ebp			; uu += duu		; 4
		adc	edx,edx			; *2 + carry (uu + duu)	; 5 (v stall)
		shr	ecx,1			; tex+pix/2		; 6
		mov	ebp,%$dvFP		; ebp = dvv		; 6
		mov	edx,[%$dudvFP_+8+edx*4]	; get delta texture	; 7
		mov	[edi],ecx		; Put 2 pixels		; 7
		add	esi,edx			; update texture ptr	; 8

		mov	ecx,[edi+4]		; Get pixel		; 8
		mov	edx,[esi]		; Get texel		; 9
		and	ecx,0xfefefe		; remove lower bits	; 9
		and	edx,0xfefefe		; remove lower bits	; 10
		mov	ebp,%$dvFP		; ebp = dvv		; 10
		add	ecx,edx			; tex' + pix'		; 11
		add	ebx,ebp			; vv += dvv		; 11
		mov	ebp,%$duFP		; ebp = duu		; 12 (v stall)
		sbb	edx,edx			; carry flag		; 13
		add	eax,ebp			; uu += duu		; 13
		adc	edx,edx			; *2 + carry (uu + duu)	; 14 (v stall)
		shr	ecx,1			; tex+pix/2		; 15
		mov	ebp,%$dvFP		; ebp = dvv		; 15
		mov	edx,[%$dudvFP_+8+edx*4]	; get delta texture	; 16
		mov	[edi+4],ecx		; Put 2 pixels		; 16
		add	esi,edx			; update texture ptr	; 17

		mov	ecx,[edi+8]		; Get pixel		; 18
		mov	edx,[esi]		; Get texel		; 19
		and	ecx,0xfefefe		; remove lower bits	; 19
		and	edx,0xfefefe		; remove lower bits	; 20
		mov	ebp,%$dvFP		; ebp = dvv		; 20
		add	ecx,edx			; tex' + pix'		; 21
		add	ebx,ebp			; vv += dvv		; 21
		mov	ebp,%$duFP		; ebp = duu		; 22 (v stall)
		sbb	edx,edx			; carry flag		; 23
		add	eax,ebp			; uu += duu		; 23
		adc	edx,edx			; *2 + carry (uu + duu)	; 24 (v stall)
		shr	ecx,1			; tex+pix/2		; 25
		mov	ebp,%$dvFP		; ebp = dvv		; 25
		mov	edx,[%$dudvFP_+8+edx*4]	; get delta texture	; 26
		mov	[edi+8],ecx		; Put 2 pixels		; 26
		add	esi,edx			; update texture ptr	; 27

		mov	ecx,[edi+12]		; Get pixel		; 28
		mov	edx,[esi]		; Get texel		; 29
		and	ecx,0xfefefe		; remove lower bits	; 29
		and	edx,0xfefefe		; remove lower bits	; 30
		mov	ebp,%$dvFP		; ebp = dvv		; 30
		add	ecx,edx			; tex' + pix'		; 31
		add	ebx,ebp			; vv += dvv		; 31
		mov	ebp,%$duFP		; ebp = duu		; 32
		lea	edi,[edi+16]		; dest++		; 32
		sbb	edx,edx			; carry flag		; 33
		add	eax,ebp			; uu += duu		; 33
		adc	edx,edx			; *2 + carry (uu + duu)	; 34 (v stall)
		shr	ecx,1			; tex+pix/2		; 35
		mov	ebp,%$dvFP		; ebp = dvv		; 35
		mov	edx,[%$dudvFP_+8+edx*4]	; get delta texture	; 36
		mov	[edi-4],ecx		; Put 2 pixels		; 36
		add	esi,edx			; update texture ptr	; 37

		cmp	edi,%$destend4					; 37
		jb	near %$sloop4					; 38
		cmp	edi,%$destend
		jae	%$sexit

%$sloop1:	mov	ecx,[edi]		; Get pixel		; 0
		mov	edx,[esi]		; Get texel		; 0
		and	ecx,0xfefefe		; Prepare pixel		; 1
		and	edx,0xfefefe		; Prepare texel		; 1
		mov	ebp,%$dvFP		; ebp = dvv		; 1
		add	ecx,edx			; tex' + pix'		; 2
		add	ebx,ebp			; vv += dvv		; 2
		mov	ebp,%$duFP		; ebp = duu		; 3
		lea	edi,[edi+4]		; dest++		; 3
		sbb	edx,edx			; carry flag		; 4
		add	eax,ebp			; uu += duu		; 4
		adc	edx,edx			; *2 + carry (uu + duu)	; 5 (v stall)
		shr	ecx,1			; tex+pix/2		; 6
		mov	ebp,%$dvFP		; ebp = dvv		; 6
		mov	edx,[%$dudvFP_+8+edx*4]	; get delta texture	; 7
		mov	[edi-4],ecx		; Put 2 pixels		; 7
		add	esi,edx			; update texture ptr	; 8

		cmp	edi,%$destend		; we're finished?	; 8
		jb	%$sloop1
%$sexit:
	%ifdef PIC
		pop	ebx
	%endif
%endmacro
%define		scanloop_map_a50_fini

;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
; Summary:
;   The internal scanloop for scan_tex_zfil
;   Draw one horizontal scanline (no lighting)
; Arguments:
;   none
; Example:
;   scanproc 32,scan_tex_zfil,0,scanloop_tex
;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
%define		scanloop_tex_args 12
%macro		scanloop_tex_init 0
		tloc	%$and_w		; texture width mask
		tloc	%$and_h		; texture height mask
		tloc	%$paltable	; 8->32 conversion table

		mov	eax,and_h
		mov	ecx,and_w
		mov	ebp,PaletteTable
		mov	%$and_h,eax
		mov	%$and_w,ecx
		mov	%$paltable,ebp
%endmacro
%macro		scanloop_tex_body 0
	%ifdef PIC
		push	ebx
	%endif
		mov	esi,bitmap
		mov	ecx,shf_h
		mov	eax,%$uu
		mov	ebx,%$vv

; --------------// Register usage //--------------
; EAX	uu		ESI	source texture
; EBX	vv		EDI	dest video ram
; ECX	shf_h		EBP	--//scratch//--
; EDX	--//scratch//--

;  do
;  {
;    *_dest++ = Scan.PaletteTable[srcTex[((uu>>16)&ander_w) + ((vv>>shifter_h)&ander_h)]];
;    uu += duu;
;    vv += dvv;
;  }
;  while (_dest <= _destend)
%$sloop:	mov	edx,ebx		; edx = vv			; 0
		mov	ebp,eax		; ebp = uu			; 0
		shr	edx,cl		; edx = vv >> shf_h		; 1/4
		add	ebx,%$dvv	; vv += dvv			; 1/2
		shr	ebp,16		; ebp = uu >> 16		; 5
		and	ebp,%$and_w	; ebp = (uu >> 16) & and_w	; 6/2
		and	edx,%$and_h	; edx = (vv >> shf_h) & and_h	; 6/2
		add	ebp,edx						; 8
		xor	edx,edx						; 8
		mov	dl,[esi+ebp]					; 9
		mov	ebp,%$paltable					; 9
		add	eax,%$duu					; 10/2
		mov	edx,[ebp+edx*4]					; 10
		mov	[edi],edx					; 12
		add	edi,4						; 12
		cmp	edi,%$destend					; 13
		jb	%$sloop						; 13

%$sexit:
	%ifdef PIC
		pop	ebx
	%endif
%endmacro
%define		scanloop_tex_fini zfill

;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
; Summary:
;   The internal scanloop for mmx_scan_tex_zfil
;   Draw one horizontal scanline (no lighting) using MMX
; Arguments:
;   none
; Example:
;   scanproc 32,mmx_scan_tex_zfil,SCANPROC_MAP,mmx_scanloop_tex
;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
%define		mmx_scanloop_tex_args scanloop_tex_args
%define		mmx_scanloop_tex_init scanloop_tex_init
%define		mmx_scanloop_tex_body scanloop_tex_body
%define		mmx_scanloop_tex_fini mmx_zfill

;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
; Summary:
;   Draw a fog scanline when we're not inside fog
; Arguments:
;   int xx, unsigned char *d, unsigned long *z_buf,
;   float inv_z, float u_div_z, float v_div_z
;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
proc		csScan_32_scan_fog,36,ebx,esi,edi,ebp
		targ	%$width		; int
		targ	%$dest		; unsigned char *
		targ	%$zbuff		; unsigned long *
		targ	%$inv_z		; float
		targ	%$u_div_z	; float
		targ	%$v_div_z	; float

		tloc	%$destend	; unsigned char * (dest + width)
		tloc	%$const24	; float 16777216
		tloc	%$dzz		; csQfixed24 (Scan.M)
		tloc	%$izz		; csQfixed24 (inv_z)
		tloc	%$fogpix	; r|g|b
		tloc	%$fogdens	; FogDensity
		tloc	%$fogr		; FogR
		tloc	%$fogg		; FogG
		tloc	%$fogb		; FogB

; if (xx <= 0) return;
		mov	eax,%$width
		mov	edi,%$dest
		test	eax,eax
		exit	le

		lea	eax,[edi+eax*4]
		mov	ecx,0x4B800000	; 16777216F
		mov	%$destend,eax
		mov	%$const24,ecx

; unsigned long izz = csQfixed24 (inv_z);
; int dzz = csQfixed24 (Scan.M);
		fld	%$inv_z		; inv_z				; 0/1
		fld	M		; M,inv_z			; 1/1
		fmul	%$const24	; M*16777216,inv_z		; 2/3
		fxch			; inv_z,M*16777216		; 2
		fmul	%$const24	; inv_z*16777216,M*16777216	; 3/3
		fxch			; M*16777216,inv_z*16777216	; 3
		fistp	%$dzz		; inv_z*16777216		; 5/6
		fistp	%$izz		; 				; 6/6

; Copy global vars to local stack
		mov	eax,FogPix					; 7
		mov	ecx,FogDensity					; 7
		mov	%$fogpix,eax					; 8
		mov	%$fogdens,ecx					; 8
		mov	eax,FogR					; 9
		mov	ecx,FogG					; 9
		mov	edx,FogB					; 10
		mov	%$fogr,eax					; 10
		mov	%$fogg,ecx					; 11
		mov	%$fogb,edx					; 11

; EAX	/	  ESI	izz
; EBX	fd	  EDI	dest
; ECX	/	  EBP	zbuff
; EDX   /
		mov	esi,%$izz					; 12
		mov	ebp,%$zbuff					; 12
		xor	ebx,ebx						; 13
%$fogloop:
; if (izz >= 0x1000000)
		mov	eax,[ebp]					; 0
		mov	ecx,esi						; 0
		cmp	esi,0x01000000					; 1
		if	ae,short					; 1
	; izz exceeds our 1/x table, so compute fd aproximatively and go on.
	; This happens seldom, only when we're very close to fog, but not
	; inside it; however we should handle this case as well.
; if ((izb < 0x1000000) && (izz > izb))
			cmp	eax,0x01000000				; 2
			jae	near %$endif				; 2
			cmp	esi,eax					; 3
			jb	near %$endif				; 3

; fd = fog_dens * (tables.one_div_z [izb >> 12] - (tables.one_div_z [izz >> 20] >> 8)) >> 12;
			shr	eax,12					; 4
			mov	edx,one_div_z				; 4
			shr	ecx,20					; 5 (np)
			mov	eax,[edx+eax*4]				; 6 (np)
			mov	ecx,[edx+ecx*4]				; 7
			mov	edx,%$fogdens				; 7
			shr	ecx,8					; 8 (np)
			sub	eax,ecx					; 9
			mul	edx					; 10(9)
			shr	eax,12					; 19

			mov	ecx,exp_256				; 19
			mov	edx,%$fogpix				; 20
; if (fd < EXP_256_SIZE)
			cmp	eax,EXP_256_SIZE			; 20
			jl	%$get_fd				; 21
		else
; if (izz > izb)
			cmp	esi,eax					; 2
			jle	near %$endif				; 2

; fd = fog_dens * (tables.one_div_z [izb >> 12] - tables.one_div_z [izz >> 12]) >> 12;
			shr	eax,12					; 3
			mov	edx,one_div_z				; 3
			shr	ecx,12					; 4 (np)
			mov	eax,[edx+eax*4]				; 5 (np)
			mov	ecx,[edx+ecx*4]				; 6
			mov	edx,%$fogdens				; 6
			sub	eax,ecx					; 7 (np)
			mul	edx					; 8(9)
			shr	eax,12					; 17

			mov	ecx,exp_256				; 17
			mov	edx,%$fogpix				; 18
; if (fd < EXP_256_SIZE)
			cmp	eax,EXP_256_SIZE			; 18
			mov	%$izz,esi		; save izz	; 19
			if	l,short
%$$get_fd:
; fd = tables.exp_256 [fd];
				mov	edx,[edi]	; get pixel	; 0
				mov	bl,[ecx+eax]			; 0
; r = (fd * ((*_dest & 0xff0000) - Scan.FogR) >> 8) + Scan.FogR;
				mov	esi,edx		; p		; 1
				mov	eax,%$fogr	; r		; 1
				and	edx,0xff0000	; p & ff0000	; 2
				sub	edx,eax		; - r		; 3
				imul	edx,ebx		; * fd		; 4(9)
				mov	ecx,%$fogg	; g		; 4
				shr	edx,8		; >> 8		; 13
				add	eax,edx		; + r		; 14
				mov	edx,esi		; p		; 14
				and	eax,0xff0000	; r & ff0000	; 15
; g = (fd * ((*_dest & 0x00ff00) - Scan.FogG) >> 8) + Scan.FogG;
				and	edx,0x00ff00	; p & 00ff00	; 15
				sub	edx,ecx		; - g		; 16
				and	esi,0x0000ff	; p & 0000ff	; 16
				imul	edx,ebx		; * fd		; 17(9)
				shr	edx,8		; >> 8		; 26
				add	ecx,edx		; + g		; 27
				mov	edx,%$fogb	; b		; 27
				and	ecx,0x00ff00	; & 00ff00	; 28
; b = (fd * ((*_dest & 0x0000ff) - Scan.FogB) >> 8) + Scan.FogB;
				sub	esi,edx		; - b		; 28
				or	eax,ecx		; r | g		; 29
				imul	esi,ebx		; * fd		; 29(9)
				shr	esi,8		; >> 8		; 38
				add	edx,esi		; + b		; 38
				mov	esi,%$izz	; restore izz	; 39
				or	edx,eax		; r | g | b	; 40
			endif
			mov    [edi],edx
		endif

		add	edi,4			; _dest++;
		mov	eax,%$dzz
		add	ebp,4			; z_buf++;
		add	esi,eax			; izz += dzz;
		cmp	edi,%$destend
		jb	near %$fogloop
%$fogexit:
endproc

;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
; Summary:
;   Draw a fog scanline when we're inside fog
; Arguments:
;   int xx, unsigned char *d, unsigned long *z_buf,
;   float inv_z, float u_div_z, float v_div_z
;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
proc		csScan_32_scan_fog_view,28,ebx,esi,edi,ebp
		targ	%$width		; int
		targ	%$dest		; unsigned char *
		targ	%$zbuff		; unsigned long *
		targ	%$inv_z		; float
		targ	%$u_div_z	; float
		targ	%$v_div_z	; float

		tloc	%$destend	; unsigned char * (dest + width)
		tloc	%$fogpix	; r|g|b
		tloc	%$fogdens	; FogDensity
		tloc	%$fogr		; FogR
		tloc	%$fogg		; FogG
		tloc	%$fogb		; FogB
		tloc	%$pix		; *dest

; if (xx <= 0) return;
		mov	eax,%$width
		mov	edi,%$dest
		test	eax,eax
		exit	le

		lea	eax,[edi+eax*4]
		mov	%$destend,eax

; Copy global vars to local stack
		mov	eax,FogPix					; 0
		mov	ecx,FogDensity					; 0
		mov	%$fogpix,eax					; 1
		mov	%$fogdens,ecx					; 1
		mov	eax,FogR					; 2
		mov	ecx,FogG					; 2
		mov	edx,FogB					; 3
		mov	%$fogr,eax					; 3
		mov	%$fogg,ecx					; 4
		mov	%$fogb,edx					; 4

; EAX	/	  ESI	/
; EBX	fd	  EDI	dest
; ECX	/	  EBP	zbuff
; EDX   /
		mov	ebp,%$zbuff					; 5
		xor	ebx,ebx						; 5
%$fogloop:
; if (izz >= 0x1000000)
		mov	eax,[ebp]					; 0
		add	ebp,4			; z_buf++;		; 0
		cmp	eax,0x01000000					; 1
		if	b						; 1
; fd = fog_dens * Scan.one_div_z [izb >> 12] >> 12;
			shr	eax,12					; 3
			mov	esi,one_div_z				; 3
			mov	edx,%$fogdens				; 4 (np)
			mov	eax,[esi+eax*4]				; 5 (np)
			mul	edx					; 6(9)
			shr	eax,12					; 15

			mov	ecx,exp_256				; 15
			mov	edx,%$fogpix				; 16
; if (fd < EXP_256_SIZE)
			cmp	eax,EXP_256_SIZE			; 16
			if	l,short
%$$get_fd:
; fd = tables.exp_256 [fd];
				mov	edx,[edi]	; get pixel	; 0
				mov	bl,[ecx+eax]			; 0
; r = (fd * ((*_dest & 0xff0000) - Scan.FogR) >> 8) + Scan.FogR;
				mov	esi,edx		; p		; 1
				mov	eax,%$fogr	; r		; 1
				and	edx,0xff0000	; p & ff0000	; 2
				sub	edx,eax		; - r		; 3
				imul	edx,ebx		; * fd		; 4(9)
				mov	ecx,%$fogg	; g		; 4
				shr	edx,8		; >> 8		; 13
				add	eax,edx		; + r		; 14
				mov	edx,esi		; p		; 14
				and	eax,0xff0000	; r & ff0000	; 15
; g = (fd * ((*_dest & 0x00ff00) - Scan.FogG) >> 8) + Scan.FogG;
				and	edx,0x00ff00	; p & 00ff00	; 15
				sub	edx,ecx		; - g		; 16
				and	esi,0x0000ff	; p & 0000ff	; 16
				imul	edx,ebx		; * fd		; 17(9)
				shr	edx,8		; >> 8		; 26
				add	ecx,edx		; + g		; 27
				mov	edx,%$fogb	; b		; 27
				and	ecx,0x00ff00	; & 00ff00		; 28
; b = (fd * ((*_dest & 0x0000ff) - Scan.FogB) >> 8) + Scan.FogB;
				sub	esi,edx		; - b		; 28
				or	eax,ecx		; r | g		; 29
				imul	esi,ebx		; * fd		; 29(9)
				shr	esi,8		; >> 8		; 38
				add	edx,esi		; + b		; 38
				or	edx,eax		; r | g | b	; 39
			endif
			mov	[edi],edx
		endif

		add	edi,4			; _dest++;
		cmp	edi,%$destend
		jb	near %$fogloop
%$fogexit:
endproc

scanproc 32,scan_map_zfil,SCANPROC_MAP,scanloop_map
scanproc 32,mmx_scan_map_zfil,SCANPROC_MAP|SCANPROC_MMX,mmx_scanloop_map
scanproc 32,scan_map_zuse,SCANPROC_MAP,scanloop_map_z
scanproc 32,scan_map_fixalpha50,SCANPROC_MAP,scanloop_map_a50
scanproc 32,scan_tex_zfil,SCANPROC_TEX,scanloop_tex
scanproc 32,mmx_scan_tex_zfil,SCANPROC_TEX|SCANPROC_MMX,mmx_scanloop_tex
