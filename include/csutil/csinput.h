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
 * These are the low-level implementations of generic classes of input devices
 * like keyboard, mouse, and joystick.  System-dependent code should inherit
 * system-specific classes from those defined below, implementing as much
 * functionality as possible.
 */

#include "csutil/scf.h"
#include "csutil/bitset.h"
#include "iutil/csinput.h"
#include "isys/plugin.h"
struct iEvent;
struct iEventQueue;
struct iObjectRegistry;

/**
 * Superclass of all the generic input drivers.
 */
class csInputDriver
{
protected:
  iObjectRegistry* Registry;
  csInputDriver(iObjectRegistry*);
  virtual ~csInputDriver();
  iEventQueue* GetEventQueue();
  virtual void LostFocus() = 0;
  virtual void Post(iEvent*);
private:
  struct FocusListener : public iPlugin
  {
    csInputDriver* Parent;
    SCF_DECLARE_IBASE;
    FocusListener() { SCF_CONSTRUCT_IBASE(0); }
    virtual bool Initialize(iObjectRegistry*) {}
    virtual bool HandleEvent(iEvent&);
  };
  friend struct FocusListener;
  FocusListener Listener;
  iEventQueue* Queue;
  void StartListening();
  void StopListening();
};

/**
 * Generic Keyboard Driver.<p>
 * Keyboard driver should generate events and put them into
 * an event queue. Also it tracks the current state of all keys.
 */
class csKeyboardDriver : public csInputDriver, public iKeyboardDriver
{
protected:
  /// Key state array.
  csBitSet KeyState;

  /**
   * Set key state. For example SetKey (CSKEY_UP, true). Called
   * automatically by do_press and do_release.
   */
  virtual void SetKeyState (int iKey, bool iDown);

public:
  SCF_DECLARE_IBASE;

  /// Initialize keyboard interface.
  csKeyboardDriver (iObjectRegistry*);

  /// Call to release all key down flags.
  virtual void Reset ();

  /// Call this routine to add a key down/up event to queue
  virtual void DoKey (int iKey, int iChar, bool iDown);

  /**
   * Query the state of a key. All key codes in range 0..255,
   * CSKEY_FIRST..CSKEY_LAST are supported. Returns true if
   * the key is pressed, false if not.
   */
  virtual bool GetKeyState (int iKey);

private:
  /// Application lost focus.
  virtual void LostFocus() { Reset(); }
};

/**
 * Generic Mouse Driver.<p>
 * Mouse driver should generate events and put them into the event queue.  Also
 * it is responsible for generating double-click events.
 */
class csMouseDriver : public csInputDriver, public iMouseDriver
{
private:
  // Generic keyboard driver (for checking modifier key states).
  iKeyboardDriver* Keyboard;

protected:
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
  csTicks DoubleClickTime;
  /// Mouse double click max distance
  size_t DoubleClickDist;
  /// Get the generic keyboard driver (for checking modifier states).
  iKeyboardDriver* GetKeyboardDriver();

public:
  SCF_DECLARE_IBASE;

  /// Initialize mouse interface.
  csMouseDriver (iObjectRegistry*);

  /// Set double-click mouse parameters
  virtual void SetDoubleClickTime (int iTime, size_t iDist);

  /// Call to release all mouse buttons.
  virtual void Reset ();

  /// Query last mouse X position
  virtual int GetLastX () { return LastX; }
  /// Query last mouse Y position
  virtual int GetLastY () { return LastY; }
  /// Query the last known mouse button state
  virtual bool GetLastButton (int button)
  {
    return (button > 0 && button <= CS_MAX_MOUSE_BUTTONS) ?
      Button [button - 1] : false;
  }

  /// Call this to add a 'mouse button down/up' event to queue
  virtual void DoButton (int button, bool down, int x, int y);
  /// Call this to add a 'mouse moved' event to queue
  virtual void DoMotion (int x, int y);

private:
  /// Application lost focus.
  virtual void LostFocus() { Reset(); }
};

/**
 * Generic Joystick driver.<p>
 * The joystick driver is responsible for tracking current
 * joystick state and also for generating joystick events.
 */
class csJoystickDriver : public csInputDriver, public iJoystickDriver
{
private:
  // Generic keyboard driver (for checking modifier key states).
  iKeyboardDriver* Keyboard;
protected:
  /// Joystick button states
  bool Button [CS_MAX_JOYSTICK_COUNT][CS_MAX_JOYSTICK_BUTTONS];
  /// Joystick axis positions
  int LastX [CS_MAX_JOYSTICK_COUNT], LastY [CS_MAX_JOYSTICK_COUNT];
  /// Get the generic keyboard driver (for checking modifier states).
  iKeyboardDriver* GetKeyboardDriver();

public:
  SCF_DECLARE_IBASE;

  /// Initialize joystick interface.
  csJoystickDriver (iObjectRegistry*);

  /// Call to release all joystick buttons.
  virtual void Reset ();

  /// Query last joystick X position
  virtual int GetLastX (int number) { return LastX [number]; }
  /// Query last joystick Y position
  virtual int GetLastY (int number) { return LastY [number]; }
  /// Query the last known joystick button state
  virtual bool GetLastButton (int number, int button)
  {
    return (number > 0 && number <= CS_MAX_JOYSTICK_COUNT
         && button > 0 && button <= CS_MAX_JOYSTICK_BUTTONS) ?
            Button [number - 1] [button - 1] : false;
  }

  /// Call this to add a 'joystick button down/up' event to queue
  virtual void DoButton (int number, int button, bool down, int x, int y);
  /// Call this to add a 'joystick moved' event to queue
  virtual void DoMotion (int number, int x, int y);

private:
  /// Application lost focus.
  virtual void LostFocus() { Reset(); }
};

#endif // __CS_CSINPUT_H__
