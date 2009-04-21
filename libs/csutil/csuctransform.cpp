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
			    utf32_char* dest, size_t destSize, 
                            bool simpleMapping)
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
      utf32_char toChar = entry.mapTo & 0x1fffff;
      // Found it...
      if ((entry.mapTo & (1 << 31)) == 0)
      {
	// Simple mapping
	_OUTPUT_CHAR(dest, toChar);
      }
      else
      {
	// Complex mapping (multiple chars)
	size_t auxLen = (entry.mapTo >> 21) & 0x1ff;
	size_t auxOffs = toChar;
        size_t simpleLen = ((entry.mapTo >> 30) & 1) + 1;
	utf32_char outCh;
	// Decode from the auxilary data.
        if (simpleMapping)
        {
	  csUnicodeTransform::UTF16Decode (mapAux + auxOffs, 
	    simpleLen, outCh);
	  _OUTPUT_CHAR(dest, outCh);
        }
        else
        {
          auxOffs += simpleLen;
	  while (auxLen > 0)
	  {
	    int n = csUnicodeTransform::UTF16Decode (mapAux + auxOffs, 
	      auxLen, outCh);
	    _OUTPUT_CHAR(dest, outCh);
	    auxLen -= n;
	    auxOffs += n;
	  }
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
				       size_t destSize, uint flags)
{
  return ApplyMapping (mapUpper, sizeof (mapUpper) / sizeof (UCMapEntry),
    mapUpper_aux, ch, dest, destSize, flags & csUcMapSimple);
}

size_t csUnicodeTransform::MapToLower (const utf32_char ch, utf32_char* dest, 
				       size_t destSize, uint flags)
{
  return ApplyMapping (mapLower, sizeof (mapLower) / sizeof (UCMapEntry),
    mapLower_aux, ch, dest, destSize, flags & csUcMapSimple);
}

size_t csUnicodeTransform::MapToFold (const utf32_char ch, utf32_char* dest, 
				      size_t destSize, uint flags)
{
  return ApplyMapping (mapFold, sizeof (mapFold) / sizeof (UCMapEntry),
    mapFold_aux, ch, dest, destSize, flags & csUcMapSimple);
}
