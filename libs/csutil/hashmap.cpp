/*
    Hash Map and hash support functions.
    Copyright (C) 2000 by Jorrit Tyberghein

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

#include <stdio.h>
#include <string.h>
#include "cssysdef.h"
#include "csutil/hashmap.h"

//-----------------------------------------------------------------------------

inline static csHashKey rotate_bits_right_3(csHashKey h)
{
  return (h >> 3) | (h << 29);
}

csHashKey csHashCompute(char const* s, int n)
{
  csHashKey h = 0;
  char const* slim = s + n;
  while (s < slim)
    h = rotate_bits_right_3(h) + *s++;
  return h;
}

csHashKey csHashCompute(char const* s)
{
  csHashKey h = 0;
  while (*s != 0)
    h = rotate_bits_right_3(h) + *s++;
  return h;
}

//-----------------------------------------------------------------------------

csHashIterator::csHashIterator (csHashMap *hm)
{
  hash = hm;
  bucket = NULL;
  element_index = 0;
  bucket_index = (uint32)-1;
  do_iterate_key = false;
  GotoNextElement ();
}

csHashIterator::csHashIterator (csHashMap *hm, csHashKey hkey)
{
  uint32 idx = hkey % hm->NumBuckets;

  hash = hm;
  bucket = hm->Buckets[idx]; // NULL if bucket is empty.
  element_index = -1;
  bucket_index = idx;
  key = hkey;
  do_iterate_key = true;
  GotoNextSameKey ();
}

bool csHashIterator::HasNext ()
{
  return bucket != NULL;
}

void csHashIterator::GotoNextElement ()
{
  element_index++;
  if (!bucket || element_index >= bucket->Length ())
  {
    // Next bucket.
    bucket_index++;
    uint32 const nbuckets = (uint32)hash->Buckets.Length();
    while (bucket_index < nbuckets && !hash->Buckets[bucket_index])
      bucket_index++;
    if (bucket_index >= nbuckets)
      bucket = NULL;	// The end
    else
    {
      bucket = hash->Buckets[bucket_index];
      element_index = 0;
    }
  }
}

void csHashIterator::GotoNextSameKey ()
{
  if (!bucket) return;
  element_index++;
  while (element_index < bucket->Length () &&
  	bucket->Get(element_index)->key != key)
  {
    element_index++;
  }
  if (element_index >= bucket->Length ()) bucket = NULL;
}

csHashObject csHashIterator::Next ()
{
  if (bucket == NULL) return NULL;
  csHashObject obj = ((csHashElement*)((*bucket)[element_index]))->object;
  if (do_iterate_key) GotoNextSameKey ();
  else GotoNextElement ();
  return obj;
}

void csHashIterator::DeleteNext ()
{
  // @@@ Not yet implemented.
}

//-----------------------------------------------------------------------------

csHashMap::csHashMap (uint32 size)
{
  NumBuckets = size;
  Buckets.SetLength (size);
  uint32 i;
  for (i = 0 ; i < size ; i++)
    Buckets[i] = NULL;
}

csHashMap::~csHashMap ()
{
  DeleteAll ();
}

void csHashMap::Put (csHashKey key, csHashObject object)
{
  uint32 idx = key % NumBuckets;
  if (!Buckets[idx]) Buckets[idx] = new csHashBucket ();
  csHashBucket* bucket = Buckets[idx];
  csHashElement* element = new csHashElement ();
  element->key = key;
  element->object = object;
  bucket->Push (element);
}

csHashObject csHashMap::Get (csHashKey key) const
{
  uint32 idx = key % NumBuckets;
  if (!Buckets[idx]) return NULL;
  csHashBucket* bucket = Buckets[idx];
  int i;
  for (i = 0 ; i < bucket->Length () ; i++)
  {
    csHashElement* element = bucket->Get(i);
    if (element->key == key) return element->object;
  }
  return NULL;
}

void csHashMap::DeleteAll (csHashKey key)
{
  uint32 idx = key % NumBuckets;
  if (!Buckets[idx]) return;
  csHashBucket* bucket = Buckets[idx];
  uint32 i;
  for (i = bucket->Length () ; i-- > 0 ; )
  {
    csHashElement* element = bucket->Get(i);
    if (element->key == key)
      bucket->Delete (i);
  }
}

void csHashMap::DeleteAll ()
{
  uint32 b;
  for (b = Buckets.Length () ; b-- > 0 ; )
  {
    delete Buckets[b];
    Buckets[b] = NULL;
  }
}

//-----------------------------------------------------------------------------

csHashSet::csHashSet (uint32 size) : map (size)
{
}

void csHashSet::Add (csHashObject object)
{
  if (In (object)) return;
  AddNoTest (object);
}

void csHashSet::AddNoTest (csHashObject object)
{
  csHashKey key = (csHashKey)object;
  map.Put (key, object);
}

bool csHashSet::In (csHashObject object)
{
  csHashKey key = (csHashKey)object;
  csHashIterator it (&map, key);
  while (it.HasNext ())
  {
    csHashObject obj = it.Next ();
    if (obj == object)
      return true;
  }
  return false;
}

void csHashSet::DeleteAll ()
{
  map.DeleteAll ();
}

void csHashSet::Delete (csHashObject)
{
}
