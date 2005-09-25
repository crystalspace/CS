  /*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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
#include "plugins/engine/3d/movable.h"
#include "plugins/engine/3d/sector.h"
#include "plugins/engine/3d/cscoll.h"
#include "plugins/engine/3d/engine.h"

//---------------------------------------------------------------------------
csMovableSectorList::csMovableSectorList ()
  : scfImplementationType (this)
{
  movable = 0;
}

csMovableSectorList::~csMovableSectorList ()
{
  DeleteAll ();
}

bool csMovableSectorList::PrepareSector (iSector* sector)
{
  // Check for a valid item.
  if (sector == 0) return false;

  // if the movable has a parent, no sectors can be added.
  // We still call MoveToSector() because MoveToSector will
  // also register portal containers to the sector.
  CS_ASSERT (movable != 0);
  csMeshWrapper *mw = movable->GetMeshWrapper ();
  if (mw) mw->MoveToSector (sector);

  csLight *l = movable->GetLight ();
  if (l) l->OnSetSector (sector);
  // Make sure camera and light only is in one sector
  CS_ASSERT (!(movable->GetLight () && Length () > 0));
  CS_ASSERT (!(movable->GetCamera () && Length () > 0));
  return true;
}

int csMovableSectorList::Add (iSector *obj)
{
  if (!PrepareSector (obj)) return -1;
  return (int)Push (obj);
}

bool csMovableSectorList::Remove (iSector *obj)
{
  csMeshWrapper* object = movable->GetMeshWrapper ();
  if (object) object->RemoveFromSectors (obj);
  return Delete (obj);
}

bool csMovableSectorList::Remove (int n)
{
  iSector* obj = Get (n);
  csMeshWrapper* object = movable->GetMeshWrapper ();
  if (object) object->RemoveFromSectors (obj);
  return DeleteIndex (n);
}

void csMovableSectorList::RemoveAll ()
{
  movable->ClearSectors ();
}

int csMovableSectorList::Find (iSector *obj) const
{
  return (int)csRefArrayObject<iSector>::Find (obj);
}

iSector *csMovableSectorList::FindByName (const char *Name) const
{
  return csRefArrayObject<iSector>::FindByName (Name);
}

//---------------------------------------------------------------------------

csMovable::csMovable ()
  : scfImplementationType (this), is_identity (true), parent (0),
  meshobject (0), lightobject (0), cameraobject (0), updatenr (0)
{
  sectors.SetMovable (this);
}

csMovable::~csMovable ()
{
  size_t i = listeners.Length ();
  while (i > 0)
  {
    i--;
    iMovableListener *ml = listeners[i];
    ml->MovableDestroyed (this);
  }
}

void csMovable::SetPosition (iSector *home, const csVector3 &pos)
{
  obj.SetOrigin (pos);
  SetSector (home);
}

void csMovable::SetTransform (const csMatrix3 &matrix)
{
  obj.SetT2O (matrix);
}

void csMovable::MovePosition (const csVector3 &rel)
{
  obj.Translate (rel);
}

void csMovable::Transform (const csMatrix3 &matrix)
{
  obj.SetT2O (matrix * obj.GetT2O ());  
}

void csMovable::SetSector (iSector *sector)
{
  if (sectors.Length () == 1 && sector == sectors[0]) return ;
  ClearSectors ();
  if (sectors.PrepareSector (sector))
    sectors.Push (sector);
}

void csMovable::ClearSectors ()
{
  if (meshobject) meshobject->RemoveFromSectors ();
  if (parent == 0)
  {
    sectors.DeleteAll ();
    sectors.SetLength (0);
  }
}

void csMovable::AddListener (iMovableListener *listener)
{
  RemoveListener (listener);
  listeners.Push (listener);
}

void csMovable::RemoveListener (iMovableListener *listener)
{
  listeners.Delete (listener);
}

void csMovable::UpdateMove ()
{
  updatenr++;
  is_identity = obj.IsIdentity ();

  if (meshobject) meshobject->UpdateMove ();
  if (lightobject) lightobject->OnSetPosition ();

  size_t i = listeners.Length ();
  while (i > 0)
  {
    i--;
    iMovableListener *ml = listeners[i];
    ml->MovableChanged (this);
  }
}
