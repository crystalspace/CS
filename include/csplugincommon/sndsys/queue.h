/*
    Copyright (C) 2004 by Andrew Mann

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


/*! \file
 *  \brief A threadsafe, pointer-passing queue implementation for the sound system.
 *
*/

#ifndef SNDSYS_QUEUE_H
#define SNDSYS_QUEUE_H


#include "csutil/threading/mutex.h"
#include "csutil/threading/condition.h"

namespace CS
{
 namespace SndSys
 {

  /// \brief Basic operation error codes.
  typedef enum
  {
    /// \brief The operation was successful
    QUEUE_SUCCESS    =  0,

    /// \brief The operation failed because the queue is m_bClosed
    ///
    /// Closed queues fail further attempts to add elements
    QUEUE_ERR_CLOSED = -1,

    /// \brief The operation failed due to a memory allocation failure
    QUEUE_ERR_NOMEM  = -2,

    /// \brief The operation failed due to another currently existing
    ///   equivalent entry
    ///
    /// If the queue is configured to check for duplicate entries, then  
    ///  this result may be returned
    QUEUE_ERR_DUPE   = -3
  } QueueErrorType;


  // Advance declaration of class QueueIterator
  template<typename T> class QueueIterator;

  /// 
  template<typename T>
  class QEntry
  {       
  public:
    T * data;
    QEntry *next, *prev;
  };    

  //////////////////////////////////////////////////////////////////////////
  /// \brief A threadsafe, pointer-passing queue (First-In First-Out)
  ///   implementation for the sound system.
  ///
  /// \warning csRef<> is not threadsafe, and csPtr<> doesn't let us do
  ///   anything with the object referenced inside so this class, which is
  ///   specifically designed to communicate between threads, has no choice
  ///   but to use raw pointers.
  ///   If this is used to communicate between threads, the 'feeder' thread
  ///   should incref the object before passing it into the queue.
  ///   The 'consumer' thread should NOT touch the refcount unless it's
  ///   certain that no other thread will be touching the refcount.
  ///   This makes cleanup ... interesting.
  ///   One possible method for cleanup is for the 'consumer' thread which
  ///   implicitly holds a single reference (passed from the 'feeder') to
  ///   wait for the refcount to reach 1 before releasing it's refcount, since
  ///   a refcount of 1 means that it should have the only reference and thus
  ///   should be guaranteed to be the only thread working with the refcount.
  ///   Another possibility is for the 'consumer' thread to queue this object
  ///   back to the 'feeder' thread (through another queue), which will
  ///   perform the decref itself.
  ///
  /// \warning If an object passed through this queue is meant to be accessed
  ///   from multiple threads at once, the object must contain threadsafe
  ///   methods itself.
  //////////////////////////////////////////////////////////////////////////
  template<typename T>
  class Queue
  {     
  public:

    /// Queue construction requires no parameters.
    Queue() :
        m_pHead(0), m_pTail(0), m_EntryCount(0), m_bClosed(false), m_bDupeCheck(false)
    { 
      
    } 

    ~Queue()
    {
      Clear(); 
    }

    /// \brief Clear all entries in the queue.
    ///
    /// \warning This call will NOT delete the underlying object, or release
    ///   any reference counts. To clear the queue in a more controlled
    ///   manner, consider calling Close(true), then fetching each queue entry
    ///   and handling them as appropriate for your use.
    void Clear()
    { 
      QEntry<T> *del;

      CS::Threading::RecursiveMutexScopedLock lock (m_pAccessMutex);
      
      while (m_pHead)
      {
        del=m_pHead; 
        m_pHead=m_pHead->next;
        delete del;
      }
      m_pTail=0;

      // Wake all waiting threads, queue is cleared
      m_pEntryReadyCondition.NotifyAll ();
    }

    /// Add the specified pointer to the end of the queue
    QueueErrorType QueueEntry(T* pData)
    {
      CS::Threading::RecursiveMutexScopedLock lock (m_pAccessMutex);

      if (m_bClosed) return QUEUE_ERR_CLOSED;

      if (m_bDupeCheck && Find(pData))
      {
        return QUEUE_ERR_DUPE;
      }

      QEntry<T> *pNewEntry= new QEntry<T>();
      if (!pNewEntry)
      {
        return QUEUE_ERR_NOMEM;
      }
      pNewEntry->data=pData;
      pNewEntry->prev=m_pTail;
      pNewEntry->next=0;

      if (!m_pTail)
        m_pHead=pNewEntry;
      else
        m_pTail->next=pNewEntry;
      m_pTail=pNewEntry;


      // Signal one waiting thread to wake up
      m_pEntryReadyCondition.NotifyOne ();

      return QUEUE_SUCCESS;
    }

    /// \brief Dequeue an entry from the queue
    ///
    /// This call can optionally wait for an entry to arrive if the bWait
    ///  parameter is specified as true.
    ///
    /// \note Even if bWait is specified as true, this function may return 0
    ///  Such a situation is possible if:
    ///  1) The Queue is cleared by destruction or a call to Clear()
    ///  2) The condition wait is interrupted - possibly by signal arrival
    T * DequeueEntry(bool bWait=false)
    {
      QEntry<T> *pRemoved;
      T* pData=0;

      CS::Threading::RecursiveMutexScopedLock lock (m_pAccessMutex);

      // Wait for an entry to be available if specified
      if (!m_pHead && bWait)
        m_pEntryReadyCondition.Wait (m_pAccessMutex);

      // Remove the m_pHead entry from the queue, shift
      //  the head pointer to the next entry
      if (m_pHead)
      {
        
        pRemoved=m_pHead;
        m_pHead=m_pHead->next;
        // Make sure the Next and Previous linked list pointers
        //  remain valid.
        if (m_pHead)
          m_pHead->prev=0;
        else
          m_pTail=0;
        pData=pRemoved->data;
        // Delete the entry wrapper object
        delete pRemoved;
      }      
      return pData;
    }

    /// Retrieve the number of entries in the queue
    size_t Length() { return m_EntryCount; }

    /// Compares pointers, and not the objects they point to
    bool Find(T *data)
    {
      CS::Threading::RecursiveMutexScopedLock lock (m_pAccessMutex);
      QEntry<T> *cur=m_pHead;
      while (cur)
      {
        if (((cur->data)) == (data))
        {
          return true;
        }
        cur=cur->next;
      }
      return false;
    }

    /// Close the queue so that no further entries may be added
    void SetClosed(bool Closed)
    {
      CS::Threading::RecursiveMutexScopedLock lock (m_pAccessMutex);
      m_bClosed=Closed;      
    }

    /// \brief This can be used to determine if the queue is closed. 
    ///   Closed queues do not allow further entries to be added.
    bool GetClosed()
    {
      bool Closed;
      CS::Threading::RecursiveMutexScopedLock lock (m_pAccessMutex);
      Closed=m_bClosed;
      
      return Closed;
    }

    /// Turn on/off duplicate entry pointer checking.
    ///
    /// This is off by default.
    ///
    /// \warning Turning duplicate entry checking on causes each insert 
    ///   operation to perform a linear search of the queue.
    void SetDupecheck(bool Check)
    {
      CS::Threading::RecursiveMutexScopedLock lock (m_pAccessMutex);
      m_bDupeCheck=Check;
    }

    /// Retrieve the status of duplicate pointer checking.
    bool GetDupecheck()
    {
      bool val;
      CS::Threading::RecursiveMutexScopedLock lock (m_pAccessMutex);
      val=m_bClosed;
      return val;
    }

    /// \brief Retrieve an iterator over this queue.
    ///
    /// \warning The provided iterator holds an exclusive-access lock on
    ///   the entire queue for its lifetime.
    QueueIterator<T>* GetIterator()
    {
      return new QueueIterator<T>(this);
    }

  protected:
    /// Pointer to the oldest entry in the queue
    QEntry<T> *m_pHead;
    /// Pointer to the newest entry in the queue
    QEntry<T> *m_pTail;
    /// Number of entries currently in the queue
    size_t m_EntryCount;
    /// Flag indicating whether new entries may be added to this queue
    volatile bool m_bClosed;
    /// \brief Flag indicating whether the same pointer may exist multiple
    ///   times in this queue.
    volatile bool m_bDupeCheck;

    // A recursive mutex is used so that the duplicate entry check can hold a
    // lock m_EntryCount of 2

    /// The mutex which restricts access to all queue operations.
    CS::Threading::RecursiveMutex m_pAccessMutex;
    /// The condition used for waiting on and signaling availability of entries.
    CS::Threading::Condition m_pEntryReadyCondition;


    friend class QueueIterator<T>;
  };



  /// \brief A simple iterator over the elements of a queue.
  ///
  /// This is not the preferred way to access the elements of the queue.
  ///  See Queue<T>::DequeueEntry() for the proper method.
  template<typename T>
  class QueueIterator
  {
  public:
    QueueIterator(Queue<T> *queue)
    {
      q=queue;
      current=0;
      q->Lock();
    }

    ~QueueIterator()
    {
      q->Unlock();
    }

    T * First()
    {
      current=q->m_pHead;
      return current;
    }

    T * Last()
    {
      current=q->m_pTail;
      return current;
    }

    T *Next()
    {
      if (current) current=current->next;
      return current;
    }


    T * Prev()
    {
      if (current) current=current->prev;
      return current;
    }

  protected:
    QEntry<T> *current;
    Queue<T> *q;
  };



 }
 // END namespace CS::SndSys
}
// END namespace CS



#endif // #ifndef SNDSYS_QUEUE_H

