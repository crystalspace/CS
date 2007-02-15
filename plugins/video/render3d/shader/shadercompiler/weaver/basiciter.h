/*
  Copyright (C) 2003-2007 by Marten Svanfeldt
		2004-2007 by Frank Richter

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

#ifndef __CS_BASICITER_H__
#define __CS_BASICITER_H__

#include "csutil/customallocated.h"

CS_PLUGIN_NAMESPACE_BEGIN(ShaderWeaver)
{
  template<typename T>
  struct BasicIterator : public CS::Memory::CustomAllocated
  {
    virtual ~BasicIterator() {}
  
    virtual bool HasNext() = 0;
    virtual const T& Next() = 0;
  };
  
  template<typename T, class ArrayType>
  class BasicIteratorImpl : public BasicIterator<T>
  {
    const ArrayType& array;
    size_t pos;
  public:
    BasicIteratorImpl (const ArrayType& array) : array (array), pos (0) {}
  
    virtual bool HasNext()
    { return pos < array.GetSize(); }
    virtual const T& Next()
    { return array[pos++]; }
  };
}
CS_PLUGIN_NAMESPACE_END(ShaderWeaver)

#endif // __CS_BASICITER_H__
