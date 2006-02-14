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
#include "csgeom/math.h"

#include "logic3.h"

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
      public:
        bool inclusive;
	    float value;
      
	    Side (bool negInf, bool inclusive) : 
          inclusive (inclusive), 
          value (negInf ? -std::numeric_limits<float>::infinity()
                        : std::numeric_limits<float>::infinity()) {}
	    Side (float v, bool inclusive) : 
          inclusive (inclusive), value (v) {}
      
        void FlipInclusive()
        { if (csFinite (value)) inclusive = !inclusive; }

        friend bool operator== (const Side& a, const Side& b)
        {
          return (a.inclusive == b.inclusive) && 
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
      };
      Side left;
      Side right;

      Interval () : left (true, false), right (false, false) {}
      Interval (const float f) : left (f, true), right (f, true) {}
      Interval (const Side& l, const Side& r) : left (l), right (r) {}

      bool Overlaps (const Interval& other) const;
      bool IsEmpty() const;

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
    static const size_t arrayGrow = 1;
    csArray<Interval> intervals;
  public:
    ValueSet (bool empty = false) : 
      intervals (empty ? 0 : 1, arrayGrow)
    {
      if (!empty) intervals.Push (Interval ());
    }
    ValueSet (const float f) : intervals (1, arrayGrow)
    {
      intervals.Push (Interval (f));
    }
    ValueSet (const Interval& i) : intervals (1, arrayGrow)
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

    ValueSet operator!() const;
    friend ValueSet operator& (const ValueSet& a, const ValueSet& b)
    {
      ValueSet result (a);
      result.Intersection (b);
      return result;
    }
    friend ValueSet operator| (const ValueSet& a, const ValueSet& b)
    {
      ValueSet result (a);
      result.Union (b);
      return result;
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
  };

}
CS_PLUGIN_NAMESPACE_END(XMLShader)

#endif // __CS_VALUESET_H__
