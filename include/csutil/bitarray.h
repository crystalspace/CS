/*
    Copyright (C) 2000 by Andrew Kirmse

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

// A one-dimensional array of bits, similar to STL bitset.
//
// Copyright 2000 Andrew Kirmse.  All rights reserved.
//
// Permission is granted to use this code for any purpose, as long as this
// copyright message remains intact.

#ifndef __CS_BITARRAY_H__
#define __CS_BITARRAY_H__

#include "csextern.h"

/// A one-dimensional array of bits, similar to STL bitset.
class CS_CSUTIL_EXPORT csBitArray
{
public:
  typedef unsigned long store_type;
private:
  enum
  {
    bits_per_byte = 8,
    cell_size     = sizeof(store_type) * bits_per_byte
  };

  store_type *mpStore;
  store_type mSingleWord; // Use this buffer when mLength is 1
  size_t mLength;       // Length of mpStore in units of store_type
  size_t mNumBits;

  /// Get the index and bit offset for a given bit number.
  static inline size_t GetIndex (size_t bit_num)
  {
    return bit_num / cell_size;
  }

  static inline size_t GetOffset (size_t bit_num)
  {
    return bit_num % cell_size;
  }

  void SetSize (size_t newSize)
  {
    size_t newLength;
    if (newSize == 0)
      newLength = 0;
    else
      newLength = 1 + GetIndex (newSize - 1);

    // Avoid allocation if length is 1 (common case)
    store_type* newStore;
    if (newLength <= 1)
      newStore = &mSingleWord;
    else
      newStore = new store_type[newLength];
    
    if (newLength > 0)
    {
      if (mLength > 0)
	memcpy (newStore, mpStore, 
	  (MIN (mLength, newLength)) * sizeof (store_type));
      else
	memset (newStore, 0, newLength * sizeof (store_type));
    }

    if (mLength > 1)
      delete mpStore;

    mpStore = newStore;
    mNumBits = newSize;
    mLength = newLength;
  }

  /// Force overhang bits at the end to 0
  inline void Trim()
  {
    size_t extra_bits = mNumBits % cell_size;
    if (mLength > 0 && extra_bits != 0)
      mpStore[mLength - 1] &= ~((~(store_type) 0) << extra_bits);
  }

public:
  /**
   * \internal Bit proxy (for csBitArray::operator[])
   */
  class CS_CSUTIL_EXPORT BitProxy
  {
  private:
    csBitArray &mArray;
    size_t mPos;
  public:
    BitProxy(csBitArray &array, unsigned pos): mArray(array), mPos(pos)
    {}

    BitProxy &operator= (bool value)
    {
      mArray.Set (mPos, value);
      return *this;
    }

    BitProxy &operator= (const BitProxy &that)
    {
      mArray.Set (mPos, that.mArray.IsBitSet (that.mPos));
      return *this;
    }

    operator bool() const
    {
      return mArray.IsBitSet (mPos);
    }

    bool Flip()
    {
      mArray.FlipBit (mPos);
      return mArray.IsBitSet (mPos);
    }
  };


  friend class BitProxy;

  //
  // Constructors and destructor
  //

  csBitArray () : mLength(0), mpStore(0)
  {
    SetSize (0);
    // Clear last bits
    Trim();
  }

  /**
   * Construct with a size of \a size bits.
   */
  explicit csBitArray(size_t size) : mLength(0), mpStore(0),
    mSingleWord(0)
  {
    SetSize (size);
    // Clear last bits
    Trim();
  }

  /**
   * construct as duplicate of \a that.
   */
  csBitArray (const csBitArray &that) : mLength(0), mpStore(0)
  {
    *this = that;
  }

  /// destructor
  virtual ~csBitArray()
  {
    if (mLength > 1)
      delete mpStore;
  }

  /// Return the number of bits store.
  size_t Length() const
  {
    return mNumBits;
  }

  void SetLength (size_t newSize)
  {
    SetSize (newSize);
    // Clear last bits
    Trim ();
  }

  //
  // Operators
  //

  /// copy from other array
  csBitArray &operator=(const csBitArray &that)
  {
    if (this != &that)
    {
      SetSize (that.mNumBits);

      memcpy (mpStore, that.mpStore, mLength * sizeof(store_type));
    }
    return *this;
  }

  /// return bit at position <code>pos</code>
  BitProxy operator[] (size_t pos)
  {
    CS_ASSERT (pos < mNumBits);
    return BitProxy(*this, pos);
  }

  /// return bit at position <code>pos</code>
  const BitProxy operator[] (size_t pos) const
  {
    CS_ASSERT (pos < mNumBits);
    return BitProxy(CS_CONST_CAST(csBitArray&,*this), pos);
  }

  /// equal to other array
  bool operator==(const csBitArray &that) const
  {
    if (mNumBits != that.mNumBits)
      return false;

    for (unsigned i = 0; i < mLength; i++)
      if (mpStore[i] != that.mpStore[i])
        return false;
    return true;
  }

  /// not equal to other array
  bool operator != (const csBitArray &that) const
  {
    return !(*this == that);
  }

  /// bit-wise and
  csBitArray& operator &= (const csBitArray &that)
  {
    CS_ASSERT (mNumBits == that.mNumBits);
    for (size_t i = 0; i < mLength; i++)
      mpStore[i] &= that.mpStore[i];
    return *this;
  }

  /// bit-wise or
  csBitArray operator |= (const csBitArray &that)
  {
    CS_ASSERT (mNumBits == that.mNumBits);
    for (size_t i = 0; i < mLength; i++)
      mpStore[i] |= that.mpStore[i];
    return *this;
  }

  /// bit-wise xor
  csBitArray operator ^= (const csBitArray &that)
  {
    CS_ASSERT (mNumBits == that.mNumBits);
    for (size_t i = 0; i < mLength; i++)
      mpStore[i] ^= that.mpStore[i];
    return *this;
  }

  /// Flip all bits
  csBitArray operator~() const
  {
    return csBitArray(*this).FlipAllBits();
  }

  /// bit-wise and
  friend csBitArray operator& (const csBitArray &a1, const csBitArray &a2)
  {
    return csBitArray(a1) &= a2;
  }

  /// bit-wise or
  friend csBitArray operator | (const csBitArray &a1, const csBitArray &a2)
  {
    return csBitArray(a1) |= a2;
  }

  /// bit-wise xor
  friend csBitArray operator ^ (const csBitArray &a1, const csBitArray &a2)
  {
    return csBitArray(a1) ^= a2;
  }

  //
  // Plain English interface
  //

  /// Set all bits to false.
  void Clear()
  {
    memset (mpStore, 0, mLength * sizeof(store_type));
  }

  /// Set the bit at position pos to true.
  void SetBit (size_t pos)
  {
    CS_ASSERT (pos < mNumBits);
    mpStore[GetIndex(pos)] |= 1 << GetOffset(pos);
  }

  /// Set the bit at position pos to false.
  void ClearBit (size_t pos)
  {
    CS_ASSERT (pos < mNumBits);
    mpStore[GetIndex(pos)] &= ~(1 << GetOffset(pos));
  }

  /// Toggle the bit at position pos.
  void FlipBit (size_t pos)
  {
    CS_ASSERT (pos < mNumBits);
    mpStore[GetIndex(pos)] ^= 1 << GetOffset(pos);
  }

  /// Set the bit at position pos to the given value.
  void Set (size_t pos, bool val)
  {
    val ? SetBit(pos) : ClearBit(pos);
  }

  /// Returns true iff the bit at position pos is true.
  bool IsBitSet (size_t pos) const
  {
    CS_ASSERT (pos < mNumBits);
    return (mpStore[GetIndex(pos)] & (1 << GetOffset(pos))) != 0;
  }

  /// Returns true iff all bits are false.
  bool AllBitsFalse() const
  {
    for (unsigned i=0; i < mLength; i++)
      if (mpStore[i] != 0)
        return false;
    return true;
  }

  /// Change value of all bits
  csBitArray &FlipAllBits()
  {
    for (unsigned i=0; i < mLength; i++)
      mpStore[i] = ~mpStore[i];

    Trim();
    return *this;
  }

  /// return the full array
  store_type* GetArrayBits()
  {
    return mpStore;
  }

  /**
   * Gets quick access to the single-word (only useful when the bit
   * array <= the word size of the machine.)
   */
  store_type GetSingleWord()
  {
    return mSingleWord;
  }

  /**
   * Sets the single-word very simply (only useful when the bit array <=
   * the word size of the machine.)
   */
  void SetSingleWord (store_type sw)
  {
    mSingleWord = sw;
  }
};

#endif // __CS_BITARRAY_H__
