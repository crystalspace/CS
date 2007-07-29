/*
  Crystal Space General Algorithms
  Copyright (C)2005 by Marten Svanfeldt

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

#ifndef __CSUTIL_ALGORITHMS_H__
#define __CSUTIL_ALGORITHMS_H__

/**\file
 * General Algorithms
 */

namespace CS
{

  /**
   * Swap two elements
   */
  template <typename T>
  CS_FORCEINLINE_TEMPLATEMETHOD void Swap (T& a, T& b)
  {
    T tmp = a;
    a = b;
    b = tmp;
  }

  /**
   * Iterate over all elements in the iterator and perform operation
   * given by Func.
   * \code
   * csArray<int> anArray;
   * anArray.Push (1);
   * anArray.Push (4);
   * ForEach (anArray.GetIterator (), OurFunctor ());
   * \endcode
   */
  template <typename T, typename Fn>
  CS_FORCEINLINE_TEMPLATEMETHOD Fn& ForEach (T it, Fn& Func)
  {
    while (it.HasNext ())
    {
      Func (it.Next ());
    }
    return Func;
  }

  /**
   * Iterate over all elements in the list and perform operation
   * given by Func.
   */
  template <typename T, typename Fn>
  CS_FORCEINLINE_TEMPLATEMETHOD Fn& ForEach (T* start, T* end, Fn& Func)
  {
    while (start != end)
    {
      Func (*start);
      start++;
    }
    return Func;
  }

  /**
   * Iterate over all elements in the iterator and perform operation
   * given by Func.
   */
  template <typename T, typename Fn, typename P>
  CS_FORCEINLINE_TEMPLATEMETHOD Fn& ForEach (T it, Fn& Func, P& p)
  {
    while (it.HasNext ())
    {
      Func (it.Next (), p);
    }
    return Func;
  }

  /**
   * Iterate over all elements in the iterator and perform operation
   * given by Func.
   */
  template <typename T, typename Fn, typename P1, typename P2>
  CS_FORCEINLINE_TEMPLATEMETHOD Fn& ForEach (T it, Fn& Func, P1& p1, P2& p2)
  {
    while (it.HasNext ())
    {
      Func (it.Next (), p1, p2);
    }
    return Func;
  }

  /**
   * Iterate over all elements in the iterator and perform operation
   * given by Func.
   */
  template <typename T, typename Fn, typename P1, typename P2, typename P3>
  CS_FORCEINLINE_TEMPLATEMETHOD Fn& ForEach (T it, Fn& Func, P1& p1, P2& p2, P3& p3)
  {
    while (it.HasNext ())
    {
      Func (it.Next (), p1, p2, p3);
    }
    return Func;
  }


}

#endif // __CSUTIL_ALGORITHMS_H__
