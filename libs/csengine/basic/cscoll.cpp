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
#include "csengine/engine.h"
#include "csengine/sector.h"

SCF_IMPLEMENT_IBASE (csCollection)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iCollection)
  SCF_IMPLEMENTS_INTERFACE (csCollection)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csCollection::Collection)
  SCF_IMPLEMENTS_INTERFACE (iCollection)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csCollection::csCollection (csEngine* engine) :
  csObject(), objects (8,8), movable ()
{
  SCF_CONSTRUCT_IBASE(NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiCollection);

  movable.SetObject (this);
  csCollection::engine = engine;
  engine->AddToCurrentRegion (this);
}

csCollection::~csCollection ()
{
}

iObject* csCollection::FindObject (char* name)
{
  for (int i = 0 ; i < objects.Length() ; i++)
  {
    iObject* obj = (iObject*)(objects[i]);
    if (!strcmp ( obj->GetName (), name)) return obj;
  }
  return NULL;
}

void csCollection::UpdateMove ()
{
}

void csCollection::MoveToSector (csSector* s)
{
  s->AddCollection (this);
}

void csCollection::RemoveFromSectors ()
{
  int i;
  const iSectorList *sectors = movable.GetSectors ();
  for (i = 0 ; i < sectors->GetSectorCount () ; i++)
  {
    iSector* ss = sectors->GetSector (i);
    if (ss)
      ss->UnlinkCollection (&scfiCollection);
  }
}
