/*
    Event system related interfaces
    Written by Andrew Zabolotny <bit@eltech.ru>

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

/*
                                *WARNING*
    This file must contain only plain-C code.  Do _not_ insert C++ code.
    This file is imported by non-C++ system driver implementations.

*/

#ifndef __IUTIL_EVDEFS_H__
#define __IUTIL_EVDEFS_H__

/// System Events: take care not to define more than 32 event types
enum
{
  csevNothing = 0,
  csevKeyDown,			// A key has been pressed
  csevKeyUp,			// A key has been released
  csevMouseMove,		// Mouse has been moved
  csevMouseDown,		// A mouse button has been pressed
  csevMouseUp,			// A mouse button has been released
  csevMouseClick,		// A mouse button has been clicked
  csevMouseDoubleClick, // A mouse button has been clicked twice
  csevJoystickMove,		// A joystick axis has been moved
  csevJoystickDown,		// A joystick button has been pressed
  csevJoystickUp,		// A joystick button has been released
  csevCommand,			// Somebody(-thing) sent a command
  csevBroadcast,		// Somebody(-thing) sent a broadcast command
  csevNetwork	,		// Something has arrived on network.
  csevMouseEnter,		// The mouse has entered a component
  csevMouseExit,		// The mouse has exited a component
  csevLostFocus,        // The component has lost keyboard focus
  csevGainFocus,        // The component has gained keyboard focus
  csevGroupOff,         // A component in a group has been selected,
                        //  everyone else should go to their off state.
  csevFrameStart        // The frame is about to draw.
};

/*
 * Event masks.<p>
 * The event masks can be used by plugins to tell an event queue, via
 * iEventQueue::RegisterListener, which kinds of events they want to receive at
 * their HandleEvent() entry.  If a plugin registers to receive CSMASK_Nothing
 * events it is always called once per frame, so that plugin can do some
 * per-frame processing.
 */
/**
 * Empty event.  If a plugin registers to receive this kind of events via
 * iEventQueue::RegisterListener(plugin, CSMASK_Nothing) this has a special
 * meaning: the plugin will be called at the start of every frame and at the
 * end of every frame with an csevBroadcast event with the Event.Command.Code
 * equal to either cscmdPreProcess or cscmdPostProcess.
 */
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
/// Mouse click events
#define CSMASK_MouseClick	(1 << csevMouseClick)
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
  (CSMASK_MouseMove | CSMASK_MouseDown | CSMASK_MouseUp | CSMASK_MouseClick | \
   CSMASK_MouseDoubleClick)
/// This mask identifies any joystick event
#define CSMASK_Joystick \
  (CSMASK_JoystickMove | CSMASK_JoystickDown | CSMASK_JoystickUp)
/// This mask identifies any input evemt
#define CSMASK_Input \
  (CSMASK_Keyboard | CSMASK_Mouse | CSMASK_Joystick)

// Some handy macros
/// Check if a event is a keyboard event
#define CS_IS_KEYBOARD_EVENT(e)	((1 << (e).Type) & CSMASK_Keyboard)
/// Check if a event is a mouse event
#define CS_IS_MOUSE_EVENT(e)	((1 << (e).Type) & CSMASK_Mouse)
/// Check if a event is a joystick event
#define CS_IS_JOYSTICK_EVENT(e)	((1 << (e).Type) & CSMASK_Joystick)
/// Check if a event is any input event
#define CS_IS_INPUT_EVENT(e)	((1 << (e).Type) & CSMASK_Input)
/// Check if the event is a network event
#define CS_IS_NETWORK_EVENT(e)	((1 << (e).Type) & CSMASK_Network)

/*
 * Event flags masks.
 * Every event has a `flags' field which describes miscelaneous
 * aspects of the event. The following constants describes every
 * used bit of the `flags' field.
 */
/**
 * Ignore `true' returned from HandleEvent which says that
 * event has been processed and should not be processed anymore.
 * Normally this is set only for csevBroadcast events.
 */
#define CSEF_BROADCAST		0x00000001

/*
 * Modifier key masks.
 * Every input event (keyboard, mouse and joystick) contains a Modifiers
 * field which is a bitfield consisting of any combination of the masks
 * below. Having one in one of the bits means that the corresponding
 * modifier was pressed at the time when event happened.
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

/*
 * Control key codes.
 * Not every existing key on any existing platform is supported by
 * Crystal Space. Instead, we tried to list here all the keys that
 * are common among all platforms on which Crystal Space runs.
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
 * Event class masks.<p>
 * Every event plug should provide information about which event
 * types that may conflict with other event plugs it is able to generate.
 * The system driver checks it and if several event plugs generates
 * conflicting types events, one of them (the one with lower priority)
 * is disabled.
 */

/// Keyboard events
#define CSEVTYPE_Keyboard	0x00000001
/// Mouse events
#define CSEVTYPE_Mouse		0x00000002
/// Joystick events
#define CSEVTYPE_Joystick	0x00000004

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
 *   cscmdMyCommand3
 *   ...
 * }
 *</pre>
 */
enum
{
  /**
   * No command. Dunno really why it is needed but traditionally
   * 0 stands for nothing.
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
   * Usually this event is accompanyed by a shutdown
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
   * The help should be displayed to standard output.
   */
  cscmdCommandLineHelp,

  /**
   * This event is broadcasted by the texture manager when the SetPalette
   * method is called.
   *<pre>
   * IN: (iTextureManager *) the texture manager object
   *</pre>
   */
  cscmdPaletteChanged,

  /**
   * Broadcasted before cscmdProcess -- on every frame --
   * as Event.Command.Code of a broadcast event.
   */
  cscmdPreProcess,

  /**
   * Broadcasted every frame as Event.Command.Code of a broadcast event.
   */
  cscmdProcess,

  /**
   * Broadcasted after cscmdProcess -- on every frame --
   * as Event.Command.Code of a broadcast event.
   */
  cscmdPostProcess,

  /**
   * Broadcasted after cscmdPostProcess -- on every frame --
   * as Event.Command.Code of a broadcast event.
   */
  cscmdFinalProcess,

  /**
   * Broadcast indicating that the display canvas is not currently
   * visible to the user (such as being iconified).
   */
  cscmdCanvasHidden,

  /**
   * Broadcast indicating that the display canvas has just become
   * visible (such as being uniconified).
   */
  cscmdCanvasExposed
};

#endif // __IUTIL_EVDEFS_H__
