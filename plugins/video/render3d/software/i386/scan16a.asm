;
;   Copyright (C) 1998 by Jorrit Tyberghein
;   Written by Andrew Zabolotny
;   16-bit scanline drawing routines
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
;   scanproc 16,scan_map_zfil,SCANPROC_MAP,scanloop_map
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
		inc	ecx			; ecx += log2(pixsize)	; 3
		shl	eax,cl			; eax = (dvv >> 16) << s; 4/4
; dudvInt[0] = dudvInt[1] + (1 << shifter);
		mov	ebx,%$vv		; ebx = vv		; 4
		lea	eax,[eax+edx*2]		; eax = dudvInt[1]	; 8
		mov	edx,1			; edx = 1		; 8
		shl	edx,cl			; edx = 1 << shifter	; 9/4
		mov	[%$dudvFP_+8],eax	; dudvInt[2] = eax	; 9
		add	eax,2			; eax += pixsize	; 13
		sar	ebx,16			; ebx = vv >> 16	; 14
		mov	[%$dudvFP_+12],eax	; dudvInt[3] = eax+2	; 14
		add	eax,edx			; eax = dudvInt[1]	; 15
		mov	edx,%$uu		; edx = uu		; 15
		sar	edx,16			; edx = uu >> 16	; 16
		mov	[%$dudvFP_+4],eax	; dudvInt[1] = eax	; 16
		sub	eax,2			; eax = dudvInt[0]	; 17
		mov	[%$dudvFP_+0],eax	; dudvInt[0] = eax	; 18
; unsigned char *s = srcTex + ((vv >> 16) << shifter) + (uu >> 16);
		mov	eax,%$uu					; 18
		shl	ebx,cl						; 19/4
		test	[edi],al		; fetch dest into cache	; 19
		lea	esi,[esi+edx*2]		; esi = source texture	; 23
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
		and	edx,~15			; (destend-dest) & ~7	; 28
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
		mov	[edi],dx		; put texel		; 4/2

		add	ebx,ebp			; vv += dvv		; 0
		mov	edx,%$duFP		; edx = duu		; 0
		sbb	ecx,ecx			; ecx = carry(vv+dvv)	; 1
		add	eax,edx			; uu += duu		; 1
		adc	ecx,ecx			; ebx = (ebx*2)+carry	; 2
		mov	edx,[esi]		; dx = texel		; 2
		add	esi,[%$dudvFP_+8+ecx*4]	; update texture ptr	; 4/3
		mov	[edi+2],dx		; put texel		; 4/2

		add	ebx,ebp			; vv += dvv		; 0
		mov	edx,%$duFP		; edx = duu		; 0
		sbb	ecx,ecx			; ecx = carry(vv+dvv)	; 1
		add	eax,edx			; uu += duu		; 1
		adc	ecx,ecx			; ebx = (ebx*2)+carry	; 2
		mov	edx,[esi]		; dx = texel		; 2
		add	esi,[%$dudvFP_+8+ecx*4]	; update texture ptr	; 4/3
		mov	[edi+4],dx		; put texel		; 4/2

		add	ebx,ebp			; vv += dvv		; 0
		mov	edx,%$duFP		; edx = duu		; 0
		sbb	ecx,ecx			; ecx = carry(vv+dvv)	; 1
		add	eax,edx			; uu += duu		; 1
		adc	ecx,ecx			; ebx = (ebx*2)+carry	; 2
		mov	edx,[esi]		; dx = texel		; 2
		add	esi,[%$dudvFP_+8+ecx*4]	; update texture ptr	; 4/3
		mov	[edi+6],dx		; put texel		; 4/2

		add	ebx,ebp			; vv += dvv		; 0
		mov	edx,%$duFP		; edx = duu		; 0
		sbb	ecx,ecx			; ecx = carry(vv+dvv)	; 1
		add	eax,edx			; uu += duu		; 1
		adc	ecx,ecx			; ebx = (ebx*2)+carry	; 2
		mov	edx,[esi]		; dx = texel		; 2
		add	esi,[%$dudvFP_+8+ecx*4]	; update texture ptr	; 4/3
		mov	[edi+8],dx		; put texel		; 4/2

		add	ebx,ebp			; vv += dvv		; 0
		mov	edx,%$duFP		; edx = duu		; 0
		sbb	ecx,ecx			; ecx = carry(vv+dvv)	; 1
		add	eax,edx			; uu += duu		; 1
		adc	ecx,ecx			; ebx = (ebx*2)+carry	; 2
		mov	edx,[esi]		; dx = texel		; 2
		add	esi,[%$dudvFP_+8+ecx*4]	; update texture ptr	; 4/3
		mov	[edi+10],dx		; put texel		; 4/2

		add	ebx,ebp			; vv += dvv		; 0
		mov	edx,%$duFP		; edx = duu		; 0
		sbb	ecx,ecx			; ecx = carry(vv+dvv)	; 1
		add	eax,edx			; uu += duu		; 1
		adc	ecx,ecx			; ebx = (ebx*2)+carry	; 2
		mov	edx,[esi]		; dx = texel		; 2
		add	esi,[%$dudvFP_+8+ecx*4]	; update texture ptr	; 4/3
		mov	[edi+12],dx		; put texel		; 4/2

		add	ebx,ebp			; vv += dvv		; 0
		mov	edx,%$duFP		; edx = duu		; 0
		sbb	ecx,ecx			; ecx = carry(vv+dvv)	; 1
		add	eax,edx			; uu += duu		; 1
		adc	ecx,ecx			; ebx = (ebx*2)+carry	; 2
		mov	edx,[esi]		; dx = texel		; 2
		add	esi,[%$dudvFP_+8+ecx*4]	; update texture ptr	; 4/3
		mov	[edi+14],dx		; put texel		; 4/2

		add	edi,16
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
		mov	[edi],dx		; put texel		; 4/2

		add	edi,2
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
;   scanproc 16,mmx_scan_map_zfil,SCANPROC_MAP|SCANPROC_MMX,mmx_scanloop_map
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
;   scanproc 16,scan_map_zuse,SCANPROC_MAP,scanloop_map_z
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
		inc	ecx			; ecx += log2(pixsize)	; 3
		shl	eax,cl			; eax = (dvv >> 16) << s; 4/4
; dudvInt[0] = dudvInt[1] + (1 << shifter);
		mov	ebx,%$vv		; ebx = vv		; 4
		lea	eax,[eax+edx*2]		; eax = dudvInt[1]	; 8
		mov	edx,1			; edx = 1		; 8
		shl	edx,cl			; edx = 1 << shifter	; 9/4
		mov	[%$dudvFP_+8],eax	; dudvInt[2] = eax	; 9
		add	eax,2			; eax += pixsize	; 13
		sar	ebx,16			; ebx = vv >> 16	; 14
		mov	[%$dudvFP_+12],eax	; dudvInt[3] = eax+2	; 14
		add	eax,edx			; eax = dudvInt[1]	; 15
		mov	edx,%$uu		; edx = uu		; 15
		sar	edx,16			; edx = uu >> 16	; 16
		mov	[%$dudvFP_+4],eax	; dudvInt[1] = eax	; 16
		sub	eax,2			; eax = dudvInt[0]	; 17
		mov	[%$dudvFP_+0],eax	; dudvInt[0] = eax	; 18
; unsigned char *s = srcTex + ((vv >> 16) << shifter) + (uu >> 16);
		mov	eax,%$uu					; 18
		shl	ebx,cl						; 19/4
		test	[edi],al		; fetch dest into cache	; 19
		lea	esi,[esi+edx*2]		; esi = source texture	; 23
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
		mov	dx,[esi]		; Get texel		; 2
		mov	[ebp],ecx		; *zbuff = z		; 2
  		mov	[edi],dx		; Put texel		; 3
%$zbelow:	add	ebx,%$dvFP 		; vv += dvv		; 3?
		sbb	edx,edx			; carry (vv + dvv)	; 4
		add	eax,%$duFP		; uu += duu		; 4/2
		adc	edx,edx			; *2+carry (uu + duu)	; 6
		add	edi,2			; dest++		; 6
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
;   scanproc 16,scan_map_fixalpha50,SCANPROC_MAP,scanloop_map_a50
;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
%define		scanloop_map_a50_args 36
%macro		scanloop_map_a50_init 0
		loc	%$dudvFP,16	; fixed-point value of (dv * tex_w + du)
		tloc	%$duFP		; fixed-point duu
		tloc	%$dvFP		; fixed-point dvv
		tloc	%$destend4	; dest+(destend-dest) & ~3
		tloc	%$texmask	; texel mask
		tloc	%$pixmask	; pixel mask

		mov	ecx,AlphaMask		; get alpha mask
		mov	eax,ecx			; propagate to high word
		mov	%$texmask,ecx		; texel mask
		shl	eax,16
		or	eax,ecx
		mov	%$pixmask,eax		; pixel mask
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
		inc	ecx			; ecx += log2(pixsize)	; 3
		shl	eax,cl			; eax = (dvv >> 16) << s; 4/4
; dudvInt[0] = dudvInt[1] + (1 << shifter);
		mov	ebx,%$vv		; ebx = vv		; 4
		lea	eax,[eax+edx*2]		; eax = dudvInt[1]	; 8
		mov	edx,1			; edx = 1		; 8
		shl	edx,cl			; edx = 1 << shifter	; 9/4
		mov	[%$dudvFP_+8],eax	; dudvInt[2] = eax	; 9
		add	eax,2			; eax += pixsize	; 13
		sar	ebx,16			; ebx = vv >> 16	; 14
		mov	[%$dudvFP_+12],eax	; dudvInt[3] = eax+2	; 14
		add	eax,edx			; eax = dudvInt[1]	; 15
		mov	edx,%$uu		; edx = uu		; 15
		sar	edx,16			; edx = uu >> 16	; 16
		mov	[%$dudvFP_+4],eax	; dudvInt[1] = eax	; 16
		sub	eax,2			; eax = dudvInt[0]	; 17
		mov	[%$dudvFP_+0],eax	; dudvInt[0] = eax	; 18
; unsigned char *s = srcTex + ((vv >> 16) << shifter) + (uu >> 16);
		mov	eax,%$uu					; 18
		shl	ebx,cl						; 19/4
		test	[edi],al		; fetch dest into cache	; 19
		lea	esi,[esi+edx*2]		; esi = source texture	; 23
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
		and	edx,~7			; (destend-dest) & ~3	; 28
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

		mov	ecx,%$pixmask		; get pixels mask	; 0
		mov	ebp,%$texmask		; get texel mask	; 0

%$sloop4:       and	ecx,[edi]		; Get/prepare 2 pixels	; 1/2
		and	ebp,[esi]		; Get/prepare texel	; 1/2
		add	ecx,ebp			; 0|tex' + pix2'|pix1'	; 3
		mov	ebp,%$dvFP		; ebp = dvv		; 3
		add	ebx,ebp			; vv += dvv		; 4
		mov	ebp,%$duFP		; ebp = duu		; 4
		sbb	edx,edx			; carry flag		; 5
		add	eax,ebp			; uu += duu		; 5
		adc	edx,edx			; *2 + carry (uu + duu)	; 6 (v stall)
		rol	ecx,16			; Next pixel		; 7 (does not pair)
		mov	ebp,[%$dudvFP_+8+edx*4]	; get delta texture	; 8
		mov	edx,%$dvFP		; edx = dvv		; 8
		add	esi,ebp			; update texture ptr	; 9

		mov	ebp,%$texmask		; get texel mask	; 9
		and	ebp,[esi]		; Get/prepare texel	; 10/3
		add	ecx,ebp			; 0|tex' + pix1'|pix2'	; 13
		rol	ecx,15			; pix /= 2		; 14
		add	ebx,edx			; vv += dvv		; 15
		mov	ebp,%$duFP		; ebp = duu		; 15
		sbb	edx,edx			; carry flag		; 16
		add	eax,ebp			; uu += duu		; 16
		adc	edx,edx			; *2 + carry (uu + duu)	; 17
		mov	[edi],ecx		; Put 2 pixels		; 17
		mov	ecx,%$pixmask		; get pixels mask	; 18
		mov	ebp,%$texmask		; get texel mask	; 18
		add	esi,[%$dudvFP_+8+edx*4]	; update texture ptr	; 19/2

		and	ecx,[edi+4]		; Get/prepare texel	; 19/2
		add	edi,8			; increment dest	; 21
		and	ebp,[esi]		; Get/prepare 2 pixels	; 21/2
		add	ecx,ebp			; 0|tex' + pix2'|pix1'	; 23
		mov	ebp,%$dvFP		; ebp = dvv		; 23
		add	ebx,ebp			; vv += dvv		; 24
		mov	ebp,%$duFP		; ebp = duu		; 24
		sbb	edx,edx			; carry flag		; 25
		add	eax,ebp			; uu += duu		; 25
		adc	edx,edx			; *2 + carry (uu + duu)	; 26 (v stall)
		rol	ecx,16			; Next pixel		; 27 (does not pair)
		mov	ebp,[%$dudvFP_+8+edx*4]	; get delta texture	; 28
		mov	edx,%$dvFP		; edx = dvv		; 28
		add	esi,ebp			; update texture ptr	; 29

		mov	ebp,%$texmask		; get texel mask	; 29
		and	ebp,[esi]		; Get/prepare texel	; 30/3
		add	ecx,ebp			; 0|tex' + pix1'|pix2'	; 33
		rol	ecx,15			; pix /= 2		; 34
		add	ebx,edx			; vv += dvv		; 35
		mov	ebp,%$duFP		; ebp = duu		; 35
		sbb	edx,edx			; carry flag		; 36
		add	eax,ebp			; uu += duu		; 36
		adc	edx,edx			; *2 + carry (uu + duu)	; 37
		mov	[edi-4],ecx		; Put 2 pixels		; 37
		mov	ecx,%$pixmask		; get pixels mask	; 38
		mov	ebp,%$texmask		; get texel mask	; 38
		add	esi,[%$dudvFP_+8+edx*4]	; update texture ptr	; 39/2

		cmp	edi,%$destend4					; 79
		jb	near %$sloop4					; 80
		cmp	edi,%$destend
		jae	%$sexit

%$sloop1:	mov	ecx,%$texmask		; get texel mask	; 0
		mov	edx,[esi]		; Get texel		; 0
		and	edx,ecx			; Prepare texel		; 1
		and	ecx,[edi]		; Get/prepare pixel	; 1/2
		add	ecx,edx			; tex' + pix1'		; 3
		mov	ebp,%$dvFP		; ebp = dvv		; 3
		add	ebx,ebp			; vv += dvv		; 4
		mov	ebp,%$duFP		; ebp = duu		; 4
		sbb	edx,edx			; carry flag		; 5
		add	eax,ebp			; uu += duu		; 5
		adc	edx,edx			; *2 + carry (uu + duu)	; 6 (v stall)
		shr	ecx,1			; Pixel ready		; 7 (v stall)
		add	esi,[%$dudvFP_+8+edx*4]	; get delta texture	; 8/2
		mov	[edi],cx		;			; 8

		add	edi,2			; increment dest ptr
		cmp	edi,%$destend		; we're finished?
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
;   scanproc 16,scan_tex_zfil,0,scanloop_tex
;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
%define		scanloop_tex_args 12
%macro		scanloop_tex_init 0
		tloc	%$and_w		; texture width mask
		tloc	%$and_h		; texture height mask
		tloc	%$paltable	; 8->16 conversion table

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
		mov	dx,[ebp+edx*2]					; 10
		mov	[edi],dx					; 12
		add	edi,2						; 12
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
;   scanproc 16,mmx_scan_tex_zfil,SCANPROC_MAP,mmx_scanloop_tex
;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
%define		mmx_scanloop_tex_args scanloop_tex_args
%define		mmx_scanloop_tex_init scanloop_tex_init
%define		mmx_scanloop_tex_body scanloop_tex_body
%define		mmx_scanloop_tex_fini mmx_zfill

;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
; Summary:
;   Draw a perspective-incorrect texture mapped polygon scanline
; Arguments:
;   void *dest, int width, unsigned long *zbuff,
;   long u, long du, long v, long dv, unsigned long z, long dz,
;   unsigned char *bitmap, int bitmap_log2w
;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
proc		csScan_16_scan_pi_tex_zuse,24,ebx,esi,edi,ebp
		targ	%$dest		; void *dest
		targ	%$width		; int width
		targ	%$zbuff		; long *zbuff
		targ	%$u		; long u
		targ	%$du		; long du
		targ	%$v		; long v
		targ	%$dv		; long dv
		targ	%$z		; long z
		targ	%$dz		; long dz
		targ	%$bitmap	; unsigned char *bitmap
		targ	%$bitmap_log2w	; int bitmap_log2w

		loc	%$dudvFP,8	; fixed-point value of (dv * tex_w + du)
		tloc	%$duFP		; fixed-point duu
		tloc	%$dvFP		; fixed-point dvv
		tloc	%$destend
		tloc	%$paltable	; 8->16 conversion table

; if (len <= 0)
;   return;
		mov	eax,%$width
		mov	edi,%$dest
		test	eax,eax
		exit	le

		lea	eax,[edi+eax*2]
		mov	ebp,PaletteTable
		mov	%$destend,eax
		mov	%$paltable,ebp

; dudvInt[1] = ((dv >> 16) << bitmap_log2w) + (du >> 16);
		mov	eax,%$dv		; eax = dv		; 0
		mov	edx,%$du		; edx = du		; 0
		sar	eax,16			; eax = dv >> 16	; 1
		mov	esi,%$bitmap		; esi = texture ptr	; 1
		sar	edx,16			; edx = du >> 16	; 2
		mov	ecx,%$bitmap_log2w	; ecx = bitmap_log2w	; 2
		shl	eax,cl			; eax = (dv >> 16) << s	; 3/4
; dudvInt[0] = dudvInt [1] + (1 << bitmap_log2w);
		mov	ebx,%$v			; ebx = v		; 3
		add	eax,edx			; eax = dudvInt[1]	; 7
		mov	edx,1			; edx = 1		; 7
		shl	edx,cl			; edx = 1 << bitmap_l2w	; 8/4
		mov	[%$dudvFP_+4],eax	; dudvInt[1] = eax	; 8
		sar	ebx,16			; ebx = v >> 16		; 12
		add	eax,edx			; eax = dudvInt[0]	; 12
		mov	[%$dudvFP_+0],eax	; dudvInt[0] = eax	; 13
; unsigned char *src = bitmap + ((v >> 16) << bitmap_log2w) + (u >> 16);
		mov	edx,%$u			; edx = u >> 16		; 13
		sar	edx,16						; 14
		mov	eax,%$u						; 14
		shl	ebx,cl						; 15/4
		test	[edi],al		; fetch dest into cache	; 15
		add	esi,edx			; esi = source texture	; 19
		add	esi,ebx						; 20

; uFrac = u << 16;
; vFrac = v << 16;
		mov	ebx,%$v						; 20
		shl	eax,16			; eax = uu << 16	; 21
		mov	ebp,%$dv					; 21
		shl	ebx,16			; ebx = vv << 16	; 22
; duFrac = du << 16;
; dvFrac = dv << 16;
		mov	ecx,%$du					; 22
		shl	ebp,16			; ebp = dvv << 16	; 23
		shl	ecx,16			; ecx = duu << 16	; 24
		mov	%$dvFP,ebp					; 25
		mov	%$duFP,ecx					; 25
		mov	ebp,%$zbuff					; 26
		mov	ecx,%$z						; 26

; --------------// Register usage //--------------
; EAX	u		ESI	source texture
; EBX	v		EDI	dest video ram
; ECX	z		EBP	Z-buffer
; EDX	--//scratch//--

%$sloop:	cmp	ecx,[ebp]		; Check Z-buffer	; 0
		jb	%$zbelow		; We're below surface	; 0
		xor	edx,edx			; clear texel buffer	; 1
		mov	[ebp],ecx		; *zbuff = z		; 1
		mov	ecx,%$paltable		; Get 8->16 table	; 2
		mov	dl,[esi]		; Get texel		; 2
		mov	dx,[ecx+edx*2]		; Convert to 16-bit	; 3/2
		mov	ecx,[ebp]		; z = *zbuff		; 3
		mov	[edi],dx		; Put texel		; 5

%$zbelow:	add	ebx,%$dvFP		; v = v + dv		; 6
		sbb	edx,edx						; 7
		add	eax,%$duFP		; u = u + du		; 7/2
		adc	esi,[%$dudvFP_+4+edx*4]	; Update texture ptr	; 9/2
		add	ecx,%$dz		; z = z + dz		; 9/2
		add	ebp,4			; zbuff++		; 11
		add	edi,2			; dest++		; 11
		cmp	edi,%$destend		; dest < destend?	; 12
		jb	%$sloop						; 12
endproc

;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
; Summary:
;   Draw a perspective-incorrect texture mapped polygon scanline using MMX
; Arguments:
;   void *dest, int width, unsigned long *zbuff,
;   long u, long du, long v, long dv, unsigned long z, long dz,
;   unsigned char *bitmap, int bitmap_log2w
;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
proc		csScan_16_mmx_scan_pi_tex_zuse,24,ebx,esi,edi,ebp
		targ	%$dest		; void *dest
		targ	%$width		; int width
		targ	%$zbuff		; long *zbuff
		targ	%$u		; long u
		targ	%$du		; long du
		targ	%$v		; long v
		targ	%$dv		; long dv
		targ	%$z		; long z
		targ	%$dz		; long dz
		targ	%$bitmap	; unsigned char *bitmap
		targ	%$bitmap_log2w	; int bitmap_log2w

		loc	%$dudvFP,8	; fixed-point value of (dv * tex_w + du)
		tloc	%$duFP		; fixed-point duu
		tloc	%$dvFP		; fixed-point dvv
		tloc	%$destend
		tloc	%$paltable	; 8->16 conversion table

; if (len <= 0)
;   return;
		mov	eax,%$width
		mov	edi,%$dest
		test	eax,eax
		exit	le

		lea	eax,[edi+eax*2]
		mov	ebp,PaletteTable
		mov	%$destend,eax
		mov	%$paltable,ebp

; dudvInt[1] = ((dv >> 16) << bitmap_log2w) + (du >> 16);
		mov	eax,%$dv		; eax = dv		; 0
		mov	edx,%$du		; edx = du		; 0
		sar	eax,16			; eax = dv >> 16	; 1
		mov	esi,%$bitmap		; esi = texture ptr	; 1
		sar	edx,16			; edx = du >> 16	; 2
		mov	ecx,%$bitmap_log2w	; ecx = bitmap_log2w	; 2
		shl	eax,cl			; eax = (dv >> 16) << s	; 3/4
; dudvInt[0] = dudvInt [1] + (1 << bitmap_log2w);
		mov	ebx,%$v			; ebx = v		; 3
		add	eax,edx			; eax = dudvInt[1]	; 7
		mov	edx,1			; edx = 1		; 7
		shl	edx,cl			; edx = 1 << bitmap_l2w	; 8/4
		mov	[%$dudvFP_+4],eax	; dudvInt[1] = eax	; 8
		sar	ebx,16			; ebx = v >> 16		; 12
		add	eax,edx			; eax = dudvInt[0]	; 12
		mov	[%$dudvFP_+0],eax	; dudvInt[0] = eax	; 13
; unsigned char *src = bitmap + ((v >> 16) << bitmap_log2w) + (u >> 16);
		mov	edx,%$u			; edx = u >> 16		; 13
		sar	edx,16						; 14
		mov	eax,%$u						; 14
		shl	ebx,cl						; 15/4
		test	[edi],al		; fetch dest into cache	; 15
		add	esi,edx			; esi = source texture	; 19
		add	esi,ebx						; 20

; uFrac = u << 16;
; vFrac = v << 16;
		mov	ebx,%$v						; 20
		shl	eax,16			; eax = uu << 16	; 21
		mov	ebp,%$dv					; 21
		shl	ebx,16			; ebx = vv << 16	; 22
; duFrac = du << 16;
; dvFrac = dv << 16;
		mov	ecx,%$du					; 22
		shl	ebp,16			; ebp = dvv << 16	; 23
		shl	ecx,16			; ecx = duu << 16	; 24
		mov	%$dvFP,ebp					; 25
		add	edi,8						; 25
		mov	%$duFP,ecx					; 26
		mov	ebp,%$zbuff					; 26
		mov	ecx,%$z						; 27

		test	ebp,7						; 27
		jz	%$zbufok					; 28

; Align Z-buffer on 8 bound
		cmp	ecx,[ebp]		; Check Z-buffer	; 0
		jb	%$zbelow1		; We're below surface	; 0
		mov	edx,%$paltable		; ecx = paltable	; 1
		mov	[ebp],ecx		; *zbuff = z		; 1
		xor	ecx,ecx			; ecx = 0		; 2
		mov	cl,[esi]		; Get texel		; 3
		mov	dx,[edx+ecx*2]		; 8->16			; 4/2
		mov	ecx,[ebp]		; z = *zbuff		; 6
		mov	[edi-8],dx		; Put texel		; 7

%$zbelow1:	add	ebx,%$dvFP		; v = v + dv		; 7/2
		sbb	edx,edx						; 9
		add	eax,%$duFP		; u = u + du		; 9
		adc	esi,[%$dudvFP_+4+edx*4]	; Update texture ptr	; 10
		add	ecx,%$dz		; z = z + dz		; 10
		add	edi,2			; dest++		; 11
		add	ebp,4			; zbuff++		; 11

%$zbufok:	cmp	edi,%$destend					;
		ja	near %$sbyone					;

; Load MMX registers
		movd	mm1,%$dz		; mm1 = 0|dz
		movd	mm0,ecx			; mm0 = 0|z0
		punpckldq mm1,mm1		; mm1 = dz|dz
		movq	mm2,mm0			; mm2 = 0|z0
		paddd	mm2,mm1			; mm2 = dz|z1
		punpckldq mm0,mm2		; mm0 = z1|z0
		pslld	mm1,1			; mm1 = dz*2|dz*2
		movq	mm5,mm0			; mm5 = z1|z0
		movq	mm4,mm0			; mm4 = z1|z0
		paddd	mm0,mm1			; mm0 = z3|z2

; EAX	u	  ESI	texture	  MM0	z1|z0	  MM4	/z1|z0
; EBX	v	  EDI	dest	  MM1	dz*2|dz*2 MM5	/m1|m0
; ECX	/texel	  EBP	/zbuff	  MM2	/	  MM6	/0|0|m1|m0
; EDX	/			  MM3	/bz1|bz0  MM7	/0|0|m3|m2

; WARNING: I've spent a whole day optimizing this routine so PLEASE
; be very careful if you think you can optimize something even more.
; The pairing is not perfect, but I doubt it can be better without
; changing the algorythm.

%$sloop4:	movq	mm3,[ebp]		; mm3 = bz1|bz0		; 0
		movq	mm7,mm0			; mm7 = z3|z2		; 0
		pcmpgtd	mm5,mm3			; mm5 = m1|m0		; 1
		xor	ecx,ecx			; ecx = 0		; 1
		movq	mm6,mm5			; mm6 = m1|m0		; 2
		pandn	mm5,mm3			; mm5 = ?bz1|?bz0	; 2
		pand	mm4,mm6			; mm4 = ?z1|?z0		; 3
		mov	cl,[esi]		; Get texel0		; 3
		movq	mm3,[ebp+8]		; mm3 = bz3|bz2		; 4
		por	mm5,mm4			; mm5 = nz1|nz0		; 4
		mov	edx,%$paltable		; edx = PaletteTable	; 5
		pcmpgtd	mm7,mm3			; mm7 = m3|m2		; 5
		movq	[ebp],mm5		; zbuff = nz1|nz0	; 6
		movq	mm4,mm0			; mm4 = z3|z2		; 6
		movq	mm5,mm7			; mm5 = m3|m2		; 7
		add	ebp,16			; zbuff++		; 7
		pandn	mm5,mm3			; mm5 = ?bz3|?bz2	; 8
		pand	mm4,mm7			; mm4 = ?z3|?z2		; 8
		mov	ecx,[edx+ecx*2]		; texel0 8->16		; 9
		mov	edx,%$dvFP		; edx = dv		; 9
		por	mm5,mm4			; mm5 = nz3|nz2		; 10
		paddd	mm0,mm1			; mm0 = z5|z4		; 10
		add	ebx,edx			; v = v + dv		; 11
		mov	%$zbuff,ebp		; update zbuff		; 11
		movq	[ebp-8],mm5		; zbuff = nz3|nz2	; 12
		packssdw mm6,mm6		; mm6 = 0|0|m1|m0	; 12
		sbb	edx,edx			; edx = carry (v + dv)	; 13
		mov	ebp,%$duFP		; ebp = du		; 13
		shl	ecx,16			; ecx = pix0|0		; 14
		add	eax,ebp			; u = u + du		; 14
		adc	esi,[%$dudvFP_+4+edx*4]	; Update texture	; 15/2
		add	ebx,%$dvFP		; v = v + dv		; 15/2
		mov	edx,0			; edx = 0		; 17
		mov	ebp,%$paltable		; edx = PaletteTable	; 17
		mov	dl,[esi]		; Get texel1		; 18
		packssdw mm7,mm7		; mm7 = 0|0|m3|m2	; 18
		mov	cx,[ebp+edx*2]		; texel1 8->16		; 19/2 (np?)
		sbb	edx,edx			; edx = carry (v + dv)	; 21
		mov	ebp,%$duFP		; ebp = du		; 21
		add	eax,ebp			; u = u + du		; 22
		movq	mm4,mm0			; mm4 = z5|z4		; 22
		adc	esi,[%$dudvFP_+4+edx*4]	; Update texture	; 23/2
		movq	mm5,mm0			; mm5 = z5|z4		; 23
		rol	ecx,16			; ecx = tex1|tex0	; 25 (np)
		movd	edx,mm6			; edx = m1|m0		; 26
		paddd	mm0,mm1			; mm0 = z7|z6		; 26
		mov	ebp,[edi-8]		; ebp = pix1|pix0	; 27
		and	ecx,edx			; ecx = ?tex1|?tex0	; 27
		not	edx			; edx = ~m1|~m0		; 28 (np)
		and	ebp,edx			; ebp = ?pix1|?pix0	; 29
		mov	edx,%$dvFP		; edx = dv		; 29
		or	ebp,ecx			; edx = t1?p1|t0?p0	; 30
		xor	ecx,ecx			; ecx = 0		; 30
		mov	[edi-8],ebp		; put pix1|pix0		; 31
		add	ebx,edx			; v += dv		; 31
		sbb	edx,edx			; edx = carry (v + dv)	; 32
		mov	cl,[esi]		; Get texel2		; 32
		add	eax,%$duFP		; u += du		; 33/2
		mov	ebp,%$paltable		; edx = PaletteTable	; 33
		adc	esi,[%$dudvFP_+4+edx*4]	; Update texture	; 35/2
		mov	edx,%$dvFP		; edx = dv		; 35
		mov	ecx,[ebp+ecx*2]		; texel2 8->16		; 37
		add	ebx,edx			; v += dv		; 37
		sbb	ebp,ebp			; ebp = carry (v + dv)	; 38
		xor	edx,edx			; edx = 0		; 38
		shl	ecx,16			; ecx = tex2|0		; 39
		mov	dl,[esi]		; Get texel3		; 39
		add	eax,%$duFP		; u += du		; 40/2
		lea	edi,[edi+8]		; Increment dest pointer; 40
		adc	esi,[%$dudvFP_+4+ebp*4]	; Update texture	; 42/2
		mov	ebp,%$paltable		; edx = PaletteTable	; 42
		mov	cx,[ebp+edx*2]		; ecx = tex3|tex2	; 44/2 (np?)
		movd	edx,mm7			; edx = m3|m2		; 46 (np)
		rol	ecx,16			; ecx = tex1|tex0	; 47 (np)
		mov	ebp,[edi-12]		; ebp = pix3|pix2	; 48
		and	ecx,edx			; ecx = ?tex3|?tex2	; 48
		not	edx			; edx = ~m3|~m2		; 49 (np)
		and	ebp,edx			; ebp = ?pix3|?pix2	; 50
		or	ecx,ebp			; ecx = t3?p3|t2?p2	; 51
		mov	ebp,%$zbuff		; restore ebp		; 51
		mov	[edi-12],ecx		; put pix3|pix2		; 52
		cmp	edi,%$destend		; edi < destend?	; 52/2
		jb	near %$sloop4					;

		movd	ecx,mm5

; Less than four pixels left
%$sbyone:	sub	edi,8
		cmp	edi,%$destend
		jae	%$sexit

%$sloop1:	cmp	ecx,[ebp]		; Check Z-buffer	; 0
		jb	%$zbelow		; We're below surface	; 0
		mov	edx,%$paltable		; ecx = paltable	; 1
		mov	[ebp],ecx		; *zbuff = z		; 1
		xor	ecx,ecx			; ecx = 0		; 2
		mov	cl,[esi]		; Get texel		; 3
		mov	dx,[edx+ecx*2]		; 8->16			; 4/2
		mov	ecx,[ebp]		; z = *zbuff		; 6
		mov	[edi],dx		; Put texel		; 7

%$zbelow:	add	ebx,%$dvFP		; v = v + dv		; 7
		sbb	edx,edx						; 8
		add	eax,%$duFP		; u = u + du		; 8/2
		adc	esi,[%$dudvFP_+4+edx*4]	; Update texture ptr	; 10/2
		add	ecx,%$dz		; z = z + dz		; 10/2
		add	ebp,4			; zbuff++		; 11
		add	edi,2			; dest++		; 11
		cmp	edi,%$destend		; dest < destend?	; 12
		jb	%$sloop1					; 12
%$sexit:	emms
endproc

;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
; Summary:
;   Draw a fog scanline when we're not inside fog for 5/5/5 modes
; Arguments:
;   int xx, unsigned char *d, unsigned long *z_buf,
;   float inv_z, float u_div_z, float v_div_z
;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
proc		csScan_16_555_scan_fog,36,ebx,esi,edi,ebp
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

		lea	eax,[edi+eax*2]
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
				mov	dx,[edi]	; get pixel	; 0
				mov	bl,[ecx+eax]			; 0
; r = (fd * ((*_dest & 0xf800) - Scan.FogR) >> 8) + Scan.FogR;
				mov	esi,edx		; p		; 1
				mov	eax,%$fogr	; r		; 1
				and	edx,0x7c00	; p & f800	; 2
				sub	edx,eax		; - r		; 3
				imul	edx,ebx		; * fd		; 4(9)
				mov	ecx,%$fogg	; g		; 4
				shr	edx,8		; >> 8		; 13
				add	eax,edx		; + r		; 14
				mov	edx,esi		; p		; 14
				and	eax,0x7c00	; r & f800	; 15
; g = (fd * ((*_dest & 0x07e0) - Scan.FogG) >> 8) + Scan.FogG;
				and	edx,0x03e0	; p & 7e0	; 15
				sub	edx,ecx		; - g		; 16
				and	esi,0x001f	; p & 1f	; 16
				imul	edx,ebx		; * fd		; 17(9)
				shr	edx,8		; >> 8		; 26
				add	ecx,edx		; + g		; 27
				mov	edx,%$fogb	; b		; 27
				and	ecx,0x03e0	; & 7e0		; 28
; b = (fd * ((*_dest & 0x001f) - Scan.FogB) >> 8) + Scan.FogB;
				sub	esi,edx		; - b		; 28
				or	eax,ecx		; r | g		; 29
				imul	esi,ebx		; * fd		; 29(9)
				shr	esi,8		; >> 8		; 38
				add	edx,esi		; + b		; 38
				mov	esi,%$izz	; restore izz	; 39
				or	edx,eax		; r | g | b	; 40
			endif
			mov    [edi],dx
		endif

		add	edi,2			; _dest++;
		mov	eax,%$dzz
		add	ebp,4			; z_buf++;
		add	esi,eax			; izz += dzz;
		cmp	edi,%$destend
		jb	near %$fogloop
%$fogexit:
endproc

;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
; Summary:
;   Draw a fog scanline when we're not inside fog for 5/6/5 modes
; Arguments:
;   int xx, unsigned char *d, unsigned long *z_buf,
;   float inv_z, float u_div_z, float v_div_z
;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
proc		csScan_16_565_scan_fog,36,ebx,esi,edi,ebp
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

		lea	eax,[edi+eax*2]
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
				mov	dx,[edi]	; get pixel	; 0
				mov	bl,[ecx+eax]			; 0
; r = (fd * ((*_dest & 0xf800) - Scan.FogR) >> 8) + Scan.FogR;
				mov	esi,edx		; p		; 1
				mov	eax,%$fogr	; r		; 1
				and	edx,0xf800	; p & f800	; 2
				sub	edx,eax		; - r		; 3
				imul	edx,ebx		; * fd		; 4(9)
				mov	ecx,%$fogg	; g		; 4
				shr	edx,8		; >> 8		; 13
				add	eax,edx		; + r		; 14
				mov	edx,esi		; p		; 14
				and	eax,0xf800	; r & f800	; 15
; g = (fd * ((*_dest & 0x07e0) - Scan.FogG) >> 8) + Scan.FogG;
				and	edx,0x07e0	; p & 7e0	; 15
				sub	edx,ecx		; - g		; 16
				and	esi,0x001f	; p & 1f	; 16
				imul	edx,ebx		; * fd		; 17(9)
				shr	edx,8		; >> 8		; 26
				add	ecx,edx		; + g		; 27
				mov	edx,%$fogb	; b		; 27
				and	ecx,0x07e0	; & 7e0		; 28
; b = (fd * ((*_dest & 0x001f) - Scan.FogB) >> 8) + Scan.FogB;
				sub	esi,edx		; - b		; 28
				or	eax,ecx		; r | g		; 29
				imul	esi,ebx		; * fd		; 29(9)
				shr	esi,8		; >> 8		; 38
				add	edx,esi		; + b		; 38
				mov	esi,%$izz	; restore izz	; 39
				or	edx,eax		; r | g | b	; 40
			endif
			mov    [edi],dx
		endif

		add	edi,2			; _dest++;
		mov	eax,%$dzz
		add	ebp,4			; z_buf++;
		add	esi,eax			; izz += dzz;
		cmp	edi,%$destend
		jb	near %$fogloop
%$fogexit:
endproc

;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
; Summary:
;   Draw a fog scanline when we're inside fog for 5/5/5 modes
; Arguments:
;   int xx, unsigned char *d, unsigned long *z_buf,
;   float inv_z, float u_div_z, float v_div_z
;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
proc		csScan_16_555_scan_fog_view,28,ebx,esi,edi,ebp
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

		lea	eax,[edi+eax*2]
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
				mov	dx,[edi]	; get pixel	; 0
				mov	bl,[ecx+eax]			; 0
; r = (fd * ((*_dest & 0xf800) - Scan.FogR) >> 8) + Scan.FogR;
				mov	esi,edx		; p		; 1
				mov	eax,%$fogr	; r		; 1
				and	edx,0x7c00	; p & f800	; 2
				sub	edx,eax		; - r		; 3
				imul	edx,ebx		; * fd		; 4(9)
				mov	ecx,%$fogg	; g		; 4
				shr	edx,8		; >> 8		; 13
				add	eax,edx		; + r		; 14
				mov	edx,esi		; p		; 14
				and	eax,0x7c00	; r & f800	; 15
; g = (fd * ((*_dest & 0x07e0) - Scan.FogG) >> 8) + Scan.FogG;
				and	edx,0x03e0	; p & 7e0	; 15
				sub	edx,ecx		; - g		; 16
				and	esi,0x001f	; p & 1f	; 16
				imul	edx,ebx		; * fd		; 17(9)
				shr	edx,8		; >> 8		; 26
				add	ecx,edx		; + g		; 27
				mov	edx,%$fogb	; b		; 27
				and	ecx,0x03e0	; & 7e0		; 28
; b = (fd * ((*_dest & 0x001f) - Scan.FogB) >> 8) + Scan.FogB;
				sub	esi,edx		; - b		; 28
				or	eax,ecx		; r | g		; 29
				imul	esi,ebx		; * fd		; 29(9)
				shr	esi,8		; >> 8		; 38
				add	edx,esi		; + b		; 38
				or	edx,eax		; r | g | b	; 39
			endif
			mov	[edi],dx
		endif

		add	edi,2			; _dest++;
		cmp	edi,%$destend
		jb	near %$fogloop
%$fogexit:
endproc

;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
; Summary:
;   Draw a fog scanline when we're inside fog for 5/5/5 modes
; Arguments:
;   int xx, unsigned char *d, unsigned long *z_buf,
;   float inv_z, float u_div_z, float v_div_z
;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
proc		csScan_16_565_scan_fog_view,28,ebx,esi,edi,ebp
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

		lea	eax,[edi+eax*2]
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
				mov	dx,[edi]	; get pixel	; 0
				mov	bl,[ecx+eax]			; 0
; r = (fd * ((*_dest & 0xf800) - Scan.FogR) >> 8) + Scan.FogR;
				mov	esi,edx		; p		; 1
				mov	eax,%$fogr	; r		; 1
				and	edx,0xf800	; p & f800	; 2
				sub	edx,eax		; - r		; 3
				imul	edx,ebx		; * fd		; 4(9)
				mov	ecx,%$fogg	; g		; 4
				shr	edx,8		; >> 8		; 13
				add	eax,edx		; + r		; 14
				mov	edx,esi		; p		; 14
				and	eax,0xf800	; r & f800	; 15
; g = (fd * ((*_dest & 0x07e0) - Scan.FogG) >> 8) + Scan.FogG;
				and	edx,0x07e0	; p & 7e0	; 15
				sub	edx,ecx		; - g		; 16
				and	esi,0x001f	; p & 1f	; 16
				imul	edx,ebx		; * fd		; 17(9)
				shr	edx,8		; >> 8		; 26
				add	ecx,edx		; + g		; 27
				mov	edx,%$fogb	; b		; 27
				and	ecx,0x07e0	; & 7e0		; 28
; b = (fd * ((*_dest & 0x001f) - Scan.FogB) >> 8) + Scan.FogB;
				sub	esi,edx		; - b		; 28
				or	eax,ecx		; r | g		; 29
				imul	esi,ebx		; * fd		; 29(9)
				shr	esi,8		; >> 8		; 38
				add	edx,esi		; + b		; 38
				or	edx,eax		; r | g | b	; 39
			endif
			mov	[edi],dx
		endif

		add	edi,2			; _dest++;
		cmp	edi,%$destend
		jb	near %$fogloop
%$fogexit:
endproc

scanproc 16,scan_map_zfil,SCANPROC_MAP,scanloop_map
scanproc 16,mmx_scan_map_zfil,SCANPROC_MAP|SCANPROC_MMX,mmx_scanloop_map
scanproc 16,scan_map_zuse,SCANPROC_MAP,scanloop_map_z
scanproc 16,scan_map_fixalpha50,SCANPROC_MAP,scanloop_map_a50
scanproc 16,scan_tex_zfil,SCANPROC_TEX,scanloop_tex
scanproc 16,mmx_scan_tex_zfil,SCANPROC_TEX|SCANPROC_MMX,mmx_scanloop_tex
