/*
  Copyright (C) 2010 by Mike Gist

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

#ifndef __CSUTIL_NULLPTR_H__
#define __CSUTIL_NULLPTR_H__

#if !defined(CS_NO_PROVIDE_NULLPTR) && !defined(CS_HAS_NULLPTR)

namespace std
{ 
  const class nullptr_t
  {
  private:
    void operator&() const;

  public:
    template<typename T>
    operator T*() const
    {
      return 0;
    }

    template<class C, typename T>
    operator T C::*() const
    {
      return 0;
    }
  };
}

using ::std::nullptr_t;
nullptr_t nullptr;

#endif // #if !defined(CS_NO_PROVIDE_NULLPTR) && !defined(CS_HAS_NULLPTR)

#endif // __CSUTIL_NULLPTR_H__
