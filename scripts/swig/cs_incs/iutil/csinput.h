/*
    Crystal Space input library
    Copyright (C) 1998,2000 by Jorrit Tyberghein
    Written by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __IUTIL_STDINPUT_H__
#define __IUTIL_STDINPUT_H__

/**
 * These are the low-level interfaces to generic classes of input devices like
 * keyboard, mouse, and joystick.  System-dependent code should inherit
 * system-specific classes from those defined below, implementing as much
 * functionality as possible.
 */

#include "csutil/scf.h"

/// Maximal number of mouse buttons supported
#define CS_MAX_MOUSE_BUTTONS	10
/// Maximal number of joysticks supported
#define CS_MAX_JOYSTICK_COUNT	2
/// Maximal number of joystick buttons supported
#define CS_MAX_JOYSTICK_BUTTONS	10

const int VERSION_iKeyboardDriver = 1;

/**
 * Generic Keyboard Driver.<p>
 * Keyboard driver should generate events and put them into an event queue.
 * Also it tracks the current state of all keys.  Typically, one instance of
 * this object is available from the shared-object registry (iObjectRegistry)
 * under the name "crystalspace.driver.input.generic.keyboard".
 */
struct iKeyboardDriver : public iBase
{
  /**
   * Call to release all key down flags (when focus switches from application
   * window, for example).
   */
  virtual void Reset () = 0;

  /// Call to add a key down/up event to queue.
  virtual void DoKey (int iKey, int iChar, bool iDown) = 0;

  /**
   * Query the state of a key. All key codes in range 0..255,
   * CSKEY_FIRST..CSKEY_LAST are supported. Returns true if
   * the key is pressed, false if not.
   */
  virtual bool GetKeyState (int iKey) = 0;
};

const int VERSION_iMouseDriver = 1;

/**
 * Generic Mouse Driver.<p>
 * Mouse driver should generate events and put them into the event queue.  Also
 * it is responsible for generating double-click events.  Typically, one
 * instance of this object is available from the shared-object registry
 * (iObjectRegistry) under the name "crystalspace.driver.input.generic.mouse".
 */
struct iMouseDriver : public iBase
{
  /// Set double-click mouse parameters.
  virtual void SetDoubleClickTime (int iTime, size_t iDist) = 0;

  /**
   * Call to release all mouse buttons * (when focus switches from application
   * window, for example).
   */
  virtual void Reset () = 0;

  /// Query last mouse X position
  virtual int GetLastX () = 0;
  /// Query last mouse Y position
  virtual int GetLastY () = 0;
  /// Query the last known mouse button state
  virtual bool GetLastButton (int button) = 0;

  /// Call this to add a 'mouse button down/up' event to queue
  virtual void DoButton (int button, bool down, int x, int y) = 0;
  /// Call this to add a 'mouse moved' event to queue
  virtual void DoMotion (int x, int y) = 0;
};

const int VERSION_iJoystickDriver = 1;

/**
 * Generic Joystick driver.<p>
 * The joystick driver is responsible for tracking current joystick state and
 * also for generating joystick events.  Typically, one instance of this object
 * is available from the shared-object registry (iObjectRegistry) under the
 * name "crystalspace.driver.input.generic.joystick".
 */
struct iJoystickDriver : public iBase
{
  /**
   * Call to release all joystick buttons (when focus switches from application
   * window, for example).
   */
  virtual void Reset () = 0;

  /// Query last joystick X position
  virtual int GetLastX (int number) = 0;
  /// Query last joystick Y position
  virtual int GetLastY (int number) = 0;
  /// Query the last known joystick button state
  virtual bool GetLastButton (int number, int button) = 0;

  /// Call this to add a 'joystick button down/up' event to queue
  virtual void DoButton (int number, int button, bool down, int x, int y) = 0;
  /// Call this to add a 'joystick moved' event to queue
  virtual void DoMotion (int number, int x, int y) = 0;
};

#endif // __IUTIL_STDINPUT_H__