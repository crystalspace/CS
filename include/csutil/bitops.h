/*
  Copyright (C) 2007 by Marten Svanfeldt

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

#ifndef __CSUTIL_BITOPS_H__
#define __CSUTIL_BITOPS_H__


namespace CS
{
  namespace Utility
  {
    /**
     * Bit operations
     */
    namespace BitOps
    {
    /**
     * Compute index of first bit set, scanned from LSB to MSB
     * \return true if any bit is found
     * \param value Input value
     * \param index Index of first bit set
     */
    CS_FORCEINLINE bool ScanBitForward (uint32 value, unsigned long& index)
    {
#if defined(CS_HAVE___BUILTIN_CTZ)
      index = __builtin_ctz (value);
      return value != 0;
#elif defined(CS_HAVE_BITSCAN_INTRINSICS)
      return _BitScanForward (&index, value) != 0;
#else
      // Generic c++ version
      index = 0;

      while (value)
      {
        if (value & 0x01)
        {
          return true;
        }
        value >>= 0x01;
        index++;
      }

      return false;
#endif
    }

    /**
     * Compute index of first bit set, scanned from MSB to LSB
     * \return true if any bit is found
     * \param value Input value
     * \param index Index of first bit set
     */
    CS_FORCEINLINE bool ScanBitReverse (uint32 value, unsigned long& index)
    {
#if defined(CS_HAVE___BUILTIN_CLZ)
      index = __builtin_clz (value);
      return value != 0;
#elif defined(CS_HAVE_BITSCAN_INTRINSICS)
      return _BitScanReverse (&index, value) != 0;
#else
      index = 0;

      while (value)
      {
        if (value & 0x80000000)
        {
          return true;
        }
        value <<= 0x01;
        index++;
      }

      return false;
#endif
    }

    /**
     * Compute number of bits set in given number
     */
    CS_FORCEINLINE uint32 ComputeBitsSet (uint32 v)
    {
#if defined(CS_HAVE___BUILTIN_POPCOUNT)
      return __builtin_popcount (v);
#else
      v = v - ((v >> 1) & 0x55555555);
      v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
      return (((v + (v >> 4)) & 0x0f0f0f0f) * 0x01010101) >> 24;
#endif
    }

    }
  }

}

#endif
