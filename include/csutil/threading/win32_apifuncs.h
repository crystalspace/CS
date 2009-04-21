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

#ifndef __CS_CSUTIL_THREADING_WIN32_APIFUNCS_H__
#define __CS_CSUTIL_THREADING_WIN32_APIFUNCS_H__


namespace CS
{
namespace Threading
{
// Do our own private declarations of some win32 functions. This could
// potentially be dangerous, but as Microsoft haven't changed these since
// dawn of time, we will live with that.
namespace Implementation
{
#ifdef _WIN64
  typedef uint64 ulong_ptr;
#else
  typedef uint32 ulong_ptr;
#endif

  extern "C"
  {
    __declspec(dllimport) 
    int __stdcall CloseHandle(void*);

    __declspec(dllimport) 
    void* __stdcall CreateSemaphoreA(_SECURITY_ATTRIBUTES*,long,long,char const*);

    __declspec(dllimport) 
    void* __stdcall GetCurrentProcess();

    __declspec(dllimport) 
    unsigned long __stdcall GetCurrentProcessId();

    __declspec(dllimport) 
    void* __stdcall GetCurrentThread();

    __declspec(dllimport)
    unsigned long __stdcall GetCurrentThreadId();    

    __declspec(dllimport)
    int __stdcall DuplicateHandle(void*,void*,void*,void**,unsigned long,
                                  int,unsigned long);

    typedef void (__stdcall *queue_user_apc_callback_function)(ulong_ptr);
    __declspec(dllimport) 
    unsigned long __stdcall QueueUserAPC(queue_user_apc_callback_function,
                                          void*,ulong_ptr);

    __declspec(dllimport) 
    int __stdcall ReleaseSemaphore(void*,long,long*);

    __declspec(dllimport) 
    unsigned long __stdcall SleepEx(unsigned long,int);

    __declspec(dllimport)
    unsigned long __stdcall WaitForSingleObject(void*,unsigned long);

  }

}
}
}

#endif
