/*
    Copyright (C) 2003 by Mat Sutcliffe <oktal@gmx.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_UTIL_HASH_H__
#define __CS_UTIL_HASH_H__

/**\file
 * A generic hash table
 */

#include "csextern.h"
#include "array.h"

/**
 * Compute a hash key for a null-terminated string.
 *
 * Note that these keys are non-unique; some dissimilar strings may generate
 * the same key. For unique keys, see csStringSet.
 */
CS_CRYSTALSPACE_EXPORT unsigned int csHashCompute (char const*);

/**
 * Compute a hash key for a string of a given length.
 *
 * Note that these keys are non-unique; some dissimilar strings may generate
 * the same key. For unique keys, see csStringSet.
 */
CS_CRYSTALSPACE_EXPORT unsigned int csHashCompute (char const*, size_t length);

/**
 * A hash key handler for integral types and types that can be casted to such.
 */
template <class T>
class csIntegralHashKeyHandler
{
public:
  static unsigned int ComputeHash (const T& key)
  {
#if (CS_PROCESSOR_SIZE == 32)
#if (_MSC_VER >= 1300)
    /*
      VC7 with 64bit warnings complains about a pointer truncation when T is
      a pointer. This isn't serious (as csHash<> does not rely on the hash of
      a key alone to retrieve a value). The __w64 causes VC7 to not emit that
      warning (on 32bit compilers at least).
     */
    return (unsigned int __w64)key;  
#else
    return (unsigned int)key;  
#endif
#else
    // Cast to uint64 first to avoid compiler warnings about truncation.
    return (unsigned int)((uint64)key);
#endif
  }

  static bool CompareKeys (const T& key1, const T& key2)
  {
    return (key1 == key2);
  }
};

/**
 * A generic hash table class,
 * which grows dynamically and whose buckets are unsorted arrays.
 */
template <class T, class K = unsigned int, 
  class KeyHandler = csIntegralHashKeyHandler<K> > 
class csHash
{
protected:
  struct Element
  {
    const K key;
    T value;

    Element (const K& key0, const T &value0) : key (key0), value (value0) {}
    Element (const Element &other) : key (other.key), value (other.value) {}
  };
  csArray< csArray<Element> > Elements;

  size_t Modulo;

private:
  size_t InitModulo;
  size_t GrowRate;
  size_t MaxSize;
  size_t Size;

  void Grow ()
  {
    static const size_t Primes[] =
    {
      53,         97,         193,       389,       769, 
      1543,       3079,       6151,      12289,     24593,
      49157,      98317,      196613,    393241,    786433,
      1572869,    3145739,    6291469,   12582917,  25165843,
      50331653,   100663319,  201326611, 402653189, 805306457,
      1610612741, 0
    };

    const size_t *p;
    size_t elen = Elements.Length ();
    for (p = Primes; *p && *p <= elen; p++) ;
    Modulo = *p;
    CS_ASSERT (Modulo);

    Elements.SetLength(Modulo, csArray<Element>(0, MIN(Modulo / GrowRate, 8)));

    for (size_t i = 0; i < elen; i++)
    {
      csArray<Element>& src = Elements[i];
      size_t slen = src.Length ();
      for (size_t j = slen; j > 0; j--)
      {
        const Element& srcElem = src[j - 1];
        csArray<Element>& dst = 
	  Elements.Get (KeyHandler::ComputeHash (srcElem.key) % Modulo);
        if (&src != &dst)
        {
          dst.Push (srcElem);
          src.DeleteIndex (j - 1);
        }
      }
    }
  }

public:
  /**
   * Construct a hash table with an array of the given size,
   * which for optimisation reasons should be a prime number.
   * 
   * Grow_rate is the rate at which the hash table grows:
   * Size doubles once there are size/grow_rate collisions.
   * It will not grow after it reaches max_size.
   *
   * Here are a few primes: 7, 11, 19, 29, 59, 79, 101, 127, 151, 199, 251,
   * 307, 401, 503, 809, 1009, 1499, 2003, 3001, 5003, 12263, 25247, 36923,
   * 50119, 70951, 90313, 104707.
   *
   * For a bigger list go to http://www.utm.edu/research/primes/
   */
  csHash (size_t size = 23, size_t grow_rate = 5, size_t max_size = 20000)
    : Elements (size), Modulo (size), InitModulo (size),
      GrowRate (MIN (grow_rate, size)), MaxSize (max_size), Size (0)
  {
    Elements.SetLength (size, csArray<Element> (0, MIN (size / GrowRate, 8)));
  }

  /// Copy constructor.
  csHash (const csHash<T> &o) : Elements (o.Elements),
    Modulo (o.Modulo), InitModulo (o.InitModulo),
    GrowRate (o.GrowRate), MaxSize (o.MaxSize), Size (o.Size) {}

  /**
   * Add an element to the hash table.
   * \remarks If `key' is already present, does NOT replace the existing value,
   *   but merely adds `value' as an additional value of `key'. To retrieve all
   *   values for a given key, use GetAll(). If you instead want to replace an
   *   existing value for 'key', use PutUnique().
   */
  void Put (const K& key, const T &value)
  {
    csArray<Element> &values = 
      Elements[KeyHandler::ComputeHash (key) % Modulo];
    values.Push (Element (key, value));
    Size++;
    if (values.Length () > Elements.Length () / GrowRate
     && Elements.Length () < MaxSize) Grow ();
  }

  /// Get all the elements with the given key, or empty if there are none.
  csArray<T> GetAll (const K& key) const
  {
    const csArray<Element> &values = 
      Elements[KeyHandler::ComputeHash (key) % Modulo];
    csArray<T> ret (values.Length () / 2);
    const size_t len = values.Length ();
    for (size_t i = 0; i < len; ++i)
    {
      const Element& v = values[i];
      if (KeyHandler::CompareKeys (v.key, key)) 
	ret.Push (v.value);
    }
    return ret;
  }

  /// Add an element to the hash table, overwriting if the key already exists.
  void PutUnique (const K& key, const T &value)
  {
    csArray<Element> &values = 
      Elements[KeyHandler::ComputeHash (key) % Modulo];
    const size_t len = values.Length ();
    for (size_t i = 0; i < len; ++i)
    {
      Element& v = values[i];
      if (KeyHandler::CompareKeys (v.key, key))
      {
        v.value = value;
        return;
      }
    }

    values.Push (Element (key, value));
    Size++;
    if (values.Length () > Elements.Length () / GrowRate
     && Elements.Length () < MaxSize) Grow ();
  }

  /**
   * Add an element to the hash table, overwriting if the key already exists.
   * \deprecated Use PutUnique() instead.
   */
  CS_DEPRECATED_METHOD void PutFirst (const K& key, const T &value)
  {
    PutUnique(key, value);
  }

  /// Returns whether at least one element matches the given key.
  bool Contains (const K& key) const
  {
    const csArray<Element> &values = 
      Elements[KeyHandler::ComputeHash (key) % Modulo];
    const size_t len = values.Length ();
    for (size_t i = 0; i < len; ++i)
      if (KeyHandler::CompareKeys (values[i].key, key)) 
	return true;
    return false;
  }

  /**
   * Returns whether at least one element matches the given key.
   * \remarks This is rigidly equivalent to Contains(key), but may be
   *   considered more idiomatic by some.
   */
  bool In (const K& key) const
  { return Contains(key); }

  /**
   * Get a pointer to the first element matching the given key, 
   * or 0 if there is none.
   */
  const T* GetElementPointer (const K& key) const
  {
    const csArray<Element> &values = 
      Elements[KeyHandler::ComputeHash (key) % Modulo];
    const size_t len = values.Length ();
    for (size_t i = 0; i < len; ++i)
    {
      const Element& v = values[i];
      if (KeyHandler::CompareKeys (v.key, key))
	return &v.value;
    }

    return 0;
  }

  /**
   * Get a pointer to the first element matching the given key, 
   * or 0 if there is none.
   */
  T* GetElementPointer (const K& key)
  {
    csArray<Element> &values = 
      Elements[KeyHandler::ComputeHash (key) % Modulo];
    const size_t len = values.Length ();
    for (size_t i = 0; i < len; ++i)
    {
      Element& v = values[i];
      if (KeyHandler::CompareKeys (v.key, key))
	return &v.value;
    }

    return 0;
  }

  /**
   * Get the first element matching the given key, or \p fallback if there is 
   * none.
   */
  const T& Get (const K& key, const T& fallback) const
  {
    const csArray<Element> &values = 
      Elements[KeyHandler::ComputeHash (key) % Modulo];
    const size_t len = values.Length ();
    for (size_t i = 0; i < len; ++i)
    {
      const Element& v = values[i];
      if (KeyHandler::CompareKeys (v.key, key))
	return v.value;
    }

    return fallback;
  }

  /**
   * Get the first element matching the given key, or \p fallback if there is 
   * none.
   */
  T& Get (const K& key, T& fallback)
  {
    csArray<Element> &values = 
      Elements[KeyHandler::ComputeHash (key) % Modulo];
    const size_t len = values.Length ();
    for (size_t i = 0; i < len; ++i)
    {
      Element& v = values[i];
      if (KeyHandler::CompareKeys (v.key, key))
	return v.value;
    }

    return fallback;
  }

  /// Delete all the elements.
  void DeleteAll ()
  {
    Elements.SetLength (Modulo = InitModulo);
    size_t elen = Elements.Length ();
    for (size_t i = 0; i < elen; i++)
      Elements[i].Empty ();
    Size = 0;
  }

  /// Delete all the elements. (Idiomatic alias for DeleteAll().)
  void Empty() { DeleteAll(); }

  /// Delete all the elements matching the given key.
  bool DeleteAll (const K& key)
  {
    bool ret = false;
    csArray<Element> &values = 
      Elements[KeyHandler::ComputeHash (key) % Modulo];
    for (size_t i = values.Length (); i > 0; i--)
    {
      const size_t idx = i - 1;
      if (KeyHandler::CompareKeys (values[idx].key, key))
      {
	values.DeleteIndexFast (idx);
        ret = true;
        Size--;
      }
    }
    return ret;
  }
  
  /// Delete all the elements matching the given key and value.
  bool Delete (const K& key, const T &value)
  {
    bool ret = false;
    csArray<Element> &values = 
      Elements[KeyHandler::ComputeHash (key) % Modulo];
    for (size_t i = values.Length (); i > 0; i--)
    {
      const size_t idx = i - 1;
      if (KeyHandler::CompareKeys (values[idx].key, key) && 
	(values[idx].value == value))
      {
        values.DeleteIndexFast (idx);
        ret = true;
        Size--;
      }
    }
    return ret;
  }

  /// Get the number of elements in the hash.
  size_t GetSize () const
  {
    return Size;
  }

  /**
   * Return true if the hash is empty.
   * \remarks Rigidly equivalent to <tt>return GetSize() == 0</tt>, but more
   *   idiomatic.
   */
  bool IsEmpty() const
  {
    return GetSize() == 0;
  }

  /// An iterator class for the hash.
  class Iterator
  {
  private:
    const csHash<T, K, KeyHandler>* hash;
    const K key;
    size_t bucket, size, element;

    void Seek ()
    {
      while ((element < size) && 
        ! KeyHandler::CompareKeys (hash->Elements[bucket][element].key, key))
          element++;
    }

  protected:
    Iterator (const csHash<T, K, KeyHandler>* hash0, const K& key0) :
      hash(hash0),
      key(key0), 
      bucket(KeyHandler::ComputeHash(key) % hash->Modulo),
      size(hash->Elements[bucket].Length ())
      { Reset (); }

    friend class csHash<T, K, KeyHandler>;
  public:
    /// Copy constructor.
    Iterator (const Iterator &o) :
      hash (o.hash),
      key(o.key),
      bucket(o.bucket),
      size(o.size),
      element(o.element) {}

    /// Assignment operator.
    Iterator& operator=(const Iterator& o)
    {
      hash = o.hash;
      key = o.key;
      bucket = o.bucket;
      size = o.size;
      element = o.element;
      return *this;
    }

    /// Returns a boolean indicating whether or not the hash has more elements.
    bool HasNext () const
    {
      return element < size;
    }

    /// Get the next element's value.
    const T& Next ()
    {
      const T &ret = hash->Elements[bucket][element].value;
      element++;
      Seek ();
      return ret;
    }

    /// Move the iterator back to the first element.
    void Reset () { element = 0; Seek (); }
  };
  friend class Iterator;

  /// An iterator class for the hash.
  class GlobalIterator
  {
  private:
    const csHash<T, K, KeyHandler> *hash;
    size_t bucket, size, element;

    void Zero () { bucket = element = 0; }
    void Init () { size = hash->Elements[bucket].Length (); }

    void FindItem ()
    {
      if (element >= size)
      {
	while (++bucket < hash->Elements.Length ())
	{
          Init ();
	  if (size != 0)
	  {
	    element = 0;
	    break;
	  }
	}
      }
    }

  protected:
    GlobalIterator (const csHash<T, K, KeyHandler> *hash0) : hash (hash0) 
    { 
      Zero (); 
      Init (); 
      FindItem ();
    }

    friend class csHash<T, K, KeyHandler>;
  public:
    /// Copy constructor.
    GlobalIterator (const Iterator &o) :
      hash (o.hash),
      bucket (o.bucket),
      size (o.size),
      element (o.element) {}

    /// Assignment operator.
    GlobalIterator& operator=(const GlobalIterator& o)
    {
      hash = o.hash;
      bucket = o.bucket;
      size = o.size;
      element = o.element;
      return *this;
    }

    /// Returns a boolean indicating whether or not the hash has more elements.
    bool HasNext () const
    {
      if (hash->Elements.Length () == 0) return false;
      return element < size || bucket < hash->Elements.Length ();
    }

    /// Advance the iterator of one step
    void Advance ()
    {
      element++;
      FindItem ();
    }

    /// Get the next element's value, don't move the iterator.
    const T& NextNoAdvance ()
    {
      return hash->Elements[bucket][element].value;
    }

    /// Get the next element's value.
    const T& Next ()
    {
      const T &ret = NextNoAdvance ();
      Advance ();
      return ret;
    }

    /// Get the next element's value and key, don't move the iterator.
    const T& NextNoAdvance (K &key)
    {
      key = hash->Elements[bucket][element].key;
      return NextNoAdvance ();
    }

    /// Get the next element's value and key.
    const T& Next (K &key)
    {
      key = hash->Elements[bucket][element].key;
      return Next ();
    }

    /// Move the iterator back to the first element.
    void Reset () { Zero (); Init (); FindItem (); }
  };
  friend class GlobalIterator;

  /// Delete the element pointed by the iterator. This is safe for this
  /// iterator, not for the others.
  void DeleteElement (GlobalIterator& iterator)
  {
    Elements[iterator.bucket].DeleteIndex(iterator.element);
    Size--;
    iterator.size--;
    iterator.FindItem ();
  }

  /**
   * Return an iterator for the hash, to iterate only over the elements
   * with the given key.
   * \warning Modifying the hash (except with DeleteElement()) while you have
   *   open iterators will result in undefined behaviour.
   */
  Iterator GetIterator (const K& key) const
  {
    return Iterator (this, key);
  }

  /**
   * Return an iterator for the hash, to iterate over all elements.
   * \warning Modifying the hash (except with DeleteElement()) while you have
   *   open iterators will result in undefined behaviour.
   */
  GlobalIterator GetIterator () const
  {
    return GlobalIterator (this);
  }
};

#endif
