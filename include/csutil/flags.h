/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef _CS_FLAGS_H
#define _CS_FLAGS_H

/**
 * Set of flags which can be accessed through masks.
 */
class csFlags
{
private:
  /// Set of flags
  unsigned flags;

public:
  /// Constructor
  csFlags (int Value = 0) : flags (Value) { }

  /// Initialize all flags to the given mask.
  void SetAll (unsigned value)
  { flags = value; }

  /// Set all given flags
  void Set (unsigned mask)
  { flags = (flags & ~mask) | mask; }

  /// Reset all given flags
  void Reset (unsigned mask)
  { flags = (flags & ~mask); }

  /// Set all flags with the given mask.
  void Set (unsigned mask, unsigned value)
  { flags = (flags & ~mask) | value; }

  /// Set all flags with the given value.
  void SetBool (unsigned mask, bool value)
  {
    if (value) flags = (flags & ~mask) | mask;
    else flags = (flags & ~mask);
  }

  /// Get flags.
  unsigned Get () const
  { return flags; }

  /// Check if any of the given flags are set.
  bool Check (unsigned mask) const
  { return (flags & mask) != 0; }

  /// Check if all the given flags are set.
  bool CheckAll (unsigned mask) const
  { return (flags & mask) == mask; }
};

#endif /*_CS_FLAGS_H*/

