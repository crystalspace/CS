/*
    Crystal Space 8-bit csGraphics2D assembler-optimized routines
    Copyright (C) 1998 by Olivier Langlois <olanglois@sympatico.ca>

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

#ifndef   _i386_GRAPH2D_H_
#define   _i386_GRAPH2D_H_


#define NO_DRAWLINE
// line drawing routine using Bresenham algorithm from the book
// Zen Graphics Programming
void csGraphics2D::DrawLine (float x1, float y1, float x2, float y2, int color)
{
  if (ClipLine (x1, y1, x2, y2, ClipX1, ClipY1, ClipX2, ClipY2))
    return;

  static unsigned oldEBP;
  static int lWidth;
  static unsigned char *lMemory = Memory;
  static int *lLineAddress = LineAddress;

  lWidth = Width;

  if( DrawPixel == DrawPixel8 )
  {
	  __asm
	  {
		  mov	oldEBP, ebp
		  mov	edi, lMemory
;
; Calculate DeltaY
;
		  mov	esi, y2
		  mov	eax, y1
		  sub	esi, eax
		  jns	CalcStartAddress
		  
          mov	eax, y2
		  mov	edx, x1
		  mov	ecx, x2
		  mov   x2, edx
		  mov   x1, ecx
		  neg	esi

CalcStartAddress:

		  add   edi, x1
		  mov   ebx, lLineAddress
          add	edi, DWORD PTR [ebx+eax*4]

;
; Calculate DeltaX
;
		  mov	ebx, x2
		  mov	eax, color
          sub	ebx, x1

		  js	NegDeltaX
		  cmp	ebx, esi
		  jb	Octant1

;
; DeltaX >= DeltaY >= 0
;
          mov	ecx, ebx ; Number of pixel in line
		  and	ecx, ecx
		  jz	LineEnd1
		  add	esi, esi ; DeltaY * 2
          mov	ebp, esi
		  sub	ebp, ebx
		  add	ebx, ebx ; DeltaX * 2
		  sub	esi, ebx
		  add	ebx, esi
LineLoop1:
		  mov	BYTE PTR [edi], al
		  and	ebp, ebp
		  js	MoveXCoord1
		  add	edi, lWidth
		  add	ebp, esi
		  jmp	MoveToNextByte1
MoveXCoord1:
		  add	ebp,ebx
MoveToNextByte1:
		  inc	edi
		  dec	ecx
		  jnz	LineLoop1
LineEnd1:
		  mov	BYTE PTR [edi], al
		  jmp	LineDone
;
; DeltaY > DeltaX >= 0
;
Octant1:
          mov	ecx, esi ; Number of pixel in line
		  and	ecx, ecx
		  jz	LineEnd2
		  add	ebx, ebx ; DeltaX * 2
          mov	ebp, ebx
		  sub	ebp, esi
		  add	esi, esi ; DeltaY * 2
		  sub	ebx, esi
		  add	esi, ebx
		  mov	BYTE PTR [edi], al
LineLoop2:
		  and	ebp, ebp
		  jns	MoveXCoord2
		  add	ebp, esi
		  jmp	MoveYCoord2
MoveXCoord2:
		  inc	edi
		  add	ebp,ebx
MoveYCoord2:
		  add	edi, lWidth
		  dec	ecx
		  mov	BYTE PTR [edi], al
		  jnz	LineLoop2
LineEnd2:         
          jmp	LineDone

NegDeltaX:
		  neg	ebx
		  cmp	ebx, esi
		  jb	Octant2
;
; |DeltaX| >= DeltaY and DeltaX < 0
;
          mov	ecx, ebx ; Number of pixel in line
		  and	ecx, ecx
		  jz	LineEnd3
		  add	esi, esi ; DeltaY * 2
          mov	ebp, esi
		  sub	ebp, ebx
		  add	ebx, ebx ; DeltaX * 2
		  sub	esi, ebx
		  add	ebx, esi
LineLoop3:
		  mov	BYTE PTR [edi], al
		  and	ebp, ebp
		  js	MoveXCoord3
		  add	edi, lWidth
		  add	ebp, esi
		  jmp	MoveToNextByte3
MoveXCoord3:
		  add	ebp,ebx
MoveToNextByte3:
		  dec	edi
		  dec	ecx
		  jnz	LineLoop3
LineEnd3:
		  mov	BYTE PTR [edi], al          
          jmp	LineDone
;
; |DeltaX| < DeltaY and DeltaX < 0
;
Octant2:
          mov	ecx, esi ; Number of pixel in line
		  and	ecx, ecx
		  jz	LineEnd4
		  add	ebx, ebx ; DeltaX * 2
          mov	ebp, ebx
		  sub	ebp, esi
		  add	esi, esi ; DeltaY * 2
		  sub	ebx, esi
		  add	esi, ebx
		  mov	BYTE PTR [edi], al
LineLoop4:
		  and	ebp, ebp
		  jns	MoveXCoord4
		  add	ebp, esi
		  jmp	MoveYCoord4
MoveXCoord4:
		  dec	edi
		  add	ebp,ebx
MoveYCoord4:
		  add	edi, lWidth
		  dec	ecx
		  mov	BYTE PTR [edi], al
		  jnz	LineLoop4
LineEnd4:
LineDone:
		  mov	ebp, oldEBP
	  }
  }
  else // DrawPixel16
  {
	  lWidth *= 2;
	  __asm
	  {
		  mov	oldEBP, ebp
;
; Calculate DeltaY
;
		  mov	esi, y2
		  mov	eax, y1
		  sub	esi, eax
		  jns	CalcStartAddress_16
		  
          mov	eax, y2
		  mov	edx, x1
		  mov	ecx, x2
		  mov   x2, edx
		  mov   x1, ecx
		  neg	esi

CalcStartAddress_16:

		  mov   edi, x1
		  mov   ebx, lLineAddress
		  add	edi, edi
		  add	edi, lMemory
          add	edi, DWORD PTR [ebx+eax*4]
;
; Calculate DeltaX
;
		  mov	ebx, x2
		  mov	eax, color
          sub	ebx, x1

		  js	NegDeltaX_16
		  cmp	ebx, esi
		  jb	Octant1_16

;
; DeltaX >= DeltaY >= 0
;
          mov	ecx, ebx ; Number of pixel in line
		  and	ecx, ecx
		  jz	LineEnd1_16
		  add	esi, esi ; DeltaY * 2
          mov	ebp, esi
		  sub	ebp, ebx
		  add	ebx, ebx ; DeltaX * 2
		  sub	esi, ebx
		  add	ebx, esi
LineLoop1_16:
		  mov	WORD PTR [edi], ax
		  and	ebp, ebp
		  js	MoveXCoord1_16
		  add	edi, lWidth
		  add	ebp, esi
		  jmp	MoveToNextByte1_16
MoveXCoord1_16:
		  add	ebp,ebx
MoveToNextByte1_16:
		  add	edi, 2
		  dec	ecx
		  jnz	LineLoop1_16
LineEnd1_16:
		  mov	WORD PTR [edi], ax
		  jmp	LineDone_16
;
; DeltaY > DeltaX >= 0
;
Octant1_16:
          mov	ecx, esi ; Number of pixel in line
		  and	ecx, ecx
		  jz	LineEnd2_16
		  add	ebx, ebx ; DeltaX * 2
          mov	ebp, ebx
		  sub	ebp, esi
		  add	esi, esi ; DeltaY * 2
		  sub	ebx, esi
		  add	esi, ebx
		  mov	WORD PTR [edi], ax
LineLoop2_16:
		  and	ebp, ebp
		  jns	MoveXCoord2_16
		  add	ebp, esi
		  jmp	MoveYCoord2_16
MoveXCoord2_16:
		  add	edi, 2
		  add	ebp,ebx
MoveYCoord2_16:
		  add	edi, lWidth
		  dec	ecx
		  mov	WORD PTR [edi], ax
		  jnz	LineLoop2_16
LineEnd2_16:         
          jmp	LineDone_16

NegDeltaX_16:
		  neg	ebx
		  cmp	ebx, esi
		  jb	Octant2_16
;
; |DeltaX| >= DeltaY and DeltaX < 0
;
          mov	ecx, ebx ; Number of pixel in line
		  and	ecx, ecx
		  jz	LineEnd3_16
		  add	esi, esi ; DeltaY * 2
          mov	ebp, esi
		  sub	ebp, ebx
		  add	ebx, ebx ; DeltaX * 2
		  sub	esi, ebx
		  add	ebx, esi
LineLoop3_16:
		  mov	WORD PTR [edi], ax
		  and	ebp, ebp
		  js	MoveXCoord3_16
		  add	edi, lWidth
		  add	ebp, esi
		  jmp	MoveToNextByte3_16
MoveXCoord3_16:
		  add	ebp,ebx
MoveToNextByte3_16:
		  sub	edi, 2
		  dec	ecx
		  jnz	LineLoop3_16
LineEnd3_16:
		  mov	WORD PTR [edi], ax          
          jmp	LineDone_16
;
; |DeltaX| < DeltaY and DeltaX < 0
;
Octant2_16:
          mov	ecx, esi ; Number of pixel in line
		  and	ecx, ecx
		  jz	LineEnd4_16
		  add	ebx, ebx ; DeltaX * 2
          mov	ebp, ebx
		  sub	ebp, esi
		  add	esi, esi ; DeltaY * 2
		  sub	ebx, esi
		  add	esi, ebx
		  mov	WORD PTR [edi], ax
LineLoop4_16:
		  and	ebp, ebp
		  jns	MoveXCoord4_16
		  add	ebp, esi
		  jmp	MoveYCoord4_16
MoveXCoord4_16:
		  sub	edi, 2
		  add	ebp,ebx
MoveYCoord4_16:
		  add	edi, lWidth
		  dec	ecx
		  mov	WORD PTR [edi], ax
		  jnz	LineLoop4_16
LineEnd4_16:
LineDone_16:
		  mov	ebp, oldEBP
	  }
  }
}

#endif /* _i386_GRAPH2D_H_ */
