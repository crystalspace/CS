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

void csString::Free ()
  {
  if (Data)
  {
    delete[] Data;
    Data = 0;
    Size = 0;
    MaxSize = 0;
  }
}

void csString::SetCapacity (size_t NewSize)
{
  NewSize++; // Plus one for implicit null byte.
  if (NewSize <= MaxSize)
    return;
  MaxSize = NewSize;

  char* buff = new char[MaxSize];
  if (Data == 0 || Size == 0)
    buff[0] = '\0';
  else
    memcpy(buff, Data, Size + 1);

  if (Data)
    delete[] Data;
  Data = buff;
}

csString &csString::Reclaim()
{
  if (Size == 0)
    Free();
  else
  {
    CS_ASSERT(Data != 0);
    MaxSize = Size + 1; // Plus one for implicit null byte.
    char* s = new char[MaxSize];
    memcpy(s, Data, MaxSize);
    delete[] Data;
    Data = s;
  }
  return *this;
}

csString &csString::Truncate (size_t iPos)
{
  if (iPos < Size)
  {
    Size = iPos;
    Data [Size] = '\0';
  }
  return *this;
}

csString &csString::DeleteAt (size_t iPos, size_t iCount)
{
  CS_ASSERT (iPos < Size && iPos + iCount <= Size);

  if (iPos + iCount < Size)
    memmove(Data + iPos, Data + iPos + iCount, Size - (iPos + iCount));
  Size -= iCount;
  Data[Size] = '\0';
  return *this;
}

csString &csString::Insert (size_t iPos, const csString &iStr)
{
  CS_ASSERT(iPos <= Size);

  if (Data == 0 || iPos == Size)
    return Append (iStr);

  size_t const sl = iStr.Length ();
  size_t const NewSize = sl + Size;
  SetCapacity (NewSize);
  memmove (Data + iPos + sl, Data + iPos, Size - iPos + 1); // Also move null.
  memcpy (Data + iPos, iStr.GetData (), sl);
  Size = NewSize;
  return *this;
}

csString &csString::Insert (size_t iPos, const char iChar)
{
  csString s(iChar);
  return Insert(iPos, s);
}

csString &csString::Overwrite (size_t iPos, const csString &iStr)
{
  CS_ASSERT (iPos <= Size);

  if (Data == 0 || iPos == Size)
    return Append (iStr);

  size_t const sl = iStr.Length ();
  size_t const NewSize = iPos + sl;
  SetCapacity (NewSize);
  memcpy (Data + iPos, iStr.GetData (), sl + 1); // Also copy null terminator.
  Size = NewSize;
  return *this;
}

csString &csString::Append (const csString &iStr, size_t iCount)
{
  return Append(iStr.GetData(), iCount);
}

csString &csString::Append (const char *iStr, size_t iCount)
{
  if (iStr == 0)
    return *this;
  if (iCount == (size_t)-1)
    iCount = strlen (iStr);
  if (iCount == 0)
    return *this;

  size_t const NewSize = Size + iCount;
  SetCapacity (NewSize);
  memcpy (Data + Size, iStr, iCount);
  Size = NewSize;
  Data [Size] = '\0';
  return *this;
}

csString &csString::LTrim()
{
  size_t i;
  for (i = 0; i < Size; i++)
    if (!isspace (Data[i]))
      break;
  if (i > 0)
    DeleteAt (0, i);
  return *this;
}

csString &csString::RTrim()
{
  if (Size > 0)
  {
    int i;
    for (i = Size - 1; i >= 0; i--)
      if (!isspace (Data[i]))
        break;
    if (i < int(Size - 1))
      Truncate(i + 1);
  }
  return *this;
}

csString &csString::Trim()
{
  return LTrim().RTrim();
}

csString &csString::Collapse()
{
  if (Size > 0)
  {
    char const* src = Data;
    char const* slim = Data + Size;
    char* dst = Data;
    bool saw_white = false;
    for ( ; src < slim; src++)
    {
      char const c = *src;
      if (isspace(c))
        saw_white = true;
      else
      {
        if (saw_white && dst > Data)
          *dst++ = ' ';
        *dst++ = c;
        saw_white = false;
      }
    }
    Size = dst - Data;
    Data[Size] = '\0';
  }
  return *this;
}

csString &csString::Format(const char *format, ...)
{
  va_list args;
  va_start(args, format);

  if (Data == 0) // Ensure that backing-store exists prior to vsnprintf().
    SetCapacity(255);

  int rc = 0;
  while (1)
  {
    rc = cs_vsnprintf(Data, MaxSize, format, args);
    // Buffer was big enough for entire string?
    if (rc >= 0 && rc < (int)MaxSize)
      break;
    // Some vsnprintf()s return -1 on failure, others return desired capacity.
    if (rc >= 0)
      SetCapacity(rc); // SetCapacity() ensures room for null byte.
    else
      SetCapacity(MaxSize * 2 - 1);
  }

  Size = rc;
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
{ char s[64], s1[64]; \
  cs_snprintf (s1, sizeof(s1), "%%%d.%d"#FMT, width, prec); \
  cs_snprintf (s, sizeof(s), s1, v); \
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
{ char s[64], s1[64]; \
  cs_snprintf (s1, sizeof(s1), "%%%d.%dg", width, prec); \
  cs_snprintf (s, sizeof(s), s1, v); \
  return csString ().Append (s); }
  STR_FORMAT_FLOAT(float)
  STR_FORMAT_FLOAT(double)
#undef STR_FORMAT_FLOAT

csString &csString::PadLeft (size_t iNewSize, char iChar)
{
  if (iNewSize > Size)
  {
    SetCapacity (iNewSize);
    const size_t toInsert = iNewSize - Size;
    memmove (Data + toInsert, Data, Size + 1); // Also move null terminator.
    for (size_t x = 0; x < toInsert; x++)
      Data [x] = iChar;
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
  if (iNewSize > Size)
  {
    SetCapacity (iNewSize);
    for (size_t x = Size; x < iNewSize; x++)
      Data [x] = iChar;
    Size = iNewSize;
    Data [Size] = '\0';
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
  if (iNewSize > Size)
  {
    SetCapacity (iNewSize);
    const size_t toInsert = iNewSize - Size;
    const size_t halfInsert = toInsert / 2;
    if (Size > 0)
      memmove (Data + halfInsert, Data, Size);
    size_t x;
    for (x = 0; x < halfInsert; x++)
      Data [x] = iChar;
    for (x = halfInsert + Size; x < iNewSize; x++)
      Data [x] = iChar;
    Size = iNewSize;
    Data [Size] = '\0';
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
