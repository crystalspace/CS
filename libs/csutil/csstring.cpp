/*
    Copyright (C) 1998 by Jorrit Tyberghein
		CSScript module created by Brandon Ehle (Azverkan)

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

#include <ctype.h>
#include <stdarg.h>
#include "sysdef.h"
#include "csutil/csstring.h"

csString::~csString ()
{
  Free ();
}

void csString::SetSize (size_t NewSize)
{
  NewSize++;
  if (NewSize == MaxSize)
    return;

  Data = (char *)realloc (Data, MaxSize = NewSize);

  if (Size >= MaxSize)
    Size = MaxSize, Data [Size] = 0;
}

csString &csString::DeleteAt (size_t iPos, size_t iCount)
{
#ifdef CS_DEBUG
  if ( (iPos > Size) || (iPos + iCount > Size) )
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
    SetSize (NewSize);
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
    SetSize (NewSize);
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
    SetSize (NewSize);
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
    SetSize (NewSize);

  memcpy (Data + Size, iStr.GetData (), iCount);
  Data [Size = NewSize] = 0;

  return *this;
}

csString &csString::Append (const char *iStr, size_t iCount)
{
  if (iCount == (size_t)-1)
    iCount = strlen (iStr);

  if (!iCount)
    return *this;

  size_t NewSize = Size + iCount;
  if (NewSize >= MaxSize)
    SetSize (NewSize);

  memcpy (Data + Size, iStr, iCount);
  Data [Size = NewSize] = 0;

  return *this;
}

csString &csString::LTrim()
{
  size_t i;
  for(i = 0; i < Size; i++) {
    if(!isspace(Data[i]))
      return DeleteAt(0, i);
  }
  return *this;
}

csString &csString::RTrim()
{
  int i;
  for(i = Size - 1; i >= 0; i--) {
    if(!isspace(Data[i])) {
      i++;
      return DeleteAt(i, Size - i);
    }
  }
  return *this;
}

csString &csString::Trim()
{
  LTrim();
  return RTrim();
}

csString &csString::Collapse()
{
  size_t i, start = (size_t) -1;

  Trim();

  for(i = 0; i < Size; i++) {

    if(isspace(Data[i])) {

      // We skip over the first whitespace, and make sure the first space IS a space
      if(start==(size_t) -1) {
	start = i + 1;
	Data[i] = ' ';
      }

    } else {

      // Delete any extra whitespace
      if((start!=(size_t) -1)&&(start!=i)) {
	DeleteAt(start, i - start);
	i -= i - start;
      }

      start = (size_t) -1;
    }
  }

  return *this;
}

#if 0 // Disabled due to lack of vsnprintf on some OSes
csString &csString::Format(const char *format, ...)
{
  va_list args;
  int NewSize = -1;

  va_start(args, format);

  // Keep trying until the buffer is big enough to hold the entire string
  while(NewSize < 0) {
    NewSize = vsnprintf(Data, MaxSize, format, args);
    // Increasing by the size of the format streams seems logical enough
    if(NewSize < 0)
      SetSize(MaxSize + strlen(format));
    // In this case we know what size it wants
    if((size_t) NewSize>=MaxSize) {
      SetSize(NewSize + 1);
      NewSize = -1; // Don't break the while loop just yet!
    } 
  }

  // Add in the terminating NULL
  Size = NewSize + 1;

  va_end(args);

  return *this;

}
#endif // DISABLED
