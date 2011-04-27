/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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

#ifndef __CS_CSUTIL_FIFO_H__
#define __CS_CSUTIL_FIFO_H__

/**\file
 * FIFO
 */

#include "csutil/array.h"

/**
 * A FIFO implemented on top of csArray, but faster than using just
 * a single array.
 */
template <class T, class ElementHandler = csArrayElementHandler<T>,
  class MemoryAllocator = CS::Container::ArrayAllocDefault,
  class CapacityHandler = csArrayCapacityFixedGrow<16> >
class csFIFO
{
public:
  typedef csFIFO<T, ElementHandler, MemoryAllocator> ThisType;
  typedef T ValueType;
  typedef ElementHandler ElementHandlerType;
  typedef MemoryAllocator AllocatorType;

private:
  csArray<T, ElementHandler, MemoryAllocator, CapacityHandler> a1, a2;
public:
  /**
   * Construct the FIFO. See csArray documentation for meaning of
   * parameters.
   */
  csFIFO (size_t icapacity = 0,
    const CapacityHandler& ch = CapacityHandler()) 
    :  a1 (icapacity, ch), a2 (icapacity, ch) { }

  /**
   * Return and remove the first element.
   */
  T PopTop ()
  {
    CS_ASSERT ((a1.GetSize () > 0) || (a2.GetSize () > 0));
    if (a2.GetSize () == 0)
    {
      size_t n = a1.GetSize ();
      while (n-- > 0)
      {
	a2.Push (a1[n]);
      }
      a1.Empty ();
    }
    return a2.Pop ();
  }

  /**
   * Return the first element
   */
  T& Top ()
  {
    CS_ASSERT ((a1.GetSize () > 0) || (a2.GetSize () > 0));

    if (a2.GetSize () == 0)
    {
      size_t n = a1.GetSize ();
      while (n-- > 0)
      {
        a2.Push (a1[n]);
      }
      a1.Empty ();
    }
    return a2.Top ();
  }

  /**
   * Return and remove the last element
   */
  T PopBottom ()
  {
    CS_ASSERT ((a1.GetSize () > 0) || (a2.GetSize () > 0));

    if(a1.GetSize () > 0)
    {
      return a1.Pop ();
    }
    else
    {
      T tmp = a2[0];
      a2.DeleteIndex (0);
      return tmp;
    }
  }

  /**
   * Return the last element
   */
  T& Bottom ()
  {
    CS_ASSERT ((a1.GetSize () > 0) || (a2.GetSize () > 0));

    if(a1.GetSize () > 0)
    {
      return a1.Top ();
    }
    else
    {
      T tmp = a2[0];
      return tmp;
    }
  }

  /**
   * Push an element onto the FIFO.
   */
  void Push (T const& what)
  {
    a1.Push (what);
  }

  /// Return the number of elements in the FIFO.
  size_t GetSize() const
  {
    return a1.GetSize() + a2.GetSize();
  }

  /**
   * Return the number of elements in the FIFO.
   * \deprecated Use GetSize() instead.
   */
  CS_DEPRECATED_METHOD_MSG("Use GetSize() instead.")
  size_t Length() const
  {
    return GetSize();
  }

  /**
   * Linearly search for an item and delete it.
   * \returns Whether the item was found and subsequently deleled.
   */
  bool Delete (T const& what)
  {
    return (a1.Delete (what) || a2.Delete (what));
  }

  /**
   * Linearly search for an item.
   * \returns Whether the item was found.
   */
  bool Contains (T const& what)
  {
    return ((a1.Find (what) != csArrayItemNotFound)
      || (a2.Find (what) != csArrayItemNotFound));
  }

  /// Delete all items.
  void DeleteAll ()
  {
    a1.DeleteAll();
    a2.DeleteAll();
  }
};

#endif // __CS_CSUTIL_FIFO_H__
