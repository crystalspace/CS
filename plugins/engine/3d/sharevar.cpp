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
#include "plugins/engine/3d/sharevar.h"
#include "csutil/csstring.h"
#include "csutil/hashmap.h"


CS_LEAKGUARD_IMPLEMENT (csSharedVariable);

SCF_IMPLEMENT_IBASE_EXT(csSharedVariable)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iSharedVariable)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSharedVariable::eiSharedVariable)
  SCF_IMPLEMENTS_INTERFACE(iSharedVariable)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

void csSharedVariable::FireListeners ()
{
  size_t i;
  for (i = 0 ; i < listeners.Length () ; i++)
    listeners[i]->VariableChanged (&scfiSharedVariable);
}

//-----------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csSharedVariableList)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iSharedVariableList)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSharedVariableList::SharedVariableList)
  SCF_IMPLEMENTS_INTERFACE(iSharedVariableList)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSharedVariableList::csSharedVariableList ()
{
  SCF_CONSTRUCT_IBASE (0);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiSharedVariableList);
}

csSharedVariableList::~csSharedVariableList ()
{
  DeleteAll ();
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiSharedVariableList);
  SCF_DESTRUCT_IBASE ();
}

int csSharedVariableList::SharedVariableList::GetCount () const
{
  return (int)scfParent->Length ();
}

iSharedVariable *csSharedVariableList::SharedVariableList::Get (int n) const
{
  return scfParent->Get (n);
}

int csSharedVariableList::SharedVariableList::Add (iSharedVariable *obj)
{
  return (int)scfParent->Push (obj);
}

bool csSharedVariableList::SharedVariableList::Remove (iSharedVariable *obj)
{
  return scfParent->Delete (obj);
}

bool csSharedVariableList::SharedVariableList::Remove (int n)
{
  return scfParent->DeleteIndex (n);
}

void csSharedVariableList::SharedVariableList::RemoveAll ()
{
  scfParent->DeleteAll ();
}

int csSharedVariableList::SharedVariableList::Find (iSharedVariable *obj) const
{
  return (int)scfParent->Find (obj);
}

iSharedVariable *csSharedVariableList::SharedVariableList::FindByName (
	const char *Name) const
{
  return scfParent->FindByName (Name);
}

csPtr<iSharedVariable> csSharedVariableList::New() const
{
  csSharedVariable *New = new csSharedVariable;
  return csPtr<iSharedVariable> (&New->scfiSharedVariable);
}

