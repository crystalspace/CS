/*
    Copyright (C) 2003 by Odes B. Boatwright.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_CSBASEEVENTH_H__
#define __CS_CSBASEEVENTH_H__

#include "csextern.h"

/**\file
 * Base implementation of a generic event handler.
 */
/**
 * \addtogroup event_handling
 * @{ */

#include "csutil/scf_implementation.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "csutil/eventhandlers.h"

#include <limits.h>

struct iEventQueue;
struct iObjectRegistry;

// Max event type (in event type enumeration) that will be handled.
#define _CSBASEEVENT_MAXARRAYINDEX csevFrameStart

/**
 * Base implementation of a generic event handler.
 * \par
 * This class provides a base object which does absolutely nothing with the
 * events that are sent to it. In order to properly use, you must derive a
 * class from this one and override the specific \c OnFoo() trigger methods you
 * are interested in processing.
 * \remarks
 * Although this class is derived from iEventHandler, you should not attempt
 * to overload the HandleEvent() method. Always override the specific
 * \c On... trigger function.
 */
class CS_CRYSTALSPACE_EXPORT csBaseEventHandler : public 
  scfImplementation1<csBaseEventHandler, iEventHandler>
{
private:
  csRef<iEventQueue> queue;

protected:
  iObjectRegistry *object_registry;
  csHandlerID self;
  csEventID FrameEvent;
  csEventID PreProcess;
  csEventID Process;
  csEventID PostProcess;
  csEventID FinalProcess;

  /**
   * Constructor.<p>
   * \remarks
   * When deriving a class from csBaseEventHandler, you must call this
   * constructor in order to initialize reference counting for the interface.
   * \par
   * The constructor is declared as protected to prevent a developer from
   * using this class directly.
   */
  csBaseEventHandler ();

public:
  /// Destructor.
  virtual ~csBaseEventHandler ();

  /**
   * Perform basic initialization.  This function MUST be called before
   * invoking any of the RegisterQueue() methods.
   */
  void Initialize (iObjectRegistry *registry);

  /**
   * Register the event handler with the event queue registered with the
   * object registry.
   * \param registry The application's object registry
   * \param name An event name handle.  May be a token from iutil/evdefs.h or
   * the result of a call to csEventNameRegistry::GetID.
   * \see iEventQueue::RegisterListener()
   */
  bool RegisterQueue (iObjectRegistry* registry, csEventID name);

  /**
   * Register the event handler with the event queue registered with the
   * object registry.
   * \param registry The application's object registry
   * \param names An array of event name handles.  Each may be a tokens from 
   * iutil/evdefs.h or the result of a call to csEventNameRegistry::GetID.
   * List must be terminated with CS_EVENTLIST_END.
   * \see iEventQueue::RegisterListener()
   */
  bool RegisterQueue (iObjectRegistry* registry, csEventID names[]);

  /**
   * Register the event handler with an event queue.
   * \param queue The event queue to register with
   * \param name An event name handle.  May be a token from iutil/evdefs.h or
   * the result of a call to csEventNameRegistry::GetID.
   * \see iEventQueue::RegisterListener()
   */
  bool RegisterQueue (iEventQueue* queue, csEventID name);
  /**
   * Register the event handler with an event queue.
   * \param queue The event queue to register with
   * \param names An array of event name handles.  Each may be a tokens from 
   * iutil/evdefs.h or the result of a call to csEventNameRegistry::GetID.
   * List must be terminated with CS_EVENTLIST_END.
   * \see iEventQueue::RegisterListener()
   */
  bool RegisterQueue (iEventQueue* queue, csEventID names[]);

protected:
  /**
   * Implementation of the event handling mechanism. This low-level method
   * examines the event dispatches it to the appropriate OnFoo() or FooFrame()
   * method.
   * \remarks
   * You should almost never need to override this function. Doing so breaks
   * the message-oriented nature of this utility class. In almost all cases,
   * should should simply override the various OnFoo() and FooFrame() methods
   * which are applicable to your situation. Typically, the only valid reason
   * to override this method is when you need to optionally delegate the event
   * handling to some foreign mechanism. (For instance, iAWS::HandleEvent()
   * expects to be handed the event under all circumstances, regardless of the
   * event's type.) If you do override this method in order to optionally
   * delegate event handling to some other entity, then you must remember to
   * invoke csBaseEventHandler::HandleEvent() if the other entity did not
   * handle the event. (Given the AWS example, this means that if
   * iAWS::HandleEvent() returns false, the event should be passed along to
   * csBaseEventHandler::HandleEvent() so that csBaseEventHandler can dispatch
   * it to the appropriate OnFoo() or FooFrame() method as usual.)
   */
  virtual bool HandleEvent (iEvent &event);

  /**
   * Override this if you want to refer to your csBaseEventHandler derived
   * event handler as anything besides "application" for purposes of
   * event subscription scheduling.
   */
  CS_CONST_METHOD virtual const char *GenericName() const 
  { return "application"; }
  
  CS_CONST_METHOD virtual csHandlerID GenericID() const { 
    return csEventHandlerRegistry::GetGenericID(object_registry, GenericName()); 
  }

  /**
   * Override this if you want to force some modules to always handle some 
   * events before csBaseEventHandler.
   */
  CS_CONST_METHOD virtual const csHandlerID *GenericPrec(csEventID) const 
  { return 0; }

  /**
   * Override this if you want to force some modules to always handle some 
   * events after csBaseEventHandler.
   */
  CS_CONST_METHOD virtual const csHandlerID *GenericSucc(csEventID) const 
  { return 0; }

  /**
   * Override this if you want to force some modules to always handle some 
   * events before this instance of csBaseEventHandler.
   */
  CS_CONST_METHOD virtual const csHandlerID *InstancePrec(csEventID) const 
  { return 0; }

  /**
   * Override this if you want to force some modules to always handle some 
   * events before this instance of csBaseEventHandler.
   */
  CS_CONST_METHOD virtual const csHandlerID *InstanceSucc(csEventID) const 
  { return 0; }

  /// Invoked by the event handler when a joystick movement event is received.
  virtual bool OnJoystickMove (iEvent &event);

  /**
   * Invoked by the event handler when a joystick button down event is
   * received.
   */
  virtual bool OnJoystickDown (iEvent &event);

  /// Invoked by the event handler when a joystick button up event is received.
  virtual bool OnJoystickUp (iEvent &event);

  /// Invoked by the event handler when a keyboard event is received.
  virtual bool OnKeyboard (iEvent &event);

  /// Invoked by the event handler when a mouse move event is received.
  virtual bool OnMouseMove (iEvent &event);

  /// Invoked by the event handler when a mouse down event is received.
  virtual bool OnMouseDown (iEvent &event);

  /// Invoked by the event handler when a mouse up event is received.
  virtual bool OnMouseUp (iEvent &event);

  /// Invoked by the event handler when a mouse button click event is received.
  virtual bool OnMouseClick (iEvent &event);

  /**
   * Invoked by the event handler when a mouse button double-click event
   * is received.
   */
  virtual bool OnMouseDoubleClick (iEvent &event);

  /**
   * Invoked by the event handler when an unknown event is received.
   * \remarks
   * Also, this function will be called when a broadcast event was not
   * processed by OnBroadcast().
   */
  virtual bool OnUnhandledEvent (iEvent &event);

  /// Invoked by the handler for the crystalspace.frame event.
  virtual void Frame ();

  /// Invoked by the handler during a pre-process frame broadcast event.
  virtual void PreProcessFrame ();

  /// Invoked by the handler during a process frame broadcast event.
  virtual void ProcessFrame ();

  /// Invoked by the handler during a post-process frame broadcast event.
  virtual void PostProcessFrame ();

  /// Invoked by the handler during a finish frame broadcast event.
  virtual void FinishFrame ();
};

/** @} */

#endif //__CS_CSBASEEVENTH_H__
