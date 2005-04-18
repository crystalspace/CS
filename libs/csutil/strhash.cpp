/*
    Copyright (C) 2002 by Jorrit Tyberghein

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
#include "csutil/strhash.h"
#include "csutil/util.h"

csStringHash::csStringHash (size_t size) : registry (size)
{
}

csStringHash::~csStringHash ()
{
  Empty ();
}

void csStringHash::Copy(csStringHash const& h)
{
  GlobalIterator it(h.GetIterator());
  while (it.HasNext())
  {
    char const* s;
    csStringID id = it.Next(s);
    Register(s, id);
  }
}

const char* csStringHash::Register (const char* s, csStringID id)
{
  char const* t = pool.Store(s);
  registry.PutUnique(t, id);
  return t;
}

csStringID csStringHash::Request (const char* s) const
{
  return registry.Get(s, csInvalidStringID);
}

const char* csStringHash::Request (csStringID id) const
{
  GlobalIterator it(GetIterator());
  while (it.HasNext())
  {
    char const* s;
    csStringID const x = it.Next(s);
    if (x == id)
      return s;
  }
  return 0;
}

bool csStringHash::Delete(char const* s)
{
  return registry.DeleteAll(s);
}

bool csStringHash::Delete(csStringID id)
{
  char const* s = Request(id);
  return s != 0 ? Delete(s) : false;
}

void csStringHash::Empty ()
{
  registry.Empty();
  pool.Empty();
}
