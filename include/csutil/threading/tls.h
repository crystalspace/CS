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
/**\file
 * Thread Local Storage
 */

#ifndef __CS_CSUTIL_THREADING_TLS_H__
#define __CS_CSUTIL_THREADING_TLS_H__

#include "../noncopyable.h"

#if defined(CS_PLATFORM_WIN32)
# include "csutil/threading/win32_tls.h"
#elif defined(CS_PLATFORM_UNIX) || \
  defined(CS_PLATFORM_MACOSX)
# include "csutil/threading/pthread_tls.h"
#else
  #ifndef DOXYGEN_RUN
    #error "No TLS implementation for your platform"
  #endif
#endif

namespace CS
{
  namespace Threading
  {
    using namespace Implementation;
    
#ifdef DOXYGEN_RUN
      /**
       * Thread local storage of a pointer value (void*).
       * Wraps the platform-specific TLS details.
       * 
       * Optionally, a "destructor" function can be specified which is called
       * when the thread exists and the TLS value for the thread is not null.
       * Note it's called <em>only</em> on thread exit, not when the TLS slot
       * itself is destroyed!
       * (Essentially, the same as pthreads behaves.)
       *
       * \warning Proper destructor function support is only guaranteed if
       *   CS' Thread facility is used.
       */
      class ThreadLocalBase
      {
      public:
	/// TLS value destructor function
	typedef void (* DestructorFn)(void*);
	
	/// Create a TLS slot with the given destructor function
        ThreadLocalBase (DestructorFn dtor = 0);
	/// Delete the TLS slot
        ~ThreadLocalBase();
	/// Set the TLS slot value for the current thread.
        void SetValue (void* data) const;
	/// Get the TLS slot value for the current thread. Defaults to 0.
        void* GetValue () const;
      };
#endif

    /**
     * Thread local storage of arbitrary C++ objects.
     * Stores an arbitrary object in a TLS slot, automatically creating an
     * instance and destroying it when the thread exits.
     * \remarks Actually, an object instance is allocated on the heap.
     * For small objects (integral values, simple pointers) it may be more
     * efficient to use ThreadLocalBase directly or to coalesce multiple
     * such small objects into a larger struct which is then stored thread
     * locally (instead of having multiple small TLS slots).
     */
    template<typename T>
    class ThreadLocal : protected ThreadLocalBase,
			public CS::NonCopyable
    {
      static void Destructor (void* p)
      {
	delete reinterpret_cast<T*> (p);
      }
      
      /// Query store object or create if none is stored yet
      T& GetObject() const
      {
	T* p = reinterpret_cast<T*> (ThreadLocalBase::GetValue ());
	if (p == 0)
	{
	  p = new T;
	  ThreadLocalBase::SetValue (p);
	}
	return *p;
      }
    public:
      ThreadLocal() : ThreadLocalBase (Destructor)
      {
      }
      ~ThreadLocal()
      {
	// Destructor function does _not_ kick in when deleting the TLS slot!
	T* p = reinterpret_cast<T*> (ThreadLocalBase::GetValue ());
	delete p;
      }
      
      /// Checks whether an instance was ever created for the current thread
      bool HasValue() const
      {
	return ThreadLocalBase::GetValue () != 0;
      }

      /// Access object for current thread (create, if necessary)
      operator T& () const
      {
	return GetObject();
      }
      
      /// Access object for current thread (create, if necessary)
      T* operator->() const
      {
	return &(GetObject());
      }
      
      /// Access object for current thread (create, if necessary)
      T& operator *() const
      {
	return GetObject();
      }

      /// Assign an instance to the object for the current thread (create, if necessary)
      ThreadLocal& operator = (const T& other)
      {
	T* p = reinterpret_cast<T*> (ThreadLocalBase::GetValue ());
	if (p == 0)
	{
	  p = new T (other);
	  ThreadLocalBase::SetValue (p);
	}
	else
	  *p = other;
        return *this;
      } 
    };
  } // Threading
} // CS

#endif // __CS_CSUTIL_THREADING_TLS_H__
