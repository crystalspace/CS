/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Contributions made by Ivan Avramovic <ivan@avramovic.com>

	ALMOST DEAD
	DO NOT MODIFY - THIS FILE WILL BE REMOVED SOON

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

#ifndef IMGTOOLS_H
#define IMGTOOLS_H

#include <stdio.h>
#include "types.h"
#include "csgfxldr/rgbpixel.h"

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

///
class ImageColorInfo
{
private:
  /**
   * Calculate the color table from the image stored in 'buf', length 'size'.
   * If the number of colors specified in 'ncol' is greater than zero, that
   * value is used.
   */
  void calc_table(const RGBPixel* buf, long size);

  /// palInd holds the pointer to an index table, each entry in the table 
  /// contains the index into color_table of that particular pixel. This 
  /// speeds up the palette con
  int num_colors, *palInd;
  ///
  RGBPalEntry* color_table;

public:
  /**
   * Construct the table using an image stored in 'buf' of length 'size'.
   * If the number of colors (ncol) is given and is greater than 0, that
   * value is used.
   */
  ImageColorInfo(const RGBPixel* buf, long size) { calc_table(buf,size); }

  ///
  ~ImageColorInfo();

  /**
   * Return a table with all colors used in the image.
   * This function returns a table with all colors that are used in the
   * image, together with a count of the number of times each color has
   * been used.  The table is sorted, with the most frequently occuring
   * colors first.
   */
  const RGBPalEntry* get_color_table () const { return color_table; }

  const int* get_palInd () const { return palInd; }

  /// Returns the number of colors in the image.
  int get_num_colors() const { return num_colors; }
};

#endif //IMGTOOLS_H
