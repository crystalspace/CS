%include "cs.ash"

__TEXT_SECT__
	
	proc yuv2rgbq_mmx
	arg %$ybuffer		; luminance values
	arg %$ubuffer		; chrominace Cb
	arg %$vbuffer		; chrominace Cr
	arg %$outbuffer		; target outbuffer
	arg %$width		; width of the source picture
	arg %$height		; height of the source picture

	mov eax, %$outbuffer	; hold even outlines
	mov edx, %$ybuffer
	mov edi, %$ubuffer
	mov esi, %$vbuffer

	pxor mm7, mm7		; zero out
	movq mm0, [edx]		; first 8 y values
	movd mm2, [edi]
	movd mm3, [esi]
	punpcklbw mm2, mm7	; make the 4 u byte values 4 words
	punpcklbw mm3, mm7	; make the 4 v byte values 4 words
	psubusb mm0, [Y_SUB]	; y -= 16
	movq mm1, [CLR_EVN]
	pand mm1, mm0		; mm1 = ..y6..y4..y2..y0
	psllw mm1, 3		; y *= 8
	psrlw mm0, 8		; mm0 = ..y7..y5..y3..y1
	psllw mm0, 3		; y *= 8
	psubw mm2, [UV_SUB]	; u -= 128
	psllw mm2, 3		; u *= 8
	psubw mm3, [UV_SUB]	; v -= 128
	psllw mm3, 3		; v *= 8
	pmulhw mm0, [Y_FACT]	; y *= luminance_factor / 65535
	pmulhw mm1, [Y_FACT]	; y *= luminance_factor / 65535
	
	movq mm7, mm1		; even y
	movq mm4, mm2		;
	pmulhw mm4, [U1_FACT]	; mm4 = ..u3..u2..u1..u0 (Cb)
	paddsw mm7, mm4		; mm7 = ..b6..b4..b2..b0
	paddsw mm4, mm0		; mm4 = ..b7..b5..b3..b1
	packuswb mm7, mm7	; mm7 = b6b4b2b0b6b4b2b0
	packuswb mm4, mm4	; mm4 = b7b5b3b1b7b5b3b1
	punpcklbw mm7, mm4	; mm7 = b7b6b5b4b3b2b1b0

	movq mm6, mm1		; even y
	movq mm4, mm3		;
	pmulhw mm4, [V1_FACT]	; mm4 = ..v3..v2..v1..v0 (Cr)
	paddsw mm6, mm4		; mm6 = ..r6..r4..r2..r0
	paddsw mm4, mm0		; mm4 = ..r7..r5..r3..r1
	packuswb mm6, mm6	; mm6 = r6r4r2r0r6r4r2r0
	packuswb mm4, mm4	; mm4 = r7r5r3r1r7r5r3r1
	punpcklbw mm6, mm4	; mm6 = r7r6r5r4r3r2r1r0
	
	movq mm5, mm1		; even y
	movq mm4, mm0		; even y
	pmulhw mm2, [U2_FACT]
	pmulhw mm3, [V2_FACT]
	paddsw mm5, mm2		; 
	paddsw mm5, mm3		;
	packuswb mm5, mm5
	paddsw mm4, mm2		; 
	paddsw mm4, mm3		;
	packuswb mm4, mm4
	punpcklbw mm5, mm4	; mm5 = g7g6g5g4g3g2g1g0
	
	;; mm7 = blue, mm6 = red, mm5 = green
	;; now we have to interleave the values to write them out at once
	;; goal is this form: rgb.rgb.

	movq mm4, mm5
	punpcklbw mm4, mm6	; mm4 = r3g3r2g2r1g1r0g0
	punpckhbw mm5, mm6	; mm5 = r7g7r6g6r5g5r4g4
	pxor mm6, mm6
	pxor mm0, mm0
	punpcklbw mm6, mm7	; mm6 = b3..b2..b1..b0..
	punpckhbw mm7, mm0	; mm7 = b7..b6..b5..b4..
	movq mm0, mm6
	punpcklwd mm0, mm4	; mm0 = r1g1b1..r0g0b0..
	punpckhwd mm6, mm4	; mm6 = r3g3b3..r2g2b2..
	movq mm1, mm7
	punpcklwd mm1, mm5	; mm1 = r5g5b5..r4g4b4..
	punpckhwd mm7, mm5	; mm7 = r7g7b7..r6g6b6..
	endproc yuv2rgbq_mmx

__DATA_SECT__
	
Y_FACT	DW 0x253f, 0x253f, 0x253f, 0x253f
V1_FACT DW 0x3312, 0x3312, 0x3312, 0x3312
U1_FACT DW 0x4093, 0x4093, 0x4093, 0x4093
U2_FACT DW 0xe5fc, 0xe5fc, 0xe5fc, 0xe5fc
V2_FACT DW 0xf37d, 0xf37d, 0xf37d, 0xf37d
Y_SUB   DW 0x1010, 0x1010, 0x1010, 0x1010
UV_SUB  DW 0x0080, 0x0080, 0x0080, 0x0080
CLR_EVN DW 0x00ff, 0x00ff, 0x00ff, 0x00ff		
