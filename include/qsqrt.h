/*
    Copyright (C) 2000 by Andrew Zabolotny
    Fast computation of sqrt(x) and 1/sqrt(x)
  
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

// Uncomment the following line to define CS_NO_QSQRT if you experience
// mysterious problems with CS which you think are related to this
// version of sqrt not behaving properly. If you find something like
// that I'd like to be notified of this so we can make sure this really
// is the problem.
//#define CS_NO_QSQRT

#ifndef __QSQRT_H__
#define __QSQRT_H__

#if (!defined (CS_NO_QSQRT)) && defined (PROC_X86) && defined (COMP_GCC)

/*
  NB: Single-precision floating-point format (32 bits):
	SEEEEEEE.EMMMMMMM.MMMMMMMM.MMMMMMMM
	S: Sign (0 - positive, 1 - negative)
	E: Exponent (plus 127, 8 bits)
	M: Mantissa (23 bits)
*/

/**
 * This is a relatively small inline routine which will compute sqrt(x)
 * very quick, taking a square root is now just a little longer than
 * doing a division. The function is inline so that it runs at maximal
 * possible speed. It does eight multiplications but multiplication is
 * cheap on P5+ processors (3 clocks). On a Celeron CPU it takes
 * approximatively 50 clocks, while a division is 42 clocks.
 */
static inline float qsqrt (float x)
{
  float ret;

// Original C++ formulae:
// float tmp = x;
// *((unsigned *)&tmp) = (0xbe6f0000 - *((unsigned *)&tmp)) >> 1;
// double h = x * 0.5;
// double a = tmp;
// a *= 1.5 - a * a * h;
// a *= 1.5 - a * a * h;
// return a * x;

  __asm__ (
		"flds	%1\n"			// x
		"movl	$0xbe6f0000,%%eax\n"
		"subl	%1,%%eax\n"
		"shrl	$1,%%eax\n"
		"movl	%%eax,%1\n"
		"flds	%2\n"			// x 0.5
		"fmul	%%st(1)\n"		// x h
		"flds	%3\n"			// x h 1.5
		"flds	%1\n"			// x h 1.5 a
		"fld	%%st\n"			// x h 1.5 a a
		"fmul	%%st\n"			// x h 1.5 a a*a
		"fmul	%%st(3)\n"		// x h 1.5 a a*a*h
		"fsubr	%%st(2)\n"		// x h 1.5 a 1.5-a*a*h
		"fmulp	%%st(1)\n"		// x h 1.5 a
		"fld	%%st\n"			// x h 1.5 a a
		"fmul	%%st\n"			// x h 1.5 a a*a
		"fmulp	%%st(3)\n"		// x a*a*h 1.5 a
		"fxch\n"			// x a*a*h a 1.5
		"fsubp  %%st,%%st(2)\n"		// x 1.5-a*a*h a
		"fmulp	%%st(1)\n"		// x a
		"fmulp	%%st(1)\n"		// a
	: "=&t" (ret), "+m" (x) : "m" (0.5F), "m" (1.5F)
	: "eax", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
  );
  return ret;
}

/**
 * This routine is basically equivalent to qsqrt() except that it returns
 * 1/sqrt(x) rather than the proper square root. It should be used anywhere
 * you need the inverse root (in 3D graphics it is a common situation),
 * because the routine is a little faster than qsqrt() and also you avoid
 * a division (which gives you an overall 2X speedup).
 */
static inline float qisqrt (float x)
{
  float ret;
  __asm__ (
		"flds	%1\n"			// x
		"movl	$0xbe6f0000,%%eax\n"
		"subl	%1,%%eax\n"
		"shrl	$1,%%eax\n"
		"movl	%%eax,%1\n"
		"flds	%2\n"			// x 0.5
		"fmulp	%%st(1)\n"		// h
		"flds	%3\n"			// h 1.5
		"flds	%1\n"			// h 1.5 a
		"fld	%%st\n"			// h 1.5 a a
		"fmul	%%st\n"			// h 1.5 a a*a
		"fmul	%%st(3)\n"		// h 1.5 a a*a*h
		"fsubr	%%st(2)\n"		// h 1.5 a 1.5-a*a*h
		"fmulp	%%st(1)\n"		// h 1.5 a
		"fld	%%st\n"			// h 1.5 a a
		"fmul	%%st\n"			// h 1.5 a a*a
		"fmulp	%%st(3)\n"		// a*a*h 1.5 a
		"fxch\n"			// a*a*h a 1.5
		"fsubp  %%st,%%st(2)\n"		// 1.5-a*a*h a
		"fmulp	%%st(1)\n"		// a
	: "=t" (ret), "+m" (x) : "m" (0.5F), "m" (1.5F)
	: "eax", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
  );
  return ret;
}

#elif !defined (CS_NO_QSQRT) && defined (COMP_VC)

#pragma warning(disable:4035)               // re-enable below

static float __sqrt_half = 0.5F;
static float __sqrt_oneandhalf = 1.5F;

static inline float __cdecl qsqrt (float x)
{
  __asm {
		fld	[x]			// x
		mov	eax,0be6f0000h
		sub	eax,[x]
		shr	eax,1
		mov	[x],eax
		fld	[__sqrt_half]		// x 0.5
		fmul	st,st(1)		// x h
		fld	[__sqrt_oneandhalf]	// x h 1.5
		fld	[x]			// x h 1.5 a
		fld	st			// x h 1.5 a a
		fmul	st,st			// x h 1.5 a a*a
		fmul	st,st(3)		// x h 1.5 a a*a*h
		fsubr	st,st(2)		// x h 1.5 a 1.5-a*a*h
		fmulp	st(1),st		// x h 1.5 a
		fld	st			// x h 1.5 a a
		fmul	st,st			// x h 1.5 a a*a
		fmulp	st(3),st		// x a*a*h 1.5 a
		fxch				// x a*a*h a 1.5
		fsubrp	st(2),st		// x 1.5-a*a*h a
		fmulp	st(1),st		// x a
		fmulp	st(1),st		// a
  }
}

static inline float __cdecl qisqrt (float x)
{
  __asm {
		fld	[x]			// x
		mov	eax,0be6f0000h
		sub	eax,[x]
		shr	eax,1
		mov	[x],eax
		fld	[__sqrt_half]		// x 0.5
		fmulp	st(1),st		// h
		fld	[__sqrt_oneandhalf]	// h 1.5
		fld	[x]			// h 1.5 a
		fld	st			// h 1.5 a a
		fmul	st,st			// h 1.5 a a*a
		fmul	st,st(3)		// h 1.5 a a*a*h
		fsubr	st,st(2)		// h 1.5 a 1.5-a*a*h
		fmulp	st(1),st		// h 1.5 a
		fld	st			// h 1.5 a a
		fmul	st,st			// h 1.5 a a*a
		fmulp	st(3),st		// a*a*h 1.5 a
		fxch				// a*a*h a 1.5
		fsubrp	st(2),st		// 1.5-a*a*h a
		fmulp	st(1),st		// a
  }
}

#pragma warning(default:4035)

#else

#include <math.h>
#define qsqrt(x)  sqrt(x)
#define qisqrt(x) (1.0/sqrt(x))

#endif

#endif // __QSQRT_H__
