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
#include "csutil/formatter.h"

csStringBase::~csStringBase ()
{
  Free ();
}

void csStringBase::Free ()
{
  delete[] Data;
  Data = 0;
  Size = 0;
  MaxSize = 0;
}

void csStringBase::SetCapacityInternal (size_t NewSize, bool extraSpace)
{
  if (extraSpace)
    NewSize = ComputeNewSize (NewSize);
  NewSize++; // Plus one for implicit null byte.
  MaxSize = NewSize;
  char* buff = new char[MaxSize];
  if (Data == 0 || Size == 0)
    buff[0] = '\0';
  else
    memcpy(buff, Data, Size + 1);

  delete[] Data;
  Data = buff;
}

size_t csStringBase::ComputeNewSize (size_t NewSize)
{
  size_t n;
  if (GrowBy != 0)
    n = (NewSize + GrowBy - 1) & ~(GrowBy - 1);
  else
  {
    n = MaxSize != 0 ? MaxSize << 1 : size_t(DEFAULT_GROW_BY);
    while (n < NewSize)
      n <<= 1;
  }
  return n;
}

void csStringBase::SetCapacity (size_t NewSize)
{
  if (NewSize < MaxSize)
    return;
  SetCapacityInternal (NewSize, false);
}

csStringBase& csStringBase::AppendFmt (const char* format, ...)
{
  va_list args;
  va_start (args, format);
  AppendFmtV (format, args);
  va_end (args);
  return *this;
}

class FmtStringWriter
{
  csStringBase& str;
public:
  FmtStringWriter (csStringBase& str) : str (str) {}
  void Put (utf32_char ch) 
  { 
    utf8_char dest[CS_UC_MAX_UTF8_ENCODED];
    size_t n = (size_t)csUnicodeTransform::Encode (ch, dest, 
      sizeof (dest) / sizeof (utf8_char));
    str.Append ((char*)dest, n);
  }
  size_t GetTotal() const { return str.Length(); }
};

csStringBase& csStringBase::AppendFmtV (const char* format, va_list args)
{
  FmtStringWriter writer (*this);
  csFmtDefaultReader<utf8_char> reader ((utf8_char*)format, strlen (format));
  csPrintfFormatter<FmtStringWriter, csFmtDefaultReader<utf8_char> >
    formatter (&reader, args);
  formatter.Format (writer);
  return *this;
}

// These 'long long' methods are not inline since "%ll" and "%llu" are not
// compatible with gcc's -ansi and -pedantic options which external projects
// may employ; thus we can not use them in public headers.
csStringBase& csStringBase::Append (longlong v)
{
  return AppendFmt ("%lld", v);
}
csStringBase& csStringBase::Append (ulonglong v)
{
  return AppendFmt ("%llu", v);
}

void csStringBase::ExpandIfNeeded(size_t NewSize)
{
  if (NewSize + 1 > MaxSize)
  {
    SetCapacityInternal (NewSize, true);
  }
}

void csStringBase::SetGrowsBy (size_t n)
{
  if (n < DEFAULT_GROW_BY)
    n = DEFAULT_GROW_BY;
  // Round `n' up to multiple of DEFAULT_GROW_BY.
  GrowBy = (n + DEFAULT_GROW_BY - 1) & ~(DEFAULT_GROW_BY - 1);
}

csStringBase &csStringBase::ShrinkBestFit()
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

csStringBase &csStringBase::Truncate (size_t iPos)
{
  if (iPos < Size)
  {
    CS_ASSERT(Data != 0);
    Size = iPos;
    Data [Size] = '\0';
  }
  return *this;
}

csStringBase &csStringBase::DeleteAt (size_t iPos, size_t iCount)
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

csStringBase &csStringBase::Insert (size_t iPos, const csStringBase &iStr)
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

csStringBase &csStringBase::Insert (size_t iPos, char iChar)
{
  csStringBase s(iChar);
  return Insert(iPos, s);
}

csStringBase &csStringBase::Insert (size_t iPos, const char* str)
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

csStringBase &csStringBase::Overwrite (size_t iPos, const csStringBase &iStr)
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

csStringBase& csStringBase::Replace (const csStringBase& Str, size_t Count)
{
  if (this != &Str)
    Replace(Str.GetData(), Count);
  else if (Count != (size_t)-1 && Count < Length())
    Truncate(Count);
  return *this;
}

csStringBase& csStringBase::Replace (const char* Str, size_t Count)
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

csStringBase &csStringBase::Append (const csStringBase &iStr, size_t iCount)
{
  return Append(iStr.GetData(), iCount);
}

csStringBase &csStringBase::Append (const char *iStr, size_t iCount)
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

void csStringBase::SubString (csStringBase& sub, size_t x, size_t len) const
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

csStringBase csStringBase::Slice(size_t start, size_t len) const
{
  csStringBase s;
  SubString(s, start, len);
  return s;
}

size_t csStringBase::FindFirst (char c, size_t pos) const
{
  if (pos > Size || Data == 0)
    return (size_t)-1;

  char const* tmp = strchr(Data + pos, c);
  if (!tmp) 
    return (size_t)-1;

  return tmp - Data;
}

size_t csStringBase::FindFirst (const char *c, size_t pos) const
{
  if (pos > Size || Data == 0)
    return (size_t)-1;

  char const* tmp = strpbrk(Data + pos, c);
  if (!tmp)
    return (size_t)-1;

  return tmp - Data;
}

size_t csStringBase::FindLast (char c, size_t pos) const
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

size_t csStringBase::Find (const char* str, size_t pos) const
{
  if (pos > Size || Data == 0)
    return (size_t)-1;

  char const* tmp = strstr (Data + pos, str);
  if (!tmp) 
    return (size_t)-1;

  return tmp - Data;
}

void csStringBase::ReplaceAll (const char* str, const char* replaceWith)
{
  csStringBase newStr;

  size_t p = 0;
  const size_t strLen = strlen (str);

  while (true)
  {
    size_t strPos = Find (str, p);
    if (strPos == (size_t)-1)
      break;
    newStr.Append (Data + p, strPos - p);
    newStr.Append (replaceWith);
    p = strPos + strLen;
  }
  newStr.Append (Data + p, Size - p);

  Replace (newStr);
}

// Note: isalpha(int c),  toupper(int c), tolower(int c), isspace(int c)
// If c is not an unsigned char value, or EOF, the behaviour of these functions
// is undefined.
csStringBase& csStringBase::Downcase()
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
csStringBase& csStringBase::Upcase()
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
csStringBase &csStringBase::LTrim()
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
csStringBase &csStringBase::RTrim()
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

csStringBase &csStringBase::Trim()
{
  return LTrim().RTrim();
}

csStringBase &csStringBase::Collapse()
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

csStringBase &csStringBase::FormatV (const char *format, va_list args)
{
  Size = 0;
  return AppendFmtV (format, args);
}

csStringBase &csStringBase::Format (const char* format, ...)
{
  va_list args;
  va_start (args, format);
  FormatV (format, args);
  va_end (args);
  return *this;
}

csStringBase &csStringBase::PadLeft (size_t iNewSize, char iChar)
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

csStringBase& csStringBase::PadRight (size_t iNewSize, char iChar)
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

csStringBase& csStringBase::PadCenter (size_t iNewSize, char iChar)
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
