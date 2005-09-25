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

#ifndef __CS_FLAGS_H__
#define __CS_FLAGS_H__

/**\file
 * Set of flags
 */

#include "csextern.h"

/**
 * Set of flags which can be accessed through masks.
 */
class csFlags
{
private:
  /// Set of flags
  uint32 flags;

public:
  /// Constructor. All flags are set to false by default.
  csFlags (uint32 value = 0) : flags (value) { }

  /**
   * Initialize all flags to the given mask. You can
   * use this function to set and clear all flags at once.
   * If you only want to set flags (and not clear others)
   * then use 'Set(mask)'.
   */
  void SetAll (uint32 value)
  { flags = value; }

  /**
   * Set all given flags. This function will set all flags
   * which are '1' in the mask. All other flags are untouched.
   * This contrasts with SetAll() which will set all flags that
   * are '1' in the input and clear the flags that are '0'.
   */
  void Set (uint32 mask)
  { flags = (flags & ~mask) | mask; }

  /**
   * Reset all given flags.
   * This will clear all flags which you specify in the mask.
   * i.e. if a mask bit is 1 then the corresponding flag will be cleared.
   * All other flags are untouched.
   */
  void Reset (uint32 mask)
  { flags = (flags & ~mask); }

  /**
   * Set all flags with the given mask.
   * With this function you can set and clear a series of flags
   * but only the flags you specify in the mask will be affected.
   */
  void Set (uint32 mask, uint32 value)
  { flags = (flags & ~mask) | (value & mask); }

  /**
   * Set all flags with the given value.
   * This function will set all flags given in the mask with either
   * '1' or '0' depending on the boolean input value.
   */
  void SetBool (uint32 mask, bool value)
  {
    if (value) flags = (flags & ~mask) | mask;
    else flags = (flags & ~mask);
  }

  /// Get flags.
  uint32 Get () const
  { return flags; }

  /// Check if any of the given flags are set.
  bool Check (uint32 mask) const
  { return (flags & mask) != 0; }

  /// Check if all the given flags are set.
  bool CheckAll (uint32 mask) const
  { return (flags & mask) == mask; }
};

#endif // __CS_FLAGS_H__

