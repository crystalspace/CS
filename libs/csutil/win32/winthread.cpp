/*
    Copyright (C) 2002 by Norman Kraemer

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

#include "cssysdef.h"
#include "csutil/sysfunc.h"
#include "winthread.h"
#include <windows.h>
#include <process.h>

#include "csutil/win32/wintools.h"

static inline bool CheckLastError (char*& lasterr)
{
  DWORD errCode = GetLastError();
  delete[] lasterr;
  if (errCode != NO_ERROR)
  {
    lasterr = cswinGetErrorMessage (errCode);
  }
  else
  {
    lasterr = 0;
  }
  return (errCode != NO_ERROR);
}

static inline void ShowError (const char* message, const char* lasterr)
{
#ifdef CS_DEBUG
  csPrintf ("'%s' failed: %s\n", message, lasterr);
#else
  /* Pacify the compiler warnings */
  (void)message;
  (void)lasterr;
#endif
}

static inline void TestError (bool result, const char* message, char*& lasterr)
{
  if (!result)
  {
    if (CheckLastError (lasterr))
      ShowError (message, lasterr);
  }
}

#define CS_TEST(x)    TestError ((x), #x, lasterr);

csRef<csMutex> csMutex::Create (bool recursive)
{
  return csPtr<csMutex>(new csWinMutex (recursive));
}

csWinMutex::csWinMutex (bool recurse) : lasterr(0), recursive(recurse)
{
  CS_TEST ((mutex = CreateMutex (0, false, 0)) != 0);
}

csWinMutex::~csWinMutex ()
{
#ifdef CS_DEBUG
  //  CS_ASSERT (Destroy ());
#endif
  Destroy ();
  delete[] lasterr;
}

bool csWinMutex::Destroy ()
{
  bool rc;
  CS_TEST (rc = CloseHandle (mutex));
  return rc;
}

bool csWinMutex::LockWait()
{
  bool rc;
  CS_TEST (rc = (WaitForSingleObject (mutex, INFINITE) != WAIT_FAILED));
  return rc;
}

bool csWinMutex::LockTry ()
{
  bool rc;
  CS_TEST (rc = (WaitForSingleObject (mutex, 0) != WAIT_FAILED));
  return rc;
}

bool csWinMutex::Release ()
{
  bool rc;
  CS_TEST (rc = ReleaseMutex (mutex));
  return rc;
}

char const* csWinMutex::GetLastError () const
{
  return (char const*)lasterr;
}

bool csWinMutex::IsRecursive() const
{
  return recursive;
}


csRef<csSemaphore> csSemaphore::Create (uint32 value)
{
  return csPtr<csSemaphore>(new csWinSemaphore (value));
}

csWinSemaphore::csWinSemaphore (uint32 v)
{
  lasterr = 0;
  value = v;
  CS_TEST ((sem = CreateSemaphore (0, (LONG)value, (LONG)value, 0)) != 0);
  if (sem == 0)
    value = 0;
}

csWinSemaphore::~csWinSemaphore ()
{
  Destroy ();
  delete[] lasterr;
}

bool csWinSemaphore::LockWait ()
{
  bool rc;
  CS_TEST (rc = (WaitForSingleObject (sem, INFINITE) != WAIT_FAILED));
  if (rc)
    value--;
  return rc;
}

bool csWinSemaphore::LockTry ()
{
  bool rc;
  CS_TEST (rc = (WaitForSingleObject (sem, 0) != WAIT_FAILED));
  if (rc)
    value--;
  return rc;
}

bool csWinSemaphore::Release ()
{
  bool rc;
  CS_TEST (rc = ReleaseSemaphore (sem, 1, &value));
  if (rc)
    value++;
  return rc;
}

uint32 csWinSemaphore::Value ()
{
  return (uint32)value;
}

bool csWinSemaphore::Destroy ()
{
  bool rc;
  CS_TEST (rc = CloseHandle (sem));
  return rc;
}

char const* csWinSemaphore::GetLastError () const
{
  return (char const*)lasterr;
}


csRef<csCondition> csCondition::Create (uint32 conditionAttributes)
{
  return csPtr<csCondition>(new csWinCondition (conditionAttributes));
}

csWinCondition::csWinCondition (uint32 /*conditionAttributes*/)
{
  lasterr = 0;
  CS_TEST ((cond = CreateEvent (0, false, false, 0)) != 0); // auto-reset
}

csWinCondition::~csWinCondition ()
{
  Destroy ();
  delete[] lasterr;
}

void csWinCondition::Signal (bool /*WakeAll*/)
{
  // only releases one waiting thread, coz its auto-reset
  CS_TEST (PulseEvent (cond));
}

bool csWinCondition::Wait (csMutex* mutex, csTicks timeout)
{
  // @@@ FIXME: SignalObjectAndWait() is only available in WinNT 4.0 and
  // above so we use the potentially dangerous version below.
  if (mutex->Release () && LockWait (timeout==0?INFINITE:(DWORD)timeout))
    return mutex->LockWait ();
  return false;
}

bool csWinCondition::LockWait (DWORD nMilliSec)
{
  bool rc;
  CS_TEST (rc = (WaitForSingleObject (cond, nMilliSec) != WAIT_FAILED));
  return rc;
}

bool csWinCondition::Destroy ()
{
  bool rc;
  CS_TEST (rc = CloseHandle (cond));
  return rc;
}

char const* csWinCondition::GetLastError () const
{
  return (char const*)lasterr;
}


csRef<csThread> csThread::Create (csRunnable* r, uint32 options)
{
  return csPtr<csThread>(new csWinThread (r, options));
}

csWinThread::csWinThread (csRunnable* r, uint32 /*options*/)
{
  runnable = r;
  running = false;
  lasterr = 0;
}

csWinThread::~csWinThread ()
{
  if (running)
    Stop ();
  CloseHandle (thread);
  delete[] lasterr;
}

bool csWinThread::Start ()
{
  #if defined (__CYGWIN__)
    thread = CreateThread (0, 0, (LPTHREAD_START_ROUTINE) ThreadRun,
      (void*)this, CREATE_SUSPENDED, 0);
  #else
    uint dummyThreadId;
    thread = (HANDLE)_beginthreadex (0, 0, ThreadRun, (void*)this,
      CREATE_SUSPENDED, &dummyThreadId);
  #endif

  bool created;
  CS_TEST (created = (thread != 0));
  CS_TEST (running = (ResumeThread (thread) != (DWORD)-1));
  return running;
}

bool csWinThread::Wait ()
{
  if (running)
  {
    bool rc;
    CS_TEST (rc = (WaitForSingleObject (thread, INFINITE) != WAIT_FAILED));
    return rc;
  }
  return true;
}

void csWinThread::Yield ()
{
  if (running)
    Sleep(0);
}

bool csWinThread::Stop ()
{
  if (running)
  {
    CS_TEST (!(running = !TerminateThread (thread, ~0)));
  }
  return !running;
}

char const* csWinThread::GetLastError () const
{
  return (char const*)lasterr;
}

uint csWinThread::ThreadRun (void* param)
{
  csWinThread *thread = (csWinThread*)param;
  thread->runnable->Run ();
  thread->running = false;
  #if defined (__CYGWIN__)
    ExitThread (0);
  #else
    _endthreadex (0);
  #endif
  return 0;
}
