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

#ifndef __DATABUF_H__
#define __DATABUF_H__

#include "iutil/databuff.h"

/**
 * This is a implementation of iDataBuffer interface.
 * The object is extremely lightweight and is recommended
 * for use in plugins as a mean to transparently exchange
 * abstract data between plugins.
 */
class csDataBuffer : public iDataBuffer
{
  /// The data buffer
  char *Data;
  /// Data size
  size_t Size;

public:
  DECLARE_IBASE;

  /// Construct an preallocated data buffer (filled with garbage initially)
  csDataBuffer (size_t iSize)
  { CONSTRUCT_IBASE (NULL); Data = new char [Size = iSize]; }
  /// Construct an data buffer object given a existing (new char []) pointer
  csDataBuffer (char *iData, size_t iSize)
  { CONSTRUCT_IBASE (NULL); Data = iData; Size = iSize; }
  /// Destroy (free) the buffer
  virtual ~csDataBuffer ();

  /// Query the buffer size
  virtual size_t GetSize () const
  { return Size; }
  /// Get the buffer as an abstract pointer
  virtual csSome GetData () const
  { return Data; }
};

#endif // __DATABUF_H__
