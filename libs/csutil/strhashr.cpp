/*
    Copyright (C) 2008 by Jorrit Tyberghein and Michael Gist

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
#include "csutil/strhashr.h"


csStringHashReversible::csStringHashReversible (size_t size) : csStringHash(size), reverse(size)
{
}

csStringHashReversible::~csStringHashReversible ()
{
  Empty();
}

void csStringHashReversible::Copy(csStringHashReversible const& h)
{
  if (&h != this)
  {
    GlobalIterator it(h.GetIterator());
    while (it.HasNext())
    {
      char const* s;
      csStringID id = it.Next(s);
      Register(s, id);
    }
  }
}

const char* csStringHashReversible::Register (const char* s, csStringID id)
{
  const char* t = csStringHash::Register(s, id);
  reverse.PutUnique(id, t);
  return t;
}

const char* csStringHashReversible::Request (csStringID id) const
{
  return reverse.Get(id, NULL);
}

bool csStringHashReversible::Delete(char const* s)
{
  reverse.DeleteAll(csStringHash::Request(s));
  return csStringHash::Delete(s);
}

bool csStringHashReversible::Delete(csStringID id)
{
  reverse.DeleteAll(id);
  return csStringHash::Delete(Request(id));
}

void csStringHashReversible::Empty ()
{
  reverse.Empty();
  csStringHash::Empty();
}
