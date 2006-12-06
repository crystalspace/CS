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
#define CS_TEXTUREFORMAT_INVALID '-'
#define CS_TEXTUREFORMAT_STAR '*'
#define CS_TEXTUREFORMAT_INTEGER 'i'
#define CS_TEXTUREFORMAT_FLOAT 'f'

  /**
   * Structured representation of a texture format.
   */
  class CS_CRYSTALSPACE_EXPORT StructuredTextureFormat
  {
  private:
    uint64 coded_components;
    char format;
    csString extra;

  public:
    /// Construct an invalid texture format.
    StructuredTextureFormat ()
    {
      coded_components = CONST_UINT64 (0);
      format = CS_TEXTUREFORMAT_INVALID;
    }

    /**
     * A starred format (like '*dxt1').
     */
    void SetStarred (const char* extra)
    {
      StructuredTextureFormat::extra = extra;
      format = CS_TEXTUREFORMAT_STAR;
    }

    /**
     * Add a new component to the texture format.
     * \param cmp is one of 'r', 'g', 'b', 'a', 'd', 's', or 'l'
     * \param size is the size of that component. If 0 then it will have
     *  to be set later using FixSizes().
     * \return false if the component couldn't be added.
     */
    bool AddComponent (char cmp, int size)
    {
      uint64 shifted = coded_components << 16;
      if ((shifted >> 16) != coded_components)
	return false;
      coded_components = shifted + (CONST_UINT64 (256) * cmp) + size;
      return true;
    }

    /**
     * Set the format (one of the CS_TEXTUREFORMAT_ constants).
     */
    void SetFormat (uint format)
    {
      StructuredTextureFormat::format = format;
    }

    /**
     * Fix the unset sizes (i.e. 0 sizes) so that they 
     * are filled with the given size.
     */
    void FixSizes (int size);

    /**
     * Convert this structured format to canonical format.
     */
    csString GetCanonical ();

    bool operator== (const StructuredTextureFormat& other) const
    {
      if (coded_components != other.coded_components) return false;
      if (format != other.format) return false;
      return (extra == other.extra);
    }

    bool IsValid () { return format != CS_TEXTUREFORMAT_INVALID; }
  };

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
    static csString ConvertCanonical (const char* in);

    /**
     * Convert a (canonical of not) texture format to a structured form.
     */
    static StructuredTextureFormat ConvertStructured (const char* in);
  };
}

#endif // __CS_CSGFX_TEXTUREFORMATSTRINGS_H__

