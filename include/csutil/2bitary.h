// A one-dimensional array of two-bit entries.
//
// Copyright 2000 Andrew Kirmse.  All rights reserved.
//
// Permission is granted to use this code for any purpose, as long as this
// copyright message remains intact.

#ifndef __CS_2BITARRAY_H__
#define __CS_2BITARRAY_H__

#include "bitarray.h"

/// A one-dimensional array of two-bit entries.
class csTwoBitArray : private csBitArray
{
   typedef csBitArray super;
   
public:

   /**
    * \internal Two bit proxy (for csTwoBitArray::operator[])
    */
   class TwoBitProxy
   {
   private:
      csTwoBitArray &mArray;
      unsigned     mPos;
   public:
      TwoBitProxy(csTwoBitArray &array, unsigned pos):
            mArray(array), mPos(pos)
      {}

      TwoBitProxy &operator=(const TwoBitProxy &that)
      {
         mArray.Set(mPos, that.mArray.Get(that.mPos));
         return *this;
      }

      TwoBitProxy &operator=(unsigned value)
      {
         mArray.Set(mPos, value);
         return *this;
      }

      operator unsigned() const
      {
         return mArray.Get(mPos);
      }
   };

   friend class TwoBitProxy;
   
   //
   // Constructors
   //
   
   /// construct with <code>size</code> two bit entries.
   explicit csTwoBitArray(unsigned size) : super(2 * size)
   {}
   
   /// construct as duplicate of <code>that</code>.
   csTwoBitArray(const csTwoBitArray &that) : super(that)
   {}
   
   //
   // Operators
   //
   
   /// copy from other array
   csTwoBitArray &operator=(const csTwoBitArray &that)
   {
      super::operator=(that);
      return *this;
   }

   /// return bit pair at position <code>pos</code>
   TwoBitProxy operator[](unsigned pos)
   {
      return TwoBitProxy(*this, pos);
   }

   /// return bit pair at position <code>pos</code>
   const TwoBitProxy operator[](unsigned pos) const
   {
      return TwoBitProxy(CONST_CAST(csTwoBitArray&,*this), pos);
   }
   
   /// equal to other array
   bool operator==(const csTwoBitArray &that) const
   {
      return super::operator==(that);
   }

   /// not equal to other array
   bool operator!=(const csTwoBitArray &that) const
   {
      return !(*this == that);
   }

   /// bit-wise and
   csTwoBitArray &operator&=(const csTwoBitArray &that)
   {
      super::operator&=(that);
      return *this;
   }

   /// bit-wise or
   csTwoBitArray operator|=(const csTwoBitArray &that)
   {
      super::operator|=(that);
      return *this;
   }

   /// bit-wise xor
   csTwoBitArray operator^=(const csTwoBitArray &that)
   {
      super::operator^=(that);
      return *this;
   }

   /// Flip all bits
   csTwoBitArray operator~() const
   {
      return csTwoBitArray(*this).FlipAllBits();
   }

   /// bit-wise and
   friend csTwoBitArray operator&(const csTwoBitArray &a1, const csTwoBitArray &a2)
   {
      return csTwoBitArray(a1) &= a2;
   }

   /// bit-wise or
   friend csTwoBitArray operator|(const csTwoBitArray &a1, const csTwoBitArray &a2)
   {
      return csTwoBitArray(a1) |= a2;
   }

   /// bit-wise xor
   friend csTwoBitArray operator^(const csTwoBitArray &a1, const csTwoBitArray &a2)
   {
      return csTwoBitArray(a1) ^= a2;
   }

   //
   // Plain English interface
   //
   
   /// Set all bits to false.
   void Clear()
   {
      super::Clear();
   }
   
   /// Toggle the bits at position pos.
   void FlipBits(unsigned pos) 
   {
      Set(pos, 3 ^ Get(pos));
   }

   /// Set the bit at position pos to the given value.
   void Set(unsigned pos, unsigned val)
   {
      super::Set(2 * pos, val & 1);
      super::Set(2 * pos + 1, (val >> 1) & 1);
   }

   /// Returns true iff the bit at position pos is true.
   unsigned Get(unsigned pos) const
   {
      return (unsigned(super::IsBitSet(2 * pos)) |
              (unsigned(super::IsBitSet(2 * pos + 1)) << 1));
   }

   /// Returns true if all bits are false.
   bool AllZero() const
   {
      return super::AllBitsFalse();
   }

   /// Change value of all bits
   csTwoBitArray &FlipAllBits()
   {
      super::FlipAllBits();
      return *this;
   }
   
private:
};

#endif // __CS_2BITARRAY_H__
