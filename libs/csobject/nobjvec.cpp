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

#include "cssysdef.h"
#include "csobject/nobjvec.h"
#include "csobject/csobject.h"

csObject *csNamedObjVector::FindByName (const char* name) const
{
  csObject* o;
  int i;
  for (i = Length () - 1; i >= 0; i--)
  {
    o = (csObject *)Get (i);
    const char* oname = o->GetName ();
    if (name == oname || (name && oname && !strcmp (oname, name)))
      return o;
  }
  return NULL;
}

int csNamedObjVector::Compare (csSome Item1, csSome Item2, int Mode) const
{
  (void)Mode;
  return (((csObject *)Item1)->GetName () == ((csObject *)Item2)->GetName ()) ? 0
       : strcmp (((csObject *)Item1)->GetName (), ((csObject *)Item2)->GetName ());
}

int csNamedObjVector::CompareKey (csSome Item, csConstSome Key, int Mode) const
{
  (void)Mode;
  return (((csObject *)Item)->GetName () == (char *)Key) ? 0
       : strcmp (((csObject *)Item)->GetName (), (char *)Key);
}
