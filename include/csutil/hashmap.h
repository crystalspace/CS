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

#ifndef __CS_HASHMAP_H__
#define __CS_HASHMAP_H__

#include "csutil/csvector.h"
class csHashMap;

/// An opaque hash key.
typedef uint32 csHashKey;
/// An opaque hash value.
typedef void* csHashObject;

/// Compute a hash key for a null-terminated string.
csHashKey csHashCompute(char const*);
/// Compute a hash key for a string of a given length.
csHashKey csHashCompute(char const*, int length);

/**
 * An element inside the hashmap (private element).
 */
struct csHashElement
{
  csHashKey key;
  csHashObject object;
};

/// a vector of csHashElements
CS_DECLARE_TYPED_VECTOR (csHashBucket, csHashElement);
/// a vector of csHashBuckets
CS_DECLARE_TYPED_VECTOR (csHashBucketVector, csHashBucket);

/**
 * An iterator to iterate over elements in the hashmap.
 * When you have an open iterator you should not alter the
 * hashmap that this object iterates over. The only safe
 * operation that you can do is to call 'Delete' on this
 * iterator to delete one element from the map. The iterator
 * will correctly point to the next element then.
 */
class csHashIterator
{
  friend class csHashMap;

private:
  /// Current bucket we are iterating over. NULL if no more elements.
  csHashBucket* bucket;
  /// Current index in bucket.
  int element_index;
  /// Current bucket index in hashmap.
  uint32 bucket_index;
  /// If true we are iterating over a key.
  bool do_iterate_key;
  /// Key to iterate over.
  csHashKey key;
  /// Pointer to the hashmap.
  csHashMap* hash;

private:
  /// Go to next element with same key.
  void GotoNextSameKey ();
  /// Go to next element.
  void GotoNextElement ();

public:

  /**
   * Constructor for an iterator to iterate over all elements in a hashmap.
   * Note that you should not do changes on the hashmap when you have
   * open iterators.
   */
  csHashIterator (csHashMap* hash);
  /**
   * Constructor for an iterator to iterate over all elements with the
   * given key. Note that you should not do changes on the hashmap when
   * you have open iterators.
   */
  csHashIterator (csHashMap* hash, csHashKey Key);

  /// Is there a next element in this iterator?
  bool HasNext ();
  /// Get the next element.
  csHashObject Next ();
  /**
   * Delete next element and fetches new one.
   * @@@ Not implemented yet!
   */
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
  friend class csHashIterator;

private:
  /// the list of buckets
  csHashBucketVector Buckets;
  /// Max size of this vector.
  uint32 NumBuckets;

public:
  /**
   * Constructor. The parameter for the constructor
   * is the initial size of the hashtable. The best
   * sizes are prime.<br>
   * Here are a few useful primes: 127, 211, 431, 701,
   * 1201, 1559, 3541, 8087, 12263, 25247, 36923,
   * 50119, 70951, 90313, 104707, ...
   * For a bigger list go to www.utm.edu/research/primes.
   * The map will grow dynamically if needed (@@@ Not implemented yet).
   */
  csHashMap (uint32 size = 211);

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
   * Use an iterator to iterate over all elements with
   * the same key.
   */
  csHashObject Get (csHashKey key) const;

  /**
   * Delete all objects from this map with a given key.
   */
  void DeleteAll (csHashKey key);

  /**
   * Delete all objects from this map.
   */
  void DeleteAll ();
};

/**
 * This class implements a basic set for objects.
 * You can basicly use this to test for the occurance
 * of some object quickly.
 */
class csHashSet
{
private:
  csHashMap map;

public:
  /**
   * Construct a new empty set.
   * The given size will be given to the hasmap.
   */
  csHashSet (uint32 size = 211);

  /**
   * Add an object to this set.
   * This will do nothing is the object is already here.
   */
  void Add (csHashObject object);

  /**
   * Add an object to this set.
   * This function does not test if the object is already
   * there. This is used for efficiency reasons. But use
   * with care!
   */
  void AddNoTest (csHashObject object);

  /**
   * Test if an object is in this set.
   */
  bool In (csHashObject object);

  /**
   * Delete all elements in the set.
   */
  void DeleteAll ();

  /**
   * Delete an object from the set. This function
   * does nothing if the object is not in the set.
   * @@@ Not implemented yet!
   */
  void Delete (csHashObject object);

  /// Return the hash map for this hash set
  inline csHashMap *GetHashMap () {return &map;}
};

#endif //__CS_HASHMAP_H__

