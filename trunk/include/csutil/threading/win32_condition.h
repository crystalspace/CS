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

#ifndef __CS_CSUTIL_THREADING_WIN32_CONDITION_H__
#define __CS_CSUTIL_THREADING_WIN32_CONDITION_H__

#if !defined(CS_PLATFORM_WIN32)
#error "This file is only for Windows and requires you to include csysdefs.h before"
#else

#include "csutil/threading/atomicops.h"
#include "csutil/threading/mutex.h"
#include "csutil/threading//win32_apifuncs.h"
#include "csutil/noncopyable.h"

namespace CS
{
namespace Threading
{
namespace Implementation
{

  static void __stdcall NotifyFunc (Implementation::ulong_ptr)
  {
  }  

  class ConditionBase
  {
  public:
    ConditionBase ()
    {
      waitingList.next = &waitingList;
      waitingList.prev = &waitingList;
    }

    void NotifyOne ()
    {
      StateGateLock lock (stateGate);

      if (waitingList.prev != &waitingList)
      {
        WaitListEntry* const entry = waitingList.prev;
        entry->Unlink ();
        NotifyEntry (entry);
      }
    }

    void NotifyAll ()
    {
      StateGateLock lock (stateGate);

      WaitListEntry* head = waitingList.prev;
      waitingList.prev = &waitingList;
      waitingList.next = &waitingList;

      while (head != &waitingList)
      {
        WaitListEntry* const prev = head->prev;
        NotifyEntry (head);
        head = prev;
      }
    }

    template<typename LockType>
    bool Wait (LockType& lock, csTicks timeout)
    {
      WaitListEntry waitEntry;

      void* const currentProcess = Implementation::GetCurrentProcess ();
      void* const currentThread = Implementation::GetCurrentThread ();

      Implementation::DuplicateHandle (currentProcess, currentThread, currentProcess,
        &waitEntry.waitingThreadHandle, 0, false, DUPLICATE_SAME_ACCESS);

      {
        AddEntryHelper<LockType> entryGuard (this, waitEntry, lock);

	DWORD r;
        // Loop until notified
        while (!AtomicOperations::Read (&waitEntry.notified) &&
          (r = Implementation::SleepEx (timeout == 0 ? INFINITE : timeout, 
	    true)) == WAIT_IO_COMPLETION)
          ;
	return r != 0;
      }
    }


  protected:

    // Keep a list of waiting threads
    struct WaitListEntry
    {
      void* waitingThreadHandle;
      int32 notified;
      WaitListEntry* next;
      WaitListEntry* prev;
    
      WaitListEntry ()
        : waitingThreadHandle (0), notified (0), next (0), prev (0)
      {
      }

      void Unlink ()
      {
        next->prev = prev;
        prev->next = next;
        next = this;
        prev = this;
      }
    };

    // Guard our own state
    Mutex stateGate;
    typedef ScopedLock<Mutex> StateGateLock;

    WaitListEntry waitingList;

    // Helper-class to add/remove entries from waiting list
    template<typename LockType>
    struct AddEntryHelper
    {
      ConditionBase* owner;
      WaitListEntry& entry;
      LockType& lock;

      AddEntryHelper (ConditionBase* owner, WaitListEntry& entry, LockType& lock)
        : owner (owner), entry (entry), lock (lock)
      {
        entry.prev = &owner->waitingList;
        StateGateLock locks (owner->stateGate);

        entry.next = owner->waitingList.next;
        owner->waitingList.next = &entry;
        entry.next->prev = &entry;

        lock.Unlock ();
      }

      ~AddEntryHelper ()
      {
        if (!entry.notified)
        {
          StateGateLock locks (owner->stateGate);
          if (!entry.notified)
          {
            entry.Unlink ();
          }
        }

        Implementation::CloseHandle ((HANDLE)entry.waitingThreadHandle);
        lock.Lock ();
      }
    };
    template<typename LockType> friend struct AddEntryHelper;

    void NotifyEntry (WaitListEntry* entry)
    {
      AtomicOperations::Set (&entry->notified, 1);
      if (entry->waitingThreadHandle)
      {
        // Wake up thread
        Implementation::QueueUserAPC (NotifyFunc, entry->waitingThreadHandle, 0);
      }
    }
  };

}
}
}

#endif // !defined(CS_PLATFORM_WIN32)

#endif // __CS_CSUTIL_THREADING_WIN32_CONDITION_H__
