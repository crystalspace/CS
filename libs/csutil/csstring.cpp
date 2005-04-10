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

//-----------------------------------------------------------------------------
// GLOBAL NOTES
//
// *1* Plus one to account for implicit null byte.
// *2* Retrieve data pointer _after_ ExpandIfNeeded() rather than before since
//     ExpandIfNeeded() may relocate the string buffer.
//-----------------------------------------------------------------------------

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

void csStringBase::SetCapacityInternal (size_t NewSize, bool soft)
{
  NewSize++; // GLOBAL NOTE *1*
  if (soft)
    NewSize = ComputeNewSize (NewSize);
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
  if (NewSize + 1 > GetCapacity() + 1) // GLOBAL NOTE *1*
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

class csStringFmtWriter
{
  csStringBase& str;
public:
  csStringFmtWriter (csStringBase& str) : str (str) {}
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
  csStringFmtWriter writer (*this);
  csFmtDefaultReader<utf8_char> reader ((utf8_char*)format, strlen (format));
  csPrintfFormatter<csStringFmtWriter, csFmtDefaultReader<utf8_char> >
    formatter (&reader, args);
  formatter.Format (writer);

  // csStringBase is capable of storing any binary data, including null bytes.
  // csPrintfFormatter() always appends a null terminator for the convenience
  // of raw string buffers (char[]), but csStringBase already maintains its own
  // null terminator one position beyond the string length. We discard the
  // final null added by csPrintfFormatter() because we do not want it to be
  // considered by Length() as actual string data. csStringBase's own final
  // null suffices as a suitable string terminator.
  if (!IsEmpty())
    Truncate(Length() - 1);

  return *this;
}

void csStringBase::ExpandIfNeeded(size_t NewSize)
{
  if (GetData() == 0 || NewSize + 1 > GetCapacity() + 1) // GLOBAL NOTE *1*
    SetCapacityInternal (NewSize, true);
}

void csStringBase::SetGrowsBy (size_t n)
{
  if (n == 0)
    GrowBy = 0; // Meaning `grow exponentially'.
  else
  {
    if (n < DEFAULT_GROW_BY)
      n = DEFAULT_GROW_BY;
    // Round `n' up to multiple of DEFAULT_GROW_BY.
    GrowBy = (n + DEFAULT_GROW_BY - 1) & ~(DEFAULT_GROW_BY - 1);
  }
}

void csStringBase::ShrinkBestFit()
{
  if (Size == 0)
    Free();
  else
  {
    CS_ASSERT(Data != 0);
    MaxSize = Size + 1; // GLOBAL NOTE *1*
    char* s = new char[MaxSize];
    memcpy(s, Data, MaxSize);
    delete[] Data;
    Data = s;
  }
}

csStringBase &csStringBase::Truncate (size_t iPos)
{
  if (iPos < Size)
  {
    Size = iPos;
    GetDataMutable() [Size] = '\0';
  }
  return *this;
}

csStringBase &csStringBase::DeleteAt (size_t iPos, size_t iCount)
{
  if (iCount <= 0) return *this;
  CS_ASSERT (iPos < Size && iPos + iCount <= Size);
  char* p = GetDataMutable();
  if (p != 0)
  {
    if (iPos + iCount < Size)
      memmove(p + iPos, p + iPos + iCount, Size - (iPos + iCount));
    Size -= iCount;
    p[Size] = '\0';
  }
  return *this;
}

csStringBase &csStringBase::Insert (size_t iPos, const csStringBase &iStr)
{
  CS_ASSERT(iPos <= Size);

  if (GetData() == 0 || iPos == Size)
    return Append (iStr);

  size_t const sl = iStr.Length ();
  size_t const NewSize = sl + Size;
  ExpandIfNeeded (NewSize);
  char* p = GetDataMutable();                         // GLOBAL NOTE *2*
  memmove (p + iPos + sl, p + iPos, Size - iPos + 1); // GLOBAL NOTE *1*
  memcpy (p + iPos, iStr.GetData (), sl);
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

  if (GetData() == 0 || iPos == Size)
    return Append (str);

  size_t const sl = strlen (str);
  size_t const NewSize = sl + Size;
  ExpandIfNeeded (NewSize);
  char* p = GetDataMutable();                         // GLOBAL NOTE *2*
  memmove (p + iPos + sl, p + iPos, Size - iPos + 1); // GLOBAL NOTE *1*
  memcpy (p + iPos, str, sl);
  Size = NewSize;
  return *this;
}

csStringBase &csStringBase::Overwrite (size_t iPos, const csStringBase &iStr)
{
  CS_ASSERT (iPos <= Size);

  if (GetData() == 0 || iPos == Size)
    return Append (iStr);

  size_t const sl = iStr.Length ();
  size_t const NewSize = iPos + sl;
  ExpandIfNeeded (NewSize);
  char* p = GetDataMutable();                 // GLOBAL NOTE *2*
  memcpy (p + iPos, iStr.GetData (), sl + 1); // GLOBAL NOTE *1*
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
  char* p = GetDataMutable();
  if (Str == 0 || Count == 0)
    Free();
  else if (p != 0 && Str >= p && Str < p + Size) // Pathalogical cases
  {
    if (Count == (size_t)-1) Count = Size - (Str - p);
    if (Str == p && Count < Size)	// i.e. `s.Replace(s.GetData(), n)'
      Truncate(Count);
    else if (Str > p)			// i.e. `s.Replace(s.GetData() + n)'
    {
      memmove(p, Str, Count);
      p[Count] = '\0';
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
  return Append(iStr.GetData(), iCount == (size_t)-1 ? iStr.Length() : iCount);
}

csStringBase &csStringBase::Append (const char *iStr, size_t iCount)
{
  if (iStr == 0 || iCount == 0)
    return *this;
  if (iCount == (size_t)-1)
    iCount = strlen (iStr);

  size_t const NewSize = Size + iCount;
  ExpandIfNeeded (NewSize);
  char* p = GetDataMutable(); // GLOBAL NOTE *2*
  CS_ASSERT(p != 0);
  memcpy (p + Size, iStr, iCount);
  Size = NewSize;
  p [Size] = '\0';
  return *this;
}

void csStringBase::SubString (csStringBase& sub, size_t x, size_t len) const
{
  CS_ASSERT(sub.GetData() != GetData()); // Check for same string
  sub.Truncate(0);
  // XXX Matze: we should rather assert or throw an exception in case the x and
  // len parameters are wrong...
  if (x < Size)
  {
    if (x + len > Size)
      len = Size - x;
    sub.Append(GetData() + x, len);
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
  char const* p = GetData();
  if (pos > Size || p == 0)
    return (size_t)-1;

  char const* tmp = strchr(p + pos, c);
  if (!tmp) 
    return (size_t)-1;

  return tmp - p;
}

size_t csStringBase::FindFirst (const char *c, size_t pos) const
{
  char const* p = GetData();
  if (pos > Size || p == 0)
    return (size_t)-1;

  char const* tmp = strpbrk(p + pos, c);
  if (!tmp)
    return (size_t)-1;

  return tmp - p;
}

size_t csStringBase::FindLast (char c, size_t pos) const
{
  char const* p = GetData();
  if (pos == (size_t)-1)
    pos = Size - 1;

  if (pos > Size || p == 0)
    return (size_t)-1;

  char const* tmp;
  for (tmp = p + pos; tmp >= p; tmp--)
    if (*tmp == c)
      return tmp - p;

  return (size_t)-1;
}

size_t csStringBase::Find (const char* str, size_t pos) const
{
  char const* p = GetData();
  if (pos > Size || p == 0)
    return (size_t)-1;

  char const* tmp = strstr (p + pos, str);
  if (!tmp) 
    return (size_t)-1;

  return tmp - p;
}

void csStringBase::ReplaceAll (const char* str, const char* replaceWith)
{
  csStringBase newStr;
  size_t p = 0;
  const size_t strLen = strlen (str);
  char* x = GetDataMutable();

  while (true)
  {
    size_t strPos = Find (str, p);
    if (strPos == (size_t)-1)
      break;
    newStr.Append (x + p, strPos - p);
    newStr.Append (replaceWith);
    p = strPos + strLen;
  }
  newStr.Append (x + p, Size - p);
  Replace (newStr);
}

// Note: isalpha(int c),  toupper(int c), tolower(int c), isspace(int c)
// If c is not an unsigned char value, or EOF, the behaviour of these functions
// is undefined.
csStringBase& csStringBase::Downcase()
{
  char* p = GetDataMutable();
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
  char* p = GetDataMutable();
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
  char const* const p = GetData();
  for (i = 0; i < Size; i++)
    if (!isspace ((unsigned char)p[i]))
      break;
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
    char const* const p = GetData();
    CS_ASSERT(p != 0);
    const char* c;
    for (c = p + Size - 1; c != p; c--)
      if (!isspace ((unsigned char)*c))
        break;
    size_t i = c - p;
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
    char* p = GetDataMutable();
    CS_ASSERT(p != 0);
    char const* src = p;
    char const* slim = p + Size;
    char* dst = p;
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
        if (saw_white && dst > p)
          *dst++ = ' ';
        *dst++ = c;
        saw_white = false;
      }
    }
    Size = dst - p;
    p[Size] = '\0';
  }
  return *this;
}

csStringBase &csStringBase::FormatV (const char *format, va_list args)
{
  Truncate(0);
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
    char* p = GetDataMutable();          // GLOBAL NOTE *2*
    CS_ASSERT(p != 0);
    const size_t toInsert = iNewSize - Size;
    memmove (p + toInsert, p, Size + 1); // GLOBAL NOTE *1*
    for (size_t x = 0; x < toInsert; x++)
      p [x] = iChar;
    Size = iNewSize;
  }
  return *this;
}

csStringBase& csStringBase::PadRight (size_t iNewSize, char iChar)
{
  if (iNewSize > Size)
  {
    ExpandIfNeeded (iNewSize);
    char* p = GetDataMutable(); // GLOBAL NOTE *2*
    CS_ASSERT(p != 0);
    for (size_t x = Size; x < iNewSize; x++)
      p [x] = iChar;
    Size = iNewSize;
    p [Size] = '\0';
  }
  return *this;
}

csStringBase& csStringBase::PadCenter (size_t iNewSize, char iChar)
{
  if (iNewSize > Size)
  {
    ExpandIfNeeded (iNewSize);
    char* p = GetDataMutable(); // GLOBAL NOTE *2*
    CS_ASSERT(p != 0);
    const size_t toInsert = iNewSize - Size;
    const size_t halfInsert = toInsert / 2;
    if (Size > 0)
      memmove (p + halfInsert, p, Size);
    size_t x;
    for (x = 0; x < halfInsert; x++)
      p [x] = iChar;
    for (x = halfInsert + Size; x < iNewSize; x++)
      p [x] = iChar;
    Size = iNewSize;
    p [Size] = '\0';
  }
  return *this;
}
