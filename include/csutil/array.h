/*
  Crystal Space Generic Array Template
  Copyright (C) 2003 by Matze Braun
  Copyright (C) 2003 by Jorrit Tyberghein
  Copyright (C) 2003,2004 by Eric Sunshine

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
#ifndef __CSUTIL_ARRAY_H__
#define __CSUTIL_ARRAY_H__

/**\file
 * Generic Array Template
 */

// Hack: Work around problems caused by #defining 'new'.
#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# undef new
#endif
#include <new>

#if defined(CS_MEMORY_TRACKER)
#include "csutil/memdebug.h"
#include "csutil/snprintf.h"
#include <typeinfo>
#endif

/**
 * A template providing various comparison and ordering functions.
 */
template <class T1, class T2>
class csOrdering
{
public:
  /**
   * Compare two objects of the same type or different types (T1 and T2).
   * \param r1 Reference to first object.
   * \param r2 Reference to second object.
   * \return Zero if the objects are equal; less-than-zero if the first object
   *   is less than the second; or greater-than-zero if the first object is
   *   greater than the second.
   * \remarks Assumes the existence of T1::operator<(T2) and T2::operator<(T1).
   *   If T1 and T2 are the same type T, then only T::operator<(T) is assumed
   *   (of course).  This is the default comparison function used by csArray
   *   for searching and sorting if the client does not provide a custom
   *   function.
   */
  static int Compare(T1 const &r1, T2 const &r2)
  {
    if (r1 < r2) return -1;
    else if (r2 < r1) return 1;
    else return 0;
  }
};

// Define CSARRAY_INHIBIT_TYPED_KEYS if the compiler is too old or too buggy to
// properly support templated functions within a templated class.  When this is
// defined, rather than using a properly typed "key" argument, search methods
// fall back to dealing with opaque void* for the "key" argument.  Note,
// however, that this fact is completely hidden from the client; the client
// simply creates csArrayCmp<> functors using correct types for the keys
// regardless of whether the compiler actually supports this feature.  (The
// MSVC6 compiler, for example, does support templated functions within a
// template class but crashes and burns horribly when a function pointer or
// functor is thrown into the mix; thus this should be defined for MSVC6.)
#if !defined(CSARRAY_INHIBIT_TYPED_KEYS)

/**
 * A functor template which encapsulates a key and a comparison function for
 * use with key-related csArray<> searching methods, such as FindKey() and
 * FindSortedKey().  Being a template instaniated upon two (possibly distinct)
 * types, this allows the searching methods to perform type-safe searches even
 * when the search key type differs from the contained element type.  The
 * supplied search function defines the relationship between the search key and
 * the contained element.
 */
template <class T, class K>
class csArrayCmp
{
public:
  /**
   * Type of the comparison function which compares a key against an element
   * contained in a csArray<>.  T is the type of the contained element.  K is
   * the type of the search key.
   */
  typedef int(*CF)(T const&, K const&);
  /// Construct a functor from a search key and a comparison function.
  csArrayCmp(K const& k, CF c = DefaultCompare) : key(k), cmp(c) {}
  /// Construct a functor from another functor.
  csArrayCmp(csArrayCmp const& o) : key(o.key), cmp(o.cmp) {}
  /// Assign another functor to this one.
  csArrayCmp& operator=(csArrayCmp const& o)
    { key = o.key; cmp = o.cmp; return *this; }
  /**
   * Invoke the functor.
   * \param r Reference to the element to which the stored key should be
   *   compared.
   * \return Zero if the key matches the element; less-than-zero if the element
   *   is less than the key; greater-than-zero if the element is greater than
   *   the key.
   */
  int operator()(T const& r) const { return cmp(r, key); }
  /// Return the comparison function with which this functor was constructed.
  operator CF() const { return cmp; }
  /// Return the key with which this functor was constructed.
  operator K const&() const { return key; }
  /**
   * Compare two objects of the same type or different types (T and K).
   * \param r Reference to the element to which the key should be compared.
   * \param k Reference to the key to which the element should be compared.
   * \return Zero if the key matches the element; less-than-zero if the element
   *   is less than the key; greater-than-zero if the element is greater than
   *   the key.
   * \remarks Assumes the presence of T::operator<(K) and K::operator<(T).
   *   Default comparison function if client does not supply one.
   */
  static int DefaultCompare(T const& r, K const& k)
    { return csOrdering<T,K>::Compare(r,k); }
private:
  K key;
  CF cmp;
};

#define csArrayTemplate(K) template <class K>
#define csArrayCmpDecl(T1,T2) csArrayCmp<T1,T2>
#define csArrayCmpInvoke(C,R) C(R)

#else // CSARRAY_INHIBIT_TYPED_KEYS

class csArrayCmpAbstract
{
public:
  typedef int(*CF)(void const*, void const*);
  virtual int operator()(void const*) const = 0;
  virtual operator CF() const = 0;
};

template <class T, class K>
class csArrayCmp : public csArrayCmpAbstract
{
public:
  typedef int(*CFTyped)(T const&, K const&);
  csArrayCmp(K const& k, CFTyped c = DefaultCompare) : key(k), cmp(CF(c)) {}
  csArrayCmp(csArrayCmp const& o) : key(o.key), cmp(o.cmp) {}
  csArrayCmp& operator=(csArrayCmp const& o)
    { key = o.key; cmp = o.cmp; return *this; }
  virtual int operator()(void const* p) const { return cmp(p, &key); }
  virtual operator CF() const { return cmp; }
  operator K const&() const { return key; }
  static int DefaultCompare(T const& r, K const& k)
    { return csOrdering<T,K>::Compare(r,k); }
private:
  K key;
  CF cmp;
};

#define csArrayTemplate(K)
#define csArrayCmpDecl(T1,T2) csArrayCmpAbstract const&
#define csArrayCmpInvoke(C,R) C(&(R))

#endif // CSARRAY_INHIBIT_TYPED_KEYS

/**
 * The default element handler for csArray.
 */
template <class T>
class csArrayElementHandler
{
public:
  static void Construct (T* address)
  {
    new (CS_STATIC_CAST(void*,address)) T();
  }

  static void Construct (T* address, T const& src)
  {
    new (CS_STATIC_CAST(void*,address)) T(src);
  }

  static void Destroy (T* address)
  {
    address->~T();
  }

  static void InitRegion (T* address, size_t count)
  {
    for (size_t i = 0 ; i < count ; i++)
      Construct (address + i);
  }
};

/**
 * The default allocator for csArray. Uses malloc/free/realloc.
 */
template <class T>
class csArrayMemoryAllocator
{
public:
  static T* Alloc (size_t count)
  {
    return (T*)malloc (count * sizeof(T));
  }

  static void Free (T* mem)
  {
    free (mem);
  }

  // The 'relevantcount' parameter should be the number of items
  // in the old array that are initialized.
  static T* Realloc (T* mem, size_t relevantcount, size_t oldcount,
    size_t newcount)
  {
    (void)relevantcount; (void)oldcount;
    return (T*)realloc (mem, newcount * sizeof(T));
  }

  // Move memory.
  static void MemMove (T* mem, size_t dest, size_t src, size_t count)
  {
    memmove (mem + dest, mem + src, count * sizeof(T));
  }
};

/**
 * Special allocator for csArray that makes sure that when
 * the array is reallocated that the objects are properly constructed
 * and destructed at their new position. This is needed for objects
 * that can not be safely moved around in memory (like weak references).
 * This is of course slower and that is the reason that this is not
 * done by default.
 */
template <class T, class ElementHandler = csArrayElementHandler<T> >
class csSafeCopyArrayMemoryAllocator
{
public:
  static T* Alloc (size_t count)
  {
    return (T*)malloc (count * sizeof(T));
  }

  static void Free (T* mem)
  {
    free (mem);
  }

  static T* Realloc (T* mem, size_t relevantcount, size_t oldcount,
    size_t newcount)
  {
    if (newcount <= oldcount)
    {
      // Realloc is safe.
      T* newmem = (T*)realloc (mem, newcount * sizeof (T));
      CS_ASSERT (newmem == mem);
      return newmem;
    }

    T* newmem = Alloc (newcount);
    size_t i;
    for (i = 0 ; i < relevantcount ; i++)
    {
      ElementHandler::Construct (newmem + i, mem[i]);
      ElementHandler::Destroy (mem + i);
    }
    Free (mem);
    return newmem;
  }

  static void MemMove (T* mem, size_t dest, size_t src, size_t count)
  {
    size_t i;
    if (dest < src)
    {
      for (i = 0 ; i < count ; i++)
      {
        ElementHandler::Construct (mem + dest + i, mem[src + i]);
	ElementHandler::Destroy (mem + src + i);
      }
    }
    else
    {
      i = count;
      while (i > 0)
      {
	i--;
        ElementHandler::Construct (mem + dest + i, mem[src + i]);
	ElementHandler::Destroy (mem + src + i);
      }
    }
  }
};

/**
 * This value is returned whenever an array item could not be located or does
 * not exist.
 */
const size_t csArrayItemNotFound = (size_t)-1;

/**
 * A templated array class.  The objects in this class are constructed via
 * copy-constructor and are destroyed when they are removed from the array or
 * the array is destroyed.
 * \note If you want to store reference-counted object pointers, such as iFoo*,
 * then you should consider csRefArray<>, which is more idiomatic than
 * csArray<csRef<iFoo>>.
 */
template <class T,
	class ElementHandler = csArrayElementHandler<T>,
	class MemoryAllocator = csArrayMemoryAllocator<T> >
class csArray
{
private:
  size_t count;
  size_t capacity;
  size_t threshold;
  T* root;
#ifdef CS_MEMORY_TRACKER
  csMemTrackerInfo* mti;
  void UpdateMti (int dn, int curcapacity)
  {
    if (!mti)
    {
      if (!curcapacity) return;
      char buf[1024];
      cs_snprintf (buf, sizeof (buf), "csArray<%s>", typeid (T).name());
      mti = mtiRegisterAlloc (1 * sizeof (T), buf);
      if (!mti) return;
      curcapacity--;
      if (curcapacity)
        mtiUpdateAmount (mti, curcapacity, curcapacity * sizeof (T));
      return;
    }
    mtiUpdateAmount (mti, dn, dn * sizeof (T));
  }
#endif

protected:
  /**
   * Initialize a region. This is a dangerous function to use because it
   * does not properly destruct the items in the array.
   */
  void InitRegion (size_t start, size_t count)
  {
    ElementHandler::InitRegion (root+start, count);
  }

private:
  /// Copy from one array to this one, properly constructing the copied items.
  void CopyFrom (const csArray& source)
  {
    if (&source != this)
    {
      DeleteAll ();
      threshold = source.threshold;
      SetSizeUnsafe (source.Length ());
      for (size_t i=0 ; i<source.Length() ; i++)
        ElementHandler::Construct (root + i, source[i]);
    }
  }

  /// Set the capacity of the array precisely to \c n elements.
  void InternalSetCapacity (size_t n)
  {
    if (root == 0)
    {
      root = MemoryAllocator::Alloc (n);
#ifdef CS_MEMORY_TRACKER
      UpdateMti (n, n);
#endif
    }
    else
    {
      root = MemoryAllocator::Realloc (root, count, capacity, n);
#ifdef CS_MEMORY_TRACKER
      UpdateMti (n-capacity, n);
#endif
    }
    capacity = n;
  }

  /**
   * Adjust capacity of this array to \c n elements rounded up to a multiple of
   * \c threshold.
   */
  void AdjustCapacity (size_t n)
  {
    if (n > capacity || (capacity > threshold && n < capacity - threshold))
    {
      InternalSetCapacity (((n + threshold - 1) / threshold ) * threshold);
    }
  }

  /**
   * Set array length.
   * \warning Do not make this public since it does not properly
   *   construct/destroy elements.  To safely truncate the array, use
   *   Truncate().  To safely set the capacity, use SetCapacity().
   */
  void SetSizeUnsafe (size_t n)
  {
    if (n > capacity)
      AdjustCapacity (n);
    count = n;
  }

public:
  /**
   * Compare two objects of the same type.
   * \param r1 Reference to first object.
   * \param r2 Reference to second object.
   * \return Zero if the objects are equal; less-than-zero if the first object
   *   is less than the second; or greater-than-zero if the first object is
   *   greater than the second.
   * \remarks Assumes the existence of T::operator<(T).  This is the default
   *   comparison function used by csArray for sorting if the client does not
   *   provide a custom function.
   */
  static int DefaultCompare(T const& r1, T const& r2)
  {
    return csOrdering<T,T>::Compare(r1,r2);
  }

  /**
   * Initialize object to have initial capacity of \c in_capacity elements, and
   * to increase storage by \c in_threshold each time the upper bound is
   * exceeded.
   */
  csArray (size_t in_capacity = 0, size_t in_threshold = 0)
  {
#ifdef CS_MEMORY_TRACKER
    mti = 0;
#endif
    count = 0;
    capacity = (in_capacity > 0 ? in_capacity : 0);
    threshold = (in_threshold > 0 ? in_threshold : 16);
    if (capacity != 0)
    {
      root = MemoryAllocator::Alloc (capacity);
#ifdef CS_MEMORY_TRACKER
      UpdateMti (capacity, capacity);
#endif
    }
    else
    {
      root = 0;
    }
  }

  /// Destroy array and all contained elements.
  ~csArray ()
  {
    DeleteAll ();
  }

  /// Copy constructor.
  csArray (const csArray& source)
  {
#ifdef CS_MEMORY_TRACKER
    mti = 0;
#endif
    root = 0;
    capacity = 0;
    count = 0;
    CopyFrom (source);
  }

  /// Assignment operator.
  csArray<T,ElementHandler>& operator= (const csArray& other)
  {
    CopyFrom (other);
    return *this;
  }

  /// Return the number of elements in the array.
  size_t GetSize () const
  {
    return count;
  }

  /**
   * Return the number of elements in the array.
   * \deprecated Use GetSize() instead.
   */
  size_t Length () const
  {
    return GetSize();
  }

  /// Query vector capacity.  Note that you should rarely need to do this.
  size_t Capacity () const
  {
    return capacity;
  }

  /**
   * Transfer the entire contents of one array to the other. The end
   * result will be that this array will be completely empty and the
   * other array will have all items that originally were in this array.
   * This operation is very efficient.
   */
  void TransferTo (csArray& destination)
  {
    if (&destination != this)
    {
      destination.DeleteAll ();
      destination.root = root;
      destination.count = count;
      destination.capacity = capacity;
      destination.threshold = threshold;
#ifdef CS_MEMORY_TRACKER
      destination.mti = mti;
      mti = 0;
#endif
      root = 0;
      capacity = count = 0;
    }
  }

  /**
   * Set the actual number of items in this array. This can be used to shrink
   * an array (like Truncate()) or to enlarge an array, in which case it will
   * properly construct all new items based on the given item.
   * \param n New array length.
   * \param what Object used as template to construct each newly added object
   *   using the object's copy constructor when the array size is increased by
   *   this method.
   */
  void SetSize (size_t n, T const& what)
  {
    if (n <= count)
    {
      Truncate (n);
    }
    else
    {
      size_t old_len = Length ();
      SetSizeUnsafe (n);
      for (size_t i = old_len ; i < n ; i++)
        ElementHandler::Construct (root + i, what);
    }
  }

  /**
   * Set the actual number of items in this array. This can be used to shrink
   * an array (like Truncate()) or to enlarge an array, in which case it will
   * properly construct all new items using their default (zero-argument)
   * constructor.
   * \param n New array length.
   */
  void SetSize (size_t n)
  {
    if (n <= count)
    {
      Truncate (n);
    }
    else
    {
      size_t old_len = Length ();
      SetSizeUnsafe (n);
      ElementHandler::InitRegion (root + old_len, n-old_len);
    }
  }

  /** @{ */
  /**
   * Set the actual number of items in this array.
   * \deprecated Use SetSize() instead.
   */
  void SetLength (size_t n, T const& what) { SetSize(n, what); }
  void SetLength (size_t n) { SetSize(n); }
  /** @} */

  /// Get an element (non-const).
  T& Get (size_t n)
  {
    CS_ASSERT (n < count);
    return root[n];
  }

  /// Get an element (const).
  T const& Get (size_t n) const
  {
    CS_ASSERT (n < count);
    return root[n];
  }

  /**
   * Get an item from the array. If the number of elements in this array is too
   * small the array will be automatically extended, and the newly added
   * objects will be created using their default (no-argument) constructor.
   */
  T& GetExtend (size_t n)
  {
    if (n >= count)
      SetSize (n+1);
    return root[n];
  }

  /// Get an element (non-const).
  T& operator [] (size_t n)
  {
    return Get(n);
  }

  /// Get a const reference.
  T const& operator [] (size_t n) const
  {
    return Get(n);
  }

  /// Insert a copy of element at the indicated position.
  void Put (size_t n, T const& what)
  {
    if (n >= count)
      SetSize (n+1);
    ElementHandler::Destroy (root + n);
    ElementHandler::Construct (root + n, what);
  }

  /**
   * Find an element based upon some arbitrary key, which may be embedded
   * within an element, or otherwise derived from it. The incoming key \e
   * functor defines the relationship between the key and the array's element
   * type.
   * \return csArrayItemNotFound if not found, else item index.
   */
  csArrayTemplate(K)
  size_t FindKey (csArrayCmpDecl(T,K) comparekey) const
  {
    for (size_t i = 0 ; i < Length () ; i++)
      if (csArrayCmpInvoke(comparekey, root[i]) == 0)
        return i;
    return csArrayItemNotFound;
  }

  /**
   * Push a copy of an element onto the tail end of the array.
   * \return Index of newly added element.
   */
  size_t Push (T const& what)
  {
    if (((&what >= root) && (&what < root + Length())) &&
      (capacity < count + 1))
    {
      /*
        Special case: An element from this very array is pushed, and a
	reallocation is needed. This could cause the passed ref to the
	element to be pushed to be read from deleted memory. Work
	around this.
       */
      size_t whatIndex = &what - root;
      SetSizeUnsafe (count + 1);
      ElementHandler::Construct (root + count - 1, root[whatIndex]);
    }
    else
    {
      SetSizeUnsafe (count + 1);
      ElementHandler::Construct (root + count - 1, what);
    }
    return count - 1;
  }

  /**
   * Push a element onto the tail end of the array if not already present.
   * \return Index of newly pushed element or index of already present element.
   */
  size_t PushSmart (T const& what)
  {
    size_t const n = Find (what);
    return (n == csArrayItemNotFound) ? Push (what) : n;
  }

  /// Pop an element from tail end of array.
  T Pop ()
  {
    CS_ASSERT (count > 0);
    T ret(root [count - 1]);
    ElementHandler::Destroy (root + count - 1);
    SetSizeUnsafe (count - 1);
    return ret;
  }

  /// Return the top element but do not remove it. (const)
  T const& Top () const
  {
    CS_ASSERT (count > 0);
    return root [count - 1];
  }

  /// Return the top element but do not remove it. (non-const)
  T& Top ()
  {
    CS_ASSERT (count > 0);
    return root [count - 1];
  }

  /// Insert element \c item before element \c n.
  bool Insert (size_t n, T const& item)
  {
    if (n <= count)
    {
      SetSizeUnsafe (count + 1); // Increments 'count' as a side-effect.
      size_t const nmove = (count - n - 1);
      if (nmove > 0)
	MemoryAllocator::MemMove (root, n+1, n, nmove);
      ElementHandler::Construct (root + n, item);
      return true;
    }
    else
      return false;
  }

  /**
   * Get the portion of the array between \c low and \c high inclusive.
   */
  csArray<T> Section (size_t low, size_t high) const
  {
    CS_ASSERT (high < count && high >= low);
    csArray<T> sect (high - low + 1);
    for (size_t i = low; i <= high; i++) sect.Push (root[i]);
    return sect;
  }

  /**
   * Find an element based on some key, using a comparison function.
   * \return csArrayItemNotFound if not found, else the item index.
   * \remarks The array must be sorted.
   */
  csArrayTemplate(K)
  size_t FindSortedKey (csArrayCmpDecl(T,K) comparekey,
                        size_t* candidate = 0) const
  {
    size_t m = 0, l = 0, r = Length ();
    while (l < r)
    {
      m = (l + r) / 2;
      int cmp = csArrayCmpInvoke(comparekey, root[m]);
      if (cmp == 0)
      {
        if (candidate) *candidate = csArrayItemNotFound;
        return m;
      }
      else if (cmp < 0)
        l = m + 1;
      else
        r = m;
    }
    if (candidate) *candidate = m;
    return csArrayItemNotFound;
  }

  /**
   * Insert an element at a sorted position, using an element comparison
   * function.
   * \return The index of the inserted item.
   * \remarks The array must be sorted.
   */
  size_t InsertSorted (const T& item,
    int (*compare)(T const&, T const&) = DefaultCompare,
    size_t* equal_index = 0)
  {
    size_t m = 0, l = 0, r = Length ();
    while (l < r)
    {
      m = (l + r) / 2;
      int cmp = compare (root [m], item);

      if (cmp == 0)
      {
	if (equal_index) *equal_index = m;
        Insert (++m, item);
        return m;
      }
      else if (cmp < 0)
        l = m + 1;
      else
        r = m;
    }
    if ((m + 1) == r)
      m++;
    if (equal_index) *equal_index = csArrayItemNotFound;
    Insert (m, item);
    return m;
  }

  /**
   * Find an element in array.
   * \return csArrayItemNotFound if not found, else the item index.
   * \warning Performs a slow linear search. For faster searching, sort the
   *   array and then use FindSortedKey().
   */
  size_t Find (T const& which) const
  {
    for (size_t i = 0 ; i < Length () ; i++)
      if (root[i] == which)
        return i;
    return csArrayItemNotFound;
  }

  /// An alias for Find() which may be considered more idiomatic by some.
  size_t Contains(T const& which) const
  { return Find(which); }

  /**
   * Given a pointer to an element in the array this function will return
   * the index. Note that this function does not check if the returned
   * index is actually valid. The caller of this function is responsible
   * for testing if the returned index is within the bounds of the array.
   */
  size_t GetIndex (const T* which) const
  {
    CS_ASSERT (which >= root);
    CS_ASSERT (which < (root + count));
    return which-root;
  }

  /**
   * Sort array using a comparison function.
   */
  void Sort (int (*compare)(T const&, T const&) = DefaultCompare)
  {
    qsort (root, Length(), sizeof(T),
      (int (*)(void const*, void const*))compare);
  }

  /**
   * Clear entire array, releasing all allocated memory.
   */
  void DeleteAll ()
  {
    if (root)
    {
      size_t i;
      for (i = 0 ; i < count ; i++)
        ElementHandler::Destroy (root + i);
      MemoryAllocator::Free (root);
#     ifdef CS_MEMORY_TRACKER
      UpdateMti (-capacity, 0);
#     endif
      root = 0;
      capacity = count = 0;
    }
  }

  /**
   * Truncate array to specified number of elements. The new number of
   * elements cannot exceed the current number of elements.
   * \remarks Does not reclaim memory used by the array itself, though the
   *   removed objects are destroyed. To reclaim the array's memory invoke
   *   ShrinkBestFit(), or DeleteAll() if you want to release all allocated
   *   resources.
   * <p>
   * \remarks The more general-purpose SetSize() method can also enlarge the
   *   array.
   */
  void Truncate (size_t n)
  {
    CS_ASSERT(n <= count);
    if (n < count)
    {
      for (size_t i = n; i < count; i++)
        ElementHandler::Destroy (root + i);
      SetSizeUnsafe(n);
    }
  }

  /**
   * Remove all elements.  Similar to DeleteAll(), but does not release memory
   * used by the array itself, thus making it more efficient for cases when the
   * number of contained elements will fluctuate.
   */
  void Empty ()
  {
    Truncate (0);
  }

  /**
   * Return true if the array is empty.
   * \remarks Rigidly equivalent to <tt>return GetSize() == 0</tt>, but more
   *   idiomatic.
   */
  bool IsEmpty() const
  {
    return GetSize() == 0;
  }

  /**
   * Set vector capacity to approximately \c n elements.
   * \remarks Never sets the capacity to fewer than the current number of
   *   elements in the array.  See Truncate() or SetSize() if you need to
   *   adjust the number of actual array elements.
   */
  void SetCapacity (size_t n)
  {
    if (n > Length ())
      InternalSetCapacity (n);
  }

  /**
   * Make the array just as big as it needs to be. This is useful in cases
   * where you know the array is not going to be modified anymore in order
   * to preserve memory.
   */
  void ShrinkBestFit ()
  {
    if (count == 0)
    {
      DeleteAll ();
    }
    else if (count != capacity)
    {
      root = MemoryAllocator::Realloc (root, count, capacity, count);
#ifdef CS_MEMORY_TRACKER
      UpdateMti (count-capacity, count);
#endif
      capacity = count;
    }
  }

  /**
   * Delete an element from the array.
   * return True if the indicated item index was valid, else false.
   * \remarks Deletion speed is proportional to the size of the array and the
   *   location of the element being deleted. If the order of the elements in
   *   the array is not important, then you can instead use DeleteIndexFast()
   *   for constant-time deletion.
  */
  bool DeleteIndex (size_t n)
  {
    if (n < count)
    {
      size_t const ncount = count - 1;
      size_t const nmove = ncount - n;
      ElementHandler::Destroy (root + n);
      if (nmove > 0)
	MemoryAllocator::MemMove (root, n, n+1, nmove);
      SetSizeUnsafe (ncount);
      return true;
    }
    else
      return false;
  }

  /**
   * Delete an element from the array in constant-time, regardless of the
   * array's size.
   * return True if the indicated item index was valid, else false.
   * \remarks This is a special version of DeleteIndex() which does not
   *   preserve the order of the remaining elements. This characteristic allows
   *   deletions to be performed in constant-time, regardless of the size of
   *   the array.
   */
  bool DeleteIndexFast (size_t n)
  {
    if (n < count)
    {
      size_t const ncount = count - 1;
      size_t const nmove = ncount - n;
      ElementHandler::Destroy (root + n);
      if (nmove > 0)
        MemoryAllocator::MemMove (root, n, ncount, 1);
      SetSizeUnsafe (ncount);
      return true;
    }
    else
      return false;
  }

  /**
   * Delete a given range (inclusive).
   * \remarks Will clamp \c start and \c end to the array limits.
   */
  void DeleteRange (size_t start, size_t end)
  {
    if (start >= count) return;
    // Treat 'csArrayItemNotFound' as invalid indices, do nothing.
    // @@@ Assert that?
    if (end == csArrayItemNotFound) return;
    if (start == csArrayItemNotFound) return;//start = 0;
    if (end >= count) end = count - 1;
    size_t i;
    for (i = start ; i <= end ; i++)
      ElementHandler::Destroy (root + i);

    size_t const range_size = end - start + 1;
    size_t const ncount = count - range_size;
    size_t const nmove = count - end - 1;
    if (nmove > 0)
      MemoryAllocator::MemMove (root, start, start + range_size, nmove);
    SetSizeUnsafe (ncount);
  }

  /**
   * Delete the given element from the array.
   * \remarks Performs a linear search of the array to locate \c item, thus it
   *   may be slow for large arrays.
   */
  bool Delete (T const& item)
  {
    size_t const n = Find (item);
    if (n != csArrayItemNotFound)
      return DeleteIndex (n);
    return false;
  }

  /**
   * Delete the given element from the array.
   * \remarks This is a special version of Delete() which does not
   *   preserve the order of the remaining elements. This characteristic allows
   *   deletions to be performed somewhat more quickly than by Delete(),
   *   however the speed gain is largely mitigated by the fact that a linear
   *   search is performed in order to locate \c item, thus this optimization
   *   is mostly illusory.
   * \deprecated The speed gain promised by this method is mostly illusory on
   *   account of the linear search for the item. In many cases, it will be
   *   faster to keep the array sorted, search for \c item using
   *   FindSortedKey(), and then remove it using the plain DeleteIndex().
   */
  bool DeleteFast (T const& item)
  {
    size_t const n = Find (item);
    if (n != csArrayItemNotFound)
      return DeleteIndexFast (n);
    return false;
  }

  /** Iterator for the Array<> class */
  class Iterator
  {
  public:
    /** Copy constructor. */
    Iterator(Iterator const& r) :
      currentelem(r.currentelem), array(r.array) {}

    /** Assignment operator. */
    Iterator& operator=(Iterator const& r)
    { currentelem = r.currentelem; array = r.array; return *this; }

    /** Returns true if the next Next() call will return an element */
    bool HasNext()
    { return currentelem < array.Length(); }

    /** Returns the next element in the array. */
    const T& Next()
    { return array.Get(currentelem++); }

    /** Reset the array to the first element */
    void Reset()
    { currentelem = 0; }

  protected:
    Iterator(const csArray<T, ElementHandler>& newarray)
	: currentelem(0), array(newarray) {}
    friend class csArray<T, ElementHandler>;

  private:
    size_t currentelem;
    const csArray<T, ElementHandler>& array;
  };

  /** Returns an Iterator which traverses the array. */
  Iterator GetIterator() const
  { return Iterator(*this); }
};

/**
 * Convenience class to make a version of csArray<> that does a
 * safe-copy in case of reallocation of the array. Useful for weak
 * references.
 */
template <class T>
class csSafeCopyArray
	: public csArray<T,
		csArrayElementHandler<T>,
		csSafeCopyArrayMemoryAllocator<T> >
{
public:
  /**
   * Initialize object to hold initially \c limit elements, and increase
   * storage by \c threshold each time the upper bound is exceeded.
   */
  csSafeCopyArray (size_t limit = 0, size_t threshold = 0)
  	: csArray<T, csArrayElementHandler<T>,
		     csSafeCopyArrayMemoryAllocator<T> > (limit, threshold)
  {
  }
};

#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# define new CS_EXTENSIVE_MEMDEBUG_NEW
#endif

#endif
