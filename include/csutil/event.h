/*
    Event system related helpers
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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
  static uint32 GetModifiersBits (const csKeyModifiers& modifiers);
  /**
   * Get a bitmask corresponding to the pressed modifier keys from the event.
   * \sa CSMASK_ALT etc.
   */
  static uint32 GetModifiersBits (const iEvent* event);
  /**
   * Convert a bitmask returned by GetModifiersBits back to a csKeyModifiers
   * struct.
   *
   * Also works for the Modifiers members of the csEventMouseData and
   * csEventJoystickData structs, if you cast them to uint32.
   */
  static void GetModifiers (uint32 mask, csKeyModifiers& modifiers);
};
/** @} */
 
/** @} */

#endif // __CS_CSUTIL_EVENT_H__
