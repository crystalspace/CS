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
  CS_CONST_METHOD static uint32 GetModifiersBits (const csKeyModifiers& modifiers);
  /**
   * Get a bitmask corresponding to the pressed modifier keys from the event.
   * \sa CSMASK_ALT etc.
   */
  CS_PURE_METHOD static uint32 GetModifiersBits (const iEvent* event);
  /**
   * Convert a bitmask returned by GetModifiersBits back to a csKeyModifiers
   * struct.
   *
   * Also works for the Modifiers members of the csMouseEventData and
   * csJoystickEventData structs, if you cast them to uint32.
   */
  static void GetModifiers (uint32 mask, csKeyModifiers& modifiers);
};
/** @} */

/**\name Mouse event related
 * @{ */
/**
 * Helper class to conveniently deal with mouse events.
 */
class CS_CRYSTALSPACE_EXPORT csMouseEventHelper
{
public:
  /// retrieve X value
  CS_PURE_METHOD static int GetX(const iEvent *event);
  /// retrieve Y value
  CS_PURE_METHOD static int GetY(const iEvent *event);
  /// retrieve button code
  CS_PURE_METHOD static int GetButton(const iEvent *event);
  /// retrieve modifier flags
  CS_PURE_METHOD static void GetModifiers(const iEvent *event, csKeyModifiers& modifiers) { return csKeyEventHelper::GetModifiers(event, modifiers); }
  /// retrieve modifiers bitmask
  CS_PURE_METHOD static uint32 GetModifiers(const iEvent *event) { csKeyModifiers modifiers; csKeyEventHelper::GetModifiers(event, modifiers); return csKeyEventHelper::GetModifiersBits(modifiers); }
  /// retrieve event dataa
  CS_PURE_METHOD static bool GetEventData (const iEvent* event, csMouseEventData& data);
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
  /// retrieve joystick number (1, 2, ...)
  CS_PURE_METHOD static int GetNumber(const iEvent *event);
  /// retrieve axis 0 value
  CS_PURE_METHOD CS_DEPRECATED_METHOD static int GetX(const iEvent *event) { return csJoystickEventHelper::GetAxis(event, 1); }
  /// retrieve axis 1 value
  CS_PURE_METHOD CS_DEPRECATED_METHOD static int GetY(const iEvent *event) { return csJoystickEventHelper::GetAxis(event, 2); }
  /// retrieve any axis (basis 1) value
  CS_PURE_METHOD static int GetAxis(const iEvent *event, int);
  /// retrieve number of axes
  CS_PURE_METHOD static uint8 GetNumAxes(const iEvent *);
  /// retrieve button number
  CS_PURE_METHOD static int GetButton(const iEvent *event);
  /// retrieve modifier flags
  static void GetModifiers(const iEvent *event, csKeyModifiers& modifiers) { return csKeyEventHelper::GetModifiers(event, modifiers); }
  /// retrieve modifiers bitmask
  CS_PURE_METHOD static uint32 GetModifiers(const iEvent *event) { csKeyModifiers modifiers; csKeyEventHelper::GetModifiers(event, modifiers); return csKeyEventHelper::GetModifiersBits(modifiers); }
  /// retrieve event dataa
  static bool GetEventData (const iEvent* event, csJoystickEventData& data);
};

/** @} */
 
/**\name Command event related
 * @{ */
/**
 * Helper class to conveniently deal with command events.
 */
class CS_CRYSTALSPACE_EXPORT csCommandEventHelper
{
public:
  /// retrieve command code
  CS_PURE_METHOD static uint GetCode(const iEvent *event);
  /// retrieve command info
  CS_PURE_METHOD static intptr_t GetInfo(const iEvent *event);
  /// retrieve event dataa
  static bool GetEventData (const iEvent* event, csCommandEventData& data);
};

/** @} */
 
/** @} */

#endif // __CS_CSUTIL_EVENT_H__
