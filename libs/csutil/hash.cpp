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
#include "cssysdef.h"
#include "csutil/hash.h"

unsigned int csHashCompute (char const* s, int n)
{
  unsigned int h = 0;
  const char* end = s + n;
  for(const char* c = s; c != end; ++c)
    h = ((h << 5) + h) + *c;

  return h;
}

unsigned int csHashCompute (char const* s)
{
  // based on D.J. Bernsteins algo
  unsigned int h = 0;
  for(const char* c = s; *c != 0; ++c)
    h = ((h << 5) + h) + *c;
  
  return h;
}
