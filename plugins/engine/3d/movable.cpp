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
SCF_IMPLEMENT_IBASE(csMovableSectorList)
  SCF_IMPLEMENTS_INTERFACE(iSectorList)
SCF_IMPLEMENT_IBASE_END

csMovableSectorList::csMovableSectorList ()
{
  SCF_CONSTRUCT_IBASE (0);
  movable = 0;
}

csMovableSectorList::~csMovableSectorList ()
{
  DeleteAll ();
  SCF_DESTRUCT_IBASE ();
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
  return true;
}

int csMovableSectorList::Add (iSector *obj)
{
  if (!PrepareSector (obj)) return -1;
  return Push (obj);
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
  return csRefArrayObject<iSector>::Find (obj);
}

iSector *csMovableSectorList::FindByName (const char *Name) const
{
  return csRefArrayObject<iSector>::FindByName (Name);
}

//---------------------------------------------------------------------------
SCF_IMPLEMENT_IBASE(csMovable)
  SCF_IMPLEMENTS_INTERFACE(iBase)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iMovable)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMovable::eiMovable)
  SCF_IMPLEMENTS_INTERFACE(iMovable)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csMovable::csMovable ()
{
  SCF_CONSTRUCT_IBASE (0);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMovable);
  parent = 0;
  object = 0;
  updatenr = 0;
  sectors.SetMovable (this);
  is_identity = true;
}

csMovable::~csMovable ()
{
  size_t i = listeners.Length ();
  while (i > 0)
  {
    i--;
    iMovableListener *ml = listeners[i];
    ml->MovableDestroyed (&scfiMovable);
  }
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiMovable);
  SCF_DESTRUCT_IBASE ();
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
  if (object) object->RemoveFromSectors ();
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

  if (object) object->UpdateMove ();

  size_t i = listeners.Length ();
  while (i > 0)
  {
    i--;
    iMovableListener *ml = listeners[i];
    ml->MovableChanged (&scfiMovable);
  }
}

csReversibleTransform csMovable::GetFullTransform () const
{
  if (parent == 0)
    return GetTransform ();
  else if (is_identity)
    return parent->GetFullTransform ();
  else
    return GetTransform () * parent->GetFullTransform ();
}

//--------------------------------------------------------------------------
iMovable *csMovable::eiMovable::GetParent () const
{
  return scfParent->parent;
}

void csMovable::eiMovable::SetSector (iSector *sector)
{
  scfParent->SetSector (sector);
}

void csMovable::eiMovable::ClearSectors ()
{
  scfParent->ClearSectors ();
}

iSectorList *csMovable::eiMovable::GetSectors ()
{
  return scfParent->GetSectors ();
}

bool csMovable::eiMovable::InSector () const
{
  return scfParent->InSector ();
}

void csMovable::eiMovable::SetPosition (iSector *home, const csVector3 &v)
{
  scfParent->SetPosition (home, v);
}

void csMovable::eiMovable::SetPosition (const csVector3 &v)
{
  scfParent->SetPosition (v);
}

const csVector3 &csMovable::eiMovable::GetPosition () const
{
  return scfParent->GetPosition ();
}

const csVector3 csMovable::eiMovable::GetFullPosition () const
{
  return scfParent->GetFullPosition ();
}

void csMovable::eiMovable::SetTransform (const csMatrix3 &matrix)
{
  scfParent->SetTransform (matrix);
}

void csMovable::eiMovable::SetTransform (const csReversibleTransform &t)
{
  scfParent->SetTransform (t);
}

csReversibleTransform &csMovable::eiMovable::GetTransform ()
{
  return scfParent->GetTransform ();
}

csReversibleTransform csMovable::eiMovable::GetFullTransform () const
{
  return scfParent->GetFullTransform ();
}

void csMovable::eiMovable::MovePosition (const csVector3 &v)
{
  scfParent->MovePosition (v);
}

void csMovable::eiMovable::Transform (const csMatrix3 &matrix)
{
  scfParent->Transform (matrix);
}

void csMovable::eiMovable::AddListener (
  iMovableListener *listener)
{
  scfParent->AddListener (listener);
}

void csMovable::eiMovable::RemoveListener (iMovableListener *listener)
{
  scfParent->RemoveListener (listener);
}

void csMovable::eiMovable::UpdateMove ()
{
  scfParent->UpdateMove ();
}

long csMovable::eiMovable::GetUpdateNumber () const
{
  return scfParent->GetUpdateNumber ();
}
