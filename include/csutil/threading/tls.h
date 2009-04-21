/*
  Copyright (C) 2008 by Michael Gist

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

#ifndef __CS_CSUTIL_THREADING_TLS_H__
#define __CS_CSUTIL_THREADING_TLS_H__

#ifdef CS_PLATFORM_WIN32
#include "win32_tls.h"
#else
#include "pthread_tls.h"
#endif

namespace CS
{
  namespace Threading
  {
    using namespace Implementation;

    template<typename T>
    class ThreadLocal : public ThreadLocalBase<T>
    {
    private:
      ThreadLocal(ThreadLocal<T> const&);
      ThreadLocal& operator=(ThreadLocal<T> &);

    public:
      ThreadLocal()
      {
      }

      virtual ~ThreadLocal()
      {
      }

      operator T () const
      {
        return ThreadLocalBase<T>::GetValue();
      }

      ThreadLocal& operator = (T other)
      {
        ThreadLocalBase<T>::SetValue(other);
        return *this;
      } 
    };
  } // Threading
} // CS

#endif // __CS_CSUTIL_THREADING_TLS_H__
