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
      return !a.inclusive || !b.inclusive;
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
    if (intervals.Length() == 0) return ValueSet();

    csArray<Interval::Side> borders;
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

      if (!((newL == newR) && (!csFinite (newL.value))))
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
      while (((i+1) < intervals.GetSize())
        && intervals[i].Overlaps (intervals[i+1]))
      {
        const Interval::Side& l1 = intervals[i].left;
        const Interval::Side& l2 = intervals[i+1].left;
        const Interval::Side& r1 = intervals[i].right;
        const Interval::Side& r2 = intervals[i+1].right;
        intervals[i] = Interval (
          CompareLeftLeft (l1, l2) <= 0 ? l1 : l2,
          CompareRightRight (r1, r2) >= 0 ? r1 : r2);
        intervals.DeleteIndex (i);
      }
      if (intervals[i].Overlaps (otherIntervals[j]))
      {
        const Interval::Side& l1 = intervals[i].left;
        const Interval::Side& l2 = otherIntervals[j].left;
        const Interval::Side& r1 = intervals[i].right;
        const Interval::Side& r2 = otherIntervals[j].right;
        intervals[i] = Interval (
          CompareLeftLeft (l1, l2) <= 0 ? l1 : l2,
          CompareRightRight (r1, r2) >= 0 ? r1 : r2);
        j++;
      }
      else
      {
        const Interval::Side& il = intervals[i].right;
        const Interval::Side& ol = otherIntervals[j].left;
        if (il >= ol)
        {
          intervals.Insert (i, otherIntervals[j]);
          j++;
        }
        else
          i++;
      }
    }
    while (j < otherIntervals.GetSize())
    {
      intervals.Push (otherIntervals[j]);
      j++;
    }

    // Merge directly adjacent intervals
    i = 0;
    while (i+1 < intervals.GetSize())
    {
      if ((intervals[i].right.value == intervals[i+1].left.value)
        && (intervals[i].right.inclusive != intervals[i+1].left.inclusive))
      {
        intervals[i].right = intervals[i+1].right;
        intervals.DeleteIndex (i+1);
      }
      else
        i++;
    }

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
          intervals.Insert (i+1, newInterval);
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
