/*
    Event system related helpers
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_CSUTIL_EVENT_H__
#define __CS_CSUTIL_EVENT_H__

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
class csKeyEventHelper
{
public:
  /// Retrieve the key's raw code.
  static utf32_char GetRawCode (iEvent* event);
  /// Retrieve the key's cooked code.
  static utf32_char GetCookedCode (iEvent* event);
  /// Retrieve the key's raw code.
  static void GetModifiers (iEvent* event, csKeyModifiers& modifiers);
  /// Retrieve the event type (key up or down.)
  static csKeyEventType GetEventType (iEvent* event);
  /**
   * Retrieve whether a keyboard down event was caused by the initial press
   * (not auto-repeat) or by having it held for a period of time (auto-repeat.)
   */
  static bool GetAutoRepeat (iEvent* event);
  /// Retrieve the character type (dead or normal.)
  static csKeyCharType GetCharacterType (iEvent* event);
  /// Get all the information in one compact struct.
  static bool GetEventData (iEvent* event, csKeyEventData& data);
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
  static uint32 GetModifiersBits (iEvent* event);
};
/** @} */
 
/** @} */

#endif // __CS_CSUTIL_EVENT_H__
