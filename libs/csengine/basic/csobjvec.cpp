/*
  Crystal Space Windowing System: object vector class
  Copyright (C) 1998 by Jorrit Tyberghein
  Written by Andrew Zabolotny <bit@eltech.ru>

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
#include "csobject/nameobj.h"
#include "csengine/csobjvec.h"

bool csObjVector::FreeItem (csSome Item)
{
  if (Item)
    delete (csBase *) Item;
  return true;
}

csObjVector::~csObjVector ()
{
  DeleteAll ();
}

csObject* csObjVector::FindByName (const char* name)
{
  for (int i = 0; i < Length (); i++)
  {
    csObject* o = (csObject*)root[i];
    if (!strcmp (csNameObject::GetName(*o), name))
      return o;
  }
  return NULL;
}
