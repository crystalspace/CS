%include "cs.ash"
	;; its not scheduled yet, let me get it working first
	;;
	;; clk symbols:
	;; xx(cause) .. instruction scheduled at clock xx
	;;	     .. if it could not be paired then because of the (cause)
	;; causes:	u  .. instruction executes in u-pipe only
	;;		ww .. write/write clash
	;;		wr .. write/read clash
	;;		x  .. special rule which does not permit pairing
__TEXT_SECT__

	proc yuv2rgba_mmx
	arg %$ybuffer		; luminance values
	arg %$ubuffer		; chrominace Cb
	arg %$vbuffer		; chrominace Cr
	arg %$outbuffer		; target outbuffer
	arg %$width		; width of the source picture
	arg %$height		; height of the source picture

	push ebp		;
	mov eax, %$outbuffer	; hold even lines in outbuffer
 	mov edx, %$ybuffer	;
	mov edi, %$ubuffer	;
	mov esi, %$vbuffer	;
	mov ebp, %$width	;

	sub edi,4		;
	sub esi,4		;
	sub edx,8		;
	sub eax,32		;
newline:
	mov ecx, ebp		;
	shr ecx, 3		; we write 8 pixel at once

lineloop:
	add edi,4		;					0
	add esi,4		;					0
	;; prepare u&v
	movd mm2, [edi]		; read 4 u values			1
	pxor mm7, mm7		;					1
	movd mm3, [esi]		; read 4 v values			2
	punpcklbw mm2, mm7	; make the 4 u byte values 4 words	2
	psubw mm2, [UV_SUB]	; u -= 128				3
	punpcklbw mm3, mm7	; make the 4 v byte values 4 words	3
	psubw mm3, [UV_SUB]	; v -= 128				4
	psllw mm2, 3		; u *= 8				4

	add edx,8		;
	;; prepare y
	movq mm1, [CLR_EVN]	;					5
	movq mm4, mm2		;					5
	movq mm0, [edx]		; first 8 y values			6
	psllw mm3, 3		; v *= 8				6
	psubusb mm0, [Y_SUB]	; y -= 16				7
	movq mm5, mm3		;					7
	pand mm1, mm0		; mm1 = ..y6..y4..y2..y0		8
	add eax,32		;					8
	pmulhw mm4, [U1_FACT]	; mm4 = ..u3..u2..u1..u0 (Cb)		9-11
	psrlw mm0, 8		; mm0 = ..y7..y5..y3..y1		10
	psllw mm1, 3		; y *= 8				11
	pmulhw mm1, [Y_FACT]	; y *= luminance_factor / 65535		12-14(u)
	psllw mm0, 3		; y *= 8				13
	pmulhw mm0, [Y_FACT]	; y *= luminance_factor / 65535		15-17(u,ww)

	;; make blue
	movq mm7, mm1		; even y				16
	paddsw mm7, mm4		; mm7 = ..b6..b4..b2..b0		18
	paddsw mm4, mm0		; mm4 = ..b7..b5..b3..b1		18
	packuswb mm7, mm7	; mm7 = b6b4b2b0b6b4b2b0		19
	movq mm6, mm1		; even y				19
	packuswb mm4, mm4	; mm4 = b7b5b3b1b7b5b3b1		20
	punpcklbw mm7, mm4	; mm7 = b7b6b5b4b3b2b1b0		21(x,wr)

	;; make red
	movq mm4, mm3		;					23
	pmulhw mm4, [V1_FACT]	; mm4 = ..v3..v2..v1..v0 (Cr)		24-26
	paddsw mm6, mm4		; mm6 = ..r6..r4..r2..r0		27(wr)
	paddsw mm4, mm0		; mm4 = ..r7..r5..r3..r1		27
	packuswb mm6, mm6	; mm6 = r6r4r2r0r6r4r2r0		28
	packuswb mm4, mm4	; mm4 = r7r5r3r1r7r5r3r1		29(x)
	punpcklbw mm6, mm4	; mm6 = r7r6r5r4r3r2r1r0		30(x,wr)

	;; make green
	movq mm4, mm2		;					30
	pmulhw mm4, [U2_FACT]	;					31-33(u)
	pmulhw mm5, [V2_FACT]	;					34-36(u)
	paddsw mm1, mm4		;					35
	paddsw mm0, mm4		;					36
	paddsw mm1, mm5		;					37
	paddsw mm0, mm5		;					37
	packuswb mm1, mm1	;					38
	movq mm4, mm6		;					38
	packuswb mm0, mm0	;					39
	punpcklbw mm1, mm0	; mm1 = g7g6g5g4g3g2g1g0		40(x,wr)

	;; mm7 = blue, mm6 = red, mm1 = green
	;; now we have to interleave the values to write them out at once
	;; goal is this form: rgb.rgb.

	punpcklbw mm4, mm1	; mm4 = g3r3g2r2g1r1g0r0		41
	movq mm0, mm7		;					41
	punpckhbw mm6, mm1	; mm6 = g7r7g6r6g5r5g4r4		42
	pxor mm1, mm1		;					42
	punpcklbw mm7, mm1	; mm7 = ..b3..b2..b1..b0		43
	punpckhbw mm0, mm1	; mm0 = ..b7..b6..b5..b4		44
	movq mm1, mm4		;					44
	punpcklwd mm4, mm7	; mm4 = ..b1g1r1..b0g0r0		45
	punpckhwd mm1, mm7	; mm1 = ..b3g3r3..b2g2r2		46
	movq mm7, mm6		;					46
	punpcklwd mm6, mm0	; mm6 = ..b5g5r5..b4g4r4		47
	punpckhwd mm7, mm0	; mm7 = ..b7g7r7..b6g6r6		48(x)

	movq [eax], mm4		;					47(u)
	movq [eax+8], mm1	;					48(u)
	movq [eax+16], mm6	;					49(u)
	movq [eax+24], mm7	;					50(u)

	;; now process the odd line in the output buffer
	;; we still have the u,v value in mm2 and mm3 for this

	;; prepare y
	movq mm0, [edx+ebp]	; first 8 y values
	psubusb mm0, [Y_SUB]	; y -= 16
	movq mm1, [CLR_EVN]
	pand mm1, mm0		; mm1 = ..y6..y4..y2..y0
	psllw mm1, 3		; y *= 8
	psrlw mm0, 8		; mm0 = ..y7..y5..y3..y1
	psllw mm0, 3		; y *= 8
	pmulhw mm0, [Y_FACT]	; y *= luminance_factor / 65535
	pmulhw mm1, [Y_FACT]	; y *= luminance_factor / 65535

	;; make blue
	movq mm7, mm1		; even y
	movq mm4, mm2		;
	pmulhw mm4, [U1_FACT]	; mm4 = ..u3..u2..u1..u0 (Cb)
	paddsw mm7, mm4		; mm7 = ..b6..b4..b2..b0
	paddsw mm4, mm0		; mm4 = ..b7..b5..b3..b1
	packuswb mm7, mm7	; mm7 = b6b4b2b0b6b4b2b0
	packuswb mm4, mm4	; mm4 = b7b5b3b1b7b5b3b1
	punpcklbw mm7, mm4	; mm7 = b7b6b5b4b3b2b1b0

	;; make red
	movq mm6, mm1		; even y
	movq mm4, mm3		;
	pmulhw mm4, [V1_FACT]	; mm4 = ..v3..v2..v1..v0 (Cr)
	paddsw mm6, mm4		; mm6 = ..r6..r4..r2..r0
	paddsw mm4, mm0		; mm4 = ..r7..r5..r3..r1
	packuswb mm6, mm6	; mm6 = r6r4r2r0r6r4r2r0
	packuswb mm4, mm4	; mm4 = r7r5r3r1r7r5r3r1
	punpcklbw mm6, mm4	; mm6 = r7r6r5r4r3r2r1r0

	;; make green
	movq mm4, mm2
	movq mm5, mm3
	pmulhw mm4, [U2_FACT]
	pmulhw mm5, [V2_FACT]
	paddsw mm1, mm4		;
	paddsw mm1, mm5		;
	packuswb mm1, mm1
	paddsw mm0, mm4		;
	paddsw mm0, mm5		;
	packuswb mm0, mm0
	punpcklbw mm1, mm0	; mm1 = g7g6g5g4g3g2g1g0

	;; mm7 = blue, mm6 = red, mm1 = green
	;; now we have to interleave the values to write them out at once
	;; goal is this form: rgb.rgb.

	movq mm4, mm6
	punpcklbw mm4, mm1	; mm4 = g3r3g2r2g1r1g0r0
	punpckhbw mm6, mm1	; mm6 = g7r7g6r6g5r5g4r4
	pxor mm1, mm1
	movq mm0, mm7
	punpcklbw mm7, mm1	; mm7 = ..b3..b2..b1..b0
	punpckhbw mm0, mm1	; mm0 = ..b7..b6..b5..b4
	movq mm1, mm4
	punpcklwd mm4, mm7	; mm4 = ..b1g1r1..b0g0r0
	punpckhwd mm1, mm7	; mm1 = ..b3g3r3..b2g2r2
	movq mm7, mm6
	punpcklwd mm6, mm0	; mm6 = ..b5g5r5..b4g4r4
	punpckhwd mm7, mm0	; mm7 = ..b7g7r7..b6g6r6

	movq [eax+4*ebp], mm4
	movq [eax+4*ebp+8], mm1
	movq [eax+4*ebp+16], mm6
	movq [eax+4*ebp+24], mm7

	dec ecx
	jnz NEAR lineloop
	lea eax, [eax+4*ebp]		; skip next line
	lea edx, [edx+ebp]		; skip next line
	DEC DWORD %$height
	DEC DWORD %$height
	jnz NEAR newline
	emms

	pop ebp
	endproc yuv2rgba_mmx

__DATA_SECT__

Y_FACT	DW 0x253f, 0x253f, 0x253f, 0x253f
V1_FACT DW 0x3312, 0x3312, 0x3312, 0x3312
U1_FACT DW 0x4093, 0x4093, 0x4093, 0x4093
U2_FACT DW 0xe5fc, 0xe5fc, 0xe5fc, 0xe5fc
V2_FACT DW 0xf37d, 0xf37d, 0xf37d, 0xf37d
Y_SUB   DW 0x1010, 0x1010, 0x1010, 0x1010
UV_SUB  DW 0x0080, 0x0080, 0x0080, 0x0080
CLR_EVN DW 0x00ff, 0x00ff, 0x00ff, 0x00ff
