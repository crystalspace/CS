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
#include "csutil/nobjvec.h"
#include "csutil/csobject.h"

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

// ---------------------------------------------------------------------------

// @@@ is there a better way than doing QUERY_INTERFACE all the time?

int csNamedObjectVector::GetIndexByName (const char *name) const
{
  int i;
  for (i = 0; i < Length (); i++)
  {
    iObject *o = Get (i);
    if (!o) continue;
    const char* oname = o->GetName ();
    if (name == oname || (name && oname && !strcmp (oname, name)))
      return i;
  }
  return -1;
}

iObject *csNamedObjectVector::FindByName (const char* name) const
{
  int n = GetIndexByName (name);
  return (n == -1) ? NULL : Get (n);
}

int csNamedObjectVector::Find (iObject *obj) const
{
  int i;
  for (i = 0; i < Length (); i++)
  {
    iObject *o = Get (i);
    if (obj == o)
      return i;
  }
  return -1;
}

int csNamedObjectVector::Compare (csSome Item1, csSome Item2, int)
{
  iObject *obj1 = SCF_QUERY_INTERFACE_FAST (((iBase*)Item1), iObject);
  iObject *obj2 = SCF_QUERY_INTERFACE_FAST (((iBase*)Item2), iObject);
  
  int res = (obj1->GetName () == obj2->GetName ()) ? 0
       : strcmp (obj1->GetName (), obj2->GetName ());
  obj1->DecRef ();
  obj2->DecRef ();
  return res;
}

int csNamedObjectVector::CompareKey (csSome Item, csConstSome Key, int)
{
  iObject *obj = SCF_QUERY_INTERFACE_FAST (((iBase*)Item), iObject);
  const char *name = (const char *)Key;

  int res = (obj->GetName () == name) ? 0
       : strcmp (obj->GetName (), name);
  obj->DecRef ();
  return res;
}

bool csNamedObjectVector::Delete (iObject *obj)
{
  int n = Find (obj);
  if (n == -1) return false;
  else return Delete (n);
}
