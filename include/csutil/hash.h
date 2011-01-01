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
#include "csutil/array.h"
#include "csutil/comparator.h"
#include "csutil/util.h"
#include "csutil/tuple.h"
#include "csutil/hashcomputer.h"

/**\addtogroup util_containers
 * @{ */


/**
 * A helper template to use pointers as keys for hashes.
 */
template <typename T>
class csPtrKey
{
  T* ptr;
public:
  csPtrKey () : ptr(0) {}
  csPtrKey (T* ptr) : ptr(ptr) {}
  csPtrKey (csPtrKey const& other) : ptr (other.ptr) {}

  uint GetHash () const { return (uintptr_t)ptr; }
  inline friend bool operator < (const csPtrKey& r1, const csPtrKey& r2)
  { return r1.ptr < r2.ptr; }
  operator T* () const
  { return ptr; }
  T* operator -> () const
  { return ptr; }
  csPtrKey& operator = (csPtrKey const& other)
  { ptr = other.ptr; return *this; }
};

/**
 * A helper template to use const pointers as keys for hashes.
 */
template <typename T>
class csConstPtrKey
{
  const T* ptr;
public:
  csConstPtrKey () : ptr(0) {}
  csConstPtrKey (const T* ptr) : ptr(ptr) {}
  csConstPtrKey (csConstPtrKey const& other) : ptr (other.ptr) {}

  uint GetHash () const { return (uintptr_t)ptr; }
  inline friend bool operator < (const csConstPtrKey& r1, const csConstPtrKey& r2)
  { return r1.ptr < r2.ptr; }
  operator const T* () const
  { return ptr; }
  const T* operator -> () const
  { return ptr; }
  csConstPtrKey& operator = (csConstPtrKey const& other)
  { ptr = other.ptr; return *this; }
};

template <class T, class K, 
  class ArrayMemoryAlloc, class ArrayElementHandler> class csHash;

namespace CS
{
  namespace Container
  {
    /**
     * An element of a hash.
     * This class is internally used by csHash<>. However, it has to be used
     * when a custom element handler needs to be used.
     */
    template <class T, class K>
    class HashElement
    {
    private:
      template <class _T, class _K, class ArrayMemoryAlloc,
        class ArrayElementHandler> friend class csHash;
      
      K key;
      T value;
    public:
      HashElement (const K& key0, const T &value0) : key (key0), value (value0) {}
      HashElement (const HashElement& other) : key (other.key), value (other.value) {}
      
      const K& GetKey() const { return key; }
      const T& GetValue() const { return value; }
      T& GetValue() { return value; }
    };
  } // namespace Container
} // namespace CS

/**
 * A generic hash table class,
 * which grows dynamically and whose buckets are unsorted arrays.
 * The hash value of a key is computed using csHashComputer<>, two keys are
 * compared using csComparator<>. You need to provide appropriate 
 * specializations of those templates if you want use non-integral types 
 * (other than const char* and csString for which appropriate specializations
 * are already provided) or special hash algorithms. 
 */
template <class T, class K = unsigned int, 
  class ArrayMemoryAlloc = CS::Memory::AllocatorMalloc,
  class ArrayElementHandler = csArrayElementHandler<
    CS::Container::HashElement<T, K> > > 
class csHash
{
public:
  typedef csHash<T, K, ArrayMemoryAlloc, ArrayElementHandler> ThisType;
  typedef T ValueType;
  typedef K KeyType;
  typedef ArrayMemoryAlloc AllocatorType;

protected:
  typedef CS::Container::HashElement<T, K> Element;
  typedef csArray<Element, ArrayElementHandler,
    ArrayMemoryAlloc, csArrayCapacityDefault> ElementArray;
  csArray<ElementArray, csArrayElementHandler<ElementArray>,
    ArrayMemoryAlloc> Elements;

  size_t Modulo;
  size_t Size;

  size_t InitModulo;
  size_t GrowRate;
  size_t MaxSize;

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
    size_t elen = Elements.GetSize ();
    for (p = Primes; *p && *p <= elen; p++) ;
    Modulo = *p;
    CS_ASSERT (Modulo);

    Elements.SetSize (Modulo, ElementArray (0, MIN(Modulo / GrowRate, 4)));

    for (size_t i = 0; i < elen; i++)
    {
      ElementArray& src = Elements[i];
      size_t slen = src.GetSize ();
      for (size_t j = slen; j > 0; j--)
      {
        const Element& srcElem = src[j - 1];
        ElementArray& dst = 
	  Elements.Get (csHashComputer<K>::ComputeHash (srcElem.key) % Modulo);
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
   * \a grow_rate is the rate at which the hash table grows:
   * \a size doubles once there are \a size/\a grow_rate collisions.
   * It will not grow after it reaches \a max_size.
   * 
   * Here are a few primes: 7, 11, 19, 29, 59, 79, 101, 127, 151, 199, 251,
   * 307, 401, 503, 809, 1009, 1499, 2003, 3001, 5003, 12263, 25247, 36923,
   * 50119, 70951, 90313, 104707.
   * 
   * For a bigger list go to http://www.utm.edu/research/primes/
   */
  csHash (size_t size = 23, size_t grow_rate = 5, size_t max_size = 20000)
    : Modulo (size), Size(0), InitModulo (size),
      GrowRate (MIN (grow_rate, size)), MaxSize (max_size)
  {
  }

  /// Copy constructor.
  csHash (const csHash<T, K, ArrayMemoryAlloc, ArrayElementHandler> &o) : 
    Elements (o.Elements),
    Modulo (o.Modulo), Size(o.Size), InitModulo (o.InitModulo),
    GrowRate (o.GrowRate), MaxSize (o.MaxSize) {}

  /**
   * Add an element to the hash table.
   * \remarks If \a key is already present, does NOT replace the existing value,
   *   but merely adds \a value as an additional value of \a key. To retrieve all
   *   values for a given key, use GetAll(). If you instead want to replace an
   *   existing value for \a key, use PutUnique().
   */
  T& Put (const K& key, const T &value)
  {
    if (Elements.GetSize() == 0) Elements.SetSize (Modulo);
    ElementArray& values = 
      Elements[csHashComputer<K>::ComputeHash (key) % Modulo];
    size_t idx = values.Push (Element (key, value));
    Size++;
    if (values.GetSize () > Elements.GetSize () / GrowRate
     && Elements.GetSize () < MaxSize)
    {
      Grow ();
      /* can't use 'values[idx]' since that is no longer the place where
         the item is stored. */
      return *(GetElementPointer (key));
    }
    return values[idx].value;
  }

  /// Get all the elements, or empty if there are none.
  csArray<T> GetAll () const
  {
    if (Elements.GetSize() == 0) return csArray<T> ();

    ConstGlobalIterator itr = GetIterator();
    csArray<T> ret;
    while(itr.HasNext())
    {
      ret.Push(itr.Next());
    }

    return ret;
  }

  /// Get all the elements with the given key, or empty if there are none.
  csArray<T> GetAll (const K& key) const
  {
    return GetAll<typename csArray<T>::ElementHandlerType, 
      typename csArray<T>::AllocatorType> (key);
  }

  /// Get all the elements with the given key, or empty if there are none.
  template<typename H, typename M>
  csArray<T, H, M> GetAll (const K& key) const
  {
    if (Elements.GetSize() == 0) return csArray<T> ();
    const ElementArray& values = 
      Elements[csHashComputer<K>::ComputeHash (key) % Modulo];
    csArray<T> ret (values.GetSize () / 2);
    const size_t len = values.GetSize ();
    for (size_t i = 0; i < len; ++i)
    {
      const Element& v = values[i];
      if (csComparator<K, K>::Compare (v.key, key) == 0) 
	ret.Push (v.value);
    }
    return ret;
  }

  /// Add an element to the hash table, overwriting if the key already exists.
  T& PutUnique (const K& key, const T &value)
  {
    if (Elements.GetSize() == 0) Elements.SetSize (Modulo);
    ElementArray& values = 
      Elements[csHashComputer<K>::ComputeHash (key) % Modulo];
    const size_t len = values.GetSize ();
    for (size_t i = 0; i < len; ++i)
    {
      Element& v = values[i];
      if (csComparator<K, K>::Compare (v.key, key) == 0)
      {
        v.value = value;
        return v.value;
      }
    }

    size_t idx = values.Push (Element (key, value));
    Size++;
    if (values.GetSize () > Elements.GetSize () / GrowRate
     && Elements.GetSize () < MaxSize)
    {
      Grow ();
      /* can't use 'values[idx]' since that is no longer the place where
         the item is stored. */
      return *(GetElementPointer (key));
    }
    return values[idx].value;
  }

  /// Returns whether at least one element matches the given key.
  bool Contains (const K& key) const
  {
    if (Elements.GetSize() == 0) return false;
    const ElementArray& values = 
      Elements[csHashComputer<K>::ComputeHash (key) % Modulo];
    const size_t len = values.GetSize ();
    for (size_t i = 0; i < len; ++i)
      if (csComparator<K, K>::Compare (values[i].key, key) == 0) 
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
    if (Elements.GetSize() == 0) return 0;
    const ElementArray& values = 
      Elements[csHashComputer<K>::ComputeHash (key) % Modulo];
    const size_t len = values.GetSize ();
    for (size_t i = 0; i < len; ++i)
    {
      const Element& v = values[i];
      if (csComparator<K, K>::Compare (v.key, key) == 0)
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
    if (Elements.GetSize() == 0) return 0;
    ElementArray& values = 
      Elements[csHashComputer<K>::ComputeHash (key) % Modulo];
    const size_t len = values.GetSize ();
    for (size_t i = 0; i < len; ++i)
    {
      Element& v = values[i];
      if (csComparator<K, K>::Compare (v.key, key) == 0)
	return &v.value;
    }

    return 0;
  }

  /**
   * h["key"] shorthand notation for h.GetElementPointer ("key")
   */
  T* operator[] (const K& key)
  {
    return GetElementPointer (key);
  }

  /**
   * Get the first element matching the given key, or \a fallback if there is 
   * none.
   */
  const T& Get (const K& key, const T& fallback) const
  {
    if (Elements.GetSize() == 0) return fallback;
    const ElementArray& values = 
      Elements[csHashComputer<K>::ComputeHash (key) % Modulo];
    const size_t len = values.GetSize ();
    for (size_t i = 0; i < len; ++i)
    {
      const Element& v = values[i];
      if (csComparator<K, K>::Compare (v.key, key) == 0)
	return v.value;
    }

    return fallback;
  }

  /**
   * Get the first element matching the given key, or \a fallback if there is 
   * none.
   */
  T& Get (const K& key, T& fallback)
  {
    if (Elements.GetSize() == 0) return fallback;
    ElementArray& values = 
      Elements[csHashComputer<K>::ComputeHash (key) % Modulo];
    const size_t len = values.GetSize ();
    for (size_t i = 0; i < len; ++i)
    {
      Element& v = values[i];
      if (csComparator<K, K>::Compare (v.key, key) == 0)
	return v.value;
    }

    return fallback;
  }

  /**
   * Get the first element matching the given key, or, if there is 
   * none, insert \a default and return a reference to the new entry.
   */
  T& GetOrCreate (const K& key, const T& defaultValue = T())
  {
    if (Elements.GetSize() != 0)
    {
      ElementArray& values = 
        Elements[csHashComputer<K>::ComputeHash (key) % Modulo];
      const size_t len = values.GetSize ();
      for (size_t i = 0; i < len; ++i)
      {
        Element& v = values[i];
        if (csComparator<K, K>::Compare (v.key, key) == 0)
	  return v.value;
      }
    }
    
    return Put (key, defaultValue);
  }

  /// Delete all the elements.
  void DeleteAll ()
  {
    Elements.DeleteAll();
    Modulo = InitModulo;
    Size = 0;
  }

  /// Delete all the elements. (Idiomatic alias for DeleteAll().)
  void Empty() { DeleteAll(); }

  /// Delete all the elements matching the given key.
  bool DeleteAll (const K& key)
  {
    bool ret = false;
    if (Elements.GetSize() == 0) return ret;
    ElementArray& values = 
      Elements[csHashComputer<K>::ComputeHash (key) % Modulo];
    for (size_t i = values.GetSize (); i > 0; i--)
    {
      const size_t idx = i - 1;
      if (csComparator<K, K>::Compare (values[idx].key, key) == 0)
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
    if (Elements.GetSize() == 0) return ret;
    ElementArray& values = 
      Elements[csHashComputer<K>::ComputeHash (key) % Modulo];
    for (size_t i = values.GetSize (); i > 0; i--)
    {
      const size_t idx = i - 1;
      if ((csComparator<K, K>::Compare (values[idx].key, key) == 0) && 
	  (csComparator<T, T>::Compare (values[idx].value, value) == 0 ))
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

  /// An iterator class for the csHash class.
  class Iterator
  {
  private:
    csHash<T, K, ArrayMemoryAlloc, ArrayElementHandler>* hash;
    const K key;
    size_t bucket, size, element;

    void Seek ()
    {
      while ((element < size) && 
        (csComparator<K, K>::Compare (hash->Elements[bucket][element].GetKey(), 
	key) != 0))
          element++;
    }

  protected:
    Iterator (csHash<T, K, ArrayMemoryAlloc, ArrayElementHandler>* hash0, 
      const K& key0) :
      hash(hash0),
      key(key0), 
      bucket(csHashComputer<K>::ComputeHash (key) % hash->Modulo),
      size((hash->Elements.GetSize() > 0) ? hash->Elements[bucket].GetSize () : 0)
      { Reset (); }

    friend class csHash<T, K, ArrayMemoryAlloc, ArrayElementHandler>;
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
    T& Next ()
    {
      T &ret = hash->Elements[bucket][element].GetValue();
      element++;
      Seek ();
      return ret;
    }

    /// Move the iterator back to the first element.
    void Reset () { element = 0; Seek (); }
  };
  friend class Iterator;

  /// An iterator class for the csHash class.
  class GlobalIterator
  {
  private:
    csHash<T, K, ArrayMemoryAlloc, ArrayElementHandler> *hash;
    size_t bucket, size, element;

    void Zero () { bucket = element = 0; }
    void Init () 
    { 
      size = 
        (hash->Elements.GetSize() > 0) ? hash->Elements[bucket].GetSize () : 0;
    }

    void FindItem ()
    {
      if (element >= size)
      {
	while (++bucket < hash->Elements.GetSize ())
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
    GlobalIterator (csHash<T, K, ArrayMemoryAlloc, ArrayElementHandler> *hash0)
    : hash (hash0) 
    { 
      Zero (); 
      Init (); 
      FindItem ();
    }

    friend class csHash<T, K, ArrayMemoryAlloc, ArrayElementHandler>;
  public:
    /// Copy constructor.
    GlobalIterator (const GlobalIterator &o) :
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
      if (hash->Elements.GetSize () == 0) return false;
      return element < size || bucket < hash->Elements.GetSize ();
    }

    /// Advance the iterator of one step
    void Advance ()
    {
      element++;
      FindItem ();
    }

    /// Get the next element's value, don't move the iterator.
    T& NextNoAdvance ()
    {
      return hash->Elements[bucket][element].GetValue();
    }

    /// Get the next element's value.
    T& Next ()
    {
      T &ret = NextNoAdvance ();
      Advance ();
      return ret;
    }

    /// Get the next element's value and key, don't move the iterator.
    T& NextNoAdvance (K &key)
    {
      key = hash->Elements[bucket][element].GetKey();
      return NextNoAdvance ();
    }

    /// Get the next element's value and key.
    T& Next (K &key)
    {
      key = hash->Elements[bucket][element].GetKey();
      return Next ();
    }

    /// Return a tuple of the value and key.
    const csTuple2<T, K> NextTuple ()
    {
      csTuple2<T, K> t (NextNoAdvance (),
        hash->Elements[bucket][element].GetKey());
      Advance ();
      return t;
    }

    /// Move the iterator back to the first element.
    void Reset () { Zero (); Init (); FindItem (); }
  };
  friend class GlobalIterator;

  /// An const iterator class for the csHash class.
  class ConstIterator
  {
  private:
    const csHash<T, K, ArrayMemoryAlloc, ArrayElementHandler>* hash;
    const K key;
    size_t bucket, size, element;

    void Seek ()
    {
      while ((element < size) && 
        (csComparator<K, K>::Compare (hash->Elements[bucket][element].GetKey(), 
	key) != 0))
          element++;
    }

  protected:
    ConstIterator (const csHash<T, K, ArrayMemoryAlloc, ArrayElementHandler>* 
	hash0, const K& key0) :
      hash(hash0),
      key(key0), 
      bucket(csHashComputer<K>::ComputeHash (key) % hash->Modulo),
      size((hash->Elements.GetSize() > 0) ? hash->Elements[bucket].GetSize () : 0)
      { Reset (); }

    friend class csHash<T, K, ArrayMemoryAlloc, ArrayElementHandler>;
  public:
    /// Copy constructor.
    ConstIterator (const ConstIterator &o) :
      hash (o.hash),
      key(o.key),
      bucket(o.bucket),
      size(o.size),
      element(o.element) {}

    /// Assignment operator.
    ConstIterator& operator=(const ConstIterator& o)
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
      const T &ret = hash->Elements[bucket][element].GetValue();
      element++;
      Seek ();
      return ret;
    }

    /// Move the iterator back to the first element.
    void Reset () { element = 0; Seek (); }
  };
  friend class ConstIterator;

  /// An const iterator class for the csHash class.
  class ConstGlobalIterator
  {
  private:
    const csHash<T, K, ArrayMemoryAlloc, ArrayElementHandler> *hash;
    size_t bucket, size, element;

    void Zero () { bucket = element = 0; }
    void Init () 
    { 
      size = 
        (hash->Elements.GetSize() > 0) ? hash->Elements[bucket].GetSize () : 0;
    }

    void FindItem ()
    {
      if (element >= size)
      {
	while (++bucket < hash->Elements.GetSize ())
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
    ConstGlobalIterator (const csHash<T, K, ArrayMemoryAlloc,
      ArrayElementHandler> *hash0) : hash (hash0) 
    { 
      Zero (); 
      Init (); 
      FindItem ();
    }

    friend class csHash<T, K, ArrayMemoryAlloc, ArrayElementHandler>;
  public:
    /// Copy constructor.
    ConstGlobalIterator (const ConstGlobalIterator &o) :
      hash (o.hash),
      bucket (o.bucket),
      size (o.size),
      element (o.element) {}

    /// Assignment operator.
    ConstGlobalIterator& operator=(const ConstGlobalIterator& o)
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
      if (hash->Elements.GetSize () == 0) return false;
      return element < size || bucket < hash->Elements.GetSize ();
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
      return hash->Elements[bucket][element].GetValue();
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
      key = hash->Elements[bucket][element].GetKey();
      return NextNoAdvance ();
    }

    /// Get the next element's value and key.
    const T& Next (K &key)
    {
      key = hash->Elements[bucket][element].GetKey();
      return Next ();
    }

    /// Return a tuple of the value and key.
    const csTuple2<T, K> NextTuple ()
    {
      csTuple2<T, K> t (NextNoAdvance (),
        hash->Elements[bucket][element].GetKey());
      Advance ();
      return t;
    }

    /// Move the iterator back to the first element.
    void Reset () { Zero (); Init (); FindItem (); }
  };
  friend class ConstGlobalIterator;

  /// Delete the element pointed by the iterator. This is safe for this
  /// iterator, not for the others.
  void DeleteElement (GlobalIterator& iterator)
  {
    Elements[iterator.bucket].DeleteIndex(iterator.element);
    Size--;
    iterator.size--;
    iterator.FindItem ();
  }

  /// Delete the element pointed by the iterator. This is safe for this
  /// iterator, not for the others.
  void DeleteElement (ConstGlobalIterator& iterator)
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
  Iterator GetIterator (const K& key)
  {
    return Iterator (this, key);
  }

  /**
   * Return an iterator for the hash, to iterate over all elements.
   * \warning Modifying the hash (except with DeleteElement()) while you have
   *   open iterators will result in undefined behaviour.
   */
  GlobalIterator GetIterator ()
  {
    return GlobalIterator (this);
  }

  /**
   * Return a const iterator for the hash, to iterate only over the elements
   * with the given key.
   * \warning Modifying the hash (except with DeleteElement()) while you have
   *   open iterators will result in undefined behaviour.
   */
  ConstIterator GetIterator (const K& key) const
  {
    return ConstIterator (this, key);
  }

  /**
   * Return a const iterator for the hash, to iterate over all elements.
   * \warning Modifying the hash (except with DeleteElement()) while you have
   *   open iterators will result in undefined behaviour.
   */
  ConstGlobalIterator GetIterator () const
  {
    return ConstGlobalIterator (this);
  }
};

/** @} */

#endif
