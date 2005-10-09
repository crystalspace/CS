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
#include "csutil/scf_implementation.h"
#include "iutil/databuff.h"

/**
 * This is a implementation of iDataBuffer interface.
 * The object is extremely lightweight and is recommended
 * for use in plugins as a mean to transparently exchange
 * abstract data between plugins.
 */
class CS_CRYSTALSPACE_EXPORT csDataBuffer : 
  public scfImplementation1<csDataBuffer, iDataBuffer>
{
  /// The data buffer
  char *Data;
  /// Data size
  size_t Size;
  /// Should the buffer be deleted when we're done with it?
  bool do_delete;

public:
  /// Construct an preallocated data buffer (filled with garbage initially)
  csDataBuffer (size_t iSize)
    : scfImplementationType (this)
  {
    Size = iSize;
    Data = new char [Size]; 
    do_delete = true;
  }

  /// Construct an data buffer object given a existing (new char []) pointer
  csDataBuffer (char *iData, size_t iSize, bool should_delete = true)
    : scfImplementationType (this)
  {
    Data = iData; 
    Size = iSize; 
    do_delete = should_delete;
  }

  /// Duplicate an existing data buffer. Also appends a 0 char.
  csDataBuffer (iDataBuffer *source)
    : scfImplementationType (this)
  {
    Size = source->GetSize();
    Data = new char [Size + 1];
    memcpy (Data, source->GetData(), Size);
    Data[Size] = 0;
    do_delete = true;
  }

  /// Destroy (free) the buffer
  virtual ~csDataBuffer ()
  {
    if (do_delete)
      delete [] Data;
  }

  /// Query the buffer size
  virtual size_t GetSize () const
  { return Size; }

  /// Get the buffer as an abstract pointer
  virtual char* GetData () const
  { return Data; }
};

#endif // __CS_DATABUF_H__
