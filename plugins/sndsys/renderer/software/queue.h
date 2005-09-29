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



#ifndef _QUEUE_H_
#define _QUEUE_H_


#include "csutil/thread.h"


//////////////////////////////////////////////////////////////////////////
// csRef<> is not threadsafe, and csPtr<> doesn't let us do anything with
// the object referenced inside so this class, which is specifically designed
// to communicate between threads, has no choice but to use raw pointers.
// If this is used to communicate between threads, the 'feeder' thread should
// incref the object before passing it into the queue.  The 'eater' thread
// should NOT touch the refcount unless it's certain that no other thread will
// be touching the refcount.  This makes cleanup ... interesting.
// One possible method for cleanup is for the 'eater' thread which implicitely
// holds a single reference (passed from the 'feeder') to wait for the
// refcount to reach 1 before releasing it's refcount, since
// a refcount of 1 means that it should have the only reference and thus
// should be guaranteed to be the only thread working with the refcount.
// Another possibility is for the 'eater' thread to queue this object back to
// the 'feeder' thread, which will perform the decref itself.
//
// Note that if an object passed through this queue is meant to be accessed
// from multiple threads at once it must of course contain threadsafe methods
// itself.
//////////////////////////////////////////////////////////////////////////

#define QUEUE_SUCCESS 0
#define QUEUE_ERR_CLOSED -1
#define QUEUE_ERR_NOMEM  -2
#define QUEUE_ERR_DUPE   -3



template<typename T> class QueueIterator;

template<typename T>
class QEntry
{       
public:
  T * data;
  QEntry *next, *prev;
};    



template<typename T>
class Queue
{     
public:
  Queue() :
      head(0), tail(0), count(0), closed(false), dupecheck(false)
  { 
    // A recursive mutex is used so that the duplicate entry check can hold a
    // lock count of 2
    queue_mutex = csMutex::Create(true);
    queue_condition = csCondition::Create();
  } 

  ~Queue()
  {
    Clear(); 
  }

  void Clear()
  { 
    QEntry<T> *del;
    Lock(); 
    while (head)
    {
      del=head; 
      head=head->next;
      delete del;
    }
    tail=0;

    // Wake all waiting threads, queue is cleared
    queue_condition->Signal(true);
    Unlock();
  }

  int QueueEntry(T * data)
  {
    Lock();

    if (closed) return QUEUE_ERR_CLOSED;

    if (dupecheck && Find(data))
    {
      Unlock();
      return QUEUE_ERR_DUPE;
    }

    QEntry<T> *entry= new QEntry<T>();
    if (!entry)
    {
      Unlock();
      return QUEUE_ERR_NOMEM;
    }
    entry->data=data;
    entry->prev=tail;
    entry->next=0;

    if (!tail)
      head=entry;
    else
      tail->next=entry;
    tail=entry;


    // Signal one waiting thread to wake up
    queue_condition->Signal();

    Unlock();
    return QUEUE_SUCCESS;
  }

  // Note that even if wait==true, this function may return 0
  //  Such a situation is possible if:
  //  1) The Queue is cleared by destruction or a call to Clear()
  //  2) The condition wait is interrupted - possibly by signal arrival
  T * DequeueEntry(bool wait=false)
  {
    QEntry<T> *removed;
    T * ret=0;

    Lock();

    // Wait for an entry to be available if specified
    //  this is ignored if threading support is disabled
    //  since a single threaded application obviously cant
    //  add new entries to the queue while the program is stuck
    //  waiting.
    if (!head && wait)
      queue_condition->Wait(queue_mutex,0);

    // Remove the head entry from the queue
    if (head)
    {
      removed=head;
      head=head->next;
      if (head)
        head->prev=0;
      else
        tail=0;
      ret=removed->data;
      delete removed;
    }
    Unlock();
    return ret;
  }

  size_t Length() { return count; }

  /// Compares pointers, and not the objects they point to
  bool Find(T *data)
  {
    Lock();
    QEntry<T> *cur=head;
    while (cur)
    {
      if (((cur->data)) == (data))
      {
        Unlock();
        return true;
      }
      cur=cur->next;
    }
    Unlock();
    return false;
  }
      

  void SetClosed(bool close)
  {
    Lock();
    closed=close;
    Unlock();
  }

  bool GetClosed()
  {
    bool val;
    Lock();
    val=closed;
    Unlock();
    return val;
  }

  void SetDupecheck(bool check)
  {
    Lock();
    dupecheck=check;
    Unlock();
  }

  bool GetDupecheck()
  {
    bool val;
    Lock();
    val=closed;
    Unlock();
    return val;
  }

  QueueIterator<T> GetIterator()
  {
    return new QueueIterator<T>(this);
  }

protected:
  inline void Lock()
  {
    queue_mutex->LockWait();
  }

  inline void Unlock()
  {
    queue_mutex->Release();
  }

protected:
  QEntry<T> *head, *tail;
  size_t count;
  bool closed;
  bool dupecheck;

  csRef<csMutex> queue_mutex;
  csRef<csCondition> queue_condition;


  friend class QueueIterator<T>;
};

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
    current=q->head;
    return current;
  }

  T * Last()
  {
    current=q->tail;
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




#endif // #ifndef _QUEUE_H_

