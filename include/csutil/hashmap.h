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

#if 0 // let's not deprecate just yet :)
#ifndef CS_COMPILER_MSVC
# warning Use of csHashMap is deprecated. Please csHash instead.
#endif
#endif

#include "csextern.h"
#include "parray.h"
#include "array.h"
// For csHashCompute() which used to be declared here.
#include "hash.h"

class csHashMapReversible;
class csHashIteratorReversible;

class csHashMap;

#if (CS_PROCESSOR_SIZE == 32)
# if (_MSC_VER >= 1300)
  /*
   * Silence VC7 64bit warning.
   */
  typedef unsigned int __w64 csHashKey;
# else
  /// An opaque hash key.
  typedef unsigned int csHashKey;
# endif
#else
  /*
   * At some places, pointers are casted to csHashKey. Work around truncation
   * problems by forcing csHashKey to at least 64bit on 64bit machines.
   */
  typedef uint64 csHashKey;
#endif
/// An opaque hash value.
typedef void* csHashObject;

/**
 * An element inside the hashmap (private element).
 */
struct csHashElement
{
  csHashKey key;
  csHashObject object;
};

/// a vector of csHashElements
typedef csArray<csHashElement> csHashBucket;
/// a vector of csHashBuckets
typedef csArray<csHashBucket> csHashBucketVector;

/**
 * An iterator to iterate over all elements in the hashmap.
 * When you have an open iterator you should not alter the
 * hashmap that this object iterates over. The only safe
 * operation that you can do is to call 'Delete' on this
 * iterator to delete one element from the map. The iterator
 * will correctly point to the next element then.
 */
class CS_CRYSTALSPACE_EXPORT csGlobalHashIterator
{
  friend class csHashMap;
  friend class csGlobalHashIteratorReversible;

private:
  /// Next bucket we are iterating over. 0 if no more elements.
  csHashBucket* bucket;
  /// Const version of bucket.
  const csHashBucket* cbucket;
  /// index of next item in bucket.
  size_t element_index;
  /// Current bucket index in hashmap.
  size_t bucket_index;
  /// Current number of items in bucket.
  size_t bucket_len;
  /// Number of buckets.
  size_t nbuckets;
  /// Pointer to the hashmap.
  csHashMap* hash;
  /// Const version of hash.
  const csHashMap* chash;

private:
  /// Go to next element.
  void GotoNextElement ();

  /// Const version of GotoNextElement().
  void GotoNextElementConst ();

public:
  /**
   * Constructor for an iterator to iterate over all elements in a hashmap.
   * Note that you should not do changes on the hashmap when you have
   * open iterators.
   */
  csGlobalHashIterator (csHashMap* hash);

  /**
   * Construct from a const hashmap. Can do everything except DeleteNext().
   */
  csGlobalHashIterator (const csHashMap* hash);

  /// Is there a next element in this iterator?
  bool HasNext () const;
  /// Get the next element.
  csHashObject Next ();
  /// deprecated. (matze: this function is totally pointless)
  csHashObject NextConst ();
  /**
   * Delete next element and fetches new one.
   * @@@ Not implemented yet!
   */
  void DeleteNext ();
};

/**
 * An iterator to iterate over elements in the hashmap.
 * When you have an open iterator you should not alter the
 * hashmap that this object iterates over. The only safe
 * operation that you can do is to call 'Delete' on this
 * iterator to delete one element from the map. The iterator
 * will correctly point to the next element then.
 */
class CS_CRYSTALSPACE_EXPORT csHashIterator
{
  friend class csHashMap;
  friend class csHashIteratorReversible;

private:
  /// Next bucket we are iterating over. 0 if no more elements.
  csHashBucket* bucket;
  /// Const version of bucket
  const csHashBucket* cbucket;
  /// index of next item in bucket.
  size_t element_index;
  /// Current index in bucket.
  size_t current_index;
  /// Current bucket index in hashmap.
  size_t bucket_index;
  /// Key to iterate over.
  csHashKey key;
  /// Pointer to the hashmap.
  csHashMap* hash;
  /// Const version of hash.
  const csHashMap* chash;

private:
  /// Go to next element with same key.
  void GotoNextSameKey ();

  /// Const version of GotoNextSameKey().
  void GotoNextSameKeyConst ();

public:
  /**
   * Constructor for an iterator to iterate over all elements with the
   * given key. Note that you should not do changes on the hashmap when
   * you have open iterators.
   */
  csHashIterator (csHashMap* hash, csHashKey Key);

  /**
   * Construct from a const hashmap. Can do everything except DeleteNext().
   */
  csHashIterator (const csHashMap* hash, csHashKey Key);

  /// Is there a next element in this iterator?
  bool HasNext () const;
  /// Get the next element.
  csHashObject Next ();
  /// deprecated. (Matze: this function is totally pointless)
  csHashObject NextConst ();
  /**
   * Delete next element and fetches new one.
   * @@@ Not implemented yet!
   */
  void DeleteNext ();
};

/**
 * This is a general hashmap. You can put elements in this
 * map using a key.
 * Deprecated. Use csHash or csGrowingHash instead.
 * Keys must not be unique. If a key is not unique then you
 * can iterate over all elements with the same key.
 */
class CS_CRYSTALSPACE_EXPORT csHashMap
{
  friend class csHashIterator;
  friend class csGlobalHashIterator;
  friend class csHashMapReversible;

private:
  /// the list of buckets
  csHashBucketVector Buckets;
  /// Max size of this vector.
  size_t NumBuckets;
  /// Number of elements in hash (to detect when to increase vector size).
  size_t hash_elements;

  /// Reorganize the hashmap with a different size of buckets.
  void ChangeBuckets (size_t newsize);

  /**
   * Put an object in the bucket vector.
   */
  void PutInternal (size_t idx, csHashKey key, csHashObject object);


  /// Find a prime number bigger then the given input number.
  static size_t FindNextPrime (size_t num);

public:
  static size_t prime_table[];

  /**
   * Constructor. The parameter for the constructor
   * is the initial size of the hashtable. The best
   * sizes are prime.<br>
   * Here are a few useful primes: 127, 211, 431, 701,
   * 1201, 1559, 3541, 8087, 12263, 25247, 36923,
   * 50119, 70951, 90313, 104707, ...
   * For a bigger list go to www.utm.edu/research/primes.
   * The map will grow dynamically if needed.
   */
  csHashMap (size_t size = 53);

  /**
   * Destructor. The objects referenced too in this hash
   * table will not be destroyed.
   */
  virtual ~csHashMap ();

  /**
   * Put an object in this map.
   * Use the csHashCompute() function to get a pseudo-unique numeric
   * key from a string.
   */
  void Put (csHashKey key, csHashObject object);

  /**
   * Get an object from this map. Returns 0 if object
   * is not there. If there are multiple elements with
   * the same key then a random one will be returned.
   * Use an iterator to iterate over all elements with
   * the same key.
   */
  csHashObject Get (csHashKey key) const;

  /**
   * Delete the given key/object from the map.
   * This function will only delete the object once. If multiple
   * 'Put''s are done with the same object then this function will
   * only delete one of them.
   */
  void Delete (csHashKey key, csHashObject object);

  /**
   * Delete all objects from this map with a given key.
   */
  void DeleteAll (csHashKey key);

  /**
   * Delete all objects from this map.
   */
  void DeleteAll ();

  /**
   * Dump statistics about bucket quality.
   */
  void DumpStats ();
};

/**
 * This class implements a basic set for objects.
 * You can basically use this to test for the occurrence
 * of some object quickly.
 */
class CS_CRYSTALSPACE_EXPORT csHashSet
{
private:
  csHashMap map;

public:
  /**
   * Construct a new empty set.
   * The given size will be given to the hasmap.
   */
  csHashSet (unsigned int size = 211);

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
   */
  void Delete (csHashObject object);

  /// Return the hash map for this hash set
  inline csHashMap *GetHashMap () {return &map;}
};

#endif // __CS_HASHMAP_H__

