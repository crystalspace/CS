/*
    Crystal Space utility library: string reader
    Copyright (C) 2004 by Jorrit Tyberghein

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
#include "csutil/stringreader.h"

csStringReader::csStringReader ()
{
  input = 0;
  cur = 0;
}

csStringReader::csStringReader (const char* input)
{
  SetInput (input);
}

void csStringReader::SetInput (const char* input)
{
  csStringReader::input = input;
  Reset ();
}

void csStringReader::Reset ()
{
  cur = input;
}

bool csStringReader::GetLine (csString& line)
{
  line.Clear ();
  if (!HasMoreLines ()) return false;
  const char* end = cur + strcspn (cur, "\n\r");
  line.Append (cur, end-cur);
  cur = end;
  if (*cur == '\r' && *(cur+1) == '\n') cur += 2;
  else if (*cur != 0) cur++;
  return true;
}

bool csStringReader::HasMoreLines ()
{
  if (cur == 0 || *cur == 0) return false;
  return true;
}


