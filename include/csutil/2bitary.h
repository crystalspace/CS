// A one-dimensional array of two-bit entries.
//
// Copyright 2000 Andrew Kirmse.  All rights reserved.
//
// Permission is granted to use this code for any purpose, as long as this
// copyright message remains intact.

#ifndef _csTwoBitArray_H
#define _csTwoBitArray_H

#include "bitarray.h"

class csTwoBitArray : private csBitArray
{
   typedef csBitArray super;
   
public:

   //
   // Two bit proxy (for operator[])
   //
   
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
   
   explicit csTwoBitArray(unsigned size) : super(2 * size)
   {}
   
   csTwoBitArray(const csTwoBitArray &that) : super(that)
   {}
   
   //
   // Operators
   //

   class TwoBitProxy;
   
   csTwoBitArray &operator=(const csTwoBitArray &that)
   {
      super::operator=(that);
      return *this;
   }

   TwoBitProxy operator[](unsigned pos)
   {
      return TwoBitProxy(*this, pos);
   }

   const TwoBitProxy operator[](unsigned pos) const
   {
      return TwoBitProxy(CONST_CAST(csTwoBitArray&,*this), pos);
   }
   
   bool operator==(const csTwoBitArray &that) const
   {
      return super::operator==(that);
   }

   bool operator!=(const csTwoBitArray &that) const
   {
      return !(*this == that);
   }

   csTwoBitArray &operator&=(const csTwoBitArray &that)
   {
      super::operator&=(that);
      return *this;
   }

   csTwoBitArray operator|=(const csTwoBitArray &that)
   {
      super::operator|=(that);
      return *this;
   }

   csTwoBitArray operator^=(const csTwoBitArray &that)
   {
      super::operator^=(that);
      return *this;
   }

   csTwoBitArray operator~() const
   {
      return csTwoBitArray(*this).FlipAllBits();
   }

   friend csTwoBitArray operator&(const csTwoBitArray &a1, const csTwoBitArray &a2)
   {
      return csTwoBitArray(a1) &= a2;
   }

   friend csTwoBitArray operator|(const csTwoBitArray &a1, const csTwoBitArray &a2)
   {
      return csTwoBitArray(a1) |= a2;
   }

   friend csTwoBitArray operator^(const csTwoBitArray &a1, const csTwoBitArray &a2)
   {
      return csTwoBitArray(a1) ^= a2;
   }

   //
   // Plain English interface
   //
   
   // Set all bits to false.
   void Clear()
   {
      super::Clear();
   }
   
   // Toggle the bits at position pos.
   void FlipBits(unsigned pos) 
   {
      Set(pos, 3 ^ Get(pos));
   }

   // Set the bit at position pos to the given value.
   void Set(unsigned pos, unsigned val)
   {
      super::Set(2 * pos, val & 1);
      super::Set(2 * pos + 1, (val >> 1) & 1);
   }

   // Returns true iff the bit at position pos is true.
   unsigned Get(unsigned pos) const
   {
      return (unsigned(super::IsBitSet(2 * pos)) |
              (unsigned(super::IsBitSet(2 * pos + 1)) << 1));
   }

   // Returns true iff all bits are false.
   bool AllZero() const
   {
      return super::AllBitsFalse();
   }

   // Change value of all bits
   csTwoBitArray &FlipAllBits()
   {
      super::FlipAllBits();
      return *this;
   }
   
private:
};

#endif
