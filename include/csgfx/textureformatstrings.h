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
   * \li \c i Unsigned integer
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
   * \subsection tfs_g_canonical Canonical Format
   * Canonical format strings, in comparison to "normal" format strings, 
   * require all "optional" parts to be present. That is,
   * \li after each component a size must appear,
   * \li a format specifier must appear.
   *
   * Examples:
   * \li \c rgba32_f becomes \c r32g32b32a32_f
   * \li \c r5g6b5 becomes \c r5g6b6_i
   * \li \c b8g8r8_i becomes \c b8g8r8_i
   * \li \c d24 becomes \c  d24_i
   * \li \c d24s8 becomes \c d24s8_i
   * \li \c *dxt1, as a special format, stays \c *dxt1
   * \li Any invalid format becomes \c -
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
    // Component data
    struct CompData
    {
      // Must be first entry. Will be '*' in case of special formats...
      unsigned char format;
      union
      {
	uint64 coded_components;
	char* specialStrPtr;
      };
    };
    enum
    {
      SpecialStrExtern = 0x80,
      SpecialStrMax = sizeof (CompData)
    };
    union
    {
      CompData cd;
      // Store 'small' special formats inline.
      char specialStr[SpecialStrMax];
    };

    void FreeSpecialStr ();
  public:
    /// Construct an invalid texture format.
    StructuredTextureFormat ();
    /// Construct a texture format with the given components and sizes.
    StructuredTextureFormat (char cmp1, int size1,
      char cmp2 = 0, int size2 = 0,
      char cmp3 = 0, int size3 = 0,
      char cmp4 = 0, int size4 = 0,
      TextureFormat fmt = Integer);
    /// Copy constructor
    StructuredTextureFormat (const StructuredTextureFormat& other);
    /// Destruct texture format
    ~StructuredTextureFormat ();

    /**
     * A special format (like '*dxt1').
     */
    void SetSpecial (const char* special);

    /**
     * Add a new component to the texture format.
     * \param cmp One of the 'Component types' listed in \ref tfs_g_comptypes.
     * \param size Size of that component. Can be 0, but then it will have to 
     *   be set later using FixSizes().
     * \remarks No effect for special formats.
     * \return Whether the component could be added or not.
     */
    bool AddComponent (char cmp, int size);

    /**
     * Set the format.
     * \remarks You can't use this method to set 'Special' formats. Use
     *   SetSpecial() instead.
     */
    void SetFormat (TextureFormat format)
    {
      CS_ASSERT_MSG ("Use SetSpecial() to set special formats", format != Special);
      if (format == Special) return;
      FreeSpecialStr ();
      if (cd.format == Special) cd.coded_components = 0;
      cd.format = format;
    }

    /**
     * Fix the unset sizes (i.e. 0 sizes) so that they 
     * are filled with the given size.
     */
    void FixSizes (int size);

    /**
     * Convert this structured format to canonical format.
     * \sa \ref tfs_g_canonical
     */
    csString GetCanonical ();

    bool operator== (const StructuredTextureFormat& other) const
    {
      if (GetFormat() != other.GetFormat()) return false;
      if (GetFormat() == Special)
      {
	const char* s1 = GetSpecial();
	const char* s2 = GetSpecial();
	if ((s1 == static_cast<const char*>(nullptr)) && (s2 == static_cast<const char*>(nullptr))) return true;
	if (s1 == static_cast<const char*>(nullptr)) return false;
	if (s1 == static_cast<const char*>(nullptr)) return false;
	return strcmp (s1, s2) == 0;
      }
      else
      {
	return (cd.coded_components == other.cd.coded_components);
      }
    }

    bool operator!= (const StructuredTextureFormat& other) const
    {
      if (GetFormat() != other.GetFormat()) return true;
      if (GetFormat() == Special)
      {
	const char* s1 = GetSpecial();
	const char* s2 = GetSpecial();
	if ((s1 == static_cast<const char*>(nullptr)) && (s2 == static_cast<const char*>(nullptr))) return false;
	if (s1 == static_cast<const char*>(nullptr)) return true;
	if (s1 == static_cast<const char*>(nullptr)) return true;
	return strcmp (s1, s2) != 0;
      }
      else
      {
	return (cd.coded_components != other.cd.coded_components);
      }
    }

    /// Returns whether the contained format is a valid texture format.
    bool IsValid () { return cd.format != Invalid; }

    /**
     * Returns the number of components in this format. Returns 0 for
     * special or invalid formats.
     */
    int GetComponentCount () const
    {
      if (((cd.format & ~SpecialStrExtern) == Special)
	  || (cd.format == Invalid))
	return 0;
      int n = 0;
      uint64 comp = cd.coded_components;
      while (comp != 0) 
      {
        comp >>= 16;
        n++;
      }
      return n;
    }

    /**
     * Get the nth component. Returns 0 if there is no such component.
     */
    char GetComponent (int n) const
    {
      int num = GetComponentCount ();
      if ((n < 0) || (n >= num)) return 0;
      return (cd.coded_components >> (16 * (num - 1 - n) + 8)) & 255;
    }

    /**
     * Get size of the nth component. 
     * \remarks If there is no such component 0 is returned. However, this 
     *   return value does \em not imply that a component doesn not exist,
     *   as 0-sized components can be added by AddComponent(). Only the
     *   return values of GetComponent() and GetComponentCount() can be used
     *   for existance checkes.
     */
    char GetComponentSize (int n) const
    {
      int num = GetComponentCount ();
      if ((n < 0) || (n >= num)) return 0;
      return (cd.coded_components >> (16 * (num - 1 - n))) & 255;
    }

    /**
     * Returns the basic storage type for this texture format.
     * \sa \ref tfs_g_format
     */
    TextureFormat GetFormat() const
    { return static_cast<TextureFormat> (cd.format & ~SpecialStrExtern); }

    /// Return the special format string.
    const char* GetSpecial() const
    {
      if ((cd.format & ~SpecialStrExtern) != Special) return 0;
      if (cd.format & SpecialStrExtern)
	return cd.specialStrPtr;
      else
	return specialStr;
    }

    /**
     * Bit flags for components present in a format.
     * \sa \ref tfs_g_comptypes
     */
    enum
    {
      /// 'r' component
      compR = 0x01,
      /// 'g' component
      compG = 0x02,
      /// 'b' component
      compB = 0x04,
      /// 'a' component
      compA = 0x08,
      /// 'x' component
      compX = 0x10,
      /// 'l' component
      compL = 0x20,
      /// 'd' component
      compD = 0x40,
      /// 's' component
      compS = 0x80,

      /// 'r', 'g' and 'b' components
      compRGB = compR | compB | compG,
      /// 'r', 'g', 'b' and 'a' components
      compRGBA = compR | compB | compG | compA,
      /// 'l' and 'a' components
      compLumA = compL | compA,
      /// 'd' and 's' components
      compDepthStencil = compD | compS,

      /// One or more components are unknown.
      compUnknown = 0x80000000
    };

    /**
     * Return a bit mask that identifies the contained components, regardless
     * of their order. This can be used to "classify" a texture format.
     * The bit flags are #compR, #compG etc.
     *
     * Example: Testing whether a format is an RGB format:
     * \code
     * const char* formatString = "rgb8";
     * CS::StructuredTextureFormat format = 
     *   CS::TextureFormatStrings::ConvertStructured (formatString);
     * // Succeeds
     * if (format.GetComponentMask() == CS::StructuredTextureFormat::compRGB)
     * { ... }
     * // Would also succeed for formatString = "bgr8" or even wierd formats like 
     * // "gbr4" and more, but not if an additional component such as alpha is
     * // present.
     * \endcode
     */
    uint GetComponentMask () const;
  };
  
  /**
   * Texture format string parser routines.
   */
  class CS_CRYSTALSPACE_EXPORT TextureFormatStrings
  {
  public:
    /**
     * Convert a texture format to a canonical form. 
     * \sa \ref TextureFormatStrings, \ref tfs_g_canonical
     */
    static csString ConvertCanonical (const char* in);

    /**
     * Convert a (canonical or not) texture format to a structured form.
     * \sa \ref TextureFormatStrings
     */
    static StructuredTextureFormat ConvertStructured (const char* in);
  };
}

/** @} */

#endif // __CS_CSGFX_TEXTUREFORMATSTRINGS_H__

