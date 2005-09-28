/*
    Event system related interfaces
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

/*
                                *WARNING*
    This file must contain only plain-C code.  Do _not_ insert C++ code.
    This file is imported by non-C++ system driver implementations.

*/

/**\file
 * Event system related interfaces
 */

/**
 * \addtogroup event_handling
 * @{ */

#ifndef __CS_IUTIL_EVDEFS_H__
#define __CS_IUTIL_EVDEFS_H__

/**
 * System Events
 * Take care not to define more than 32 event types
 */
typedef enum _csEventType
{
  /// Nothing happened	
  csevNothing = 0,
  /** A keyboard event (key down/up)  
        @sa csKeyEventHelper
        @sa csKeyEventType
    */
  csevKeyboard,
  /// Mouse has been moved
  csevMouseMove,		
  /// A mouse button has been pressed
  csevMouseDown,		
  /// A mouse button has been released
  csevMouseUp,			
  /// A mouse button has been clicked
  csevMouseClick,		
  /// A mouse button has been clicked twice
  csevMouseDoubleClick, 
  /// A joystick axis has been moved
  csevJoystickMove,		
  /// A joystick button has been pressed
  csevJoystickDown,		
  /// A joystick button has been released
  csevJoystickUp,		
  /**
   * Somebody(-thing) sent a command. @see csCommandEventData
   */
  csevCommand,			
  /// Somebody(-thing) sent a broadcast command
  csevBroadcast,		
  /// The mouse has entered a component
  csevMouseEnter,		
  /// The mouse has exited a component
  csevMouseExit,		
  /// The component has lost keyboard focus
  csevLostFocus,        
  /// The component has gained keyboard focus
  csevGainFocus,        
  /**
   * A component in a group has been selected,
   * everyone else should go to their off state.
   */
  csevGroupOff,         
  /// The frame is about to draw.
  csevFrameStart        
} csEventType;

/// Keyboard event type
typedef enum _csKeyEventType
{
  /// A 'key down' event
  csKeyEventTypeUp = 0,
  /// A 'key up' event
  csKeyEventTypeDown
} csKeyEventType;

/** \name Event masks
 * The event masks can be used by plugins to tell an event queue, via
 * iEventQueue::RegisterListener, which kinds of events they want to receive at
 * their HandleEvent() entry.  If a plugin registers to receive CSMASK_Nothing
 * events it is always called once per frame, so that plugin can do some
 * per-frame processing.
 * @{ */
/**
 * Event mask: Empty event.  If a plugin registers to receive this kind of
 * events via iEventQueue::RegisterListener(plugin, CSMASK_Nothing) this has a
 * special meaning: the plugin will be called at the start of every frame and
 * at the end of every frame with an csevBroadcast event with the
 * Event.Retrieve("cmdCode") equal to either cscmdPreProcess or cscmdPostProcess.
 */
#define CSMASK_Nothing		(1 << csevNothing)
/**
 * The plugin will be called at the start of every frame and at the
 * end of every frame with an csevBroadcast event with the Event.Retrieve(
 * "cmdCode") equal to cscmdPreProcess, csProcess, cscmdPostProcess, or
 * cscmdFinalProcess.
 */
#define CSMASK_FrameProcess	CSMASK_Nothing
/// Keyboard events
#define CSMASK_Keyboard		(1 << csevKeyboard)
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
/// Check if a event is a broadcast event
#define CS_IS_BROADCAST_EVENT(e) ((1 << (e).Type) & CSMASK_Broadcast)
/// Check if a event is a command event
#define CS_IS_COMMAND_EVENT(e)   ((1 << (e).Type) & CSMASK_Command)
/// Check if a event is a keyboard event
#define CS_IS_KEYBOARD_EVENT(e)	 ((1 << (e).Type) & CSMASK_Keyboard)
/// Check if a event is a mouse event
#define CS_IS_MOUSE_EVENT(e)	 ((1 << (e).Type) & CSMASK_Mouse)
/// Check if a event is a joystick event
#define CS_IS_JOYSTICK_EVENT(e)	 ((1 << (e).Type) & CSMASK_Joystick)
/// Check if a event is any input event
#define CS_IS_INPUT_EVENT(e)	 ((1 << (e).Type) & CSMASK_Input)
/** @} */

/** \name Event flags masks
 * Every event has a `flags' field which describes miscelaneous
 * aspects of the event. The following constants describes every
 * used bit of the `flags' field.
 * @{ */
/**
 * Event flag: Ignore `true' returned from HandleEvent which says that
 * event has been processed and should not be processed anymore.
 * Normally this is set only for csevBroadcast events.
 */
#define CSEF_BROADCAST		0x00000001
/** @} */

/**\name Modifier keys
 * \sa \ref Keyboard, Modifier key masks
 * @{ */
/// Modifier types
typedef enum _csKeyModifierType
{
  /// 'Shift' is held
  csKeyModifierTypeShift = 0,
  /// 'Ctrl' is held
  csKeyModifierTypeCtrl,
  /// 'Alt' is held
  csKeyModifierTypeAlt,
  /// 'CapsLock' is toggled
  csKeyModifierTypeCapsLock,
  /// 'NumLock' is toggled
  csKeyModifierTypeNumLock,
  /// 'ScrollLock' is toggled
  csKeyModifierTypeScrollLock,
  
  /**
   * \internal Can be used to get the number of defined modifier types.
   */
  csKeyModifierTypeLast
} csKeyModifierType;

/// Modifier numbers
typedef enum _csKeyModifierNumType
{
  /// The default number for a 'left' version of a key.
  csKeyModifierNumLeft = 0,
  /// The default number for a 'right' version of a key.
  csKeyModifierNumRight,
  
  /**
   * 'Magic' modifier number used if there shouldn't be distinguished between
   * multiple modifier keys of the same type.
   */
  csKeyModifierNumAny = 0x1f
} csKeyModifierNumType;

/// Flags for all currently pressed modifiers.
typedef struct _csKeyModifiers
{
  /**
   * Bitmasks for different modifiers.
   * If the modifier number <i>n</i> was pressed, the <i>n</i>th bit is set.
   * <p>
   * Example - testing for a specific modifier:
   * \code
   * csKeyModifiers m;
   * bool rightAlt = m.modifiers[csKeyModifierTypeAlt] & 
   *   (1 << csKeyModifierNumRight);
   * \endcode
   * Example - testing if any modifier of a type is pressed:
   * \code
   * csKeyModifiers m;
   * bool ctrl = m.modifiers[csKeyModifierTypeAlt] != 0;
   * \endcode
   */
  unsigned int modifiers[csKeyModifierTypeLast];
} csKeyModifiers;
/** @} */

/** \name Modifier key masks
 * csKeyEventHelper::GetModifiersBits() returns such a bitfields consisting 
 * of any combination of the masks below. Having one in one of the bits means 
 * that the corresponding modifier was pressed in the modifier state passed 
 * in.
 * @{ */
/// "Shift" key mask
#define CSMASK_SHIFT		(1 << csKeyModifierTypeShift)
/// "Ctrl" key mask
#define CSMASK_CTRL		(1 << csKeyModifierTypeCtrl)
/// "Alt" key mask
#define CSMASK_ALT		(1 << csKeyModifierTypeAlt)
/// All shift keys
#define CSMASK_ALLSHIFTS	(CSMASK_SHIFT | CSMASK_CTRL | CSMASK_ALT)

/// "CapsLock" key mask
#define CSMASK_CAPSLOCK		(1 << csKeyModifierTypeCapsLock)
/// "NumLock" key mask
#define CSMASK_NUMLOCK		(1 << csKeyModifierTypeNumLock)
/// "ScrollLock" key mask
#define CSMASK_SCROLLLOCK	(1 << csKeyModifierTypeScrollLock)
/// All modifiers, shift and lock types
#define CSMASK_ALLMODIFIERS	(CSMASK_CAPSLOCK | CSMASK_NUMLOCK | \
				 CSMASK_SCROLLLOCK | CSMASK_ALLSHIFTS)
/** @} */

/** \name Control key codes
 * Not every existing key on any existing platform is supported by
 * Crystal Space. Instead, we tried to list here all the keys that
 * are common among all platforms on which Crystal Space runs. There
 * may still be some keys that aren't supported on some platforms, tho.
 * <p>
 * Be aware that the range of the special keys has been arbitrarily, but
 * careful chosen. In particular, all special keys fall into a part of the
 * Unicode "Supplementary Private Use Area-B", so all keycodes in CS are
 * always valid Unicode codepoints.
 * @{ */
/// ESCape key
#define CSKEY_ESC			27
/// Enter key
#define CSKEY_ENTER			'\n'
/// Tab key
#define CSKEY_TAB			'\t'
/// Back-space key
#define CSKEY_BACKSPACE			'\b'
/// Space key
#define CSKEY_SPACE			' '

/// The lowest code of a special key.
#define CSKEY_SPECIAL_FIRST		0x108000
/// The highest code of a special key.
#define CSKEY_SPECIAL_LAST		0x10fffd
/// Helper macro to construct a special key code.
#define CSKEY_SPECIAL(code)		(CSKEY_SPECIAL_FIRST + (code))
/// Helper macro to determine whether a key code identifies a special key.
#define CSKEY_IS_SPECIAL(rawCode)	\
  ((rawCode >= CSKEY_SPECIAL_FIRST) && ((rawCode) <= CSKEY_SPECIAL_LAST))
/// Helper macro to determine the parameter that was given to #CSKEY_SPECIAL.
#define CSKEY_SPECIAL_NUM(rawCode)	((rawCode) - CSKEY_SPECIAL_FIRST)

/// Up arrow key
#define CSKEY_UP			CSKEY_SPECIAL(0x00)
/// Down arrow key
#define CSKEY_DOWN			CSKEY_SPECIAL(0x01)
/// Left arrow key
#define CSKEY_LEFT			CSKEY_SPECIAL(0x02)
/// Right arrow key
#define CSKEY_RIGHT			CSKEY_SPECIAL(0x03)
/// PageUp key
#define CSKEY_PGUP			CSKEY_SPECIAL(0x04)
/// PageDown key
#define CSKEY_PGDN			CSKEY_SPECIAL(0x05)
/// Home key
#define CSKEY_HOME			CSKEY_SPECIAL(0x06)
/// End key
#define CSKEY_END			CSKEY_SPECIAL(0x07)
/// Insert key
#define CSKEY_INS			CSKEY_SPECIAL(0x08)
/// Delete key
#define CSKEY_DEL			CSKEY_SPECIAL(0x09)
/// The "Context menu" key on Windows keyboards
#define CSKEY_CONTEXT			CSKEY_SPECIAL(0x0a)
/// The Print Screen key
#define CSKEY_PRINTSCREEN		CSKEY_SPECIAL(0x0b)
/// The Pause key
#define CSKEY_PAUSE			CSKEY_SPECIAL(0x0c)
/// Function key F1
#define CSKEY_F1			CSKEY_SPECIAL(0x10)
/// Function key F2
#define CSKEY_F2			CSKEY_SPECIAL(0x11)
/// Function key F3
#define CSKEY_F3			CSKEY_SPECIAL(0x12)
/// Function key F4
#define CSKEY_F4			CSKEY_SPECIAL(0x13)
/// Function key F5
#define CSKEY_F5			CSKEY_SPECIAL(0x14)
/// Function key F6
#define CSKEY_F6			CSKEY_SPECIAL(0x15)
/// Function key F7
#define CSKEY_F7			CSKEY_SPECIAL(0x16)
/// Function key F8
#define CSKEY_F8			CSKEY_SPECIAL(0x17)
/// Function key F9
#define CSKEY_F9			CSKEY_SPECIAL(0x18)
/// Function key F10
#define CSKEY_F10			CSKEY_SPECIAL(0x19)
/// Function key F11
#define CSKEY_F11			CSKEY_SPECIAL(0x1a)
/// Function key F12
#define CSKEY_F12			CSKEY_SPECIAL(0x1b)

/// The lowest code of a modifier key.
#define CSKEY_MODIFIER_FIRST		0x2000
/// The highest code of a modifier key.
#define CSKEY_MODIFIER_LAST		0x3fff
/**
 * \internal How many bits to distinguish between modifier keys of the same 
 * type?
 */
#define CSKEY_MODIFIERTYPE_SHIFT	5
/// Helper macro to construct a modifiers key code.
#define CSKEY_MODIFIER(type, num)		\
  CSKEY_SPECIAL(CSKEY_MODIFIER_FIRST + ((type) << CSKEY_MODIFIERTYPE_SHIFT) + (num))
/// Helper macro to test whether a key code identifies a modifier.
#define CSKEY_IS_MODIFIER(rawCode)	\
  (CSKEY_IS_SPECIAL(rawCode) && 	\
    ((CSKEY_SPECIAL_NUM(rawCode) >= CSKEY_MODIFIER_FIRST) && \
     (CSKEY_SPECIAL_NUM(rawCode) <= CSKEY_MODIFIER_LAST)))
/// Helper macro to determine the modifier type of a key code.
#define CSKEY_MODIFIER_TYPE(rawCode)	\
  (((rawCode) - CSKEY_MODIFIER_FIRST - CSKEY_SPECIAL_FIRST) >> \
  CSKEY_MODIFIERTYPE_SHIFT)
/// Helper macro to determine the modifier number of a key code.
#define CSKEY_MODIFIER_NUM(rawCode)	\
  (((rawCode) - CSKEY_MODIFIER_FIRST - CSKEY_SPECIAL_FIRST) & \
  ((1 << CSKEY_MODIFIERTYPE_SHIFT) - 1))  

/// Helper macro to determine if two modifier bitmasks can be considered equal.
#define CSKEY_MODIFIER_COMPARE(bitmask1, bitmask2)			     \
  ((bitmask1) == (bitmask2)						     \
|| ((bitmask1) == (unsigned)(1 << csKeyModifierNumAny) && (bitmask2) != 0)   \
|| ((bitmask2) == (unsigned)(1 << csKeyModifierNumAny) && (bitmask1) != 0))

/// Construct a key code for the Shift modifier key number \a n.
#define CSKEY_SHIFT_NUM(n)		CSKEY_MODIFIER(csKeyModifierTypeShift,n)
/// Lowest code of the Shift modifier keys
#define CSKEY_SHIFT_FIRST		CSKEY_SHIFT_NUM(0)
/// Highest code of the Shift modifier keys
#define CSKEY_SHIFT_LAST		CSKEY_SHIFT_NUM(0x1e)

/// Left Shift
#define CSKEY_SHIFT_LEFT		CSKEY_SHIFT_NUM(csKeyModifierNumLeft)
/// Right Shift
#define CSKEY_SHIFT_RIGHT		CSKEY_SHIFT_NUM(csKeyModifierNumRight)
/// Undistinguished Shift
#define CSKEY_SHIFT			CSKEY_SHIFT_NUM(csKeyModifierNumAny)

/// Construct a key code for the Ctrl modifier key number \a n.
#define CSKEY_CTRL_NUM(n)		CSKEY_MODIFIER(csKeyModifierTypeCtrl,n)
/// Lowest code of the Ctrl modifier keys
#define CSKEY_CTRL_FIRST		CSKEY_CTRL_NUM(0)
/// Highest code of the Ctrl modifier keys
#define CSKEY_CTRL_LAST			CSKEY_CTRL_NUM(0x1e)

/// Left Ctrl
#define CSKEY_CTRL_LEFT			CSKEY_CTRL_NUM(csKeyModifierNumLeft)
/// Right Ctrl
#define CSKEY_CTRL_RIGHT		CSKEY_CTRL_NUM(csKeyModifierNumRight)
/// Undistinguished Ctrl
#define CSKEY_CTRL			CSKEY_CTRL_NUM(csKeyModifierNumAny)

/// Construct a key code for the Alt modifier key number \a n.
#define CSKEY_ALT_NUM(n)		CSKEY_MODIFIER(csKeyModifierTypeAlt,n)
/// Lowest code of the Alt modifier keys
#define CSKEY_ALT_FIRST			CSKEY_ALT_NUM(0)
/// Highest code of the Alt modifier keys
#define CSKEY_ALT_LAST			CSKEY_ALT_NUM(0x1e)

/// Left Alt
#define CSKEY_ALT_LEFT			CSKEY_ALT_NUM(csKeyModifierNumLeft)
/// Right Alt
#define CSKEY_ALT_RIGHT			CSKEY_ALT_NUM(csKeyModifierNumRight)
/// Undistinguished Alt
#define CSKEY_ALT			CSKEY_ALT_NUM(csKeyModifierNumAny)

/// Bit that is set if a key is from the keypad.
#define CSKEY_PAD_FLAG			0x4000
/// Helper macro to construct a keypade key code.
#define CSKEY_PAD_KEY(code) CSKEY_SPECIAL((unsigned int)(code) | CSKEY_PAD_FLAG)

/// Helper macro to test whether a key code identifies a keypad key.
#define CSKEY_IS_PAD_KEY(rawCode)	(((rawCode) & CSKEY_PAD_FLAG) != 0)
/**
 * Helper macro to convert a 'pad' key code into a 'normal' special key code.
 */
#define CSKEY_PAD_TO_NORMAL(rawCode)	((rawCode) & (~CSKEY_PAD_FLAG))

/// Keypad 1
#define CSKEY_PAD1			CSKEY_PAD_KEY('1')
/// Keypad 2
#define CSKEY_PAD2			CSKEY_PAD_KEY('2')
/// Keypad 3
#define CSKEY_PAD3			CSKEY_PAD_KEY('3')
/// Keypad 4
#define CSKEY_PAD4			CSKEY_PAD_KEY('4')
/// Keypad 5
#define CSKEY_PAD5			CSKEY_PAD_KEY('5')
/// Keypad "Center" (5)
#define CSKEY_CENTER			CSKEY_PAD5
/// Keypad 6
#define CSKEY_PAD6			CSKEY_PAD_KEY('6')
/// Keypad 7
#define CSKEY_PAD7			CSKEY_PAD_KEY('7')
/// Keypad 8
#define CSKEY_PAD8			CSKEY_PAD_KEY('8')
/// Keypad 9
#define CSKEY_PAD9			CSKEY_PAD_KEY('9')
/// Keypad 0
#define CSKEY_PAD0			CSKEY_PAD_KEY('0')
/// Keypad Decimal ('.' on English keyboards)
#define CSKEY_PADDECIMAL		CSKEY_PAD_KEY('.')
/// Keypad Divide
#define CSKEY_PADDIV			CSKEY_PAD_KEY('/')
/// Keypad Multiply
#define CSKEY_PADMULT			CSKEY_PAD_KEY('*')
/// Keypad Minus
#define CSKEY_PADMINUS			CSKEY_PAD_KEY('-')
/// Keypad Plus
#define CSKEY_PADPLUS			CSKEY_PAD_KEY('+')
/// Keypad Enter
#define CSKEY_PADENTER			CSKEY_PAD_KEY('\n')

/**
 * NumLock key.
 * Both a modifier and a keypad key.
 */
#define CSKEY_PADNUM \
  CSKEY_MODIFIER(csKeyModifierTypeNumLock,csKeyModifierNumAny) | CSKEY_PAD_FLAG
/// CapsLock key
#define CSKEY_CAPSLOCK \
  CSKEY_MODIFIER(csKeyModifierTypeCapsLock,csKeyModifierNumAny)
/// ScrollLock key
#define CSKEY_SCROLLLOCK \
  CSKEY_MODIFIER(csKeyModifierTypeScrollLock,csKeyModifierNumAny)

/// Character types
typedef enum _csKeyCharType
{
  /// Normal character
  csKeyCharTypeNormal = 0,
  /// "Dead" character
  csKeyCharTypeDead
} csKeyCharType;

/** @} */

/** \name Event class masks
 * Every event plug should provide information about which event
 * types that may conflict with other event plugs it is able to generate.
 * The system driver checks it and if several event plugs generates
 * conflicting types events, one of them (the one with lower priority)
 * is disabled.
 * @{ */
/// Keyboard events
#define CSEVTYPE_Keyboard	0x00000001
/// Mouse events
#define CSEVTYPE_Mouse		0x00000002
/// Joystick events
#define CSEVTYPE_Joystick	0x00000004
/** @} */

/**
 * General Command Codes<p>
 * The list below does not contain all defined command codes; these are only
 * the most general ones.
 *<p>
 * Third-party applications should use command codes greater than these
 * standard code, for example:
 *<pre>
 * enum
 * {
 *   cscmdMyCommand1 = 0x10000000,
 *   cscmdMyCommand2,
 *   cscmdMyCommand3
 *   ...
 * }
 *</pre>
 */
typedef enum _csCommandEventCode
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
   * This event is generated whenever user application receives/loses focus
   * (that is, when the application itself activates or deactivates).
   * Upon this event the application may react correspondingly - stop music,
   * hide software mouse cursor and so on. iEnable = true in the event
   * application receives focus and false if it loses focus.
   * <pre>
   * Info: NULL -> window lose focus, non-NULL -> window got focus
   * </pre>
   */
  cscmdFocusChanged,

  /**
   * This event is broadcasted to all event listeners just after all modules
   * have been initialized.
   */
  cscmdSystemOpen,

  /**
   * This event is broadcasted to all event listeners just before modules are
   * about to be shutdown and unloaded (that is, the system is shutting down).
   */
  cscmdSystemClose,

  /**
   * This event is generated when user resizes the application window.
   * The argument points to the graphics context that has been resized.
   * <pre>
   * Info: (iGraphics2D *) The context that has been resized
   * </pre>
   */
  cscmdContextResize,

  /**
   * This event is sent when a graphics context is being destroyed.
   * Usually this event is accompanyed by a shutdown
   * but there is one exception: when a dynamic texture is closed
   * (a dynamic texture is a graphics context as well).
   * <pre>
   * Info: (iGraphics2D *) The context that has been closed
   * </pre>
   */
  cscmdContextClose,

  /**
   * This event is broadcasted when system driver displays the
   * help text for all supported command-line switches. Upon reception
   * of such event every plugin should display a short help for any
   * of the command-line switches it supports. The general format is:
   * <pre>&lt;2 spaces&gt;&lt;switch - 18 positions&gt;&lt;space&gt;</pre>
   * <pre>&lt;switch description&gt;{default value}</pre>
   * The help should be displayed to standard output.
   */
  cscmdCommandLineHelp,

  /**
   * Broadcasted before cscmdProcess -- on every frame --
   * as Event.Retrieve("cmdCode") of a broadcast event.  Use the event mask
   * CSMASK_FrameProcess to receive this pseudo-event.
   */
  cscmdPreProcess,

  /**
   * Broadcasted every frame as Event.Retrieve("cmdCode") of a broadcast event.
   * Use the event mask CSMASK_FrameProcess to receive this pseudo-event.
   */
  cscmdProcess,

  /**
   * Broadcasted after cscmdProcess -- on every frame --
   * as Event.Retrieve("cmdCode") of a broadcast event.  Use the event mask
   * CSMASK_FrameProcess to receive this pseudo-event.
   */
  cscmdPostProcess,

  /**
   * Broadcasted after cscmdPostProcess -- on every frame --
   * as Event.Retrieve("cmdCode") of a broadcast event.  Use the event mask
   * CSMASK_FrameProcess to receive this pseudo-event.
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
} csCommandEventCode;

/** @} */

#endif // __CS_IUTIL_EVDEFS_H__
