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

#ifndef __CS_CSUTIL_THREADING_WIN32_THREAD_H__
#define __CS_CSUTIL_THREADING_WIN32_THREAD_H__

#include "csutil/sysfunc.h"

#if !defined(CS_PLATFORM_WIN32)
#error "This file is only for Windows and requires you to include csysdefs.h before"
#endif


namespace CS
{
namespace Threading
{

  enum ThreadPriority;
  class Runnable;

namespace Implementation
{

  // Thread base-class for win32
  class ThreadBase
  {
  public:
    ThreadBase (Runnable* runnable);

    void Start ();

    void Stop ();

    bool IsRunning () const;

    bool SetPriority (ThreadPriority prio);
    
    void Wait () const;

    static void Yield ();

  private:
    csRef<Runnable> runnable;

    pthread_thread_t threadHandle;
    
    int32 isRunning;
  };


}
}
}

#endif
