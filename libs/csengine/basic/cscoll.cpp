/*
    Copyright (C) 1998,2000 by Jorrit Tyberghein
  
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
#include "csengine/world.h"
#include "csengine/sector.h"

IMPLEMENT_CSOBJTYPE (csCollection,csObject);

csCollection::csCollection (csWorld* world) : csObject(), objects (8,8), movable ()
{
  movable.SetObject (this);
  csCollection::world = world;
  world->AddToCurrentRegion (this);
}

csCollection::~csCollection ()
{
  world->UnlinkCollection (this);
}

csObject* csCollection::FindObject (char* name)
{
  for (int i = 0 ; i < objects.Length() ; i++)
  {
    csObject* csobj = (csObject*)(objects[i]);
    if (!strcmp ( csobj->GetName (), name)) return csobj;
  }
  return NULL;
}

void csCollection::UpdateMove ()
{
}

void csCollection::MoveToSector (csSector* s)
{
  s->collections.Push (this);
}

void csCollection::RemoveFromSectors ()
{
  int i;
  csVector& sectors = movable.GetSectors ();
  for (i = 0 ; i < sectors.Length () ; i++)
  {
    csSector* ss = (csSector*)sectors[i];
    if (ss)
    {
      int idx = ss->collections.Find (this);
      if (idx >= 0)
      {
        ss->things[idx] = NULL;
        ss->things.Delete (idx);
      }
    }
  }
}

//---------------------------------------------------------------------------
