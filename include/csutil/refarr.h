/*
  Crystal Space Smart Pointers
  Copyright (C) 2002 by Jorrit Tyberghein and Matthias Braun
                2006 by Marten Svanfeldt

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

#ifndef __CS_REFARR_H__
#define __CS_REFARR_H__

/**\file
 * Smart Pointer array
 */

//-----------------------------------------------------------------------------
// Note *1*: The explicit "this->" is needed by modern compilers (all standard
// compilant ones, such as gcc after version 3.4.x and MSVC after 7.1) 
// which distinguish between dependent and non-dependent names in
// templates.  See: http://gcc.gnu.org/onlinedocs/gcc/Name-lookup.html or
// section 14.6 of ISO C++ standard (ISO 14882-1998)
//-----------------------------------------------------------------------------

#include "csutil/array.h"
#include "csutil/ref.h"

#ifdef CS_REF_TRACKER
 #include <typeinfo>
 #include "csutil/reftrackeraccess.h"

 #define CSREFARR_TRACK(x, cmd, refCount, obj, tag) \
  {						    \
    const int rc = obj ? refCount : -1;		    \
    if (obj) cmd;				    \
    if (obj)					    \
    {						    \
      csRefTrackerAccess::SetDescription (obj,	    \
	typeid(T).name());			    \
      csRefTrackerAccess::Match ## x (obj, rc, tag);\
    }						    \
  }
 #define CSREFARR_TRACK_INCREF(obj,tag)	\
  CSREFARR_TRACK(IncRef, obj->IncRef(), obj->GetRefCount(), obj, tag);
 #define CSREFARR_TRACK_DECREF(obj,tag)	\
  CSREFARR_TRACK(DecRef, obj->DecRef(), obj->GetRefCount(), obj, tag);
#else
 #define CSREFARR_TRACK_INCREF(obj,tag) \
  if (obj) obj->IncRef();
 #define CSREFARR_TRACK_DECREF(obj,tag) \
  if (obj) obj->DecRef();
#endif

template <class T>
class csRefArrayElementHandler : public csArrayElementHandler<T>
{
public:
  static void Construct (T* address, T const& src)
  {
    *address = src;
    CSREFARR_TRACK_INCREF (src, address);
  }

  static void Destroy (T* address)
  {
    CSREFARR_TRACK_DECREF ((*address), address);
  }

  static void InitRegion (T* address, size_t count)
  {
    memset (address, 0, count*sizeof (T));
  }
};

/**
 * An array of smart pointers.
 */
template <class T, class Allocator = CS::Memory::AllocatorMalloc>
class csRefArray : public csArray<T*, csRefArrayElementHandler<T*>, Allocator>
{
public:
  typedef csRefArray<T, Allocator> ThisType;
  typedef T ValueType;
  typedef Allocator AllocatorType;
  typedef csArray<T*, csRefArrayElementHandler<T*>, Allocator> BaseType;

  /**
   * Initialize object to hold initially 'ilimit' elements, and increase
   * storage by 'ithreshold' each time the upper bound is exceeded.
   */
  csRefArray (int ilimit = 0, int ithreshold = 0)
    : csArray<T*, csRefArrayElementHandler<T*>, Allocator> (ilimit, ithreshold)
  {
  }

  /// Pop an element from tail end of array.
  csPtr<T> Pop ()
  {
    CS_ASSERT (this->Length () > 0);
    csRef<T> ret (this->Get (this->Length () - 1)); // see *1*
    SetLength (this->Length () - 1);
    return csPtr<T> (ret);
  }

  /**
   * Internal proxy-class for access to elements (via operator[] etc)
   */
  class RefProxy
  {
  public:
    /// Constructor
    RefProxy (csRefArray& array, size_t pos)
      : ownerArray (array), position (pos)
    {
    }

    /// Assignment operators
    RefProxy& operator= (T* value)
    {
      ownerArray.Put (position, value);
      return *this;
    }

    RefProxy& operator= (const RefProxy& other)
    {
      ownerArray.Put (position, other.ownerArray.BaseType::Get (other.position));
      return *this;
    }

    /// Object accessor (const version)
    operator const T* () const
    {
      return ownerArray.BaseType::Get (position);
    }
  
    /// Object accessor (non-const version)
    operator T* () const
    {
      return ownerArray.BaseType::Get (position);
    }

    /// Dereference underlying object.
    T* operator -> () const
    { 
      return ownerArray.BaseType::Get (position); 
    }

    /// Dereference underlying object.
    T& operator* () const
    { 
      return *ownerArray.BaseType::Get (position); 
    }

    /// Comparison (inequality)
    bool operator!= (T* other) const
    {
      return ownerArray.BaseType::Get (position) != other;
    }

    /// Comparison (equality)
    bool operator== (T* other) const
    {
      return ownerArray.BaseType::Get (position) == other;
    }

  private:
    csRefArray& ownerArray;
    size_t position;
  };

  // Hide the csArray copies of these and provide our own

  /// Get an element (non-const)
  RefProxy operator [] (size_t n)
  {
    return this->Get (n);
  }

  /// Get an element (const)
  T* const operator [] (size_t n) const
  {
    return BaseType::Get (n);
  }

  /// Get an element (non-const)
  RefProxy Get (size_t n)
  {
    CS_ASSERT(n < this->GetSize ());
    return RefProxy (*this, n); 
  }

  /// Get an element (const)
  T* const Get (size_t n) const
  {
    return BaseType::Get (n);
  }

  /**
   * Get an item from the array. If the number of elements in this array is too
   * small the array will be automatically extended.
   */
  RefProxy GetExtend (size_t n)
  {
    if (n >= this->GetSize ())
      this->SetSize (n+1);
    return RefProxy (*this, n);
  }
};

#undef CSREFARR_TRACK_INCREF
#undef CSREFARR_TRACK_DECREF

#endif // __CS_REFARR_H__
