/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

// Assembly math routines for the VC compiler

#ifdef COMP_VC

#define CS_ASM__VECTOR3_PLUSEQ            \
    __asm                                 \
    {                                     \
	mov		eax, this         \
	mov		ebx, v            \
                                          \
	fld		[eax]this.x       \
	fadd	[ebx]v.x                  \
	fld		[eax]this.y       \
	fadd	[ebx]v.y                  \
	fld		[eax]this.z       \
	fadd	[ebx]v.z                  \
                                          \
	fxch	st(2)                     \
                                          \
	fstp	[eax]this.x               \
	fstp	[eax]this.y               \
	fstp	[eax]this.z               \
    }

#define CS_ASM__VECTOR3_MINUSEQ           \
    __asm                                 \
    {                                     \
	mov		eax, this         \
	mov		ebx, v            \
                                          \
	fld		[eax]this.x       \
	fsub	[ebx]v.x                  \
	fld		[eax]this.y       \
	fsub	[ebx]v.y                  \
	fld		[eax]this.z       \
	fsub	[ebx]v.z                  \
                                          \
	fxch	st(2)                     \
                                          \
	fstp	[eax]this.x               \
	fstp	[eax]this.y               \
	fstp	[eax]this.z               \
    }

#define CS_ASM__MATRIX3_MULTSCL           \
    __asm                                 \
    {                                     \
	  fld	C.m11                     \
	  fmul	s                         \
	  fld	C.m12                     \
	  fmul	s                         \
	  fld	C.m13                     \
	  fmul	s                         \
                                          \
	  fxch	st(2)                     \
                                          \
	  fstp	C.m11                     \
	  fstp	C.m12                     \
	  fstp	C.m13                     \
                                          \
	  fld	C.m21                     \
	  fmul	s                         \
	  fld	C.m22                     \
	  fmul	s                         \
	  fld	C.m23                     \
	  fmul	s                         \
                                          \
	  fxch	st(2)                     \
                                          \
	  fstp	C.m21                     \
	  fstp	C.m22                     \
	  fstp	C.m23                     \
                                          \
	  fld	C.m31                     \
	  fmul	s                         \
	  fld	C.m32                     \
	  fmul	s                         \
	  fld	C.m33                     \
	  fmul	s                         \
                                          \
	  fxch	st(2)                     \
                                          \
	  fstp	C.m31                     \
	  fstp	C.m32                     \
	  fstp	C.m33                     \
    }

#define CS_ASM__CALC_PLANE_NORMAL         \
    float x1, y1, z1, x2, y2, z2;         \
                                          \
    __asm                                 \
    {                                     \
	mov		eax, v1           \
	mov		ebx, v2           \
	mov		ecx, v3           \
                                          \
	; v1-v2                           \
	fld		[eax]v1.x         \
	fsub	[ebx]v2.x                 \
	fld		[eax]v1.y         \
	fsub	[ebx]v2.y                 \
	fld		[eax]v1.z         \
	fsub	[ebx]v2.z                 \
                                          \
	; Save result                     \
	fxch	st(2)                     \
	fst		x1                \
	fxch	st(1)                     \
	fst		y1                \
	fxch	st(2)                     \
	fst		z1                \
                                          \
    ; v1-v3                               \
	fld		[eax]v1.z         \
	fsub	[ecx]v3.z                 \
	fld		[eax]v1.y         \
	fsub	[ecx]v3.y                 \
	fld		[eax]v1.x         \
	fsub	[ecx]v3.x                 \
	fxch	st(2)                     \
                                          \
	fst		z2                \
	fmulp	st(5),st(0)               \
	fst		y2                \
	fmulp	st(3),st(0)               \
	fst		x2                \
	fmulp	st(1),st(0)               \
                                          \
; Floating point stack:                   \
;                                         \
; v1.z*v2.x                               \
; v1.x*v2.y                               \
; v1.y*v2.z                               \
                                          \
    fld		z1                        \
	fmul	y2                        \
	fld		y1                \
	fmul	x2                        \
	fld		x1                \
	fmul	z2                        \
	fxch	st(2)                     \
                                          \
	fsubp	st(5),st(0)               \
	fsubp	st(3),st(0)               \
	fsubp	st(1),st(0)               \
	fxch	st(2)                     \
                                          \
	fstp	x1                        \
	fstp	z1                        \
	fstp	y1                        \
    }                                     \
    norm.Set( x1, y1, z1 );

#define CS_ASM__TRANSFORM_O2T                   \
	float x,y,z;                            \
	const Vector3 &v2 = v_o2t;              \
	const Matrix3 &m  = m_o2t;              \
                                                \
    __asm                                       \
    {                                           \
	mov		eax, v                  \
	mov		ecx, v2                 \
	mov		ebx, m                  \
	fld		[eax]v.x                \
	fsub	[ecx]v2.x                       \
	fld		[eax]v.y                \
	fsub	[ecx]v2.y                       \
	fld		[eax]v.z                \
	fsub	[ecx]v2.z                       \
	fxch	st(2)                           \
                                                \
; Floating point stack:                         \
;                                               \
; x                                             \
; y                                             \
; z                                             \
	fst		x                       \
	fmul	[ebx]m.m21                      \
	fld		x                       \
	fmul	[ebx]m.m31                      \
	fld		x                       \
    fmul    [ebx]m.m11                          \
	fxch    st(3)                           \
                                                \
; Floating point stack:                         \
;                                               \
; y                                             \
; m31*x                                         \
; m21*x                                         \
; m11*x                                         \
; z                                             \
	fst		y                       \
	fmul	[ebx]m.m12                      \
	fld		y                       \
	fmul	[ebx]m.m22                      \
	fld		y                       \
	fmul	[ebx]m.m32                      \
	fxch	st(2)                           \
	faddp	st(5),st(0)                     \
	faddp	st(3),st(0)                     \
	faddp	st(1),st(0)                     \
	fxch	st(3)                           \
                                                \
; Floating point stack:                         \
;                                               \
; z                                             \
; m21*x+m22*y                                   \
; m11*x+m12*y                                   \
; m31*x+m32*y                                   \
	fst		z                       \
	fmul	[ebx]m.m33                      \
	fld		z                       \
	fmul	[ebx]m.m13                      \
	fld		z                       \
	fmul	[ebx]m.m23                      \
	fxch	st(2)                           \
	faddp	st(5),st(0)                     \
	faddp	st(3),st(0)                     \
	faddp	st(1),st(0)                     \
	fxch	st(2)                           \
                                                \
	fstp	z                               \
	fstp	x                               \
	fstp	y                               \
    }                                           \
    return Vector3(x, y, z);

#endif

