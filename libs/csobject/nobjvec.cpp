/*
    Crystal Space: Named Object Vector class
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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
#include "csobject/nobjvec.h"
#include "csobject/nameobj.h"

csObject *csNamedObjVector::FindByName (const char* name)
{
  for (int i = Length () - 1; i >= 0; i--)
  {
    csObject *o = (csObject *)Get (i);
    if (!strcmp (o->GetName (), name))
      return o;
  }
  return NULL;
}

int csNamedObjVector::Compare (csSome Item1, csSome Item2, int Mode) const
{
  (void)Mode;
  return strcmp (((csObject *)Item1)->GetName (), ((csObject *)Item2)->GetName ());
}

int csNamedObjVector::CompareKey (csSome Item, csConstSome Key, int Mode) const
{
  (void)Mode;
  return strcmp (((csObject *)Item)->GetName (), (char *)Key);
}
