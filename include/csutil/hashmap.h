/*
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

#ifndef _CS_HASHMAP_H
#define _CS_HASHMAP_H

#include "csutil/csvector.h"

class csHashMap;
typedef int csHashKey;
typedef void* csHashObject;

/**
 * An element inside the hashmap (private element).
 */
struct csHashElement
{
  csHashKey key;
  csHashObject object;
};

/**
 * Subclass of csVector that will hold csHashElement
 * instances and can clean them up.
 */
class csHashBucket : public csVector
{
public:
  virtual bool FreeItem (csSome Item)
  {
    csHashElement* el = (csHashElement*)Item;
    delete el;
    return true;
  }
};

/**
 * Subclass of csVector to contain other csVectors and make
 * sure they are cleaned up.
 */
class csVectorVector : public csVector
{
public:
  virtual bool FreeItem (csSome Item)
  {
    csVector* vec = (csVector*)Item;
    delete vec;
    return true;
  }
};

/**
 * An iterator to iterate over elements in the hashmap.
 * When you have an open iterator you should not alter the
 * hashmap that this iterator emerged from. The only safe
 * operation that you can do is to call 'Delete' on this
 * iterator to delete one element from the map. The iterator
 * will correctly point to the next element then.
 */
class csHashIterator
{
  friend csHashMap;

private:
  /// Bucket we are iterating over.
  csHashBucket* bucket;
  /// Current index in bucket.
  int element_index;
  /// Next index in hashmap to test or -1 if at end.
  int next_bucket_index;
  /// Pointer to the hashmap.
  csHashMap* hash;

public:
  /// Is there a next element in this iterator?
  bool HasNext ();
  /// Get the next element.
  csHashObject Next ();
  /// Delete next element and fetches new one.
  void DeleteNext ();
};

/**
 * This is a general hashmap. You can put elements in this
 * map using a key.
 * Keys must not be unique. If a key is not unique then you
 * can iterate over all elements with the same key.
 */
class csHashMap
{
  friend csHashIterator;

private:
  /// A hashmap is implemented as a csVector of csVectors.
  csVectorVector buckets;
  /// Max size of this vector.
  int max_size;

public:
  /**
   * Constructor. The parameter for the constructor
   * is the initial size of the hashtable. The best
   * sizes are prime.
   * The map will grow dynamically if needed (@@@ Not implemented yet).
   */
  csHashMap (int size = 211);

  /**
   * Destructor. The objects referenced too in this hash
   * table will not be destroyed.
   */
  virtual ~csHashMap ();

  /**
   * Put an object in this map.
   */
  void Put (csHashKey key, csHashObject object);

  /**
   * Get an object from this map. Returns NULL if object
   * is not there. If there are multiple elements with
   * the same key then a random one will be returned.
   * Use GetIterator() to iterate over all elements with
   * the same key.
   */
  csHashObject Get (csHashKey key);

  /**
   * Get an iterator to iterate over all elements with the
   * given key. 'delete' this iterator when you don't want to
   * use it anymore. Note that you should not do changes on the
   * hashmap when you have open iterators. This function will
   * always return an iterator even if there are no elements with
   * this key.
   */
  csHashIterator* GetIterator (csHashKey key);

  /**
   * Get an iterator to iterate over all elements in this hashmap.
   * Note that you should not do changes on the hashmap when you have
   * open iterators.
   */
  csHashIterator* GetIterator ();

  /**
   * Delete all objects from this map with a given key.
   */
  void DeleteAll (csHashKey key);

  /**
   * Delete all objects from this map.
   */
  void DeleteAll ();
};

#endif //_CS_HASHMAP_H

