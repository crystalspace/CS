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

#include "cssysdef.h"
#include "csgeom/math.h"

#include "valueset.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

  bool operator< (const ValueSet::Interval::Side& a, 
    const ValueSet::Interval::Side& b)
  {
    if (a.value == b.value)
    {
      return !a.GetInclusive() || !b.GetInclusive();
    }
    else
      return a.value < b.value;
  }
  bool operator<= (const ValueSet::Interval::Side& a, 
    const ValueSet::Interval::Side& b)
  {
    return a.value <= b.value;
  }
  bool operator> (const ValueSet::Interval::Side& a, 
    const ValueSet::Interval::Side& b)
  {
    return !(a <= b);
  }
  bool operator>= (const ValueSet::Interval::Side& a, 
    const ValueSet::Interval::Side& b)
  {
    return !(a < b);
  }

  bool ValueSet::Interval::Overlaps (const Interval& other) const
  {
    return (right >= other.left) && (other.right >= left);
  }

  bool ValueSet::Interval::IsEmpty() const
  {
    return (left > right);
  }

  bool ValueSet::Overlaps (const ValueSet& other) const
  {
    ValueSet newSet (*this);
    newSet.Intersection (other);
    return !newSet.intervals.IsEmpty();
  }

  ValueSet::Interval::Side ValueSet::GetMin() const
  {
    Interval::Side min (false, false);
    for (size_t i = 0; i < intervals.GetSize(); i++)
      min = csMin (min, intervals[i].left);
    return min;
  }

  ValueSet::Interval::Side ValueSet::GetMax() const
  {
    Interval::Side max (true, false);
    for (size_t i = 0; i < intervals.GetSize(); i++)
      max = csMax (max, intervals[i].right);
    return max;
  }

  ValueSet ValueSet::operator!() const
  {
    if (intervals.GetSize () == 0) return ValueSet ();

    csArray<Interval::Side,
      csArrayElementHandler<Interval::Side>,
      TempHeapAlloc> borders;
    int currentSide = 0;

    borders.Push (Interval::Side (true, false));
    size_t i = 0;
    while (i < intervals.GetSize())
    {
      const Interval::Side& side = 
        (currentSide == 0) ? intervals[i].left : intervals[i].right;
      borders.Push (side);
      if (currentSide == 1)
      {
        currentSide = 0;
        i++;
      }
      else
        currentSide = 1;
    }
    borders.Push (Interval::Side (false, false));

    ValueSet newSet (true);
    i = 0;
    while (i < borders.GetSize())
    {
      Interval::Side newL = borders[i];
      newL.FlipInclusive();
      Interval::Side newR = borders[i+1];
      newR.FlipInclusive();

      if (!((newL == newR) && (!CS::IsFinite (newL.GetValue()))))
        newSet.intervals.Push (Interval (newL, newR));

      i += 2;
    }
    return newSet;
  }

  ValueSet& ValueSet::Union (const ValueSet& other)
  {
    size_t i = 0, j = 0;
    const IntervalArray& otherIntervals = other.intervals;

    while ((i < intervals.GetSize()) && (j < otherIntervals.GetSize()))
    {
      //if (intervals.GetSize() >= 3) CS_DEBUG_BREAK;
      Interval& intv = intervals[i];
      while (((i+1) < intervals.GetSize())
        && intv.Overlaps (intervals[i+1]))
      {
        Interval& intvNext = intervals[i+1];
        const Interval::Side& l1 = intv.left;
        const Interval::Side& l2 = intvNext.left;
        const Interval::Side& r1 = intv.right;
        const Interval::Side& r2 = intvNext.right;
        intv = Interval (
          CompareLeftLeft (l1, l2) <= 0 ? l1 : l2,
          CompareRightRight (r1, r2) >= 0 ? r1 : r2);
        intervals.DeleteIndex (i);
      }
      const Interval& intvOther = otherIntervals[j];
      if (intv.Overlaps (intvOther))
      {
        const Interval::Side& l1 = intv.left;
        const Interval::Side& l2 = intvOther.left;
        const Interval::Side& r1 = intv.right;
        const Interval::Side& r2 = intvOther.right;
        intv = Interval (
          CompareLeftLeft (l1, l2) <= 0 ? l1 : l2,
          CompareRightRight (r1, r2) >= 0 ? r1 : r2);
        while ((i > 0)
          && (intervals[i-1].right.GetValue() == intv.left.GetValue())
          && (intervals[i-1].right.GetInclusive() != intv.left.GetInclusive()))
        {
          intv.left = intervals[i-1].left;
          intervals.DeleteIndex (i-1);
          i--;
        }
        while (((i+1) < intervals.GetSize())
          && (intv.right.GetValue() == intervals[i+1].left.GetValue())
          && (intv.right.GetInclusive() != intervals[i+1].left.GetInclusive()))
        {
          intv.right = intervals[i+1].right;
          intervals.DeleteIndex (i+1);
        }
        j++;
      }
      else
      {
        const Interval::Side& ir = intv.right;
        const Interval::Side& ol = intvOther.left;
        if (ir >= ol)
        {
          if ((intvOther.right.GetValue() == intv.left.GetValue())
            && (intvOther.right.GetInclusive() != intv.left.GetInclusive()))
          {
            intv.left = intvOther.left;
          }
          else
            intervals.Insert (i, intvOther);
          j++;
        }
        else
          i++;
      }
    }
    {
      Interval::Side& intvRight = intervals[intervals.GetSize()-1].right;
      while (j < otherIntervals.GetSize())
      {
        const Interval::Side& intvNextLeft = otherIntervals[j].left;
        if ((intvRight.GetValue() == intvNextLeft.GetValue())
          && (intvRight.GetInclusive() != intvNextLeft.GetInclusive()))
        {
          intvRight = otherIntervals[j].right;
        }
        else
          intervals.Push (otherIntervals[j]);
        j++;
      }
    }
    //if (intervals.GetSize() >= 3) CS_DEBUG_BREAK;

    // Merge directly adjacent intervals
    /*i = 0;
    while (i+1 < intervals.GetSize())
    {
      Interval::Side& intvRight = intervals[i].right;
      const Interval::Side& intvNextLeft = intervals[i+1].left;
      if ((intvRight.GetValue() == intvNextLeft.GetValue())
        && (intvRight.GetInclusive() != intvNextLeft.GetInclusive()))
      {
        intvRight = intervals[i+1].right;
        intervals.DeleteIndex (i+1);
      }
      else
        i++;
    }*/

    return *this;
  }

  ValueSet& ValueSet::Intersection (const ValueSet& other)
  {
    size_t i = 0, j = 0;
    const IntervalArray& otherIntervals = other.intervals;

    while ((i < intervals.GetSize()) && (j < otherIntervals.GetSize()))
    {
      while ((i < intervals.GetSize())
        && intervals[i].IsEmpty ())
      {
        intervals.DeleteIndex (i);
      }
      if (intervals[i].Overlaps (otherIntervals[j]))
      {
        if (CompareLeftLeft (intervals[i].left,
          otherIntervals[j].left) < 0)
        {
          intervals[i].left = otherIntervals[j].left;
        }
        if (CompareRightRight (intervals[i].right,
          otherIntervals[j].right) > 0)
        {
          Interval newInterval;
          newInterval.left = otherIntervals[j].right;
          newInterval.left.FlipInclusive();
          newInterval.right = intervals[i].right;
          intervals[i].right = otherIntervals[j].right;
          if (!newInterval.IsEmpty()
            && (j+1 < otherIntervals.GetSize()))
          {
            intervals.Insert (i+1, newInterval);
          }
        }
        i++;
      }
      else
      {
        const Interval::Side& il = intervals[i].right;
        const Interval::Side& ol = otherIntervals[j].left;
        if (il >= ol)
          j++;
        else
          intervals.DeleteIndex (i);
      }
    }
    while (i < intervals.GetSize())
    {
      intervals.DeleteIndex (i);
    }

    return *this;
  }

}
CS_PLUGIN_NAMESPACE_END(XMLShader)
