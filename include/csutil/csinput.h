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

#ifndef __CS_CSINPUT_H__
#define __CS_CSINPUT_H__

/**
 * These are the low-level interfaces to input devices like keyboard
 * and mouse (possibly joystick will be added in the future).
 * System-dependent code should inherit system-specific classes from
 * those defined below, implementing as much functionality as possible.
 */

#include "csutil/scf.h"
#include "csutil/bitset.h"
#include "cssys/csevent.h"
#include "cssys/cseventq.h"
class csSystemDriver;

/// Maximal number of mouse buttons supported
#define CS_MAX_MOUSE_BUTTONS	10
/// Maximal number of joysticks supported
#define CS_MAX_JOYSTICK_COUNT	2
/// Maximal number of joystick buttons supported
#define CS_MAX_JOYSTICK_BUTTONS	10

/**
 * System Keyboard Driver.<p>
 * Keyboard driver should generate events and put them into the
 * system event queue. Also it tracks the current state for all keys.
 */
class csKeyboardDriver
{
  /// The system driver
  csSystemDriver *System;
  /// key state array
  csBitSet KeyState;

public:
  /// Initialize keyboard interface
  csKeyboardDriver (csSystemDriver* = NULL);

  /// Set the system driver.
  void SetSystemDriver (csSystemDriver* system)
  { System = system; }

  /**
   * Call to release all key down flags
   * (when focus switches from application window, for example)
   */
  void Reset ();

  /// Call this routine to add a key down/up event to queue
  void DoKey (int iKey, int iChar, bool iDown);

  /**
   * Query the state of a key. All key codes in range 0..255,
   * CSKEY_FIRST..CSKEY_LAST are supported. Returns true if
   * the key is pressed, false if not.
   */
  bool GetKeyState (int iKey);

private:
  /**
   * Set key state. For example SetKey (CSKEY_UP, true). Called
   * automatically by do_press and do_release.
   */
  void SetKeyState (int iKey, bool iDown);
};

/**
 * System Mouse Driver.<p>
 * Mouse driver should generate events and put them into the
 * event queue which is passed to object on Open(). Also it
 * is responsible for generating double-click events.
 */
class csMouseDriver
{
  /// The system driver
  csSystemDriver *System;
  /// Last "mouse down" event time
  csTicks LastClickTime;
  /// Last "mouse down" event button
  int LastClickButton;
  /// Last "mouse down" event position
  int LastClickX, LastClickY;
  /// Last mouse position
  int LastX, LastY;
  /// Mouse buttons state
  bool Button [CS_MAX_MOUSE_BUTTONS];
  /// Mouse double click max interval in 1/1000 seconds
  static csTicks DoubleClickTime;
  /// Mouse double click max distance
  static size_t DoubleClickDist;

public:
  /// Initialize mouse interface
  csMouseDriver (csSystemDriver* = NULL);

  /// Set the system driver.
  void SetSystemDriver (csSystemDriver* system)
  { System = system; }

  /// Set double-click mouse parameters
  static void SetDoubleClickTime (int iTime, size_t iDist);

  /**
   * Call to release all mouse buttons
   * (when focus switches from application window, for example)
   */
  void Reset ();

  /// Query last mouse X position
  int GetLastX () { return LastX; }
  /// Query last mouse Y position
  int GetLastY () { return LastY; }
  /// Query the last known mouse button state
  bool GetLastButton (int button)
  {
    return (button > 0 && button <= CS_MAX_MOUSE_BUTTONS) ?
      Button [button - 1] : false;
  }

  /// Call this to add a 'mouse button down/up' event to queue
  void DoButton (int button, bool down, int x, int y);
  /// Call this to add a 'mouse moved' event to queue
  void DoMotion (int x, int y);
};

/**
 * System Joystick driver.<p>
 * The joystick driver is responsible for tracking current
 * joystick state and also for generating joystick events.
 */
class csJoystickDriver
{
  /// The system driver
  csSystemDriver *System;
  /// Joystick button states
  bool Button [CS_MAX_JOYSTICK_COUNT][CS_MAX_JOYSTICK_BUTTONS];
  /// Joystick axis positions
  int LastX [CS_MAX_JOYSTICK_COUNT], LastY [CS_MAX_JOYSTICK_COUNT];

public:
  /// Initialize joystick interface
  csJoystickDriver (csSystemDriver* = NULL);

  /// Set the system driver.
  void SetSystemDriver (csSystemDriver* system)
  { System = system; }

  /**
   * Call to release all joystick buttons
   * (when focus switches from application window, for example)
   */
  void Reset ();

  /// Query last joystick X position
  int GetLastX (int number) { return LastX [number]; }
  /// Query last joystick Y position
  int GetLastY (int number) { return LastY [number]; }
  /// Query the last known joystick button state
  bool GetLastButton (int number, int button)
  {
    return (number > 0 && number <= CS_MAX_JOYSTICK_COUNT
         && button > 0 && button <= CS_MAX_JOYSTICK_BUTTONS) ?
            Button [number - 1] [button - 1] : false;
  }

  /// Call this to add a 'joystick button down/up' event to queue
  void DoButton (int number, int button, bool down, int x, int y);
  /// Call this to add a 'joystick moved' event to queue
  void DoMotion (int number, int x, int y);
};

#endif // __CS_CSINPUT_H__
