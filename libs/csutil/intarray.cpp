/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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
#include "csutil/intarray.h"

int csIntArray::Find (int val) const
{
  int i;
  for (i=0; i<Length (); i++)
    if ((*this) [i] == val)
      return i;
  return -1;
}

void csIntArray::PushFill (int val, int n)
{
  int i;
  for (i=0; i<n; i++)
    Push (val);
}

void csIntArray::PushIdentityMapping (int n, int offset)
{
  int i;
  for (i=0; i<n; i++)
    Push (offset + i);
}

csIntArray *csIntArray::CreateUniformMapping (int val, int n)
{
  csIntArray *a = new csIntArray ();
  a->PushFill (val, n);
  return a;
}

csIntArray *csIntArray::CreateIdentityMapping (int n, int offset)
{
  csIntArray *a = new csIntArray ();
  a->PushIdentityMapping (n, offset);
  return a;
}

