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

struct iEvent;
class csInputBinder;

/**
 * This class holds a description of a physical source of input events, such
 * as a keyboard key, mouse or joystick button, or a mouse or joystick axis.
 */
class CS_CSUTIL_EXPORT csInputDefinition
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

  csInputDefinition (uint32 honorModifiers = 0, bool useCookedCode = false);

  friend class csInputBinder;

public:
  /// Copy constructor.
  csInputDefinition (const csInputDefinition &other);

  /// Construct an input description from an iEvent (usually a button).
  csInputDefinition (iEvent *event,
		     uint32 honorModifiers = 0, bool useCookedCode = false);

  /// Construct an input description from an iEvent (usually an axis).
  /// Axis: 0 = x, 1 = y.
  csInputDefinition (iEvent *event, int axis);

  /// Construct an input description from a string like "mouse1" or "shift+a".
  csInputDefinition (const char *string,
		     uint32 honorModifiers = 0, bool useCookedCode = false);

  /// Return a boolean indicating whether the object contains a valid input.
  bool IsValid () const;

  /// Get the string representation of the description.
  csString ToString () const;

  /// Generate a hash value from the object.
  uint32 ComputeHash () const;

  /// Return a boolean indicating whether the definitions are equal.
  bool Compare (csInputDefinition const &) const;

  /// Put here to allow this class to be used as a csHash key handler.
  static unsigned int ComputeHash (const csInputDefinition &key)
  { return key.ComputeHash (); }

  /// Put here to allow this class to be used as a csHash key handler.
  static bool CompareKeys (const csInputDefinition &key1,
			   const csInputDefinition &key2)
  { return key1.Compare (key2); }
};

#endif // __CS_UTIL_INPUTDEF_H__
