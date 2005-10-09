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

/**\file
 * Input drivers.
 * These are the low-level implementations of generic classes of input devices
 * like keyboard, mouse, and joystick.
 */

#include "csextern.h"

#include "csutil/array.h"
#include "csutil/hash.h"
#include "csutil/scf.h"
#include "csutil/scf_implementation.h"

#include "iutil/csinput.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

struct iEvent;
struct iEventQueue;
struct iObjectRegistry;

/**
 * Superclass of all the generic input drivers.
 */
class CS_CRYSTALSPACE_EXPORT csInputDriver
{
private:
  bool Registered;
protected:
  iObjectRegistry* Registry;
  iEventHandler* Listener;
  csInputDriver(iObjectRegistry*);
  virtual ~csInputDriver();
  csPtr<iEventQueue> GetEventQueue();
  virtual void GainFocus() = 0;
  virtual void LostFocus() = 0;
  virtual void Post(iEvent*);
  virtual bool HandleEvent(iEvent&);
  friend struct FocusListener;
  void StartListening();
  void StopListening();
};

class CS_CRYSTALSPACE_EXPORT csKeyComposer : 
  public scfImplementation1<csKeyComposer, iKeyComposer>
{
protected:
  utf32_char lastDead;

public:  
  csKeyComposer ();
  virtual ~csKeyComposer ();

  virtual csKeyComposeResult HandleKey (const csKeyEventData& keyEventData,
    utf32_char* buf, size_t bufChars, int* resultChars = 0);
  virtual void ResetState ();
};

#ifdef CS_DEBUG
  #ifndef CS_KEY_DEBUG_ENABLE
    /**
     * Define at CrystalSpace build time to enable support for keyboard input 
     * debugging. (By default available in debug, but not optimize builds.)
     */
    #define CS_KEY_DEBUG_ENABLE
  #endif
#endif

/**
 * Generic Keyboard Driver.<p>
 * Keyboard driver should generate events and put them into
 * an event queue. Also it tracks the current state of all keys.
 */
class CS_CRYSTALSPACE_EXPORT csKeyboardDriver : public csInputDriver,
  public scfImplementation2<csKeyboardDriver, iKeyboardDriver, iEventHandler>
{
protected:
  /// Key state array.
  csHash<bool, utf32_char> keyStates;
  csKeyModifiers modifiersState;
  bool keyDebug;
  bool keyDebugChecked;

  /**
   * Set key state. For example SetKey (CSKEY_UP, true). Called
   * automatically by do_press and do_release.
   */
  virtual void SetKeyState (utf32_char codeRaw, bool iDown,
    bool autoRepeat);
  /**
   * Generates a 'cooked' key code for a 'raw' key code from some simple
   * rules.
   */
  virtual void SynthesizeCooked (utf32_char codeRaw,
    const csKeyModifiers& modifiers, utf32_char& codeCooked);

  const char* GetKeycodeString (utf32_char code);
  bool IsKeyboardDebugging ();

  /// Application lost focus.
  virtual void LostFocus() { Reset(); }
  virtual void GainFocus() { RestoreKeys(); }

  virtual bool HandleEvent (iEvent& e)
  {
    return csInputDriver::HandleEvent (e);
  }
public:
  /// Initialize keyboard interface.
  csKeyboardDriver (iObjectRegistry*);
  /// Destructor.
  virtual ~csKeyboardDriver ();

  /// Call to release all key down flags.
  virtual void Reset ();
  /// Call to get the key down flags in sync with the actual pressed keys.
  virtual void RestoreKeys ();

  /**
   * Call this routine to add a key down/up event to queue.
   * \param codeRaw 'Raw' code of the pressed key.
   * \param codeCooked 'Cooked' code of the pressed key.
   * \param iDown Whether the key is up or down.
   * \param autoRepeat Auto-repeat flag for the key event. Typically only
   *  used by the platform-specific keyboard agents.
   * \param charType When the cooked code is a character, it determines
   *  whether it is a normal, dead or composed character.
   */
  virtual void DoKey (utf32_char codeRaw, utf32_char codeCooked, bool iDown,
    bool autoRepeat = false, csKeyCharType charType = csKeyCharTypeNormal);

  /**
   * Query the state of a key. All key codes are supported. Returns true if
   * the key is pressed, false if not.
   */
  CS_PURE_METHOD virtual bool GetKeyState (utf32_char codeRaw) const;

  /**
   * Query the state of a modifier key.
   * Returns a bit field, where the nth bit is set if the nth modifier of a
   * type is pressed. If a specific modifier is requested, e.g. 
   * #CSKEY_SHIFT_LEFT, only the according bit is set. Otherwise, for a generic
   * modifier (e.g. #CSKEY_SHIFT), all distinct modifier keys of that type are 
   * represented.<p>
   * Example: Test if any Alt key is pressed:
   * \code
   *   bool pressed = (KeyboardDriver->GetModifierState(CSKEY_ALT) != 0);
   * \endcode
   * Example: Test if the right Ctrl key is pressed:
   * \code
   *   bool pressed = (KeyboardDriver->GetModifierState(CSKEY_CTRL_RIGHT) != 0);
   * \endcode
   * \param codeRaw Raw code of the modifier key.
   * \return Bit mask with the pressed modifiers.
   */
  CS_PURE_METHOD virtual uint32 GetModifierState (utf32_char codeRaw) const;

  virtual csPtr<iKeyComposer> CreateKeyComposer ();

  /// Fills in the 'cooked' key code of an event with only a 'raw' key code.
  virtual csEventError SynthesizeCooked (iEvent *);
};

/**
 * Generic Mouse Driver.<p>
 * Mouse driver should generate events and put them into the event queue.  Also
 * it is responsible for generating double-click events. Mouse button numbers
 * are 1-based.
 */
class CS_CRYSTALSPACE_EXPORT csMouseDriver : public csInputDriver, 
  public scfImplementation2<csMouseDriver, iMouseDriver, iEventHandler>
{
private:
  // Generic keyboard driver (for checking modifier key states).
  csRef<iKeyboardDriver> Keyboard;

  virtual bool HandleEvent (iEvent& e)
  {
    return csInputDriver::HandleEvent (e);
  }
protected:
  /// Last "mouse down" event time
  csTicks LastClickTime[CS_MAX_MOUSE_COUNT];
  /// Last "mouse down" event button
  uint LastClickButton[CS_MAX_MOUSE_COUNT];
  /// Last "mouse down" event position
  int LastClick [CS_MAX_MOUSE_COUNT][CS_MAX_MOUSE_AXES];
  /// Last mouse positions
  int32 Last [CS_MAX_MOUSE_COUNT][CS_MAX_MOUSE_AXES];
  uint Axes [CS_MAX_MOUSE_COUNT];
  /// Mouse buttons state
  bool Button [CS_MAX_MOUSE_COUNT][CS_MAX_MOUSE_BUTTONS];
  /// Mouse double click max interval in 1/1000 seconds
  csTicks DoubleClickTime;
  /// Mouse double click max distance
  size_t DoubleClickDist;
  /// Get the generic keyboard driver (for checking modifier states).
  iKeyboardDriver* GetKeyboardDriver();

public:
  /// Initialize mouse interface.
  csMouseDriver (iObjectRegistry*);
  /// Destructor.
  virtual ~csMouseDriver ();

  /// Set double-click mouse parameters
  virtual void SetDoubleClickTime (int iTime, size_t iDist);

  /// Call to release all mouse buttons.
  virtual void Reset ();

  /// Query last mouse X position for mouse \#n (1, 2, ...)
  CS_PURE_METHOD virtual int GetLastX (uint n) const { return Last[n - 1][0]; }
  /// Query last mouse Y position for mouse \#n (1, 2, ...)
  CS_PURE_METHOD virtual int GetLastY (uint n) const { return Last[n - 1][1]; }
  /// Query last mouse position on axis ax (1, 2, ...) for mouse n (1, 2, ...)
  CS_PURE_METHOD virtual int GetLast (uint n, uint axis) const
  { return Last[n - 1][axis - 1]; }
  /// Query last mouse axis array for mouse n (1, 2, ...)
  CS_PURE_METHOD virtual const int32 *GetLast (uint n) const
  { return Last [n - 1]; }
  /// Query the last known mouse button state for mouse \#1
  CS_PURE_METHOD virtual bool GetLastButton (uint button) const
  { return GetLastButton(1, button); }
  /// Query the last known mouse button state
  CS_PURE_METHOD virtual bool GetLastButton (uint number, uint button) const
  {
    return (number > 0 && number <= CS_MAX_MOUSE_COUNT
	    && button > 0 && button <= CS_MAX_MOUSE_BUTTONS) ?
	    Button [number - 1][button - 1] : false;
  }

  /// Call this to add a 'mouse button down/up' event to queue
  virtual void DoButton (uint number, uint button, bool down,
  	const int32 *axes, uint numAxes);
  virtual void DoButton (uint button, bool down, const int32 *axes,
  	uint numAxes) 
  { DoButton (1, button, down, axes, numAxes); }
  virtual void DoButton (uint button, bool down, int x, int y)
  { int32 axes[2] = {x, y}; DoButton (1, button, down, axes, 2); }
  /// Call this to add a 'mouse moved' event to queue
  virtual void DoMotion (uint number, const int32 *axes, uint numAxes);
  virtual void DoMotion (const int32 *axes, uint numAxes) 
  { DoMotion (1, axes, numAxes); }
  virtual void DoMotion (int x, int y)
  { int32 axes[2] = {x, y}; DoMotion (1, axes, 2); }
  /// Application lost focus.
  virtual void LostFocus() { Reset(); }
  virtual void GainFocus() { }


};

/**
 * Generic Joystick driver.<p>
 * The joystick driver is responsible for tracking current
 * joystick state and also for generating joystick events.
 * Joystick numbers and button numbers are 1-based.
 */
class CS_CRYSTALSPACE_EXPORT csJoystickDriver : public csInputDriver,
  public scfImplementation2<csJoystickDriver, iJoystickDriver, iEventHandler>
{
private:
  // Generic keyboard driver (for checking modifier key states).
  csRef<iKeyboardDriver> Keyboard;
protected:
  /// Joystick button states
  bool Button [CS_MAX_JOYSTICK_COUNT][CS_MAX_JOYSTICK_BUTTONS];
  /// Joystick axis positions
  int32 Last [CS_MAX_JOYSTICK_COUNT][CS_MAX_JOYSTICK_AXES];
  uint Axes [CS_MAX_JOYSTICK_COUNT];
  /// Get the generic keyboard driver (for checking modifier states).
  iKeyboardDriver* GetKeyboardDriver();
  virtual bool HandleEvent (iEvent& e)
  {
    return csInputDriver::HandleEvent (e);
  }
public:

  /// Initialize joystick interface.
  csJoystickDriver (iObjectRegistry*);
  /// Destructor.
  virtual ~csJoystickDriver ();

  /// Call to release all joystick buttons.
  virtual void Reset ();

  /// Query last joystick X position
  CS_DEPRECATED_METHOD CS_PURE_METHOD virtual int GetLastX (uint number) const 
  { return Last [number - 1][0]; }
  /// Query last joystick Y position
  CS_DEPRECATED_METHOD CS_PURE_METHOD virtual int GetLastY (uint number) const 
  { return Last [number - 1][1]; }
  CS_PURE_METHOD virtual const int32 *GetLast (uint number) const 
  { return Last [number - 1]; }
  CS_PURE_METHOD virtual int GetLast (uint number, uint axis) const 
  { return Last [number - 1][axis - 1]; }
  /// Query the last known joystick button state
  CS_PURE_METHOD virtual bool GetLastButton (uint number, uint button) const
  {
    return (number > 0 && number <= CS_MAX_JOYSTICK_COUNT
         && button > 0 && button <= CS_MAX_JOYSTICK_BUTTONS) ?
            Button [number - 1][button - 1] : false;
  }

  /// Call this to add a 'joystick button down/up' event to queue
  virtual void DoButton (uint number, uint button, bool down, 
    const int32 *axes, uint numAxes);
  /// Call this to add a 'joystick moved' event to queue
  virtual void DoMotion (uint number, const int32 *axes, uint numAxes);

  /// Application lost focus.
  virtual void LostFocus() { Reset(); }
  virtual void GainFocus() { }

};

#endif // __CS_CSINPUT_H__
