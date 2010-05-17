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

#ifndef __CS_CSUTIL_THREADING_THREAD_H__
#define __CS_CSUTIL_THREADING_THREAD_H__

#include "csutil/noncopyable.h"
#include "csutil/refcount.h"
#include "csutil/refarr.h"

#include "csutil/threading/mutex.h"

namespace CS
{
namespace Threading
{
  /**
   * Priority values indicate how frequently a thread runs compared to other threads.
   * Thread scheduling is handled by the underlying OS, and so the true meaning of these
   * values will vary depending on platform.  A minimal set of values is defined for CS
   * so that chances of support of the full range of values by the platform are greater.
   */
  enum ThreadPriority
  {
    /// Reduced thread priority. Useful for background tasks.
    THREAD_PRIO_LOW = 0,

    /// Normal thread priority.
    THREAD_PRIO_NORMAL = 1,

    /**
     * Increased thread priority. Useful for tasks that needs precedence over
     * all other.
     */
    THREAD_PRIO_HIGH = 2
  };

  /**
   * Abstract base class for objects acting as executor in separate threads.
   * The lifetime of the Runnable object must at least be as long as the thread
   * object.
   */
  class Runnable : public csRefCount, private CS::NonCopyable
  {
  public:
    /**
     * Main method for thread.
     * Will be called as soon as the thread is started and thread will be active
     * until it returns.
     */
    virtual void Run () = 0;

    /**
     * 
     */
    virtual const char* GetName () const
    {
      return "Unnamed thread";
    }
  };

  /// OS specific thread identifier
  typedef uintptr_t ThreadID;

}
}

// Include implementation specific versions
#if defined(CS_PLATFORM_WIN32)
# include "csutil/threading/win32_thread.h"
#elif defined(CS_PLATFORM_UNIX) || \
  defined(CS_PLATFORM_MACOSX)
# include "csutil/threading/pthread_thread.h"
#else
#error "No threading implementation for your platform"
#endif


namespace CS
{
namespace Threading
{
  
  
  /**
   * Object representing a separate execution thread.
   * Used to create, manage and control execution threads.
   */
  class Thread : private CS::NonCopyable, 
    private Implementation::ThreadBase
  {
  public:
    using ThreadBase::IncRef;
    using ThreadBase::DecRef;
    using ThreadBase::GetRefCount;

    /**
     * Initialize a new thread object.
     * \param runnable Runnable object to connect the thread to.
     * \param start If execution in the new thread should start immediately
     * or if you have to start it manually.
     */
    Thread (Runnable* runnable, bool start = false)
      : ThreadBase (runnable)
    {
      if (start)
        Start ();
    }

    /**
     * Initialize a new thread object.
     * \param runnable Runnable object to connect the thread to.
     * \param prio Initial execution priority.
     */
    Thread (Runnable* runnable, ThreadPriority prio)
      : ThreadBase (runnable)
    {
      SetPriority (prio);
    }

    /**
     * Initialize a new thread object.
     * \param runnable Runnable object to connect the thread to.
     * \param prio Initial execution priority.
     * \param start If execution in the new thread should start immediately
     * or if you have to start it manually.
     */
    Thread (Runnable* runnable, bool start, ThreadPriority prio)
      : ThreadBase (runnable)
    {
      SetPriority (prio);

      if (start)
        Start ();
    }
  
    ~Thread ()
    {
      if (IsRunning ())
        Stop ();
    }

    /**
     * Start the thread.
     */
    void Start ()
    {
      ThreadBase::Start ();
    }

    /**
    * Unmercifully stop the thread as soon as possible.
    * This method performs a dirty shutdown of the thread.  The thread is not
    * given a chance to exit normally.  Do not invoke this method unless you
    * have a very good reason for doing so.  In general, it is best to implement
    * some sort of communication with threads so that you can ask them to
    * terminate in an orderly fashion.
    */
    void Stop ()
    {
      ThreadBase::Stop ();
    }

    /**
     * Return whether thread is running or not.
     */
    bool IsRunning () const
    {
      return ThreadBase::IsRunning ();
    }

    /**
     * Set the current execution priority of this thread.  
     * The specifics of when this takes effect and what underlying platform 
     * priority each value maps to are properties of the specific 
     * platform-based implementation.
     * \return true if the priority was successfully set.
     */
    bool SetPriority (ThreadPriority prio)
    {      
      return ThreadBase::SetPriority (prio);          
    }

    /**
     * Get current execution priority for this thread.
     */
    ThreadPriority GetPriority () const
    {
      return ThreadBase::GetPriority ();
    }

    /**
     * Wait for thread to finish its execution.
     * This will block until thread is finished.
     */
    void Wait () const
    {
      ThreadBase::Wait ();
    }

    /**
     * Yield Thread frees CPU time if nothing to do.
     * \remark This Yields execution time in the thread in which this function 
     * is called. For example,  OtherThread->Yield() will NOT have the results 
     * that would be expected.
     */
    static void Yield ()
    {
      ThreadBase::Yield ();
    }

    /**
     * Get an OS specific thread identifier.
     * \remark This gets the thread id of the thread in which this function is called.
     * For example,  OtherThread->GetThreadID() will NOT have the results that would be expected.
     */
    static ThreadID GetThreadID ()
    {
      return ThreadBase::GetThreadID ();
    }
  };


  /**
   * A group of threads handled as one unit.
   */
  class ThreadGroup : private CS::NonCopyable
  {
  public:

    /**
     * Add a thread to the group.
     * A thread will only be added once, even if you run Add two times with
     * same thread object.
     */
    void Add (Thread* thread)
    {
      ScopedLock<Mutex> lock (mutex);
      allThreads.PushSmart (thread);
    }

    /**
     * Remove thread from group.
     */
    void Remove (Thread* thread)
    {
      ScopedLock<Mutex> lock (mutex);
      allThreads.Delete (thread);
    }
    
    /**
     * Get thread with specific index.
     */
    Thread* GetThread (size_t index) const
    {
      ScopedLock<Mutex> lock (mutex);
      return allThreads.Get (index);
    }
    
    /**
     * Get number of threads in group.
     */
    size_t GetSize () const
    {
      return allThreads.GetSize ();
    }

    /**
     * Start all threads in the group.
     * \sa Thread::Start
     */
    void StartAll ()
    {
      ScopedLock<Mutex> lock (mutex);

      for (size_t i = 0; i < allThreads.GetSize (); ++i)
      {
        allThreads[i]->Start ();
      }
    }

    /**
     * Stop all threads in the group.
     * \sa Thread::Stop
     */
    void StopAll ()
    {
      ScopedLock<Mutex> lock (mutex);

      for (size_t i = 0; i < allThreads.GetSize (); ++i)
      {
        allThreads[i]->Stop ();
      }
    }

    /**
     * Wait for all threads in the group.
     * \sa Thread::Wait
     */
    void WaitAll ()
    {
      ScopedLock<Mutex> lock (mutex);
      
      for (size_t i = 0; i < allThreads.GetSize (); ++i)
      {
        allThreads[i]->Wait ();
      }
    }

  private:
    csRefArray<Thread> allThreads;
    mutable Mutex mutex;
  };
}
}



#endif
