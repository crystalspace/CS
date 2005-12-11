/*
    Crystal Space 3D engine: Event Queue interface
    Copyright (C) 1998 by Andrew Zabolotny <bit@freya.etu.ru>
    Copyright (C) 2001 by Eric Sunshine <sunshine@sunshineco.com>
    Copyright (C) 2005 by Adam D. Bradley <artdodge@cs.bu.edu>

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

#ifdef ADB_DEBUG /* debugging output... */
#include <iostream>
#endif

/**\file 
 * Event Queue interface
 */

#include "csextern.h"

#include "csutil/array.h"
#include "csutil/hash.h"
#include "csutil/list.h"
#include "csutil/ref.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"
#include "csutil/thread.h"
#include "csutil/weakref.h"
#include "iutil/eventnames.h"
#include "iutil/eventhandlers.h"
#include "iutil/eventq.h"
#include "cssubscription.h"

struct iObjectRegistry;

class csEventCord;
class csEventOutlet;
class csPoolEvent;

/**\internal
 * Default event queue size: the queue will automatically grow
 * when the queue will overflow.
 */
#define DEF_EVENT_QUEUE_LENGTH  256

CS_SPECIALIZE_TEMPLATE
class csHashComputer<iEventHandler *> : public csHashComputerIntegral<iEventHandler *> {};

/**
 * This class represents a general event queue.  See the documentation of
 * iEventQueue for a detailed description of each method.  One instance of this
 * class is usually shared via iObjectRegistry.  Event queues are thread-safe.
 */
class CS_CRYSTALSPACE_EXPORT csEventQueue : 
  public scfImplementation1<csEventQueue, iEventQueue>
{
  friend class csEventOutlet;
  friend class csPoolEvent;
  friend class csEventTree;

private:
  // Shared-object registry
  iObjectRegistry* Registry;
  // Event name registry
  csRef<iEventNameRegistry> NameRegistry;
  // Event handler registry
  csRef<iEventHandlerRegistry> HandlerRegistry;
  // The queue itself (technically, a ring buffer)
  volatile iEvent** EventQueue;
  // Queue head and tail pointers
  volatile size_t evqHead, evqTail;
  // The maximum queue length
  volatile size_t Length;
  // Protection against multiple threads accessing same event queue
  csRef<csMutex> Mutex;
  // Event tree.  All subscription PO graphs and delivery queues hang off of this.
  csEventTree *EventTree;
  // Shortcut to per-event-name delivery queues.
  csHash<csEventTree *,csEventID> EventHash;
  // Array of allocated event outlets.
  csArray<csEventOutlet*> EventOutlets;
  // Array of allocated event cords.
  csHash<csEventCord *, csEventID> EventCords;
  // Pool of event objects
  csPoolEvent* EventPool;

  // Enlarge the queue size.
  void Resize (size_t iLength);
  // Lock the queue for modifications: NESTED CALLS TO LOCK/UNLOCK NOT ALLOWED!
  inline void Lock () { Mutex->LockWait(); }
  // Unlock the queue
  inline void Unlock () { Mutex->Release(); }
  // Send broadcast pseudo-events (bypassing event queue).
  void Notify (const csEventID &name);

  /*
   * Start a loop. The purpose of this function is to protect
   * against modifications to the Listeners array while this array
   * is being processed.
   */
  void StartLoop ();
  // End a loop.
  void EndLoop ();

public:

  /// Initialize the event queue
  csEventQueue (iObjectRegistry*, size_t iLength = DEF_EVENT_QUEUE_LENGTH);
  /// Destroy an event queue object
  virtual ~csEventQueue ();

  /// Process the event queue.  Calls Dispatch () once for each contained event.
  virtual void Process ();
  /// Dispatch a single event from the queue; normally called by Process ().
  virtual void Dispatch (iEvent&);

  /// Make the event scheduler subsystem aware of an event handler
  virtual csHandlerID RegisterListener (iEventHandler*);
  /// Shorthand for RegisterListener() followed by Subscribe()
  virtual csHandlerID RegisterListener (iEventHandler *handler, const csEventID &event) 
  { 
	  csHandlerID id = RegisterListener(handler);
	  if (id!=CS_HANDLER_INVALID)
	  {
		  if (Subscribe(handler, event))
			  return id;
		  else
			  RemoveListener(handler); /* fall through */
	  }
	  return CS_HANDLER_INVALID;
  }
  /// Shorthand for RegisterListener() followed by Subscribe()
  virtual csHandlerID RegisterListener (iEventHandler *handler, const csEventID events[]) 
  { 
	  csHandlerID id = RegisterListener(handler);
	  if (id!=CS_HANDLER_INVALID)
	  {
		  if (Subscribe(handler, events))
			  return id;
		  else
			  RemoveListener(handler); /* fall through */
	  }
	  return CS_HANDLER_INVALID;
  }
  /**
   * Subscribe a listener to an event subtree.
   * Thinly wraps csEventTree::Subscribe
   * \sa csEventTree::Subscribe
   */
  virtual bool Subscribe (iEventHandler*, const csEventID &);
  /**
   * Subscribe a listener to a set of event subtrees.
   * The event subtrees should be disjoint, i.e., no event name
   * should be a prefix of another.
   * \sa csEventTree::Subscribe
   */
  virtual bool Subscribe (iEventHandler*, const csEventID[]);
  /**
   * Unsubscribe a listener from an event subtree.
   * Thinly wraps csEventTree::Unsubscribe
   * \sa csEventTree::Unsubscribe
   */
  virtual void Unsubscribe (iEventHandler*, const csEventID &);
  /**
   * Unsubscribe a listener from a set of event subtrees.
   * \sa csEventTree::Unsubscribe
   */
  virtual void Unsubscribe (iEventHandler*, const csEventID[]);
  /**
   * Remove a given listener from the event queue.  
   * Removes all subscriptions.
   * It is <b>VERY</b> important that this be called before deleting the
   * event handler!
   */
  virtual void RemoveListener (iEventHandler*);
  /**
   * Remove all listeners from the queue.
   * \copydoc iEventQueue::RemoveAllListeners ()
   */
  virtual void RemoveAllListeners ();

  /// Register an event plug and return a new outlet.
  virtual csPtr<iEventOutlet> CreateEventOutlet (iEventPlug*);
  /// Get a public event outlet for posting just an event.
  virtual iEventOutlet* GetEventOutlet ();
  /// Get the event cord for a given category and subcategory.
  virtual iEventCord* GetEventCord (const csEventID &);

  /// Get a count of events in the pool, for testing only.
  uint32 CountPool ();
protected:
  virtual iEvent *CreateRawEvent ();
public:
  /// Grab an event from the pool or make a new one if it's empty.
  virtual csPtr<iEvent> CreateEvent ();
  virtual csPtr<iEvent> CreateEvent (const csEventID &name, bool broadcast);
  virtual csPtr<iEvent> CreateEvent (const csEventID &name);
  virtual csPtr<iEvent> CreateEvent (const char *name);
  virtual csPtr<iEvent> CreateBroadcastEvent (const csEventID &name);
  virtual csPtr<iEvent> CreateBroadcastEvent (const char *name);
  /// Place an event into queue.
  virtual void Post (iEvent*);
  /// Get next event from queue or a null references if no event.
  virtual csPtr<iEvent> Get ();
  /// Clear event queue
  virtual void Clear ();
  /// Query if queue is empty (@@@ Not thread safe!)
  virtual bool IsEmpty () { return evqHead == evqTail; }

  csEventID Frame;
  csEventID PreProcess;
  csEventID ProcessEvent;
  csEventID PostProcess;
  csEventID FinalProcess;

  /**
   * As a transitional measure, the csevPreProcess, csevProcess,
   * csevPostProcess and csevFinalProcess events are actually sub-events
   * dispatched by a csevFrame handler.  Each of the TypedFrameEventDispatcher
   * child classes receives the csevFrame event in order and dispatches
   * its name event.  Event handlers should either subscribe to these
   * events or subscribe to csevFrame.  They may wish to use these
   * event handlers as "milestones" relative to which they define their
   * own order (e.g., "I can process the Frame event at any time between
   * the handling of the Process event and the handling of the FinalProcess 
   * event").
   */
  struct iTypedFrameEventDispatcher : public iEventHandler 
  {
  protected:
    csWeakRef<csEventQueue> parent;
    csEventID sendEvent;
  public:
    iTypedFrameEventDispatcher () 
    {
    }
    virtual ~iTypedFrameEventDispatcher ()
    {
    }
    CS_EVENTHANDLER_DEFAULT_INSTANCE_CONSTRAINTS
    virtual bool HandleEvent (iEvent& e) 
    { 
      parent->Notify (sendEvent);
      return false;
    }
  };

  class PreProcessFrameEventDispatcher 
    : public scfImplementation2<PreProcessFrameEventDispatcher, 
                                csEventQueue::iTypedFrameEventDispatcher, 
                                scfFakeInterface<iEventHandler> > 
  {
  public:
    PreProcessFrameEventDispatcher (csEventQueue* parent) 
      : scfImplementationType (this)
    {
      iTypedFrameEventDispatcher::parent = parent;
      sendEvent = parent->PreProcess;
    }
    CS_EVENTHANDLER_NAMES("crystalspace.frame.preprocess")
    CS_CONST_METHOD virtual const csHandlerID * GenericPrec(csRef<iEventHandlerRegistry> &r1,
							    csRef<iEventNameRegistry> &r2,
							    csEventID e) const {
      return 0;
    }
    CS_CONST_METHOD virtual const csHandlerID * GenericSucc(csRef<iEventHandlerRegistry> &r1,
							    csRef<iEventNameRegistry> &r2,
							    csEventID e) const 
    {
      static csHandlerID constraint[2] = { 0, CS_HANDLERLIST_END };
      if (e == csevFrame(r2))
      {
	constraint[0] = ProcessFrameEventDispatcher::StaticID (r1);
	return constraint;
      }
      return 0;
    }
  };
  
  class ProcessFrameEventDispatcher 
    : public scfImplementation2<ProcessFrameEventDispatcher, 
                                csEventQueue::iTypedFrameEventDispatcher, 
                                scfFakeInterface<iEventHandler> > 
  {
  public:
    ProcessFrameEventDispatcher (csEventQueue* parent) 
      : scfImplementationType (this)
    {
      iTypedFrameEventDispatcher::parent = parent;
      sendEvent = parent->ProcessEvent;
    }
    CS_EVENTHANDLER_NAMES("crystalspace.frame.process")
    CS_CONST_METHOD virtual const csHandlerID * GenericPrec(csRef<iEventHandlerRegistry> &r1,
							    csRef<iEventNameRegistry> &r2,
							    csEventID e) const {
      return 0;
    }
    CS_CONST_METHOD virtual const csHandlerID * GenericSucc(csRef<iEventHandlerRegistry> &r1,
							    csRef<iEventNameRegistry> &r2,
							    csEventID e) const 
    {
      static csHandlerID constraint[2] = { 0, CS_HANDLERLIST_END };
      if (e == csevFrame(r2))
      {
	constraint[0] = PostProcessFrameEventDispatcher::StaticID (r1);
	return constraint;
      }
      return 0;
    }
  };

  class PostProcessFrameEventDispatcher 
    : public scfImplementation2<PostProcessFrameEventDispatcher, 
                                csEventQueue::iTypedFrameEventDispatcher, 
                                scfFakeInterface<iEventHandler> > 
  {
  public:
    PostProcessFrameEventDispatcher (csEventQueue* parent) 
      : scfImplementationType (this)
    {
      iTypedFrameEventDispatcher::parent = parent;
      sendEvent = parent->PostProcess;
    }
    CS_EVENTHANDLER_NAMES("crystalspace.frame.postprocess")
    CS_CONST_METHOD virtual const csHandlerID * GenericPrec(csRef<iEventHandlerRegistry> &r1,
							    csRef<iEventNameRegistry> &r2,
							    csEventID e) const 
    {
      return 0;
    }
    CS_CONST_METHOD virtual const csHandlerID * GenericSucc(csRef<iEventHandlerRegistry> &r1,
							    csRef<iEventNameRegistry> &r2,
							    csEventID e) const 
    {
      static csHandlerID constraint[2] = { 0, CS_HANDLERLIST_END };
      if (e == csevFrame(r2))
      {
	constraint[0] = FinalProcessFrameEventDispatcher::StaticID (r1);
	return constraint;
      }
      return 0;
    }
  };
  
  class FinalProcessFrameEventDispatcher 
    : public scfImplementation2<FinalProcessFrameEventDispatcher, 
                                csEventQueue::iTypedFrameEventDispatcher, 
                                scfFakeInterface<iEventHandler> > 
  {
  public:
    FinalProcessFrameEventDispatcher (csEventQueue* parent) 
      : scfImplementationType (this)
    {
      iTypedFrameEventDispatcher::parent = parent;
      sendEvent = parent->FinalProcess;
    }
    CS_EVENTHANDLER_NAMES("crystalspace.frame.finalprocess")
    CS_CONST_METHOD virtual const csHandlerID * GenericPrec(csRef<iEventHandlerRegistry> &r1,
							    csRef<iEventNameRegistry> &r2,
							    csEventID e) const {
      return 0;
    }
    CS_CONST_METHOD virtual const csHandlerID * GenericSucc(csRef<iEventHandlerRegistry> &r1,
							    csRef<iEventNameRegistry> &r2,
							    csEventID e) const {
      return 0;
    }
  };
};

#endif // __CS_CSEVENTQ_H__
