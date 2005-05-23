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

#ifndef __CS_IUTIL_STDINPUT_H__
#define __CS_IUTIL_STDINPUT_H__

/**\file 
 * Crystal Space input library.
 * These are the low-level interfaces to generic classes of input devices like
 * keyboard, mouse, and joystick.  System-dependent code should inherit
 * system-specific classes from those defined below, implementing as much
 * functionality as possible.
 */

/**
 * \addtogroup event_handling
 * @{ */

#include "csutil/scf.h"
#include "iutil/event.h"
#include "csutil/csunicode.h"

/**
 * Results for attempts to process a character key.
 */
enum csKeyComposeResult
{
  /// No character could be retrieved. Possibly the key was dead.
  csComposeNoChar = -1,
  /// A single, normal chararacter is retrieved.
  csComposeNormalChar,
  /// A single, composed chararacter is retrieved.
  csComposeComposedChar,
  /**
   * A key couldn't be combined with a previously pressed dead key,
   * and both characters are returned individually.
   */
  csComposeUncomposeable
};

SCF_VERSION(iKeyComposer, 0, 0, 1);

/**
 * Keyboard input handler.
 */
struct iKeyComposer : public iBase
{
  /**
   * Handle keyboard input.
   * Converts the input to characters, if possible. If the key passed in is a 
   * dead key, it will be stored internally and affect the returned data of the
   * subsequent keypress.
   * \param keyEventData Information from a keyboard event.
   * \param buf Buffer to store the output in. Should be at least contain 2
   *  characters (however, the method will work with smaller buffers as well.)
   * \param bufChars Number of characters the output buffer is actually sized.
   * \param resultChars If not 0, returns the number of characters written to
   *  the output buffer.
   * \return The type of character(s) that has been written to the output
   *  buffer.
   */
  virtual csKeyComposeResult HandleKey (const csKeyEventData& keyEventData,
    utf32_char* buf, size_t bufChars, int* resultChars = 0) = 0;
  /**
   * Reset the composer's internal state.
   * Specifically, it will clear any stored dead key - the next key won't be
   * combined with it.
   */
  virtual void ResetState () = 0;
};

SCF_VERSION(iKeyboardDriver, 0, 0, 2);

/**
 * Generic Keyboard Driver.<p>
 * Keyboard driver listens for keyboard-related events from the event queue,
 * stores state about the keyboard, and possibly synthesizes additional events,
 * such as when a character is "composed".  Typically, one instance of this
 * object is available from the shared-object registry (iObjectRegistry) under
 * the name "crystalspace.driver.input.generic.keyboard".
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>csInitializer::CreateEnvironment()
 *   <li>csInitializer::CreateInputDrivers()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>CS_QUERY_REGISTRY()
 *   </ul>
 * 
 * \todo Need a simple way to query all currently-set modifiers for event 
 *   construction.
 * \see csMouseDriver::DoButton() 
 * \see csMouseDriver::DoMotion() 
 * \see csJoystickDriver::DoButton() 
 * \see csJoystickDriver::DoMotion() 
 */
struct iKeyboardDriver : public iBase
{
  /**
   * Call to release all key down flags (when focus switches from application
   * window, for example).
   */
  virtual void Reset () = 0;

  /**
   * Call this routine to add a key down/up event to queue.
   * \param codeRaw 'Raw' code of the pressed key.
   * \param codeCooked 'Cooked' code of the pressed key.
   * \param iDown Whether the key is up or down.
   * \param autoRepeat Auto-repeat flag for the key event. Typically only
   *  used by the platform-specific keyboard agents.
   * \param charType When the cooked code is a character, it determines
   *  whether it is a normal, or dead character.
   */
  virtual void DoKey (utf32_char codeRaw, utf32_char codeCooked, bool iDown,
    bool autoRepeat = false, csKeyCharType charType = csKeyCharTypeNormal) = 0;

  /**
   * Query the state of a key. All key codes are supported. Returns true if
   * the key is pressed, false if not.
   */
  virtual bool GetKeyState (utf32_char codeRaw) const = 0;

  /**
   * Get the current state of the modifiers.
   */
  virtual uint32 GetModifierState (utf32_char codeRaw) const = 0;

  /**
   * Return an instance of the keyboard composer.
   * \remark All composers are independent. Specifically, passing a dead key
   * to one composer won't affect the result after the next keyboard event
   * of any other composer.
   */
  virtual csPtr<iKeyComposer> CreateKeyComposer () = 0;

  /**
   * For an event that contains only a raw code, this adds cooked code and
   * modifiers.
   */
  virtual csEventError SynthesizeCooked (iEvent *) = 0;
};

SCF_VERSION(iMouseDriver, 0, 0, 1);

/**
 * Generic Mouse Driver.<p>
 * The mouse driver listens for mouse-related events from the event queue and
 * records state information about recent events.  It is responsible for
 * synthesizing double-click events when it detects that two mouse-down events
 * have occurred for the same mouse button within a short interval.  Mouse
 * button numbers start at 1.  The left mouse button is 1, the right is 2, the
 * middle 3, and so on.  Typically, one instance of this object is available
 * from the shared-object registry (iObjectRegistry) under the name
 * "crystalspace.driver.input.generic.mouse".
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>csInitializer::CreateEnvironment()
 *   <li>csInitializer::CreateInputDrivers()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>CS_QUERY_REGISTRY()
 *   </ul>
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

  /// Query last mouse X position for mouse number (1, 2, ...)
  virtual int GetLastX (int number) const = 0;
  virtual int GetLastX () const = 0;
  /// Query last mouse Y position
  virtual int GetLastY (int number) const = 0;
  virtual int GetLastY () const = 0;
  /// Query last mouse position for mouse n on axis a
  virtual int GetLast (int n, uint8 a) const = 0;
  /// Query last mouse position for mouse n
  virtual const int *GetLast (int n) const = 0;
  /// Query the last known mouse button state. Button numbers start at 1.
  virtual bool GetLastButton (int number, int button) const = 0;
  virtual bool GetLastButton (int button) const = 0;

  /**
   * Call this to add a 'mouse button down/up' event to queue. Button numbers
   * start at one.
   */
  virtual void DoButton (int number, int button, bool down, const int *axes, uint8 numAxes) = 0;
  virtual void DoButton (int button, bool down, int x, int y) = 0;
  /// Call this to add a 'mouse moved' event to queue
  virtual void DoMotion (int number, const int *axes, uint8 numAxes) = 0;
  virtual void DoMotion (int x, int y) = 0;
};

SCF_VERSION(iJoystickDriver, 0, 0, 1);

/**
 * Generic Joystick driver.<p>
 * The joystick driver is responsible for tracking current joystick state and
 * also for synthesizing joystick movement events.  Multiple joysticks are
 * supported; they are numbered starting at one.  Joystick button numbers also
 * start at one.  Typically, a single instance of this object is available from
 * the shared-object registry (iObjectRegistry) under the name
 * "crystalspace.driver.input.generic.joystick".
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>csInitializer::CreateEnvironment()
 *   <li>csInitializer::CreateInputDrivers()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>CS_QUERY_REGISTRY()
 *   </ul>
 */
struct iJoystickDriver : public iBase
{
  /**
   * Call to release all joystick buttons (when focus switches from application
   * window, for example).
   */
  virtual void Reset () = 0;

  /// Query last X position of joystick 'number'.
  CS_DEPRECATED_METHOD virtual int GetLastX (int number) const = 0;
  /// Query last Y position of joystick 'number'.
  CS_DEPRECATED_METHOD virtual int GetLastY (int number) const = 0;
  /// Query last position on all axes of joystick 'number'.
  virtual const int *GetLast (int number) const = 0;
  /// Query last position on 'axis' of joystick 'number'.
  virtual int GetLast (int number, uint8 axis) const = 0;

  /**
   * Query the last known button state of joystick 'number'.  Joystick numbers
   * start at 1.  Button numbers start at 1.
   */
  virtual bool GetLastButton (int number, int button) const = 0;

  /**
   * Call this to add a 'button down/up' event to queue.  Joystick
   * numbers start at 1.  Button numbers start at 1.
   */
  virtual void DoButton (int number, int button, bool down, 
    const int *axes, uint8 numAxes) = 0;
  /// Call this to add a 'moved' event to queue for joystick 'number'.
  virtual void DoMotion (int number, const int *axes, uint8 nunmAxes) = 0;
};

/** @} */

#endif // __CS_IUTIL_STDINPUT_H__
