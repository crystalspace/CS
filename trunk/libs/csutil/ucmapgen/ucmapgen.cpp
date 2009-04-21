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
#include "csutil/array.h"
#include "csutil/csunicode.h"
#include "csutil/csuctransform.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/hash.h"
#include "csutil/physfile.h"
#include "csutil/stringreader.h"
#include "csutil/sysfunc.h"

CS_IMPLEMENT_APPLICATION

struct CharMap
{
  struct CharMapping
  {
    utf32_char from;
    utf32_char simpleTo;
    csArray<utf32_char> complexTo;

    CharMapping() : simpleTo(0) {}
  };
  csArray<CharMapping> mappings;

  static int MappingCompare (const CharMapping& m1, const CharMapping& m2)
  {
    if (m1.from < m2.from)
      return -1;
    else if (m1.from > m2.from)
      return 1;
    else
      return 0;
  }

  void Insert (utf32_char code, utf32_char mapTo)
  {
    CharMapping newMapping;
    newMapping.from = code;
    size_t index = mappings.FindSortedKey (csArrayCmp<CharMapping, CharMapping> (
      newMapping, MappingCompare));
    if (index != csArrayItemNotFound)
    {
      CharMapping& mapping = mappings[index];
      mapping.simpleTo = mapTo;
    }
    else
    {
      newMapping.simpleTo = mapTo;
      mappings.InsertSorted (newMapping, &MappingCompare);
    }
  }
  void Insert (utf32_char code, const csArray<utf32_char>& mapTo)
  {
    CharMapping newMapping;
    newMapping.from = code;
    size_t index = mappings.FindSortedKey (csArrayCmp<CharMapping, CharMapping> (
      newMapping, MappingCompare));
    if (index != csArrayItemNotFound)
    {
      CharMapping& mapping = mappings[index];
      mapping.complexTo = mapTo;
    }
    else
    {
      newMapping.complexTo = mapTo;
      mappings.InsertSorted (newMapping, &MappingCompare);
    }
  }
};
typedef csArray<const char*> UniDBEntry;

static void ParseEntry (csString& line, UniDBEntry& entry)
{
  entry.Empty();

  const char* lineStr = line.GetData();
  const size_t lineLen = line.Length();
  size_t pos = 0;
  while (pos < lineLen)
  {
    const char* itemStart = lineStr + pos;
    while (*itemStart == ' ') itemStart++;
    entry.Push (itemStart);
    pos = line.FindFirst (';', pos);
    if (pos == (size_t)-1) break;
    line.SetAt (pos++, 0);
  }
}

void WriteMapToFile (FILE* file, const CharMap& map, const char* varPref)
{
  csArray<utf16_char> auxdata;
  csFPrintf (file, "static const UCMapEntry %s[] = {\n", varPref);
  for (size_t i = 0; i < map.mappings.Length(); i++)
  {
    const CharMap::CharMapping curMapping = map.mappings[i];
    uint outGlyph;
    if (curMapping.complexTo.Length() > 0)
    {
      outGlyph = (uint)auxdata.Length() | (1 << 31);

      utf16_char toEnc[CS_UC_MAX_UTF16_ENCODED];
      int n = csUnicodeTransform::EncodeUTF16 (
        (curMapping.simpleTo != 0) ? curMapping.simpleTo : curMapping.from, 
        toEnc, sizeof (toEnc) / sizeof (utf16_char));
      for (int k = 0; k < n; k++) auxdata.Push (toEnc[k]);

      outGlyph |= ((n-1) << 30);

      size_t simpLen = 0;
      for (size_t j = 0; j < curMapping.complexTo.Length(); j++)
      {
	n = csUnicodeTransform::EncodeUTF16 (curMapping.complexTo[j], toEnc,
	  sizeof (toEnc) / sizeof (utf16_char));
	for (int k = 0; k < n; k++) auxdata.Push (toEnc[k]);
        simpLen += n;
      }
      outGlyph |= (simpLen << 21);
    }
    else
      outGlyph = curMapping.simpleTo;
    csFPrintf (file, "  {0x%.4" PRIx32 ", 0x%.4x},\n", 
      curMapping.from, outGlyph);
  }
  csFPrintf (file, "};\n");
  csFPrintf (file, "static const utf16_char %s_aux[] = {\n", varPref);
  if (auxdata.Length() == 0)
    csFPrintf (file, "  0\n");
  else
  {
    for (size_t i = 0; i < auxdata.Length(); i++)
    {
      csFPrintf (file, "  0x%.4" PRIx16 ",\n", auxdata[i]);
    }
  }
  csFPrintf (file, "};\n");
}

int main (int argc, char* argv[])
{
  csPhysicalFile unicodeData ("UnicodeData.txt", "rb");
  if (unicodeData.GetStatus() != VFS_STATUS_OK)
  {
    csPrintf ("Couldn't open UnicodeData.txt\n");
    return -1;
  }

  csPhysicalFile specialCasing ("SpecialCasing.txt", "rb");
  if (specialCasing.GetStatus() != VFS_STATUS_OK)
  {
    csPrintf ("Couldn't open SpecialCasing.txt\n");
    return -1;
  }

  csPhysicalFile caseFolding ("CaseFolding.txt", "rb");
  if (caseFolding.GetStatus() != VFS_STATUS_OK)
  {
    csPrintf ("Couldn't open CaseFolding.txt\n");
    return -1;
  }

  CharMap uppercaseMap;
  CharMap lowercaseMap;
  CharMap foldMap;
  {
    csRef<iDataBuffer> ucData = unicodeData.GetAllData (true);
    csStringReader ucDataReader (ucData->GetData());

    csString line;
    UniDBEntry entry;
    while (ucDataReader.GetLine (line))
    {
      ParseEntry (line, entry);
      int charCode;
      sscanf (entry[0], "%x", &charCode);
      if (strlen (entry[12]) > 0)
      {
	int upChar;
	sscanf (entry[12], "%x", &upChar);
	uppercaseMap.Insert (charCode, upChar);
      }
      if (strlen (entry[13]) > 0)
      {
	int lowChar;
	sscanf (entry[13], "%x", &lowChar);
	lowercaseMap.Insert (charCode, lowChar);
      }
    }
  }
  {
    csRef<iDataBuffer> scData = specialCasing.GetAllData (true);
    csStringReader scDataReader (scData->GetData());

    csString line;
    UniDBEntry entry;
    csArray<utf32_char> mapTo;
    while (scDataReader.GetLine (line))
    {
      if ((line.Length() == 0) || (line.GetAt(0) == '#')) continue;
      ParseEntry (line, entry);
      if (entry[4][0] != '#') continue;
      int charCode;
      sscanf (entry[0], "%x", &charCode);
      mapTo.Empty();
      {
	int code;
	const char* curPos = entry[1];
	while (true)
	{
	  sscanf (curPos, "%x", &code);
	  mapTo.Push (code);
	  curPos = strchr (curPos, ' ');
	  if (curPos == 0) break;
	  curPos++;
	}
	if ((mapTo.Length() > 1) || (mapTo[0] != charCode))
        {
          if (mapTo.Length() > 1)
	    lowercaseMap.Insert (charCode, mapTo);
          else
	    lowercaseMap.Insert (charCode, mapTo[0]);
        }
      }
      mapTo.Empty();
      {
	int code;
	const char* curPos = entry[3];
	while (true)
	{
	  sscanf (curPos, "%x", &code);
	  mapTo.Push (code);
	  curPos = strchr (curPos, ' ');
	  if (curPos == 0) break;
	  curPos++;
	}
	if ((mapTo.Length() > 1) || (mapTo[0] != charCode))
        {
          if (mapTo.Length() > 1)
	    uppercaseMap.Insert (charCode, mapTo);
          else
	    uppercaseMap.Insert (charCode, mapTo[0]);
        }
      }
    }
  }
  {
    csRef<iDataBuffer> cfData = caseFolding.GetAllData (true);
    csStringReader cfDataReader (cfData->GetData());

    csString line;
    UniDBEntry entry;
    csArray<utf32_char> mapTo;
    while (cfDataReader.GetLine (line))
    {
      if ((line.Length() == 0) || (line.GetAt(0) == '#')) continue;
      ParseEntry (line, entry);
      if (entry[1][0] == 'T') continue;
      int charCode;
      sscanf (entry[0], "%x", &charCode);
      mapTo.Empty();
      {
	int code;
	const char* curPos = entry[2];
	while (true)
	{
	  sscanf (curPos, "%x", &code);
	  mapTo.Push (code);
	  curPos = strchr (curPos, ' ');
	  if (curPos == 0) break;
	  curPos++;
	}
        if (entry[1][0] == 'F')
	  foldMap.Insert (charCode, mapTo);
        else
	  foldMap.Insert (charCode, mapTo[0]);
      }
    }
  }

  {
    FILE* mappingsFile = fopen ("csucmappings.h", "w");

    csFPrintf (mappingsFile, "// Automatically generated\n");
    csFPrintf (mappingsFile, "struct UCMapEntry\n");
    csFPrintf (mappingsFile, "{\n");
    csFPrintf (mappingsFile, "  utf32_char mapFrom;\n");
    csFPrintf (mappingsFile, "  utf32_char mapTo;\n");
    csFPrintf (mappingsFile, "};\n");
    csFPrintf (mappingsFile, "\n");

    WriteMapToFile (mappingsFile, uppercaseMap, "mapUpper");
    csFPrintf (mappingsFile, "\n");
    WriteMapToFile (mappingsFile, lowercaseMap, "mapLower");
    csFPrintf (mappingsFile, "\n");
    WriteMapToFile (mappingsFile, foldMap, "mapFold");
    csFPrintf (mappingsFile, "\n");

    fclose (mappingsFile);
  }

  return 0;
}
