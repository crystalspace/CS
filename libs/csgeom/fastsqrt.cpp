/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Largely rewritten by Ivan Avramovic <ivan@avramovic.com>
  
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

#include "cssysdef.h"
#include <stdio.h>
#include <math.h>
#include "csgeom/fastsqrt.h"

union FastSqrtUnion
{
  float d;
  uint32 i;
};

static uint32 fast_sqrt_table[0x10000];

// declare table of square roots
void BuildSqrtTable ()
{
  uint32 i;
  FastSqrtUnion s;

  for (i = 0; i <= 0x7FFF; i++)
  {
    // Build a float with the bit pattern i as mantissa
    // and an exponent of 0, stored as 127
    s.i = (i << 8) | (0x7F << 23);
    s.d = sqrt (s.d);

    // Take the square root then strip the first 7 bits of
    // the mantissa into the table
    fast_sqrt_table[i + 0x8000] = (s.i & 0x7FFFFF);

    // Repeat the process, this time with an exponent of 1,
    // stored as 128
    s.i = (i << 8) | (0x80 << 23);
    s.d = (float)sqrt (s.d);

    fast_sqrt_table[i] = (s.i & 0x7FFFFF);
  }
}

float FastSqrt (float n)
{
  *(int *) &n = fast_sqrt_table[(*(int *) &n >> 8) & 0xFFFF] ^ ((((*(int *) &n - 0x3F800000) >> 1) + 0x3F800000) & 0x7F800000);
  return n;
}

