/*
  Crystal Space Windowing System: Event class interface
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

#include <stddef.h>
#include "csutil/csbase.h"

/// Windowing System Events
enum
{
  csevNothing = 0,
  csevKeyDown,                    // A key has been pressed
  csevKeyUp,                      // A key has been released
  csevMouseMove,                  // Mouse has been moved
  csevMouseDown,                  // A mouse button has been pressed
  csevMouseUp,                    // A mouse button has been released
  csevMouseDoubleClick,           // A mouse button has been clicked twice
  csevCommand,                    // Somebody(-thing) sent a command
  csevBroadcast                   // Somebody(-thing) sent a broadcast command
};

// Some handy macros
#define IS_MOUSE_EVENT(e)    (((e).Type == csevMouseUp)		\
			   || ((e).Type == csevMouseDown)	\
			   || ((e).Type == csevMouseDoubleClick)\
			   || ((e).Type == csevMouseMove))
#define IS_KEYBOARD_EVENT(e) (((e).Type == csevKeyUp)		\
			   || ((e).Type == csevKeyDown))

/**
 * Predefined Windowing System Command Codes<p>
 * The list below does not contain all defined messages; these are only the
 * most general ones. Any class which defines some class-specific messages
 * should ensure that no other command is using the integer value of its
 * proprietary command codes. To avoid this as much as possible, the following
 * ranges are reserved:<p>
 * <ul>
 *   <li>0x00000000 ... 0x7FFFFFFF: Reserved for CrystalSpace Windowing System
 *       <ul>
 *         <li>0x00000000 ... 0x000000FF: Non-class specific commands
 *         <li>0x00000100 ... 0x000001FF: csWindow class messages
 *         <li>0x00000200 ... 0x000002FF: csMenu class messages
 *         <li>0x00000300 ... 0x000003FF: csTimer class messages
 *         <li>0x00000400 ... 0x000004FF: csListBox class messages
 *         <li>0x00000500 ... 0x000005FF: csButton class messages
 *         <li>0x00000600 ... 0x000006FF: csScrollBar class messages
 *         <li>0x00000700 ... 0x000007FF: csStatic class messages
 *         <li>0x00000800 ... 0x000008FF: csCheckBox class messages
 *         <li>0x00000900 ... 0x000009FF: csRadioButton class messages
 *         <li>0x00000A00 ... 0x00000AFF: csSpinBox class messages
 *       </ul>
 *   <li>0x80000000 ... 0xFFFFFFFF: Reserved for user class-specific messages
 * </ul>
 * All commands receives a input parameter in the Command.Info field of csEvent
 * object. They can reply to the message by assigning to Command.Info a value.
 * In the description of messages below they are marked by 'IN' (the value
 * is initially passed to object) and 'OUT' (the value is expected to be filled
 * in by the object) labels. If no IN or OUT labels are present, the value of
 * Command.Info is ignored. Since Command.Info is of type (void *) it should
 * be casted to appropiate type before filling/after reading.
 */
enum
{
  /**
   * No command
   */
  cscmdNothing = 0,
  /**
   * The event below causes application to quit immediately, no matter
   * which window posted the event.
   */
  cscmdQuit,
  /**
   * Broadcasted before csApp::Process () begins to process current messages
   * in application message queue.
   */
  cscmdPreProcess,
  /**
   * Broadcasted after csApp::Process () finished to process messages
   * in application message queue.
   */
  cscmdPostProcess,
  /**
   * This event is broadcasted to refresh invalidated components.
   */
  cscmdRedraw,
  /**
   * Program window changed in-focus status.
   * <pre>
   * IN: false -> window lose focus, true -> window got focus
   * </pre>
   */
  cscmdFocusChanged,
  /**
   * Query a control if it would like to be the default control in a dialog.<p>
   * The control is 'default' if it has a 'default' attribute (this is
   * control-specific, for example buttons have the CSBSTY_DEFAULT style).
   * <pre>
   * IN: NULL
   * OUT: (csComponent *) or NULL;
   * </pre>
   */
  cscmdAreYouDefault,
  /**
   * This message is sent by parent to its active child to activate
   * whatever action it does. For example, this message is sent by a
   * dialog window to its active child when user presses Enter key.
   * <pre>
   * IN: NULL
   * OUT: (csComponent *)this if successful;
   * </pre>
   */
  cscmdActivate,
  /**
   * This broadcast message is posted after system palette has been changed.
   * If class has looked up any colors in palette, it should redo it.
   */
  cscmdPaletteChanged,
  /**
   * The "hide window" command
   */
  cscmdHide,
  /**
   * The "maximize window" command
   */
  cscmdMaximize,
  /**
   * The "close window" button
   */
  cscmdClose,
  /**
   * These commands are used for message boxes. MessageBox (...) returns
   * cscmdOK, cscmdCancel and so on depending on which button user presses.
   */
  cscmdOK,
  ///
  cscmdCancel,
  ///
  cscmdAbort,
  ///
  cscmdRetry,
  ///
  cscmdIgnore
};

/// Shift key masks
/// "Shift" key mask
#define CSMASK_SHIFT		0x00000001
/// "Ctrl" key mask
#define CSMASK_CTRL		0x00000002
/// "Alt" key mask
#define CSMASK_ALT		0x00000004
/// All shift keys
#define CSMASK_ALLSHIFTS	(CSMASK_SHIFT | CSMASK_CTRL | CSMASK_ALT)
/// Key is pressed for first time?
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
#define CSKEY_BACKSPACE	'\b'
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
/// Function keys
#define CSKEY_F1		1013
#define CSKEY_F2		1014
#define CSKEY_F3		1015
#define CSKEY_F4		1016
#define CSKEY_F5		1017
#define CSKEY_F6		1018
#define CSKEY_F7		1019
#define CSKEY_F8		1020
#define CSKEY_F9		1021
#define CSKEY_F10		1022
#define CSKEY_F11		1023
#define CSKEY_F12		1024
/// The "center" key ("5" on numeric keypad)
#define CSKEY_CENTER		1025

/**
 * This class represents a windowing system event.<p>
 * Events can be generated by hardware (keyboard, mouse)
 * as well as by software. There are so much constructors of
 * this class as much different types of events exists.
 */
class csEvent : public csBase
{
public:
  int Type;
  long Time;			// Time when the event occured
  union
  {
    struct
    {
      int Code;                 // Key code
      int ShiftKeys;            // Control key state
    } Key;
    struct
    {
      int x,y;                  // Mouse coords
      int Button;               // Button number: 1-left, 2-right, 3-middle
      int ShiftKeys;            // Control key state
    } Mouse;
    struct
    {
      unsigned int Code;        // Command code
      void *Info;               // Command info
    } Command;			// to allow virtual destructors
  };

  /// Empty initializer
  csEvent () {}

  /// Create a keyboard event object
  csEvent (long iTime, int eType, int kCode, int kShiftKeys);

  /// Create a mouse event object
  csEvent (long iTime, int eType, int mx, int my, int mbutton, int mShiftKeys);

  /// Create a command event object
  csEvent (long iTime, int eType, unsigned int cCode, void *cInfo = NULL);

  /// Destroy an event object
  virtual ~csEvent ();
};

#endif // __CSEVENT_H__
