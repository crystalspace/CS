/*
    Copyright (C) 2006 by Frank Richter

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

#ifndef __CS_CSUTIL_COWWRAPPER_H__
#define __CS_CSUTIL_COWWRAPPER_H__

#include "csutil/custom_new_disable.h"

#include "csutil/allocator.h"

/**\file
 * Copy-on-write wrapper for arbitrary types.
 */

namespace CS
{

  /**
   * Copy-on-write wrapper for arbitrary types.
   * <pre>
   *  _________________________________________
   * / Instances transparently maintain a      \
   * | reference-counted pointer to the actual |
   * | data; if an instance is copied, merely  |
   * | this pointer is copied.                 |
   * |                                         |
   * | As long as all access to the data is    |
   * | constant, the data is shared between    |
   * | instances. Only if non-const access is  |
   * | requested the data is duplicated.       |
   * |                                         |
   * \ The wrapper itself acts like a pointer. /
   *  -----------------------------------------
   *         \   ^__^
   *          \  (oo)\\_______
   *             (__)\       )\/\
   *                 ||----w |
   *                 ||     ||
   * </pre>
   *
   * \remarks Contained types must have a proper copy constructor.
   * \remarks Care should be taken that the data is accessed with proper 
   *   const-ness, since only the then data can be shared. Otherwise, data
   *   may be duplicated needlessly.
   */
  template<typename T, class MemoryAllocator = Memory::AllocatorMalloc>
  class CowWrapper
  {
    /// Helper class wrapping the actual data
    struct WrappedData
    {
      int refcount;
  
      void IncRef () { refcount++; }
      void DecRef () 
      { 
	refcount--; 
        if (refcount == 0) 
        {
          this->~WrappedData();
          MemoryAllocator::Free (this);
        }
      }
      int GetRefCount () const { return refcount; }
      
      T data;
    public:
      WrappedData (const T& other) : refcount (1), data (other) {}
      WrappedData (const WrappedData& other) : refcount (1), data (other.data) {}
    };
  public:
    /**
     * This is the size of the memory block the wrapper internally uses to
     * store the actual data. It is published to make fixed-size allocators
     * possible.
     */
    static const size_t allocSize = sizeof (WrappedData);
  private:
    csRef<WrappedData> data;
    inline WrappedData* NewData (const T& other)
    {
      WrappedData* p = (WrappedData*)MemoryAllocator::Alloc (allocSize);
      new (p) WrappedData (other);
      return p;
    }
  public:
    /// Copy reference to data from \p other.
    CowWrapper (const CowWrapper& other) : data (other.data) {}
    /// Create a new wrapper and initialize with \p other.
    CowWrapper (const T& other)
    {
      data.AttachNew (NewData (other));
    }

    /// Return a const reference to the contained data.
    const T& operator* () const
    {
      return data->data;
    }
    /**
     * Return a non-const reference to the contained data.
     * \remarks This will copy the contained data, so only use when really
     *   necessary.
     */
    T& operator* ()
    {
      if (data->GetRefCount() > 1)
      {
	WrappedData* newData = NewData (data->data);
        data.AttachNew (newData);
      }
      return data->data;
    }
    /// Return a const pointer to the contained data.
    const T* operator -> () const
    { return &(operator*()); }
    /**
     * Return a non-const pointer to the contained data.
     * \remarks This will copy the contained data, so only use when really
     *   necessary.
     */
    T* operator -> ()
    { return &(operator*()); }
  };

} // namespace CS

#include "csutil/custom_new_enable.h"

#endif // __CS_CSUTIL_COWWRAPPER_H__
