// A one-dimensional array of bits, similar to STL bitset.
//
// Copyright 2000 Andrew Kirmse.  All rights reserved.
//
// Permission is granted to use this code for any purpose, as long as this
// copyright message remains intact.

#ifndef __CS_BITARRAY_H__
#define __CS_BITARRAY_H__

#include <memory.h>
#include <assert.h>

/// A one-dimensional array of bits, similar to STL bitset.
class csBitArray
{
private:
   
   typedef unsigned long store_type;
   enum
   {
      bits_per_byte = 8,
      cell_size     = sizeof(store_type) * bits_per_byte
   };

   store_type        *mpStore;  
   store_type         mSingleWord; // Use this buffer when mLength is 1
   unsigned           mLength;     // Length of mpStore in units of store_type
   unsigned           mNumBits;

   /// Get the index and bit offset for a given bit number.
   static unsigned GetIndex(unsigned bit_num)
   {
      return bit_num / cell_size;
   }

   static unsigned GetOffset(unsigned bit_num)
   {
      return bit_num % cell_size;
   }

   void Init(unsigned size)
   {
      mNumBits = size;
      
      if (size == 0)
         mLength = 0;
      else
         mLength = 1 + GetIndex(size - 1);

      // Avoid allocation if length is 1 (common case)
      if (mLength <= 1)
         mpStore = &mSingleWord;      
      else
         mpStore = new store_type[mLength];
   }
   
   /// Force overhang bits at the end to 0
   inline void Trim()
   {
      unsigned extra_bits = mNumBits % cell_size;
      if (mLength > 0 && extra_bits != 0)
         mpStore[mLength - 1] &= ~((~(store_type) 0) << extra_bits);
   }

public:

   /**
    * \internal Bit proxy (for csBitArray::operator[])
    */
   class BitProxy
   {
   private:
      csBitArray &mArray;
      unsigned  mPos;
   public:
      BitProxy(csBitArray &array, unsigned pos):
            mArray(array), mPos(pos)
      {}

      BitProxy &operator=(bool value)
      {
         mArray.Set(mPos, value);
         return *this;
      }

      BitProxy &operator=(const BitProxy &that)
      {
         mArray.Set(mPos, that.mArray.IsBitSet(that.mPos));
         return *this;
      }

      operator bool() const
      {
         return mArray.IsBitSet(mPos);
      }

      bool Flip()
      {
         mArray.FlipBit(mPos);
         return mArray.IsBitSet(mPos);
      }
   };

   
   friend class BitProxy;
   
   //
   // Constructors and destructor
   //
   
   /// construct with <code>size</code> bits.
   explicit csBitArray(unsigned size)
   {
      Init(size);
      // Clear last bits
      Trim();
   }
   
   /// construct as duplicate of <code>that</code>.
   csBitArray(const csBitArray &that)
   {
      mpStore = 0;
      *this = that;
   }
   
   /// destructor
   virtual ~csBitArray()
   {
      if (mLength > 1)
         delete mpStore;
   }

   //
   // Operators
   //

   /// copy from other array
   csBitArray &operator=(const csBitArray &that)
   {
      if (this != &that)
      {
         if (mLength > 1)
            delete mpStore;

         Init(that.mNumBits);
         
         memcpy(mpStore, that.mpStore, mLength * sizeof(store_type));
      }
      return *this;
   }

   /// return bit at position <code>pos</code>
   BitProxy operator[](unsigned pos)
   {
      assert(pos < mNumBits);
      return BitProxy(*this, pos);
   }

   /// return bit at position <code>pos</code>
   const BitProxy operator[](unsigned pos) const
   {
      assert(pos < mNumBits);
      return BitProxy(CONST_CAST(csBitArray&,*this), pos);
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
   bool operator!=(const csBitArray &that) const
   {
      return !(*this == that);
   }

   /// bit-wise and
   csBitArray &operator&=(const csBitArray &that)
   {
      assert(mNumBits == that.mNumBits);
      for (unsigned i = 0; i < mLength; i++)
         mpStore[i] &= that.mpStore[i];
      return *this;
   }

   /// bit-wise or
   csBitArray operator|=(const csBitArray &that)
   {
      assert(mNumBits == that.mNumBits);
      for (unsigned i = 0; i < mLength; i++)
         mpStore[i] |= that.mpStore[i];
      return *this;
   }

   /// bit-wise xor
   csBitArray operator^=(const csBitArray &that)
   {
      assert(mNumBits == that.mNumBits);
      for (unsigned i = 0; i < mLength; i++)
         mpStore[i] ^= that.mpStore[i];
      return *this;
   }

   /// Flip all bits
   csBitArray operator~() const
   {
      return csBitArray(*this).FlipAllBits();
   }

   /// bit-wise and
   friend csBitArray operator&(const csBitArray &a1, const csBitArray &a2)
   {
      return csBitArray(a1) &= a2;
   }

   /// bit-wise or
   friend csBitArray operator|(const csBitArray &a1, const csBitArray &a2)
   {
      return csBitArray(a1) |= a2;
   }

   /// bit-wise xor
   friend csBitArray operator^(const csBitArray &a1, const csBitArray &a2)
   {
      return csBitArray(a1) ^= a2;
   }

   //
   // Plain English interface
   //
   
   /// Set all bits to false.
   void Clear()
   {
      memset(mpStore, 0, mLength * sizeof(store_type));
   }
   
   /// Set the bit at position pos to true.
   void SetBit(unsigned pos)
   {
      assert(pos < mNumBits);
      mpStore[GetIndex(pos)] |= 1 << GetOffset(pos); 
   }

   /// Set the bit at position pos to false.
   void ClearBit(unsigned pos)
   { 
      assert(pos < mNumBits);
      mpStore[GetIndex(pos)] &= ~(1 << GetOffset(pos)); 
   }

   /// Toggle the bit at position pos.
   void FlipBit(unsigned pos) 
   { 
      assert(pos < mNumBits);
      mpStore[GetIndex(pos)] ^= 1 << GetOffset(pos); 
   }

   /// Set the bit at position pos to the given value.
   void Set(unsigned pos, bool val)
   { 
      val ? SetBit(pos) : ClearBit(pos);
   }

   /// Returns true iff the bit at position pos is true.
   bool IsBitSet(unsigned pos) const
   {
      assert(pos < mNumBits);
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

   /// Gets quick access to the single-word (only useful when the bit array <= the word size of the machine.)
   unsigned GetSingleWord()
   {
     return mSingleWord;
   }

   /// Sets the single-word very simply (only useful when the bit array <= the word size of the machine.)
   void SetSingleWord(unsigned w)
   {
     mSingleWord=w;
   }
};

#endif // __CS_BITARRAY_H__
