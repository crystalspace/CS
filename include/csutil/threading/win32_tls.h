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

#ifndef __CS_CSUTIL_THREADING_WIN32_TLS_H__
#define __CS_CSUTIL_THREADING_WIN32_TLS_H__

#if !defined(CS_PLATFORM_WIN32)
#error "This file is only for Windows and requires you to include csysdefs.h before"
#else

#include "thread.h"

namespace CS
{
  namespace Threading
  {
    namespace Implementation
    {
      class ThreadLocalBase
      {
      public:
	typedef void (* DestructorFn)(void*);
	
        ThreadLocalBase (DestructorFn dtor = 0) : dtor (dtor)
        {
          threadIndex = TlsAlloc();
	  // Register for later cleanup
	  if (dtor != 0)
	    ThreadBase::RegisterTlsInstance (this);
        }

        ~ThreadLocalBase()
        {
	  // Remove from cleanup set
	  if (dtor != 0)
	    ThreadBase::UnregisterTlsInstance (this);
	  
          if(threadIndex != TLS_OUT_OF_INDEXES)
          {
            TlsFree(threadIndex);
          }
        }

        void SetValue(void* data) const
        {
          TlsSetValue(threadIndex, data);
        }

        void* GetValue() const
        {
          return TlsGetValue(threadIndex);
        }

      protected:
	friend class ThreadBase;
	
	void CleanupInstance ()
	{
	  void* p = TlsGetValue (threadIndex);
	  if (p != 0) dtor (p);
	}
	
        DWORD threadIndex;
	DestructorFn dtor;
      };
    }
  } // Threading
} // CS

#endif // !defined(CS_PLATFORM_WIN32)

#endif // __CS_CSUTIL_THREADING_WIN32_TLS_H__
