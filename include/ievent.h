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

#ifndef __IEVENT_H__
#define __IEVENT_H__

#include "csutil/scf.h"

struct iPlugIn;

/// System Events: take care not to define more than 32 event types
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

/*
 * Event masks.<p>
 * The event masks can be used by plugins to tell the system driver
 * (via iSystem::CallOnEvents) which kinds of events they want to receive
 * at their HandleEvent() entry. If a plugin registers to receive
 * CSMASK_Nothing events it is always called once per frame,
 * so that plugin can do some per-frame processing.
 */
/**
 * Empty event. If a plugin registers to receive this kind of events
 * (iSystem::CallOnEvents (CSMASK_Nothing, ...) this has a special meaning:
 * the plugin will be called at the start of every frame and at the end
 * of every frame with an csevBroadcast event with the Event.Command.Code
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

SCF_VERSION (iEvent, 0, 0, 1);

/**
 * This interface describes any system event.<p>
 * Events can be generated by hardware (keyboard, mouse)
 * as well as by software (commands and broadcasts). Not all
 * events neccessarily pass through the system event queue;
 * you may organize point-to-point event flows between some
 * plugins and so on.
 *<p>
 * The events can be generated by the event outlet (see the CreateEvent
 * method in iEventOutlet) if you don't want to create your own
 * implementations of this interface. On the other hand, if you want to
 * provide extra functionality you may subclass iEvent interface and
 * add another interface (say iExtEvent) then you may query that interface
 * using normal SCF QueryInterface method.
 */
struct iEvent : public iBase
{
  uchar Type;			// Event type (one of csevXXX)
  uchar Category;		// Event cathegory (unused by CSWS)
  uchar SubCategory;		// Even finer granularity
  uchar Flags;			// Miscelaneous event flags
  cs_time Time;			// Time when the event occured
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
      uint Code;		// Command code
      void *Info;		// Command info
    } Command;
  };
};

/**
 * The overall structure of the basic event flow in Crystal Space:
 * <pre>
 *   ......................
 *   .User application    .
 *   . +----------------+ .
 * +-->+ Event consumer | .
 * | . +----------------+ .
 * | ......................
 * |
 * | .....................................
 * | .System driver plugin               .
 * | .                          +------+ .   +-----+
 * | .                       +<-|event +-<<--+event|
 * | .   +---------------+   |  |outlet| .   |plug |
 * +-----+  event queue  +<--+  +------+ .   +-----+
 * | .   +---------------+   |  +------+ .   +-----+
 * | .                       +<-|event +-<<--+event|
 * | .   +---------------+   |  |outlet| .   |plug |
 * +-----+ event cord    +<--|  +------+ .   +-----+
 *   .   +---------------+   |  +------+ .   +-----+
 *   .                       +<-|event +-<<--+event|
 *   .                          |outlet| .   |plug |
 *   .                          +------+ .   +-----+
 *   .                            ....   .     ...
 *   .....................................
 * </pre>
 * The events are generated by 'event plugs' which are plugged into
 * 'event outlets'. The event outlets are reponsible for filtering
 * the possibly duplicate messages that are coming from different event
 * plugs (for example two different plugs may both intercept the keyboard
 * and generate duplicate keydown events).<p>
 * Events are put into the system event queue, from where they are sent to
 * applications and plugins.
 * Event cords bypass the system queue for specific command event categories
 * and deliver events immediately in a prioritizied chain to specific plugins
 * which request the categories.
 */

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

SCF_VERSION (iEventPlug, 0, 0, 1);

/**
 * Event plug interface, also referred as "event source".<p>
 * This interface should be implemented by any plugin that wants to be able
 * to generate events and to put them into system event queue. The plugin
 * registers itself with the system driver as a event source, and gets a
 * pointer to a new iEventOutlet object which manages event the event flow
 * from this particular event source.
 */
struct iEventPlug : public iBase
{
  /**
   * Get the mask of events that can be generated by this source
   * and are generated directly from user actions (e.g. key presses,
   * mouse clicks and so on). This is used to locate potentialy conflicting
   * combinations of event source plugins (for example two event sources
   * may generate a csevKeyDown event each from every key press).<p>
   * The mask is a combination of CSEVTYPE_XXX values ORed together.
   */
  virtual unsigned GetPotentiallyConflictingEvents () = 0;

  /**
   * Query how strong the plug's wish to generate certain class of events is.
   * The plug with the strongest wish wins. The argument is one of CSEVTYPE_XXX
   * values (but never a combination of several ORed together).<p>
   * The typical value is somewhere around 100; the event plugs which are
   * sometimes implemented inside the system drivers (such as for Windows
   * and DJGPP) usually have the priority 100.
   */
  virtual unsigned QueryEventPriority (unsigned iType) = 0;

  /**
   * Enable or disable certain event class(es).<p>
   * This is not a mandatory function; in fact most event plugs may safely
   * ignore it. The mean of this function is purely advisory; for example
   * if both keyup and keydown events are disabled the plug may want to
   * release the keyboard and so on.
   */
  virtual void EnableEvents (unsigned /*iType*/, bool /*iEnable*/) {}
};

SCF_VERSION (iEventOutlet, 0, 0, 1);

/**
 * The iEventOutlet is the interface to an object that is provided by
 * system driver to every event plug when it registers itself. Any event
 * plug will interact with event outlet to put events into system queue
 * and so on.<p>
 * The system driver is responsible for detecting potentially conflicting
 * situations when several event plugs may generate a event from same
 * original event (e.g. a key press will cause several keydown events
 * coming from several event source plugins). In this case the event
 * sources are queried for how strong their "wish" to generate certain
 * types of events is, and the one with the strongest "wish" wins. Then
 * the respective outlet is set up such that any unwanted events coming
 * from 'disqualified' event plugs are discarded.
 */
struct iEventOutlet : public iBase
{
  /**
   * Create a event object on behalf of the system driver.<p>
   * A general function for generating virtually any type of event.
   * Since events should be created inside the system driver plugin,
   * you should generate first a event object (through CreateEvent
   * method) then you fill it whatever you like and finally you
   * insert it into the event queue with the PutEvent method.
   */
  virtual iEvent *CreateEvent () = 0;

  /**
   * Put a previously created event into system event queue.<p>
   * <b>NOTE:</b> YOU SHOULD PASS HERE ONLY OBJECTS THAT WERE CREATED
   * VIA CreateEvent FUNCTION! IF YOU PASS ARBITRARY EVENTS CREATED
   * BY YOUR PROGRAM/PLUGIN IN SOME ENVIRONMENTS IT WILL CRASH!
   */
  virtual void PutEvent (iEvent *Event) = 0;

  /**
   * Put a keyboard event into event queue.<p>
   * Note that iKey is the key code, either the alphanumeric symbol
   * that is emmited by the given key when no shift keys/modes are
   * active (e.g. 'a', 'b', '.', '/' and so on) or one of CSKEY_XXX
   * values (with value above 255) and the iChar parameter is the
   * translated key, after applying all modeshift keys. Never assume
   * that any of these two codes is always less 127, not being 255
   * or 224 -- these are common mistakes for English-speaking programmers.
   *<p>
   * if you pass -1 as character code, the iChar argument is computed
   * using an simple internal translation table that takes care of
   * Control/Shift/Alt for English characters. But in general it is
   * hardly advised your driver to make the conversion using OS-specific
   * National Language Support subsystem so that national characters
   * are properly supported.
   */
  virtual void Key (int iKey, int iChar, bool iDown) = 0;

  /**
   * Put a mouse event into event queue.<p>
   * If iButton == 0, this is a mouse motion event, and iDown argument
   * is ignored. Otherwise an mousedown or mouseup event is generated at
   * respective location. iButton can be in the range from 1 to
   * CS_MAX_MOUSE_BUTTONS (or 0 for mouse move event).
   */
  virtual void Mouse (int iButton, bool iDown, int x, int y) = 0;

  /**
   * Put a joystick event into event queue.<p>
   * iNumber is joystick number (from 0 to CS_MAX_JOYSTICK_COUNT-1).<p>
   * If iButton == 0, this is a joystick move event and iDown is ignored.
   * Otherwise an joystick up/down event is generated. iButton can be from
   * 1 to CS_MAX_JOYSTICK_BUTTONS (or 0 for a joystick move event).
   */
  virtual void Joystick (int iNumber, int iButton, bool iDown, int x, int y) = 0;

  /**
   * Put a broadcast event into event queue.<p>
   * This is a generalized way to put any broadcast event into the system
   * event queue. Command code may be used to tell user application that
   * application's focused state has changed (cscmdFocusChanged), that
   * a graphics context has been resized (cscmdContextResize), that it
   * has been closed (cscmdContextClose), to finish the application
   * immediately (cscmdQuit) and so on.
   */
  virtual void Broadcast (int iCode, void *iInfo = NULL) = 0;

  /**
   * This is a special routine which is called for example when the
   * application is going to be suspended (suspended means "frozen",
   * that is, application is forced to not run for some time). This happens
   * for example when user switches away from a full-screen application on
   * any OS with MGL canvas driver, or when it presses <Pause> with the OS/2
   * DIVE driver, or in any other drivers that supports forced pausing of
   * applications.<p>
   * This generates a `normal' broadcast event with given command code;
   * the crucial difference is that the event is being delivered to all
   * clients *immediately*. The reason is that the application is frozen
   * right after returning from this routine thus it will get the next
   * chance to process any events only after it will be resumed (which
   * is kind of too late to process this kind of events).
   */
  virtual void ImmediateBroadcast (int iCode, void *iInfo) = 0;
};

SCF_VERSION (iEventCord, 0, 0, 1);

/**
 * The iEventCord is an interface provided by the system driver to
 * any plugins wanting to receive some subclasses of events ASAP
 * in a specified priority, bypassing system event queue.
 * It can optionally pass events onto the system queue, as well.
 */
struct iEventCord
{
  /**
   * Insert a plugin into the queue.  The priority defines
   * when it will receive the event with respect to other
   * registered plugins.  Plugins with the same priority are
   * handled in a first-come first-served fashion.  This is 
   * significant since returning true from HandleEvent will
   * stop further event processing.
   */
  virtual int Insert (iPlugIn *plugin, int priority) = 0;

  /**
   * Remove a plugin from the queue.
   */
  virtual void Remove (iPlugIn *plugin) = 0;

  /**
   * Returns true if events are passed on to the
   * system queue after all plugins in the local
   * queue have processed (and returned false).
   */
  virtual bool GetPass () const = 0;

  /**
   * Sets whether events are passed to the system
   * queue after the local queue have been processed
   * and all returned false.  This could cause the 
   * event to be processed twice under some circumstances.
   */
  virtual void SetPass (bool pass) = 0;
};

#endif // __IEVENT_H__
