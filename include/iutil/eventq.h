/*
    Copyright (C) 2001 by Eric Sunshine <sunshine@sunshineco.com>
    Copyright (C) 2000 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CS_IUTIL_EVENTQ_H__
#define __CS_IUTIL_EVENTQ_H__

/**\file
 * General event queue
 */
/**
 * \addtogroup event_handling
 * @{ */
 
#include "csutil/scf.h"
struct iEvent;
struct iEventCord;
struct iEventOutlet;
struct iEventPlug;
struct iEventHandler;

SCF_VERSION(iEventQueue, 0, 0, 1);

/**
 * This interface represents a general event queue.
 *
 * Events may be posted to the queue by various sources.  Listeners
 * (implementing iEventHandler) can register to receive notification when
 * various events are processed.  Typically, one instance of this object is
 * available from the shared-object registry (iObjectRegistry).
 *
 * Main creators of instances implementing this interface:
 * - csInitializer::CreateEnvironment()
 * - csInitializer::CreateEventQueue()
 * 
 * Main ways to get pointers to this interface:
 * - csQueryRegistry()
 */
struct iEventQueue : public iBase
{
  /**
   * Process the event queue.  Calls Dispatch() once for each event in the
   * queue in order to actually dispatch the event.  Typically, this method is
   * invoked by the host application on a periodic basis (often from the host's
   * own event loop) in order to give Crystal Space modules a chance to run and
   * respond to events.
   */
  virtual void Process () = 0;

  /**
   * Dispatch a single event from the queue.  This is normally called by
   * Process() once for each event in the queue.  Events are dispatched to the
   * appropriate listeners (implementors of iEventHandler) which have been
   * registered via RegisterListener().
   */
  virtual void Dispatch (iEvent&) = 0;

  /**
   * Register a listener for specific events.  If the listener is already
   * registered, then this method merely changes the listener's `trigger'. The
   * `trigger' argument is a combination of the CSMASK_XXX event triggers
   * defined in iutil/evdefs.h.  Multiple triggers may be specified by
   * combining them with the bitwise "or" operator (`|').  The CSMASK_Nothing
   * event trigger is special.  If registered with this trigger, the listener
   * will be called in just before Process() iterates over the event queue, 
   * and then it will be called with 3 different csevBroadcast events after 
   * has been dispatched the last event. In this case, the listener will be 
   * sent an csevBroadcast event with the Event.Retrieve("cmdCode") equal to 
   * cscmdPreProcess before events dispatching, and after events dispatching
   * the listener will receive three csevBroadcast events, in 3 successive 
   * phases, following this ordering: the first broadcasted event has the 
   * Event.Retrieve("cmdCode") equal to cscmdProcess, then the second broadcasted 
   * event has a value of cscmdPostProcess and finally the last event 
   * broadcasted has the cscmdFinalProcess value .
   */
  virtual void RegisterListener (iEventHandler*, unsigned int trigger) = 0;

  /**
   * Unregister a listener. It is important to call RemoveListener() before
   * deleting your event handler!
   */
  virtual void RemoveListener (iEventHandler*) = 0;

  /**
   * Change a listener's trigger.  See RegisterListener() for a discussion of
   * the trigger.
   */
  virtual void ChangeListenerTrigger (iEventHandler*, unsigned int trigger) = 0;

  /**
   * Register an event plug and return a new outlet.  Any module which
   * generates events should consider using this interface for posting those
   * events to the queue.  The module should implement the iEventPlug interface
   * and register that interface with this method.  In return, an iEventOutlet
   * object will be created which can be used to actually post events to the
   * queue.  It is the caller's responsibility to send a DecRef() message to
   * the returned event outlet when it is no longer needed.
   */
  virtual csPtr<iEventOutlet> CreateEventOutlet (iEventPlug*) = 0;

  /**
   * Get a public event outlet for posting just an event.
   *<p>
   * In general most modules should create their own private outlet via
   * CreateEventOutlet() and register as a normal event plug.  However, there
   * are cases when you just need to post one event from time to time; in these
   * cases it is easier to post it without the bulk of creating a new
   * iEventPlug interface.  In these cases, you can post the event by obtaining
   * the shared event outlet from GetEventOutlet(), and use it to post an event
   * instead.
   */
  virtual iEventOutlet* GetEventOutlet () = 0;

  /**
   * Get the event cord for a given category and subcategory.
   * <p>
   * This allows events to be delivered immediately, bypassing the normal event
   * queue, to a chain of plugins that register with the implementation of
   * iEventCord returned by this function.  The category and subcategory are
   * matched against the category and subcategory of each actual iEvent.
   */
  virtual iEventCord* GetEventCord (int Category, int Subcategory) = 0;

  /**
   * Create an event, from the pool if there are any free events available. Else
   * create a new event in the pool and use it.
   */
  virtual csPtr<iEvent> CreateEvent(uint8 type) = 0;

  /**
   * Place an event into queue.  In general, clients should post events to the
   * queue via an iEventOutlet rather than directly via Post(), however there
   * may be certain circumanstances where posting directly to the queue is
   * preferred.
   * \remark Post() takes ownership of the event. That means that you MUST NOT
   * DecRef() the event after passing it to Post(). Instead, if you want to 
   * keep it, IncRef() it.
   */
  virtual void Post (iEvent*) = 0;

  /**
   * Get next event from queue; returns a null reference if no events are
   * present.  There is rarely any need to manually retrieve events from the
   * queue.  Instead, normal event processing via Process() takes care of this
   * responsibility.  iEventQueue gives up ownership of the returned iEvent.
   */
  virtual csPtr<iEvent> Get () = 0;

  /// Clear event queue.
  virtual void Clear () = 0;

  /// Query if queue is empty.
  virtual bool IsEmpty () = 0;

  /**
   * Unregister all listeners.
   * \remarks
   * This function is used to clear all listeners from the event queue stack.
   * You should only call this function at the end of your program after the
   * event queue processing has terminated.
   * \par
   * If you make use of csInitializer::DestroyApplication(), this is done for
   * you by that call.
   */
  virtual void RemoveAllListeners () = 0;

};

/** @} */

#endif // __CS_IUTIL_EVENTQ_H__
