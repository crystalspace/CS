/*
  Copyright (C) 2009-2011 by Frank Richter

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
    namespace Checksum
    {
      /**
       * Compute Adler-32 checksum for some data.
       * 
       * This class can be used in two ways, function-style (for checksumming
       * a single block of data) and as an instance (to checksum data in
       * multiple chunks).
       * 
       * Example for function-style use:
       * \code
       * uint32 check = CS::Utility::Checksum::Adler32 (data, dataSize);
       * \endcode
       * 
       * Example for use as an instance:
       * \code
       * CS::Utility::Checksum::Adler32 computeSum;
       * computeSum.Append (dataPart1, dataPart1Size);
       * // ... 
       * computeSum.Append (dataPart2, dataPart2Size);
       * uint32 checksum = computeSum.Finish();
       * \endcode
       */
      class Adler32
      {
	static inline uint32 Initial() { return 1; }
	static CS_CRYSTALSPACE_EXPORT uint32 Compute (uint32 prevCheckSum,
	  const void* data, size_t size);
	
	uint32 checksum;
	
      public:
	Adler32 () : checksum (Initial()) {}

	//@{
	/// Compute Adler-32 checksum for given data buffer.
	Adler32 (const void* data, size_t size) : checksum (Compute (Initial(), data, size)) {}
	Adler32 (iDataBuffer* data) 
	 : checksum (Compute (Initial(), data->GetData(), data->GetSize())) {}
	//@}
	
	//@{
	/**
	 * Continue computing Adler-32 checksum for given data buffer.
	 * \remarks It's recommended to use the Append() method to compute the
	 *   checksum over multiple chunks of data.
	 */
	Adler32 (uint32 prevCheckSum, const void* data, size_t size)
	 : checksum (Compute (prevCheckSum, data, size)) {}
	Adler32 (uint32 prevCheckSum, iDataBuffer* data)
	 : checksum (Compute (prevCheckSum, data->GetData(), data->GetSize())) {}
	//@}
	
	/// Feed in more data to checksum.
	void Append (const uint8* data, size_t size)
	{ checksum = Compute (checksum, data, size); }
	/// "Finish" checksum computation and return checksum over all data.
	uint32 Finish ()
	{ return checksum; }
	
	/// Return current checksum. For function-style use.
	operator uint32() const
	{ return checksum; }
      };

      /**
       * Compute CRC32 checksum for some data.
       * 
       * This class can be used in two ways, function-style (for checksumming
       * a single block of data) and as an instance (to checksum data in
       * multiple chunks).
       * 
       * Example for function-style use:
       * \code
       * uint32 check = CS::Utility::Checksum::CRC32 (data, dataSize);
       * \endcode
       * 
       * Example for use as an instance:
       * \code
       * CS::Utility::Checksum::CRC32 computeSum;
       * computeSum.Append (dataPart1, dataPart1Size);
       * // ... 
       * computeSum.Append (dataPart2, dataPart2Size);
       * uint32 checksum = computeSum.Finish();
       * \endcode
       */
      class CRC32
      {
	static inline uint32 Initial() { return 0; }
	static CS_CRYSTALSPACE_EXPORT uint32 Compute (uint32 prevCheckSum,
	  const void* data, size_t size);
	
	uint32 checksum;
	
      public:
	CRC32 () : checksum (Initial()) {}

	//@{
	/// Compute CRC-32 checksum for given data buffer.
	CRC32 (const void* data, size_t size) : checksum (Compute (Initial(), data, size)) {}
	CRC32 (iDataBuffer* data) 
	 : checksum (Compute (Initial(), data->GetData(), data->GetSize())) {}
	//@}
	
	//@{
	/**
	 * Continue computing CRC-32 checksum for given data buffer.
	 * \remarks It's recommended to use the Append() method to compute the
	 *   checksum over multiple chunks of data.
	 */
	CRC32 (uint32 prevCheckSum, const void* data, size_t size)
	 : checksum (Compute (prevCheckSum, data, size)) {}
	CRC32 (uint32 prevCheckSum, iDataBuffer* data)
	 : checksum (Compute (prevCheckSum, data->GetData(), data->GetSize())) {}
	//@}
	
	/// Feed in more data to checksum.
	void Append (const uint8* data, size_t size)
	{ checksum = Compute (checksum, data, size); }
	/// "Finish" checksum computation and return checksum over all data.
	uint32 Finish ()
	{ return checksum; }
	
	/// Return current checksum. For function-style use.
	operator uint32() const
	{ return checksum; }
      };

    } // namespace Checksum
  } // namespace Utility
} // namespace CS

#endif // __CS_CSUTIL_CHECKSUM_H__
