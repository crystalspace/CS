//
//   scan_m68k.S
//
//   Made by Aki Laukkanen (amlaukka@cc.helsinki.fi)
//
//   TODO
//   * combine the two routines
//   * make comments
//

#ifdef ELF
#define C(label) label
#else
#define C(label) _##label
#endif
 
//extern void REGARGS draw_scanline_map_m68k(REG(d0,int width),
//                                           REG(d1,int texture_width),
//                                           REG(d2,int texture_height),
//                                           REG(d3,int shift_v),
//                                           REG(a0,unsigned char *chunky),
//                                           REG(a1,unsigned char *tmap),
//                                           REG(fp0,float inverse_z),
//                                           REG(fp1,float u_div_z),
//                                           REG(fp2,float v_div_z),
//                                           REG(fp3,float delta_inv_z),
//                                           REG(fp4,float delta_u),
//                                           REG(fp5,float delta_v) );

#define INTERPOL_STEP 16
#define INTERPOL_STEP_SHIFT 4

	.text

// The problem with the draw_sl_map_n68k_small() function
// is that it doesn't work with textures larger than certain
// size. Currently it's not used at all because of some
// trouble when selecting the routine (small vs. big).

	.align  4   
.globl C(draw_sl_map_m68k_small),C(draw_sl_map_m68k_big)
C(draw_sl_map_m68k_small):
	movml   %d2-%d7/%a2-%a6,%sp@-
	fmovx   %fp7,%sp@-
	fmovx   %fp6,%sp@-
#ifdef OS_LINUX
	fmovx	%fp5,%sp@-
	fmovx	%fp4,%sp@-
	fmovx	%fp3,%sp@-
#endif
	fmovx   %fp2,%sp@-

#ifdef OS_LINUX
	movl  	%sp@(4+116),%d0
    	movl  	%sp@(8+116),%d1
    	movl  	%sp@(12+116),%d2
    	movl  	%sp@(16+116),%d3
    	movl  	%sp@(20+116),%a0
    	movl  	%sp@(24+116),%a1
    	fmovs 	%sp@(28+116),%fp0
    	fmovs 	%sp@(32+116),%fp1
    	fmovs 	%sp@(36+116),%fp2
    	fmovs 	%sp@(40+116),%fp3
    	fmovs 	%sp@(44+116),%fp4
    	fmovs 	%sp@(48+116),%fp5
#endif

	fmovs   #0x47800000,%fp6
	fdivx   %fp0,%fp6

	swap    %d1         // tw
	swap    %d2                 // th
	movl    %d0,%a2             // xx
	movl    %d1,%a3             // tw
	movl    %d2,%a4             // th
	movl    %d3,%a5             // shf_u
	movw    %pc@(Lumask:W,%d3:L:2),%d4 // umask
	movw    %pc@(Lvmask:W,%d3:L:2),%d5 // vmask

	fmovx   %fp2,%fp7           // v
	fmulx   %fp6,%fp7
	fmulx   %fp1,%fp6           // u

	fmovl   %fp6,%sp@-
	fmovl   %fp7,%sp@-
	faddx   %fp3,%fp0           // inv_z += delta_z
	faddx   %fp4,%fp1           // u_div_z += delta_u
	faddx   %fp5,%fp2           // v_div_z += delta_v
	fmovs   #0x47800000,%fp6
	fdivx   %fp0,%fp6
	movl    %sp@+,%d7           // clamp uu and vv
					// * uses sxx instructions to
					//   eliminate conditions branches
	spl     %d4                 // set if positive
	cmpl    %a4,%d7
	slt     %d5                 // set if lower than
	extbl   %d4
	extbl   %d5
	andl    %d4,%d7
	movl    %a4,%d4
	andl    %d5,%d7
	subql   #1,%d4
	notl    %d5
	andl    %d5,%d4
	orl     %d4,%d7
	movl    %sp@+,%d6           // clamp uu and vv
	spl     %d4                 // set if positive
	cmpl    %a3,%d6
	slt     %d5                 // set if lower than
	extbl   %d4
	extbl   %d5
	andl    %d4,%d6
	movl    %a3,%d4
	andl    %d5,%d6
	subql   #1,%d4
	notl    %d5
	movl    %d7,%sp@-           // v
	andl    %d5,%d4
	orl     %d4,%d6
	movl    %d6,%sp@-           // u

Lloop:
	cmpl    #INTERPOL_STEP,%a2
	jlt     Lendloop

	fmovx   %fp2,%fp7           // v
	fmulx   %fp6,%fp7
	subw    #INTERPOL_STEP,%a2

	fmovl   %fp7,%sp@-
	fmulx   %fp1,%fp6           // u
	movl    %sp@+,%d3           // clamp uu and vv
	spl     %d6                 // set if positive
	cmpl    %a4,%d3
	slt     %d7                 // set if lower than
	extbl   %d6
	extbl   %d7
	andl    %d6,%d3
	movl    %a4,%d4
	andl    %d7,%d3
	subql   #1,%d4
	notl    %d7
	andl    %d7,%d4
	orl     %d4,%d3
	fmovl   %fp6,%sp@-
	movl    %sp@+,%d2           // clamp uu and vv
	spl     %d6                 // set if positive
	cmpl    %a3,%d2
	slt     %d7                 // set if lower than
	extbl   %d6
	extbl   %d7
	andl    %d6,%d2
	movl    %a3,%d4
	movl    %sp@+,%d0
	andl    %d7,%d2
	movl    %sp@+,%d1
	subql   #1,%d4
	notl    %d7
	movl    %d3,%sp@-
	andl    %d7,%d4
	orl     %d4,%d2

	movl    %d2,%sp@-
	subl    %d0,%d2
	subl    %d1,%d3

	notl    %d0
	notl    %d1
	notl    %d2
	notl    %d3

	asrl    #INTERPOL_STEP_SHIFT,%d2
	asrl    #INTERPOL_STEP_SHIFT,%d3

	faddx   %fp3,%fp0           // inv_z += delta_z

	eorw    %d0,%d1
	eorw    %d2,%d3
	eorw    %d1,%d0
	eorw    %d3,%d2
	eorw    %d0,%d1
	eorw    %d2,%d3

	faddx   %fp4,%fp1           // u_div_z += delta_u

	moveq   #16,%d6
	roll    %d6,%d0
	roll    %d6,%d1
	roll    %d6,%d2
	roll    %d6,%d3

	faddx   %fp5,%fp2           // v_div_z += delta_v

	movl    %a5,%d6
	lslw    %d6,%d1
	lslw    %d6,%d3

	moveq   #0,%d6

	fmovs   #0x47800000,%fp6
	subw    %d3,%d1
	addl    %d3,%d1
	fdivx   %fp0,%fp6

// d0: uu (%vvvv vvvv vvvv vvvv 1111 1111 UUUU UUUU)
// d1: vv (%uuuu uuuu uuuu uuuu VVVV VVVV 1111 1111)
// d2: duu
// d3: dvv
// d4: mask_uu (%1111 1111 0000 0000)
// d5: mask_vv (%0000 0000 1111 1111)
// a0: d
// a1: tmap2
Linner:
	orw     %d4,%d0
	orw     %d5,%d1
	movw    %d0,%d6
	eorw    %d1,%d6
	addxl   %d2,%d0
	addxl   %d3,%d1
	movw    %a1@(%d6:L),%d7
	orw     %d4,%d0
	orw     %d5,%d1
	movw    %d0,%d6
	eorw    %d1,%d6
	addxl   %d2,%d0
	addxl   %d3,%d1
	movb    %a1@(%d6:L),%d7
	.rept   INTERPOL_STEP/2-1
	orw     %d4,%d0
	orw     %d5,%d1
	movw    %d0,%d6
	movw    %d7,%a0@+
	eorw    %d1,%d6
	addxl   %d2,%d0
	addxl   %d3,%d1
	movw    %a1@(%d6:L),%d7
	orw     %d4,%d0
	orw     %d5,%d1
	movw    %d0,%d6
	eorw    %d1,%d6
	addxl   %d2,%d0
	addxl   %d3,%d1
	movb    %a1@(%d6:L),%d7
	.endr
	movw    %d7,%a0@+
	jbra    Lloop
Lendloop:
	movl    %sp@+,%d0
	movl    %sp@+,%d1
	tstl    %a2
	jeq     Lend

	fmovx   %fp2,%fp7           // v
	fmulx   %fp6,%fp7

	fmovl   %fp7,%sp@-
	fmulx   %fp1,%fp6           // u
	movl    %sp@+,%d3           // clamp uu and vv
	spl     %d6                 // set if positive
	cmpl    %a4,%d3
	slt     %d7                 // set if lower than
	extbl   %d6
	extbl   %d7
	andl    %d6,%d3
	movl    %a4,%d4
	andl    %d7,%d3
	subql   #1,%d4
	notl    %d7
	andl    %d7,%d4
	orl     %d4,%d3
	fmovl   %fp6,%sp@-
	movl    %sp@+,%d2           // clamp uu and vv
	spl     %d6                     // set if positive
	cmpl    %a3,%d2
	slt     %d7                 // set if lower than
	extbl   %d6
	extbl   %d7
	andl    %d6,%d2
	movl    %a3,%d4
	andl    %d7,%d2
	subql   #1,%d4
	notl    %d7
	andl    %d7,%d4
	orl     %d4,%d2

	subl    %d0,%d2
	subl    %d1,%d3

	notl    %d0
	notl    %d1
	notl    %d2
	notl    %d3

	asrl    #INTERPOL_STEP_SHIFT,%d2
	asrl    #INTERPOL_STEP_SHIFT,%d3

	eorw    %d0,%d1
	eorw    %d2,%d3
	eorw    %d1,%d0
	eorw    %d3,%d2
	eorw    %d0,%d1
	eorw    %d2,%d3
	
	movq    #16,%d6
	roll    %d6,%d0
	roll    %d6,%d1
	roll    %d6,%d2
	roll    %d6,%d3

	movl    %a5,%d6
	lslw    %d6,%d1
	lslw    %d6,%d3

	movq    #0,%d6

	subw    %d3,%d1
	addl    %d3,%d1

Lloop2:
	orw     %d4,%d0
	orw     %d5,%d1
	movw    %d0,%d6
	eorw    %d1,%d6
	addxl   %d2,%d0
	addxl   %d3,%d1
	movb    %a1@(%d6:l),%d7
	subql   #1,%a2
	movb    %d7,%a0@+
	tstl    %a2
	jne     Lloop2

Lend:
	fmovx   %sp@+,%fp2
#ifdef OS_LINUX
	fmovx	%sp@+,%fp3
	fmovx	%sp@+,%fp4
	fmovx	%sp@+,%fp5
#endif
	fmovx   %sp@+,%fp6
	fmovx   %sp@+,%fp7
	movml   %sp@+,%d2-%d7/%a2-%a6
	rts

	.align  4 
Lumask:
	.word   0x0000,0xfffe,0xfffc,0xfff8
	.word   0xfff0,0xffe0,0xffc0,0xff80
	.word   0xff00,0xfe00,0xfc00,0xf800
	.word   0xf000,0xe000,0xc000,0x8000
Lvmask:
	.word   0x0000,0x0001,0x0003,0x0007
	.word   0x000f,0x001f,0x003f,0x007f
	.word   0x00ff,0x01ff,0x03ff,0x07ff
	.word   0x0fff,0x1fff,0x3fff,0x7fff

	.align  4

C(draw_sl_map_m68k_big):
	movml   %d2-%d7/%a2-%a6,%sp@-
	fmovx   %fp7,%sp@-
	fmovx   %fp6,%sp@-
#ifdef OS_LINUX
	fmovx	%fp5,%sp@-
	fmovx	%fp4,%sp@-
	fmovx	%fp3,%sp@-
#endif
	fmovx   %fp2,%sp@-

#ifdef OS_LINUX
        movl    %sp@(4+116),%d0
        movl    %sp@(8+116),%d1
        movl    %sp@(12+116),%d2
        movl    %sp@(16+116),%d3
        movl    %sp@(20+116),%a0
        movl    %sp@(24+116),%a1
        fmovs   %sp@(28+116),%fp0
        fmovs   %sp@(32+116),%fp1
        fmovs   %sp@(36+116),%fp2
        fmovs   %sp@(40+116),%fp3
        fmovs   %sp@(44+116),%fp4
        fmovs   %sp@(48+116),%fp5
#endif

	fmovs   #0x47800000,%fp6
	fdivx   %fp0,%fp6

	swap    %d1
	swap    %d2
	movl    %d0,%a2             // xx
	movl    %d1,%a3             // tw
	movl    %d2,%a4             // th
	movl    %d3,%a5             // shf_u

	fmovx   %fp2,%fp7           // v
	fmulx   %fp6,%fp7
	fmulx   %fp1,%fp6           // u

	fmovl   %fp6,%sp@-
	fmovl   %fp7,%sp@-
	faddx   %fp3,%fp0           // inv_z += delta_z
	faddx   %fp4,%fp1           // u_div_z += delta_u
	faddx   %fp5,%fp2           // v_div_z += delta_v
	fmovs   #0x47800000,%fp6
	fdivx   %fp0,%fp6
	movl    %sp@+,%d7           // clamp uu and vv
	spl     %d4                 // set if positive
	cmpl    %a4,%d7
	slt     %d5                 // set if lower than
	extbl   %d4
	extbl   %d5
	andl    %d4,%d7
	movl    %a4,%d4
	andl    %d5,%d7
	subql   #1,%d4
	notl    %d5
	andl    %d5,%d4
	orl     %d4,%d7
	movl    %sp@+,%d6           // clamp uu and vv
	spl     %d4                 // set if positive
	cmpl    %a3,%d6
	slt     %d5                 // set if lower than
	extbl   %d4
	extbl   %d5
	andl    %d4,%d6
	movl    %a3,%d4
	andl    %d5,%d6
	subql   #1,%d4
	notl    %d5
	movl    %d7,%sp@-           // v
	andl    %d5,%d4
	orl     %d4,%d6
	movl    %d6,%sp@-           // u

Lloop_b:
	cmpl    #INTERPOL_STEP,%a2
	jlt     Lendloop_b

	fmovx   %fp2,%fp7           // v
	fmulx   %fp6,%fp7
	subw    #INTERPOL_STEP,%a2

	fmovl   %fp7,%sp@-
	fmulx   %fp1,%fp6           // u
	movl    %sp@+,%d3           // clamp uu and vv
	spl     %d6                 // set if positive
	cmpl    %a4,%d3
	slt     %d7                 // set if lower than
	extbl   %d6
	extbl   %d7
	andl    %d6,%d3
	movl    %a4,%d4
	andl    %d7,%d3
	subql   #1,%d4
	notl    %d7
	andl    %d7,%d4
	orl     %d4,%d3
	fmovl   %fp6,%sp@-
	movl    %sp@+,%d2           // clamp uu and vv
	spl     %d6                 // set if positive
	cmpl    %a3,%d2
	slt     %d7                 // set if lower than
	extbl   %d6
	extbl   %d7
	andl    %d6,%d2
	movl    %a3,%d4
	movl    %sp@+,%d0
	andl    %d7,%d2
	movl    %sp@+,%d1
	subql   #1,%d4
	notl    %d7
	movl    %d3,%sp@-
	andl    %d7,%d4
	orl     %d4,%d2

	movl    %d2,%sp@-

	subl    %d0,%d2
	subl    %d1,%d3

	faddx   %fp3,%fp0           // inv_z += delta_z

	asrl    #8,%d0
	asrl    #8,%d2
	asrl    #INTERPOL_STEP_SHIFT,%d2
	asrl    #INTERPOL_STEP_SHIFT,%d3

	faddx   %fp4,%fp1           // u_div_z += delta_u

	movl    %d2,%a6
	movl    %d3,%d2

	movq    #16,%d3
	subl    %a5,%d3

	faddx   %fp5,%fp2           // v_div_z += delta_v

	movq    #1,%d4
	movl    %a5,%d5
	lsll    %d5,%d4
	fmovs   #0x47800000,%fp6
	subql   #1,%d4
	notl    %d4

	fdivx   %fp0,%fp6

// d0: uu16
// d1: vv16
// a6: duu16
// d2: dvv16
// d3: shift
// d4: mask
// a0: d
// a1: tmap2
Linner_b:
	movl    %d0,%d6
	movl    %d1,%d7
	asrl    %d3,%d7
	asrl    #8,%d6
	andl    %d4,%d7
	addl    %d6,%d7
	addl    %a6,%d0
	addl    %d2,%d1
	movw    %a1@(%d7:l),%d5
	movl    %d0,%d6
	movl    %d1,%d7
	asrl    %d3,%d7
	asrl    #8,%d6
	andl    %d4,%d7
	addl    %d6,%d7
	addl    %a6,%d0
	addl    %d2,%d1
	movb    %a1@(%d7:l),%d5
	.rept   INTERPOL_STEP/2-1
	movl    %d0,%d6
	movl    %d1,%d7
	asrl    %d3,%d7
	asrl    #8,%d6
	andl    %d4,%d7
	movw    %d5,%a0@+
	addl    %d6,%d7
	addl    %a6,%d0
	addl    %d2,%d1
	movw    %a1@(%d7:l),%d5
	movl    %d0,%d6
	movl    %d1,%d7
	asrl    %d3,%d7
	asrl    #8,%d6
	andl    %d4,%d7
	addl    %d6,%d7
	addl    %a6,%d0
	addl    %d2,%d1
	movb    %a1@(%d7:l),%d5
	.endr
	movw    %d5,%a0@+
	jbra    Lloop_b
Lendloop_b:
	movl    %sp@+,%d0
	movl    %sp@+,%d1
	tstl    %a2
	jeq     Lend_b

	fmovx   %fp2,%fp7           // v
	fmulx   %fp6,%fp7

	fmovl   %fp7,%sp@-
	fmulx   %fp1,%fp6           // u
	movl    %sp@+,%d3           // clamp uu and vv
	spl     %d6                 // set if positive
	cmpl    %a4,%d3
	slt     %d7                 // set if lower than
	extbl   %d6
	extbl   %d7
	andl    %d6,%d3
	movl    %a4,%d4
	andl    %d7,%d3
	subql   #1,%d4
	notl    %d7
	andl    %d7,%d4
	orl     %d4,%d3
	fmovl   %fp6,%sp@-
	movl    %sp@+,%d2           // clamp uu and vv
	spl     %d6                 // set if positive
	cmpl    %a3,%d2
	slt     %d7                 // set if lower than
	extbl   %d6
	extbl   %d7
	andl    %d6,%d2
	movl    %a3,%d4
	andl    %d7,%d2
	subql   #1,%d4
	notl    %d7
	andl    %d7,%d4
	orl     %d4,%d2

	subl    %d0,%d2
	subl    %d1,%d3

	asrl    #8,%d0
	asrl    #8,%d2

	asrl    #INTERPOL_STEP_SHIFT,%d2
	asrl    #INTERPOL_STEP_SHIFT,%d3

	movl    %d2,%a6
	movl    %d3,%d2

	movq    #16,%d3
	subl    %a5,%d3

	movq    #1,%d4
	movl    %a5,%d5
	lsll    %d5,%d4
	subql   #1,%d4
	notl    %d4

Lloop2_b:
	movl    %d0,%d6
	movl    %d1,%d7
	asrl    %d3,%d7
	asrl    #8,%d6
	andl    %d4,%d7
	orl     %d6,%d7
	addl    %a6,%d0
	addl    %d2,%d1
	movb    %a1@(%d7:l),%d5
	subql   #1,%a2
	movb    %d5,%a0@+
	tstl    %a2
	jne     Lloop2_b
Lend_b:
	fmovx   %sp@+,%fp2
#ifdef OS_LINUX
	fmovx	%sp@+,%fp3
	fmovx	%sp@+,%fp4
	fmovx	%sp@+,%fp5
#endif
	fmovx   %sp@+,%fp6
	fmovx   %sp@+,%fp7
	movml   %sp@+,%d2-%d7/%a2-%a6
	rts

