/*
    Copyright (C) 1998-2006 by Jorrit Tyberghein and Keith Fulton

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
#include "plugins/engine/3d/engine.h"

CS_LEAKGUARD_IMPLEMENT (csSharedVariable);


void csSharedVariable::FireListeners ()
{
  size_t i;
  for (i = 0 ; i < listeners.GetSize () ; i++)
    listeners[i]->VariableChanged (this);
}

void csSharedVariable::SelfDestruct ()
{
  variables->Remove (static_cast<iSharedVariable*> (this));
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
  CS::Threading::ScopedReadLock lock(shvarLock);
  return (int)list.GetSize ();
}

iSharedVariable *csSharedVariableList::Get (int n) const
{
  CS::Threading::ScopedReadLock lock(shvarLock);
  return list.Get (n);
}

int csSharedVariableList::Add (iSharedVariable *obj)
{
  CS::Threading::ScopedWriteLock lock(shvarLock);
  return (int)list.Push (obj);
}

void csSharedVariableList::AddBatch (csRef<iSharedVarLoaderIterator> itr)
{
  CS::Threading::ScopedWriteLock lock(shvarLock);
  while(itr->HasNext())
  {
    list.Push (itr->Next());
  }
}

bool csSharedVariableList::Remove (iSharedVariable *obj)
{
  CS::Threading::ScopedWriteLock lock(shvarLock);
  return list.Delete (obj);
}

bool csSharedVariableList::Remove (int n)
{
  CS::Threading::ScopedWriteLock lock(shvarLock);
  return list.DeleteIndex (n);
}

void csSharedVariableList::RemoveAll ()
{
  CS::Threading::ScopedWriteLock lock(shvarLock);
  list.Empty ();
}

int csSharedVariableList::Find (iSharedVariable *obj) const
{
  CS::Threading::ScopedReadLock lock(shvarLock);
  return (int)list.Find (obj);
}

iSharedVariable *csSharedVariableList::FindByName (
	const char *Name) const
{
  CS::Threading::ScopedReadLock lock(shvarLock);
  return list.FindByName (Name);
}

csPtr<iSharedVariable> csSharedVariableList::New()
{
  csSharedVariable *New = new csSharedVariable (this);
  return csPtr<iSharedVariable> (New);
}

