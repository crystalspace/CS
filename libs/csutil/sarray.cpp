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
#include "csutil/sarray.h"

csStaticArray::csStaticArray (int s)
{
  Map = NULL;
  Size = 0;
  Alloc (s);
}

csStaticArray::~csStaticArray ()
{
  Clear ();
}

void csStaticArray::Clear (bool DeleteOld)
{
  if (Map && DeleteOld) DeleteArray (Map);
  Map = NULL; Size = 0;
}

void csStaticArray::Alloc (int s, bool DeleteOld)
{
  if (Size == s && DeleteOld) return;
  Clear (DeleteOld);
  if (s>0) { Map = AllocateArray (s); Size = s; }
}

void csStaticArray::ReAlloc (int s)
{
  if (Size == s) return;
  if (s<1) { Clear (); return; }
  void *OldMap = Map;
  Map = AllocateArray (s);
  CopyArray (Map, OldMap, s>Size ? Size : s);
  DeleteArray (OldMap);
  Size = s;
}

void csStaticArray::Copy (const csStaticArray *other, bool DeleteOld)
{
  Alloc (other->Size, DeleteOld);
  CopyArray (Map, other->Map, Size);
}

void csStaticArray::Copy (void *NewData, int NewSize, bool DeleteOld)
{
  Alloc (NewSize, DeleteOld);
  CopyArray (Map, NewData, Size);
}
