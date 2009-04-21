/*
    Crystal Space Data Buffer class
    Copyright (C) 2000 by Andrew Zabolotny

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

#ifndef __CS_DATABUF_H__
#define __CS_DATABUF_H__

/**\file
 * Implementation for iDataBuffer
 */

#include "csextern.h"
#include "csutil/allocator.h"
#include "csutil/scf_implementation.h"
#include "iutil/databuff.h"

namespace CS
{
  /**
   * This is an implementation of iDataBuffer interface.
   * The object is extremely lightweight and is recommended
   * for use in plugins as a mean to transparently exchange
   * abstract data between plugins.
   */
  template<class Allocator = Memory::AllocatorMalloc>
  class DataBuffer : public scfImplementation1<DataBuffer<Allocator>, 
                                               iDataBuffer>
  {
    /// The data buffer
    Memory::AllocatorPointerWrapper<char, Allocator> Data;
    /// Data size
    size_t Size;
    /// Should the buffer be deleted when we're done with it?
    bool do_delete;

  public:
    /// Construct an preallocated data buffer (filled with garbage initially)
    DataBuffer (size_t iSize)
      : scfImplementation1<DataBuffer, iDataBuffer> (this), Size (iSize), 
      do_delete (true)
    {
      Data.p = (char*)Data.Alloc (Size);
    }
    /// Construct an preallocated data buffer (filled with garbage initially)
    DataBuffer (size_t iSize, const Allocator& alloc)
      : scfImplementation1<DataBuffer, iDataBuffer> (this), Data (alloc), 
      Size (iSize), do_delete (true)
    {
      Data.p = (char*)Data.Alloc (Size);
    }


    /**
     * Construct an data buffer object given a existing pointer. The pointer
     * must be allocated by an allocator compatible to the given.
     */
    DataBuffer (char *iData, size_t iSize, bool should_delete = true)
      : scfImplementation1<DataBuffer, iDataBuffer> (this), Size (iSize), 
      do_delete (should_delete)
    {
      Data.p = iData; 
    }

    /**
     * Construct an data buffer object given a existing pointer. The pointer
     * must be allocated by an allocator compatible to the given.
     */
    DataBuffer (char *iData, size_t iSize, bool should_delete, 
                const Allocator& alloc)
      : scfImplementation1<DataBuffer, iDataBuffer> (this), Data (alloc),
        Size (iSize), do_delete (should_delete)
    {
      Data.p = iData; 
    }

    /// Duplicate an existing data buffer. Also appends a 0 char.
    DataBuffer (iDataBuffer *source, bool appendNull = true)
      : scfImplementation1<DataBuffer, iDataBuffer> (this), Size (source->GetSize()),
        do_delete (true)
    {
      if (appendNull)
      {
	Data.p = (char*)Data.Alloc (Size);
	memcpy (Data.p, source->GetData(), Size);
      }
      else
      {
	Data.p = (char*)Data.Alloc (Size+1);
	memcpy (Data.p, source->GetData(), Size);
	Data.p[Size] = 0;
      }
    }

    /// Destroy (free) the buffer
    virtual ~DataBuffer ()
    {
      if (do_delete)
        Data.Free (Data.p);
    }

    /// Query the buffer size
    virtual size_t GetSize () const
    { return Size; }

    /// Get the buffer as an abstract pointer
    virtual char* GetData () const
    { return Data.p; }

    /**
     * Return true if this databuffer will destroy its memory
     * on destruction.
     */
    bool GetDeleteOnDestruct () const { return do_delete; }
  };
} // namespace CS

/**
 * Standard data buffer, using new char[] for allocations.
 */
typedef CS::DataBuffer<CS::Memory::AllocatorNewChar<false > > csDataBuffer;

#endif // __CS_DATABUF_H__
