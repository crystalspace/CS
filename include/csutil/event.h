/*
    Event system related helpers
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter
              (C) 2005 by Adam D. Bradley <artdodge@cs.bu.edu>

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

#ifndef __CS_CSUTIL_EVENT_H__
#define __CS_CSUTIL_EVENT_H__

#include "csextern.h"
#include "iutil/event.h"
#include "iutil/eventhandlers.h"

/**\file
 * Event system related helpers
 */
/**
 * \addtogroup event_handling
 * @{ */

/**\name Keyboard event related
 * @{ */
/**
 * Helper class to conveniently deal with keyboard events.
 */
class CS_CRYSTALSPACE_EXPORT csKeyEventHelper
{
public:
  /// Retrieve the key's raw code.
  CS_PURE_METHOD static utf32_char GetRawCode (const iEvent* event);
  /// Retrieve the key's cooked code.
  CS_PURE_METHOD static utf32_char GetCookedCode (const iEvent* event);
  /// Retrieve the key's raw code.
  static void GetModifiers (const iEvent* event, csKeyModifiers& modifiers);
  /// Retrieve the event type (key up or down.)
  CS_PURE_METHOD static csKeyEventType GetEventType (const iEvent* event);
  /**
   * Retrieve whether a keyboard down event was caused by the initial press
   * (not auto-repeat) or by having it held for a period of time (auto-repeat.)
   */
  CS_PURE_METHOD static bool GetAutoRepeat (const iEvent* event);
  /// Retrieve the character type (dead or normal.)
  CS_PURE_METHOD static csKeyCharType GetCharacterType (const iEvent* event);
  /// Get all the information in one compact struct.
  static bool GetEventData (const iEvent* event, csKeyEventData& data);
  /**
   * Get a bitmask corresponding to the pressed modifier keys from the
   * keyboard modifiers struct.
   * \sa CSMASK_ALT etc.
   */
  CS_CONST_METHOD static uint32 GetModifiersBits (
    const csKeyModifiers& modifiers);
  /**
   * Get a bitmask corresponding to the pressed modifier keys from the event.
   * \sa CSMASK_ALT etc.
   */
  CS_PURE_METHOD static uint32 GetModifiersBits (const iEvent* event);
  /**
   * Convert a bitmask returned by GetModifiersBits back to a csKeyModifiers
   * struct.
   * <p>
   * Also works for the Modifiers members of the csMouseEventData and
   * csJoystickEventData structs, if you cast them to uint32.
   */
  static void GetModifiers (uint32 mask, csKeyModifiers& modifiers);
};
/** @} */

/* forward declaration */
class csEvent;

/**\name Mouse event related
 * @{ */
/**
 * Helper class to conveniently deal with mouse events.
 */
class CS_CRYSTALSPACE_EXPORT csMouseEventHelper
{
public:
  //@{
  /// Create a new mouse event
  static csEvent *NewEvent (csRef<iEventNameRegistry> &reg, 
    csTicks, csEventID name, csMouseEventType etype, int x, int y, 
    uint32 AxesChanged, uint button, bool buttonstate, uint32 buttonMask, 
    uint32 modifiers);
  static csEvent *NewEvent (csRef<iEventNameRegistry> &reg, csTicks, 
    csEventID name, uint8 n, csMouseEventType etype, int x, int y, 
    uint32 axesChanged, uint button, bool buttonstate, uint32 buttonMask, 
    uint32 modifiers);
  static csEvent *NewEvent (csRef<iEventNameRegistry> &reg, csTicks, 
    csEventID name, uint8 n, csMouseEventType etype, const int32 *axes, 
    uint8 numAxes, uint32 axesChanged, uint button, bool buttonstate, 
    uint32 buttonMask, uint32 modifiers);
  //@}

  /// Retrieve the event type (key up or down.)
  CS_PURE_METHOD static csMouseEventType GetEventType (const iEvent* event);
  /// retrieve mouse number (0, 1, ...)
  CS_PURE_METHOD static uint GetNumber(const iEvent *event);
  /// retrieve X value of mouse #0
  CS_PURE_METHOD static int GetX(const iEvent *event)
  { return csMouseEventHelper::GetAxis(event, 0); }
  /// Retrieve Y value of mouse #0
  CS_PURE_METHOD static int GetY(const iEvent *event)
  { return csMouseEventHelper::GetAxis(event, 1); }
  /// retrieve any axis (basis 0) value
  CS_PURE_METHOD static int GetAxis(const iEvent *event, uint axis);
  /// retrieve number of axes
  CS_PURE_METHOD static uint GetNumAxes(const iEvent *event);
  /// retrieve button code
  CS_PURE_METHOD static uint GetButton(const iEvent *event);
  /// retrieve button state (pressed/released)
  CS_PURE_METHOD static bool GetButtonState(const iEvent *event);
  /// Retrieve current button mask
  CS_PURE_METHOD static uint32 GetButtonMask(const iEvent *event);
  /// retrieve modifier flags
  CS_PURE_METHOD static void GetModifiers(const iEvent *event, 
    csKeyModifiers& modifiers) 
  { csKeyEventHelper::GetModifiers(event, modifiers); }
  /// Retrieve modifiers bitmask
  CS_PURE_METHOD static uint32 GetModifiers(const iEvent *event) 
  { 
    csKeyModifiers modifiers; 
    csKeyEventHelper::GetModifiers(event, modifiers); 
    return csKeyEventHelper::GetModifiersBits(modifiers); 
  }
  /// Retrieve event data
  static bool GetEventData (const iEvent* event, 
    csMouseEventData& data);
};

/** @} */
 
/**\name Joystick event related
 * @{ */
/**
 * Helper class to conveniently deal with joystick events.
 */
class CS_CRYSTALSPACE_EXPORT csJoystickEventHelper
{
public:
  //@{
  /// Create new joystick event
  static csEvent *NewEvent (csRef<iEventNameRegistry> &reg, csTicks, 
    csEventID name, int n, int x, int y, uint32 axesChanged, uint button, 
    bool buttonState, uint32 buttonMask, uint32 modifiers);
  static csEvent *NewEvent (csRef<iEventNameRegistry> &reg, csTicks, 
    csEventID name, int n, const int32* axes, uint8 numAxes, uint32 axesChanged, 
    uint button, bool buttonState, uint32 buttonMask, uint32 modifiers);
  //@}

  /// Retrieve joystick number (0, 1, 2, ...)
  CS_PURE_METHOD static uint GetNumber(const iEvent *event);
  /// Retrieve axis 0 value (X)
  CS_PURE_METHOD CS_DEPRECATED_METHOD static int GetX(const iEvent *event) 
  { return csJoystickEventHelper::GetAxis(event, 0); }
  /// Retrieve axis 1 value (Y)
  CS_PURE_METHOD CS_DEPRECATED_METHOD static int GetY(const iEvent *event) 
  { return csJoystickEventHelper::GetAxis(event, 1); }
  /// retrieve any axis (basis 0) value
  CS_PURE_METHOD static int GetAxis(const iEvent *event, uint);
  /// retrieve number of axes
  CS_PURE_METHOD static uint GetNumAxes(const iEvent *);
  /// retrieve button number
  CS_PURE_METHOD static uint GetButton(const iEvent *event);
  /// retrieve button state (pressed/released)
  CS_PURE_METHOD static bool GetButtonState(const iEvent *event);
  /// Retrieve current button mask
  CS_PURE_METHOD static uint32 GetButtonMask(const iEvent *event);
  /// retrieve modifier flags
  static void GetModifiers(const iEvent *event, csKeyModifiers& modifiers) 
  { csKeyEventHelper::GetModifiers(event, modifiers); }
  /// Retrieve modifiers bitmask
  CS_PURE_METHOD static uint32 GetModifiers(const iEvent *event) 
  { 
    csKeyModifiers modifiers; 
    csKeyEventHelper::GetModifiers(event, modifiers); 
    return csKeyEventHelper::GetModifiersBits(modifiers); 
  }
  /// Retrieve event data
  static bool GetEventData (const iEvent* event, csJoystickEventData& data);
};

/** @} */

/**\name Generic input event stuff
 * @{ */
/** 
 * Helper class to conveniently pull generic data out of input events.
 */
class CS_CRYSTALSPACE_EXPORT csInputEventHelper
{
public:
  /// Retrieve button number (0 on error or if keyboard event)
  CS_PURE_METHOD static uint GetButton (iEventNameRegistry *,
  	const iEvent *event);
  /// Retrieve button/key state (true = press, false = release)
  CS_PURE_METHOD static bool GetButtonState (iEventNameRegistry *,
  	const iEvent *event);
};

/**\name Command event related
 * @{ */
/**
 * Helper class to conveniently deal with command events.
 * Note that "command" is a vestigial event type; this class
 * will probably go away before long.
 */
class CS_CRYSTALSPACE_EXPORT csCommandEventHelper
{
public:
  /// Create a new "command" event
  static csEvent *NewEvent (csTicks, csEventID name, bool Broadcast, 
    intptr_t info = 0);

  /// Retrieve command code
  CS_PURE_METHOD static uint GetCode(const iEvent *event);
  /// Retrieve command info
  CS_PURE_METHOD static intptr_t GetInfo(const iEvent *event);
  /// Retrieve event data
  static bool GetEventData (const iEvent* event, csCommandEventData& data);
};

/** @} */

struct iEventQueue;
struct iObjectRegistry;

/**
 * Helper function for registering an event handler using a weak reference.
 * Use RemoveWeakListener() to remove an event handler registered with this
 * function.
 */
csHandlerID RegisterWeakListener (iEventQueue *q, iEventHandler *listener,
  csRef<iEventHandler> &handler);
csHandlerID RegisterWeakListener (iEventQueue *q, iEventHandler *listener,
  const csEventID &ename, csRef<iEventHandler> &handler);
csHandlerID RegisterWeakListener (iEventQueue *q, iEventHandler *listener,
  const csEventID ename[], csRef<iEventHandler> &handler);

/**
 * Helper function for removing an event handler that was registered with
 * RegisterWeakListener().
 */
void RemoveWeakListener (iEventQueue *q, iObjectRegistry *r,
  csRef<iEventHandler> &handler);
 
/** @} */

#endif // __CS_CSUTIL_EVENT_H__
