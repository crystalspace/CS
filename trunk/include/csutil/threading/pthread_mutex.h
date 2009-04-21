/*
  Copyright (C) 2006 by Marten Svanfeldt

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

#ifndef __CS_CSUTIL_THREADING_PTHREAD_MUTEX_H__
#define __CS_CSUTIL_THREADING_PTHREAD_MUTEX_H__

#ifndef DOXYGEN_RUN

#include <pthread.h>

namespace CS
{
namespace Threading
{
namespace Implementation
{
  class ConditionBase;

  /**
   * Basic implementation of non-recursive mutex for pthread
   */
  class MutexBase
  {
  public:
    void Initialize ()
    {
      pthread_mutex_init (&mutex, 0);
    }

    void Destroy ()
    {
      pthread_mutex_destroy (&mutex);
    }
    
    bool Lock ()
    {
      int status = pthread_mutex_lock (&mutex);
      return status == 0;
    }

    bool TryLock ()
    {
      int status = pthread_mutex_trylock (&mutex);
      return status == 0;
    }

    void Unlock ()
    {
      pthread_mutex_unlock (&mutex);
    }

  protected:
    friend class ConditionBase;

    pthread_mutex_t mutex;
  };


  /**
  * Basic implementation of recursive mutex for pthread
  */
#ifdef CS_PTHREAD_MUTEX_RECURSIVE
  class RecursiveMutexBase : public MutexBase
  {
  public:
    void Initialize ()
    {
      // Use different initialization
      pthread_mutexattr_t attr;
      pthread_mutexattr_init (&attr);
      pthread_mutexattr_settype (&attr, CS_PTHREAD_MUTEX_RECURSIVE);
      pthread_mutex_init (&mutex, &attr);
    }
  }; 
#else
  
  // Recursive mutex when native recursive mutex isn't supported by pthread
  // Emulate it
  class RecursiveMutexBase : public MutexBase
  {
  public:
    void Initialize ()
    {
      MutexBase::Initialize ();

      recursionCount = 0;
      validID = false;
      pthread_cond_init (&unlockedCond, 0);
    }

    void Destroy ()
    {
      MutexBase::Destroy ();
      pthread_cond_destroy (&unlockedCond);
    }

    bool Lock ()
    {
      MutexBase::Lock ();

      pthread_t tid = pthread_self ();
      if (validID && pthread_equal (threadID, tid))
      {
        ++recursionCount;
      }
      else
      {
        while (validID)
        {
          pthread_cond_wait (&unlockedCond, &mutex);
        }

        threadID = tid;
        validID = true;
        recursionCount = 1;
      }

      MutexBase::Unlock ();
    }

    bool TryLock ()
    {
      bool ret = false;
      MutexBase::Lock ();

      pthread_t tid = pthread_self ();
      if (validID && pthread_equal (threadID, tid))
      {
        ++recursionCount;
        ret = true;
      }
      else if (!validID)
      {
        threadID = tid;
        validID = true;
        recursionCount = 1;
        ret = true;
      }

      MutexBase::Unlock ();
      return ret;
    }

    void Unlock ()
    {
      MutexBase::Lock ();
      pthread_t tid = pthread_self ();
      if (validID && !pthread_equal (threadID, tid))
      {
        MutexBase::Unlock ();
        return;
      }

      if (--recursionCount == 0)
      {
        validID = false;
        pthread_cond_signal (&unlockedCond);
      }

      MutexBase::Unlock ();
    }

  protected:
    pthread_cond_t unlockedCond;
    pthread_t threadID;
    int32 recursionCount;
    bool validID;
  };

#endif

} // namespace Implementation
} // namespace Threading
} // namespace CS

#endif // DOXYGEN_RUN

#endif // __CS_CSUTIL_THREADING_PTHREAD_MUTEX_H__
