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
#include <sys/time.h>

/**
 * A pthread implementation of the CS thread interface.
 */

uint32 csThreadInit (csThread *thread, csThreadFunc func, void* param, uint32 /*threadAttributes*/)
{
   pthread_attr_t attr;
   int rc;
   uint32 ret;

   /* 
      Force thread to be joinable, in later pthread implementations this is default already
      Thread cancellation state is _assumed_ to be PTHREAD_CANCEL_ENABLE and cancellation type is PTHREAD_CANCEL_DEFERRED
   */
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
   rc = pthread_create((pthread_t*)thread, &attr, func, param); 

   switch (rc)
   {
   case EAGAIN:
     ret = CS_THREAD_OUT_OF_RESOURCES;
     break;
   case EINVAL:
     ret = CS_THREAD_ERR_ATTRIBUTE;
     break;
   case EPERM:
     ret = CS_THREAD_NO_PERMISSION;
     break;
   case 0:
     ret = CS_THREAD_NO_ERROR;
     pthread_attr_destroy(&attr);
     break;
   default:
     ret = CS_THREAD_UNKNOWN_ERROR;
     break;
   }

   return ret;
}

void csThreadExit (void* exitValue)
{
  pthread_exit (exitValue);
}

uint32 csThreadJoin (csThread *thread, void** retValue)
{
  int rc = pthread_join (*(pthread_t*)thread, retValue);
  uint32 ret;

  switch (rc)
  {
  case ESRCH:
    ret = CS_THREAD_UNKNOWN_THREAD;
    break;
  case EDEADLK:
    ret = CS_THREAD_DEADLOCK;
    break;
  case 0:
    ret = CS_THREAD_NO_ERROR;
    break;
  default:
    ret = CS_THREAD_UNKNOWN_ERROR;
    break;
  }

  return ret;
}

uint32 csThreadKill (csThread *thread, csThreadSignal signal)
{
  uint32 ret = CS_THREAD_NO_ERROR;
  int sig;

  if (signal == csThreadSignalTerminate)
    sig = SIGTERM;
  else if (signal == csThreadSignalKill)
    sig = SIGKILL;
  else
    ret = CS_THREAD_SIGNAL_UNKNOWN;

  if (ret == CS_THREAD_NO_ERROR)
  {
    int rc;
    rc = pthread_kill (*(pthread_t*)thread, sig);
    switch (rc)
    {
    case ESRCH:
      ret = CS_THREAD_UNKNOWN_THREAD;
      break;
    case EINVAL:
      ret = CS_THREAD_SIGNAL_UNKNOWN;
      break;
    case 0:
      ret = CS_THREAD_NO_ERROR;
      break;
    default:
      ret = CS_THREAD_UNKNOWN_ERROR;
      break;
    }
  }
  return ret;
}

uint32 csThreadCancel (csThread *thread)
{
  int rc = pthread_cancel (*(pthread_t*)thread);
  uint32 ret;

  switch (rc)
  {
  case ESRCH:
    ret = CS_THREAD_UNKNOWN_THREAD;
    break;
  case 0:
    ret = CS_THREAD_NO_ERROR;
    break;
  default:
    ret = CS_THREAD_UNKNOWN_ERROR;
    break;
  }
  return ret;
}

uint32 csMutexInit (csMutex* mutex)
{
  /*
    Create an 'fast' mutex, that is a deadlock occurs if a thread 
    tries to LockWait a mutex it already owns.
  */

  pthread_mutex_init ((pthread_mutex_t*)mutex, NULL);
  return CS_THREAD_NO_ERROR;
}

uint32 csMutexLockWait (csMutex* mutex)
{
  uint32 ret;
  int rc = pthread_mutex_lock ((pthread_mutex_t*)mutex);
  switch (rc)
  {
  case EINVAL:
    ret = CS_THREAD_MUTEX_NOT_INITIALIZED;
    break;
  case EDEADLK:
    ret = CS_THREAD_DEADLOCK;
    break;
  case 0:
    ret = CS_THREAD_NO_ERROR;
    break;
  default:
    ret = CS_THREAD_UNKNOWN_ERROR;
    break;
  }
  return ret;
}

uint32 csMutexLockTry (csMutex* mutex)
{
  uint32 ret;
  int rc = pthread_mutex_trylock ((pthread_mutex_t*)mutex);
  switch (rc)
  {
  case EINVAL:
    ret = CS_THREAD_MUTEX_NOT_INITIALIZED;
    break;
  case EBUSY:
    ret = CS_THREAD_MUTEX_BUSY;
    break;
  case 0:
    ret = CS_THREAD_NO_ERROR;
    break;
  default:
    ret = CS_THREAD_UNKNOWN_ERROR;
    break;
  }
  return ret;
}

uint32 csMutexRelease (csMutex* mutex)
{
  uint32 ret;
  int rc = pthread_mutex_unlock ((pthread_mutex_t*)mutex);
  switch (rc)
  {
  case EINVAL:
    ret = CS_THREAD_MUTEX_NOT_INITIALIZED;
    break;
  case EPERM:
    ret = CS_THREAD_NO_PERMISSION;
    break;
  case 0:
    ret = CS_THREAD_NO_ERROR;
    break;
  default:
    ret = CS_THREAD_UNKNOWN_ERROR;
    break;
  }
  return ret;
}

uint32 csMutexDestroy (csMutex* mutex)
{
  uint32 ret;
  int rc = pthread_mutex_destroy ((pthread_mutex_t*)mutex);
  switch (rc)
  {
  case EBUSY:
    ret = CS_THREAD_MUTEX_BUSY;
    break;
  case 0:
    ret = CS_THREAD_NO_ERROR;
    break;
  default:
    ret = CS_THREAD_UNKNOWN_ERROR;
    break;
  }
  return ret;
}

uint32 csSemaphoreInit (csSemaphore* sem, uint32 value)
{
  int rc = sem_init ((sem_t*)sem, 0, (unsigned int)value);
  uint32 ret;
  if (rc)
  {
    if (errno == EINVAL)
      ret = CS_THREAD_SEMA_VALUE_TOO_LARGE;
  }
  else
    ret = CS_THREAD_NO_ERROR;
  return ret;
}

uint32 csSemaphoreLock (csSemaphore* sem)
{
  sem_wait ((sem_t*)sem);
  return CS_THREAD_NO_ERROR;
}

uint32 csSemaphoreLockTry (csSemaphore* sem)
{
  int rc = sem_trywait ((sem_t*)sem);
  uint32 ret;
  if (rc)
    switch (errno)
    {
    case EAGAIN:
      ret = CS_THREAD_SEMA_BUSY;
      break;
    default:
      ret = CS_THREAD_UNKNOWN_ERROR;
      break;
    }
  else
    ret = CS_THREAD_NO_ERROR;

  return ret;
}

uint32 csSemaphoreRelease (csSemaphore* sem)
{
  int rc = sem_post ((sem_t*)sem);
  uint32 ret;
  if (rc)
    ret = CS_THREAD_UNKNOWN_ERROR;
  else
    ret = CS_THREAD_NO_ERROR;
  return ret;
}

uint32 csSemaphoreValue (csSemaphore* sem)
{
  int val;
  sem_getvalue ((sem_t*)sem, &val);
  return (uint32)val;
}

uint32 csSemaphoreDestroy (csSemaphore* sem)
{
  int rc = sem_destroy ((sem_t*)sem);
  uint32 ret;

  if (rc)
    switch (errno)
    {
    case EBUSY:
      ret = CS_THREAD_SEMA_BUSY;
      break;
    default:
      ret = CS_THREAD_UNKNOWN_ERROR;
      break;
    }
  else
    ret = CS_THREAD_NO_ERROR;

  return ret;
}

uint32 csConditionInit (csCondition *cond, uint32 /*conditionAttributes*/)
{
  pthread_cond_init ((pthread_cond_t*)cond, NULL);
  return CS_THREAD_NO_ERROR;
}

uint32 csConditionSignalOne (csCondition *cond)
{
  pthread_cond_signal ((pthread_cond_t*)cond);
  return CS_THREAD_NO_ERROR;
}

uint32 csConditionSignalAll (csCondition *cond)
{
  pthread_cond_broadcast ((pthread_cond_t*)cond);
  return CS_THREAD_NO_ERROR;
}

uint32 csConditionWait (csCondition *cond, csMutex *mutex)
{
  pthread_cond_wait ((pthread_cond_t*)cond, (pthread_mutex_t*)mutex);
  return CS_THREAD_NO_ERROR;
}

uint32 csConditionWait (csCondition *cond, csMutex *mutex, csTicks timeout)
{
  uint32 ret;
  struct timeval now;
  struct timezone tz;
  struct timespec to;
  gettimeofday (&now, &tz);
  to.tv_sec = now.tv_sec + (timeout / 1000);
  to.tv_nsec = (now.tv_usec + (timeout % 1000)*1000)*1000;
  int rc = pthread_cond_timedwait ((pthread_cond_t*)cond, (pthread_mutex_t*)mutex, &to);
  switch (rc)
  {
  case ETIMEDOUT:
    ret = CS_THREAD_CONDITION_TIMEOUT;
    break;
  case EINTR:
    ret = CS_THREAD_CONDITION_WAIT_INTERRUPTED;
    break;
  case 0:
    ret = CS_THREAD_NO_ERROR;
    break;
  default:
    ret = CS_THREAD_UNKNOWN_ERROR;
    break;
  }
  return ret;
}

uint32 csConditionDestroy (csCondition *cond)
{
  int rc = pthread_cond_destroy ((pthread_cond_t*)cond);
  uint32 ret;
  switch (rc)
  {
  case EBUSY:
    ret = CS_THREAD_CONDITION_BUSY;
    break;
  case 0:
    ret = CS_THREAD_NO_ERROR;
    break;
  default:
    ret = CS_THREAD_UNKNOWN_ERROR;
    break;
  }
  return ret;
}

