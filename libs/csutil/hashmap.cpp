/*
    Hash Map.
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
#include "cssysdef.h"
#include "csutil/hashmap.h"

//---------------------------------------------------------------------------

bool csHashIterator::HasNext ()
{
  return bucket != NULL;
}

csHashObject csHashIterator::Next ()
{
  if (bucket == NULL) return NULL;
  csHashObject obj = ((csHashElement*)((*bucket)[element_index]))->object;
  element_index++;
  if (element_index >= bucket->Length ())
  {
    // Next bucket.
    if (next_bucket_index == -1)
      bucket = NULL;
    else
    {
      bucket = (csHashBucket*)hash->buckets[next_bucket_index];
      element_index = 0;
      next_bucket_index++;
      while (next_bucket_index < hash->buckets.Length () &&
      		!hash->buckets[next_bucket_index])
	next_bucket_index++;
      if (next_bucket_index == hash->buckets.Length ())
        next_bucket_index = -1;
    }
  }
  return obj;
}

void csHashIterator::DeleteNext ()
{
  // @@@ Not yet implemented.
}

//---------------------------------------------------------------------------

csHashMap::csHashMap (int size)
{
  max_size = size;
  buckets.SetLength (max_size);
  int i;
  for (i = 0 ; i < max_size ; i++)
    buckets[i] = NULL;
}

csHashMap::~csHashMap ()
{
}

void csHashMap::Put (csHashKey key, csHashObject object)
{
  int idx = key % max_size;
  if (!buckets[idx]) buckets[idx] = new csHashBucket ();
  csHashBucket& bucket = *(csHashBucket*)buckets[idx];
  csHashElement* element = new csHashElement ();
  element->key = key;
  element->object = object;
  bucket.Push (element);
}

csHashObject csHashMap::Get (csHashKey key)
{
  int idx = key % max_size;
  if (!buckets[idx]) return NULL;
  csHashBucket& bucket = *(csHashBucket*)buckets[idx];
  int i;
  for (i = 0 ; i < bucket.Length () ; i++)
  {
    csHashElement* element = (csHashElement*)bucket[i];
    if (element->key == key) return element->object;
  }
  return NULL;
}

csHashIterator* csHashMap::GetIterator (csHashKey key)
{
  csHashIterator* iterator = new csHashIterator ();

  int idx = key % max_size;

  iterator->bucket = (csHashBucket*)buckets[idx]; // Will be NULL if bucket is empty.
  iterator->element_index = 0;
  iterator->next_bucket_index = -1;
  iterator->hash = this;

  return iterator;
}

csHashIterator* csHashMap::GetIterator ()
{
  csHashIterator* iterator = new csHashIterator ();

  iterator->bucket = (csHashBucket*)buckets[0];
  iterator->element_index = 0;
  iterator->next_bucket_index = 1;
  iterator->hash = this;

  return iterator;
}

void csHashMap::DeleteAll (csHashKey key)
{
  int idx = key % max_size;
  if (!buckets[idx]) return;
  csHashBucket& bucket = *(csHashBucket*)buckets[idx];
  int i;
  for (i = bucket.Length () ; i >= 0 ; i--)
  {
    csHashElement* element = (csHashElement*)bucket[i];
    if (element->key == key)
      bucket.Delete (i);
  }
}

void csHashMap::DeleteAll ()
{
  int b;
  for (b = buckets.Length () ; b >= 0 ; b--)
    buckets.Delete (b);
}

