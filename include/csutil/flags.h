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
  ULong flags;

public:
  /// Constructor.
  csFlags () : flags (0) { }

  /// Initialize all flags to the given mask.
  void Set (ULong value) { flags = value; }

  /// Set all flags with the given mask.
  void Set (ULong mask, ULong value) { flags = (flags & ~mask) | value; }

  /// Get flags.
  ULong Get () { return flags; }

  /// Check if any of the given flags are set.
  bool Check (ULong to_check) { return (flags & to_check) != 0; }

  /// Check if all the given flags are set.
  bool CheckAll (ULong to_check) { return (flags & to_check) == to_check; }
};

#endif /*_CS_FLAGS_H*/

