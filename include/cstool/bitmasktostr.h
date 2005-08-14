/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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

#ifndef __CS_CSTOOL_BITMASKTOSTR_H__
#define __CS_CSTOOL_BITMASKTOSTR_H__

/**\file
 * Small helper to get a "pretty" string for a combination of bit masks.
 */

#include "csutil/csstring.h"

/**
 * \addtogroup util
 * @{ */

/**
 * Small helper to get a "pretty" string for a combination of bit masks.
 */
class csBitmaskToString
{
public:
  /// Structure containing a name for possible bit masks.
  struct MaskNames
  {
    /// Bit mask
    uint bits;
    /// Name of the mask. Emitted when \a bits are set in a given mask.
    const char* name;
  };
  /// string used to store the prettified strings.
  CS_DECLARE_STATIC_CLASSVAR_REF(scratch, GetScratch, csString)
  /**
   * Retrieve "pretty" string for value composed of bit masks ORed together.
   * \param mask The value for which a string should be retrieved.
   * \param names Table with bitmask-to-name mappings. Last entry must be
   *   {0, 0}.
   * \remarks The returned string is only valid until the next call to 
   *   GetStr().
   */
  static const char* GetStr (uint mask, const MaskNames* names)
  {
    if (mask == 0)
      return "0";
    
    GetScratch().Clear();
    while (names->bits != 0)
    {
      if (mask & names->bits)
      {
	GetScratch() <<  " | " << names->name;
	mask &= ~names->bits;
      }
      names++;
    }
    if (mask != 0)
    {
      GetScratch().AppendFmt (" | %#x", mask);
    }
    return GetScratch().GetData() + 3;
  }
};

/**
 * Helper macro to begin a bitmask-to-name table suitable for
 * csBitmaskToString::GetStr();
 */
#define CS_BITMASKTOSTR_MASK_TABLE_BEGIN(tableName)	      \
  static const csBitmaskToString::MaskNames tableName[] = {
/**
 * Helper macro to add an entry to a bitmask-to-name table that is a
 * \#define.
 */
#define CS_BITMASKTOSTR_MASK_TABLE_DEFINE(def)		      \
    {def, #def},
/// Helper macro to enf a bitmask-to-name table.
#define CS_BITMASKTOSTR_MASK_TABLE_END			      \
    {0, 0}						      \
  }

/** @}*/

#endif // __CS_CSTOOL_BITMASKTOSTR_H__
