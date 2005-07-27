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
 */
/**
 * \addtogroup event_handling
 * @{ */

#include <iutil/event.h>
#include <iutil/eventh.h>

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
class CS_CRYSTALSPACE_EXPORT csBaseEventHandler : public iEventHandler
{
private:
  csRef<iEventQueue> queue;

protected:
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

protected:
  SCF_DECLARE_IBASE;

public:
  /**
   * Register the event handler with the event queue registered with the
   * object registry.
   * \param registry The application's object registry
   * \param trigger Combination of the \c CSMASK_XXX event triggers defined in
   * iutil/evdefs.h. Multiple triggers may be specified by combining them with
   * the bitwise "or" operator (`|').
   * \see iEventQueue::RegisterListener()
   */
  bool RegisterQueue (iObjectRegistry* registry,
  	unsigned int trigger = UINT_MAX);

  /**
   * Register the event handler with an event queue.
   * \param queue The event queue to register with
   * \param trigger Combination of the \c CSMASK_XXX event triggers defined in
   * iutil/evdefs.h. Multiple triggers may be specified by combining them with
   * the bitwise "or" operator (`|').
   * \see iEventQueue::RegisterListener()
   */
  bool RegisterQueue (iEventQueue* queue, unsigned int trigger = UINT_MAX);

private:
  // This contains an array of trigger functions which are used to dispatch
  // events based on event type.
  static bool (csBaseEventHandler::*pmfnTriggers[
  	_CSBASEEVENT_MAXARRAYINDEX+1])(iEvent &event);

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
   * Invoked by the event handler when a broadcast event is received.
   * \remarks
   * Ordinarily a developer should not override this funtion, as the default
   * implementation will call PreProcessFrame(), ProcessFrame(),
   * PostProcessFrame(), or FinishFrame() as appropriate. If you do override
   * this function, either call the base implementation to invoke the
   * aforementioned trigger functions or provide your own implementation.
   * \par
   * If you are expecting to handle a broadcast event which is not one of the
   * standard four used for frame rendering, override OnUnhandledEvent(),
   * which is called by the implementaion of OnBroadcast() for any broadcast
   * event which is not processed by one of the 4 frame rendering trigger
   * functions. If you don't plan on responding to the standard 4 broadcast
   * functions, then overriding OnBroadcast() is appropriate.
   */
  virtual bool OnBroadcast (iEvent &event);

  /// Invoked by the event handler when a command event is received.
  virtual bool OnCommand (iEvent &event);

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
