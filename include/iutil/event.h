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

#ifndef __CS_IUTIL_EVENT_H__
#define __CS_IUTIL_EVENT_H__

#include "iutil/evdefs.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/csunicode.h"

/**\file
 * Event system related interfaces
 */
/**
 * \addtogroup event_handling
 * @{ */

/// Maximal number of mice supported
#define CS_MAX_MOUSE_COUNT      4
/// Maximal number of mouse axes supported
#define CS_MAX_MOUSE_AXES       8
/// Maximal number of mouse buttons supported
#define CS_MAX_MOUSE_BUTTONS	10
/// Maximal number of joysticks supported
#define CS_MAX_JOYSTICK_COUNT	16
/// Maximal number of joystick buttons supported
#define CS_MAX_JOYSTICK_BUTTONS	10
/// Maximal number of joystick axes supported
#define CS_MAX_JOYSTICK_AXES    8
/* Architecturally, AXES can go as high as 32 (limited by the uint32 change mask). */

struct iEventHandler;
struct iEvent;

SCF_VERSION (iEventAttributeIterator, 0, 0, 1);

/**
 * Event attribute iterator. Can be used to enumerate all attribute names
 * an event possesses.
 */
struct iEventAttributeIterator : public iBase
{
  /// Whether a next attribute is available.
  virtual bool HasNext() = 0;
  /// Return the name of the next attribute.
  virtual const char* Next() = 0;
  /// Reset the iterator to the start.
  virtual void Reset() = 0;
};

SCF_VERSION (iEvent, 0, 2, 0);

// Event data structs. Defined outside of iEvent to allow SWIG to
// handle the nested structs and union. Does not break any code.

/**\page Keyboard Keyboard events
 * Keyboard events are emitted when the user does something with the keyboard -
 * presses down a key ("key down" events), holds it down (more "key down" 
 * events in a specific interval - "auto-repeat") and releases it ("key up").
 * <p>
 * Every keyboard event has a bunch of data associated with it. First, there
 * is a code to identify the key: the 'raw' code. It uniquely identifies the 
 * key, and every key has a distinct code, independent from any pressed 
 * modifiers: For example, pressing the "A" key will always result in the raw 
 * code 'a', holding shift or any other modifier down won't change it. However, 
 * the 'cooked' code contains such additional information: If Shift+A is 
 * pressed, the cooked code will be 'A', while the raw code is still 'a'.
 * Other keys are also normalized; for example keypad keys: pressing "9" will
 * result in either CSKEY_PGUP or '9', depending on the NumLock state.
 * So, the same key can result in different 'cooked' codes, and the same 
 * 'cooked' code can be caused by different keys.
 * <p>
 * Other data contained in a keyboard event is:
 * <ul>
 * <li>Whether it is a key up or down event</li>
 * <li>Whether it is an autorepeat of an earlier keypress</li>
 * <li>Modifiers at the time of the keypress</li>
 * <li>When it is a character, whether it is a normal or dead character</li>
 * </ul>
 * <p>
 * Keyboard event data is stored as properties of iEvent, accessible thorugh
 * iEvent->Find() and iEvent->Add().
 * <table>
 * <tr><td><b>Property Name</b></td><td><b>Type</b></td>
 *  <td><b>Description</b></td></tr>
 * <tr><td>keyEventType</td><td>csKeyEventType (stored as uint8)</td>
 *  <td>Event type (up vs down)</td></tr>
 * <tr><td>keyCodeRaw</td><td>uint32</td><td>Raw key code</td></tr>
 * <tr><td>keyCodeCooked</td><td>uint32</td><td>Cooked key code</td></tr>
 * <tr><td>keyModifiers</td><td>csKeyModifiers</td><td>Modifiers at time of 
 *  the key press</td></tr>
 * <tr><td>keyAutoRepeat</td><td>bool</td><td>Autorepeat flag</td></tr>
 * <tr><td>keyCharType</td><td>csKeyCharType (stored as uint8)</td>
 *  <td>Character type</td></tr>
 * </table>
 * <p>
 * A way to retrieve an keyboard event's data without requiring a plethora
 * of iEvent->Find() invocations provides the csKeyEventHelper class.
 * <p>
 * Also see iKeyComposer for informations about composing accented etc.
 * characters from dead and normal keys.
 */

/**
 * Structure that collects the data a keyboard event carries.
 * The event it self doesn't transfer the data in this structure; it is merely 
 * meant to pass around keyboard event data in a compact way within client code
 * without having to pass around the event itself. 
 * @sa csKeyEventHelper
 */
struct csKeyEventData
{
  /// Event type
  csKeyEventType eventType;
  /// Raw key code
  utf32_char codeRaw;
  /// Cooked key code
  utf32_char codeCooked;
  /// Modifiers at the time the event was generated
  csKeyModifiers modifiers;
  /// Auto-repeat flag
  bool autoRepeat;
  /// Type of the key, if it is a character key
  csKeyCharType charType;
};

/**
 * Constants for mouse buttons.
 * Note: the possibly occuring values are not limited to those below, e.g.
 * maybe some day 6-button mice are available...
 */
enum csMouseButton
{
  /// Left button
  csmbLeft = 1,
  /// Right button
  csmbRight = 2,
  /// Middle button
  csmbMiddle = 3,
  /// Wheel was scrolled up
  csmbWheelUp = 4,
  /// Wheel was scrolled up
  csmbWheelDown = 5,
  /// Thumb button 1 (e.g. on 5-button mice)
  csmbExtra1 = 6,
  /// Thumb button 2 (e.g. on 5-button mice)
  csmbExtra2 = 7
};

/**
 * Structure that collects the data a mouse event carries.
 * The event it self doesn't transfer the data in this structure; it is merely 
 * meant to pass around mouse event data in a compact way within client code
 * without having to pass around the event itself. 
 * @sa csMouseEventHelper
 */
struct csMouseEventData
{
  /// Mouse x (same as axes[0])
  int x;
  /// Mouse y (same as axes[1])
  int y;
  /// Mouse axis values
  int32 axes[CS_MAX_MOUSE_AXES];
  /// Mouse axis count
  uint numAxes;
  /**
   * Button number.
   * \sa csMouseButton
   */
  uint Button;
  /// Control key state
  uint32 Modifiers;
};

/**
 * Structure that collects the data a joystick event carries.
 * The event it self doesn't transfer the data in this structure; it is merely 
 * meant to pass around joystick event data in a compact way within client code
 * without having to pass around the event itself. 
 * @sa csJoystickEventHelper
 */
struct csJoystickEventData
{
  /// Joystick number (1, 2, ...)       
  uint number;
  /// Joystick axis values
  int32 axes[CS_MAX_JOYSTICK_AXES];
  /// Axes count
  uint numAxes;
  /// Axes change mask
  uint32 axesChanged;
  /// Joystick button number
  uint Button;
  /// Control key state
  uint32 Modifiers;
};

/**
 * Structure that collects the data a command event carries.
 * The event it self doesn't transfer the data in this structure; it is merely 
 * meant to pass around command event data in a compact way within client code
 * without having to pass around the event itself. 
 * @sa csCommandEventHelper
 */
struct csCommandEventData
{
  /** Command code. @see csCommandEventCode for common codes. */
  uint Code;
  /** Command info. Meaning depends on the particular command. */
  intptr_t Info;
};

/**
 * Error codes for event attribute retrieval.
 */
enum csEventError
{
  /// No error
  csEventErrNone,
  /**
   * The attribute value could be converted to the requested type, however,
   * data was lost during the conversion.
   */
  csEventErrLossy,
  /// The requested attribute was not found.
  csEventErrNotFound,
  //@{
  /**
   * The contained value can not be converted to the requested type. The
   * error code indicates the actual contained type.
   */
  csEventErrMismatchInt,
  csEventErrMismatchUInt,
  csEventErrMismatchFloat,
  csEventErrMismatchBuffer,
  csEventErrMismatchEvent,
  csEventErrMismatchIBase,
  //@}
  /**
   * Unknown error. Something doesn't work like it should.
   */
  csEventErrUhOhUnknown
};

/// Various attribute data types supported by the event system (iEvent).
enum csEventAttributeType
{
  /**
   * The attribute type is unknown. This shouldn't occur.
   */
  csEventAttrUnknown,
  /// A signed integer is contained.
  csEventAttrInt,
  /// An unsigned integer is contained.
  csEventAttrUInt,
  /// A floating point number is contained.
  csEventAttrFloat,
  /// A string or raw data buffer is contained.
  csEventAttrDatabuffer,
  /// An iEvent is contained.
  csEventAttrEvent,
  /// An iBase interface is contained.
  csEventAttriBase
};

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
  /**
   * Event type. @see csEventType
   */
  uint8 Type;			
  /// Event category
  uint8 Category;		
  /// Even finer granularity
  uint8 SubCategory;		
  /// Miscelaneous event flags
  uint8 Flags;			
  /// Time when the event occured
  csTicks Time;			

  //@{
  /**
   * Add an attribute to the event.
   */
  virtual bool Add (const char *name, int8 v) = 0;
  virtual bool Add (const char *name, uint8 v) = 0;
  virtual bool Add (const char *name, int16 v) = 0;
  virtual bool Add (const char *name, uint16 v) = 0;
  virtual bool Add (const char *name, int32 v) = 0;
  virtual bool Add (const char *name, uint32 v) = 0;
  virtual bool Add (const char *name, int64 v) = 0;
  virtual bool Add (const char *name, uint64 v) = 0;
  virtual bool Add (const char *name, float v) = 0;
  virtual bool Add (const char *name, double v) = 0;
  virtual bool Add (const char *name, const char *v) = 0;
  virtual bool Add (const char *name, const void *v, size_t size) = 0;
  virtual bool Add (const char *name, bool v) = 0;
  virtual bool Add (const char *name, iEvent* v) = 0;
  virtual bool Add (const char *name, iBase* v) = 0;
  //@}

  //@{
  /**
   * Retrieve an attribute from the event.
   */
  virtual csEventError Retrieve (const char *name, int8 &v) const = 0;
  virtual csEventError Retrieve (const char *name, uint8 &v) const = 0;
  virtual csEventError Retrieve (const char *name, int16 &v) const = 0;
  virtual csEventError Retrieve (const char *name, uint16 &v) const = 0;
  virtual csEventError Retrieve (const char *name, int32 &v) const = 0;
  virtual csEventError Retrieve (const char *name, uint32 &v) const = 0;
  virtual csEventError Retrieve (const char *name, int64 &v) const = 0;
  virtual csEventError Retrieve (const char *name, uint64 &v) const = 0;
  virtual csEventError Retrieve (const char *name, float &v) const = 0;
  virtual csEventError Retrieve (const char *name, double &v) const = 0;
  virtual csEventError Retrieve (const char *name, const char *&v) const = 0;
  virtual csEventError Retrieve (const char *name, const void *&v, 
    size_t& size) const = 0;
  virtual csEventError Retrieve (const char *name, bool &v) const = 0;
  virtual csEventError Retrieve (const char *name, csRef<iEvent> &v) const = 0;
  virtual csEventError Retrieve (const char *name, csRef<iBase> &v) const = 0;
  //@}

  /// Test whether an attribute exists.
  virtual bool AttributeExists (const char* name) = 0;
  /// Query the type of an attribute.
  virtual csEventAttributeType GetAttributeType (const char* name) = 0;

  /// Remove a specific attribute.
  virtual bool Remove (const char *name) = 0;
  /// Remove all attributes.
  virtual bool RemoveAll() = 0;

  /// Get an iterator for all attributes.
  virtual csRef<iEventAttributeIterator> GetAttributeIterator() = 0;
};

/** \page EventFlow Overall structure of the basic event flow in Crystal Space
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
 * Events are put into the event queue, from where they are sent to
 * applications and plugins.
 * Event cords bypass the queue for specific command event categories
 * and deliver events immediately in a prioritizied chain to specific plugins
 * which request the categories.
 */
SCF_VERSION (iEventPlug, 0, 0, 1);

/**
 * Event plug interface, also referred as "event source".<p>
 * This interface should be implemented by any plugin that wants to be able
 * to generate events and to put them into system event queue. The plugin
 * registers itself with an event queue as an event source, and gets a
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
   * values (but never a combination of several OR'ed together).<p>
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

SCF_VERSION (iEventOutlet, 0, 1, 0);

/**
 * The iEventOutlet is the interface to an object that is provided by
 * an event queue to every event plug when it registers itself. Any event
 * plug will interact with event outlet to put events into system queue
 * and so on.<p>
 * The event queue is responsible for detecting potentially conflicting
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
   * Create a event object on behalf of the event queue.<p>
   * A general function for generating virtually any type of event.  Since all
   * events for a particular event queue should be created from the same heap,
   * you should generate first a event object (through CreateEvent method) then
   * you fill it whatever you like and finally you insert it into the event
   * queue with the Post() method.
   */
  virtual csPtr<iEvent> CreateEvent () = 0;

  /**
   * Put a previously created event into system event queue.<p>
   * \remarks The event you pass to this method should be heap-allocated rather
   * than stack-allocated since the event will be queued for later dispatch and
   * because receivers of the event may claim their own references to it.  The
   * typical way to create a heap-allocated event is with
   * iEventQueue::CreateEvent(), iEventOutlet::CreateEvent(), or via the C++
   * 'new' operator. The CreateEvent() methods have the benefit that they pool
   * "dead" events and re-issue them to you when needed, thus they are quite
   * efficient.
   */
  virtual void Post (iEvent*) = 0;

  /**
   * Put a keyboard event into event queue.<p>
   * Note that codeRaw is the key code, either the alphanumeric symbol
   * that is emmited by the given key when no shift keys/modes are
   * active (e.g. 'a', 'b', '.', '/' and so on) or one of CSKEY_XXX
   * values (with value above 255) and the codeCooked parameter is the
   * translated key, after applying all modeshift keys.
   * <p>
   * If you pass 0 as codeCooked, a synthesized value is created based upon
   * codeRaw using an simple internal translation table that takes care of
   * Control/Shift/Alt for English characters. However, in general, it is
   * best if the entity posting the event can provide both codes.
   */
  virtual void Key (utf32_char codeRaw, utf32_char codeCooked, bool iDown) = 0;

  /**
   * Put a mouse event into event queue.<p>
   * If iButton == 0, this is a mouse motion event, and iDown argument
   * is ignored. Otherwise an mousedown or mouseup event is generated at
   * respective location. iButton can be in the range from 1 to
   * CS_MAX_MOUSE_BUTTONS (or 0 for mouse move event).
   */
  virtual void Mouse (uint iButton, bool iDown, int x, int y) = 0;

  /**
   * Put a joystick event into event queue.<p>
   * iNumber is joystick number (from 0 to CS_MAX_JOYSTICK_COUNT-1).<p>
   * If iButton == 0, this is a joystick move event and iDown is ignored.
   * numAxes can be from 1 to CS_MAX_JOYSTICK_AXES.
   * Otherwise an joystick up/down event is generated. iButton can be from
   * 1 to CS_MAX_JOYSTICK_BUTTONS (or 0 for a joystick move event).
   */
  virtual void Joystick(uint iNumber, uint iButton, bool iDown, 
    const int32 *axes, uint numAxes) = 0;

  /**
   * Put a broadcast event into event queue.<p>
   * This is a generalized way to put any broadcast event into the system
   * event queue. Command code may be used to tell user application that
   * application's focused state has changed (cscmdFocusChanged), that
   * a graphics context has been resized (cscmdContextResize), that it
   * has been closed (cscmdContextClose), to finish the application
   * immediately (cscmdQuit) and so on.
   */
  virtual void Broadcast (uint iCode, intptr_t iInfo = 0) = 0;

  /**
   * This is a special routine which is called for example when the
   * application is going to be suspended (suspended means "frozen",
   * that is, application is forced to not run for some time). This happens
   * for example when user switches away from a full-screen application on
   * any OS with MGL canvas driver, or when it presses &lt;Pause&gt; with the 
   * OS/2 DIVE driver, or in any other drivers that supports forced pausing of
   * applications.<p>
   * This generates a `normal' broadcast event with given command code;
   * the crucial difference is that the event is being delivered to all
   * clients *immediately*. The reason is that the application is frozen
   * right after returning from this routine thus it will get the next
   * chance to process any events only after it will be resumed (which
   * is kind of too late to process this kind of events).
   */
  virtual void ImmediateBroadcast (uint iCode, intptr_t iInfo) = 0;
};

SCF_VERSION (iEventCord, 0, 0, 3);

/**
 * The iEventCord is an interface provided by an event queue to
 * any event handlers wanting to receive some subclasses of events ASAP
 * in a specified priority, bypassing the queue itself.
 * Events may also optionally be sent to the normal event queue itself
 * if none of the event handlers in the cord handle the event.
 */
struct iEventCord : public iBase
{
  /**
   * Insert an event handler into the cord.  The priority defines when it
   * will receive the event with respect to other registered event handlers.
   * Event handlers with the same priority are handled in a first-come
   * first-served fashion.  This is significant since returning true from
   * HandleEvent() will stop further event processing.
   */
  virtual int Insert (iEventHandler*, int priority) = 0;

  /**
   * Remove an event handler from the cord.
   */
  virtual void Remove (iEventHandler*) = 0;

  /**
   * Returns true if events are passed on to the owning event queue if all
   * plugins in the cord return false from HandleEvent().
   */
  virtual bool GetPass () const = 0;

  /**
   * Sets whether events are passed along to the owning event queue if all
   * plugins in the cord return false from HandleEvent().
   */
  virtual void SetPass (bool) = 0;

  /// Get the category of this cord.
  virtual int GetCategory() const = 0;
  // Get the subcategory of this cord.
  virtual int GetSubcategory() const = 0;
};

/** @} */

#endif // __CS_IUTIL_EVENT_H__
