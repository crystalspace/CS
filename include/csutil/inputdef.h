/*
    Crystal Space input library
    Copyright (C) 2000 by Andrew Zabolotny <bit@eltech.ru>
    Copyright (C) 2002, 04 by Mathew Sutcliffe <oktal@gmx.co.uk>

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

#ifndef __CS_UTIL_INPUTDEF_H__
#define __CS_UTIL_INPUTDEF_H__

/**\file 
 * Crystal Space input library
 */

#include "csextern.h"

#include "csutil/csstring.h"
#include "csutil/comparator.h"
#include "csutil/hash.h"

#include "iutil/evdefs.h"
#include "iutil/event.h"

struct iEvent;
class csInputBinder;

/**
 * This class holds a description of a physical source of input events, such
 * as a keyboard key, mouse or joystick button, or a mouse or joystick axis.
 */
class CS_CRYSTALSPACE_EXPORT csInputDefinition
{
public:
  csRef<iEventNameRegistry> name_reg;

protected:
  csEventID containedName;

  uint32 modifiersHonored;
  csKeyModifiers modifiers;
  // The (basis-0) identifier for the device from which this event came
  uint deviceNumber;
  union
  {
    struct
    {
      utf32_char code;
      bool isCooked;
    } keyboard;
    int mouseButton;
    int mouseAxis;
    int joystickButton;
    int joystickAxis;
  };

  void Initialize (uint32 honorModifiers, bool useCookedCode);
  void InitializeFromEvent (iEvent *ev);

  friend class csInputBinder;

public:
  /**
   * Default constructor.
   * \param name_reg A pointer to the event name registry.
   * \param honorModifiers A bitmask of modifier keys that will be recognised.
   * \param useCookedCode If true, will use the cooked key code instead of raw.
   */
  csInputDefinition (iEventNameRegistry* name_reg, 
		     uint32 honorModifiers = 0, bool useCookedCode = false);

  /// Copy constructor.
  csInputDefinition (const csInputDefinition &other);

  /**
   * Construct an input description from an iEvent (usually a button).
   * \param name_reg A pointer to the event name registry.
   * \param event The event to analyse for input data.
   * \param honorModifiers A bitmask of modifier keys that will be recognised.
   * \param useCookedCode If true, will use the cooked key code instead of raw.
   */
  csInputDefinition (iEventNameRegistry* name_reg, iEvent *event,
		     uint32 honorModifiers = 0, bool useCookedCode = false);

  /**
   * Construct an input description from an iEvent (usually an axis).
   * \param name_reg A pointer to the event name registry.
   * \param event The event to analyse for input data.
   * \param axis Events include all axes, so choose: 0 = x, 1 = y.
   */
  csInputDefinition (iEventNameRegistry* name_reg, iEvent *event, uint8 axis);

  /**
   * Construct an input description from a string.
   * \param name_reg A pointer to the event name registry.
   * \param string The string to parse, e.g. "mousebutton1", "shift+a".
   * \param honorModifiers A bitmask of modifier keys that will be recognised.
   * \param useCookedCode If true, will use the cooked key code instead of raw.
   * More precisely, the syntax for <tt>string</tt> is (in EBNF):
   * <pre>
   * [ mod ("+"|"-") ]* ( ( devNumber? ("mouse" | "joystick") ( "X" | "Y" | ("Axis" axisNumber) | ("Button" buttonNumber) | buttonNumber ) ) | key )
   * </pre>
   * So the fourth axis of the third joystick is "2JoystickAxis3", and its 
   * first button is "2JoystickButton0" or "2Joystick0".  The leading number
   * must be omitted for the first (primary) input device, e.g.,
   * "MouseAxis0" or "JoystickButton3".
   */
  csInputDefinition (iEventNameRegistry* name_reg, const char *string,
		     uint32 honorModifiers = 0, bool useCookedCode = false);

  /**
   * Gets the string representation of the description.
   * \param distinguishModifiers If false, left and right modifiers will be
   *   output as plain-old modifiers (e.g. "LAlt" and "RAlt" become just "Alt").
   * \return The string representation of the description (e.g. "mousebutton1",
   *   "shift+a").
   */
  csString ToString (bool distinguishModifiers = true) const;

  /// Returns a boolean indicating whether the object contains a valid input.
  bool IsValid () const;

  /// Returns the event name of the description (a csev... constant).
  csEventID GetName () const { return containedName; }

  /// Set the event type of the description (a csev... constant).
  void SetName (csEventID n) { containedName = n; }

  /**
   * Gives the key code of the description, assuming it is a keyboard type.
   * \param code Will be set to the key code.
   * \param isCooked Will be set to true if the code is cooked, false if raw.
   * \return False if the description is not a keyboard type.
   */
  bool GetKeyCode (utf32_char &code, bool &isCooked) const
  { code = keyboard.code;
    isCooked = keyboard.isCooked;
    return (containedName == csevKeyboardEvent(name_reg)); }

  /// Sets the key code of the description, assuming it is a keyboard type.
  bool SetKeyCode (utf32_char code)
  { if (containedName != csevKeyboardEvent(name_reg)) return false;
    keyboard.code = code;
    return true; }

  /**
   * Returns the numeric value of the description.
   * \return If non-keyboard button event, the button number. If axis event,
   *   the axis number (0 = x, 1 = y).
   */
  int GetNumber () const { return mouseButton; }

  /**
   * Sets the numeric value of the description
   * \param n If non-keyboard button event, the button number. If axis event,
   *   the axis number (0 = x, 1 = y).
   */
  void SetNumber (int n) { mouseButton = n; }

  /// Returns the keyboard modifiers of the description.
  const csKeyModifiers& GetModifiers () const { return modifiers; }

  /// Sets the keyboard modifiers of the description.
  void SetModifiers (const csKeyModifiers &mods) { modifiers = mods; }

  /// Generate a hash value from the object.
  uint32 ComputeHash () const;

  /// Returns a number indicating the relation of the two definitions.
  int Compare (csInputDefinition const &) const;

  /**
   * Helper function to parse a string (eg. "Ctrl+A") into values describing
   * a keyboard event, returning both raw and cooked key codes.
   * \param reg A pointer to the event name registry.
   * \param iStr The string to parse.
   * \param oKeyCode Will be set to the raw code of the parsed description.
   * \param oCookedCode Will be set to the cooked code of the description.
   * \param oModifiers The modifiers of the description.
   * \return Whether the string could be successfully parsed.
   * \remarks Any of the output parameters may be null, in which case they are
   *   ignored.
   */
  static bool ParseKey (iEventNameRegistry* reg, 
			const char *iStr, utf32_char *oKeyCode,
			utf32_char *oCookedCode, csKeyModifiers *oModifiers);

  /**
   * Helper function to parse a string (eg. "MouseX", "Alt+Mouse1") into
   * values describing a non-keyboard event.
   * \param reg A pointer to the event name registry.
   * \param iStr The string to parse.
   * \param oType Will be set to the event name of the description
   *   (a csev... identifier).
   * \param oDevice For mouse and joystick events, will be set to the
   *   device number, basis 0 (e.g., the third joystick device is "2").
   * \param oNumeric For button events, will be set to the button number.
   *   For axis events, will be set to the axis number (0 = x, 1 = y).
   * \param oModifiers Will be populated with the modifiers of the description.
   * \return Whether the string could be successfully parsed.
   * \remarks Any of the output parameters may be null, in which case they are
   *   ignored.
   */
  static bool ParseOther (iEventNameRegistry* reg, 
			  const char *iStr, csEventID *oType, uint *oDevice,
			  int *oNumeric, csKeyModifiers *oModifiers);

  /**
   * Helper function to return a string (eg. "Ctrl+A") from values
   * describing a keyboard event.
   * \param reg A pointer to the event name registry.
   * \param code The key code, treated as a raw code although raw vs. cooked
   *   doesn't matter here.
   * \param mods The keyboard modifiers. Will be ignored if 0.
   * \param distinguishModifiers Whether to output distinguished modifiers
   *   (eg. "LCtrl" as opposed to just "Ctrl").
   * \return The description string.
   */
  static csString GetKeyString (iEventNameRegistry* reg,
				utf32_char code, const csKeyModifiers *mods,
				bool distinguishModifiers = true);

  /**
   * Helper function to return a string (eg. "MouseX", "Alt+Mouse1") from
   * values describing a non-keyboard event.
   * \param reg A pointer to the event name registry.
   * \param type The event type of the description (a csev... identifier).
   * \param device For mouse and joystick events, the device number, basis 0
   *   (first mouse is 0, second joystick is 1, etc).
   * \param num For button events, the button number. For axis events, the
   *   axis number (0 = x, 1 = y).
   * \param mods The keyboard modifiers. Will be ignored if 0.
   * \param distinguishModifiers Whether to output distinguished modifiers
   *   (eg. "LCtrl" as opposed to just "Ctrl").
   * \return The description string.
   */
  static csString GetOtherString (iEventNameRegistry* reg,
				  csEventID type, uint device, int num, 
				  const csKeyModifiers *mods, bool distinguishModifiers = true);
};

/**
 * csComparator<> specialization for csInputDefinition to allow its use as 
 * e.g. hash key type.
 */
CS_SPECIALIZE_TEMPLATE
class csComparator<csInputDefinition, csInputDefinition>
{
public:
  static int Compare (csInputDefinition const& r1, csInputDefinition const& r2)
  {
    return r1.Compare (r2);
  }
};

/**
 * csHashComputer<> specialization for csInputDefinition to allow its use as 
 * hash key type.
 */
CS_SPECIALIZE_TEMPLATE
class csHashComputer<csInputDefinition>
{
public:
  static uint ComputeHash (csInputDefinition const& key)
  {
    return key.ComputeHash (); 
  }
};

#endif // __CS_UTIL_INPUTDEF_H__
