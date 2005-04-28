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
#include "iutil/evdefs.h"
#include "iutil/event.h"
#include "csstring.h"
#include "comparator.h"
#include "hash.h"

struct iEvent;
class csInputBinder;

/**
 * This class holds a description of a physical source of input events, such
 * as a keyboard key, mouse or joystick button, or a mouse or joystick axis.
 */
class CS_CRYSTALSPACE_EXPORT csInputDefinition
{
protected:
  int containedType;

  uint32 modifiersHonored;
  csKeyModifiers modifiers;

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
   * \param honorModifiers A bitmask of modifier keys that will be recognised.
   * \param useCookedCode If true, will use the cooked key code instead of raw.
   */
  csInputDefinition (uint32 honorModifiers = 0, bool useCookedCode = false);

  /// Copy constructor.
  csInputDefinition (const csInputDefinition &other);

  /**
   * Construct an input description from an iEvent (usually a button).
   * \param event The event to analyse for input data.
   * \param honorModifiers A bitmask of modifier keys that will be recognised.
   * \param useCookedCode If true, will use the cooked key code instead of raw.
   */
  csInputDefinition (iEvent *event,
		     uint32 honorModifiers = 0, bool useCookedCode = false);

  /**
   * Construct an input description from an iEvent (usually an axis).
   * \param event The event to analyse for input data.
   * \param axis Events include all axes, so choose: 0 = x, 1 = y.
   */
  csInputDefinition (iEvent *event, int axis);

  /**
   * Construct an input description from a string.
   * \param string The string to parse, e.g. "mouse1", "shift+a".
   * \param honorModifiers A bitmask of modifier keys that will be recognised.
   * \param useCookedCode If true, will use the cooked key code instead of raw.
   */
  csInputDefinition (const char *string,
		     uint32 honorModifiers = 0, bool useCookedCode = false);

  /**
   * Gets the string representation of the description.
   * \param distinguishModifiers If false, left and right modifiers will be
   *   output as plain-old modifiers (e.g. "LAlt" and "RAlt" become just "Alt").
   * \return The string representation of the description (e.g. "mouse1",
   *   "shift+a").
   */
  csString ToString (bool distinguishModifiers = true) const;

  /// Returns a boolean indicating whether the object contains a valid input.
  bool IsValid () const;

  /// Returns the event type of the description (a csev... constant).
  int GetType () const { return containedType; }

  /// Set the event type of the description (a csev... constant).
  void SetType (int t) { containedType = t; }

  /**
   * Gives the key code of the description, assuming it is a keyboard type.
   * \param code Will be set to the key code.
   * \param isCooked Will be set to true if the code is cooked, false if raw.
   * \return False if the description is not a keyboard type.
   */
  bool GetKeyCode (utf32_char &code, bool &isCooked) const
    { code = keyboard.code;
      isCooked = keyboard.isCooked;
      return containedType == csevKeyboard; }

  /// Sets the key code of the description, assuming it is a keyboard type.
  bool SetKeyCode (utf32_char code)
    { if (containedType != csevKeyboard) return false;
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

  /// Returns an int indicating the relation og the two definitions.
  int Compare (csInputDefinition const &) const;

  /**
   * Helper function to parse a string (eg. "Ctrl+A") into values describing
   * a keyboard event, returning both raw and cooked key codes.
   * \param iStr The string to parse.
   * \param oKeyCode Will be set to the raw code of the parsed description.
   * \param oCookedCode Will be set to the cooked code of the description.
   * \param oModifiers The modifiers of the description.
   * \return Whether the string could be successfully parsed.
   * \remarks Any of the output parameters may be null, in which case they are
   *   ignored.
   */
  static bool ParseKey (const char *iStr, utf32_char *oKeyCode,
    utf32_char *oCookedCode, csKeyModifiers *oModifiers);

  /**
   * Helper function to parse a string (eg. "MouseX", "Alt+Mouse1") into
   * values describing a non-keyboard event.
   * \param iStr The string to parse.
   * \param oType Will be set to the event type of the description
   *   (a csev... constant).
   * \param oNumeric For button events, will be set to the button number.
   *   For axis events, will be set to the axis number (0 = x, 1 = y).
   * \param oModifiers Will be populated with the modifiers of the description.
   * \return Whether the string could be successfully parsed.
   * \remarks Any of the output parameters may be null, in which case they are
   *   ignored.
   */
  static bool ParseOther (const char *iStr, int *oType, int *oNumeric,
    csKeyModifiers *oModifiers);

  /**
   * Helper function to return a string (eg. "Ctrl+A") from values
   * describing a keyboard event.
   * \param code The key code, treated as a raw code although raw vs. cooked
   *   doesn't matter here.
   * \param mods The keyboard modifiers. Will be ignored if 0.
   * \param distinguishModifiers Whether to output distinguished modifiers
   *   (eg. "LCtrl" as opposed to just "Ctrl").
   * \return The description string.
   */
  static csString GetKeyString (utf32_char code, const csKeyModifiers *mods,
    bool distinguishModifiers = true);

  /**
   * Helper function to return a string (eg. "MouseX", "Alt+Mouse1") from
   * values describing a non-keyboard event.
   * \param type The event type of the description (a csev... constant).
   * \param num For button events, the button number. For axis events, the
   *   axis number (0 = x, 1 = y).
   * \param mods The keyboard modifiers. Will be ignored if 0.
   * \param distinguishModifiers Whether to output distinguished modifiers
   *   (eg. "LCtrl" as opposed to just "Ctrl").
   * \return The description string.
   */
  static csString GetOtherString (int type, int num, const csKeyModifiers *mods,
    bool distinguishModifiers = true);
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
