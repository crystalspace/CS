/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Contributions made by Ivan Avramovic <ivan@avramovic.com>

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

/**\file
 * RGB colors.
 */
/**\addtogroup gfx
 * @{
 */
//-----------------------------------------------------------------------------
// Implementation Note: Eric Sunshine <sunshine@sunshineco.com>      1999/02/09
//
// Certain portions of the Crystal Space code have strict requirements about
// the sizes of the structures csRGBcolor and csRGBpixel.  In particular, some
// pieces of code make these assumptions:
//
//    sizeof(csRGBcolor) == 3  (byte:rgb)
//    sizeof(csRGBpixel) == 4  (byte:rgb + byte:alpha)
//
// Originally, csRGBpixel was implemented as a subclasse of csRGBcolor and
// added a byte-sized "alpha" variable.  Thus, the original implementation made
// the assumption that the compiler would not pad out the csRGBcolor structure.
//
// Unfortunately in some environments (such as the NextStep compiler on M68K
// hardware) the compiler does pad csRGBcolor thus breaking the original
// assumptions about structure sizes.  In such cases, csRGBcolor is padded out
// to 4 bytes instead of 3 and csRGBpixel is padded out to 6 bytes instead of
// 4.  This padding results in problems in code which makes assumptions about
// the sizes of each structure.  In practice, problems were observed in code
// which expected csRGBpixel to be 4 bytes.
//
// To work around this problem, csRGBpixel has been re-implemented so that it
// is no longer derived from csRGBcolor.  An unfortunate side-effect of this
// re-implementation is that code is no longer inherited, and is thus
// duplicated in each class.  However, except for this minor point, the size of
// each structure should now be more stable between various compilers.
//-----------------------------------------------------------------------------

#ifndef __CS_RGBPIXEL_H__
#define __CS_RGBPIXEL_H__

#include "csextern.h"
#include "csutil/comparator.h"


/**
 * An RGB color. This class is used whenever we need just R, G and B
 * information, such as when defining a color palette.
 */
CS_STRUCT_ALIGN_4BYTE_BEGIN
struct csRGBcolor
{
  /// The red, green, blue components
  unsigned char red, green, blue;
  /// Constructor (initialize to zero)
  csRGBcolor () : red(0), green(0), blue(0) {}
  /// Initialize the color with some R/G/B value
  csRGBcolor (unsigned char r, unsigned char g, unsigned char b) :
    red(r), green(g), blue(b) {}
  /// Assign given red/green/blue values to this pixel
  void Set (unsigned char r, unsigned char g, unsigned char b)
  { red = r; green = g; blue = b; }
  /// Compare with an csRGBcolor
  bool operator == (const csRGBcolor& c) const
  { return (c.red == red) && (c.green == green) && (c.blue == blue); }
  /// Compare with an csRGBcolor
  bool operator != (const csRGBcolor& c) const
  { return !operator == (c); }
  /// add two csRGBcolors
  csRGBcolor operator + (const csRGBcolor& c) const
  { return csRGBcolor (
    (unsigned char)(c.red + red),
    (unsigned char)(c.green + green),
    (unsigned char)(c.blue + blue)); }
} CS_STRUCT_ALIGN_4BYTE_END;


/**
 * csComparator<> specialization for csRGBcolor.
 */
CS_SPECIALIZE_TEMPLATE
class csComparator<csRGBcolor, csRGBcolor>
{
public:
  static int Compare (csRGBcolor const& r1, csRGBcolor const& r2)
  {
    if (r1.red != r2.red)
      return (int)r1.red - (int)r2.red;
    if (r1.green != r2.green)
      return (int)r1.green - (int)r2.green;
    if (r1.blue != r2.blue)
      return (int)r1.blue - (int)r2.blue;
    return 0;
  }
};

/**
 * csComparator<> specialization for csColor*. Mostly for compatibility with
 * now-defunct csRGBVector class.
 */
CS_SPECIALIZE_TEMPLATE
class csComparator<csRGBcolor*, csRGBcolor*> 
{
public:
  static int Compare (csRGBcolor* const& r1, csRGBcolor* const& r2)
  { return csComparator<csRGBcolor, csRGBcolor>::Compare (*r1, *r2); }
};

/**
 * An RGB pixel. In addition to R,G,B color components this structure also
 * contains the Alpha channel component, which is used in images
 * (that potentially have an alpha channel).
 */
CS_STRUCT_ALIGN_4BYTE_BEGIN
struct csRGBpixel
{
  /// The red, green, blue and alpha components
  unsigned char red, green, blue, alpha;
  /// Constructor (initialize to zero, alpha to 255)
  csRGBpixel () : red(0), green(0), blue(0), alpha(255) {}
  /// Copy constructor
  csRGBpixel (const csRGBpixel& p) :
    red (p.red), green (p.green), blue (p.blue), alpha (p.alpha) {}
  /// Construct from a csRGBcolor object; sets alpha to 255.
  csRGBpixel (const csRGBcolor& c) :
    red (c.red), green (c.green), blue (c.blue), alpha (255) {}
  /// Initialize the pixel with some R/G/B/A value.
  csRGBpixel (int r, int g, int b, int a = 255) :
    red ((unsigned char)r),
    green ((unsigned char)g),
    blue ((unsigned char)b),
    alpha ((unsigned char)a) {}
  /// Compare with an csRGBcolor
  bool operator == (const csRGBcolor& c) const
  { return (c.red == red) && (c.green == green) && (c.blue == blue); }
  /// Compare with an csRGBpixel (including alpha value)
  bool operator == (const csRGBpixel& p) const
  { return (p.red == red) && (p.green == green) && (p.blue == blue); }
  /// Check if the csRGBpixel is not equal to an csRGBcolor
  bool operator != (const csRGBcolor& c) const
  { return !operator == (c); }
  /**
   * Check if this csRGBpixel is not equal to another csRGBpixel (including
   * alpha).
   */
  bool operator != (const csRGBpixel& p) const
  { return !operator == (p); }
  /// Construct an csRGBcolor from this csRGBpixel
  operator csRGBcolor () const
  { return csRGBcolor (red, green, blue); }
  /// Compare with another csRGBpixel, but don't take alpha into account
  bool eq (const csRGBpixel& p) const
  { return operator==(p); }
  /// Get the pixel intensity
  int Intensity () const
  { return (red + green + blue) / 3; }
  /// Get the pixel luminance
  unsigned char Luminance () const
  {
    return (unsigned char)(((int)red*30 + (int)green*59 + (int)blue*11) / 100);
  }
  /// Assign given red/green/blue/alpha values to this pixel
  void Set (const int r, const int g, const int b, const int a = 255)
  {
    red = (unsigned char)r;
    green = (unsigned char)g;
    blue = (unsigned char)b;
    alpha = (unsigned char)a;
  }
  /// Assign another csRGBpixel
  void Set (const csRGBpixel& p)
  { red = p.red; green = p.green; blue = p.blue; alpha = p.alpha; }
  /// Add a csRGBcolor
  void operator += (const csRGBcolor& c)
  {
      red   = (unsigned char)(red   + c.red  );
      green = (unsigned char)(green + c.green);
      blue  = (unsigned char)(blue  + c.blue );
  }
  /**
   * Use this only if you know there is no overflow. Also take
   * care to set alpha to 0 in 'c'!
   */
  void UnsafeAdd (const csRGBpixel& c)
  {
    red   = (unsigned char)(red   + c.red  );
    green = (unsigned char)(green + c.green);
    blue  = (unsigned char)(blue  + c.blue );
  }
  /**
   * Use this in the general case. This version test for overflow.
   * Note that alpha is ignored by this routine.
   */
  void SafeAdd (const csRGBpixel& c)
  {
    int color = red + c.red;
    red   = (unsigned char)(color > 255 ? 255 : color);
    color = green + c.green;
    green = (unsigned char)(color > 255 ? 255 : color);
    color = blue + c.blue;
    blue  = (unsigned char)(color > 255 ? 255 : color);
  }
} CS_STRUCT_ALIGN_4BYTE_END;


/** \name Eye sensivity to different color components
 * Eye sensivity to different color components, from NTSC grayscale equation.
 * The coefficients are multiplied by 100 and rounded towards nearest integer,
 * to facilitate integer math. The squared coefficients are also multiplied
 * by 100 and rounded to nearest integer (thus 173 == 1.73, 242 == 2.42 etc).
 * @{ */
/// Red component sensivity
#define R_COEF		173
/// Green component sensivity
#define G_COEF		242
/// Blue component sensivity
#define B_COEF		107
/** @} */
/**\name Eye sensivity to different color components, squared
 * @{ */
/// Red component sensivity, squared
#define R_COEF_SQ	299
/// Green component sensivity, squared
#define G_COEF_SQ	587
/// Blue component sensivity, squared
#define B_COEF_SQ	114
/** @} */

/** @} */

#endif // __CS_RGBPIXEL_H__
