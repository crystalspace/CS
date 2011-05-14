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

#include "cssysdef.h"

#include "csutil/checksum.h"

#if defined(__cplusplus) && !defined(CS_COMPILER_BCC)
extern "C" {
#endif

#define Byte z_Byte	/* Kludge to avoid conflicting typedef in zconf.h */
#include <zlib.h>
#undef Byte

#if defined(__cplusplus) && !defined(CS_COMPILER_BCC)
}
#endif

namespace CS
{
  namespace Utility
  {

    namespace Checksum
    {
      uint32 Adler32::Compute (uint32 prevCheckSum, const void* data, size_t size)
      {
	return adler32 (prevCheckSum, (z_Byte*)data, (uInt)size);
      }

      uint32 CRC32::Compute (uint32 prevCheckSum, const void* data, size_t size)
      {
	return crc32 (prevCheckSum, (z_Byte*)data, (uInt)size);
      }

    } // namespace Checksum
  } // namespace Utility
} // namespace CS
