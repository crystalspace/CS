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

csGlobalHashIterator::csGlobalHashIterator (csHashMap *hm)
{
  hash = hm;
  chash = 0;
  cbucket = bucket = 0;
  bucket_len = 0;
  element_index = 0;
  bucket_index = (size_t)-1;
  nbuckets = hash->Buckets.Length();
  GotoNextElement ();
}

csGlobalHashIterator::csGlobalHashIterator (const csHashMap *hm)
{
  chash = hm;
  hash = 0;
  cbucket = bucket = 0;
  bucket_len = 0;
  element_index = 0;
  bucket_index = (size_t)-1;
  nbuckets = chash->Buckets.Length();
  GotoNextElementConst ();
}

bool csGlobalHashIterator::HasNext () const
{
  return bucket != 0 || cbucket != 0;
}

void csGlobalHashIterator::GotoNextElement ()
{
  element_index++;
  if (element_index >= (size_t)bucket_len)
  {
    // Next bucket.
    bucket_index++;
    while (bucket_index < nbuckets)
    {
      bucket = &hash->Buckets[bucket_index];
      bucket_len = bucket->Length ();
      if (bucket_len != 0)
      {
        element_index = 0;
	return;
      }
      bucket_index++;
    }
    bucket = 0;
  }
}

void csGlobalHashIterator::GotoNextElementConst ()
{
  element_index++;
  if (element_index >= bucket_len)
  {
    // Next bucket.
    bucket_index++;
    while (bucket_index < nbuckets)
    {
      cbucket = &chash->Buckets[bucket_index];
      bucket_len = cbucket->Length ();
      if (bucket_len != 0)
      {
        element_index = 0;
	return;
      }
      bucket_index++;
    }
    cbucket = 0;
  }
}

csHashObject csGlobalHashIterator::Next ()
{
  CS_ASSERT(bucket);
  csHashObject obj = ((*bucket)[element_index]).object;
  GotoNextElement ();
  return obj;
}

csHashObject csGlobalHashIterator::NextConst ()
{
  CS_ASSERT(cbucket);
  csHashObject obj = ((*cbucket)[element_index]).object;
  GotoNextElementConst ();
  return obj;
}

void csGlobalHashIterator::DeleteNext ()
{
  // @@@ Not yet implemented.
}

//-----------------------------------------------------------------------------

csHashIterator::csHashIterator (csHashMap *hm, csHashKey hkey)
{
  size_t idx = hkey % hm->NumBuckets;

  hash = hm;
  chash = 0;
  bucket = &(hm->Buckets[idx]);
  cbucket = 0;
  element_index = (size_t)-1;
  current_index = (size_t)-1;
  bucket_index = idx;
  key = hkey;
  GotoNextSameKey ();
}

csHashIterator::csHashIterator (const csHashMap *hm, csHashKey hkey)
{
  size_t idx = hkey % hm->NumBuckets;

  hash = 0;
  chash = hm;
  bucket = 0;
  cbucket = &(hm->Buckets[idx]);
  element_index = (size_t)-1;
  current_index = (size_t)-1;
  bucket_index = idx;
  key = hkey;
  GotoNextSameKeyConst ();
}

bool csHashIterator::HasNext () const
{
  if (bucket) return bucket->Length () > 0;
  else if (cbucket) return cbucket->Length () > 0;
  else return false;
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
  if (element_index >= bucket->Length ()) bucket = 0;
}

void csHashIterator::GotoNextSameKeyConst ()
{
  if (!cbucket) return;
  element_index++;
  while (element_index < cbucket->Length () &&
  	cbucket->Get(element_index).key != key)
  {
    element_index++;
  }
  if (element_index >= cbucket->Length ()) cbucket = 0;
}

csHashObject csHashIterator::Next ()
{
  CS_ASSERT(bucket);
  csHashObject obj = ((*bucket)[element_index]).object;
  current_index = element_index;
  GotoNextSameKey ();
  return obj;
}

csHashObject csHashIterator::NextConst ()
{
  CS_ASSERT(cbucket);
  csHashObject obj = ((*bucket)[element_index]).object;
  current_index = element_index;
  GotoNextSameKeyConst ();
  return obj;
}

void csHashIterator::DeleteNext ()
{
  // @@@ Not yet implemented.
}

//-----------------------------------------------------------------------------

size_t csHashMap::prime_table[] =
{
  53,         97,         193,       389,       769, 
  1543,       3079,       6151,      12289,     24593,
  49157,      98317,      196613,    393241,    786433,
  1572869,    3145739,    6291469,   12582917,  25165843,
  50331653,   100663319,  201326611, 402653189, 805306457,
  1610612741, 0
};

csHashMap::csHashMap (size_t size)
{
  size = FindNextPrime(size);
  NumBuckets = size;
  Buckets.SetLength (size, csHashBucket ());
  hash_elements = 0;
}

csHashMap::~csHashMap ()
{
  DeleteAll ();
}

size_t csHashMap::FindNextPrime (size_t num)
{
  size_t i = 0;
  size_t p = prime_table[i];
  while (p)
  {
    if (p >= num) return p;
    i++;
    p = prime_table[i];
  }
  return 0;
}

void csHashMap::ChangeBuckets (size_t newsize)
{
  Buckets.SetLength (newsize, csHashBucket ());
  size_t i;
  // Only go up to old size.
  size_t old_NumBuckets = NumBuckets;
  NumBuckets = newsize;
  for (i = 0 ; i < old_NumBuckets ; i++)
  {
    csHashBucket& bucket = Buckets[i];
    if (bucket.Length () == 0) continue;
    csHashBucket b;
    bucket.TransferTo (b);
    size_t bucket_len = b.Length ();
    size_t j;
    for (j = 0 ; j < bucket_len ; j++)
    {
      csHashElement& el = b[j];
      size_t new_idx =  el.key % NumBuckets;
      PutInternal (new_idx, el.key, el.object);
    }
  }
}

void csHashMap::PutInternal (size_t idx, csHashKey key,
  csHashObject object)
{
  csHashBucket& bucket = Buckets[idx];
  size_t i = bucket.Push (csHashElement ());
  bucket[i].key = key;
  bucket[i].object = object;
}

void csHashMap::Put (csHashKey key, csHashObject object)
{
  size_t idx = key % NumBuckets;
  PutInternal (idx, key, object);
  hash_elements++;
  if (NumBuckets < 20000UL && hash_elements > (NumBuckets*4))
    ChangeBuckets (FindNextPrime (NumBuckets*4));
}

csHashObject csHashMap::Get (csHashKey key) const
{
  size_t idx = key % NumBuckets;
  const csHashBucket& bucket = Buckets[idx];
  size_t i;
  size_t len = bucket.Length ();
  for (i = 0 ; i < len ; i++)
  {
    const csHashElement& element = bucket[i];
    if (element.key == key) return element.object;
  }
  return 0;
}

void csHashMap::Delete (csHashKey key, csHashObject object)
{
  size_t idx = key % NumBuckets;
  csHashBucket& bucket = Buckets[idx];
  size_t i;
  for (i = bucket.Length () ; i > 0 ; i--)
  {
    csHashElement& element = bucket.Get (i - 1);
    if (element.key == key && element.object == object)
    {
      bucket.DeleteIndex (i - 1);
      hash_elements--;
      break;
    }
  }
}

void csHashMap::DeleteAll (csHashKey key)
{
  size_t idx = key % NumBuckets;
  csHashBucket& bucket = Buckets[idx];
  size_t i;
  for (i = bucket.Length () ; i-- > 0 ; )
  {
    csHashElement& element = bucket.Get (i);
    if (element.key == key)
    {
      bucket.DeleteIndex (i);
      hash_elements--;
    }
  }
}

void csHashMap::DeleteAll ()
{
  size_t b;
  for (b = Buckets.Length () ; b-- > 0 ; )
  {
    Buckets[b].DeleteAll ();
  }
  hash_elements = 0;
}

void csHashMap::DumpStats ()
{
  size_t i;
  size_t count_null = 0;
  size_t count_empty_but_not_null = 0;
  size_t count_elements = 0;
  size_t max_elements = 0;
  for (i = 0 ; i < NumBuckets ; i++)
  {
    csHashBucket& bucket = Buckets[i];
    if (bucket.Length () == 0)
    {
      count_empty_but_not_null++;
    }
    else
    {
      count_elements += bucket.Length ();
      if (bucket.Length () > max_elements)
	max_elements = bucket.Length ();
    }
  }
  printf ("buckets=%lu null=%lu empty=%lu el=%lu/%lu max_el=%lu avg_el=%g\n",
    (unsigned long)NumBuckets,
    (unsigned long)count_null,
    (unsigned long)count_empty_but_not_null,
    (unsigned long)count_elements,
    (unsigned long)hash_elements,
    (unsigned long)max_elements,
    float (count_elements) / float (NumBuckets));
}

//-----------------------------------------------------------------------------

csHashSet::csHashSet (unsigned int size) : map (size)
{
}

void csHashSet::Add (csHashObject object)
{
  if (In (object)) return;
  AddNoTest (object);
}

void csHashSet::AddNoTest (csHashObject object)
{
  // @@@ FIXME 64bit: pointer truncation
  csHashKey key = (csHashKey)object;
  map.Put (key, object);
}

bool csHashSet::In (csHashObject object)
{
  // @@@ FIXME 64bit: pointer truncation
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
  // @@@ FIXME 64bit: pointer truncation
  csHashKey key = (csHashKey)object;
  map.Delete (key, object);
}
