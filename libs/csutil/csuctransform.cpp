/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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
#include "csutil/csuctransform.h"

#include "csucmappings.h"

#define _OUTPUT_CHAR(buf, chr)				\
  if (bufRemaining > 0)					\
  {							\
    if(buf) *buf++ = chr;				\
    bufRemaining--;					\
  }							\
  encodedLen++;

static size_t ApplyMapping (const UCMapEntry* map, const size_t mapSize,
			    const utf16_char* mapAux, const utf32_char ch, 
			    utf32_char* dest, size_t destSize)
{
  size_t bufRemaining = destSize;
  size_t encodedLen = 0;

  // Search for the source char in the mapping table.
  size_t l = 0, r = mapSize;
  while (l < r)
  {
    size_t m = (l + r) / 2;
    const UCMapEntry& entry = map[m];
    if (entry.mapFrom == ch)
    {
      // Found it...
      if ((entry.mapTo & 0xff000000) == 0)
      {
	// Simple mapping
	_OUTPUT_CHAR(dest, entry.mapTo);
      }
      else
      {
	// Complex mapping (multiple chars)
	size_t auxLen = entry.mapTo >> 24;
	size_t auxOffs = entry.mapTo & 0xffffff;
	utf32_char outCh;
	// Decode from the auxilary data.
	while (auxLen > 0)
	{
	  int n = csUnicodeTransform::UTF16Decode (mapAux + auxOffs, 
	    CS_UC_MAX_UTF16_ENCODED, outCh);
	  _OUTPUT_CHAR(dest, outCh);
	  auxLen -= n;
	  auxOffs += n;
	}
      }
      return encodedLen;
    }
    if (entry.mapFrom < ch)
      l = m + 1;
    else
      r = m;
  }
  // Nothing found? Return source char.
  _OUTPUT_CHAR(dest, ch);
  return encodedLen;
}

size_t csUnicodeTransform::MapToUpper (const utf32_char ch, utf32_char* dest, 
				       size_t destSize)
{
  return ApplyMapping (mapUpper, sizeof (mapUpper) / sizeof (UCMapEntry),
    mapUpper_aux, ch, dest, destSize);
}

size_t csUnicodeTransform::MapToLower (const utf32_char ch, utf32_char* dest, 
				       size_t destSize)
{
  return ApplyMapping (mapLower, sizeof (mapLower) / sizeof (UCMapEntry),
    mapLower_aux, ch, dest, destSize);
}

size_t csUnicodeTransform::MapToFold (const utf32_char ch, utf32_char* dest, 
				      size_t destSize)
{
  return ApplyMapping (mapFold, sizeof (mapFold) / sizeof (UCMapEntry),
    mapFold_aux, ch, dest, destSize);
}
