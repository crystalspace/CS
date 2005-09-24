/*
    Copyright (C) 1998-2002 by Jorrit Tyberghein and Keith Fulton

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
#include "csqint.h"
#include "csutil/csstring.h"
#include "plugins/engine/3d/sharevar.h"

CS_LEAKGUARD_IMPLEMENT (csSharedVariable);


void csSharedVariable::FireListeners ()
{
  size_t i;
  for (i = 0 ; i < listeners.Length () ; i++)
    listeners[i]->VariableChanged (this);
}

//-----------------------------------------------------------------------------

csSharedVariableList::csSharedVariableList ()
  : scfImplementationType (this)
{
  
}

csSharedVariableList::~csSharedVariableList ()
{
  list.DeleteAll ();
}

int csSharedVariableList::GetCount () const
{
  return (int)list.GetSize ();
}

iSharedVariable *csSharedVariableList::Get (int n) const
{
  return list.Get (n);
}

int csSharedVariableList::Add (iSharedVariable *obj)
{
  return (int)list.Push (obj);
}

bool csSharedVariableList::Remove (iSharedVariable *obj)
{
  return list.Delete (obj);
}

bool csSharedVariableList::Remove (int n)
{
  return list.DeleteIndex (n);
}

void csSharedVariableList::RemoveAll ()
{
  list.DeleteAll ();
}

int csSharedVariableList::Find (iSharedVariable *obj) const
{
  return (int)list.Find (obj);
}

iSharedVariable *csSharedVariableList::FindByName (
	const char *Name) const
{
  return list.FindByName (Name);
}

csPtr<iSharedVariable> csSharedVariableList::New() const
{
  csSharedVariable *New = new csSharedVariable;
  return csPtr<iSharedVariable> (New);
}

