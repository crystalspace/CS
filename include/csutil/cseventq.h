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

#ifndef __CS_CSEVENTQ_H__
#define __CS_CSEVENTQ_H__

/**\file 
 * Event Queue interface
 */

#include "csextern.h"
#include "csevent.h"
#include "csevcord.h"
#include "evoutlet.h"
#include "array.h"
#include "ref.h"
#include "refarr.h"
#include "thread.h"
#include "iutil/eventq.h"

class csPoolEvent;
struct iObjectRegistry;

/**\internal
 * Default event queue size: the queue will automatically grow
 * when the queue will overflow.
 */
#define DEF_EVENT_QUEUE_LENGTH  256

/**
 * This class represents a general event queue.  See the documentation of
 * iEventQueue for a detailed description of each method.  One instance of this
 * class is usually shared via iObjectRegistry.  Event queues are thread-safe.
 */
class CS_CRYSTALSPACE_EXPORT csEventQueue : public iEventQueue
{
  friend class csEventOutlet;
  friend class csPoolEvent;

private:
  struct CS_CRYSTALSPACE_EXPORT Listener
  {
    iEventHandler* object;
    unsigned int trigger;
  };
  typedef csArray<Listener> ListenerVector;

  size_t EventCordsFind (int cat, int subcat);

  // Shared-object registry
  iObjectRegistry* Registry;
  // The queue itself
  volatile iEvent** EventQueue;
  // Queue head and tail pointers
  volatile size_t evqHead, evqTail;
  // The maximum queue length
  volatile size_t Length;
  // Protection against multiple threads accessing same event queue
  csRef<csMutex> Mutex;
  // Protection against delete while looping through the listeners.
  int busy_looping;
  /*  
   * If a delete happened while busy_looping is true we set the
   * following to true.
   */
  bool delete_occured;
  // Registered listeners.
  ListenerVector Listeners;
  // Array of allocated event outlets.
  csArray<csEventOutlet*> EventOutlets;
  // Array of allocated event cords.
  csRefArray<csEventCord> EventCords;
  // Pool of event objects
  csPoolEvent* EventPool;

  // Enlarge the queue size.
  void Resize (size_t iLength);
  // Lock the queue for modifications: NESTED CALLS TO LOCK/UNLOCK NOT ALLOWED!
  inline void Lock () { Mutex->LockWait(); }
  // Unlock the queue
  inline void Unlock () { Mutex->Release(); }
  // Find a particular listener index; return -1 if listener is not registered.
  size_t FindListener (iEventHandler*) const;
  // Send listeners of CSMASK_FrameProcess one of the frame processing
  // pseudo-events.
  void Notify (unsigned int pseudo_event);

  /*
   * Start a loop. The purpose of this function is to protect
   * against modifications to the Listeners array while this array
   * is being processed.
   */
  void StartLoop ();
  // End a loop.
  void EndLoop ();

public:
  SCF_DECLARE_IBASE;

  /// Initialize the event queue
  csEventQueue (iObjectRegistry*, size_t iLength = DEF_EVENT_QUEUE_LENGTH);
  /// Destroy an event queue object
  virtual ~csEventQueue ();

  /// Process the event queue.  Calls Dispatch () once for each contained event.
  virtual void Process ();
  /// Dispatch a single event from the queue; normally called by Process ().
  virtual void Dispatch (iEvent&);

  /// Register a listener for specific events.
  virtual void RegisterListener (iEventHandler*, unsigned int trigger);
  /// Unregister a listener.
  virtual void RemoveListener (iEventHandler*);
  /**
   * Unregister all listeners.
   * \copydoc iEventQueue::RemoveAllListeners ()
   */
  virtual void RemoveAllListeners ();
  /// Change a listener's trigger.
  virtual void ChangeListenerTrigger (iEventHandler*, unsigned int trigger);

  /// Register an event plug and return a new outlet.
  virtual csPtr<iEventOutlet> CreateEventOutlet (iEventPlug*);
  /// Get a public event outlet for posting just an event.
  virtual iEventOutlet* GetEventOutlet ();
  /// Get the event cord for a given category and subcategory.
  virtual iEventCord* GetEventCord (int Category, int Subcategory);

  /// Get a count of events in the pool, for testing only.
  uint32 CountPool ();
  /// Grab an event from the pool or make a new one if it's empty.
  virtual csPtr<iEvent> CreateEvent (uint8 type);
  /// Place an event into queue.
  virtual void Post (iEvent*);
  /// Get next event from queue or a null references if no event.
  virtual csPtr<iEvent> Get ();
  /// Clear event queue
  virtual void Clear ();
  /// Query if queue is empty (@@@ Not thread safe!)
  virtual bool IsEmpty () { return evqHead == evqTail; }
};

#endif // __CS_CSEVENTQ_H__
