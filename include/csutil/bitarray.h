/*
    Copyright (C) 2000 by Andrew Kirmse
                  2008 by Marten Svanfeldt

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
#include "csutil/allocator.h"
#include "csutil/bitops.h"
#include "csutil/comparator.h"
#include "csutil/hash.h"

#include "csutil/metautils.h"
#include "csutil/compileassert.h"

#if defined(CS_COMPILER_MSVC) && (CS_PROCESSOR_SIZE == 64)
/* long is 32 bit even on 64-bit MSVC, so use uint64 to utilize a storage in
 * 64 bit units.
 */
typedef uint64 csBitArrayStorageType;
#else
/// Storage type utilized by csBitArray to store the bits.
typedef unsigned long csBitArrayStorageType;
#endif
const size_t csBitArrayDefaultInlineBits = 
  sizeof (csBitArrayStorageType) * 8;
  
  
/// Base comparator for bit arrays
template<typename BitArray>
class csComparatorBitArray
{
public:
  static int Compare (BitArray const& key1, BitArray const& key2)
  {
    csBitArrayStorageType const* p0 = key1.GetStore();
    csBitArrayStorageType const* p1 = key2.GetStore();
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

  
/// Base hash computer for bit arrays
template<typename BitArray>
class csHashComputerBitArray
{
public:
  static uint ComputeHash (BitArray const& key)
  {
    const size_t uintCount = sizeof (csBitArrayStorageType) / sizeof (uint);
    uint ui[uintCount];
    uint hash = 0;
    csBitArrayStorageType const* p = key.GetStore();
    // \todo Not very good. Find a better hash function; however, it should
    // return the same hash for two bit arrays that are the same except for
    // the amount of trailing zeros. (e.g. f(10010110) == f(100101100000...))
    for (size_t i = 0; i < key.mLength; i++)
    {
      memcpy (ui, &p[i], sizeof (ui));
      for (size_t j = 0; j < uintCount; j++)
	hash += ui[j];
    }
    return hash;
  }
};

/**
 * A one-dimensional array of bits, similar to STL bitset.
 * The amount of bits is dynamic at runtime.
 *
 * Internally, bits are stored in multiple values of the type 
 * csBitArrayStorageType. If the number of bits is below a certain threshold,
 * the bits are stored in a field inside the class for more performance, 
 * above that threshold, the data is stored on the heap.
 *
 * This threshold can be tweaked by changing the \a InlinedBits template
 * parameter. At least \a InlinedBits bits will be stored inlined in the
 * class (the actual amount is the next bigger multiple of the number of bits
 * fitting into one csBitArrayStorageType). In scenarios where you can
 * anticipate that the common number of stored bits is larger than the 
 * default number, you can tweak InlinedBits to gain more performance.
 *
 * The \a Allocator template allocator allows you to override the logic
 * to allocate bits from the heap.
 */
template<int InlinedBits = csBitArrayDefaultInlineBits,
  typename Allocator = CS::Memory::AllocatorMalloc>
class csBitArrayTweakable
{
public:
  typedef csBitArrayTweakable<InlinedBits, Allocator> ThisType;
  typedef Allocator AllocatorType;

protected:
  template<typename BitArray> friend class csComparatorBitArray;
  template<typename BitArray> friend class csHashComputerBitArray;

  enum
  {
    cellSize    = csBitArrayDefaultInlineBits,            // This _have_ to be PO2
    cellCount   = (InlinedBits + (cellSize-1)) / cellSize,
    
    cellMask    = (cellSize - 1),
    cellShift   = CS::Meta::Log2<cellSize>::value
  };
  CS_COMPILE_ASSERT(CS::Meta::IsLog2<cellSize>::value);

  struct Storage : public Allocator
  {
    union
    {
      csBitArrayStorageType* heapStore;
      csBitArrayStorageType inlineStore[cellCount];
    };
    Storage()
    {
      memset (&inlineStore, 0, 
        MAX(sizeof (heapStore), sizeof (inlineStore)));
    }
  };
  Storage storage;
  /// Length of heapStore/inlineStore in units of csBitArrayStorageType
  size_t mLength;          
  size_t mNumBits;

  /// Get the GetStore()[] index for a given bit number.
  static size_t GetIndex (size_t bit_num)
  {
    return bit_num >> cellShift;
  }

  /// Get the offset within GetStore()[GetIndex()] for a given bit number.
  static size_t GetOffset (size_t bit_num)
  {
    return bit_num & cellMask;
  }

  /// Return whether the inline or heap store is used
  bool UseInlineStore () const
  {
    return mLength <= cellCount;
  }

  /**
   * Get a constant pointer to bit store, which may be internal mSingleWord or
   * heap-allocated mpStore.
   */
  csBitArrayStorageType const* GetStore() const
  {
    return UseInlineStore () ? storage.inlineStore : storage.heapStore;
  }

  /**
   * Get a non-constant pointer to bit store, which may be internal mSingleWord
   * or heap-allocated mpStore.
   */
  csBitArrayStorageType* GetStore()
  {
    return UseInlineStore () ? storage.inlineStore : storage.heapStore;
  }

  /// Force overhang bits at the end to 0.
  void Trim()
  {
    size_t extra_bits = mNumBits % cellSize;
    if (mLength > 0 && extra_bits != 0)
      GetStore()[mLength - 1] &= ~((~(csBitArrayStorageType)0) << extra_bits);
  }

public:
  /**
   * \internal Bit proxy (for csBitArray::operator[])
   */
  class BitProxy
  {
  private:
    csBitArrayTweakable& mArray;
    size_t mPos;
  public:
    /// Constructor.
    BitProxy (csBitArrayTweakable& array, size_t pos): mArray(array), mPos(pos)
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
  csBitArrayTweakable () : mLength(0), mNumBits(0)
  {
    SetSize (0);
  }

  /**
   * Construct with a size of \a size bits.
   */
  explicit csBitArrayTweakable (size_t size) : mLength(0), mNumBits(0)
  {
    SetSize (size);
  }

  /**
   * Construct as duplicate of \a that (copy constructor).
   */
  csBitArrayTweakable (const csBitArrayTweakable& that) : mLength(0), mNumBits(0)
  {
    *this = that; // Invokes this->operator=().
  }

  /// Destructor.
  ~csBitArrayTweakable()
  {
    if (!UseInlineStore ())
      storage.Free (storage.heapStore);
  }

  /// Return the number of stored bits.
  size_t GetSize() const
  {
    return mNumBits;
  }

  /**
   * Return the number of stored bits.
   * \deprecated Deprecated in 1.3. Use GetSize() instead.
   */
  CS_DEPRECATED_METHOD_MSG("Use GetSize() instead.")
  size_t Length () const
  {
    return GetSize();
  }

  /**
   * Set the number of stored bits.
   * \deprecated Deprecated in 1.3. Use SetSize() instead.
   */
  CS_DEPRECATED_METHOD_MSG("Use SetSize() instead.")
  void SetLength (size_t newSize)
  {
    SetSize (newSize);
  }

  /**
   * Set the number of stored bits.
   * \remarks If the new size is larger than the old size, the newly added
   *    bits are cleared.
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
      csBitArrayStorageType* newStore;
      if (newLength <= cellCount)
        newStore = storage.inlineStore;
      else
	newStore = (csBitArrayStorageType*)storage.Alloc (
          newLength * sizeof (csBitArrayStorageType));

      if (newLength > 0)
      {
	if (mLength > 0)
	{
	  csBitArrayStorageType* oldStore = GetStore();
	  if (newStore != oldStore)
	  {
	    memcpy (newStore, oldStore, 
	      (MIN (mLength, newLength)) * sizeof (csBitArrayStorageType));
	    if (newLength > mLength)
	      memset(newStore + mLength, 0,
		     (newLength - mLength) * sizeof (csBitArrayStorageType));
            if (!UseInlineStore ())
              storage.Free (oldStore);
	  }
	}
	else
	  memset (newStore, 0, newLength * sizeof (csBitArrayStorageType));
      }
      mLength = newLength;
      if (!UseInlineStore()) storage.heapStore = newStore;
    }

    mNumBits = newSize;
    Trim();
  }

  //
  // Operators
  //

  /// Copy from other array.
  csBitArrayTweakable& operator=(const csBitArrayTweakable& that)
  {
    if (this != &that)
    {
      SetSize (that.mNumBits);
      memcpy (GetStore(), that.GetStore(), 
        mLength * sizeof (csBitArrayStorageType));
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
  bool operator==(const csBitArrayTweakable& that) const
  {
    if (mNumBits != that.mNumBits)
      return false;

    csBitArrayStorageType const* p0 = GetStore();
    csBitArrayStorageType const* p1 = that.GetStore();
    for (unsigned i = 0; i < mLength; i++)
      if (p0[i] != p1[i])
        return false;
    return true;
  }

  /// Not equal to other array?
  bool operator != (const csBitArrayTweakable& that) const
  {
    return !(*this == that);
  }

  /// Bit-wise `and'. The arrays must be the same length.
  csBitArrayTweakable& operator &= (const csBitArrayTweakable &that)
  {
    CS_ASSERT (mNumBits == that.mNumBits);
    csBitArrayStorageType* p0 = GetStore();
    csBitArrayStorageType const* p1 = that.GetStore();
    for (size_t i = 0; i < mLength; i++)
      p0[i] &= p1[i];
    return *this;
  }

  /// Bit-wise `or'. The arrays must be the same length.
  csBitArrayTweakable operator |= (const csBitArrayTweakable& that)
  {
    CS_ASSERT (mNumBits == that.mNumBits);
    csBitArrayStorageType* p0 = GetStore();
    csBitArrayStorageType const* p1 = that.GetStore();
    for (size_t i = 0; i < mLength; i++)
      p0[i] |= p1[i];
    return *this;
  }

  /// Bit-wise `xor'. The arrays must be the same length.
  csBitArrayTweakable operator ^= (const csBitArrayTweakable& that)
  {
    CS_ASSERT (mNumBits == that.mNumBits);
    csBitArrayStorageType* p0 = GetStore();
    csBitArrayStorageType const* p1 = that.GetStore();
    for (size_t i = 0; i < mLength; i++)
      p0[i] ^= p1[i];
    return *this;
  }

  /// Return complement bit array in which all bits are flipped from this one.
  csBitArrayTweakable operator~() const
  {
    return csBitArrayTweakable(*this).FlipAllBits();
  }

  /// Bit-wise `and'. The arrays must be the same length.
  friend csBitArrayTweakable operator& (const csBitArrayTweakable& a1, 
    const csBitArrayTweakable& a2)
  {
    return csBitArrayTweakable (a1) &= a2;
  }

  /// Bit-wise `or'. The arrays must be the same length.
  friend csBitArrayTweakable operator | (const csBitArrayTweakable& a1, 
    const csBitArrayTweakable& a2)
  {
    return csBitArrayTweakable (a1) |= a2;
  }

  /// Bit-wise `xor'. The arrays must be the same length.
  friend csBitArrayTweakable operator ^ (const csBitArrayTweakable& a1, 
    const csBitArrayTweakable& a2)
  {
    return csBitArrayTweakable (a1) ^= a2;
  }

  //
  // Plain English interface
  //

  /// Set all bits to false.
  void Clear()
  {
    memset (GetStore(), 0, mLength * sizeof(csBitArrayStorageType));
  }

  /// Set the bit at position pos to true.
  void SetBit (size_t pos)
  {
    CS_ASSERT (pos < mNumBits);
    GetStore()[GetIndex(pos)] |= ((csBitArrayStorageType)1) << GetOffset(pos);
  }

  /// Set the bit at position pos to false.
  void ClearBit (size_t pos)
  {
    CS_ASSERT (pos < mNumBits);
    GetStore()[GetIndex(pos)] &= ~(((csBitArrayStorageType)1) << GetOffset(pos));
  }

  /// Toggle the bit at position pos.
  void FlipBit (size_t pos)
  {
    CS_ASSERT (pos < mNumBits);
    GetStore()[GetIndex(pos)] ^= ((csBitArrayStorageType)1) << GetOffset(pos);
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
    return (GetStore()[GetIndex(pos)] 
      & (((csBitArrayStorageType)1) << GetOffset(pos))) != 0;
  }

  /**
   * Returns true if the bit at position \a pos is true.
   * The difference to IsBitSet() is that this methods accepts positions
   * outside the bit array (returns \c false) instead of throwing an
   * assertion in that case.
   */
  bool IsBitSetTolerant (size_t pos) const
  {
    if (pos >= mNumBits) return false;
    return (GetStore()[GetIndex(pos)] 
      & (((csBitArrayStorageType)1) << GetOffset(pos))) != 0;
  }
  
  /// Checks whether at least one of \a count bits is set starting at \a pos.
  bool AreSomeBitsSet (size_t pos, size_t count) const
  {
    CS_ASSERT (pos + count <= mNumBits);
    csBitArrayStorageType const* p = GetStore();
    while (count > 0)
    {
      size_t index = GetIndex (pos);
      size_t offset = GetOffset (pos);
      size_t checkCount = MIN(count, cellSize - offset);
      csBitArrayStorageType mask = ((checkCount == cellSize) 
        ? ~(csBitArrayStorageType)0 
        : ((((csBitArrayStorageType)1) << checkCount) - 1)) << offset;
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
    csBitArrayStorageType const* p = GetStore();
    for (size_t i = 0; i < mLength; i++)
      if (p[i] != 0)
        return false;
    return true;
  }

  /// Change value of all bits
  csBitArrayTweakable& FlipAllBits()
  {
    csBitArrayStorageType* p = GetStore();
    for (size_t i = 0; i < mLength; i++)
      p[i] = ~p[i];
    Trim();
    return *this;
  }
  
  /// Count the number of bits that are set.
  size_t NumBitsSet() const
  {
    const size_t ui32perStorage = 
      sizeof (csBitArrayStorageType) / sizeof (uint32);

    union
    {
      csBitArrayStorageType s;
      uint32 ui32[ui32perStorage];
    } v;

    const csBitArrayStorageType* p = GetStore();
    size_t num = 0;
    for (size_t i = 0; i < mLength; i++)
    {
      v.s = p[i];
      for (size_t j = 0; j < ui32perStorage; j++)
        num += CS::Utility::BitOps::ComputeBitsSet (v.ui32[j]);
    }

    return num;
  }
  
  /**
   * Find first bit in array which is set.
   * \return First bit set or csArrayItemNotFound if all bits are set.
   */
  size_t GetFirstBitSet() const
  {
    const size_t ui32perStorage = 
      sizeof (csBitArrayStorageType) / sizeof (uint32);

    union
    {
      csBitArrayStorageType s;
      uint32 ui32[ui32perStorage];
    } v;

    const csBitArrayStorageType* p = GetStore();
    size_t ofs = 0, result;
    for (size_t i = 0; i < mLength; i++)
    {
      v.s = p[i];
      for (size_t j = 0; j < ui32perStorage; j++)
      {
        if (CS::Utility::BitOps::ScanBitForward (v.ui32[j], result))
        {
          size_t r = ofs + result;
          if (r >= mNumBits) return csArrayItemNotFound;
          return r;
        }
        ofs += 32;
      }
    }
    return csArrayItemNotFound;
  }
  /**
   * Find first bit in array which is not set.
   * \return First bit set or csArrayItemNotFound if all bits are not set.
   */
  size_t GetFirstBitUnset() const
  {
    const size_t ui32perStorage = 
      sizeof (csBitArrayStorageType) / sizeof (uint32);

    union
    {
      csBitArrayStorageType s;
      uint32 ui32[ui32perStorage];
    } v;

    const csBitArrayStorageType* p = GetStore();
    size_t ofs = 0, result;
    for (size_t i = 0; i < mLength; i++)
    {
      v.s = p[i];
      for (size_t j = 0; j < ui32perStorage; j++)
      {
        if (CS::Utility::BitOps::ScanBitForward (~v.ui32[j], result))
        {
          size_t r = ofs + result;
          if (r >= mNumBits) return csArrayItemNotFound;
          return r;
        }
        ofs += 32;
      }
    }
    return csArrayItemNotFound;
  }
  /**
   * Find last bit in array which is set.
   * \return First bit set or csArrayItemNotFound if all bits are set.
   */
  size_t GetLastBitSet() const
  {
    const size_t ui32perStorage = 
      sizeof (csBitArrayStorageType) / sizeof (uint32);

    union
    {
      csBitArrayStorageType s;
      uint32 ui32[ui32perStorage];
    } v;

    const csBitArrayStorageType* p = GetStore();
    size_t ofs, result;
    ofs = 32 * (mLength*ui32perStorage-1);
    for (size_t i = mLength; i-- > 0;)
    {
      v.s = p[i];
      for (size_t j = ui32perStorage; j-- > 0; )
      {
        if (CS::Utility::BitOps::ScanBitForward (v.ui32[j], result))
        {
          size_t r = ofs + result;
          if (r >= mNumBits) return csArrayItemNotFound;
          return r;
        }
        ofs -= 32;
      }
    }
    return csArrayItemNotFound;
  }
  /**
   * Find last bit in array which is not set.
   * \return First bit set or csArrayItemNotFound if all bits are not set.
   */
  size_t GetLastBitUnset() const
  {
    const size_t ui32perStorage = 
      sizeof (csBitArrayStorageType) / sizeof (uint32);

    union
    {
      csBitArrayStorageType s;
      uint32 ui32[ui32perStorage];
    } v;

    const csBitArrayStorageType* p = GetStore();
    size_t ofs, result;
    ofs = 32 * (mLength*ui32perStorage-1);
    for (size_t i = mLength; i-- > 0;)
    {
      v.s = p[i];
      for (size_t j = ui32perStorage; j-- > 0; )
      {
        if (CS::Utility::BitOps::ScanBitForward (~v.ui32[j], result))
        {
          size_t r = ofs + result;
          if (r >= mNumBits) return csArrayItemNotFound;
          return r;
        }
        ofs -= 32;
      }
    }
    return csArrayItemNotFound;
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
  csBitArrayTweakable Slice(size_t pos, size_t count) const
  {
    CS_ASSERT(pos + count <= mNumBits);
    csBitArrayTweakable a (count);
    for (size_t i = pos, o = 0; i < pos + count; i++)
      if (IsBitSet(i))
	a.SetBit(o++);
    return a;
  }

  //@{
  /// Return the full backing-store.
  csBitArrayStorageType* GetArrayBits() { return GetStore(); }
  const csBitArrayStorageType* GetArrayBits() const { return GetStore(); }
  //@}

  //@{
  /**
   * 
   */
  class SetBitIterator
  {
  public:
    SetBitIterator (const SetBitIterator& other)
      : bitArray (other.bitArray), pos (other.pos), offset (other.offset),
      cachedStore (other.cachedStore)
    {}

    /// 
    size_t Next ()
    {
      while (offset < 8*sizeof(csBitArrayStorageType))
      {        
        if ((cachedStore & 0x1) != 0)
        {
          const size_t result = (pos-1)*sizeof(csBitArrayStorageType)*8 + offset;

          ++offset;
          cachedStore >>= 1;
          if (!cachedStore)
            GetNextCacheItem ();

          return result;
        }        
        else
        {
          ++offset;
          cachedStore >>= 1;
          if (!cachedStore)
            GetNextCacheItem ();
        }
      }
      CS_ASSERT_MSG ("Invalid iteration", false);
      return 0;
    }

    ///
    bool HasNext () const
    {
      return cachedStore != 0;
    }

    ///
    void Reset ()
    {
      pos = 0;
      GetNextCacheItem ();
    }

  protected:
    SetBitIterator (const ThisType& bitArray)
      : bitArray (bitArray), pos (0), offset (0), cachedStore (0)
    {
      Reset ();
    }

    friend class csBitArrayTweakable<InlinedBits, Allocator>;

    void GetNextCacheItem ()
    {
      offset = 0;
      while (pos < bitArray.mLength)
      {
        cachedStore = bitArray.GetStore ()[pos++];
        if (cachedStore)
          return;
      }
      cachedStore = 0;
    }

    const ThisType& bitArray;
    size_t pos, offset;
    csBitArrayStorageType cachedStore;    
  };
  friend class SetBitIterator;

  SetBitIterator GetSetBitIterator () const
  {
    return SetBitIterator (*this);
  }
  //@}
  
  /**\name Serialization
   * @{ */
  /**
   * Get byte stream with the contents of the bit array.
   * \param numBytes Number of bytes in the returned buffer.
   * \return A byte stream with the contents of the bit array.
   *  May be 0. Must be freed with cs_free().
   */
  uint8* Serialize (size_t& numBytes) const
  {
    if (mNumBits == 0)
    {
      numBytes = 0;
      return 0;
    }
  
    struct SerializeHelper
    {
      uint8* buf;
      size_t bufSize, bufUsed;
      
      SerializeHelper () : buf (0), bufSize (0), bufUsed (0) {}
      void PushByte (uint8 b)
      {
        if (bufUsed >= bufSize)
        {
          bufSize += 4;
          buf = (uint8*)cs_realloc (buf, bufSize);
        }
        buf[bufUsed++] = b;
      }
      void TruncZeroes ()
      {
        while ((bufUsed > 0) && (buf[bufUsed-1] == 0))
          bufUsed--;
      }
    } serHelper;
    
    // Write out bits number
    {
      size_t remainder = mNumBits;
      
      while (remainder >= 128)
      {
        uint8 b = (remainder & 0x7f) | 0x80;
        serHelper.PushByte (b);
        remainder >>= 7;
      }
      serHelper.PushByte (uint8 (remainder));
    }
  
    const size_t ui8Count = sizeof (csBitArrayStorageType) / sizeof (uint8);
    uint8 ui8[ui8Count];
    csBitArrayStorageType const* p = GetStore();
    for (size_t i = 0; i < mLength; i++)
    {
      memcpy (ui8, &p[i], sizeof (ui8));
#ifdef CS_LITTLE_ENDIAN
      for (size_t j = 0; j < ui8Count; j++)
#else
      for (size_t j = ui8Count; j-- > 0; )
#endif
      {
	serHelper.PushByte (ui8[j]);
      }
    }
    
    serHelper.TruncZeroes();
    numBytes = serHelper.bufUsed;
    return serHelper.buf;
  }
  /**
   * Create a new instance of a bit array with the contents as given
   * in the byte stream.
   */
  static ThisType Unserialize (uint8* bytes, size_t numBytes)
  {
    if ((bytes == 0) || (numBytes == 0))
      return ThisType(); // empty bit array
  
    size_t bufPos = 0;
  
    // Read the bits number
    size_t numBits = 0;
    int shift = 0;
    while (bufPos < numBytes)
    {
      uint8 b = bytes[bufPos++];
      numBits |= (b & 0x7f) << shift;
      if ((b & 0x80) == 0) break;
      shift += 7;
    }
    
    ThisType newArray (numBits);
    
    // Read the actual bits
    csBitArrayStorageType* p = newArray.GetStore();
    size_t storeIndex = 0;
    while (bufPos < numBytes)
    {
      const size_t ui8Count = sizeof (csBitArrayStorageType) / sizeof (uint8);
      uint8 ui8[ui8Count];
      memset (ui8, 0, sizeof (ui8));
#ifdef CS_LITTLE_ENDIAN
      for (size_t j = 0; j < ui8Count; j++)
#else
      for (size_t j = ui8Count; j-- > 0; )
#endif
      {
        ui8[j] = bytes[bufPos++];
        if (bufPos >= numBytes) break;
      }
      memcpy (p + (storeIndex++), ui8, sizeof (ui8));
    }
    
    return newArray;
  }
  /** @} */
};

/**
 * A one-dimensional array of bits, similar to STL bitset.
 * The amount of bits is dynamic at runtime.
 */
class csBitArray : public csBitArrayTweakable<>
{
public:
  /// Default constructor.
  csBitArray () { }
  /// Construct with a size of \a size bits.
  explicit csBitArray (size_t size) : csBitArrayTweakable<> (size) { }
  /// Construct as duplicate of \a that (copy constructor).
  csBitArray (const csBitArray& that) : csBitArrayTweakable<> (that) { }

  /// Copy from other array.
  template<int A, typename B>
  csBitArray& operator=(const csBitArrayTweakable<A, B>& that)
  {
    if (this != &that)
    {
      SetSize (that.GetSize());
      memcpy (GetStore(), that.GetArrayBits(), 
        mLength * sizeof (csBitArrayStorageType));
    }
    return *this;
  }

};


/**
 * csComparator<> specialization for csBitArray to allow its use as 
 * e.g. hash key type.
 */
template<>
class csComparator<csBitArray, csBitArray> : 
  public csComparatorBitArray<csBitArray> { };


/**
 * csHashComputer<> specialization for csBitArray to allow its use as 
 * hash key type.
 */
template<>
class csHashComputer<csBitArray> : 
  public csHashComputerBitArray<csBitArray> { };


#endif // __CS_BITARRAY_H__
