;--------=========xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx=========--------
;
;   Copyright (C) 1998 by Jorrit Tyberghein
;   Written by Andrew Zabolotny
;   Definitions for scanline drawing routines
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

%ifndef __SCAN_ASH__
%define __SCAN_ASH__

__TEXT_SECT__

; WARNING: Please use 8-space tabs while viewing/modifying this file -
; please don't convert tabs to spaces or to awful 2- or 4-space tabs
; some brain-dead editors uses.
;
; Several implementation notes:
;
; -*- WARNING: Keep in mind that NASM has a slightly different syntax
;     than both MASM/TASM & Co and GAS as well. Before doing any change
;     to this file please bother to RTFM at least a bit, please.
;
; -*- Due to the portable nature of Crystal Space we cannot use any absolute
;     references to variables, because OSes that impose restrictions on
;     dynamically-loaded code to be PIC (position-independent code)
;     requires addressing such variables through Global Offset Table;
;     This problem is resolved in the following way: GOT is handled using
;     macros which are empty for non-PIC platforms, but we should reserve
;     anyway the ebx register for GOT offset; the definitions in scan.ash
;     are suitable for use on both PIC and non-PIC platforms; i.e. if you
;     need the InterpolStep variable you just write
;
;		mov	eax,InterpolStep
;
;     note thet I don't use square brackets because InterpolStep will
;     expand to something like "dword [ebx+something wrt ..got]".
;
;     If you need ebx register (i.e. for a tight internal loop) you should
;     save/restore it on stack, and do not make any references to external
;     variables inside that loop; this will work okay on non-PIC platforms
;     but will go nuts on platforms with PIC.
;
; -*- On P5 and above FPU does multiplication as fast as addition, so
;     we use fmul without fear for slowdowns. Also it pairs as well as fadd.
;
; -*- Think twice before making any modifications to existing code. I've
;     carefully scheduled all the code with hand; its very easy to break it.
;     For example, you can see such a piece of code:
;
;		mov	edx,%$uu		; edx = u >> 16		; 13
;		sar	edx,16						; 14
;		mov	eax,%$uu					; 14
;
;     You can feel the functionally equivalent code
;
;		mov	edx,%$uu		; edx = u >> 16		; 13
;		mov	eax,edx						; 14
;		sar	edx,16						; 15
;
;     will execute faster (one memory access instead of two). However, this
;     is not true: SAR requires the U pipe; if we exchange instructions
;     2 and 3 we will simply lose a clock in V pipe: this means the code
;     will execute 2.5 clocks instead of 1.5. That is.

; Do (almost) bogus checking on texture overflows
%define STUPID_TEST

; Flag: scanproc uses light mapping
%define SCANPROC_MAP		0x00000001
; Flag: scanproc uses unlighted texture
%define SCANPROC_TEX		0x00000002
; Flag: scanproc uses MMX instructions (so FPU is clobbered)
%define SCANPROC_MMX		0x00000004

; Predefine several macros to make it maximally possible C-like;
; this makes easier porting the structure from scan.h

; Define a macro to define and access Scan structure fields
; In PIC environments we assume ebx already contains the
; absolute address of the Scan structure. This means we can
; use global variables only while ebx is unchanged.
%macro		__dword_field	1
.%1		resd	1
	%ifdef PIC
		%define %1 dword [ebx+csScanSetup.%1]
	%else
		%define	%1 dword [Scan+csScanSetup.%1]
	%endif
%endmacro
%define int				__dword_field
%define float				__dword_field
%define unsigned_long			__dword_field
%define csTextureHandleSoftware_P	__dword_field
%define unsigned_char_P			__dword_field
%define unsigned_int			__dword_field
%define unsigned_short_P		__dword_field
%define RGB8map_P			__dword_field
%define int_P				__dword_field
%define unsigned_int_P			__dword_field

; A coefficient for planar fog density: bigger is denser
%define PLANAR_FOG_DENSITY_COEF	6

; At this point csQround (255 * exp (-float (i) / 256.)) reaches zero
%define EXP_256_SIZE			1420
; Same for csQround (32 * exp (-float (i) / 256.))
%define EXP_32_SIZE			1065

struc csScanSetup
  ; Interpolation step for semi-perspective correct texture mapping
  int InterpolStep;
  ; Interpolation step (shift-value) for semi-perspective correct texture mapping
  int InterpolShift;
  ; Interpolation mode
  int InterpolMode;

  ; Fog color
  int FogR;
  int FogG;
  int FogB;
  ; The pixel value of fog (FogR|FogG|FogB for truecolor modes)
  int FogPix;
  ; Fog density
  unsigned_int FogDensity;

  ; A pointer to the texture.
  csTextureHandleSoftware_P Texture;

  ; The lighted texture bitmap from the texture cache.
  unsigned_char_P bitmap2;
  ; Width of the texture from the texture cache.
  int tw2;
  ; Height of the texture from the texture cache.
  int th2;
  ; Texture width in 16:16 fixed-point format - 1
  int tw2fp;
  ; Texture height in 16:16 fixed-point format - 1
  int th2fp;
  ; the difference of U-coordinates between cached texture and alpha map
  int amap_uofs;
  ; the difference of V-coordinates between cached texture and alpha map
  int amap_vofs;

  ; The unlighted texture bitmap.
  unsigned_char_P bitmap;
  ; Width of unlighted texture.
  int tw;
  ; Height of unlighted texture.
  int th;

  ;
  ; The following fields are used by the polygon drawer
  ; and contain information fot calculating the 1/z, u/z, and v/z linear
  ; equations.
  ;

  ; Difference value for every horizontal dx.
  float M;
  ; Difference value for every horizontal dx.
  float J1;
  ; Difference value for every horizontal dx.
  float K1;
  ; Difference for every 16 pixels.
  float dM;
  ; Difference for every 16 pixels.
  float dJ1;
  ; Difference for every 16 pixels.
  float dK1;

  ; Mean color value.
  int FlatColor;
  ; R/G/B components of flatcolor
  unsigned_int FlatRGB;

  ; Alpha mask for 16-bit renderer.
  unsigned_int AlphaMask;
  ; General alpha factor for 16-bit renderer (0 to 255).
  int AlphaFact;

  ; log2 (texture_u)
  int shf_u;

  ;
  ; The following fields are only used when drawing
  ; unlighted tiled textures (the scan_..._map_...
  ; routines don't need them).
  ;

  ; log2(texture_width)
  int shf_w;
  ; 1 << shf_w - 1
  int and_w;
  ; log2(texture_height)
  int shf_h;
  ; 1 << shf_h - 1
  int and_h;
  ; U at the origin of the texture
  int min_u;
  ; V at the origin of the texture
  int min_v;

  ; Actual texture palette
  unsigned_int_P TexturePalette;
  ; 8-bit to native pixel format conversion table
  unsigned_char_P PaletteTable;
  ; Set up by poly renderer to alpha blending table
  RGB8map_P AlphaMap;

  ; Current blending table
  unsigned_char_P BlendTable;

  ; 4096 1/z values where z is in fixed-point 0.12 format
  unsigned_int_P one_div_z;

  ; A table of exp(x) in the range 0..255; x == 0..EXP_256_SIZE
  unsigned_char_P exp_256;

  ; Blending tables
  unsigned_char_P BlendingTable_ADD
  unsigned_char_P BlendingTable_MULTIPLY
  unsigned_char_P BlendingTable_MULTIPLY2
  unsigned_char_P BlendingTable_ALPHA25
  unsigned_char_P BlendingTable_ALPHA50
  unsigned_char_P BlendingTable_ALPHA75

  ; Blending tables for proc. textures
  unsigned_char_P BlendingTableProc_ADD
  unsigned_char_P BlendingTableProc_MULTIPLY
  unsigned_char_P BlendingTableProc_MULTIPLY2
  unsigned_char_P BlendingTableProc_ALPHA25
  unsigned_char_P BlendingTableProc_ALPHA50
  unsigned_char_P BlendingTableProc_ALPHA75
endstruc

; The only external variable of csScanSetup type
extvar	Scan

%undef	int
%undef	float
%undef	unsigned_long
%undef	csTextureHandleSoftware_P
%undef	unsigned_char_P
%undef	unsigned_int
%undef	unsigned_short_P
%undef	RGB8map_P

;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
; Summary:
;   Check if uu & vv are out of texture bounds
; Arguments:
;   1 if scanproc uses texture mapping, 0 if not
;   the U variable
;   the V variable
; Example:
;   stupid_test	%%flags
; Modifies:
;   ebp
;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
%macro		stupid_test 3
	%ifdef STUPID_TEST
	%if %1 & SCANPROC_MAP
; if (uu < 0) uu = 0; else if (uu > tw2fp) uu = tw2fp;
		mov	ebp,%2
		test	ebp,ebp
		if	l,short
			xor	ebp,ebp
			mov	%2,ebp
		else	short
			cmp	ebp,tw2fp
			if	a,short
				mov	ebp,tw2fp
				mov	%2,ebp
			endif
		endif
; if (vv < 0) vv = 0; else if (vv > th2fp) vv = th2fp;
		mov	ebp,%3
		test	ebp,ebp
		if	l,short
			xor	ebp,ebp
			mov	%3,ebp
		else	short
			cmp	ebp,th2fp
			if	a,short
				mov	ebp,th2fp
				mov	%3,ebp
			endif
		endif
	%endif
	%endif
%endmacro

;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
; Summary:
;   This is the external scanline loop macro
; Arguments:
;   - bits per pixel (8/16/32)
;   - The name of scanline routine (without csScan_#_ prefix)
;   - texture flags ORed together
;	SCANPROC_MAP - if scanloop uses light map (vs unlighted textures)
;	SCANPROC_MMX - if zloop uses MMX instructions (and we don't need
;	to clear z and inv_z from fpu stack since we anyway need "emms")
;   - The basename of scanline routine
;	(appended suffixes: _args,_init,_body,_fini)
; Example:
;   scanproc 8,scan_map_zfill,SCANPROC_MAP,scanloop_map
; Comments:
;   The following variables are placed in registers (and
;   should not be changed by scanline loop macro):
;     EDI = dest (target video RAM location)
;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
%macro		scanproc 4
	; To avoid messing with %1, %2 etc we define aliases
	; for them here - once - and use further only aliases
		%define %%bpp		%1
		%define %%name		csScan_%{1}_%2
		%assign %%flags		%3

		%define %%scanloop_args	%4 %+ _args
		%define %%scanloop_init	%4 %+ _init
		%define %%scanloop_body	%4 %+ _body
		%define %%scanloop_fini	%4 %+ _fini

%if (%%flags & SCANPROC_MAP)
%define	%%args_size	52
%else
%define	%%args_size	60
%endif

proc		%%name,%%args_size+%%scanloop_args,ebx,esi,edi,ebp
		targ	%$width		; int
		targ	%$dest		; unsigned char *
		targ	%$zbuff		; unsigned long *
		targ	%$inv_z		; float
		targ	%$u_div_z	; float
		targ	%$v_div_z	; float

		tloc	%$const16	; float	2^16
		tloc	%$const24	; float	2^24
		tloc	%$uu		; int	u at beginning of scanline segment
		tloc	%$uu1		; int	u at end of scanline segment
		tloc	%$duu		; int	delta u on this segment
		tloc	%$vv		; int	v at beginning of scanline segment
		tloc	%$vv1		; int	v at end of scanline segment
		tloc	%$dvv		; int	delta v on this segment
		tloc	%$izz		; int	1/z at the beginning of scanline
		tloc	%$dzz		; int	delta z on this scanline
		tloc	%$scanwidth	; int	initial scanline width (not decremented)
		tloc	%$destend	; char*	the end of VRAM to be filled
		tloc	%$zbuffend	; long*	the end of Z-buffer to be filled
%if (%%flags & SCANPROC_MAP) == 0
		tloc	%$min_u		; u at texture origin
		tloc	%$min_v		; v at texture origin
%endif

; if (width <= 0) return;
		cmp	%$width,0
		exit	le

; The FPU instructions have the FPU stack in the comment to the right
; in the state AFTER the instruction; the second comment, if present,
; gives the relative clock number BEFORE instruction starts
					; st0, st1, ...	; clock/duration (pipe)
; float z = 1 / inv_z;
		fld	%$inv_z		; inv_z				; 0/1
		fld1			; 1, inv_z			; 1/2
		fdiv	st1		; z, inv_z			; 3/39

; the previous fdiv takes 39 clocks, in the meantime do some integer math
	%ifdef PIC
		GetGOT			; Get Global Offset Table offset in ebx
		mov	ebx,Scan	; Now ebx points to csScanSetup structure
	%endif

		%%scanloop_init

%if (%%flags & SCANPROC_MAP) == 0
		mov	eax,min_u
		mov	ecx,min_v
		mov	%$min_u,eax
		mov	%$min_v,ecx
%endif

; const16 = 65536; const24 = 16777216;
		mov	eax,0x47800000	; 65536F
		mov	ecx,0x4B800000	; 16777216F
		mov	%$const16,eax
		mov	%$const24,ecx
; unsigned long *lastZbuf = z_buffer + width -1;
		mov	ecx,%$width
		mov	eax,%$zbuff
		mov	%$scanwidth,ecx
		lea	eax,[eax+ecx*4]
; unsigned char *_dest = d;
		mov	edi,%$dest
		mov	%$zbuffend,eax
                mov	eax,InterpolStep
		cmp	ecx,eax

; float u = u_div_z * z;
; float v = v_div_z * z;
		fld	st0		; z, z, inv_z			; 0/1
		fmul	%$u_div_z	; u, z, inv_z			; 1/3
		fxch			; z, u, inv_z			; 1
		fmul	%$v_div_z	; v, u, inv_z			; 2/3

; int uu = csQfixed16 (u);
; int vv = csQfixed16 (v);
; long izz = csQfixed24 (inv_z);
; dzz = csQfixed24 (M);
		fld	st2		; inv_z, v, u, inv_z		; 3/1
		fmul	%$const24	; izz, v, u, inv_z		; 4/3
		fxch	st2		; u, v, izz, inv_z		; 4
		fmul	%$const16	; uu, v, izz, inv_z		; 5/3
		fxch			; v, uu, izz, inv_z		; 5
		fmul	%$const16	; vv, uu, izz, inv_z		; 6/3
		fxch	st2		; izz, uu, vv, inv_z		; 6
		fistp	%$izz		; uu, vv, inv_z			; 7/6
		fld	M		; M, uu, vv, inv_z		; 8/1
		fxch			; uu, M, vv, inv_z		; 8
		fistp	%$uu		; M, vv, inv_z			; 9/6
		fmul	%$const24	; dzz, vv, inv_z		; 10/3
		fxch			; vv, dzz, inv_z		; 10
		fistp	%$vv		; dzz, inv_z			; 11/6
		fistp	%$dzz		; inv_z				; 12/6

; if (width >= INTERPOL_STEP)
;   inv_z += dM;
; else
;   inv_z += M * width;
		if	ae,short
			fld	dM	; dM, inv_z
		else	short
			fld	M	; M, inv_z
			fimul	%$width	; M*width, inv_z
		endif
		faddp	st1		; inv_z + dM

; z1 = 1 / inv_z;
		fld1			; 1, inv_z
		fdiv	st1		; z1=1/inv_z, inv_z

;#if STUPID_TEST && SCANMAP
		stupid_test %%flags,%$uu,%$vv
; do
; {
%$loop:
;   if (width > INTERPOL_STEP)
;   {
		mov	eax,%$width
		cmp	eax,InterpolStep
		if	ae
;     u_div_z += dJ1;
;     v_div_z += dK1;
			fld	dJ1	; dJ1, z1, inv_z		; 0/1
			fld	dK1	; dK1, dJ1, z1, inv_z		; 1/1
			fxch		; dJ1, dK1, z1, inv_z		; 1
			fadd	%$u_div_z;u_div_z, dK1, z1, inv_z	; 2/3
			fxch		; dK1, u_div_z, z1, inv_z	; 2
			fadd	%$v_div_z;v_div_z, u_div_z, z1, inv_z	; 3/3

;     width -= INTERPOL_STEP;
			mov	ecx,InterpolStep			; 4
			sub	%$width,ecx				; 5/3

			fst	%$v_div_z;v_div_z, u_div_z, z1, inv_z	; 6/3
			fxch		; u_div_z, v_div_z, z1, inv_z	; 6
			fst	%$u_div_z;u_div_z, v_div_z, z1, inv_z	; 7/3
;     u1 = u_div_z * z1;
;     v1 = v_div_z * z1;
			fmul	st2	; u1, v_div_z, z1, inv_z	; 8/3
			fxch		; v_div_z, u1, z1, inv_z	; 8
			fmulp	st2	; u1, v1, inv_z			; 9/3

;     _destend = _dest + INTERPOL_STEP - 1;
			lea	eax,[edi+ecx*(%%bpp/8)]			; 10
			mov	%$destend,eax				; 11

;     uu1 = csQfixed16 (u1);
;     vv1 = csQfixed16 (v1);
			fmul	%$const16; uu1, v1, inv_z		; 12/3
			fxch		; v1, uu1, inv_z		; 12
			fmul	%$const16;vv1, uu1, inv_z		; 13/3
			fxch		; uu1, vv1, inv_z		; 13 (stall)
			mov	eax,%$width				; 14
			fistp	%$uu1	; vv1, inv_z			; 15/6
			fistp	%$vv1	; inv_z				; 16/6
			wait

;#    if STUPID_TEST && SCANMAP
			stupid_test %%flags,%$uu1,%$vv1

;     if (width >= INTERPOL_STEP)
;       inv_z += dM;
;     else
;       inv_z += M * width;
			cmp	eax,ecx
			if	ae,short
				fld	dM	; dM, inv_z
			else	short
				fld	M	; M, inv_z
				fimul	%$width	; M*width, inv_z
			endif
			mov	ecx,InterpolShift
			faddp	st1	; inv_z
;     duu = (uu1 - uu) >> INTERPOL_SHFT;
;     dvv = (vv1 - vv) >> INTERPOL_SHFT;
			mov	eax,%$uu1				; 0
			mov	edx,%$vv1				; 0
			sub	eax,%$uu				; 1
			sub	edx,%$vv				; 1
			sar	eax,cl					; 2(4)
			sar	edx,cl					; 6(4)
			mov	%$duu,eax				; 10
			mov	%$dvv,edx				; 10
;   z1 = 1 / inv_z;
			fld1		; 1, inv_z			; 11/2
			fdiv	st1	; z1, inv_z			; 13/39
;   }
;   else
;   {
		else
;     u_div_z += J1 * width;
;     v_div_z += K1 * width;
;     _destend = _dest + width - 1;
			fild	%$width	; width, z1, inv_z		; 0
			fld	J1	; J1, width, z1, inv_z		; 1/1
			fld	K1	; K1, J1, width, z1, inv_z	; 2/1
			fmul	st2	; K1*width, J1, width, z1, inv_z; 3/3
                        fxch		; J1, K1*width, width, z1, inv_z; 3
			fmulp	st2	; K1*width, J1*width, z1, inv_z	; 4/3
			fxch		; J1*width, K1*width, z1, inv_z	; 4 (stall)
			mov	ecx,%$width				; 5
			fadd	%$u_div_z;u_div_z, K1*width, z1, inv_z	; 6/3
			fxch		; K1*width, u_div_z, z1, inv_z	; 6
			fadd	%$v_div_z;v_div_z, u_div_z, z1, inv_z	; 7/3
			fxch		; u_div_z, v_div_z, z1, inv_z	; 7 (stall)
			lea	eax,[edi+ecx*(%%bpp/8)]			; 8
			fst	%$u_div_z;u_div_z, v_div_z, z1, inv_z	; 9/2
			fxch		; v_div_z, u_div_z, z1, inv_z	; 9
			fst	%$v_div_z;v_div_z, u_div_z, z1, inv_z	; 10/2
			fxch		; u_div_z, v_div_z, z1, inv_z	; 10
;     u1 = u_div_z * z1;
;     v1 = v_div_z * z1;
			fmul	st2	; u1, v_div_z, z1, inv_z	; 11/3
			fxch		; v_div_z, u1, z1, inv_z	; 11
			fmulp	st2	; u1, v1, inv_z			; 12 (stall)
			mov	%$destend,eax				; 13
;     uu1 = csQfixed16 (u1);
;     vv1 = csQfixed16 (v1);
			fmul	%$const16;uu1, v1, inv_z		; 14/3
			fxch		; v1, uu1, inv_z		; 14
			fmul	%$const16;vv1, uu1, inv_z		; 15/3
			fxch		; uu1, vv1, inv_z		; 15 (stall)
			fistp	%$uu1	; vv1, inv_z			; 17/6
			fistp	%$vv1	; inv_z				; 18/6
			wait
;#if STUPID_TEST && SCANMAP
			stupid_test %%flags,%$uu1,%$vv1
;     duu = (uu1 - uu) / width;
;     dvv = (vv1 - vv) / width;
;     width = 0;
			fild	%$width	; width, inv_z			; 0/3
			fild	%$vv1	; vv1, width, inv_z		; 1/3
			mov	eax,%$uu1				; 2
			sub	eax,%$uu				; 3
			fisub	%$vv	; vv1-vv, width, inv_z		; 4/6
			cdq						; 6/2
			idiv	ecx					; 8/46
			fdivrp	st1	; dvv, inv_z			; 9/39
			mov	%$width,0				; 10
		; 38 clocks stall: two divisions at once
			fist	%$dvv	; dvv, inv_z
; Note that we don't do "fistp" above because we have to leave something
; on FPU stack in place where usually "z" resides. It does not really matter
; what we have there on last pass, just to keep two "fstp"s below happy.
			mov	%$duu,eax
;   }
		endif

%if (%%flags & SCANPROC_MAP) == 0
		mov	eax,%$min_u
		mov	ecx,%$min_v
		add	%$uu,eax
		add	%$vv,ecx
%endif

		%%scanloop_body

; uu = uu1; vv = vv1;
		mov	eax,%$uu1					; 0
		mov	ecx,%$vv1					; 0
		mov	%$uu,eax					; 1
		mov	%$vv,ecx					; 1
; }
; while (width);
		mov	eax,%$width					; 2
		test	eax,eax						; 3
		jne	near %$loop					; 3

; pop z and inv_z from the FPU stack; mmx %%scanloop_fini routines do emms
	%if (%%flags & SCANPROC_MMX) == 0
		fstp	st0
		fstp	st0
	%endif

		%%scanloop_fini
endproc
%endmacro

;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
; Summary:
;   Common routine for filling Z buffer with values
; Arguments:
;   none
; Example:
;   %define scanloop_map_fini zfill
;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
%macro		zfill 0
		mov	ecx,%$scanwidth		; number of Z values	; 0
		mov	eax,%$izz		; eax = z1		; 0
		mov	ebx,%$dzz		; ebx = dz		; 1
		mov	edi,%$zbuff					; 1
		sub	ecx,8						; 2
		jb	%$zbyone					; 3

		lea	edx,[eax+ebx]		; edx = z1 + dz		; 4
		shl	ebx,1			; ebx = dz * 2		; 4

%$zloop8:	mov	[edi+4*0],eax					; 0
		add	eax,ebx						; 0
		mov	[edi+4*1],edx					; 1
		add	edx,ebx						; 1

		mov	[edi+4*2],eax					; 2
		add	eax,ebx						; 2
		mov	[edi+4*3],edx					; 3
		add	edx,ebx						; 3

		mov	[edi+4*4],eax					; 4
		add	eax,ebx						; 4
		mov	[edi+4*5],edx					; 5
		add	edx,ebx						; 5

		mov	[edi+4*6],eax					; 6
		add	eax,ebx						; 6
		mov	[edi+4*7],edx					; 7
		add	edx,ebx						; 7

		add	edi,8*4						; 8
		sub	ecx,8						; 8
		jnb	%$zloop8					; 9
		sar	ebx,1			; ebx = dz

%$zbyone:	add	ecx,8
		jz	%$zexit
;    do
%$zloop1:
;    {
;      *z_buffer++ = izz;
		mov	[edi],eax					; 0
		add	edi,4						; 0
;      izz += dzz;
		add	eax,ebx						; 1
;    }
;    while (z_buffer <= lastZbuf)
		dec	ecx						; 1
		jnz	%$zloop1					; 2
%$zexit:
%endmacro

;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
; Summary:
;   The routine for filling Z buffer with values using MMX instructions
; Arguments:
;   none
; Example:
;   %define mmx_scanloop_map_fini mmx_zfill
;-----======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx======-----
%macro		mmx_zfill 0
		mov	ecx,%$scanwidth
		mov	edi,%$zbuff
		cmp	ecx,16
		mov	eax,%$izz
		mov	ebx,%$dzz
		jb	near %$zloop1

		movd	mm2,ebx			; mm2 =	0 | dz
		movd	mm0,eax			; mm0 =	0 | z0
		punpckldq mm2,mm2		; mm2 =	dz | dz
		movq	mm1,mm0			; mm1 =	0 | z0
		paddd	mm1,mm2			; mm1 =	dz | z1
		punpckldq mm0,mm1		; mm0 =	z1 | z0
		movq	mm1,mm0			; mm1 =	z1 | z0
		pslld	mm2,1			; mm2 =	dz*2 | dz*2
		paddd	mm1,mm2			; mm1 =	z3 | z2
		pslld	mm2,1			; mm2 =	dz*4 | dz*4

%$zloop16:	movq	[edi+8*0],mm0
		paddd	mm0,mm2
		movq	[edi+8*1],mm1
		paddd	mm1,mm2
		movq	[edi+8*2],mm0
		paddd	mm0,mm2
		movq	[edi+8*3],mm1
		paddd	mm1,mm2
		sub	ecx,16			; decrement counter
		movq	[edi+8*4],mm0
		paddd	mm0,mm2
		movq	[edi+8*5],mm1
		paddd	mm1,mm2
		movq	[edi+8*6],mm0
		paddd	mm0,mm2
		movq	[edi+8*7],mm1
		paddd	mm1,mm2
		add	edi,4*16		; increment bufptr
		cmp	ecx,16
		jae	%$zloop16

		sub	ecx,2
		jb	%$zless2
		movq	[edi],mm0
		add	edi,8
		sub	ecx,2
		jb	%$zless2
		movq	[edi],mm1
		add	edi,8
		sub	ecx,2
%$zless2:	add	ecx,2
		mov	eax,[edi-4]
		test	ecx,ecx
		jz	%$zexit

%$zloop1:	add	eax,ebx
		mov	[edi],eax
		add	edi,4
		dec	ecx
		jnz	%$zloop1
%$zexit:	emms
%endmacro

%endif ; __SCAN_ASH__
