/*
  Copyright (C) 2005 by Marten Svanfeldt

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

#ifndef __CS_CSGFX_VERTEXLISTWALKER_H__
#define __CS_CSGFX_VERTEXLISTWALKER_H__

/**\file
 * Helper to access elements from renderbuffers.
 */

/**
 * Helper class to make it easier to access single elements from
 * renderbuffers (with stride)
 */
template<class T>
class csVertexListWalker
{
public:
  /**
   * Construct new walker.
   * \param listPointer Pointer to buffer data.
   * \param elements Number of elements in the buffer.
   * \param dist Distance, in bytes, between two elements in the buffer.
   */
  csVertexListWalker (void *listPointer, size_t elements, size_t dist = sizeof(T))
    : list ((uint8*)listPointer), elements (elements), dist (dist), currElement (0)
  {
    if (dist == 0)
      this->dist = sizeof(T);
  }

  /// Get current element.
  T& operator*()
  {
    return *((T*)(list+currElement*dist));
  }

  /// Get a specific element.
  T& operator[] (size_t n)
  {
    CS_ASSERT (n<elements);
    return *((T*)(list+n*dist));
  }

  /// Set current element to the next.
  void operator++ ()
  {
    currElement++;
    CS_ASSERT(currElement<elements);
  }

private:
  /// Buffer data
  uint8 *list;
  /// Number of elements
  size_t elements;
  /// Distance between two elements
  size_t dist;
  /// Index of current element
  size_t currElement;
};

#endif
