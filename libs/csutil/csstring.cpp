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

csString &csString::Insert (size_t iPos, const csString &iStr)
{
#ifdef DEBUG
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
  memcpy (Data + iPos + sl, Data + iPos, Size - iPos);
  memcpy (Data + iPos, iStr.GetData (), sl);
  Data [Size = NewSize] = 0;

  return *this;
}

csString &csString::Overwrite (size_t iPos, const csString &iStr)
{
#ifdef DEBUG
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
