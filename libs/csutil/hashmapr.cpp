/*
    Copyright (C) 2002 by Mathew Sutcliffe

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
#include "csutil/hashmapr.h"
#include "csutil/hashmap.h"

csHashMapReversible::csHashMapReversible (uint32 size)
  : csHashMap (size)
{
  Reverse = new csHashMap (size);
  HalfReverse = new csHashMap (size);
}

csHashMapReversible::~csHashMapReversible ()
{
  delete Reverse;
  delete HalfReverse;
}

void csHashMapReversible::Put (const char *key, csHashObject object)
{
  Reverse->Put ((csHashKey) object, (csHashObject) key);

  csHashKey keynum = csHashCompute (key);
  HalfReverse->Put (keynum, (csHashObject) key);

  csHashMap::Put (keynum, object);
}

const char* csHashMapReversible::GetKey (csHashObject value)
{
  return (char *) Reverse->Get ((csHashKey) value);
}

const char* csHashMapReversible::GetKey (csHashKey key)
{
  return (char *) HalfReverse->Get (key);
}

csHashIteratorReversible::csHashIteratorReversible (csHashMapReversible *r)
  : csHashIterator (r)
{
  hashr = r;
}

const char* csHashIteratorReversible::GetKey ()
{
  return hashr->GetKey ( ((csHashElement *)((*bucket)[element_index])) ->key);
}

