/*
    Copyright (C) 2002 by Norman Krämer
  
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
#include "cssys/thread.h"
#include <process.h>

#ifdef CS_DEBUG
#define CS_SHOW_ERROR if (lasterr) printf ("%s\n",lasterr)
#else
#define CS_SHOW_ERROR
#endif

#define CS_GET_SYSERROR() \
if (lasterr){LocalFree (lasterr); lasterr = NULL;}\
FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | \
			   FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, \
			   NULL, (DWORD)GetLastError (), \
			   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lasterr, 0, NULL)

#define CS_TEST(x) if(!(x)) {CS_GET_SYSERROR(); CS_SHOW_ERROR;}

csMutex* csMutex::Create ()
{
  return new csWinMutex ();
}

csWinMutex::csWinMutex ()
{
  lasterr = NULL;
  mutex = CreateMutex (NULL, false, NULL);
  CS_TEST (mutex != NULL);
}

csWinMutex::~csWinMutex ()
{
#ifdef CS_DEBUG
  //  CS_ASSERT (Destroy ());
  Destroy ();
#else
  Destroy ();
#endif
  if (lasterr) {LocalFree (lasterr); lasterr = NULL;}
}

bool csWinMutex::Destroy ()
{
  bool rc=CloseHandle (mutex);
  CS_TEST (rc);
  return rc;
}

bool csWinMutex::LockWait()
{
  bool rc = (WaitForSingleObject (mutex, INFINITE) != WAIT_FAILED);
  CS_TEST (rc);
  return rc;
}

bool csWinMutex::LockTry ()
{
  bool rc = (WaitForSingleObject (mutex, 1) != WAIT_FAILED);
  CS_TEST (rc);
  return rc;
}

bool csWinMutex::Release ()
{
  bool rc = ReleaseMutex (mutex);
  CS_TEST (rc);
  return rc;
}

const char* csWinMutex::GetLastError ()
{
  return (const char*)lasterr;
}


csSemaphore* csSemaphore::Create (uint32 value)
{
  return new csWinSemaphore (value);
}

csWinSemaphore::csWinSemaphore (uint32 value)
{
  lasterr = NULL;
  value=0;
  sem = CreateSemaphore (NULL, (LONG)value, (LONG)value, NULL);
  if (sem != NULL) this->value=value;
  CS_TEST (sem != NULL);
}

csWinSemaphore::~csWinSemaphore ()
{
  Destroy ();
}

bool csWinSemaphore::LockWait ()
{
  bool rc = (WaitForSingleObject (sem, INFINITE) != WAIT_FAILED);
  if (rc) value--;
  CS_TEST (rc);
  return rc;
}

bool csWinSemaphore::LockTry ()
{
  bool rc = (WaitForSingleObject (sem, 1) != WAIT_FAILED);
  if (rc) value--;
  CS_TEST (rc);
  return rc;
}


bool csWinSemaphore::Release ()
{
  bool rc=ReleaseSemaphore (sem, 1, &value);
  if (rc) value++;
  CS_TEST (rc);
  return rc;
}

uint32 csWinSemaphore::Value ()
{
  return (uint32)value;
}

bool csWinSemaphore::Destroy ()
{
  bool rc=CloseHandle (sem);
  CS_TEST (rc);
  return rc;
}

const char* csWinSemaphore::GetLastError ()
{
  return (const char*)lasterr;
}

csCondition* csCondition::Create (uint32 conditionAttributes)
{
  return new csWinCondition (conditionAttributes);
}

csWinCondition::csWinCondition (uint32 /*conditionAttributes*/)
{
  lasterr = NULL;
  cond = CreateEvent (NULL, false, false, NULL); // auto-reset
  CS_TEST (cond != NULL);
}

csWinCondition::~csWinCondition ()
{
  Destroy ();
}

void csWinCondition::Signal (bool bAll)
{
  bool rc = PulseEvent (cond); // only releases one waiting thread, coz its auto-reset
  CS_TEST (rc);
}

bool csWinCondition::Wait (csMutex *mutex, csTicks timeout)
{
  DWORD rc;
  // SignalObjectAndWait is only available in WinNT 4.0 and above
  // so we use the potentially dangerous version below
  if (mutex->Release () && LockWait ((DWORD)timeout))
    return mutex->LockWait ();
  return false;
}

bool csWinCondition::LockWait (DWORD nMilliSec)
{
  bool rc = (WaitForSingleObject (cond, nMilliSec) != WAIT_FAILED);
  CS_TEST (rc);
  return rc;
}

bool csWinCondition::Destroy ()
{
  bool rc = CloseHandle (cond);
  CS_TEST (rc);
  return rc;
}

const char* csWinCondition::GetLastError ()
{
  return (const char*)lasterr;
}

csThread* csThread::Create (iRunnable *runnable, uint32 options)
{
  return new csWinThread (runnable, options);
}

csWinThread::csWinThread (iRunnable *runnable, uint32 /*options*/)
{
  csWinThread::runnable = runnable;
  running = false;
  lasterr = NULL;
}

csWinThread::~csWinThread ()
{
  if (running)
    Kill ();
}

bool csWinThread::Start ()
{
  thread = (HANDLE)_beginthread (ThreadRun, 0, (void*)this);
  running = ((unsigned long)thread != -1);
  CS_TEST (running);
  return running;
}

bool csWinThread::Stop ()
{
  return Kill ();
}

bool csWinThread::Wait ()
{
  if (running)
  {
    bool rc = (WaitForSingleObject (thread, INFINITE) != WAIT_FAILED);
	CS_TEST (rc);
	return rc;
  }
  return true;
}

bool csWinThread::Kill ()
{
  if (running)
  {
    running = !TerminateThread (thread, -1);
	CS_TEST (!running);
  }
  return !running;
}

const char *csWinThread::GetLastError ()
{
  return (const char*)lasterr;
}

void csWinThread::ThreadRun (void *param)
{
  csWinThread *thread = (csWinThread *)param;
  thread->runnable->Run ();
  thread->running = false;
  _endthread();
}

#undef CS_TEST
#undef CS_SHOW_ERROR
