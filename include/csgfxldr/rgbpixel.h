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

//-----------------------------------------------------------------------------
// Implementation Note: Eric Sunshine <sunshine@sunshineco.com>      1999/02/09
//
// Certain portions of the Crystal Space code have strict requirements about
// the sizes of the structures RGBcolor, RGBPixel, and RGBPalEntry.  In
// particular, some pieces of code make these assumptions:
//
//    sizeof(RGBcolor) == 3       (byte:rgb)
//    sizeof(RGBPixel) == 4       (byte:rgb + byte:alpha)
//    sizeof(RGBPalEntry) == 8    (byte:rgb + long:count)
//
// Originally, RGBPixel and RGBPalEntry were implemented as subclasses of
// RGBcolor.  RGBPixel added a byte-sized "alpha" variable, and RGBPalEntry added
// a longword-sized "count" variable.  Thus, the original implementation made
// the assumption that the compiler would not pad out the RGBcolor structure.
//
// Unfortunately in some environments (such as the NextStep compiler on m68k
// hardware) the compiler does pad RGBcolor thus breaking the original
// assumptions about structure sizes.  In such cases, RGBcolor is padded out
// to 4 bytes instead of 3 and RGBPixel is padded out to 6 bytes instead of 4.
//
// This padding results in problems in code which makes assumptions about the
// sizes of each structure.  In practice, problems were observed in code which
// expected RGBPixel to be 4 bytes.
//
// To work around this problem, I re-implemented RGBPixel and RGBPalEntry such
// that they are no longer derived from RGBcolor.  An unfortunate side-effect
// of this re-implementation is that code is no longer inherited, and is thus
// duplicated in each class.  However, except for this minor point, the size of
// each structure should now be more stable between various compilers.
//-----------------------------------------------------------------------------

#ifndef RGBPIXEL_H
#define RGBPIXEL_H

#include <stdio.h>
#include "types.h"

/**
 * An RGB color. This class is used whenever we need just R, G and B
 * information, such as when defining a color palette.
 */
struct RGBcolor
{
  unsigned char red, green, blue;
  RGBcolor () : red(0), green(0), blue(0) {}
  RGBcolor (unsigned char r, unsigned char g, unsigned char b) :
    red(r), green(g), blue(b) {}
  void Set (unsigned char r, unsigned char g, unsigned char b)
  { red = r; green = g; blue = b; }
  bool operator == (const RGBcolor& c) const
  { return (c.red == red) && (c.green == green) && (c.blue == blue); }
  bool operator != (const RGBcolor& c) const
  { return !operator == (c); }
  RGBcolor operator + (const RGBcolor& c) const
  { return RGBcolor (c.red + red, c.green + green, c.blue + blue); }
};

/**
 * An RGB pixel. Besides R,G,B color components this structure also
 * contains the Alpha channel component, which is used in images
 * (that potentially have an alpha channel).
 */
struct RGBPixel
{
  unsigned char red, green, blue, alpha;
  RGBPixel () : red(0), green(0), blue(0), alpha(255) {}
  RGBPixel (const RGBcolor& c) :
    red (c.red), green (c.green), blue (c.blue), alpha (255) {}
  RGBPixel (const RGBPixel& p) :
    red (p.red), green (p.green), blue (p.blue), alpha (p.alpha) {}
  bool operator == (const RGBcolor& c) const
  { return (c.red == red) && (c.green == green) && (c.blue == blue); }
  bool operator == (const RGBPixel& p) const
  { return (p.red == red) && (p.green == green) && (p.blue == blue); }
  bool operator != (const RGBcolor& c) const
  { return !operator == (c); }
  bool operator != (const RGBPixel& p) const
  { return !operator == (p); }
  operator RGBcolor () const
  { return RGBcolor (red, green, blue); }
};

/**
 * Eye sensivity to different color components, from NTSC grayscale equation.
 * The coefficients are multiplied by 100 and rounded towards nearest integer,
 * to facilitate integer math. The squared coefficients are also multiplied
 * by 100 and rounded to nearest integer (thus 173 == 1.73, 242 == 2.42 etc).
 */
/// Red component sensivity
#define R_COEF		173
/// Green component sensivity
#define G_COEF		242
/// Blue component sensivity
#define B_COEF		107
/// Eye sensivity to different color components, squared
/// Red component sensivity, squared
#define R_COEF_SQ	299
/// Green component sensivity, squared
#define G_COEF_SQ	587
/// Blue component sensivity, squared
#define B_COEF_SQ	114

#endif // RGBPIXEL_H
