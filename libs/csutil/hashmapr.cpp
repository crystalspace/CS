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

csHashMapReversible::csHashMapReversible (uint32 size)
  : csHashMap (size)
{
  Reverse = new csHashMap (size);
  HalfReverse = new csHashMap (size);
}

csHashMapReversible::csHashMapReversible (const csHashMapReversible& m) :
	csHashMap (m)
{
  Reverse = new csHashMap (*m.Reverse);
  HalfReverse = new csHashMap (*m.HalfReverse);
}

csHashMapReversible& csHashMapReversible::operator= (
		const csHashMapReversible& other)
{
  if (this == &other) return *this;
  delete Reverse;
  delete HalfReverse;
  Reverse = new csHashMap (*other.Reverse);
  HalfReverse = new csHashMap (*other.HalfReverse);
  return *this;
}

csHashMapReversible::~csHashMapReversible ()
{
  delete Reverse;
  delete HalfReverse;
}

void csHashMapReversible::Put (const char *key, csHashObject object)
{
  // @@@ FIXME 64bit: pointer truncation
  Reverse->Put ((csHashKey) object, (csHashObject) key);

  csHashKey keynum = csHashCompute (key);
  HalfReverse->Put (keynum, (csHashObject) key);

  csHashMap::Put (keynum, object);
}

const char* csHashMapReversible::GetKey (csHashObject value) const
{
  // @@@ FIXME 64bit: pointer truncation
  return (char *) Reverse->Get ((csHashKey) value);
}

const char* csHashMapReversible::GetKey (csHashKey key) const
{
  return (char *) HalfReverse->Get (key);
}

csHashObject csHashMapReversible::Get (const char* key) const
{
  csHashKey keynum = csHashCompute (key);
  return csHashMap::Get (keynum);
}

void csHashMapReversible::Delete (const char* key, csHashObject object)
{
  csHashKey keynum = csHashCompute (key);
  const char* myKey = (const char*)HalfReverse->Get (keynum);
  if (myKey == 0) return; 
  csHashMap::Delete (keynum, object);
  HalfReverse->Delete (keynum, (csHashObject)myKey);
  // @@@ FIXME 64bit: pointer truncation
  Reverse->Delete ((csHashKey)object, (csHashObject)myKey);
}

//----------------------------------------------------------------------------

csHashIteratorReversible::csHashIteratorReversible (csHashMapReversible *r,
  csHashKey k) : csHashIterator (r, k)
{
  hashr = r;
  iterr = 0;
}

csHashIteratorReversible::csHashIteratorReversible (csHashMapReversible *r,
  const char *k) : csHashIterator (r, csHashCompute (k))
{
  hashr = r;
  iterr = k;
}

csHashObject csHashIteratorReversible::Next ()
{
  if (! iterr) return csHashIterator::Next ();

  csHashObject obj;
  while ((obj = csHashIterator::Next ()))
    if (strcmp (GetKey (), iterr) == 0) return obj;

  return 0;
}

const char* csHashIteratorReversible::GetKey () const
{
  if ((bucket != 0) && (current_index >= 0)
  	&& (current_index <= bucket->Length())) 
    return hashr->GetKey ( ((*bucket) [current_index]) .key);
  else
    return 0;
}

//----------------------------------------------------------------------------

csGlobalHashIteratorReversible::csGlobalHashIteratorReversible (
	csHashMapReversible *r) : csGlobalHashIterator (r)
{
  hashr = r;
}

csHashObject csGlobalHashIteratorReversible::Next ()
{
  if (bucket == 0) return 0;
  csHashObject obj = ((*bucket)[element_index]).object;
  current_index = element_index;
  current_bucket = bucket;
  GotoNextElement ();
  return obj;
}

const char* csGlobalHashIteratorReversible::GetKey () const
{
  if ((current_bucket != 0) && (current_index >= 0)
  	&& (current_index <= current_bucket->Length())) 
    return hashr->GetKey ( ((*current_bucket)
    	[current_index]) .key);
  else
    return 0;
}

