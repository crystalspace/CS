/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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
#include "csengine/cscoll.h"
#include "csengine/thing.h"

IMPLEMENT_CSOBJTYPE (csCollection,csObject);

csCollection::csCollection () : csObject(), objects(8,8) {}

csObject* csCollection::FindObject (char* name)
{
  for (int i = 0 ; i < objects.Length() ; i++)
  {
    csObject* csobj = (csObject*)(objects[i]);
    if (!strcmp ( csobj->GetName (), name)) return csobj;
  }
  return NULL;
}

void csCollection::Transform ()
{
  for (int i = 0 ; i < objects.Length() ; i++)
    if ( ((csObject*)(objects[i]))->GetType () == csThing::Type)
      ((csThing*)(objects[i]))->GetMovable ().UpdateMove ();
}

void csCollection::SetPosition (csSector* home, const csVector3& pos)
{
  for (int i = 0 ; i < objects.Length() ; i++)
    if ( ((csObject*)(objects[i]))->GetType () == csThing::Type)
      ((csThing*)(objects[i]))->GetMovable ().SetPosition (home, pos);
}

void csCollection::SetTransform (const csMatrix3& matrix)
{
  for (int i = 0 ; i < objects.Length() ; i++)
    if ( ((csObject*)(objects[i]))->GetType () == csThing::Type)
      ((csThing*)(objects[i]))->GetMovable ().SetTransform (matrix);
}

void csCollection::MovePosition (const csVector3& rel)
{
  for (int i = 0 ; i < objects.Length() ; i++)
    if ( ((csObject*)(objects[i]))->GetType () == csThing::Type)
      ((csThing*)(objects[i]))->GetMovable ().MovePosition (rel);
}

void csCollection::Transform (csMatrix3& matrix)
{
  for (int i = 0 ; i < objects.Length() ; i++)
    if ( ((csObject*)(objects[i]))->GetType () == csThing::Type)
      ((csThing*)(objects[i]))->GetMovable ().Transform (matrix);
}

//---------------------------------------------------------------------------
