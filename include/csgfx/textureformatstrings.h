/*
    Copyright (C) 2006 by Jorrit Tyberghein

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

#ifndef __CS_CSGFX_TEXTUREFORMATSTRINGS_H__
#define __CS_CSGFX_TEXTUREFORMATSTRINGS_H__

#include "csextern.h"

#include "csutil/csstring.h"

/**\file
 * Parser for texture format strings.
 */

namespace CS
{
  /**
   * Texture format string parser routines.
   */
  class CS_CRYSTALSPACE_EXPORT TextureFormatStrings
  {
  public:
    /**
     * Convert a texture format to a canonical form. Here are a few examples:
     *    - rgba32_f -> r32g32b32a32_f
     *    - r5g6b5   -> r5g6b6_i
     *    - b8g8r8_i -> b8g8r8_i
     *    - d24      -> d24_i
     *    - d24s8    -> d24s8_i
     *    - *dxt1    -> *dxt1 (everything after '*' is unchanged)
     *    - invalid  -> -
     */
    static csString CanonicalTextureFormat (const char* in);
  };
}

#endif // __CS_CSGFX_TEXTUREFORMATSTRINGS_H__

