// A two-dimensional array of bits.
//
// Copyright 2000 Andrew Kirmse.  All rights reserved.
//
// Permission is granted to use this code for any purpose, as long as this
// copyright message remains intact.

#ifndef _csBitArray2D_H
#define _csBitArray2D_H

#include "bitarray.h"

class csBitArray2D : public csBitArray
{
   typedef csBitArray super;
   
private:
   unsigned mWidth;

public:

   //
   // Array proxy (for operator[])
   //
   
   class ArrayProxy
   {
   private:
      csBitArray2D &mArray;
      unsigned    mPos;  // We are a proxy for this row of the array
   public:
      ArrayProxy(csBitArray2D &array, unsigned pos):
            mArray(array), mPos(pos)
      {}

      super::BitProxy operator[](unsigned pos) const
      {
         return super::BitProxy(mArray, mPos * mArray.mWidth + pos);
      }
   };

   friend class ArrayProxy;
   
   //
   // Constructors
   //

   csBitArray2D(unsigned dim1, unsigned dim2) : super(dim1 * dim2)
   {
      mWidth = dim2;
   }

   csBitArray2D(const csBitArray2D &that) : super(that)
   {
      mWidth = that.mWidth;
   }

   //
   // Operators
   //
   
   csBitArray2D &operator=(const csBitArray2D &that)
   {
      super::operator=(that);
      mWidth = that.mWidth;
      return *this;
   }

   ArrayProxy operator[](unsigned pos)
   {
      return ArrayProxy(*this, pos);
   }

   const ArrayProxy operator[](unsigned pos) const
   {
      return ArrayProxy(CONST_CAST(csBitArray2D&,*this), pos);
   }
   
   bool operator==(const csBitArray2D &that) const
   {
      return super::operator==(that);
   }

   bool operator!=(const csBitArray2D &that) const
   {
      return !(*this == that);
   }

   csBitArray2D &operator&=(const csBitArray2D &that)
   {
      super::operator&=(that);
      return *this;
   }

   csBitArray2D operator|=(const csBitArray2D &that)
   {
      super::operator|=(that);
      return *this;
   }

   csBitArray2D operator^=(const csBitArray2D &that)
   {
      super::operator^=(that);
      return *this;
   }

   csBitArray2D operator~() const
   {
      return csBitArray2D(*this).FlipAllBits();
   }

   friend csBitArray2D operator&(const csBitArray2D &a1, const csBitArray2D &a2)
   {
      return csBitArray2D(a1) &= a2;
   }

   friend csBitArray2D operator|(const csBitArray2D &a1, const csBitArray2D &a2)
   {
      return csBitArray2D(a1) |= a2;
   }

   friend csBitArray2D operator^(const csBitArray2D &a1, const csBitArray2D &a2)
   {
      return csBitArray2D(a1) ^= a2;
   }

   //
   // Plain English interface
   //
   
   /// Set all bits to false.
   void Clear()
   {
      super::Clear();
   }
   
   /// Set the bit at position pos to true.
   void SetBit(unsigned pos1, unsigned pos2)
   {
      super::SetBit(pos1 * mWidth + pos2);
   }

   /// Set the bit at position pos to false.
   void ClearBit(unsigned pos1, unsigned pos2)
   { 
      super::ClearBit(pos1 * mWidth + pos2);
   }

   /// Toggle the bit at position pos.
   void FlipBit(unsigned pos1, unsigned pos2)
   { 
      super::FlipBit(pos1 * mWidth + pos2);
   }

   /// Set the bit at position pos to the given value.
   void Set(unsigned pos1, unsigned pos2, bool val)
   { 
      val ? SetBit(pos1, pos2) : ClearBit(pos1, pos2);
   }

   /// Returns true iff the bit at position pos is true.
   bool IsBitSet(unsigned pos1, unsigned pos2) const
   {
      return super::IsBitSet(pos1 * mWidth + pos2);
   }

   /// Returns true iff all bits are false.
   bool AllBitsFalse() const
   {
      return super::AllBitsFalse();
   }

   /// Change value of all bits
   csBitArray2D &FlipAllBits()
   {
      super::FlipAllBits();
      return *this;
   }
};

#endif
