//
//  c2p8_n.S
//
//  A basic 8 bpl chunky to planar function based on the
//  merge routine by Mikael Kalms.
//

#ifdef ELF
#define C(label) label
#else
#define C(label) _##label
#endif

#ifdef OS_AMIGAOS
#define bm_Planes 8
#define MEMF_ANY 0
#define _LVOAllocVec -684
#define _LVOCacheClearU -636
#endif
	.text

#ifdef OS_AMIGAOS
	.align  4
.globl C(c2p8_reloc),C(c2p8)
C(c2p8_reloc):
	movml   %a2/%a3,%sp@-
	movl    %a0,%a3
	lea     %a1@(bm_Planes:W),%a2
	movl    #c2p8_end-_c2p8,%d0
	movq    #MEMF_ANY,%d1
	movl    4:W,%a6
	jsr     %a6@(_LVOAllocVec:W)
	tstl    %d0
	jeq     Lfail_openlib
	movl    %d0,%a0
	lea     %pc@(C(c2p8)),%a1
	movw    #c2p8_end-C(c2p8)-1,%d1
Lcopy_loop:
	movb    %a1@+,%a0@+
	dbf     %d1,Lcopy_loop

	movl    %d2,%sp@-
	movl    %d0,%a0
	movl    %a3,%a0@(test1-C(c2p8)-4)
	movl    %a3,%a0@(test2-C(c2p8)-4)
	movl    %a2@(4:W),%d1
	subl    %a2@,%d1
	movl    %d1,%a0@(p1_1-C(c2p8)-4)
	movl    %d1,%a0@(p1_2-C(c2p8)-4)
	movl    %a2@(8:W),%d1
	subl    %a2@,%d1
	movl    %d1,%a0@(p2_1-C(c2p8)-4)
	movl    %d1,%a0@(p2_2-C(c2p8)-4)
	movl    %a2@(12:W),%d1
	subl    %a2@,%d1
	movl    %d1,%a0@(p3_1-C(c2p8)-4)
	movl    %d1,%a0@(p3_2-C(c2p8)-4)
	movl    %a2@(16:W),%d1
	subl    %a2@,%d1
	movl    %d1,%a0@(p4_1-C(c2p8)-4)
	movl    %d1,%a0@(p4_2-C(c2p8)-4)
	movl    %a2@(20:W),%d1
	subl    %a2@,%d1
	movl    %d1,%a0@(p5_1-C(c2p8)-4)
	movl    %d1,%a0@(p5_2-C(c2p8)-4)
	movl    %a2@(24:W),%d1
	subl    %a2@,%d1
	movl    %d1,%a0@(p6_1-C(c2p8)-4)
	movl    %d1,%a0@(p6_2-C(c2p8)-4)
	movl    %a2@(28:W),%d1
	subl    %a2@,%d1
	movl    %d1,%a0@(p7_1-C(c2p8)-4)
	movl    %d1,%a0@(p7_2-C(c2p8)-4)

	movl    %sp@+,%d2
	movl    %d0,%a2
	jsr     %a6@(_LVOCacheClearU:W)
	movl    %a2,%d0
Lfail_openlib:
	movml   %sp@+,%a2/%a3
	rts
#endif

#define STACK 0
#define ST_SIZE 4

	.align  4
C(c2p8):
	movml   %d2-%d7/%a2-%a6,%sp@-
	lea     %sp@(-(ST_SIZE+16):W),%sp
	movl    %sp,%d2
	addql   #8,%d2
	addql   #8,%d2
	andb    #0xf0,%d2
	movl    %d2,%d1
	subl    %sp,%d1
	movl    %d2,%sp

	movl    %d1,%sp@(STACK:W)
	movl    %a1@(bm_Planes:W),%a1
Lstart:
	movl    %a0@+,%d0
	movl    %a0@+,%d1
	movl    %a0@+,%d2
	movl    %a0@+,%d3
	movl    %a0@+,%d4
	movl    %a0@+,%d5
	movl    %a0@+,%d6
	movl    %a0@+,%a2

// 16x4: (d0,d4),(d2,d6),(d1,d5)
// 2x4: (d0,d4),(d2,d6),(d1,d5)
// 16x4: (d3,d7)
// 2x4: (d3,d7)
// 8x2: (d0,d2),(d1,d3)
// 1x2: (d0,d2),(d1,d3)
// 4x1: (d0,d1)
// write d0
// 4x1: (d2,d3)
// 8x2: (d4,d6),(d5,d7)
// 1x2: (d4,d6),(d5,d7)
// 4x1: (d4,d5),(d6,d7)

// 16x4: (d0,d4),(d1,d5),(d2,d6)
	swap    %d4
	swap    %d5
	swap    %d6
	eorw    %d4,%d0
	eorw    %d5,%d1
	eorw    %d6,%d2
	eorw    %d0,%d4
	eorw    %d1,%d5
	eorw    %d2,%d6
	eorw    %d4,%d0
	eorw    %d5,%d1
	eorw    %d6,%d2
	swap    %d4
	swap    %d5
	swap    %d6

// 2x4: (d0,d4),(d1,d5),(d2,d6)
	movl    %d4,%d7
	lsrl    #2,%d7
	eorl    %d0,%d7
	andl    #0x33333333,%d7
	eorl    %d7,%d0
	lsll    #2,%d7
	eorl    %d7,%d4

	movl    %d5,%d7
	lsrl    #2,%d7
	eorl    %d1,%d7
	andl    #0x33333333,%d7
	eorl    %d7,%d1
	lsll    #2,%d7
	eorl    %d7,%d5

	movl    %d6,%d7
	lsrl    #2,%d7
	eorl    %d2,%d7
	andl    #0x33333333,%d7
	eorl    %d7,%d2
	lsll    #2,%d7
	eorl    %d7,%d6

	exg     %a2,%d6             // d7 -> d6
					// d6 -> a2

// 16x4: (d3,d7)

	swap    %d6
	eorw    %d6,%d3
	eorw    %d3,%d6
	eorw    %d6,%d3
	swap    %d6

// 2x4 (d3,d7)

	movl    %d6,%d7
	lsrl    #2,%d7
	eorl    %d3,%d7
	andl    #0x33333333,%d7
	eorl    %d7,%d3
	lsll    #2,%d7
	eorl    %d7,%d6

// 8x2: (d0,d2),(d1,d3)

	movl    %d2,%d7
	lsrl    #8,%d7
	eorl    %d0,%d7
	andl    #0x00ff00ff,%d7
	eorl    %d7,%d0
	lsll    #8,%d7
	eorl    %d7,%d2

	movl    %d3,%d7
	lsrl    #8,%d7
	eorl    %d1,%d7
	andl    #0x00ff00ff,%d7
	eorl    %d7,%d1
	lsll    #8,%d7
	eorl    %d7,%d3

// 1x2: (d0,d2),(d1,d3)

	movl    %d2,%d7
	lsrl    #1,%d7
	eorl    %d0,%d7
	andl    #0x55555555,%d7
	eorl    %d7,%d0
	addl    %d7,%d7
	eorl    %d7,%d2

	movl    %d3,%d7
	lsrl    #1,%d7
	eorl    %d1,%d7
	andl    #0x55555555,%d7
	eorl    %d7,%d1
	addl    %d7,%d7
	eorl    %d7,%d3

// 4x1: (d0,d1)

	movl    %d1,%d7
	lsrl    #4,%d7
	eorl    %d0,%d7
	andl    #0x0f0f0f0f,%d7
	eorl    %d7,%d0
	lsll    #4,%d7
	eorl    %d7,%d1

// 4x1: (d2,d3)

	movl    %d3,%d7
	lsrl    #4,%d7
	eorl    %d2,%d7

	movl    %d0,%a1@(0x12341234)
p7_1:
	andl    #0x0f0f0f0f,%d7
	eorl    %d7,%d2
	lsll    #4,%d7
	eorl    %d7,%d3

	movl    %a2,%d0               // d6 -> %d0

// 8x2: (d4,d6),(d5,d7)

	movl    %d0,%d7
	lsrl    #8,%d7
	eorl    %d4,%d7
	andl    #0x00ff00ff,%d7
	eorl    %d7,%d4
	lsll    #8,%d7
	eorl    %d7,%d0

	movl    %d6,%d7
	lsrl    #8,%d7
	eorl    %d5,%d7
	andl    #0x00ff00ff,%d7
	eorl    %d7,%d5
	movl    %d1,%a1@(0x12341234)
p3_1:
	lsll    #8,%d7
	eorl    %d7,%d6

// 1x2: (d4,d6),(d5,d7)

	movl    %d0,d7
	lsrl    #1,%d7
	eorl    %d4,%d7
	andl    #0x55555555,%d7
	eorl    %d7,d4
	addl    %d7,%d7
	eorl    %d7,%d0

	movl    %d6,%d7
	lsrl    #1,%d7
	eorl    %d5,%d7
	andl    #0x55555555,%d7
	eorl    %d7,%d5
	addl    %d7,%d7
	eorl    %d7,%d6

// 4x1: (d4,d5),(d6,d7)

	movl    %d5,%d7
	movl    %d2,%a1@(0x12341234)
p6_1:
	lsrl    #4,%d7
	eorl    %d4,%d7
	andl    #0x0f0f0f0f,%d7
	eorl    %d7,%d4
	lsll    #4,%d7
	eorl    %d7,%d5

	movl    %d6,%d7
	lsrl    #4,%d7
	eorl    %d0,%d7
	andl    #0x0f0f0f0f,%d7
	eorl    %d7,%d0
	lsll    #4,%d7
	eorl    %d7,%d6

	movl    %d3,%d7
	movl    %d4,%a3
	movl    %d5,%a4
	movl    %d0,%a5
	movl    %d6,%a6

	cmpl    #0xDEADBEEF,%a0
test1:
	jeq     Lend
Lloop:
	movl    %a0@+,%d0
	movl    %a0@+,%d1
	movl    %a0@+,%d2
	movl    %a0@+,%d3
	movl    %a0@+,%d4
	movl    %a0@+,%d5
	movl    %a0@+,%d6
	movl    %a0@+,%a2
	movl    %d7,%a1@(0x12341234)
p2_1:
// 16x4: (d0,d4),(d1,d5),(d2,d6)
	swap    %d4
	swap    %d5
	swap    %d6
	eorw    %d4,%d0
	eorw    %d5,%d1
	eorw    %d6,%d2
	eorw    %d0,%d4
	eorw    %d1,%d5
	eorw    %d2,%d6
	eorw    %d4,%d0
	eorw    %d5,%d1
	eorw    %d6,%d2
	swap    %d4
	swap    %d5
	swap    %d6

// 2x4: (d0,d4),(d1,d5),(d2,d6)
	movl    %d4,%d7
	lsrl    #2,%d7
	movl    %a3,%a1@(0x12341234)
p5_1: 
	eorl    %d0,%d7
	andl    #0x33333333,%d7
	eorl    %d7,%d0
	lsll    #2,%d7
	eorl    %d7,%d4

	movl    %d5,%d7
	lsrl    #2,%d7
	eorl    %d1,%d7
	andl    #0x33333333,%d7
	eorl    %d7,%d1
	lsll    #2,%d7
	eorl    %d7,%d5

	movl    %d6,%d7
	lsrl    #2,%d7
	eorl    %d2,%d7
	andl    #0x33333333,%d7
	eorl    %d7,%d2
	movl    %a4,%a1@(0x12341234)
p1_1: 
	lsll    #2,%d7
	eorl    %d7,%d6

	exg     %a2,%d6             // d7 -> d6
					// d6 -> a2

// 16x4: (d3,d7)

	swap    %d6
	eorw    %d6,%d3
	eorw    %d3,%d6
	eorw    %d6,%d3
	swap    %d6

// 2x4 (d3,d7)

	movl    %d6,%d7
	lsrl    #2,%d7
	eorl    %d3,%d7
	andl    #0x33333333,%d7
	eorl    %d7,%d3
	lsll    #2,%d7
	eorl    %d7,%d6

// 8x2: (d0,d2),(d1,d3)

	movl    %d2,%d7
	lsrl    #8,%d7
	eorl    %d0,%d7
	movl    %a5,%a1@(0x12341234)
p4_1:
	andl    #0x00ff00ff,%d7
	eorl    %d7,%d0
	lsll    #8,%d7
	eorl    %d7,%d2

	movl    %d3,%d7
	lsrl    #8,%d7
	eorl    %d1,%d7
	andl    #0x00ff00ff,%d7
	eorl    %d7,%d1
	lsll    #8,%d7
	eorl    %d7,%d3

// 1x2: (d0,d2),(d1,d3)

	movl    %d2,%d7
	lsrl    #1,%d7
	eorl    %d0,%d7
	andl    #0x55555555,%d7
	eorl    %d7,%d0
	addl    %d7,%d7
	movl    %a6,%a1@+            // plane 0
	eorl    %d7,%d2

	movl    %d3,%d7
	lsrl    #1,%d7
	eorl    %d1,%d7
	andl    #0x55555555,%d7
	eorl    %d7,%d1
	addl    %d7,%d7
	eorl    %d7,%d3

// 4x1: (d0,d1)

	movl    %d1,%d7
	lsrl    #4,%d7
	eorl    %d0,%d7
	andl    #0x0f0f0f0f,%d7
	eorl    %d7,%d0
	lsll    #4,%d7
	eorl    %d7,%d1

// 4x1: (d2,d3)

	movl    %d3,%d7
	lsrl    #4,%d7
	movl    %d0,%a1@(0x12341234)
p7_2: 
	eorl    %d2,%d7
	andl    #0x0f0f0f0f,%d7
	eorl    %d7,%d2
	lsll    #4,%d7
	eorl    %d7,%d3

	movl    %a2,%d0               // d6 -> d0

// 8x2: (d4,d6),(d5,d7)

	movl    %d0,%d7
	lsrl    #8,%d7
	eorl    %d4,%d7
	andl    #0x00ff00ff,%d7
	eorl    %d7,%d4
	lsll    #8,%d7
	eorl    %d7,%d0

	movl    %d6,%d7
	lsrl    #8,%d7
	eorl    %d5,%d7
	andl    #0x00ff00ff,%d7
	eorl    %d7,%d5
	movl    %d1,%a1@(0x12341234)
p3_2: 
	lsll    #8,%d7
	eorl    %d7,%d6

// 1x2: (d4,d6),(d5,d7)

	movl    %d0,%d7
	lsrl    #1,%d7
	eorl    %d4,%d7
	andl    #0x55555555,%d7
	eorl    %d7,%d4
	addl    %d7,%d7
	eorl    %d7,%d0

	movl    %d6,%d7
	lsrl    #1,%d7
	eorl    %d5,%d7
	andl    #0x55555555,%d7
	eorl    %d7,%d5
	addl    %d7,%d7
	eorl    %d7,%d6

// 4x1: (d4,d5),(d6,d7)

	movl    %d5,%d7
	movl    %d2,%a1@(0x12341234)
p6_2: 
	lsrl    #4,%d7
	eorl    %d4,%d7
	andl    #0x0f0f0f0f,%d7
	eorl    %d7,%d4
	lsll    #4,%d7
	eorl    %d7,%d5

	movl    %d6,%d7
	lsrl    #4,%d7
	eorl    %d0,%d7
	andl    #0x0f0f0f0f,%d7
	eorl    %d7,%d0
	lsll    #4,%d7
	eorl    %d7,%d6

	movl    %d3,%d7
	movl    %d4,%a3
	movl    %d5,%a4
	movl    %d0,%a5
	movl    %d6,%a6

	cmpl    #0xDEADBEEF,%a0
test2: 
	jne     Lloop
Lend:
	movl    %d7,%a1@(0x12341234)
p2_2: 
	movl    %a3,%a1@(0x12341234)
p5_2: 
	movl    %a4,%a1@(0x12341234)
p1_2: 
	movl    %a5,%a1@(0x12341234)
p4_2: 
	movl    %a6,%a1@

	movl    %sp,%d0
	subl    %sp@(STACK),%d0
	addl    #ST_SIZE+16,%d0
	movl    %d0,%sp

	moveml  %sp@+,%d2-%d7/%a2-%a6
	rts
c2p8_end:




