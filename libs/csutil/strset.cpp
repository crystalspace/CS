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
#include "csutil/strset.h"
#include "csutil/util.h"

csStringSet::csStringSet (size_t size) :
  registry(size), reverse(size), next_id(0)
{
}

csStringSet::~csStringSet ()
{
}

void csStringSet::Copy(csStringSet const& s)
{
  if (&s != this)
  {
    registry = s.registry;
    reverse  = s.reverse;
    next_id  = s.next_id;
  }
}

csStringID csStringSet::Request (const char* s)
{
  csStringID id = registry.Request(s);
  if (id == csInvalidStringID)
  {
    const char* t = registry.Register(s, next_id);
    reverse.Put(next_id, t);
    id = next_id;
    next_id++;
  }
  return id;
}

const char* csStringSet::Request (csStringID id) const
{
  return reverse.Get(id, 0);
}

bool csStringSet::Contains (char const* s) const
{
  return registry.Request(s) != csInvalidStringID;
}

bool csStringSet::Delete (char const* s)
{
  csStringID const id = registry.Request(s);
  bool const ok = (id != csInvalidStringID);
  if (ok)
  {
    registry.Delete(s);
    reverse.DeleteAll(id);
  }
  return ok;
}

bool csStringSet::Delete (csStringID id)
{
  char const* s = reverse.Get(id,0);
  bool const ok = (s != 0);
  if (ok)
  {
    registry.Delete(s);
    reverse.DeleteAll(id);
  }
  return ok;
}

void csStringSet::Empty ()
{
  registry.Empty();
  reverse.Empty();
}
