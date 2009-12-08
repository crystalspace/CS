/*
  Copyright (C) 2009 by Frank Richter

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_CSUTIL_CHECKSUM_H__
#define __CS_CSUTIL_CHECKSUM_H__

#include "csextern.h"
#include "iutil/databuff.h"

namespace CS
{
  namespace Utility
  {
    /// Functions to compute checksums of data.
    struct Checksum
    {
      //@{
      /// Compute adler-32 checksum for given data buffer.
      static CS_CRYSTALSPACE_EXPORT uint32 Adler32 (void* data, size_t size);
      static inline uint32 Adler32 (iDataBuffer* data)
      {
	if (!data) return Adler32 ((void*)0, 0);
	return Adler32 (data->GetData(), data->GetSize());
      }
      //@}
      //@{
      /// Continue computing adler-32 checksum for given data buffer.
      static CS_CRYSTALSPACE_EXPORT uint32 Adler32 (uint32 prevCheckSum,
	void* data, size_t size);
      static inline uint32 Adler32 (uint32 prevCheckSum, iDataBuffer* data)
      {
	if (!data) return Adler32 (prevCheckSum, 0, 0);
	return Adler32 (prevCheckSum, data->GetData(), data->GetSize());
      }
      //@}

      //@{
      /// Compute crc-32 checksum for given data buffer.
      static CS_CRYSTALSPACE_EXPORT uint32 CRC32 (void* data, size_t size);
      static inline uint32 CRC32 (iDataBuffer* data)
      {
	if (!data) return CRC32 ((void*)0, 0);
	return CRC32 (data->GetData(), data->GetSize());
      }
      //@}
      //@{
      /// Continue computing crc-32 checksum for given data buffer.
      static CS_CRYSTALSPACE_EXPORT uint32 CRC32 (uint32 prevCheckSum,
	void* data, size_t size);
      static inline uint32 CRC32 (uint32 prevCheckSum, iDataBuffer* data)
      {
	if (!data) return CRC32 (prevCheckSum, 0, 0);
	return CRC32 (prevCheckSum, data->GetData(), data->GetSize());
      }
      //@}
    };
  } // namespace Utility
} // namespace CS

#endif // __CS_CSUTIL_CHECKSUM_H__
