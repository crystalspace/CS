/*
  Crystal Space Windowing System: Event class constants
  Copyright (C) 1999 by Jorrit Tyberghein
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

#ifndef __CSEVDEFS_H__
#define __CSEVDEFS_H__

// IMPORTANT: This file provides event-system constants.  Please keep it pure.
// Do not include any COM headers in this interface.  Do not include any
// other headers which include COM headers.  COM headers break the MacOS/X
// Server, OpenStep, and NextStep ports.

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
 * Predefined Command Codes<p>
 * The list below does not contain all defined messages; these are only the
 * most general ones. Crystal Space Windowing System has a broad range of
 * predefined commands; look in CSWS header files for more info.
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
   * Application has changed its "focused" status.
   * This command is posted (or is not posted) by system-dependent driver.
   * <pre>
   * IN: false -> window lose focus, true -> window got focus
   * </pre>
   */
  cscmdFocusChanged,
};

#endif // __CSEVDEFS_H__
