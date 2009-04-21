/* Basic platform-independent macro definitions for mutexes,
   thread-specific data and parameters for malloc.
   Posix threads (pthreads) version.
   Copyright (C) 2004 Wolfram Gloger <wg@malloc.de>.

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that (i) the above copyright notices and this permission
notice appear in all copies of the software and related documentation,
and (ii) the name of Wolfram Gloger may not be used in any advertising
or publicity relating to the software.

THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.

IN NO EVENT SHALL WOLFRAM GLOGER BE LIABLE FOR ANY SPECIAL,
INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY
DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY
OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef _WIN32_MALLOC_MACHINE_H
#define _WIN32_MALLOC_MACHINE_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#undef thread_atfork_static

/* Use fast inline spinlocks with gcc.  */
#if (defined __i386__ || defined __x86_64__) && defined __GNUC__ && \
    !defined USE_NO_SPINLOCKS

#ifndef WIN32
#include <time.h>
#include <sched.h>
#endif

typedef struct {
  volatile unsigned int lock;
  int pad0_;
} mutex_t;

#define MUTEX_INITIALIZER          { 0 }
#define mutex_init(m)              ((m)->lock = 0)
static inline int mutex_lock(mutex_t *m) {
  int cnt = 0, r;
#ifndef WIN32
  struct timespec tm;
#endif

  for(;;) {
    __asm__ __volatile__
      ("xchgl %0, %1"
       : "=r"(r), "=m"(m->lock)
       : "0"(1), "m"(m->lock)
       : "memory");
    if(!r)
      return 0;
#ifdef WIN32
    if(++cnt > 50) {
      SleepEx (0, FALSE);
      cnt = 0;
    }
#else
    if(cnt < 50) {
      sched_yield();
      cnt++;
    } else {
      tm.tv_sec = 0;
      tm.tv_nsec = 2000001;
      nanosleep(&tm, NULL);
      cnt = 0;
    }
#endif
  }
}
static inline int mutex_trylock(mutex_t *m) {
  int r;

  __asm__ __volatile__
    ("xchgl %0, %1"
     : "=r"(r), "=m"(m->lock)
     : "0"(1), "m"(m->lock)
     : "memory");
  return r;
}
static inline int mutex_unlock(mutex_t *m) {
  m->lock = 0;
  __asm __volatile ("" : "=m" (m->lock) : "m" (m->lock));
  return 0;
}

#else

typedef struct {
  volatile LONG lock;
  int pad0_;
} mutex_t;

#if _MSC_VER >= 1400
  #include <intrin.h>
#else
  long _InterlockedExchange (long volatile *, long);
#endif
#pragma intrinsic (_InterlockedExchange)

#ifdef CS_THREAD_CHECKER
#include <libittnotify.h>
#endif

#define MUTEX_INITIALIZER          { 0 }
#define mutex_init(m)              ((m)->lock = 0)
static __inline int mutex_lock(mutex_t *m) {
  int cnt = 0;

  int spins = 0;
#ifdef CS_THREAD_CHECKER
      __itt_notify_sync_prepare((void *)m);
#endif
  for (;;) {
    if (!_InterlockedExchange(&m->lock, 1)) {
#ifdef CS_THREAD_CHECKER
      __itt_notify_sync_acquired((void *)m);
#endif
      return 0;
    }
    if(++cnt > 50) {
      SleepEx (0, FALSE);
      cnt = 0;
    }
  }
}
static __inline int mutex_trylock(mutex_t *m) {
#ifdef CS_THREAD_CHECKER
      int lock;
      __itt_notify_sync_prepare((void *)m);
      lock = _InterlockedExchange (&m->lock, 1);
      if(lock == 0)
        __itt_notify_sync_acquired((void *)m);
      else
        __itt_notify_sync_cancel((void *)m);
      return lock;
#else
  return _InterlockedExchange (&m->lock, 1);
#endif
}
static __inline int mutex_unlock(mutex_t *m) {
#ifdef CS_THREAD_CHECKER
  __itt_notify_sync_releasing((void *)m);
  __itt_thr_mode_set(__itt_thr_prop_quiet, __itt_thr_state_set);
#endif
  m->lock = 0;
#ifdef CS_THREAD_CHECKER
  __itt_thr_mode_set(__itt_thr_prop_quiet, __itt_thr_state_clr);
#endif
  return 0;
}

#endif /* (__i386__ || __x86_64__) && __GNUC__ && !USE_NO_SPINLOCKS */

typedef DWORD tsd_key_t;

#define tsd_key_create(key, destr) *(key) = TlsAlloc()
#define tsd_setspecific(key, data) TlsSetValue(key, data)
#define tsd_getspecific(key, vptr) (vptr = TlsGetValue(key))

/* at fork */
#define thread_atfork(prepare, parent, child) 

static HANDLE sharemem_mapping;

static __inline void* sharemem_init (size_t size, int* created)
{
  size_t neededSize = size + sizeof(HANDLE);
  char* p;
  HANDLE newHandle;

  if (sharemem_mapping == NULL)
  {
    char buf[64];

    sprintf (buf, "ptmalloc-%lu", GetCurrentProcessId());
    sharemem_mapping = OpenFileMappingA (FILE_MAP_ALL_ACCESS, FALSE,
      buf);
    if (sharemem_mapping == NULL)
    {
      newHandle = CreateFileMappingA (INVALID_HANDLE_VALUE, NULL, 
        PAGE_READWRITE, 0, (DWORD)neededSize, buf);
      *created = 1;
      DuplicateHandle (GetCurrentProcess(), newHandle,
        GetCurrentProcess(), &sharemem_mapping, 0, FALSE, 
        DUPLICATE_SAME_ACCESS);
    }
    else
      *created = 0;
    if (sharemem_mapping == NULL) return NULL;
  }
  else
    *created = 0;
  p = MapViewOfFile (sharemem_mapping, FILE_MAP_WRITE | FILE_MAP_READ,
    0, 0, neededSize);
  if (*created)
    *((HANDLE*)p) = newHandle;
  p += sizeof (HANDLE);
  return p;
}

static __inline void sharemem_close (void* p, size_t size)
{
  (void)size;
  UnmapViewOfFile (p);
  CloseHandle (sharemem_mapping);
}

static __inline void sharemem_destroy (void)
{
  char buf[64];
  HANDLE mapping;
  HANDLE* p;

  sprintf (buf, "ptmalloc-%lu", GetCurrentProcessId());
  mapping = OpenFileMappingA (FILE_MAP_ALL_ACCESS, FALSE, buf);
  if (mapping != 0)
  {
    p = (HANDLE*)MapViewOfFile (mapping, FILE_MAP_WRITE | FILE_MAP_READ,
      0, 0, sizeof (HANDLE));
    if (p != 0)
    {
      CloseHandle (*p);
      UnmapViewOfFile (p);
    }
    CloseHandle (mapping);
  }
  sharemem_mapping = 0;
}

#include "../generic/malloc-machine.h"

#endif /* !defined(_WIN32_MALLOC_MACHINE_H) */
