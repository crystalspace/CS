/*
    Copyright (C) 2003 by Mat Sutcliffe <oktal@gmx.co.uk>

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
  typedef First FirstType;
  typedef Second SecondType;

  First first;
  Second second;

  csTuple2 () {}
  csTuple2 (const First first, const Second second)
    : first (first), second (second) {}
  template <typename AlFirst, typename AlSecond>
  csTuple2 (const csTuple2<AlFirst, AlSecond>& t)
    : first (t.first), second (t.second) {}
};

/** @} */

#endif // __CS_UTIL_TUPLE_H__
