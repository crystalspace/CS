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

#include "cssysdef.h"
extern "C" 
{
#include <ctype.h>
#include <stdarg.h>
}
#include "csutil/csstring.h"
#include "csutil/snprintf.h"

csString::~csString ()
{
  Free ();
}

void csString::Free ()
{
  delete[] Data;
  Data = 0;
  Size = 0;
  MaxSize = 0;
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

void csString::ExpandIfNeeded(size_t NewSize)
{
  if (NewSize + 1 > MaxSize)
  {
    size_t n;
    if (!GrowExponentially)
      n = (NewSize + GrowBy - 1) & ~(GrowBy - 1);
    else
    {
      n = MaxSize != 0 ? MaxSize << 1 : size_t(DEFAULT_GROW_BY);
      while (n < NewSize)
        n <<= 1;
    }
    SetCapacity(n);
  }
}

void csString::Mug (csString& other)
{
  delete[] Data;
  Size = other.Size;
  MaxSize = other.MaxSize;
  Data = other.Detach();
}

void csString::SetGrowsBy(size_t n)
{
  if (n < DEFAULT_GROW_BY)
    n = DEFAULT_GROW_BY;
  // Round `n' up to multiple of DEFAULT_GROW_BY.
  GrowBy = (n + DEFAULT_GROW_BY - 1) & ~(DEFAULT_GROW_BY - 1);
  GrowExponentially = false;
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
    CS_ASSERT(Data != 0);
    Size = iPos;
    Data [Size] = '\0';
  }
  return *this;
}

csString &csString::DeleteAt (size_t iPos, size_t iCount)
{
  if (iCount <= 0) return *this;
  CS_ASSERT (iPos < Size && iPos + iCount <= Size);
  if (Data != 0)
  {
    if (iPos + iCount < Size)
      memmove(Data + iPos, Data + iPos + iCount, Size - (iPos + iCount));
    Size -= iCount;
    Data[Size] = '\0';
  }
  return *this;
}

csString &csString::Insert (size_t iPos, const csString &iStr)
{
  CS_ASSERT(iPos <= Size);

  if (Data == 0 || iPos == Size)
    return Append (iStr);

  size_t const sl = iStr.Length ();
  size_t const NewSize = sl + Size;
  ExpandIfNeeded (NewSize);
  memmove (Data + iPos + sl, Data + iPos, Size - iPos + 1); // Also move null.
  memcpy (Data + iPos, iStr.GetData (), sl);
  Size = NewSize;
  return *this;
}

csString &csString::Insert (size_t iPos, char iChar)
{
  csString s(iChar);
  return Insert(iPos, s);
}

csString &csString::Insert (size_t iPos, const char* str)
{
  CS_ASSERT(iPos <= Size);

  if (Data == 0 || iPos == Size)
    return Append (str);

  size_t const sl = strlen (str);
  size_t const NewSize = sl + Size;
  ExpandIfNeeded (NewSize);
  memmove (Data + iPos + sl, Data + iPos, Size - iPos + 1); // Also move null.
  memcpy (Data + iPos, str, sl);
  Size = NewSize;
  return *this;
}

csString &csString::Overwrite (size_t iPos, const csString &iStr)
{
  CS_ASSERT (iPos <= Size);

  if (Data == 0 || iPos == Size)
    return Append (iStr);

  size_t const sl = iStr.Length ();
  size_t const NewSize = iPos + sl;
  ExpandIfNeeded (NewSize);
  memcpy (Data + iPos, iStr.GetData (), sl + 1); // Also copy null terminator.
  Size = NewSize;
  return *this;
}

csString& csString::Replace (const csString& Str, size_t Count)
{
  if (this != &Str)
    Replace(Str.GetData(), Count);
  else if (Count != (size_t)-1 && Count < Length())
    Truncate(Count);
  return *this;
}

csString& csString::Replace (const char* Str, size_t Count)
{
  if (Str == 0 || Count == 0)
    Free();
  else if (Data != 0 && Str >= Data && Str < Data + Size) // Pathalogical cases
  {
    if (Count == (size_t)-1) Count = Size - (Str - Data);
    if (Str == Data && Count < Size)	// i.e. `s.Replace(s.GetData(), n)'
      Truncate(Count);
    else if (Str > Data)		// i.e. `s.Replace(s.GetData() + n)'
    {
      memmove(Data, Str, Count);
      Data[Count] = '\0';
      Size = Count;
    }
  }
  else
  {
    Truncate(0);
    Append (Str, Count);
  }
  return *this;
}

csString &csString::Append (const csString &iStr, size_t iCount)
{
  return Append(iStr.GetData(), iCount);
}

csString &csString::Append (const char *iStr, size_t iCount)
{
  if (iStr == 0 || iCount == 0)
    return *this;
  if (iCount == (size_t)-1)
    iCount = strlen (iStr);

  size_t const NewSize = Size + iCount;
  ExpandIfNeeded (NewSize);
  CS_ASSERT(Data != 0);
  memcpy (Data + Size, iStr, iCount);
  Size = NewSize;
  Data [Size] = '\0';
  return *this;
}

void csString::SubString (csString& sub, size_t x, size_t len) const
{
  CS_ASSERT(sub.Data != Data); // Check for same string
  sub.Truncate(0);
  // XXX Matze: we should rather assert or throw an exception in case the x and
  // len parameters are wrong...
  if (x < Size)
  {
    if (x + len > Size)
      len = Size - x;
    sub.Append(Data + x, len);
  }
}

csString csString::Slice(size_t start, size_t len) const
{
  csString s;
  SubString(s, start, len);
  return s;
}

size_t csString::FindFirst (char c, size_t pos) const
{
  if (pos > Size || Data == 0)
    return (size_t)-1;

  char const* tmp = strchr(Data + pos, c);
  if (!tmp) 
    return (size_t)-1;

  return tmp - Data;
}

size_t csString::FindFirst (const char *c, size_t pos) const
{
  if (pos > Size || Data == 0)
    return (size_t)-1;

  char const* tmp = strpbrk(Data + pos, c);
  if (!tmp)
    return (size_t)-1;

  return tmp - Data;
}

size_t csString::FindLast (char c, size_t pos) const
{
  if (pos == (size_t)-1)
    pos = Size - 1;

  if (pos > Size || Data == 0)
    return (size_t)-1;

  char const* tmp;
  for (tmp = Data + pos; tmp >= Data; tmp--)
    if (*tmp == c)
      return tmp - Data;

  return (size_t)-1;
}

size_t csString::FindStr (const char* str, size_t pos) const
{
  if (pos > Size || Data == 0)
    return (size_t)-1;

  char const* tmp = strstr (Data + pos, str);
  if (!tmp) 
    return (size_t)-1;

  return tmp - Data;
}

void csString::FindReplace (const char* str, const char* replaceWith)
{
  csString newStr;

  size_t p = 0;
  const size_t strLen = strlen (str);

  while (true)
  {
    size_t strPos = FindStr (str, p);
    if (strPos == (size_t)-1) break;
    newStr.Append (Data + p, strPos - p);
    newStr.Append (replaceWith);
    p = strPos + strLen;
  }
  newStr.Append (Data + p, Size - p);

  Mug (newStr);
}

// Note: isalpha(int c),  toupper(int c), tolower(int c), isspace(int c)
// If c is not an unsigned char value, or EOF, the behaviour of these functions
// is undefined.
csString& csString::Downcase()
{
  char* p = Data;
  if (p != 0)
  {
    char const* const pN = p + Length();
    for ( ; p < pN; p++)
      if (isalpha((unsigned char)(*p)))
        *p = (char)tolower((unsigned char)(*p));
  }
  return *this;
}

// Note: isalpha(int c),  toupper(int c), tolower(int c), isspace(int c)
// If c is not an unsigned char value, or EOF, the behaviour of these functions
// is undefined.
csString& csString::Upcase()
{
  char* p = Data;
  if (p != 0)
  {
    char const* const pN = p + Length();
    for ( ; p < pN; p++)
      if (isalpha((unsigned char)(*p)))
        *p = (char)toupper((unsigned char)(*p));
  }
  return *this;
}

// Note: isalpha(int c),  toupper(int c), tolower(int c), isspace(int c)
// If c is not an unsigned char value, or EOF, the behaviour of these functions
// is undefined.
csString &csString::LTrim()
{
  size_t i;
  for (i = 0; i < Size; i++)
  {
    // CS_ASSERT(Data != 0); -- commented out for efficiency.
    if (!isspace ((unsigned char)Data[i]))
      break;
  }
  if (i > 0)
    DeleteAt (0, i);
  return *this;
}

// Note: isalpha(int c),  toupper(int c), tolower(int c), isspace(int c)
// If c is not an unsigned char value, or EOF, the behaviour of these functions
// is undefined.
csString &csString::RTrim()
{
  if (Size > 0)
  {
    CS_ASSERT(Data != 0);
    const char* c;
    for (c = Data + Size - 1; c != Data; c--)
      if (!isspace ((unsigned char)*c))
        break;
    size_t i = c - Data;
    if (i < Size - 1)
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
    CS_ASSERT(Data != 0);
    char const* src = Data;
    char const* slim = Data + Size;
    char* dst = Data;
    bool saw_white = false;
    for ( ; src < slim; src++)
    {
      // if c is signed char isspace() may trigger an
      // assertion for chars >= 0x80
      unsigned char const c = *src;
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

csString &csString::FormatV (const char *format, va_list args)
{
  if (Data == 0) // Ensure that backing-store exists prior to vsnprintf().
    SetCapacity(255);

  int rc = 0;
  while (1)
  {
    va_list ap;
    CS_VA_COPY (ap, args);
    rc = cs_vsnprintf (Data, MaxSize, format, ap);
    va_end (ap);
    // Buffer was big enough for entire string?
    if (rc >= 0 && rc < (int)MaxSize)
      break;
    SetCapacity(MaxSize * 2);
  }
  Size = rc;
  return *this;
}

csString &csString::Format (const char* format, ...)
{
  va_list args;
  va_start (args, format);
  FormatV (format, args);
  va_end (args);
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
    ExpandIfNeeded (iNewSize);
    CS_ASSERT(Data != 0);
    const size_t toInsert = iNewSize - Size;
    memmove (Data + toInsert, Data, Size + 1); // Also move null terminator.
    for (size_t x = 0; x < toInsert; x++)
      Data [x] = iChar;
    Size = iNewSize;
  }
  return *this;
}

csString csString::AsPadLeft (size_t iNewSize, char iChar) const
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
  STR_PADLEFT(bool)
#undef STR_PADLEFT

csString& csString::PadRight (size_t iNewSize, char iChar)
{
  if (iNewSize > Size)
  {
    ExpandIfNeeded (iNewSize);
    CS_ASSERT(Data != 0);
    for (size_t x = Size; x < iNewSize; x++)
      Data [x] = iChar;
    Size = iNewSize;
    Data [Size] = '\0';
  }
  return *this;
}

csString csString::AsPadRight (size_t iNewSize, char iChar) const
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
  STR_PADRIGHT(bool)
#undef STR_PADRIGHT

csString& csString::PadCenter (size_t iNewSize, char iChar)
{
  if (iNewSize > Size)
  {
    ExpandIfNeeded (iNewSize);
    CS_ASSERT(Data != 0);
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

csString csString::AsPadCenter (size_t iNewSize, char iChar) const
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
  STR_PADCENTER(bool)
#undef STR_PADCENTER
