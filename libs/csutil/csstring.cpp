/*
    Crystal Space utility library: string class
    Copyright (C) 1999,2000 by Andrew Zabolotny <bit@eltech.ru>

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

extern "C" {
#include <ctype.h>
#include <stdarg.h>
}
#include "cssysdef.h"
#include "csutil/csstring.h"
#include "csutil/snprintf.h"

csString::~csString ()
{
  Free ();
}

void csString::SetCapacity (size_t NewSize)
{
  NewSize++;
  if (NewSize == MaxSize)
    return;

  MaxSize = NewSize;
  Data = (char *)realloc (Data, MaxSize);

  if (Size >= MaxSize)
  {
    Size = MaxSize - 1;
    Data [Size] = 0;
  }
}

csString &csString::Truncate (size_t iPos)
{
  if (iPos < Size)
  {
    Size = iPos;
    Data [Size] = 0;
  }
  return *this;
}

csString &csString::DeleteAt (size_t iPos, size_t iCount)
{
#ifdef CS_DEBUG
  if (iPos > Size || iPos + iCount > Size)
    STR_FATAL (("Tried to delete characters beyond the end of the string!\n"))
#endif
  memmove(Data + iPos, Data + iPos + iCount, Size - (iPos + iCount));
  Size = Size - iCount;
  Data[Size] = 0;
  return *this;
}

csString &csString::Insert (size_t iPos, const csString &iStr)
{
#ifdef CS_DEBUG
  if (iPos > Size)
    STR_FATAL (("Inserting `%s' into `%s' at position %lu\n",
      iStr.GetData (), Data, (unsigned long)iPos))
#endif

  if (Data == NULL)
  {
    Append (iStr);
    return *this;
  }

  size_t sl = iStr.Length ();
  size_t NewSize = sl + Length ();
  if (NewSize >= MaxSize)
    SetCapacity (NewSize);
  memmove (Data + iPos + sl, Data + iPos, Size - iPos);
  memcpy (Data + iPos, iStr.GetData (), sl);
  Data [Size = NewSize] = 0;

  return *this;
}

csString &csString::Insert (size_t iPos, const char iChar)
{
#ifdef CS_DEBUG
  if (iPos > Size)
    STR_FATAL (("Inserting `%c' into `%s' at position %lu\n",
      iChar, Data, (unsigned long)iPos))
#endif

  if (Data == NULL)
  {
    Append (iChar);
    return *this;
  }

  size_t NewSize = 1 + Length ();
  if (NewSize >= MaxSize)
    SetCapacity (NewSize);
  memmove (Data + iPos + 1, Data + iPos, Size - iPos);
  Data[iPos] = iChar;
  Data [Size = NewSize] = 0;

  return *this;
}

csString &csString::Overwrite (size_t iPos, const csString &iStr)
{
#ifdef CS_DEBUG
  if (iPos > Size)
    STR_FATAL (("Overwriting `%s' into `%s' at position %lu\n",
      iStr.GetData (), Data, (unsigned long)iPos))
#endif

  if (Data == NULL)
  {
    Append (iStr);
    return *this;
  }

  size_t sl = iStr.Length ();
  size_t NewSize = iPos + sl;
  if (NewSize >= MaxSize)
    SetCapacity (NewSize);
  memcpy (Data + iPos, iStr.GetData (), sl);
  Data [Size = NewSize] = 0;

  return *this;
}

csString &csString::Append (const csString &iStr, size_t iCount)
{
  if (iCount == (size_t)-1)
    iCount = iStr.Length ();

  if (!iCount)
    return *this;

  size_t NewSize = Size + iCount;
  if (NewSize >= MaxSize)
    SetCapacity (NewSize);

  memcpy (Data + Size, iStr.GetData (), iCount);
  Data [Size = NewSize] = 0;

  return *this;
}

csString &csString::Append (const char *iStr, size_t iCount)
{
  if (!iStr) return *this;
  if (iCount == (size_t)-1)
    iCount = strlen (iStr);

  if (!iCount)
    return *this;

  size_t NewSize = Size + iCount;
  if (NewSize >= MaxSize)
    SetCapacity (NewSize);

  memcpy (Data + Size, iStr, iCount);
  Data [Size = NewSize] = 0;

  return *this;
}

csString &csString::LTrim()
{
  size_t i;
  for (i = 0; i < Size; i++)
  {
    if (!isspace (Data[i]))
      return DeleteAt (0, i);
  }
  return *this;
}

csString &csString::RTrim()
{
  if (Size == 0) return *this;

  int i;
  for(i = Size - 1; i >= 0; i--)
  {
    if (!isspace (Data[i]))
    {
      i++;
      return DeleteAt (i, Size - i);
    }
  }
  return *this;
}

csString &csString::Trim()
{
  return LTrim().RTrim();
}

csString &csString::Collapse()
{
  if (Size == 0) return *this;

  size_t start = (size_t) -1;
  Trim();
  size_t i;
  for (i = 1; i < Size - 1; i++)
  {
    if (isspace (Data[i]))
    {
      if (start==(size_t) -1)
      {
		start = i + 1;
		Data[i] = ' '; // Force 'space' as opposed to anything isspace()able.
      }
    }
    else
    {
      // Delete any extra whitespace
      if (start != (size_t)-1 && start != i)
      {
		DeleteAt (start, i - start);
		i -= i - start;
      }
      start = (size_t) -1;
    }
  }
  return *this;
}

csString &csString::Format(const char *format, ...)
{
  va_list args;
  int NewSize = -1;

  va_start(args, format);

  // Keep trying until the buffer is big enough to hold the entire string
  while(NewSize < 0)
  {
    NewSize = cs_vsnprintf(Data, MaxSize, format, args);
    // Increasing by the size of the format streams seems logical enough
    if(NewSize < 0)
      SetCapacity(MaxSize + strlen(format));
    // In this case we know what size it wants
    if((size_t) NewSize>=MaxSize)
    {
      SetCapacity(NewSize + 1);
      NewSize = -1; // Don't break the while loop just yet!
    } 
  }

  // Add in the terminating NULL
  Size = NewSize + 1;
  va_end(args);
  return *this;

}

#define STR_FORMAT(TYPE,FMT,SZ) \
csString csString::Format (TYPE v) \
{ char s[SZ]; cs_snprintf (s, SZ, #FMT, v); return csString ().Append (s); }
  STR_FORMAT(short, %hd, 32)
  STR_FORMAT(unsigned short, %hu, 32)
  STR_FORMAT(int, %d, 32)
  STR_FORMAT(unsigned int, %u, 32)
  STR_FORMAT(long, %ld, 32)
  STR_FORMAT(unsigned long, %lu, 32)
  STR_FORMAT(float, %g, 64)
  STR_FORMAT(double, %g, 64)
#undef STR_FORMAT

#define STR_FORMAT_INT(TYPE,FMT) \
csString csString::Format (TYPE v, int width, int prec/*=0*/) \
{ char s[32], s1[32]; \
  cs_snprintf (s1, 32, "%%%d.%d"#FMT, width, prec); cs_snprintf (s, 32, s1, v); \
  return csString ().Append (s); }
  STR_FORMAT_INT(short, hd)
  STR_FORMAT_INT(unsigned short, hu)
  STR_FORMAT_INT(int, d)
  STR_FORMAT_INT(unsigned int, u)
  STR_FORMAT_INT(long, ld)
  STR_FORMAT_INT(unsigned long, lu)
#undef STR_FORMAT_INT

#define STR_FORMAT_FLOAT(TYPE) \
csString csString::Format (TYPE v, int width, int prec/*=6*/) \
{ char s[64], s1[32]; \
  cs_snprintf (s1, 32, "%%%d.%dg", width, prec); cs_snprintf (s, 64, s1, v); \
  return csString ().Append (s); }
  STR_FORMAT_FLOAT(float)
  STR_FORMAT_FLOAT(double)
#undef STR_FORMAT_FLOAT

csString &csString::PadLeft (size_t iNewSize, char iChar)
{
  if (Size < iNewSize)
  {
    SetCapacity (iNewSize);
    if (Size == 0) return *this;
    const size_t toInsert = iNewSize - Size;

    memmove (Data + toInsert, Data, Size);
    size_t x;
    for (x = 0; x < toInsert; x++)
    {
      Data [x] = iChar;
    }
    Data [iNewSize] = '\0';
    Size = iNewSize;
  }
  return *this;
}

csString csString::AsPadLeft (size_t iNewSize, char iChar)
{
  csString newStr = Clone ();
  newStr.PadLeft (iChar, iNewSize);
  return newStr;
}

#define STR_PADLEFT(TYPE) \
csString csString::PadLeft (TYPE v, size_t iNewSize, char iChar) \
{ csString newStr; return newStr.Append (v).PadLeft (iNewSize, iChar); }
  STR_PADLEFT(const csString&)
  STR_PADLEFT(const char*)
  STR_PADLEFT(char)
  STR_PADLEFT(unsigned char)
  STR_PADLEFT(short)
  STR_PADLEFT(unsigned short)
  STR_PADLEFT(int)
  STR_PADLEFT(unsigned int)
  STR_PADLEFT(long)
  STR_PADLEFT(unsigned long)
  STR_PADLEFT(float)
  STR_PADLEFT(double)
#if !defined(CS_USE_FAKE_BOOL_TYPE)
  STR_PADLEFT(bool)
#endif
#undef STR_PADLEFT

csString& csString::PadRight (size_t iNewSize, char iChar)
{
  if (Size < iNewSize)
  {
    SetCapacity (iNewSize);
    size_t x;
    for (x = Size; x < iNewSize; x++)
      Data [x] = iChar;
    Data [iNewSize] = '\0';
    Size = iNewSize;
  }
  return *this;
}

csString csString::AsPadRight (size_t iNewSize, char iChar)
{
  csString newStr = Clone ();
  newStr.PadRight (iChar, iNewSize);
  return newStr;
}

#define STR_PADRIGHT(TYPE) \
csString csString::PadRight (TYPE v, size_t iNewSize, char iChar) \
{ csString newStr; return newStr.Append (v).PadRight (iNewSize, iChar); }
  STR_PADRIGHT(const csString&)
  STR_PADRIGHT(const char*)
  STR_PADRIGHT(char)
  STR_PADRIGHT(unsigned char)
  STR_PADRIGHT(short)
  STR_PADRIGHT(unsigned short)
  STR_PADRIGHT(int)
  STR_PADRIGHT(unsigned int)
  STR_PADRIGHT(long)
  STR_PADRIGHT(unsigned long)
  STR_PADRIGHT(float)
  STR_PADRIGHT(double)
#if !defined(CS_USE_FAKE_BOOL_TYPE)
  STR_PADRIGHT(bool)
#endif
#undef STR_PADRIGHT

csString& csString::PadCenter (size_t iNewSize, char iChar)
{
  if (Size < iNewSize)
  {
    SetCapacity (iNewSize);
    if (Size == 0) return *this;
    const size_t toInsert = iNewSize - Size;

    memmove (Data + toInsert / 2 + toInsert % 2, Data, Size);

    size_t x;
    for (x = 0; x < toInsert / 2 + toInsert % 2; x++)
    {
      Data [x] = iChar;
    }
    for (x = iNewSize - toInsert / 2; x < iNewSize; x++)
    {
      Data [x] = iChar;
    }
    Data [iNewSize] = '\0';
    Size = iNewSize;
  }
  return *this;
}

csString csString::AsPadCenter (size_t iNewSize, char iChar)
{
  csString newStr = Clone ();
  newStr.PadCenter (iChar, iNewSize);
  return newStr;
}

#define STR_PADCENTER(TYPE) \
csString csString::PadCenter (TYPE v, size_t iNewSize, char iChar) \
{ csString newStr; return newStr.Append (v).PadCenter (iNewSize, iChar); }
  STR_PADCENTER(const csString&)
  STR_PADCENTER(const char*)
  STR_PADCENTER(char)
  STR_PADCENTER(unsigned char)
  STR_PADCENTER(short)
  STR_PADCENTER(unsigned short)
  STR_PADCENTER(int)
  STR_PADCENTER(unsigned int)
  STR_PADCENTER(long)
  STR_PADCENTER(unsigned long)
  STR_PADCENTER(float)
  STR_PADCENTER(double)
#if !defined(CS_USE_FAKE_BOOL_TYPE)
  STR_PADCENTER(bool)
#endif
#undef STR_PADCENTER
