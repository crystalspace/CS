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
#include "csutil/objiter.h"

csTypedObjectIterator::csTypedObjectIterator (iObject *Parent)
{
  iter = Parent->GetIterator ();
}

csTypedObjectIterator::~csTypedObjectIterator ()
{
}

void csTypedObjectIterator::FetchObject ()
{
  CurrentTypedObject = 0;
  if (iter->IsFinished ())
    return;

  scfInterfaceID id;
  int ver;
  GetRequestedInterface (id, ver);

  CurrentTypedObject = csPtr<iBase> (
  	(iBase*)(iter->GetObject ()->QueryInterface (id, ver)));
  while (!CurrentTypedObject && !iter->IsFinished ())
  {
    iter->Next ();
    if (iter->IsFinished ())
    {
      return;
    }
    CurrentTypedObject = csPtr<iBase> (
  	(iBase*)(iter->GetObject ()->QueryInterface (id, ver)));
  }
}

