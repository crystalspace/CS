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
  static utf32_char GetRawCode (const iEvent* event);
  /// Retrieve the key's cooked code.
  static utf32_char GetCookedCode (const iEvent* event);
  /// Retrieve the key's raw code.
  static void GetModifiers (const iEvent* event, csKeyModifiers& modifiers);
  /// Retrieve the event type (key up or down.)
  static csKeyEventType GetEventType (const iEvent* event);
  /**
   * Retrieve whether a keyboard down event was caused by the initial press
   * (not auto-repeat) or by having it held for a period of time (auto-repeat.)
   */
  static bool GetAutoRepeat (const iEvent* event);
  /// Retrieve the character type (dead or normal.)
  static csKeyCharType GetCharacterType (const iEvent* event);
  /// Get all the information in one compact struct.
  static bool GetEventData (const iEvent* event, csKeyEventData& data);
  /**
   * Get a bitmask corresponding to the pressed modifier keys from the
   * keyboard modifiers struct.
   * \sa CSMASK_ALT etc.
   */
  static uint32 GetModifiersBits (
    const csKeyModifiers& modifiers);
  /**
   * Get a bitmask corresponding to the pressed modifier keys from the event.
   * \sa CSMASK_ALT etc.
   */
  static uint32 GetModifiersBits (const iEvent* event);
  /**
   * Convert a bitmask returned by GetModifiersBits back to a csKeyModifiers
   * struct.
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
    uint32 AxesChanged, int button, bool buttonstate, uint32 buttonMask, 
    const csKeyModifiers& modifiers);
  static csEvent *NewEvent (csRef<iEventNameRegistry> &reg, csTicks, 
    csEventID name, uint8 n, csMouseEventType etype, int x, int y, 
    uint32 axesChanged, int button, bool buttonstate, uint32 buttonMask, 
    const csKeyModifiers& modifiers);
  static csEvent *NewEvent (csRef<iEventNameRegistry> &reg, csTicks, 
    csEventID name, uint8 n, csMouseEventType etype, const int32 *axes, 
    uint8 numAxes, uint32 axesChanged, int button, bool buttonstate, 
    uint32 buttonMask, const csKeyModifiers& modifiers);
  //@}

  // Deprecated in 1.3.
  static CS_DEPRECATED_METHOD_MSG("Use the variant with csKeyModifiers modifiers")
  csEvent *NewEvent (csRef<iEventNameRegistry> &reg, 
    csTicks t, csEventID name, csMouseEventType etype, int x, int y, 
    uint32 AxesChanged, int button, bool buttonstate, uint32 buttonMask, 
    uint32 modifiers)
  {
    csKeyModifiers m;
    csKeyEventHelper::GetModifiers (modifiers, m);
    return NewEvent (reg, t, name, etype, x, y, AxesChanged, button, 
      buttonstate, buttonMask, m);
  }
  // Deprecated in 1.3.
  static CS_DEPRECATED_METHOD_MSG("Use the variant with csKeyModifiers modifiers")
  csEvent *NewEvent (csRef<iEventNameRegistry> &reg, csTicks t, 
    csEventID name, uint8 n, csMouseEventType etype, int x, int y, 
    uint32 axesChanged, int button, bool buttonstate, uint32 buttonMask, 
    uint32 modifiers)
  {
    csKeyModifiers m;
    csKeyEventHelper::GetModifiers (modifiers, m);
    return NewEvent (reg, t, name, n, etype, x, y, axesChanged, button, 
      buttonstate, buttonMask, m);
  }
  // Deprecated in 1.3.
  static CS_DEPRECATED_METHOD_MSG("Use the variant with csKeyModifiers modifiers")
  csEvent *NewEvent (csRef<iEventNameRegistry> &reg, csTicks t, 
    csEventID name, uint8 n, csMouseEventType etype, const int32 *axes, 
    uint8 numAxes, uint32 axesChanged, int button, bool buttonstate, 
    uint32 buttonMask, uint32 modifiers)
  {
    csKeyModifiers m;
    csKeyEventHelper::GetModifiers (modifiers, m);
    return NewEvent (reg, t, name, n, etype, axes, numAxes, axesChanged,
      button, buttonstate, buttonMask, m);
  }

  /// Retrieve the event type (key up or down.)
  static csMouseEventType GetEventType (const iEvent* event);
  /// retrieve mouse number (0, 1, ...)
  static uint GetNumber(const iEvent *event);
  /// retrieve X value of mouse #0
  static int GetX(const iEvent *event)
  { return GetAxis(event, 0); }
  /// Retrieve Y value of mouse #0
  static int GetY(const iEvent *event)
  { return GetAxis(event, 1); }
  /// retrieve any axis (basis 0) value
  static int GetAxis(const iEvent *event, uint axis);
  /// retrieve number of axes
  static uint GetNumAxes(const iEvent *event);
  /// retrieve button code
  static int GetButton(const iEvent *event);
  /// retrieve button state (pressed/released)
  static bool GetButtonState(const iEvent *event);
  /// Retrieve current button mask
  static uint32 GetButtonMask(const iEvent *event);
  /// retrieve modifier flags
  static void GetModifiers(const iEvent *event, 
    csKeyModifiers& modifiers) 
  { csKeyEventHelper::GetModifiers(event, modifiers); }
  /// Retrieve modifiers bitmask
  static uint32 GetModifiers(const iEvent *event) 
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
    bool buttonState, uint32 buttonMask, const csKeyModifiers& modifiers);
  static csEvent *NewEvent (csRef<iEventNameRegistry> &reg, csTicks, 
    csEventID name, int n, const int32* axes, uint8 numAxes, uint32 axesChanged, 
    uint button, bool buttonState, uint32 buttonMask, const csKeyModifiers& modifiers);
  //@}

  // Deprecated in 1.3.
  static CS_DEPRECATED_METHOD_MSG("Use the variant with csKeyModifiers modifiers")
  csEvent *NewEvent (csRef<iEventNameRegistry> &reg, csTicks t, 
    csEventID name, int n, int x, int y, uint32 axesChanged, uint button, 
    bool buttonState, uint32 buttonMask, uint32 modifiers)
  {
    csKeyModifiers m;
    csKeyEventHelper::GetModifiers (modifiers, m);
    return NewEvent (reg, t, name, n, x, y, axesChanged, button, 
      buttonState, buttonMask, m);
  }
  // Deprecated in 1.3.
  static CS_DEPRECATED_METHOD_MSG("Use the variant with csKeyModifiers modifiers")
  csEvent *NewEvent (csRef<iEventNameRegistry> &reg, csTicks t, 
    csEventID name, int n, const int32* axes, uint8 numAxes, uint32 axesChanged, 
    uint button, bool buttonState, uint32 buttonMask, uint32 modifiers)
  {
    csKeyModifiers m;
    csKeyEventHelper::GetModifiers (modifiers, m);
    return NewEvent (reg, t, name, n, axes, numAxes, axesChanged, button, 
      buttonState, buttonMask, m);
  }

  /// Retrieve joystick number (0, 1, 2, ...)
  static uint GetNumber(const iEvent *event);
  /// retrieve any axis (basis 0) value
  static int GetAxis(const iEvent *event, uint);
  /// retrieve number of axes
  static uint GetNumAxes(const iEvent *);
  /// retrieve button number
  static uint GetButton(const iEvent *event);
  /// retrieve button state (pressed/released)
  static bool GetButtonState(const iEvent *event);
  /// Retrieve current button mask
  static uint32 GetButtonMask(const iEvent *event);
  /// retrieve modifier flags
  static void GetModifiers(const iEvent *event, csKeyModifiers& modifiers) 
  { csKeyEventHelper::GetModifiers(event, modifiers); }
  /// Retrieve modifiers bitmask
  static uint32 GetModifiers(const iEvent *event) 
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
  static uint GetButton (iEventNameRegistry *,
  	const iEvent *event);
  /// Retrieve button/key state (true = press, false = release)
  static bool GetButtonState (iEventNameRegistry *,
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
  static uint GetCode(const iEvent *event);
  /// Retrieve command info
  static intptr_t GetInfo(const iEvent *event);
  /// Retrieve event data
  static bool GetEventData (const iEvent* event, csCommandEventData& data);
};

/** @} */

struct iEventQueue;
struct iObjectRegistry;

namespace CS
{

/**
 * Helper function for registering an event handler using a weak reference.
 * Use RemoveWeakListener() to remove an event handler registered with this
 * function.
 */
csHandlerID CS_CRYSTALSPACE_EXPORT RegisterWeakListener (iEventQueue *q, 
  iEventHandler *listener, csRef<iEventHandler> &handler);
csHandlerID CS_CRYSTALSPACE_EXPORT RegisterWeakListener (iEventQueue *q, 
  iEventHandler *listener, const csEventID &ename, csRef<iEventHandler> &handler);
csHandlerID CS_CRYSTALSPACE_EXPORT RegisterWeakListener (iEventQueue *q, 
  iEventHandler *listener, const csEventID ename[], csRef<iEventHandler> &handler);

/**
 * Helper function for removing an event handler that was registered with
 * RegisterWeakListener().
 */
void CS_CRYSTALSPACE_EXPORT RemoveWeakListener (iEventQueue *q, 
  csRef<iEventHandler> &handler);

} // namespace CS
  
/** @} */

#endif // __CS_CSUTIL_EVENT_H__
