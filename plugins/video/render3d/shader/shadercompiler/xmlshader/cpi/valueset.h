/*
  Copyright (C) 2006 by Frank Richter
	    (C) 2006 by Jorrit Tyberghein

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

#ifndef __CS_VALUESET_H__
#define __CS_VALUESET_H__

#include <limits>
#include "csutil/array.h"
#include "csutil/memheap.h"
#include "csgeom/math.h"

#include "logic3.h"
#include "tempheap.h"

#include "csutil/custom_new_disable.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

  /**
   * A set of values, stored as a number of interval.
   */
  class ValueSet
  {
  public:
    /**
     * An interval of number.
     */
    class Interval
    {
    public:
      /**
       * An interval side.
       */
      class Side
      {
        enum { Inclusive = 1, Finite = 2, FiniteDirty = 4 };
        mutable uint flags;
	float value;
      public:
      
	Side (bool negInf, bool inclusive) : flags (inclusive ? Inclusive : 0),
          value (negInf ? -CS::Infinity() : CS::Infinity()) {}
	Side (float v, bool inclusive) : 
          flags (Finite | (inclusive ? Inclusive : 0)), value (v) {}
      
        inline bool GetInclusive () const { return flags & Inclusive; }
        inline void SetInclusive (bool inclusive)
        { if (inclusive) flags |= Inclusive; else flags &= ~Inclusive; }
        inline void FlipInclusive()
        { 
          if (IsFinite ()) flags = flags ^ Inclusive;
        }
        inline bool IsFinite() const
        {
          if (flags & FiniteDirty)
          {
            if (CS::IsFinite (value))
            {
              flags = (flags & ~FiniteDirty) | Finite;
              return true;
            }
            else
            {
              flags &= ~(FiniteDirty | Finite);
              return false;
            }
          }
          else
            return (flags & Finite) != 0;
        }
        inline float& GetValue() { return value; }
        inline float GetValue() const { return value; }

        friend bool operator== (const Side& a, const Side& b)
        {
          return (a.GetInclusive() == b.GetInclusive()) && 
            (a.value == b.value)
            /*(fabsf (a.value - b.value) < SMALL_EPSILON)*/;
        }
        friend bool operator!= (const Side& a, const Side& b)
        { return !operator== (a, b); }
        //@{
        /**
         * Compare two interval sides.
         * \remarks The first operand must be a \em right interval limit,
         *  the second interval must be a \em left limit. This is required
         *  since the "inclusive" flags has a different meaning, depending
         *  on whether the interval limit is a left or right one.
         */
        friend bool operator< (const Side& a, const Side& b);
        friend bool operator<= (const Side& a, const Side& b);
        friend bool operator> (const Side& a, const Side& b);
        friend bool operator>= (const Side& a, const Side& b);
        //@}

        friend int CompareLeftLeft (const Side& a, const Side& b)
        {
          if (a.value < b.value)
            return -1;
          else if (a.value > b.value)
            return 1;
          else
          {
            if (a.GetInclusive() == b.GetInclusive())
              return 0;
            else if (a.GetInclusive() && !b.GetInclusive())
              return -1;
            else
              return 1;
          }
        }
        friend int CompareRightRight (const Side& a, const Side& b)
        {
          if (a.value < b.value)
            return -1;
          else if (a.value > b.value)
            return 1;
          else
          {
            if (a.GetInclusive() == b.GetInclusive())
              return 0;
            else if (a.GetInclusive() && !b.GetInclusive())
              return 1;
            else
              return -1;
          }
        }
      };
      Side left;
      Side right;

      Interval () : left (true, false), right (false, false) {}
      Interval (const float f) : left (f, true), right (f, true) {}
      Interval (const Side& l, const Side& r) : left (l), right (r) {}

      bool Overlaps (const Interval& other) const;
      bool IsEmpty() const;
      bool IsSingleValue() const
      {
        return ((left == right) && CS::IsFinite (left.GetValue()));
      }

      friend bool operator== (const Interval& a, const Interval& b)
      {
        return (a.left == b.left) && (a.right == b.right);
      }
      friend bool operator!= (const Interval& a, const Interval& b)
      {
        return (a.left != b.left) || (a.right != b.right);
      }
    };
  protected:
    static const size_t arrayGrow = 3;
    class IntervalElementHandler : public csArrayElementHandler<Interval>
    {
    public:
      CS_FORCEINLINE
      static void Construct (Interval* address)
      {
        new (static_cast<void*> (address)) Interval();
      }

      CS_FORCEINLINE
      static void Construct (Interval* address, Interval const& src)
      {
        memcpy (address, &src, sizeof (Interval));
      }

      CS_FORCEINLINE
      static void Destroy (Interval* /*address*/)
      {
      }

      static void InitRegion (Interval* address, size_t count)
      {
        for (size_t i = 0 ; i < count ; i++)
          Construct (address + i);
      }
    };
    typedef csArray<Interval, IntervalElementHandler,
      CS::Memory::LocalBufferAllocator<Interval, arrayGrow, TempHeapAlloc, true>,
      csArrayCapacityLinear<csArrayThresholdFixed<arrayGrow> > > IntervalArray;
    IntervalArray intervals;
  public:
    ValueSet (bool empty = false) : 
      intervals (empty ? 0 : 1, arrayGrow)
    {
      if (!empty) intervals.SetSize (1);
    }
    ValueSet (const float f) : 
      intervals (1, arrayGrow)
    {
      intervals.Push (Interval (f));
    }
    ValueSet (const Interval& i) : 
      intervals (1, arrayGrow)
    {
      intervals.Push (i);
    }
  
    bool Overlaps (const ValueSet& other) const;
    /// Unite this value set with another.
    ValueSet& Union (const ValueSet& other);
    /// Intersect this value set with another.
    ValueSet& Intersection (const ValueSet& other);

    Interval::Side GetMin() const;
    Interval::Side GetMax() const;

    bool IsSingleValue() const
    {
      return ((intervals.GetSize() == 1)
        && intervals[0].IsSingleValue());
    }
    float GetSingleValue() const
    {
      return intervals[0].left.GetValue();
    }

    ValueSet operator!() const;
    friend ValueSet operator& (const ValueSet& a, const ValueSet& b)
    {
      ValueSet result (a);
      result.Intersection (b);
      return result;
    }
    ValueSet& operator&= (const ValueSet& other)
    {
      Intersection (other);
      return *this;
    }
    friend ValueSet operator| (const ValueSet& a, const ValueSet& b)
    {
      ValueSet result (a);
      result.Union (b);
      return result;
    }
    ValueSet& operator|= (const ValueSet& other)
    {
      Union (other);
      return *this;
    }
    friend bool operator== (const ValueSet& a, const ValueSet& b)
    {
      return a.intervals == b.intervals;
    }

    operator Logic3() const
    {
      ValueSet falseSet (0.0f);
      ValueSet trueSet (1.0f);
      bool canTrue = Overlaps (trueSet);
      bool canFalse = Overlaps (falseSet);

      if (canTrue && !canFalse)
        return Logic3::Truth;
      else if (!canTrue && canFalse)
        return Logic3::Lie;
      else
        return Logic3::Uncertain;
    }

    void Dump (csString& str) const
    {
      for (size_t i = 0; i < intervals.GetSize(); i++)
      {
        if (intervals[i].left.GetInclusive ()) 
          str << '[';
        else
          str << ']';
        if (!intervals[i].left.IsFinite ())
          str << "-INF";
        else
          str << intervals[i].left.GetValue ();
        str << ';';
        if (!intervals[i].right.IsFinite ())
          str << "+INF";
        else
          str << intervals[i].right.GetValue ();
        if (intervals[i].right.GetInclusive ()) 
          str << ']';
        else
          str << '[';
        str << ' ';
      }
    }
  };
  
  CS_ALIGNED_STRUCT(class, 1) ValueSetBool
  {
    enum { flagTrue = 1, flagFalse = 2 };
    uint8 flags;
    
    explicit ValueSetBool (uint8 flags) : flags (flags) {}
  public:
    ValueSetBool () : flags (flagTrue | flagFalse) {}
    ValueSetBool (const bool b) : flags (b ? flagTrue : flagFalse) {}
    
    bool Overlaps (const ValueSetBool& other) const
    {
      return (flags & other.flags) != 0;
    }
    
    bool IsSingleValue() const
    {
      return (flags == flagTrue) || (flags == flagFalse);
    }
    bool GetSingleValue() const
    {
      switch (flags)
      {
        case flagTrue:  return true;
        default: 
        case flagFalse: return false;
      }
    }
    
    ValueSetBool operator!() const
    { return ValueSetBool (uint8 (flags ^ (flagTrue | flagFalse))); }
    friend ValueSetBool operator& (const ValueSetBool& a, const ValueSetBool& b)
    { return ValueSetBool (uint8 (a.flags & b.flags)); }
    ValueSetBool& operator&= (const ValueSetBool& other)
    { flags &= other.flags; return *this; }
    friend ValueSetBool operator| (const ValueSetBool& a, const ValueSetBool& b)
    { return ValueSetBool (uint8 (a.flags | b.flags)); }
    ValueSetBool& operator|= (const ValueSetBool& other)
    { flags |= other.flags; return *this; }
    friend bool operator== (const ValueSetBool& a, const ValueSetBool& b)
    { return a.flags == b.flags; }

    operator Logic3() const
    {
      switch (flags)
      {
        case flagTrue:  return Logic3::Truth;
        case flagFalse: return Logic3::Lie;
        default: return Logic3::Uncertain;
      }
    }

    void Dump (csString& str) const
    {
      switch (flags)
      {
        case 0:
          str = "[] ";
          break;
        case flagTrue:
          str = "[true] ";
          break;
        case flagFalse:
          str = "[false] ";
          break;
        case flagTrue | flagFalse:
          str = "[true, false] ";
          break;
      }
    }
  };

}
CS_PLUGIN_NAMESPACE_END(XMLShader)

#include "csutil/custom_new_enable.h"

#endif // __CS_VALUESET_H__
