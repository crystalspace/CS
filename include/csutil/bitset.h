/*
    Copyright (C) 2000 by Andrew Zabolotny, <bit@eltech.ru>

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

#ifndef __CS_BITSET_H__
#define __CS_BITSET_H__

#include <stdlib.h>
#include <string.h>

/// How many bits there are in one byte (usually 8 :-)
#define BITS_PER_BYTE	8
/// Max value for a byte
#define MAX_BYTE_VALUE	0xff

// Processor-dependent macros
#if defined (PROC_X86) && defined (COMP_GCC) && !defined(OS_NEXT)
#  define BIT_SET(bits,index) \
   asm ("bts %1,%0" : : "o" (*bits), "r" (index))
#  define BIT_RESET(bits,index) \
   asm ("btr %1,%0" : : "o" (*bits), "r" (index))
#  define BIT_GET(bits,index) \
   ({ bool result; \
      asm ("bt %2,%1\nsetc %%al" : "=a" (result) : "o" (*bits), "r" (index)); \
      result; })
#else
#  define BIT_SET(bits,index) \
   bits [index / BITS_PER_BYTE] |= (1 << (index % BITS_PER_BYTE))
#  define BIT_RESET(bits,index) \
   bits [index / BITS_PER_BYTE] &= ~(1 << (index % BITS_PER_BYTE))
#  define BIT_GET(bits,index) \
   !!(bits [index / BITS_PER_BYTE] & (1 << (index % BITS_PER_BYTE)))
#endif

/**
 * A BitSet is an array of bits. The csBitSet class allow you to
 * allocate, resize and manipulate such an array.
 *<p>
 * The bit set is a highly effective way to store an array of booleans.
 * The implementation uses assembly whenever possible, and most methods
 * are inline, thus it is highly recommended to use it whenever possible.
 */
class csBitSet
{
protected:
  unsigned bit_count;
  unsigned byte_count;
  unsigned char *bits;
public:
  /// Create an empty bit set
  csBitSet () : bit_count (0), byte_count (0), bits (NULL)
  {
  }

  /// Create bit set of given size
  csBitSet (unsigned iBitCount) :
    bit_count (iBitCount), byte_count (0), bits (NULL)
  {
    if (iBitCount > 0)
    {
      byte_count = (iBitCount + BITS_PER_BYTE - 1) / BITS_PER_BYTE;
      bits = (unsigned char *)calloc (1, byte_count);
    }
  }

  /// Destroy the bit array
  ~csBitSet ()
  { free (bits); }

  /// Query number of bytes used to represent bit set
  unsigned GetByteCount () const
  { return byte_count; }

  /// Query number of bits represented by bit set
  unsigned GetBitCount () const
  { return bit_count; }

  /// Resize the array
  void Resize (unsigned iBitCount)
  {
    bit_count = iBitCount;
    unsigned old_bytes = byte_count;
    byte_count = (iBitCount + BITS_PER_BYTE - 1) / BITS_PER_BYTE;
    if (byte_count > 0)
    {
      if (!bits)
        bits = (unsigned char *)calloc (1, byte_count);
      else
      {
        bits = (unsigned char *)realloc (bits, byte_count);
        if (byte_count > old_bytes)
          memset (bits + old_bytes, 0, byte_count - old_bytes);
      }
    }
    else if (bits)
    {
      free (bits);
      bits = NULL;
    }
  }

  /// Get a pointer to entire array for custom manipulations
  unsigned char *GetBits () const
  { return bits; }

  /// Clear the entire array
  inline void Reset ()
  { memset (bits, 0, byte_count); }

  /// Set all the bits to one
  inline void Set ()
  { memset (bits, MAX_BYTE_VALUE, byte_count); }

  /// Set a bit in the array
  inline void Set (unsigned index)
  { BIT_SET (bits, index); }

  /// Set a number of bits in the array, starting with given index
  inline void Set (unsigned index, unsigned count)
  {
    unsigned char *start = bits + index / BITS_PER_BYTE;
    unsigned char smask = MAX_BYTE_VALUE << (index % BITS_PER_BYTE);
    unsigned char *end = bits + (index + count) / BITS_PER_BYTE;
    unsigned char emask =
      MAX_BYTE_VALUE >> (8 - (index + count) % BITS_PER_BYTE);
    if (start == end)
      *start |= smask & emask;
    else
    {
      *start++ |= smask;
      while (start < end)
        *start++ = MAX_BYTE_VALUE;
      if (emask) *start |= emask;
    }
  }

  /// Reset a bit in the array
  inline void Reset (unsigned index)
  { BIT_RESET (bits, index); }

  /// Set a number of bits in the array, starting with given index
  inline void Reset (unsigned index, unsigned count)
  {
    unsigned char *start = bits + index / BITS_PER_BYTE;
    unsigned char smask = MAX_BYTE_VALUE >> (8 - index % BITS_PER_BYTE);
    unsigned char *end = bits + (index + count) / BITS_PER_BYTE;
    unsigned char emask = MAX_BYTE_VALUE << ((index + count) % BITS_PER_BYTE);
    if (start == end)
      *start &= smask | emask;
    else
    {
      *start++ &= smask;
      while (start < end)
        *start++ = 0;
      if (emask != MAX_BYTE_VALUE) *start &= emask;
    }
  }

  /// Get the value of a bit in the array
  inline bool Get (unsigned index) const
  { return BIT_GET (bits, index); }

  /// Same but in a more nice form
  inline bool operator [] (unsigned index) const
  { return Get (index); }

  /// OR two bit sets together
  inline csBitSet &operator |= (csBitSet &bs)
  {
    unsigned sz = MIN (byte_count, bs.byte_count);
    uint32 *ldst = (uint32 *)bits;
    uint32 *lsrc = (uint32 *)bs.bits;
    while (sz >= sizeof (uint32))
    {
      *ldst++ |= *lsrc++;
      sz -= sizeof (uint32);
    }
    uint8 *bdst = (uint8 *)ldst;
    uint8 *bsrc = (uint8 *)lsrc;
    while (sz--)
      *bdst++ |= *bsrc++;
    return *this;
  }

  /// AND two bit sets together
  inline csBitSet &operator &= (csBitSet &bs)
  {
    unsigned sz = MIN (byte_count, bs.byte_count);
    uint32 *ldst = (uint32 *)bits;
    uint32 *lsrc = (uint32 *)bs.bits;
    while (sz >= sizeof (uint32))
    {
      *ldst++ &= *lsrc++;
      sz -= sizeof (uint32);
    }
    uint8 *bdst = (uint8 *)ldst;
    uint8 *bsrc = (uint8 *)lsrc;
    while (sz--)
      *bdst++ &= *bsrc++;
    return *this;
  }

  /**
   * Dump an ASCII representation of the bit set to a string.  Caller is
   * responsible for destroying the returned string with delete[].
   */
  char *Description() const
  { 
    char *s = new char [bit_count + 1];
    char *p = s;
    for (unsigned i = 0; i < bit_count; i++)
      *p++ = Get(i) ? '1' : '0';
    *p = '\0';
    return s;
  }
};

#endif // __CS_BITSET_H__
