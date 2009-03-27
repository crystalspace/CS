/*
 (C) Copyright 2006-8 Anthony Williams

  Distributed under the Boost Software License, Version 1.0. (See
  accompanying file LICENSE_1_0.txt or copy at
  http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef __CS_CSUTIL_THREADING_RWMUTEX_H__
#define __CS_CSUTIL_THREADING_RWMUTEX_H__

#include "csutil/threading/condition.h"
#include "csutil/threading/mutex.h"

namespace CS
{
namespace Threading
{
  /**
   * A mutex class which provides concurrent reads and exclusive writes.
   */
  class ReadWriteMutex
  {
  private:
    struct StateData
    {
      unsigned shared_count;
      bool exclusive;
      bool upgrade;
      bool exclusive_waiting_blocked;

      StateData() : shared_count(0),
        exclusive(false), upgrade(false),
        exclusive_waiting_blocked(false)
      {
      }
    };

    StateData state;
    Mutex stateChange;
    Condition sharedCond;
    Condition exclusiveCond;
    Condition upgradeCond;

    void ReleaseWaiters()
    {
      exclusiveCond.NotifyOne();
      sharedCond.NotifyAll();
    }

  public:
    ReadWriteMutex()
    {
    }

    ~ReadWriteMutex()
    {
    }

    void ReadLock()
    {
      MutexScopedLock lock(stateChange);

      while(state.exclusive || state.exclusive_waiting_blocked)
      {
        sharedCond.Wait(stateChange);
      }
      ++state.shared_count;
    }

    void ReadUnlock()
    {
      MutexScopedLock lock(stateChange);
      bool const last_reader=!--state.shared_count;

      if(last_reader)
      {
        if(state.upgrade)
        {
          state.upgrade=false;
          state.exclusive=true;
          upgradeCond.NotifyOne();
        }
        else
        {
          state.exclusive_waiting_blocked=false;
        }
        ReleaseWaiters();
      }
    }

    void WriteLock()
    {
      MutexScopedLock lock(stateChange);

      while(state.shared_count || state.exclusive)
      {
        state.exclusive_waiting_blocked=true;
        exclusiveCond.Wait(stateChange);
      }
      state.exclusive=true;
    }

    void WriteUnlock()
    {
      MutexScopedLock lock(stateChange);
      state.exclusive=false;
      state.exclusive_waiting_blocked=false;
      ReleaseWaiters();
    }

    void UpgradeLock()
    {
      MutexScopedLock lock(stateChange);
      while(state.exclusive || state.exclusive_waiting_blocked || state.upgrade)
      {
        sharedCond.Wait(stateChange);
      }
      ++state.shared_count;
      state.upgrade=true;
    }

    void UpgradeUnlock()
    {
      MutexScopedLock lock(stateChange);
      state.upgrade=false;
      bool const last_reader=!--state.shared_count;

      if(last_reader)
      {
        state.exclusive_waiting_blocked=false;
        ReleaseWaiters();
      }
    }

    void UpgradeUnlockAndWriteLock()
    {
      MutexScopedLock lock(stateChange);
      --state.shared_count;
      while(state.shared_count)
      {
        upgradeCond.Wait(stateChange);
      }
      state.upgrade=false;
      state.exclusive=true;
    }

    void WriteUnlockAndUpgradeLock()
    {
      MutexScopedLock lock(stateChange);
      state.exclusive=false;
      state.upgrade=true;
      ++state.shared_count;
      state.exclusive_waiting_blocked=false;
      ReleaseWaiters();
    }

    void WriteUnlockAndReadLock()
    {
      MutexScopedLock lock(stateChange);
      state.exclusive=false;
      ++state.shared_count;
      state.exclusive_waiting_blocked=false;
      ReleaseWaiters();
    }

    void UpgradeUnlockAndReadLock()
    {
      MutexScopedLock lock(stateChange);
      state.upgrade=false;
      state.exclusive_waiting_blocked=false;
      ReleaseWaiters();
    }
  };

  class ScopedWriteLock
  {
  public:
    ScopedWriteLock (ReadWriteMutex& lockObj)
      : lockObj (lockObj)
    {
      lockObj.WriteLock ();
    }

    ~ScopedWriteLock ()
    {
      lockObj.WriteUnlock ();
    }

  private:
    ReadWriteMutex& lockObj;    
  };

  class ScopedReadLock
  {
  public:
    ScopedReadLock (ReadWriteMutex& lockObj)
      : lockObj (lockObj)
    {
      lockObj.ReadLock ();
    }

    ~ScopedReadLock ()
    {
      lockObj.ReadUnlock ();
    }

  private:
    ReadWriteMutex& lockObj;    
  };

  class ScopedUpgradeableLock
  {
  public:
    ScopedUpgradeableLock (ReadWriteMutex& lockObj)
      : lockObj (lockObj)
    {
      lockObj.UpgradeLock ();
    }

    ~ScopedUpgradeableLock ()
    {
      lockObj.UpgradeUnlock ();
    }

  private:
    ReadWriteMutex& lockObj;    
  };
}
}

#endif
