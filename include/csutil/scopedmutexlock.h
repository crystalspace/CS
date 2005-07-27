/*
  Crystal Space Scoped Mutex Lock Class
  Copyright (C) 2003 by Matze Braun

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
#ifndef __CSUTIL_SCOPEDMUTEX_H__
#define __CSUTIL_SCOPEDMUTEX_H__

#include "csextern.h"
#include "csutil/thread.h"

/**
 * This is a utility class for locking a Mutex. If A MutexLock class is
 * created it locks the mutex, when it is destroyed it unlocks the Mutex
 * again. So locking a mutex can happen by creating a MutexLock object on the
 * stack. The compiler will then take care that the Unlock calls will be done
 * in each case.
 * Example:
 *   void Myfunc() {
 *      csScopedMutexLock lock(mymutex);
 *      do something special
 *
 *      return;
 *  }
 */
class CS_CRYSTALSPACE_EXPORT csScopedMutexLock
{
public:
  csScopedMutexLock (csMutex* newmutex)
    : mutex(newmutex)
  { mutex->LockWait (); }
  ~csScopedMutexLock ()
  { mutex->Release (); }

  csMutex* mutex;
};

#endif

