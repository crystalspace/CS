/*
  Crystal Space In-Place Radix Sort
  Copyright (C) 2003 by Jorrit Tyberghein

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

#ifndef __CSUTIL_RADIX_SORT_H__
#define __CSUTIL_RADIX_SORT_H__

/**\file
 * 32-bit unsigned integer in-place radix-sorter
 */

#include "csextern.h"

/**
 * An in-place radix-sorter for a raw array of 32-bit unsigned integers.
 */
class CS_CRYSTALSPACE_EXPORT csRadixSorter
{
private:
  // Element counter table. This is used to count the
  // distribution of all elements in the input table (for the
  // given part of the elements we are sorting).
  int counter_table[256];
  // Offset table where every element starts in the sorted result.
  int offset_table[256];

  // Sort from one table to the other on the given bit-mask.
  void Sort (uint32* table_source, uint32* table_dest, int size,
  	uint32 bit_mask, int shift);

public:
  /**
   * Sort a given table numerically. If you know the maximum value
   * of all elements in the array you can pass that to Sort. It will
   * then try to minimize the passes based on that max_value.
   */
  void Sort (uint32* table, int size, int max_value);
};

#endif
