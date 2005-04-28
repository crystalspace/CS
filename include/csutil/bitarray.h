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

/**\file
 * A one-dimensional array of bits, similar to STL bitset.
 */

#include "csextern.h"
#include "comparator.h"
#include "hash.h"

class csBitArray;
CS_SPECIALIZE_TEMPLATE class csComparator<csBitArray, csBitArray>;
CS_SPECIALIZE_TEMPLATE class csHashComputer<csBitArray>;

/**
 * A one-dimensional array of bits, similar to STL bitset.
 */
class CS_CRYSTALSPACE_EXPORT csBitArray
{
public:
  typedef unsigned long store_type;

private:
  friend class csComparator<csBitArray, csBitArray>;
  friend class csHashComputer<csBitArray>;
  enum
  {
    bits_per_byte = 8,
    cell_size     = sizeof(store_type) * bits_per_byte
  };

  store_type* mpStore;
  store_type  mSingleWord; // Use this buffer when mLength is 1
  size_t mLength;          // Length of mpStore in units of store_type
  size_t mNumBits;

  /// Get the GetStore()[] index for a given bit number.
  static size_t GetIndex (size_t bit_num)
  {
    return bit_num / cell_size;
  }

  /// Get the offset within GetStore()[GetIndex()] for a given bit number.
  static size_t GetOffset (size_t bit_num)
  {
    return bit_num % cell_size;
  }

  /**
   * Get a constant pointer to bit store, which may be internal mSingleWord or
   * heap-allocated mpStore.
   */
  store_type const* GetStore() const
  {
    return mLength <= 1 ? &mSingleWord : mpStore;
  }

  /**
   * Get a non-constant pointer to bit store, which may be internal mSingleWord
   * or heap-allocated mpStore.
   */
  store_type* GetStore()
  {
    return mLength <= 1 ? &mSingleWord : mpStore;
  }

  /// Force overhang bits at the end to 0.
  void Trim()
  {
    size_t extra_bits = mNumBits % cell_size;
    if (mLength > 0 && extra_bits != 0)
      GetStore()[mLength - 1] &= ~((~(store_type)0) << extra_bits);
  }

  /**
   * Resize the array to \a newSize bits. If this operations enlarges the
   * array, the newly added bits are cleared.
   */
  void SetSize (size_t newSize)
  {
    size_t newLength;
    if (newSize == 0)
      newLength = 0;
    else
      newLength = 1 + GetIndex (newSize - 1);

    if (newLength != mLength)
    {
      // Avoid allocation if length is 1 (common case)
      store_type* newStore;
      if (newLength <= 1)
	newStore = &mSingleWord;
      else
	newStore = new store_type[newLength];

      if (newLength > 0)
      {
	if (mLength > 0)
	{
	  store_type const* oldStore = GetStore();
	  if (newStore != oldStore)
	  {
	    memcpy (newStore, oldStore, 
	      (MIN (mLength, newLength)) * sizeof (store_type));
	    if (newLength > mLength)
	      memset(newStore + mLength, 0,
		     (newLength - mLength) * sizeof (store_type));
	  }
	}
	else
	  memset (newStore, 0, newLength * sizeof (store_type));
      }

      if (mpStore != 0)
	delete[] mpStore;

      mpStore = newLength > 1 ? newStore : 0;
      mLength = newLength;
    }

    mNumBits = newSize;
    Trim();
  }

public:
  /**
   * \internal Bit proxy (for csBitArray::operator[])
   */
  class CS_CRYSTALSPACE_EXPORT BitProxy
  {
  private:
    csBitArray &mArray;
    size_t mPos;
  public:
    /// Constructor.
    BitProxy(csBitArray &array, size_t pos): mArray(array), mPos(pos)
    {}

    /// Boolean assignment.
    BitProxy &operator= (bool value)
    {
      mArray.Set (mPos, value);
      return *this;
    }

    /// Proxy assignment.
    BitProxy &operator= (const BitProxy &that)
    {
      mArray.Set (mPos, that.mArray.IsBitSet (that.mPos));
      return *this;
    }

    /// Boolean accessor.
    operator bool() const
    {
      return mArray.IsBitSet (mPos);
    }

    /**
     * Flip state of this bit.
     * \return New state of bit.
     */
    bool Flip()
    {
      mArray.FlipBit (mPos);
      return mArray.IsBitSet (mPos);
    }
  };
  friend class BitProxy;

  /**
   * Default constructor.
   */
  csBitArray () : mpStore(0), mSingleWord(0), mLength(0), mNumBits(0)
  {
    SetSize (0);
  }

  /**
   * Construct with a size of \a size bits.
   */
  explicit csBitArray(size_t size) :
    mpStore(0), mSingleWord(0), mLength(0), mNumBits(0)
  {
    SetSize (size);
  }

  /**
   * Construct as duplicate of \a that (copy constructor).
   */
  csBitArray (const csBitArray &that) :
    mpStore(0), mSingleWord(0), mLength(0), mNumBits(0)
  {
    *this = that; // Invokes this->operator=().
  }

  /// Destructor.
  ~csBitArray()
  {
    if (mpStore != 0)
      delete[] mpStore;
  }

  /// Return the number of stored bits.
  size_t Length() const
  {
    return mNumBits;
  }

  /**
   * Set the number of stored bits.
   * \remarks If the new size is larger than the old size, the newly added
   *    bits are cleared.
   */
  void SetLength (size_t newSize)
  {
    SetSize (newSize);
  }

  //
  // Operators
  //

  /// Copy from other array.
  csBitArray &operator=(const csBitArray &that)
  {
    if (this != &that)
    {
      SetSize (that.mNumBits);
      memcpy (GetStore(), that.GetStore(), mLength * sizeof(store_type));
    }
    return *this;
  }

  /// Return bit at position \a pos.
  BitProxy operator[] (size_t pos)
  {
    CS_ASSERT (pos < mNumBits);
    return BitProxy(*this, pos);
  }

  /// Return bit at position \a pos.
  bool operator[] (size_t pos) const
  {
    return IsBitSet(pos);
  }

  /// Equal to other array?
  bool operator==(const csBitArray &that) const
  {
    if (mNumBits != that.mNumBits)
      return false;

    store_type const* p0 = GetStore();
    store_type const* p1 = that.GetStore();
    for (unsigned i = 0; i < mLength; i++)
      if (p0[i] != p1[i])
        return false;
    return true;
  }

  /// Not equal to other array?
  bool operator != (const csBitArray &that) const
  {
    return !(*this == that);
  }

  /// Bit-wise `and'. The arrays must be the same length.
  csBitArray& operator &= (const csBitArray &that)
  {
    CS_ASSERT (mNumBits == that.mNumBits);
    store_type* p0 = GetStore();
    store_type const* p1 = that.GetStore();
    for (size_t i = 0; i < mLength; i++)
      p0[i] &= p1[i];
    return *this;
  }

  /// Bit-wise `or'. The arrays must be the same length.
  csBitArray operator |= (const csBitArray &that)
  {
    CS_ASSERT (mNumBits == that.mNumBits);
    store_type* p0 = GetStore();
    store_type const* p1 = that.GetStore();
    for (size_t i = 0; i < mLength; i++)
      p0[i] |= p1[i];
    return *this;
  }

  /// Bit-wise `xor'. The arrays must be the same length.
  csBitArray operator ^= (const csBitArray &that)
  {
    CS_ASSERT (mNumBits == that.mNumBits);
    store_type* p0 = GetStore();
    store_type const* p1 = that.GetStore();
    for (size_t i = 0; i < mLength; i++)
      p0[i] ^= p1[i];
    return *this;
  }

  /// Return complement bit array in which all bits are flipped from this one.
  csBitArray operator~() const
  {
    return csBitArray(*this).FlipAllBits();
  }

  /// Bit-wise `and'. The arrays must be the same length.
  friend csBitArray operator& (const csBitArray &a1, const csBitArray &a2)
  {
    return csBitArray(a1) &= a2;
  }

  /// Bit-wise `or'. The arrays must be the same length.
  friend csBitArray operator | (const csBitArray &a1, const csBitArray &a2)
  {
    return csBitArray(a1) |= a2;
  }

  /// Bit-wise `xor'. The arrays must be the same length.
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
    memset (GetStore(), 0, mLength * sizeof(store_type));
  }

  /// Set the bit at position pos to true.
  void SetBit (size_t pos)
  {
    CS_ASSERT (pos < mNumBits);
    GetStore()[GetIndex(pos)] |= ((store_type)1) << GetOffset(pos);
  }

  /// Set the bit at position pos to false.
  void ClearBit (size_t pos)
  {
    CS_ASSERT (pos < mNumBits);
    GetStore()[GetIndex(pos)] &= ~(((store_type)1) << GetOffset(pos));
  }

  /// Toggle the bit at position pos.
  void FlipBit (size_t pos)
  {
    CS_ASSERT (pos < mNumBits);
    GetStore()[GetIndex(pos)] ^= ((store_type)1) << GetOffset(pos);
  }

  /// Set the bit at position \a pos to the given value.
  void Set (size_t pos, bool val = true)
  {
    if (val)
      SetBit(pos);
    else
      ClearBit(pos);
  }

  /// Returns true if the bit at position \a pos is true.
  bool IsBitSet (size_t pos) const
  {
    CS_ASSERT (pos < mNumBits);
    return (GetStore()[GetIndex(pos)] & (((store_type)1) << GetOffset(pos)))
      != 0;
  }

  /// Checks whether at least one of \a count bits is set starting at \a pos.
  bool AreSomeBitsSet (size_t pos, size_t count) const
  {
    CS_ASSERT (pos + count <= mNumBits);
    store_type const* p = GetStore();
    while (count > 0)
    {
      size_t index = GetIndex (pos);
      size_t offset = GetOffset (pos);
      size_t checkCount = MIN(count, cell_size - offset);
      store_type mask = ((checkCount == cell_size) ? ~(store_type)0 :
			 ((((store_type)1) << checkCount) - 1)) << offset;
      if (p[index] & mask)
	return true;
      pos += checkCount;
      count -= checkCount;
    }
    return false;
  }
  
  /// Returns true if all bits are false.
  bool AllBitsFalse() const
  {
    store_type const* p = GetStore();
    for (size_t i = 0; i < mLength; i++)
      if (p[i] != 0)
        return false;
    return true;
  }

  /// Change value of all bits
  csBitArray &FlipAllBits()
  {
    store_type* p = GetStore();
    for (size_t i = 0; i < mLength; i++)
      p[i] = ~p[i];
    Trim();
    return *this;
  }

  /**
   * Delete from the array \a count bits starting at \a pos, making the array
   * shorter.
   */
  void Delete(size_t pos, size_t count)
  {
    if (count > 0)
    {
      size_t dst = pos;
      size_t src = pos + count;
      CS_ASSERT(src <= mNumBits);
      size_t ntail = mNumBits - src;
      while (ntail-- > 0)
	Set(dst++, IsBitSet(src++));
      SetSize(mNumBits - count);
    }
  }

  /**
   * Return a new bit array containing a slice \a count bits in length from
   * this array starting at \a pos. Does not modify this array.
   */
  csBitArray Slice(size_t pos, size_t count) const
  {
    CS_ASSERT(pos + count <= mNumBits);
    csBitArray a(count);
    for (size_t i = pos, o = 0; i < pos + count; i++)
      if (IsBitSet(i))
	a.SetBit(o++);
    return a;
  }

  /// Return the full backing-store.
  store_type* GetArrayBits()
  {
    return GetStore();
  }

  /**
   * Gets quick access to the single-word (only useful when the bit
   * array <= the word size of the machine.)
   */
  store_type GetSingleWord()
  {
    CS_ASSERT(mpStore == 0);
    return mSingleWord;
  }

  /**
   * Sets the single-word very simply (only useful when the bit array <=
   * the word size of the machine.)
   */
  void SetSingleWord (store_type sw)
  {
    CS_ASSERT(mpStore == 0);
    mSingleWord = sw;
  }
};

/**
 * csComparator<> specialization for csBitArray to allow its use as 
 * e.g. hash key type.
 */
CS_SPECIALIZE_TEMPLATE
class csComparator<csBitArray, csBitArray>
{
public:
  static int Compare (csBitArray const& key1, csBitArray const& key2)
  {
    csBitArray::store_type const* p0 = key1.GetStore();
    csBitArray::store_type const* p1 = key2.GetStore();
    size_t compareNum = MIN (key1.mLength, key2.mLength);
    size_t i = 0;
    for (; i < compareNum; i++)
      if (p0[i] != p1[i])
	return (int)p0[i] - (int)p1[i];
    if (key1.mLength > key2.mLength)
    {
      for (; i < key1.mLength; i++)
	if (p0[i] != 0)
	  return (int)p0[i];
    }
    else
    {
      for (; i < key2.mLength; i++)
	if (p1[i] != 0)
	  return -((int)p1[i]);
    }
    return 0;
  }
};

/**
 * csHashComputer<> specialization for csBitArray to allow its use as 
 * hash key type.
 */
CS_SPECIALIZE_TEMPLATE
class csHashComputer<csBitArray>
{
public:
  static uint ComputeHash (csBitArray const& key)
  {
    const size_t uintCount = sizeof (csBitArray::store_type) / sizeof (uint);
    union
    {
      csBitArray::store_type store;
      uint ui[uintCount];
    } bitStoreToUint;
    uint hash = 0;
    csBitArray::store_type const* p = key.GetStore();
    // @@@ Not very good. Find a better hash function; however, it should
    // return the same hash for two bit arrays that are the same except for
    // the amount of trailing zeros. (e.g. f(10010110) == f(100101100000...))
    for (size_t i = 0; i < key.mLength; i++)
    {
      bitStoreToUint.store = p[i];
      for (size_t j = 0; j < uintCount; j++)
	hash += bitStoreToUint.ui[j];
    }
    return hash;
  }
};


#endif // __CS_BITARRAY_H__
