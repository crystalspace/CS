/*
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

#include "cssysdef.h"
#include "csutil/radixsort.h"

void csRadixSorter::Sort (uint32* table_source, uint32* table_dest,
	int size, uint32 bit_mask, int shift)
{
  int i;
  // Reset the counter table.
  memset (counter_table, 0, 256 * sizeof (uint32));

  // Count the distribution of all elements.
  for (i = 0 ; i < size ; i++)
  {
    int val = (table_source[i] & bit_mask) >> shift;
    counter_table[val]++;
  }

  // Update the offset table.
  int offset = 0;
  for (i = 0 ; i < 256 ; i++)
  {
    offset_table[i] = offset;
    offset += counter_table[i];
  }

  // Sort the elements to the destination table based on the current mask.
  for (i = 0 ; i < size ; i++)
  {
    int val = (table_source[i] & bit_mask) >> shift;
    table_dest[offset_table[val]++] = table_source[i];
  }
}

void csRadixSorter::Sort (uint32* table, int size, int max_value)
{
  uint32* table_copy = new uint32[size];
  Sort (table, table_copy, size, 0x000000ff, 0);
  if (max_value > 255)
  {
    Sort (table_copy, table, size, 0x0000ff00, 8);
    if (max_value > 65535)
    {
      Sort (table, table_copy, size, 0x00ff0000, 16);
      if (max_value > 16777215)
        Sort (table_copy, table, size, 0xff000000, 24);
      else
        memcpy (table, table_copy, size * sizeof (uint32));
    }
  }
  else
  {
    memcpy (table, table_copy, size * sizeof (uint32));
  }
  delete[] table_copy;
}


