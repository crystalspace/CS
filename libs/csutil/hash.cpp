/*
    Copyright (C) 2000 by Jorrit Tyberghein

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
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

#define CS_CSUTIL_LIB
#include "cssysdef.h"
#include "csutil/hash.h"

inline uint32 rotate_bits_right_3 (uint32 h)
{
  return (h >> 3) | (h << 29);
}

uint32 csHashCompute (char const* s, int n)
{
  uint32 h = 0;
  char const* slim = s + n;
  while (s < slim)
    h = rotate_bits_right_3(h) + *s++;
  return h;
}

uint32 csHashCompute (char const* s)
{
  uint32 h = 0;
  while (*s != 0)
    h = rotate_bits_right_3(h) + *s++;
  return h;
}
