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
  current_bucket = NULL;
  current_index = -1;
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
    while (bucket_index < nbuckets && (!hash->Buckets[bucket_index]
	    || hash->Buckets[bucket_index]->Length()==0))
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
  	bucket->Get(element_index).key != key)
  {
    element_index++;
  }
  if (element_index >= bucket->Length ()) bucket = NULL;
}

csHashObject csHashIterator::Next ()
{
  if (bucket == NULL) return NULL;
  csHashObject obj = ((*bucket)[element_index]).object;
  current_index = element_index;
  current_bucket = bucket;
  if (do_iterate_key) GotoNextSameKey ();
  else GotoNextElement ();
  return obj;
}

void csHashIterator::DeleteNext ()
{
  // @@@ Not yet implemented.
}

//-----------------------------------------------------------------------------

uint32 csHashMap::prime_table[] =
{
  53,         97,         193,       389,       769, 
  1543,       3079,       6151,      12289,     24593,
  49157,      98317,      196613,    393241,    786433,
  1572869,    3145739,    6291469,   12582917,  25165843,
  50331653,   100663319,  201326611, 402653189, 805306457,
  1610612741, 0
};

csHashMap::csHashMap (uint32 size)
{
  NumBuckets = size;
  Buckets.SetLength (size);
  hash_elements = 0;
}

csHashMap::~csHashMap ()
{
  DeleteAll ();
}

uint32 csHashMap::FindLargerPrime (uint32 num)
{
  int i = 0;
  uint32 p = prime_table[i];
  while (p)
  {
    if (p > num) return p;
    i++;
    p = prime_table[i];
  }
  return 0;
}

void csHashMap::ChangeBuckets (uint32 newsize)
{
//printf ("Extend from %d to %d (hash_elements=%d)\n", NumBuckets, newsize, hash_elements);
  Buckets.SetLength (newsize);
  int i;
  // Only go up to old size.
  uint32 old_NumBuckets = NumBuckets;
  NumBuckets = newsize;
  for (i = 0 ; i < old_NumBuckets ; i++)
  {
    csHashBucket* bucket = Buckets[i];
    if (!bucket) continue;
    csHashBucket b;
    bucket->TransferTo (b);
    int bucket_len = b.Length ();
    int j;
    for (j = 0 ; j < bucket_len ; j++)
    {
      csHashElement& el = b[j];
      uint32 new_idx =  el.key % NumBuckets;
      PutInternal (new_idx, el.key, el.object);
    }
    if (bucket->Length () == 0) Buckets.Put (i, NULL);
  }
}

void csHashMap::PutInternal (uint32 idx, csHashKey key, csHashObject object)
{
  csHashBucket* bucket = Buckets[idx];
  if (!bucket)
  {
    bucket = new csHashBucket ();
    Buckets.Put (idx, bucket);
  }

  int i = bucket->Push (csHashElement ());
  (*bucket)[i].key = key;
  (*bucket)[i].object = object;
}

void csHashMap::Put (csHashKey key, csHashObject object)
{
  uint32 idx = key % NumBuckets;
  PutInternal (idx, key, object);
  hash_elements++;
  if (NumBuckets < 10000 && hash_elements > NumBuckets*4)
    ChangeBuckets (FindLargerPrime (NumBuckets*4));
}

csHashObject csHashMap::Get (csHashKey key) const
{
  uint32 idx = key % NumBuckets;
  csHashBucket* bucket = Buckets[idx];
  if (!bucket) return NULL;
  int i;
  for (i = 0 ; i < bucket->Length () ; i++)
  {
    csHashElement& element = bucket->Get(i);
    if (element.key == key) return element.object;
  }
  return NULL;
}

void csHashMap::Delete (csHashKey key, csHashObject object)
{
  uint32 idx = key % NumBuckets;
  csHashBucket* bucket = Buckets[idx];
  if (!bucket) return;
  int i;
  for (i = bucket->Length ()-1 ; i >= 0 ; i--)
  {
    csHashElement& element = bucket->Get(i);
    if (element.key == key && element.object == object)
    {
      bucket->DeleteIndex (i);
      hash_elements--;
      break;
    }
  }
}

void csHashMap::DeleteAll (csHashKey key)
{
  uint32 idx = key % NumBuckets;
  csHashBucket* bucket = Buckets[idx];
  if (!bucket) return;
  uint32 i;
  for (i = bucket->Length () ; i-- > 0 ; )
  {
    csHashElement& element = bucket->Get(i);
    if (element.key == key)
    {
      bucket->DeleteIndex (i);
      hash_elements--;
    }
  }
}

void csHashMap::DeleteAll ()
{
  uint32 b;
  for (b = Buckets.Length () ; b-- > 0 ; )
  {
    Buckets.Put (b, NULL);
  }
  hash_elements = 0;
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

void csHashSet::Delete (csHashObject object)
{
  csHashKey key = (csHashKey)object;
  map.Delete (key, object);
}

