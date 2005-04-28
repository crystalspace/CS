/*
  Crystal Space Generic Array Template
  Copyright (C) 2003 by Matze Braun
  Copyright (C) 2003 by Jorrit Tyberghein
  Copyright (C) 2003,2004 by Eric Sunshine

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
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
#ifndef __CSUTIL_COMPARATOR_H__
#define __CSUTIL_COMPARATOR_H__

/**\file
 * Template providing various comparison and ordering functions.
 */

#include "csstring.h"

/**\addtogroup util
 * @{ */

/**
 * A template providing various comparison and ordering functions.
 */
template <class T1, class T2>
class csComparator
{
public:
  /**
   * Compare two objects of the same type or different types (T1 and T2).
   * \param r1 Reference to first object.
   * \param r2 Reference to second object.
   * \return Zero if the objects are equal; less-than-zero if the first object
   *   is less than the second; or greater-than-zero if the first object is
   *   greater than the second.
   * \remarks Assumes the existence of T1::operator<(T2) and T2::operator<(T1).
   *   If T1 and T2 are the same type T, then only T::operator<(T) is assumed
   *   (of course).  This is the default comparison function used by csArray
   *   for searching and sorting if the client does not provide a custom
   *   function.
   */
  static int Compare(T1 const &r1, T2 const &r2)
  {
    if (r1 < r2) return -1;
    else if (r2 < r1) return 1;
    else return 0;
  }
};

/**
 * csComparator<> specialization for strings that uses strcmp().
 */
CS_SPECIALIZE_TEMPLATE
class csComparator<const char*, const char*>
{
public:
  static int Compare (const char* const& r1, const char* const& r2)
  {
    return strcmp (r1, r2);
  }
};

/**
 * Template that can be used as a base class for hash comparators for POD 
 * structs.
 * Example:
 * \code
 * CS_SPECIALIZE_TEMPLATE csHashComputer<MyStruct> : 
 *   public csComparatorStruct<MyStruct> {};
 * \endcode
 */
template<class T>
class csComparatorStruct
{
public:
  static int Compare (T const& r1, T const& r2)
  {
    return memcmp (&r1, &r2, sizeof (T));
  }
};

/**
 * csComparator<> specialization for csString that uses strcmp().
 */
CS_SPECIALIZE_TEMPLATE
class csComparator<csString, csString>
{
public:
  static int Compare (csString const& r1, csString const& r2)
  {
    return strcmp (r1, r2);
  }
};

/** @} */

#endif // __CSUTIL_COMPARATOR_H__
