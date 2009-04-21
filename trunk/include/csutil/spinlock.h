/*
    Copyright (C) 2006 by Frank Richter

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

#ifndef __CS_CSUTIL_SPINLOCK_H__
#define __CS_CSUTIL_SPINLOCK_H__

/**\file
 */

#ifndef CS_PLATFORM_WIN32
#include <pthread.h>
#include <sched.h>
#endif

/**\addtogroup util
 * @{ */

namespace CS
{
  class SpinLock
  {
  #ifdef CS_PLATFORM_WIN32
    typedef DWORD ThreadID;
  #else
    typedef pthread_t ThreadID;
  #endif
  #if defined (CS_PROCESSOR_X86) && defined (CS_COMPILER_GCC)
    volatile ThreadID threadid;
    volatile uint l;
  #elif defined(CS_PLATFORM_WIN32)
    volatile ThreadID threadid;
    volatile LONG l;
  #else
    pthread_mutex_t l;
  #endif
    
    volatile uint c;
    
    static const int spinsPerYield = 63;
    
  #if defined(CS_PLATFORM_WIN32)
    CS_FORCEINLINE ThreadID CurrentThreadID()
    { return GetCurrentThreadId(); }
  #else
    CS_FORCEINLINE ThreadID CurrentThreadID()
    { return pthread_self(); }
  #endif  
    
    // Spinlock implementation from ptmalloc3's malloc.c
  #if defined (CS_PROCESSOR_X86) && defined (CS_COMPILER_GCC)
    CS_FORCEINLINE bool DoLockWait()
    {
      ThreadID mythreadid = CurrentThreadID();
      if(mythreadid == threadid)
	++c;
      else 
      {
	int spins = 0;
	for (;;) {
	  int ret;
	  __asm__ __volatile__ ("lock; cmpxchgl %2,(%1)" : "=a" (ret) : "r" (&l), "r" (1), "a" (0));
	  if(!ret) 
	  {
	    CS_ASSERT(!threadid);
	    threadid = mythreadid;
	    c = 1;
	    break;
	  }
	  if ((++spins & spinsPerYield) == 0) {
    #if defined(CS_PLATFORM_UNIX)
	    sched_yield();
    #elif defined(CS_PLATFORM_WIN32)
	    SleepEx (0, FALSE);
    #else  /* no-op yield on unknown systems */
	    ;
    #endif /* CS_PLATFORM_UNIX, CS_PLATFORM_WIN32 */
	  }
	}
      }
      return true;
    }
    CS_FORCEINLINE bool DoLockTry()
    {
      int ret;
      __asm__ __volatile__ ("lock; cmpxchgl %2,(%1)" : "=a" (ret) : "r" (&l), "r" (1), "a" (0));
      if(!ret){
	CS_ASSERT(!threadid);
	threadid = CurrentThreadID();
	c=1;
	return true;
      }
      return false;
    }
    CS_FORCEINLINE void DoRelease()
    {
      int ret;
      CS_ASSERT(CurrentThreadID() == threadid);
      if (!--c) {
	threadid=0;
	__asm__ __volatile__ ("xchgl %2,(%1)" : "=r" (ret) : "r" (&l), "0" (0));
      }
    }
    CS_FORCEINLINE void Init() { threadid = 0; c = 0; l = 0; }
    CS_FORCEINLINE void Destroy() {}
    //------------------------------------------------------------------------
  #elif defined(CS_PLATFORM_WIN32)
    CS_FORCEINLINE bool DoLockWait()
    {
      ThreadID mythreadid = CurrentThreadID();
      if(mythreadid == threadid)
	++c;
      else {
	int spins = 0;
	for (;;) {
	  if (!_InterlockedExchange(&l, 1)) {
	    CS_ASSERT(!threadid);
	    threadid = mythreadid;
	    c = 1;
	    break;
	  }
	  if ((++spins & spinsPerYield) == 0)
	    SleepEx (0, FALSE);
	}
      }
      return true;
    }
    CS_FORCEINLINE bool DoLockTry()
    {
      if (!_InterlockedExchange (&l, 1)) {
	CS_ASSERT (!threadid);
	threadid = CurrentThreadID();
	c = 1;
	return true;
      }
      return false;
    }
    CS_FORCEINLINE void DoRelease()
    {
      CS_ASSERT (CurrentThreadID() == threadid);
      if (!--c) {
	threadid = 0;
	_InterlockedExchange (&l, 0);
      }
    }
    CS_FORCEINLINE void Init() { threadid = 0; c = 0; l = 0; }
    CS_FORCEINLINE void Destroy() {}
    //------------------------------------------------------------------------
  #else
    CS_FORCEINLINE bool DoLockWait()
    {
      if(!pthread_mutex_lock(&l)){
	c++;
	return true;
      }
      return false;
    }
    CS_FORCEINLINE bool DoLockTry()
    {
      if(!pthread_mutex_trylock(&l)){
	c++;
	return true;
      }
      return false;
    }
    CS_FORCEINLINE void DoRelease()
    {
       --c;
      pthread_mutex_unlock(&l);
    }
    CS_FORCEINLINE void Init() 
    {
      pthread_mutexattr_t attr;
      c=0;
      if(pthread_mutexattr_init (&attr)) return;
      if(pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE)) return;
      if(pthread_mutex_init (&l, &attr)) return;
      pthread_mutexattr_destroy (&attr);
    }
    CS_FORCEINLINE void Destroy() {}
#endif
  public:
    SpinLock()
    { Init(); }
    ~SpinLock() { Destroy(); }
  
    CS_FORCEINLINE bool LockWait()
    { return DoLockWait(); }
    CS_FORCEINLINE bool LockTry()
    { return DoLockTry(); }
    CS_FORCEINLINE void Release()
    { DoRelease(); }
  };
} // namespace CS

/** @} */

#endif // __CS_CSUTIL_SPINLOCK_H__
