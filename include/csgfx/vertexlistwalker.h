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

template<class T>
class csVertexListWalker
{
public:
  csVertexListWalker (void *listPointer, uint elements, uint stride = sizeof(T))
    : list ((uint8*)listPointer), elements (elements), stride (stride), currElement (0)
  {
    if (stride == 0)
      this->stride = sizeof(T);
  }

  T& operator*()
  {
    return *((T*)(list+currElement*stride));
  }

  T& operator[] (size_t n)
  {
    CS_ASSERT (n<elements);
    return *((T*)(list+n*stride));
  }

  void operator++ ()
  {
    currElement++;
    CS_ASSERT(currElement<elements);
  }

private:
  uint8 *list;
  uint elements, stride;
  uint currElement;
};

#endif
