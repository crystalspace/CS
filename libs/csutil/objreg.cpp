/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "cssysdef.h"
#include "csutil/util.h"
#include "csutil/objreg.h"

SCF_IMPLEMENT_IBASE (csObjectRegistry)
  SCF_IMPLEMENTS_INTERFACE (iObjectRegistry)
SCF_IMPLEMENT_IBASE_END

csObjectRegistry::csObjectRegistry () : clearing (false)
{
  SCF_CONSTRUCT_IBASE (NULL);
}

void csObjectRegistry::Clear ()
{
  clearing = true;
  int i;
  for (i = registry.Length() - 1; i >= 0; i--)
  {
    // Take special care to ensure that this object is no longer on the list
    // before calling DecRef(), since we don't want some other object asking
    // for it during its own destruction.
    iBase* b = (iBase*)registry[i];
    char* t = (char*)tags[i];
    registry.Delete(i); // Remove from list before DecRef().
    tags.Delete(i);
    b->DecRef();
    delete[] t;
  }
  clearing = false;
}

bool csObjectRegistry::Register (iBase* obj, char const* tag)
{
  if (!clearing)
  {
    obj->IncRef();
    registry.Push(obj);
    tags.Push(tag ? csStrNew(tag) : 0);
    return true;
    }
  return false;
}

void csObjectRegistry::Unregister (iBase* obj, char const* tag)
{
  if (!clearing)
  {
	int i;
    for (i = registry.Length() - 1; i >= 0; i--)
    {
      iBase* b = (iBase*)registry[i];
      if (b == obj)
      {
        char* t = (char*)tags[i];
        if ((t == 0 && tag == 0) || (t != 0 && tag != 0 && !strcmp (tag, t)))
        {
          delete[] t;
          b->DecRef();
	  registry.Delete(i);
	  if (tag != 0) // For a tagged object, we're done.
	    break;
        }
      }
    }
  }
}

iBase* csObjectRegistry::Get (char const* tag)
{
  int i;
  for (i = registry.Length() - 1; i >= 0; i--)
  {
    char* t = (char*)tags[i];
    if (t && !strcmp (tag, t))
      return (iBase*)registry[i];
  }
  return 0;
}

iBase* csObjectRegistry::Get (scfInterfaceID id, int version)
{
  iBase* found_one = 0;
  int i;
  for (i = registry.Length() - 1; i >= 0; i--)
  {
    iBase* b = (iBase*)registry[i];
    iBase* interf = (iBase*)(b->QueryInterface (id, version));
    if (interf)
    {
      interf->DecRef ();
      char* t = (char*)tags[i];
      if (!t)
        return interf;
      else
        found_one = interf;
    }
  }
  return found_one;
}

