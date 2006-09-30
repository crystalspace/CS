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

#ifndef __CS_CSUTIL_THREADING_PTHREAD_CONDITION_H__
#define __CS_CSUTIL_THREADING_PTHREAD_CONDITION_H__


#include "csutil/threading/atomicops.h"
#include "csutil/threading/mutex.h"
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
      pthread_cond_init (&condition, 0);
    }

    void NotifyOne ()
    {
      pthread_cond_signal (&condition);
    }

    void NotifyAll ()
    {
      pthread_cond_broadcast (&condition);
    }

    template<typename LockType>
    void Wait (LockType& lock)
    {
      pthread_cond_wait (&condition, &lock.mutex);
    }


  protected:
    pthread_cond_t condition;
  };

}
}
}

#endif
