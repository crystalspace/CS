/*
  Copyright (C) 2007 by Frank Richter

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

#include "xr.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLRead)
{

/* Character types, from XML 1.0 spec.
 * We're really only interested whether a character can be a first or 
 * subsequent character for a name, so that's the info recorded here.
 */

#define NAME_FIRST        0x01
#define NAME_SUBSEQ       0x02

// Shortcuts
#define F   (NAME_SUBSEQ | NAME_FIRST)
#define S   NAME_SUBSEQ

// Properties for a row of characters
#define CHAR_ROW(c0,c1,c2,c3,c4,c5,c6,c7,c8,c9,cA,cB,cC,cD,cE,cF) \
  (c0 <<  0) | (c1 <<  2) | (c2 <<  4) | (c3 <<  6) |             \
  (c4 <<  8) | (c5 << 10) | (c6 << 12) | (c7 << 14) |             \
  (c8 << 16) | (c9 << 18) | (cA << 20) | (cB << 22) |             \
  (cC << 24) | (cD << 26) | (cE << 28) | (cF << 30)

static const uint32 charTypeISO_8859_1[] = {
  //       0 1 2 3 4 5 6 7 8 9 A B C D E F
  CHAR_ROW(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0), // 00-0f
  CHAR_ROW(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0), // 10-1f
  CHAR_ROW(0,0,0,0,0,0,0,0,0,0,0,0,0,S,S,0), // 20-2f
  CHAR_ROW(S,S,S,S,S,S,S,S,S,S,F,0,0,0,0,0), // 30-3f
  CHAR_ROW(0,F,F,F,F,F,F,F,F,F,F,F,F,F,F,F), // 40-4f
  CHAR_ROW(F,F,F,F,F,F,F,F,F,F,F,0,0,0,0,F), // 50-5f
  CHAR_ROW(0,F,F,F,F,F,F,F,F,F,F,F,F,F,F,F), // 60-6f
  CHAR_ROW(F,F,F,F,F,F,F,F,F,F,F,0,0,0,0,0), // 70-7f
  CHAR_ROW(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0), // 80-8f
  CHAR_ROW(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0), // 90-9f
  CHAR_ROW(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0), // a0-af
  CHAR_ROW(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0), // b0-bf
  CHAR_ROW(F,F,F,F,F,F,F,F,F,F,F,F,F,F,F,F), // c0-cf
  CHAR_ROW(F,F,F,F,F,F,F,0,F,F,F,F,F,F,F,F), // d0-df
  CHAR_ROW(F,F,F,F,F,F,F,F,F,F,F,F,F,F,F,F), // e0-ef
  CHAR_ROW(F,F,F,F,F,F,F,0,F,F,F,F,F,F,F,F), // f0-ff
};

#undef CHAR_ROW
#undef F
#undef S

bool TrDocument::IsNameStart_ISO_8859_1 (const char* p)
{
  unsigned char c = (unsigned char)*p;
  return (charTypeISO_8859_1[c >> 4] >> 2*(c & 0xf)) & NAME_FIRST;
}

char* TrDocument::ReadName_ISO_8859_1 (char* p)
{
  if (!p || !*p) return 0;
  unsigned char c = (unsigned char)*p++;
  if (!(charTypeISO_8859_1[c >> 4] >> 2*(c & 0xf)) & NAME_FIRST) return 0;

  do
  {
    c = (unsigned char)*p++;
  }
  while ((charTypeISO_8859_1[c >> 4] >> 2*(c & 0xf)) & NAME_SUBSEQ);

  return p-1;
}

}
CS_PLUGIN_NAMESPACE_END(XMLRead)
