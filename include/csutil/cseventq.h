/*
    Crystal Space 3D engine: Event Queue interface
    Copyright (C) 1998 by Andrew Zabolotny <bit@freya.etu.ru>
    Copyright (C) 2001 by Eric Sunshine <sunshine@sunshineco.com>

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

#ifndef __CSEVENTQ_H__
#define __CSEVENTQ_H__

#include "csutil/csevent.h"
#include "csutil/csevcord.h"
#include "csutil/csvector.h"
#include "csutil/evoutlet.h"
#include "csutil/garray.h"
#include "iutil/eventq.h"
struct iObjectRegistry;

// Default event queue size: the queue will automatically grow
// when the queue will overflow.
#define DEF_EVENT_QUEUE_LENGTH  256

/**
 * This class represents a general event queue.  See the documentation of
 * iEventQueue for a detailed description of each method.  One instance of this
 * class is usually shared via iObjectRegistry.
 * <p>
 * The implemented event queue is limited thread-safe.  There are some
 * primitive spinlocks acquired/released in critical sections.
 */
class csEventQueue : public iEventQueue
{
  friend class csEventOutlet;
private:
  struct Listener
  {
    iEventHandler* object;
    unsigned int trigger;
  };
  CS_TYPEDEF_GROWING_ARRAY(ListenerVector, Listener);

  // The array of all allocated event outlets.  *NOTE* It is not the
  // responsibility of this class to free the contained event outlets, thus
  // this class does not override FreeItem().  Instead, it is the
  // responsibility of the caller of iEventQueue::CreateEventOutlet() to send
  // the outlet a DecRef() message (which, incidentally, will result in the
  // automatic removal of the outlet from this list if no references to the
  // outlet remain).
  class EventOutletsVector : public csVector
  {
  public:
    EventOutletsVector() : csVector (16, 16) {}
    virtual ~EventOutletsVector () { DeleteAll(); }
    csEventOutlet* Get(int i)
      { return (csEventOutlet*)csVector::Get(i); }
  };

  // The array of all allocated event cords.
  class EventCordsVector : public csVector
  {
  public:
    EventCordsVector() : csVector (16, 16) {}
    virtual ~EventCordsVector() { DeleteAll(); }
    virtual bool FreeItem(csSome p)
      { ((csEventCord*)p)->DecRef(); return true; }
    csEventCord* Get(int i)
      { return (csEventCord*)csVector::Get(i); }
    int Find(int Category, int SubCategory);
  };

  // Shared-object registry
  iObjectRegistry* Registry;
  // The queue itself
  volatile iEvent** EventQueue;
  // Queue head and tail pointers
  volatile size_t evqHead, evqTail;
  // The maximum queue length
  volatile size_t Length;
  // Protection against multiple threads accessing same event queue
  volatile int SpinLock;
  // Protection against delete while looping through the listeners.
  int busy_looping;
  // If a delete happened while busy_looping is true we set the
  // following to true.
  bool delete_occured;
  // Registered listeners.
  ListenerVector Listeners;
  // Array of allocated event outlets.
  EventOutletsVector EventOutlets;
  // Array of allocated event cords.
  EventCordsVector EventCords;

  // Enlarge the queue size.
  void Resize(size_t iLength);
  // Lock the queue for modifications: NESTED CALLS TO LOCK/UNLOCK NOT ALLOWED!
  inline void Lock() { while (SpinLock) {} SpinLock++; }
  // Unlock the queue
  inline void Unlock() { SpinLock--; }
  // Find a particular listener index; return -1 if listener is not registered.
  int FindListener(iEventHandler*) const;
  // Notify listeners of CSMASK_Nothing.
  void Notify(iEvent&);

  // Start a loop. The purpose of this function is to protect
  // against modifications to the Listeners array while this array
  // is being processed.
  void StartLoop ();
  void EndLoop ();

public:
  SCF_DECLARE_IBASE;

  /// Initialize the event queue
  csEventQueue(iObjectRegistry*, size_t iLength = DEF_EVENT_QUEUE_LENGTH);
  /// Destroy an event queue object
  virtual ~csEventQueue();

  /// Process the event queue.  Calls Dispatch() once for each contained event.
  virtual void Process();
  /// Dispatch a single event from the queue; normally called by Process().
  virtual void Dispatch(iEvent&);

  /// Register a listener for specific events.
  virtual void RegisterListener(iEventHandler*, unsigned int trigger);
  /// Unregister a listener.
  virtual void RemoveListener(iEventHandler*);
  /// Change a listener's trigger.
  virtual void ChangeListenerTrigger(iEventHandler*, unsigned int trigger);

  /// Register an event plug and return a new outlet.
  virtual iEventOutlet* CreateEventOutlet(iEventPlug*);
  /// Get a public event outlet for posting just an event.
  virtual iEventOutlet* GetEventOutlet();
  /// Get the event cord for a given category and subcategory.
  virtual iEventCord* GetEventCord (int Category, int Subcategory);

  /// Place an event into queue.
  virtual void Post(iEvent*);
  /// Get next event from queue or NULL
  virtual iEvent* Get();
  /// Clear event queue
  virtual void Clear();
  /// Query if queue is empty (@@@ Not thread safe!)
  virtual bool IsEmpty() { return evqHead == evqTail; }
};

#endif // __CSEVENTQ_H__
