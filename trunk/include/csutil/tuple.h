/*
    Copyright (C) 2003 by Marten Svanfeldt <developer@svanfeldt.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_UTIL_TUPLE_H__
#define __CS_UTIL_TUPLE_H__

/**\file
 * A tuple (fixed size collection of elements)
 */

/**\addtogroup util_containers
 * @{ */

/**
 * A two length tuple (fixed size collection of elements)
 * Tuples are typically used for quickly bounding grouped values
 * around with a low overhead.
 */
template <typename First, typename Second>
class csTuple2
{
public:
  /// typedef that can be accessed to recreate object of
  /// the first type
  typedef First FirstType;
  /// typedef that can be accessed to recreate object of
  /// the second type
  typedef Second SecondType;

  /// First element in Tuple
  First first;
  /// Second element in Tuple
  Second second;

  /// Empty default constructor.
  csTuple2 () : first (), second () {}
  /// Constructor to initialise both elements
  csTuple2 (const First& first, const Second& second)
    : first (first), second (second) {}
  /// Templated constructor from another csTuple2
  template <typename AlFirst, typename AlSecond>
  csTuple2 (const csTuple2<AlFirst, AlSecond>& t)
    : first (t.first), second (t.second) {}
};

/// Convenience function to create a csTuple2 from 2 types
template <typename First, typename Second>
inline csTuple2<First, Second> MakeTuple(First first, Second second)
{
  return csTuple2<First, Second> (first, second);
}

/** @} */

#endif // __CS_UTIL_TUPLE_H__
