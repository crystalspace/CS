/*
    Copyright (C) 2001-2005 by Jorrit Tyberghein
	      (C) 2001 by Martin Geisse
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
#include <ctype.h>
#include "csutil/filereadhelper.h"
#include "iutil/databuff.h"

#define IMPLEMENT_READ_INT(name,type)					\
bool csFileReadHelper::name (type &var)					\
{									\
  return (file->Read ((char*)&var, sizeof(type)) == sizeof(type));	\
}

IMPLEMENT_READ_INT (ReadInt8, int8)
IMPLEMENT_READ_INT (ReadUInt8, uint8)
IMPLEMENT_READ_INT (ReadInt16, int16)
IMPLEMENT_READ_INT (ReadUInt16, uint16)
IMPLEMENT_READ_INT (ReadInt32, int32)
IMPLEMENT_READ_INT (ReadUInt32, uint32)

int csFileReadHelper::GetChar ()
{
  char val;
  if (file->Read (&val, sizeof(char)) < sizeof(char))
    return EOF;
  return val;
}

int csFileReadHelper::LookChar ()
{
  size_t OldPosition = file->GetPos();
  int Result = GetChar ();
  file->SetPos (OldPosition);
  return Result;
}

bool csFileReadHelper::GetString (char* buf, size_t len, bool OmitNewline)
{
  // test for EOF
  if (file->AtEOF()) return false;
  if (len == 0) return true;

  // find the end of the current line
  csRef<iDataBuffer> fileData = file->GetAllData (false);
  char *String = fileData->GetData()  + file->GetPos();
  char *FirstNewline = strchr (String, '\n');

  // look if there are now more lines
  if (!FirstNewline)
  {
    len = file->Read (buf, len - 1);
    buf [len] = 0;
    return true;
  }

  // calculate the length of the line including the newline character
  size_t LineLen = FirstNewline - String + 1;

  // truncate buffer length if greater than length of the line
  // (+1 for the null character)
  if (len > LineLen + 1) len = LineLen + 1;

  // read the line
  file->Read (buf, len - 1);
  buf [len] = 0;

  // look for the newline character
  if ((buf [len-1] == '\n') && OmitNewline) buf [len-1] = 0;

  return true;
}

int csFileReadHelper::ReadTextInt ()
{
  int Value, Length;
  char buf[16];
  size_t Position = file->GetPos();
  if (!GetString (buf, sizeof (buf), true)) return false;
    
  if (sscanf (buf, "%d%n", &Value, &Length) != 1)
  {
    file->SetPos (file->GetSize ());
    return 0;
  }
  else
  {
    file->SetPos (Position + Length);
    return Value;
  }
}

float csFileReadHelper::ReadTextFloat ()
{
  float Value;
  int Length;
  char buf[16];
  size_t Position = file->GetPos();
  if (!GetString (buf, sizeof (buf), true)) return false;
    
  if (sscanf (buf, "%f%n", &Value, &Length) != 1)
  {
    file->SetPos (file->GetSize ());
    return 0;
  }
  else
  {
    file->SetPos (Position + Length);
    return Value;
  }
}

void csFileReadHelper::SkipWhitespace ()
{
  char ch;
  while (((ch = GetChar ()) != EOF)  && isspace (ch)) {};
  file->SetPos (file->GetPos() -1);
}
