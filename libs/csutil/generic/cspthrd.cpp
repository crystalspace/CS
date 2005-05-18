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
// make sure this is before all other things!
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "cssysdef.h"
#include "cspthrd.h"
#include "csutil/sysfunc.h"
#include <sys/time.h>

// Depending upon the platform, one of these headers declares strerror().
#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef CS_DEBUG
#define CS_SHOW_ERROR if (lasterr) csPrintfErr("%s\n", GetLastError ())
#else
#define CS_SHOW_ERROR
#endif

csRef<csMutex> csMutex::Create (bool recurse)
{
#ifdef CS_PTHREAD_MUTEX_RECURSIVE
  if (recurse)
  {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init (&attr);
    pthread_mutexattr_settype (&attr, CS_PTHREAD_MUTEX_RECURSIVE);
    return csPtr<csMutex> (new csPosixMutex (&attr, recurse));
  }
  return csPtr<csMutex> (new csPosixMutex (0, recurse));
#else
  return csPtr<csMutex>(new csPosixMutexRecursiveEmulator (0, recurse));
#endif
}

csPosixMutex::csPosixMutex (pthread_mutexattr_t* attr, bool recurse) :
  lasterr(0), recursive(recurse)
{
  pthread_mutex_init (&mutex, attr);
}

csPosixMutex::~csPosixMutex ()
{
  lasterr = pthread_mutex_destroy (&mutex);
  CS_SHOW_ERROR;
}

bool csPosixMutex::LockWait()
{
  lasterr = pthread_mutex_lock (&mutex);
  CS_SHOW_ERROR;
  return lasterr == 0;
}

bool csPosixMutex::LockTry ()
{
  lasterr = pthread_mutex_trylock (&mutex);
  CS_SHOW_ERROR;
  return lasterr == 0;
}

bool csPosixMutex::Release ()
{
  lasterr = pthread_mutex_unlock (&mutex);
  CS_SHOW_ERROR;
  return lasterr == 0;
}

char const* csPosixMutex::GetLastError () const
{
  switch (lasterr)
  {
    case EINVAL:
      return "Mutex not initialized";
    case EPERM:
      return "No permission";
    case 0:
      return "";
    default:
      return "Unknown error";
  }
}

bool csPosixMutex::IsRecursive() const
{
  return recursive;
}

//---------------------------------------------------------------------------

#ifndef CS_PTHREAD_MUTEX_RECURSIVE

csPosixMutexRecursiveEmulator::csPosixMutexRecursiveEmulator (
  pthread_mutexattr_t* attr, bool recurse) :
  csPosixMutex(attr,recurse), count(0), owner(0)
{
}

csPosixMutexRecursiveEmulator::~csPosixMutexRecursiveEmulator ()
{
  CS_ASSERT (count==0);
}

bool csPosixMutexRecursiveEmulator::LockWait ()
{
  pthread_t self = pthread_self ();
  
  if (owner != self)
  {
    lasterr = pthread_mutex_lock(&mutex);  
    owner = self;
  }
  else
    lasterr = 0;
  count += 1;

  CS_SHOW_ERROR;
  return lasterr == 0;
}

bool csPosixMutexRecursiveEmulator::LockTry ()
{
  int rc = pthread_mutex_trylock (&mutex); 

  pthread_t self = pthread_self ();
  if (rc == 0)
  {
    owner = self;
    count = 1;
    return true;
  }
  else if (owner == self)
  {
    count += 1;
    return true;
  }

  lasterr = rc;
  CS_SHOW_ERROR;
  return false;
}

bool csPosixMutexRecursiveEmulator::Release ()
{
  pthread_t self = pthread_self ();
  
  if (owner != self)
  {
    lasterr = EPERM;
    CS_SHOW_ERROR;
    return false;
  }

  count -= 1;
  if (count == 0)
  {
    owner = 0;
    lasterr = pthread_mutex_unlock (&mutex);
  }
  
  CS_SHOW_ERROR;
  return lasterr == 0;
}
#endif

//---------------------------------------------------------------------------

csRef<csSemaphore> csSemaphore::Create (uint32 value)
{
  return csPtr<csSemaphore>(new csPosixSemaphore (value));
}

csPosixSemaphore::csPosixSemaphore (uint32 value)
{
  int rc = sem_init (&sem, 0, (unsigned int)value);
  if (rc)
    lasterr = strerror(errno);
  else
    lasterr = 0;
  CS_SHOW_ERROR;
}

csPosixSemaphore::~csPosixSemaphore ()
{
  Destroy ();
}

bool csPosixSemaphore::LockWait ()
{
  sem_wait (&sem);
  return true;
}

bool csPosixSemaphore::LockTry ()
{
  int rc = sem_trywait (&sem);
  if (rc)
    lasterr = strerror(errno);
  else
    lasterr = 0;
  CS_SHOW_ERROR;
  return rc == 0;
}

bool csPosixSemaphore::Release ()
{
  int rc = sem_post (&sem);
  if (rc)
    lasterr = strerror(errno);
  else
    lasterr = 0;
  CS_SHOW_ERROR;
  return rc == 0;
}

uint32 csPosixSemaphore::Value ()
{
  int val;
  sem_getvalue (&sem, &val);
  return (uint32)val;
}

bool csPosixSemaphore::Destroy ()
{
  int rc = sem_destroy (&sem);
  if (rc)
    lasterr = strerror(errno);
  else
    lasterr = 0;
  CS_SHOW_ERROR;
  return rc == 0;
}

char const* csPosixSemaphore::GetLastError () const
{
  return lasterr;
}


csRef<csCondition> csCondition::Create (uint32 conditionAttributes)
{
  return csPtr<csCondition>(new csPosixCondition (conditionAttributes));
}

csPosixCondition::csPosixCondition (uint32 /*conditionAttributes*/)
{
  pthread_cond_init (&cond, 0);
  lasterr = 0;
}

csPosixCondition::~csPosixCondition ()
{
  Destroy ();
}

void csPosixCondition::Signal (bool WakeAll)
{
  if (WakeAll)
    pthread_cond_broadcast (&cond);
  else
    pthread_cond_signal (&cond);
}

bool csPosixCondition::Wait (csMutex* mutex, csTicks timeout)
{
  int rc = 0;
  if (timeout > 0)
  {
    long const nsec_per_sec = 1000 * 1000 * 1000;
    struct timeval now;
    struct timezone tz;
    struct timespec to;
    gettimeofday (&now, &tz);
    to.tv_sec = now.tv_sec + (timeout / 1000);
    to.tv_nsec = (now.tv_usec + (timeout % 1000) * 1000) * 1000;
    if (to.tv_nsec >= nsec_per_sec) // Catch overflow.
    {
      to.tv_sec += to.tv_nsec / nsec_per_sec;
      to.tv_nsec %= nsec_per_sec;
    }
    rc = pthread_cond_timedwait (&cond, &((csPosixMutex*)mutex)->mutex, &to);
    switch (rc)
    {
    case ETIMEDOUT:
      lasterr = "Timeout";
      return false; 
    case EINTR:
      lasterr = "Wait interrupted";
      break;
    case EINVAL:
      lasterr = "Invalid argument (timeout, mutex, or condition)";
      break;
    case 0:
      lasterr = 0;
      break;
    default:
      lasterr = "Unknown error while timed waiting for condition";
      break;
    }
  }
  else
    pthread_cond_wait (&cond, &((csPosixMutex*)mutex)->mutex);
  CS_SHOW_ERROR;
  return rc == 0;
}

bool csPosixCondition::Destroy ()
{
  int rc = pthread_cond_destroy (&cond);
  switch (rc)
  {
  case EBUSY:
    lasterr = "Condition busy";
    break;
  case 0:
    lasterr = 0;
    break;
  default:
    lasterr = "Unknown error while destroying condition";
    break;
  }
  CS_SHOW_ERROR;
  return rc == 0;
}

char const* csPosixCondition::GetLastError () const
{
  return lasterr;
}


csRef<csThread> csThread::Create (csRunnable* r, uint32 options)
{
  return csPtr<csThread>(new csPosixThread (r, options));
}

csPosixThread::csPosixThread (csRunnable* r, uint32 /*options*/)
{
  runnable = r;
  running = false;
  created = false;
  lasterr = 0;
}

csPosixThread::~csPosixThread ()
{
  if (running)
    Stop ();
//if (created)
//  pthread_join (thread, 0); // clean up resources
}

bool csPosixThread::Start ()
{
  if (!running && runnable)
  {
    if (created)
    {
      pthread_join (thread, 0); // clean up resources
      created = false;
    }
    pthread_attr_t attr;
    int rc;

    // Force thread to be joinable, in later pthread implementations this
    // is default already Thread cancellation state is _assumed_ to be
    // PTHREAD_CANCEL_ENABLE and cancellation type is
    // PTHREAD_CANCEL_DEFERRED
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    rc = pthread_create(&thread, &attr, ThreadRun, (void*)this); 

    switch (rc)
    {
    case EAGAIN:
      lasterr = "Out of system resources.";
      break;
    case EINVAL:
      lasterr = "Tried to create thread with wrong attributes";
      break;
    case EPERM:
      lasterr = "No permission to create thread";
      break;
    case 0:
      lasterr = 0;
      running = true;
      created = true;
      break;
    default:
      lasterr = "Unknown error while creating thread";
      break;
    }
    pthread_attr_destroy(&attr);
  }
  CS_SHOW_ERROR;
  return running;
}

bool csPosixThread::Stop ()
{
  if (running)
  {
    int rc = pthread_cancel (thread);
    switch (rc)
    {
    case ESRCH:
      lasterr = "Trying to stop unknown thread";
      break;
    case 0:
      lasterr = 0;
      running = false;
      break;
    default:
      lasterr = "Unknown error while cancelling thread";
      break;
    }
  }
  CS_SHOW_ERROR;
  return !running;
}

bool csPosixThread::Wait ()
{
  if (running)
  {
    int rc = pthread_join (thread,0);
    switch (rc)
    {
    case ESRCH:
      lasterr = "Trying to wait for unknown thread";
      break;
    case 0:
      lasterr = 0;
      running = false;
      created=false;
      break;
    default:
      //      lasterr = "Unknown error while waiting for thread";
      lasterr = strerror(errno);
      break;
    }
  }
  CS_SHOW_ERROR;
  return !running;
}

void csPosixThread::Yield ()
{
  if (running) sched_yield();
}

char const* csPosixThread::GetLastError () const
{
  return lasterr;
}

void* csPosixThread::ThreadRun (void* param)
{
  csPosixThread* thread = (csPosixThread*)param;
  thread->runnable->Run ();
  thread->running = false;
  pthread_exit (0);
  return 0;
}

#undef CS_SHOW_ERROR
