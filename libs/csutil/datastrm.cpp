/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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
#include "csutil/datastrm.h"
#include <ctype.h>

csDataStream::csDataStream (void *buf, size_t n, bool del) :
  Data ((uint8*)buf), Position (0), Size (n), DeleteBuffer (del)
{
}

csDataStream::~csDataStream ()
{
  if (DeleteBuffer)
    delete[] Data;
}

size_t csDataStream::GetPosition ()
{
  return Position;
}

void csDataStream::SetPosition (size_t pos)
{
  Position = pos;
}

size_t csDataStream::GetLength ()
{
  return Size;
}

bool csDataStream::Finished ()
{
  return (Position == Size);
}

void csDataStream::Skip (size_t num)
{
  Position += num;
  if (Position > Size) Position = Size;
}

size_t csDataStream::Read (void *buf, size_t n)
{
  if (Position + n > Size)
    n = Size - Position;
  memcpy (buf, Data + Position, n);
  Position += n;
  return n;
}

#define IMPLEMENT_READ_INT(name,type)			\
bool csDataStream::name (type &var)			\
{							\
  return (Read (&var, sizeof(type)) == sizeof(type));	\
}

IMPLEMENT_READ_INT (ReadInt8, int8);
IMPLEMENT_READ_INT (ReadUInt8, uint8);
IMPLEMENT_READ_INT (ReadInt16, int16);
IMPLEMENT_READ_INT (ReadUInt16, uint16);
IMPLEMENT_READ_INT (ReadInt32, int32);
IMPLEMENT_READ_INT (ReadUInt32, uint32);

int csDataStream::GetChar ()
{
  char val;
  if (Read (&val, sizeof(char)) < (int)sizeof(char))
    return EOF;
  return val;
}

int csDataStream::LookChar ()
{
  size_t OldPosition = Position;
  int Result = GetChar ();
  Position = OldPosition;
  return Result;
}

bool csDataStream::GetString (char* buf, size_t len, bool OmitNewline)
{
  // test for EOF
  if (Position == Size) return false;

  // find the end of the current line
  char *String = (char*)(Data+Position);
  char *FirstNewline = strchr (String, '\n');

  // look if there are now more lines
  if (!FirstNewline)
  {
    len = Read (buf, len - 1);
    buf [len - 1] = 0;
    return true;
  }

  // calculate the length of the line including the newline character
  size_t LineLen = FirstNewline - String + 1;

  // truncate buffer length if greater than length of the line
  // (+1 for the null character)
  if (len > LineLen + 1) len = LineLen + 1;

  // read the line
  Read (buf, len - 1);
  buf [len - 1] = 0;

  // look for the newline character
  if ((buf [len-2] == '\n') && OmitNewline) buf [len-2] = 0;

  return true;
}

int csDataStream::ReadTextInt ()
{
  int Value, Length;
  if (sscanf ((char*)(Data+Position), "%d%n", &Value, &Length) != 1)
  {
    Position = Size;
    return 0;
  }
  else
  {
    Position += Length;
    return Value;
  }
}

float csDataStream::ReadTextFloat ()
{
  float Value;
  int Length;
  if (sscanf ((char*)(Data+Position), "%f%n", &Value, &Length) != 1)
  {
    Position = Size;
    return 0;
  }
  else
  {
    Position += Length;
    return Value;
  }
}

void csDataStream::SkipWhitespace ()
{
  while (Position < Size && isspace (Data[Position]))
    Position++;
}


