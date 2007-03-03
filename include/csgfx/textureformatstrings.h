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
  /**\addtogroup gfx3d
   * @{ */
  
  /**\page TextureFormatStrings Texture format strings
   * \section tfs_s Special formats
   * Special formats are simple: they're designated by a string starting with 
   * <tt>*</tt>,  everything else afterwards is arbitrary.
   *
   * Examples:
   * \li \c *dxt1 DXT1 compressed texture
   * \li \c *3dc 3Dc compressed texture 
   *
   * \section tsf_g General formats
   * The general syntax is (in dog EBNF): 
   * <pre>
   * format := components {components} [_ format] 
   * components := component {component} size
   * </pre>
   * whereas <tt>component</tt> is the component type, summarized below, 
   * <tt>size</tt> is the width of the component, in bit, and <tt>format</tt> is 
   * an optional specifier for the component format.
   *
   *
   * \subsection tfs_g_comptypes Component types
   * \li \c r Red
   * \li \c g Green
   * \li \c b Blue
   * \li \c a Alpha
   * \li \c x Junk (unused)
   * \li \c l Luminance
   * \li \c d Depth
   * \li \c s Stencil
   *
   * \subsection tfs_g_format Format
   * Format optionally specifies how the data is encoded.
   * \li \c f Float
   *
   * Examples:
   * \li \c abgr32_f: Red, green, blue, alpha are stored as 32-bit floats. 
   *
   * \subsection tfs_g_matching Matching components and sizes
   * A component has the size as specified by the next size specifier right of 
   * it.
   *
   * Examples:
   * \li <tt>argb8, a8r8g8b8:</tt> Red, green, blue, alpha are all 8 bit wide.
   * \li \c r5g6b5: Red is 5 bit wide, green 6 bit, blue 5bit.
   * \li <tt>x1rgb5, x1r5g5b5:</tt> Red, green, blue are all 5 bit wide. One 
   *   bit is unused. 
   * 
   * \subsection tfs_g_storage Storage
   * The leftmost component is stored in the most significant bits; the 
   * rightmost component in the least significant bits. A tuple of components 
   * is interpreted as a word with the size being the sum of all component 
   * sizes, rounded up to the next multiple of 8. These words are then stored 
   * in little-endian.
   * 
   * This means that for formats with 8-bit components, the bytes in memory are 
   * swapped in comparison to their order in the format string. See the examples.
   * 
   * Examples:
   * \li \c argb8 When interpreted as 32-bit words, blue is in bits 0-7, 
   *        green in 8-15, red in 16-23, alpha 24-31. When interpreted as 
   *        bytes, it's four bytes, the first stores blue, the second green, 
   *        the third red, the fourth alpha.
   * \li \c r5g6b5: Red is stored in the 5 most significant bits of the second 
   *        byte, green in the 3 least significant bits of the second byte and 
   *        3 most significant bits of the first byte, blue in the 5 least 
   *        significant bits of the first byte. 
   * 
   * \subsection tfs_g_moreexamples More examples
   * \li \c d24: 24-bit depth texture.
   * \li \c d24s8: Combined 24-bit depth and 8-bit stencil texture. 
   */
 
  /**
   * Structured representation of a texture format.
   */
  class CS_CRYSTALSPACE_EXPORT StructuredTextureFormat
  {
  public:
    /// Texture storage format.
    enum TextureFormat
    {
      /// Invalid format
      Invalid = '-',
      /// Components are stored as integer tuples
      Integer = 'i',
      /// Components are stored as float tuples
      Float = 'f',
      /// "Special" format (e.g. compressed formats)
      Special = '*'
    };
  private:
    uint64 coded_components;
    TextureFormat format;
    csString special;

  public:
    /// Construct an invalid texture format.
    StructuredTextureFormat ();

    /**
     * A special format (like '*dxt1').
     */
    void SetSpecial (const char* special)
    {
      StructuredTextureFormat::special = special;
      format = Special;
    }

    /**
     * Add a new component to the texture format.
     * \param cmp One of the 'Component types' listed in \ref tfs_g_comptypes.
     * \param size Size of that component. Can be 0, but then it will have to 
     *   be set later using FixSizes().
     * \return Whether the component could be added or not.
     */
    bool AddComponent (char cmp, int size);

    /**
     * Set the format.
     */
    void SetFormat (TextureFormat format)
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
      return (special == other.special);
    }

    bool IsValid () { return format != Invalid; }
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
     *
     * \sa \ref TextureFormatStrings 
     */
    static csString ConvertCanonical (const char* in);

    /**
     * Convert a (canonical of not) texture format to a structured form.
     */
    static StructuredTextureFormat ConvertStructured (const char* in);
  };
}

/** @} */

#endif // __CS_CSGFX_TEXTUREFORMATSTRINGS_H__

