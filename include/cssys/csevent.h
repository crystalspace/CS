/*
    Crystal Space 3D engine: Event class interface
    Copyright (C) 1998 by Jorrit Tyberghein
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

#ifndef __CSEVENT_H__
#define __CSEVENT_H__

#include "csutil/csbase.h"

/// System Events: take care to not define more than 32 event types
enum
{
  csevNothing = 0,
  csevKeyDown,			// A key has been pressed
  csevKeyUp,			// A key has been released
  csevMouseMove,		// Mouse has been moved
  csevMouseDown,		// A mouse button has been pressed
  csevMouseUp,			// A mouse button has been released
  csevMouseDoubleClick,		// A mouse button has been clicked twice
  csevJoystickMove,		// A joystick axis has been moved
  csevJoystickDown,		// A joystick button has been pressed
  csevJoystickUp,		// A joystick button has been released
  csevCommand,			// Somebody(-thing) sent a command
  csevBroadcast,		// Somebody(-thing) sent a broadcast command
  csevNetwork			// Something has arrived on network.
};

/**
 * Event masks.<p>
 * The event masks can be used by plugins to tell the system driver
 * (via iSystem::CallOnEvents) which kinds of events they want to receive
 * at their HandleEvent() entry. If a plugin registers to receive
 * CSMASK_Nothing events it is always called once per frame,
 * so that plugin can do some per-frame processing.
 */
/// Unused event (plugins will receive control once per frame)
#define CSMASK_Nothing		(1 << csevNothing)
/// Key down events
#define CSMASK_KeyDown		(1 << csevKeyDown)
/// Key up events
#define CSMASK_KeyUp		(1 << csevKeyUp)
/// Mouse move events
#define CSMASK_MouseMove	(1 << csevMouseMove)
/// Mouse down events
#define CSMASK_MouseDown	(1 << csevMouseDown)
/// Mouse up events
#define CSMASK_MouseUp		(1 << csevMouseUp)
/// Mouse double click events
#define CSMASK_MouseDoubleClick	(1 << csevMouseDoubleClick)
/// Joystick movement events
#define CSMASK_JoystickMove	(1 << csevJoystickMove)
/// Joystick button down events
#define CSMASK_JoystickDown	(1 << csevJoystickDown)
/// Joystick button up events
#define CSMASK_JoystickUp	(1 << csevJoystickUp)
/// Command message events
#define CSMASK_Command		(1 << csevCommand)
/// Broadcast message events
#define CSMASK_Broadcast	(1 << csevBroadcast)
/// Network message events
#define CSMASK_Network		(1 << csevNetwork)

/// This mask identifies any keyboard event
#define CSMASK_Keyboard \
  (CSMASK_KeyDown | CSMASK_KeyUp)
/// This mask identifies any mouse event
#define CSMASK_Mouse \
  (CSMASK_MouseMove | CSMASK_MouseDown | CSMASK_MouseUp | CSMASK_MouseDoubleClick)
/// This mask identifies any joystick event
#define CSMASK_Joystick \
  (CSMASK_JoystickMove | CSMASK_JoystickDown | CSMASK_JoystickUp)
/// This mask identifies any input evemt
#define CSMASK_Input \
  (CSMASK_Keyboard | CSMASK_Mouse | CSMASK_Joystick)

/// Some handy macros
/// Check if a event is a keyboard event
#define IS_KEYBOARD_EVENT(e)	((1 << (e).Type) & CSMASK_Keyboard)
/// Check if a event is a mouse event
#define IS_MOUSE_EVENT(e)	((1 << (e).Type) & CSMASK_Mouse)
/// Check if a event is a joystick event
#define IS_JOYSTICK_EVENT(e)	((1 << (e).Type) & CSMASK_Joystick)
/// Check if the event is a network event
#define IS_NETWORK_EVENT(e)	((1 << (e).Type) & CSMASK_Network)

/**
 * Modifier key masks
 */
/// "Shift" key mask
#define CSMASK_SHIFT		0x00000001
/// "Ctrl" key mask
#define CSMASK_CTRL		0x00000002
/// "Alt" key mask
#define CSMASK_ALT		0x00000004
/// All shift keys
#define CSMASK_ALLSHIFTS	(CSMASK_SHIFT | CSMASK_CTRL | CSMASK_ALT)
/// Key is pressed for first time or it is an autorepeat?
#define CSMASK_FIRST		0x80000000

/**
 * Control key codes
 */
/// ESCape key
#define CSKEY_ESC		27
/// Enter key
#define CSKEY_ENTER		'\n'
/// Tab key
#define CSKEY_TAB		'\t'
/// Back-space key
#define CSKEY_BACKSPACE		'\b'
/// Space key
#define CSKEY_SPACE		' '
/// Up arrow key
#define CSKEY_UP		1000
/// Down arrow key
#define CSKEY_DOWN		1001
/// Left arrow key
#define CSKEY_LEFT		1002
/// Right arrow key
#define CSKEY_RIGHT		1003
/// PageUp key
#define CSKEY_PGUP		1004
/// PageDown key
#define CSKEY_PGDN		1005
/// Home key
#define CSKEY_HOME		1006
/// End key
#define CSKEY_END		1007
/// Insert key
#define CSKEY_INS		1008
/// Delete key
#define CSKEY_DEL		1009
/// Control key
#define CSKEY_CTRL		1010
/// Alternative shift key
#define CSKEY_ALT		1011
/// Shift key
#define CSKEY_SHIFT		1012
/// Function key F1
#define CSKEY_F1		1013
/// Function key F2
#define CSKEY_F2		1014
/// Function key F3
#define CSKEY_F3		1015
/// Function key F4
#define CSKEY_F4		1016
/// Function key F5
#define CSKEY_F5		1017
/// Function key F6
#define CSKEY_F6		1018
/// Function key F7
#define CSKEY_F7		1019
/// Function key F8
#define CSKEY_F8		1020
/// Function key F9
#define CSKEY_F9		1021
/// Function key F10
#define CSKEY_F10		1022
/// Function key F11
#define CSKEY_F11		1023
/// Function key F12
#define CSKEY_F12		1024
/// The "center" key ("5" on numeric keypad)
#define CSKEY_CENTER		1025
/// Numeric keypad '+'
#define CSKEY_PADPLUS		1026
/// Numeric keypad '-'
#define CSKEY_PADMINUS		1027
/// Numeric keypad '*'
#define CSKEY_PADMULT		1028
/// Numeric keypad '/'
#define CSKEY_PADDIV		1029

// First and last control key code
#define CSKEY_FIRST		1000
#define CSKEY_LAST		1029

/**
 * General Command Codes<p>
 * The list below does not contain all defined command codes; these are only
 * the most general ones. Crystal Space Windowing System has a broad range of
 * predefined commands; look in CSWS header files for more info.
 *<p>
 * Basically CSWS uses command codes below 0x80000000; user applications
 * should use command codes above this number, for example:
 *<pre>
 * enum
 * {
 *   cscmdMyCommand1 = 0xdeadf00d,
 *   cscmdMyCommand2,
 *   cscmdMyCommand3,
 *   ...
 * }
 *</pre>
 */
enum
{
  /**
   * No command.
   */
  cscmdNothing = 0,
  /**
   * The event below causes system driver to quit the event loop,
   * even if the event loop has been entered multiple times. The
   * quit condition is never reset thus even if you call ::Loop
   * again, the system driver will quit immediately. Such
   * broadcasts are posted for irreversible finalization of the
   * application such as when user closes the application window.
   */
  cscmdQuit,
  /**
   * The following event is very similar to cscmdQuit except that
   * it causes system driver to quit just one level of the event loop,
   * if it was called recursively. Before exiting ::Loop the quit
   * condition is reset, thus you may call ::Loop later again.
   */
  cscmdQuitLoop,
  /**
   * Application has changed its "focused" status.<p>
   * This command is posted (or is not posted) by system-dependent driver.
   * This event is generated whenever user application receives/loses focus.
   * Upon this event application may react correspondingly - stop music,
   * hide software mouse cursor and so on. iEnable = true in the event
   * application receives focus and false if it loses focus.
   * <pre>
   * IN: false -> window lose focus, true -> window got focus
   * </pre>
   */
  cscmdFocusChanged,
  /**
   * This event is broadcasted to all plugins inside csSystemDriver::Open
   * right after all drivers were initialized and opened.
   */
  cscmdSystemOpen,
  /**
   * This event is broadcasted to all plugins inside csSystemDriver::Close
   * right before starting to close all drivers.
   */
  cscmdSystemClose,
  /**
   * This event is generated when user resizes the application window.
   * The argument points to the graphics context that has been resized.
   * <pre>
   * IN: (iGraphics2D *) The context that has been resized
   * </pre>
   */
  cscmdContextResize,
  /**
   * This event is sent when a graphics context is being destroyed.
   * Usually this event is accompanyed by an iSystem->StartShutdown()
   * but there is one exception: when a dynamic texture is closed
   * (a dynamic texture is a graphics context as well).
   * <pre>
   * IN: (iGraphics2D *) The context that has been closed
   * </pre>
   */
  cscmdContextClose,
  /**
   * This event is broadcasted when system driver displays the
   * help text for all supported command-line switches. Upon reception
   * of such event every plugin should display a short help for any
   * of the command-line switches it supports. The general format is:
   *<pre>
   * <2 spaces><switch - 18 positions><space><switch description>{default value}
   *</pre>
   * The help should be displayed through iSystem::Printf (MSG_STDOUT, ...).
   */
  cscmdCommandLineHelp,
  /**
   * This message is sent to the client associated with a console
   * when console visibility status changes.
   *<pre>
   * IN: (iConsole *) the console object
   *</pre>
   */
  cscmdConsoleStatusChange,
  /**
   * This event is broadcasted by the texture manaher when the SetPalette
   * method is called.
   *<pre>
   * IN: (iTextureManager *) the texture manager object
   *</pre>
   */
  cscmdPaletteChanged,
  /**
   * Broadcasted before iSystem::NextFrame begins to process current
   * messages -- on every frame -- as Event.Command.Code of a broadcast event.
   */
  cscmdPreProcess,
  /**
   * Broadcasted after csApp::Process () finished to process all messages
   * in the system message queue -- on every frame -- as Event.Command.Code
   * of a broadcast event.
   */
  cscmdPostProcess
};

/**
 * This class represents a system event.<p>
 * Events can be generated by hardware (keyboard, mouse)
 * as well as by software. There are so much constructors of
 * this class as much different types of events exists.
 *<p>
 * This class cannot be subclassed. The problem is that if you
 * allocate an event object in one plugin (say in canvas driver)
 * and destroy it in another plugin (like in system driver) you
 * will simply crash in some environments (where shared libraries
 * have separate memory heaps). Thus the only way to extend this
 * class is to add the appropiate csevXXX identifier for the new
 * events and add appropiate sub-structures to the unnamed union
 * inside the event class.
 *<p>
 * Another reason is that if you subclass the event class, you
 * will need lots of ugly typedefs anywhere you receive derived
 * types of events.
 *<p>
 * So here is the rule of thumb: destroy the event in the same
 * module where you have allocated it. For the events that are
 * put into the system queue this means you have to allocate them
 * inside the system driver (this is possible through the
 * iEventOutlet interface, see ievent.h). For other events
 * (such as those used in point-to-point communications where you
 * pass an csEvent object directly through HandleEvent method of
 * iPlugIn) you have to destroy them in same place where you
 * allocated them.
 */
class csEvent
{
public:
  unsigned char Type;		// Event type (one of csevXXX)
  unsigned char Category;	// Event category (unused by CSWS)
  unsigned char SubCategory;	// Finer granularity
  unsigned char UnusedField;	// Reserved for Great Future Extensions
  time_t Time;			// Time when the event occured
  union
  {
    struct
    {
      int Code;			// Key code
      int Char;			// Character code
      int Modifiers;		// Control key state
    } Key;
    struct
    {
      int x,y;			// Mouse coords
      int Button;		// Button number: 1-left, 2-right, 3-middle
      int Modifiers;		// Control key state
    } Mouse;
    struct
    {
      int number;		// Joystick number (1, 2, ...)
      int x, y;			// Joystick x, y
      int Button;		// Joystick button number
      int Modifiers;		// Control key state
    } Joystick;
    struct
    {
      unsigned int Code;	// Command code
      void *Info;		// Command info
    } Command;
  };

  /// Empty initializer
  csEvent () {}

  /// Create a keyboard event object
  csEvent (time_t eTime, int eType, int kCode, int kChar, int kModifiers);

  /// Create a mouse event object
  csEvent (time_t eTime, int eType, int mx, int my, int mButton, int mModifiers);

  /// Create a joystick event object
  csEvent (time_t eTime, int eType, int jn, int jx, int jy, int jButton, int jModifiers);

  /// Create a command event object
  csEvent (time_t eTime, int eType, int cCode, void *cInfo = NULL);
};

#endif // __CSEVENT_H__
