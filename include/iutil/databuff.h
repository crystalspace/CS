/*
    Crystal Space Data Buffer interface
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

#ifndef __CS_IUTIL_DATABUFF_H__
#define __CS_IUTIL_DATABUFF_H__

/**\file
 * Data Buffer interface
 */
/**\addtogroup util
 * @{ */
#include "csutil/scf_interface.h"

/**
 * The iDataBuffer interface can be used to exchange buffers with
 * abstract data between plugins. The object does not care about the
 * contents of the buffer; it just can perform several simple operations
 * on them. The interface is designed to be light-weight, as an alternative
 * to the heavy iString interface.
 */
struct iDataBuffer : public virtual iBase
{
  SCF_INTERFACE(iDataBuffer, 2,0,0);
  /// Query the buffer size
  virtual size_t GetSize () const = 0;
  /// Get the buffer as an abstract pointer
  virtual char* GetData () const = 0;
  /// Get the buffer as an (char *) pointer
  inline char *operator * () const
  { return (char *)GetData (); }
  /// Get as an int8 *
  inline int8* GetInt8 ()
  { return (int8 *)GetData (); }
  /// Get as an uint8 *
  inline uint8* GetUint8 ()
  { return (uint8 *)GetData (); }
};
/** @} */

#endif // __CS_IUTIL_DATABUFF_H__
