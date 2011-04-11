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

#include "csutil/custom_new_disable.h"
#include <algorithm>
#include "csutil/custom_new_enable.h"

#include "csutil/allocator.h"
#include "csutil/comparator.h"
#include "csutil/customallocated.h"
#include "csutil/util.h"

#include "csutil/custom_new_disable.h"

#if defined(CS_MEMORY_TRACKER)
#include "csutil/memdebug.h"
#include "csutil/snprintf.h"
#include <typeinfo>
#endif

/**\addtogroup util_containers
 * @{ */

/**
 * A functor template which encapsulates a key and a comparison function for
 * use with key-related csArray searching methods, such as FindKey() and
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
   * contained in a csArray.  T is the type of the contained element.  K is
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
    { return csComparator<T,K>::Compare(r,k); }
private:
  K key;
  CF cmp;
};

/**
 * The default element handler for csArray.
 */
template <class T>
class csArrayElementHandler
{
public:
  /// Construct an element
  static void Construct (T* address)
  {
    new (static_cast<void*> (address)) T();
  }

  /// Copy-construct an element
  static void Construct (T* address, T const& src)
  {
    new (static_cast<void*> (address)) T(src);
  }

  /// Destroy an element
  static void Destroy (T* address)
  {
    address->~T();
  }

  /// Construct a number of elements
  static void InitRegion (T* address, size_t count)
  {
    for (size_t i = 0 ; i < count ; i++)
      Construct (address + i);
  }
  
  /// Reallocate a region allocated by \p Allocator.
  template<typename Allocator>
  static T* ResizeRegion (Allocator& alloc, T* mem, size_t relevantcount, 
    size_t oldcount, size_t newcount)
  {
    (void)relevantcount; 
    T* newp = (T*)alloc.Realloc (mem, newcount * sizeof(T));
    if (newp != 0) return newp;
    // Realloc() failed - allocate a new block
    newp = (T*)alloc.Alloc (newcount * sizeof(T));
    if (newcount < oldcount)
      memcpy (newp, mem, newcount * sizeof(T));
    else
      memcpy (newp, mem, oldcount * sizeof(T));
    alloc.Free (mem);
    return newp;
  }

  /// Move elements inside a region.
  static void MoveElements (T* mem, size_t dest, size_t src, size_t count)
  {
    memmove (mem + dest, mem + src, count * sizeof(T));
  }

  /**
   * Move elements inside a region.
   * \warning Only use if you _know_ source and destination regions don't overlap!
   */
  static void MoveElementsNoOverlap (T* mem, size_t dest, size_t src, size_t count)
  {
    memcpy (mem + dest, mem + src, count * sizeof(T));
  }
};

/**
 * Special element handler for csArray that makes sure that when
 * the array is reallocated that the objects are properly constructed
 * and destructed at their new position. This is needed for objects
 * that can not be safely moved around in memory (like weak references).
 * This is of course slower and that is the reason that this is not
 * done by default.
 */
template <class T>
class csArraySafeCopyElementHandler
{
public:
  static void Construct (T* address)
  {
    new (static_cast<void*> (address)) T();
  }

  static void Construct (T* address, T const& src)
  {
    new (static_cast<void*> (address)) T(src);
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
  
  /**
   * Reallocate a region allocated by \p Allocator. Ensure that all elements
   * are properly moved, ie they are copy-constructed at the new position 
   * in memory, the old instance is then destroyed.
   */
  template<typename Allocator>
  static T* ResizeRegion (Allocator& alloc, T* mem, size_t relevantcount, 
    size_t oldcount, size_t newcount)
  {
    if (newcount <= oldcount)
    {
      // Realloc is safe.
      T* newmem = (T*)alloc.Realloc (mem, newcount * sizeof (T));
      if (newmem != 0)
      {
	CS_ASSERT (newmem == mem);
	return newmem;
      }
      // else Realloc() failed (probably not supported) - allocate a new block
    }

    T* newmem = (T*)alloc.Alloc (newcount * sizeof (T));
    size_t i;
    for (i = 0 ; i < relevantcount ; i++)
    {
      Construct (newmem + i, mem[i]);
      Destroy (mem + i);
    }
    alloc.Free (mem);
    return newmem;
  }

  /**
   * Move elements inside a region. Ensure that all elements
   * are properly moved, ie they are copy-constructed at the new position 
   * in memory, the old instance is then destroyed.
   */
  static void MoveElements (T* mem, size_t dest, size_t src, size_t count)
  {
    size_t i;
    if (dest < src)
    {
      for (i = 0 ; i < count ; i++)
      {
        Construct (mem + dest + i, mem[src + i]);
	Destroy (mem + src + i);
      }
    }
    else
    {
      i = count;
      while (i > 0)
      {
	i--;
        Construct (mem + dest + i, mem[src + i]);
	Destroy (mem + src + i);
      }
    }
  }
  
  /// \copydoc MoveElements
  static void MoveElementsNoOverlap (T* mem, size_t dest, size_t src, size_t count)
  {
    MoveElements (mem, dest, src, count);
  }
};


/**
 * csArray variable threshold for capacity handlers.
 * The threshold is variable at run-time object construction.
 */
class csArrayThresholdVariable
{
  size_t threshold;
public:
  /// Construct with given threshold
  csArrayThresholdVariable (size_t in_threshold = 0)
  { threshold = (in_threshold > 0 ? in_threshold : 16); }
  /// Return the threshold.
  size_t GetThreshold() const { return threshold; }
};

/**
 * csArray fixed threshold for capacity handlers.
 * The threshold is specified at compile time, hence fixed at runtime.
 */
template<int N>
class csArrayThresholdFixed
{
public:
  /// Construct. \a x is ignored.
  csArrayThresholdFixed (size_t x = 0)
  { (void)x; }
  /// Return the threshold.
  size_t GetThreshold() const { return N; }
  // Work around VC7 bug apparently incorrectly copying this empty class
  csArrayThresholdFixed& operator= (const csArrayThresholdFixed&) 
  { return *this; }
};

/**
 * csArray capacity handler.
 * Different capacity handlers allow to realize different csArray internal
 * growth behaviours, if needed. This default "linear" handler will
 * result in array capacity growth to multiples of \a Threshold and works
 * well enough in most cases.
 */
template<typename Threshold = csArrayThresholdVariable>
class csArrayCapacityLinear : public Threshold
{
public:
  //@{
  /// Construct capacity handler
  csArrayCapacityLinear () : Threshold () {}
  csArrayCapacityLinear (const Threshold& threshold) : Threshold (threshold)
  {}
  //@}
  /**
   * Construct capacity handler with a given size_t parameter for the 
   * \a Threshold object. The exact meaning of \a x depends on the \a Threshold
   * implementation.
   * \remarks Mostly for compatibility with existing code.
   */
  csArrayCapacityLinear (const size_t x) : Threshold (x)
  {}

  /**
   * Return "true" if the given capacity is too large for the given count,
   * that is, if GetCapacity() would return a value for \a count smaller than 
   * \a capacity.
   */
  bool IsCapacityExcessive (size_t capacity, size_t count) const
  {
    return (capacity > this->GetThreshold() 
      && count < capacity - this->GetThreshold());
  }
  /**
   * Compute the capacity for a given number of items. The capacity will 
   * commonly be larger than the actual \a count to reduce the number of
   * memory allocations when pushing items onto an array.
   */
  size_t GetCapacity (size_t count) const
  {
    return ((count + this->GetThreshold() - 1) / this->GetThreshold()) * 
      this->GetThreshold();
  }
};

// Alias for csArrayCapacityLinear<csArrayThresholdVariable> to keep
// SWIG generated Java classes (and thus filenames) short enough for Windows.
// Note that a typedef wont work because SWIG would expand it.
struct csArrayCapacityVariableGrow :
  public csArrayCapacityLinear<csArrayThresholdVariable>
{
  csArrayCapacityVariableGrow () :
    csArrayCapacityLinear<csArrayThresholdVariable> () {}
  csArrayCapacityVariableGrow (const csArrayThresholdVariable& threshold) :
    csArrayCapacityLinear<csArrayThresholdVariable> (threshold) {}
  csArrayCapacityVariableGrow (const size_t x) :
    csArrayCapacityLinear<csArrayThresholdVariable> (x) {}
} ;
// @@@ Deprecate? Name is non-descriptive/misleading
typedef csArrayCapacityVariableGrow csArrayCapacityDefault;

/**
 * Shortcut for an array capacity handler with a compile-time fixed rate of 
 * growth
 */
template<int N>
struct csArrayCapacityFixedGrow :
  public csArrayCapacityLinear<csArrayThresholdFixed<N> >
{
  csArrayCapacityFixedGrow () :
    csArrayCapacityLinear<csArrayThresholdFixed<N> > () {}
};

namespace CS
{
  namespace Container
  {
    typedef CS::Memory::AllocatorMalloc ArrayAllocDefault;
    typedef csArrayCapacityFixedGrow<16> ArrayCapacityDefault;
    
    template<int MaxGrow = 1 << 20>
    struct ArrayCapacityExponential
    {
      bool IsCapacityExcessive (size_t capacity, size_t count) const
      {
        return size_t (csFindNearestPowerOf2 ((int)count)) < (capacity/2);
      }
      size_t GetCapacity (size_t count) const
      {
        size_t newCap = (size_t)csFindNearestPowerOf2 ((int)count);
        return newCap < MaxGrow ? newCap : MaxGrow;
      }
    };
  } // namespace Container
} // namespace CS

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
 * then you should consider csRefArray, which is more idiomatic than
 * csArray<csRef<iFoo> >.
 */
template <class T,
	class ElementHandler = csArrayElementHandler<T>,
        class MemoryAllocator = CS::Container::ArrayAllocDefault,
        class CapacityHandler = CS::Container::ArrayCapacityDefault>
class csArray : public CS::Memory::CustomAllocated
{
public:
  typedef csArray<T, ElementHandler, MemoryAllocator, CapacityHandler> ThisType;
  typedef T ValueType;
  typedef ElementHandler ElementHandlerType;
  typedef MemoryAllocator AllocatorType;
  typedef CapacityHandler CapacityHandlerType;

private:
  size_t count;
  /**
   * Class to eliminate overhead from possibly empty CapacityHandler.
   * See http://www.cantrip.org/emptyopt.html for details.
   */
  struct ArrayCapacity : public CapacityHandler
  {
    size_t c;
    ArrayCapacity (size_t in_capacity)
    { c = (in_capacity > 0 ? in_capacity : 0); }
    ArrayCapacity (size_t in_capacity, const CapacityHandler& ch) : 
      CapacityHandler (ch) 
    { c = (in_capacity > 0 ? in_capacity : 0); }
    void CopyFrom (const CapacityHandler& source)
    {
      CapacityHandler::operator= (source);
    }
  };
  ArrayCapacity capacity;
  CS::Memory::AllocatorPointerWrapper<T, MemoryAllocator> root;

protected:
  /**
   * Initialize a region. This is a dangerous function to use because it
   * does not properly destruct the items in the array.
   */
  void InitRegion (size_t start, size_t count)
  {
    ElementHandler::InitRegion (root.p+start, count);
  }
  
  /**
   * Set the internal pointer to the data.
   * \warning This is \em obviously dangerous.
   */
  void SetDataVeryUnsafe (T* data) { root.p = data; }
  /**
   * Set the internal array size.
   * \warning This is \em obviously dangerous.
   */
  void SetSizeVeryUnsafe (size_t n) { count = n; }
  /**
   * Set the internal array capacity.
   * \warning This is \em obviously dangerous.
   */
  void SetCapacityVeryUnsafe (size_t n) { capacity.c = n; }
private:
  /// Copy from one array to this one, properly constructing the copied items.
  void CopyFrom (const csArray& source)
  {
    capacity.CopyFrom (source.capacity);
    SetSizeUnsafe (source.GetSize ());
    for (size_t i=0 ; i<source.GetSize () ; i++)
      ElementHandler::Construct (root.p + i, source[i]);
  }

  /// Set the capacity of the array precisely to \c n elements.
  void InternalSetCapacity (size_t n)
  {
    if (root.p == 0)
    {
      root.p = (T*)root.Alloc (n * sizeof (T));
    }
    else
    {
      root.p = ElementHandler::ResizeRegion (root, root.p, count, capacity.c, n);
    }
    capacity.c = n;
  }

  /**
   * Adjust capacity of this array to \c n elements rounded up to a multiple of
   * \c threshold.
   */
  void AdjustCapacity (size_t n)
  {
    if (n > capacity.c || capacity.IsCapacityExcessive (capacity.c, n))
    {
      InternalSetCapacity (capacity.GetCapacity (n));
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
    if (n > capacity.c)
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
    return csComparator<T,T>::Compare(r1,r2);
  }

  /**
   * Initialize object to have initial capacity of \c in_capacity elements.
   * The storage increase depends on the specified capacity handler. The
   * default capacity handler accepts a threshold parameter by which the 
   * storage is increased each time the upper bound is exceeded.
   */
  csArray (size_t in_capacity = 0,
    const CapacityHandler& ch = CapacityHandler()) : count (0), 
    capacity (in_capacity, ch)
  {
#ifdef CS_MEMORY_TRACKER
    root.SetMemTrackerInfo (typeid(*this).name());
#endif
    if (capacity.c != 0)
    {
      root.p = (T*)root.Alloc (capacity.c * sizeof (T));
    }
    else
    {
      root.p = 0;
    }
  }
  /**
   * Initialize object to have initial capacity of \c in_capacity elements
   * and with specific memory allocator and capacity handler initializations.
   */
  csArray (size_t in_capacity, 
    const MemoryAllocator& alloc,
    const CapacityHandler& ch) : count (0), 
    capacity (in_capacity, ch), root (alloc)
  {
#ifdef CS_MEMORY_TRACKER
    root.SetMemTrackerInfo (typeid(*this).name());
#endif
    if (capacity.c != 0)
    {
      root.p = (T*)root.Alloc (capacity.c * sizeof (T));
    }
    else
    {
      root.p = 0;
    }
  }

  /// Destroy array and all contained elements.
  ~csArray ()
  {
    DeleteAll ();
  }

  /// Copy constructor.
  csArray (const csArray& source) : count (0), capacity (0), root (0)
  {
#ifdef CS_MEMORY_TRACKER
    root.SetMemTrackerInfo (typeid(*this).name());
#endif
    CopyFrom (source);
  }

  /// Assignment operator.
  csArray<T,ElementHandler,MemoryAllocator,CapacityHandler>& operator= (
    const csArray& other)
  {
    if (&other != this)
    {
      DeleteAll ();
      CopyFrom (other);
    }
    return *this;
  }

  /// Return the number of elements in the array.
  size_t GetSize () const
  {
    return count;
  }

  /// Query vector capacity.  Note that you should rarely need to do this.
  size_t Capacity () const
  {
    return capacity.c;
  }

  /**
   * Transfer the entire contents of one array to the other. The end
   * result will be that this array will be completely empty and the
   * other array will have all items that originally were in this array.
   * This operation is very efficient.
   */
  // @@@ FIXME: What about custom allocators?
  void TransferTo (csArray& destination)
  {
    if (&destination != this)
    {
      destination.DeleteAll ();
      destination.root.p = root.p;
      destination.count = count;
      destination.capacity = capacity;
      root.p = 0;
      capacity.c = count = 0;
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
      size_t old_len = GetSize ();
      SetSizeUnsafe (n);
      for (size_t i = old_len ; i < n ; i++)
        ElementHandler::Construct (root.p + i, what);
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
      size_t old_len = GetSize ();
      SetSizeUnsafe (n);
      ElementHandler::InitRegion (root.p + old_len, n-old_len);
    }
  }


  /// Get an element (non-const).
  T& Get (size_t n)
  {
    CS_ASSERT (n < count);
    return root.p[n];
  }

  /// Get an element (const).
  T const& Get (size_t n) const
  {
    CS_ASSERT (n < count);
    return root.p[n];
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
    return root.p[n];
  }

  /**
   * Get an item from the array. If the number of elements in this array is too
   * small the array will be automatically extended, and the newly added
   * objects will be constructed based on the given item \a what.
   */
  T& GetExtend (size_t n, T const& what)
  {
    if (n >= count)
      SetSize (n+1, what);
    return root.p[n];
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

  /**
   * Insert or reset a copy of the element \c what at the position with index \c n.
   * If the size of the array is smaller than \c n then it will be resized.
   */
  void Put (size_t n, T const& what)
  {
    if (n >= count)
      SetSize (n+1);
    ElementHandler::Destroy (root.p + n);
    ElementHandler::Construct (root.p + n, what);
  }

  /**
   * Find an element based upon some arbitrary key, which may be embedded
   * within an element, or otherwise derived from it. The incoming key \e
   * functor defines the relationship between the key and the array's element
   * type.
   * \return csArrayItemNotFound if not found, else item index.
   */
  template <class K>
  size_t FindKey (csArrayCmp<T,K> comparekey) const
  {
    for (size_t i = 0 ; i < GetSize () ; i++)
      if (comparekey (root.p[i]) == 0)
        return i;
    return csArrayItemNotFound;
  }

  /**
   * Push a copy of an element onto the tail end of the array.
   * \return Index of newly added element.
   */
  size_t Push (T const& what)
  {
    if (((&what >= root.p) && (&what < root.p + GetSize())) &&
      (capacity.c < count + 1))
    {
      /*
        Special case: An element from this very array is pushed, and a
	reallocation is needed. This could cause the passed ref to the
	element to be pushed to be read from deleted memory. Work
	around this.
       */
      size_t whatIndex = &what - root.p;
      SetSizeUnsafe (count + 1);
      ElementHandler::Construct (root.p + count - 1, root.p[whatIndex]);
    }
    else
    {
      SetSizeUnsafe (count + 1);
      ElementHandler::Construct (root.p + count - 1, what);
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
  
  /**
   * Push the elements of an array onto the tail end of the array.
   * 
   * \param origin The array to push at the end of this array.
   */
  void Merge(const csArray& origin)
  {
    for(size_t i = 0; i < origin.GetSize(); i++)
      Push(origin.Get(i));
  }
  
  /**
   * Push the elements of an array onto the tail end of the array if its elements aren't already present.
   * If an element is found duplicate it's skipped.
   * 
   * \param origin The array to push at the end of this array.
   */
  void MergeSmart(const csArray& origin)
  {
    for(size_t i = 0; i < origin.GetSize(); i++)
      PushSmart(origin.Get(i));
  }

  /// Pop an element from tail end of array.
  T Pop ()
  {
    CS_ASSERT (count > 0);
    T ret(root.p [count - 1]);
    ElementHandler::Destroy (root.p + count - 1);
    SetSizeUnsafe (count - 1);
    return ret;
  }

  /// Return the top element but do not remove it. (const)
  T const& Top () const
  {
    CS_ASSERT (count > 0);
    return root.p [count - 1];
  }

  /// Return the top element but do not remove it. (non-const)
  T& Top ()
  {
    CS_ASSERT (count > 0);
    return root.p [count - 1];
  }

  /// Insert element \c item before element \c n.
  bool Insert (size_t n, T const& item)
  {
    if (n <= count)
    {
      SetSizeUnsafe (count + 1); // Increments 'count' as a side-effect.
      size_t const nmove = (count - n - 1);
      if (nmove > 0)
        ElementHandler::MoveElements (root.p, n+1, n, nmove);
      ElementHandler::Construct (root.p + n, item);
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
    for (size_t i = low; i <= high; i++) sect.Push (root.p[i]);
    return sect;
  }

  /**
   * Find an element based on some key, using a comparison function.
   * \return csArrayItemNotFound if not found, else the item index.
   * \remarks The array must be sorted.
   */
  template <class K>
  size_t FindSortedKey (csArrayCmp<T,K> comparekey,
                        size_t* candidate = 0) const
  {
    size_t m = 0, l = 0, r = GetSize ();
    while (l < r)
    {
      m = (l + r) / 2;
      int cmp = comparekey (root.p[m]);
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
    if ((m + 1) == r)
      m++;
    if (candidate) *candidate = m;
    return csArrayItemNotFound;
  }

  /**
   * Insert an element at a sorted position, using an element comparison
   * function.
   * \param item The item to insert.
   * \param compare [optional] Pointer to a function to compare two elements.
   * \param equal_index [optional] Returns the index of an element equal to
   *   the one to be inserted, if one is found. csArrayItemNotFound otherwise.
   * \return The index of the inserted item.
   * \remarks The array must be sorted.
   */
  size_t InsertSorted (const T& item,
    int (*compare)(T const&, T const&) = DefaultCompare,
    size_t* equal_index = 0)
  {
    size_t m = 0, l = 0, r = GetSize ();
    while (l < r)
    {
      m = (l + r) / 2;
      int cmp = compare (root.p [m], item);

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
   * Find an element in this array.
   * \return csArrayItemNotFound if not found, else the item index.
   * \warning Performs a slow linear search. For faster searching, sort the
   *   array and then use FindSortedKey().
   */
  size_t Find (T const& which) const
  {
    for (size_t i = 0 ; i < GetSize () ; i++)
      if (root.p[i] == which)
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
    CS_ASSERT (which >= root.p);
    CS_ASSERT (which < (root.p + count));
    return which-root.p;
  }

  /**
   * Sort array using a comparison function.
   */
  void Sort (int (*compare)(T const&, T const&) = DefaultCompare)
  {
    qsort (root.p, GetSize (), sizeof(T),
      (int (*)(void const*, void const*))compare);
  }

  /**
   * Sort array using a binary predicate
   */
  template<typename Pred>
  void Sort (Pred& pred)
  {
    std::sort (root.p, root.p + GetSize (), pred);
  }

  /**
   * Sort array using a binary predicate and a stable sorting algorithm
   */
  template<typename Pred>
  void SortStable (Pred& pred)
  {
    std::stable_sort (root.p, root.p + GetSize (), pred);
  }

  /**
   * Clear the entire array, releasing all allocated memory.
   * \sa Empty()
   */
  void DeleteAll ()
  {
    if (root.p)
    {
      size_t i;
      for (i = 0 ; i < count ; i++)
        ElementHandler::Destroy (root.p + i);
      root.Free (root.p);
      root.p = 0;
      capacity.c = count = 0;
    }
  }

  /**
   * Truncate the array to the specified number of elements. The new number of
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
        ElementHandler::Destroy (root.p + i);
      SetSizeUnsafe(n);
    }
  }

  /**
   * Remove all elements. Similar to DeleteAll(), but does not release memory
   * used by the array itself, thus making it more efficient for cases when the
   * number of contained elements will fluctuate.
   */
  void Empty ()
  {
    Truncate (0);
  }

  /**
   * Return whether the array is empty or not.
   * \return True if the array is empty, false otherwise.
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
    if (n > GetSize ())
      InternalSetCapacity (n);
  }

  /**
   * Set vector capacity to at least \c n elements.
   * \remarks Never sets the capacity to fewer than the current number of
   *   elements in the array.  See Truncate() or SetSize() if you need to
   *   adjust the number of actual array elements. This function will also
   *   never shrink the current capacity.
   */
  void SetMinimalCapacity (size_t n)
  {
    if (n < Capacity ()) return;
    if (n > GetSize ())
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
    else if (count != capacity.c)
    {
      root.p = ElementHandler::ResizeRegion (root, root.p, count, capacity.c, count);
      capacity.c = count;
    }
  }

  /**
   * Delete an element from the array.
   * \return True if the indicated item index was valid, false otherwise.
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
      ElementHandler::Destroy (root.p + n);
      if (nmove > 0)
        ElementHandler::MoveElements (root.p, n, n+1, nmove);
      SetSizeUnsafe (ncount);
      return true;
    }
    else
      return false;
  }

  /**
   * Delete an element from the array in constant-time, regardless of the
   * array's size.
   * \return True if the indicated item index was valid, false otherwise.
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
      ElementHandler::Destroy (root.p + n);
      if (nmove > 0)
        ElementHandler::MoveElementsNoOverlap (root.p, n, ncount, 1);
      SetSizeUnsafe (ncount);
      return true;
    }
    else
      return false;
  }

  /**
   * Delete a given range (inclusive).
   * \remarks Will clamp \c start and \c end to the array limits.
   * \return false in case the inputs were invalid (csArrayItemNotFound)
   * or if the start is greater then the number of items in the array.
   */
  bool DeleteRange (size_t start, size_t end)
  {
    if (start >= count) return false;
    // Treat 'csArrayItemNotFound' as invalid indices, do nothing.
    if (end == csArrayItemNotFound) return false;
    if (start == csArrayItemNotFound) return false;//start = 0;
    if (end >= count) end = count - 1;
    size_t i;
    for (i = start ; i <= end ; i++)
      ElementHandler::Destroy (root.p + i);

    size_t const range_size = end - start + 1;
    size_t const ncount = count - range_size;
    size_t const nmove = count - end - 1;
    if (nmove > 0)
      ElementHandler::MoveElements (root.p, start, start + range_size, nmove);
    SetSizeUnsafe (ncount);
    return true;
  }

  /**
   * Delete the given element from the array.
   * \return True if the item has been found and deleted, false otherwise.
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

  /** Iterator for the csArray class */
  class Iterator
  {
  public:
    /** Copy constructor. */
    Iterator(Iterator const& r) :
      currentelem(r.currentelem), array(r.array) {}

    /** Assignment operator. */
    Iterator& operator=(Iterator const& r)
    { currentelem = r.currentelem; array = r.array; return *this; }

    /** Return true if the next Next() call will return an element */
    bool HasNext() const
    { return currentelem < array.GetSize (); }

    /** Return the next element in the array and increment the iterator. */
    T& Next()
    { return array.Get(currentelem++); }

    /** Reset the iterator to the first element */
    void Reset()
    { currentelem = 0; }

  protected:
    Iterator(csArray<T, ElementHandler, MemoryAllocator, CapacityHandler>& newarray)
	: currentelem(0), array(newarray) {}
    friend class csArray<T, ElementHandler, MemoryAllocator, CapacityHandler>;

  private:
    size_t currentelem;
    csArray<T, ElementHandler, MemoryAllocator, CapacityHandler>& array;
  };

  /** Iterator for the csArray class */
  class ConstIterator
  {
  public:
    /** Copy constructor. */
    ConstIterator(ConstIterator const& r) :
      currentelem(r.currentelem), array(r.array) {}

    /** Assignment operator. */
    ConstIterator& operator=(ConstIterator const& r)
    { currentelem = r.currentelem; array = r.array; return *this; }

    /** Return true if the next Next() call will return an element */
    bool HasNext() const
    { return currentelem < array.GetSize (); }

    /** Return the next element in the array. */
    const T& Next()
    { return array.Get(currentelem++); }

    /** Reset the array to the first element */
    void Reset()
    { currentelem = 0; }

  protected:
    ConstIterator(const csArray<T, ElementHandler, MemoryAllocator, CapacityHandler>& newarray)
      : currentelem(0), array(newarray) {}
    friend class csArray<T, ElementHandler, MemoryAllocator, CapacityHandler>;

  private:
    size_t currentelem;
    const csArray<T, ElementHandler, MemoryAllocator, CapacityHandler>& array;
  };

  /** Reverse iterator for the csArray class */
  class ReverseIterator
  {
  public:
    /** Copy constructor. */
    ReverseIterator(ReverseIterator const& r) :
      currentelem(r.currentelem), array(r.array) {}

    /** Assignment operator. */
    ReverseIterator& operator=(ReverseIterator const& r)
    { currentelem = r.currentelem; array = r.array; return *this; }

    /** Return true if the next Next() call will return an element */
    bool HasNext() const
    { return currentelem > 0 && currentelem <= array.GetSize (); }

    /** Return the next element in the array. */
    T& Next()
    { return array.Get(--currentelem); }

    /** Reset the array to the first element */
    void Reset()
    { currentelem = array.GetSize (); }

  protected:
    ReverseIterator(csArray<T, ElementHandler, MemoryAllocator, CapacityHandler>& newarray)
	: currentelem(newarray.GetSize ()), array(newarray) {}
    friend class csArray<T, ElementHandler, MemoryAllocator, CapacityHandler>;

  private:
    size_t currentelem;
    csArray<T, ElementHandler, MemoryAllocator, CapacityHandler>& array;
  };

  /** Reverse iterator for the csArray class */
  class ReverseConstIterator
  {
  public:
    /** Copy constructor. */
    ReverseConstIterator(ReverseConstIterator const& r) :
      currentelem(r.currentelem), array(r.array) {}

    /** Assignment operator. */
    ReverseConstIterator& operator=(ReverseConstIterator const& r)
    { currentelem = r.currentelem; array = r.array; return *this; }

    /** Return true if the next Next() call will return an element */
    bool HasNext() const
    { return currentelem > 0 && currentelem <= array.GetSize (); }

    /** Return the next element in the array. */
    const T& Next()
    { return array.Get(--currentelem); }

    /** Reset the array to the first element */
    void Reset()
    { currentelem = array.GetSize (); }

  protected:
    ReverseConstIterator(const csArray<T, ElementHandler, MemoryAllocator, CapacityHandler>& newarray)
      : currentelem(newarray.GetSize ()), array(newarray) {}
    friend class csArray<T, ElementHandler, MemoryAllocator, CapacityHandler>;

  private:
    size_t currentelem;
    const csArray<T, ElementHandler, MemoryAllocator, CapacityHandler>& array;
  };

  /** Return an Iterator which traverses the array. */
  Iterator GetIterator()
  { return Iterator(*this); }

  /** Return an Iterator which traverses the array. */
  ConstIterator GetIterator() const
  { return ConstIterator(*this); }

  /** Return an ReverseIterator which traverses the array in reverse direction. */
  ReverseIterator GetReverseIterator()
  { return ReverseIterator(*this); }

  /** Return an Iterator which traverses the array. */
  ReverseConstIterator GetReverseIterator() const
  { return ReverseConstIterator(*this); }
  
  /// Check if this array has the exact same contents as \a other.
  bool operator== (const csArray& other) const
  {
    if (other.GetSize() != GetSize()) return false;
    for (size_t i = 0; i < GetSize(); i++)
      if (!(Get (i) == other[i])) 
        return false;
    return true;
  }

  bool operator!= (const csArray& other) const { return !(*this==other); }

  /// Return a reference to the allocator of this array.
  const MemoryAllocator& GetAllocator() const
  {
    return root;
  }
};

/**
 * Convenience class to make a version of csArray that does a
 * safe-copy in case of reallocation of the array. Useful for weak
 * references.
 */
template <class T, 
          class Allocator = CS::Memory::AllocatorMalloc,
          class CapacityHandler = CS::Container::ArrayCapacityDefault>
class csSafeCopyArray
	: public csArray<T,
		csArraySafeCopyElementHandler<T>,
		Allocator, CapacityHandler>
{
public:
  /**
   * Initialize object to hold initially \c limit elements, and increase
   * storage by \c threshold each time the upper bound is exceeded.
   */
  csSafeCopyArray (size_t limit = 0,
    const CapacityHandler& ch = CapacityHandler())
  	: csArray<T, csArraySafeCopyElementHandler<T>, Allocator, 
  	          CapacityHandler> (limit, ch)
  {
  }
};

#include "csutil/custom_new_enable.h"

/** @} */

#endif
