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

#ifndef _CS_THREAD_H_
#define _CS_THREAD_H_

// list of thread initialisation attributes

// list of errorcodes
#define CS_THREAD_NO_ERROR                    0
#define CS_THREAD_UNKNOWN_ERROR               (CS_THREAD_NO_ERROR+1)
#define CS_THREAD_OUT_OF_RESOURCES            (CS_THREAD_UNKNOWN_ERROR+1)
#define CS_THREAD_ERR_ATTRIBUTE               (CS_THREAD_OUT_OF_RESOURCES+1)
#define CS_THREAD_NO_PERMISSION               (CS_THREAD_ERR_ATTRIBUTE+1)
#define CS_THREAD_UNKNOWN_THREAD              (CS_THREAD_NO_PERMISSION+1)
#define CS_THREAD_DEADLOCK                    (CS_THREAD_UNKNOWN_THREAD+1)
#define CS_THREAD_OPERATION_PENDING           (CS_THREAD_DEADLOCK+1)
#define CS_THREAD_MUTEX_NOT_INITIALIZED       (CS_THREAD_OPERATION_PENDING+1)
#define CS_THREAD_MUTEX_BUSY                  (CS_THREAD_MUTEX_NOT_INITIALIZED+1)
#define CS_THREAD_MUTEX_UNKNOWN               (CS_THREAD_MUTEX_BUSY+1)
#define CS_THREAD_CONDITION_TIMEOUT           (CS_THREAD_MUTEX_UNKNOWN+1)
#define CS_THREAD_CONDITION_BUSY              (CS_THREAD_CONDITION_TIMEOUT+1)
#define CS_THREAD_CONDITION_WAIT_INTERRUPTED  (CS_THREAD_CONDITION_BUSY+1)
#define CS_THREAD_SIGNAL_UNKNOWN              (CS_THREAD_CONDITION_WAIT_INTERRUPTED+1)
#define CS_THREAD_SEMA_VALUE_TOO_LARGE        (CS_THREAD_SIGNAL_UNKNOWN+1)
#define CS_THREAD_SEMA_BUSY                   (CS_THREAD_SEMA_VALUE_TOO_LARGE+1)

// list of signals we send to threads

enum csThreadSignal {
  /**
   * We are kindly asking the thread to terminate. 
   * The thread is allowed to ignore this request kindly.
   */
  csThreadSignalTerminate = 1,
  /**
   * We really want the thread to terminate.
   * Thread can no longer ignore this.
   */
  csThreadSignalKill = 2
};

uint32 csThreadInit   (csThread *thread, csThreadFunc func, void* param, uint32 threadAttributes);
void   csThreadExit   (void* exitValue);
uint32 csThreadJoin   (csThread *thread, void** retValue);
uint32 csThreadKill   (csThread *thread, csThreadSignal signal);
uint32 csThreadCancel (csThread *thread);

uint32 csMutexInit     (csMutex* mutex);
uint32 csMutexLockWait (csMutex* mutex);
uint32 csMutexLockTry  (csMutex* mutex);
uint32 csMutexRelease  (csMutex* mutex);
uint32 csMutexDestroy  (csMutex* mutex);

uint32 csSemaphoreInit    (csSemaphore* sem, uint32 value);
uint32 csSemaphoreLock    (csSemaphore* sem);
uint32 csSemaphoreLockTry (csSemaphore* sem);
uint32 csSemaphoreRelease (csSemaphore* sem);
uint32 csSemaphoreValue   (csSemaphore* sem);
uint32 csSemaphoreDestroy (csSemaphore* sem);

uint32 csConditionInit      (csCondition *cond, uint32 conditionAttributes);
uint32 csConditionSignalOne (csCondition *cond);
uint32 csConditionSignalAll (csCondition *cond);
uint32 csConditionWait      (csCondition *cond, csMutex *mutex);
uint32 csConditionWait      (csCondition *cond, csMutex *mutex, csTicks timeout);
uint32 csConditionDestroy   (csCondition *cond);

#endif
