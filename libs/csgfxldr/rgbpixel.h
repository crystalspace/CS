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
//    sizeof(RGBPixel) == 4       (byte:rgb + byte:pad)
//    sizeof(RGBPalEntry) == 8    (byte:rgb + long:count)
//
// Originally, RGBPixel and RGBPalEntry were implemented as subclasses of
// RGBcolor.  RGBPixel added a byte-sized "pad" variable, and RGBPalEntry added
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

/// An RGB color.
struct RGBcolor
{
  unsigned char red, green, blue;
  RGBcolor () : red(0), green(0), blue(0) {}
  RGBcolor (unsigned char r, unsigned char g, unsigned char b) :
    red(r), green(g), blue(b) {}
  bool operator == (const RGBcolor& c) const
  { return (c.red == red) && (c.green == green) && (c.blue == blue); }
  bool operator != (const RGBcolor& c) const
  { return !operator == (c); }
  RGBcolor operator + (const RGBcolor& c) const
  { return RGBcolor (c.red + red, c.green + green, c.blue + blue); }
};

/// An RGB pixel.
struct RGBPixel
{
  unsigned char red, green, blue, pad;
  RGBPixel () : red(0), green(0), blue(0), pad(0) {}
  RGBPixel (const RGBcolor& c) :
    red (c.red), green (c.green), blue (c.blue), pad (0) {}
  RGBPixel (const RGBPixel& p) :
    red (p.red), green (p.green), blue (p.blue), pad (p.pad) {}
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

/// An RGB palette entry with statistics information.
struct RGBPalEntry
{
  unsigned char red, green, blue;
  long count;
  RGBPalEntry () : red(0), green(0), blue(0), count(0) {}
  RGBPalEntry (const RGBcolor& c) :
    red (c.red), green (c.green), blue (c.blue), count (0) {}
  RGBPalEntry (const RGBPalEntry& e) :
    red (e.red), green (e.green), blue (e.blue), count (e.count) {}
  bool operator == (const RGBcolor& c) const
  { return (c.red == red) && (c.green == green) && (c.blue == blue); }
  bool operator == (const RGBPalEntry& e) const
  { return (e.red == red) && (e.green == green) && (e.blue == blue) && (e.count == count); }
  bool operator != (const RGBcolor& c) const
  { return !operator == (c); }
  bool operator != (const RGBPalEntry& e) const
  { return !operator==(e); }
  operator RGBcolor () const
  { return RGBcolor (red, green, blue); }
};

#endif // RGBPIXEL_H
