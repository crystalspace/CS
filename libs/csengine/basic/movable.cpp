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
#include "qint.h"
#include "csengine/movable.h"

#include "csengine/sector.h"
#include "csengine/thing.h"
#include "csengine/meshobj.h"
#include "csengine/cscoll.h"
#include "csengine/engine.h"


//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csMovableSectorList)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iSectorList)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMovableSectorList::SectorList)
  SCF_IMPLEMENTS_INTERFACE (iSectorList)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csMovableSectorList::csMovableSectorList ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiSectorList);
  movable = NULL;
}

csMovableSectorList::~csMovableSectorList ()
{
  DeleteAll ();
}

bool csMovableSectorList::PrepareItem (csSome item)
{
  // check for a valid item and IncRef it
  iSector* sector = (iSector*)item;
  if (sector == NULL) return false;

  // if the movable has a parent, no sectors can be added.
  CS_ASSERT (movable != NULL);
  if (movable->GetParent ())
    return false;

  sector->IncRef ();
  csMeshWrapper* mw = movable->GetMeshWrapper ();
  if (mw)
    mw->MoveToSector (sector->GetPrivateObject ());
  return true;
}

bool csMovableSectorList::FreeItem (void *item)
{
  iSector *Sector = (iSector*)item;

  Sector->DecRef ();
  return true;
}

int csMovableSectorList::SectorList::GetCount () const
  { return scfParent->Length (); }
iSector *csMovableSectorList::SectorList::Get (int n) const
  { return scfParent->Get (n); }
int csMovableSectorList::SectorList::Add (iSector *obj)
  { return scfParent->Push (obj); }
bool csMovableSectorList::SectorList::Remove (iSector *obj)
  { return scfParent->Delete (obj); }
bool csMovableSectorList::SectorList::Remove (int n)
  { return scfParent->Delete (n); }
void csMovableSectorList::SectorList::RemoveAll ()
  { scfParent->DeleteAll (); }
int csMovableSectorList::SectorList::Find (iSector *obj) const
  { return scfParent->Find (obj); }
iSector *csMovableSectorList::SectorList::FindByName (const char *Name) const
  { return scfParent->FindByName (Name); }

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csMovable)
  SCF_IMPLEMENTS_INTERFACE (iBase)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iMovable)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMovable::eiMovable)
  SCF_IMPLEMENTS_INTERFACE (iMovable)
SCF_IMPLEMENT_EMBEDDED_IBASE_END


csMovable::csMovable ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMovable);
  parent = NULL;
  object = NULL;
  updatenr = 0;
  sectors.SetMovable (this);
}

csMovable::~csMovable ()
{
  int i;
  for (i = 0 ; i < listeners.Length () ; i++)
  {
    iMovableListener* ml = listeners[i];
    void* ml_data = listener_userdata[i];
    ml->MovableDestroyed (&scfiMovable, ml_data);
  }
}

void csMovable::SetPosition (iSector* home, const csVector3& pos)
{
  obj.SetOrigin (pos);
  SetSector (home);
}

void csMovable::SetTransform (const csMatrix3& matrix)
{
  obj.SetT2O (matrix);
}

void csMovable::MovePosition (const csVector3& rel)
{
  obj.Translate (rel);
}

void csMovable::Transform (const csMatrix3& matrix)
{
  obj.SetT2O (matrix * obj.GetT2O ());
}

void csMovable::SetSector (iSector* sector)
{
  if (sectors.Length () == 1 && sector == sectors[0])
    return;
  ClearSectors ();
  sectors.Push (sector);
}

void csMovable::ClearSectors ()
{
  if (parent == NULL)
  {
    if (object) object->RemoveFromSectors ();
    sectors.DeleteAll ();
    sectors.SetLength (0);
  }
}

void csMovable::AddListener (iMovableListener* listener, void* userdata)
{
  RemoveListener (listener);
  listeners.Push (listener);
  listener_userdata.Push (userdata);
}

void csMovable::RemoveListener (iMovableListener* listener)
{
  int idx = listeners.Find (listener);
  if (idx == -1) return;
  listeners.Delete (idx);
  listener_userdata.Delete (idx);
}

void csMovable::UpdateMove ()
{
  updatenr++;
  if (object) object->UpdateMove ();

  int i;
  for (i = 0 ; i < listeners.Length () ; i++)
  {
    iMovableListener* ml = listeners[i];
    void* ml_data = listener_userdata[i];
    ml->MovableChanged (&scfiMovable, ml_data);
  }
}

csReversibleTransform csMovable::GetFullTransform () const
{
  if (parent == NULL)
    return GetTransform ();
  else
    return GetTransform () * parent->GetFullTransform ();
}

//--------------------------------------------------------------------------

iMovable* csMovable::eiMovable::GetParent () const
  { return scfParent->parent; }
void csMovable::eiMovable::SetSector (iSector* sector)
  { scfParent->SetSector (sector); }
void csMovable::eiMovable::ClearSectors ()
  { scfParent->ClearSectors (); }
iSectorList *csMovable::eiMovable::GetSectors ()
  { return scfParent->GetSectors (); }
bool csMovable::eiMovable::InSector () const
  { return scfParent->InSector (); }
void csMovable::eiMovable::SetPosition (iSector* home, const csVector3& v)
  { scfParent->SetPosition (home, v); }
void csMovable::eiMovable::SetPosition (const csVector3& v)
  { scfParent->SetPosition (v); }
const csVector3& csMovable::eiMovable::GetPosition () const
  { return scfParent->GetPosition (); }
const csVector3 csMovable::eiMovable::GetFullPosition () const
  { return scfParent->GetFullPosition (); }
void csMovable::eiMovable::SetTransform (const csMatrix3& matrix)
  { scfParent->SetTransform (matrix); }
void csMovable::eiMovable::SetTransform (const csReversibleTransform& t)
  { scfParent->SetTransform (t); }
csReversibleTransform& csMovable::eiMovable::GetTransform ()
  { return scfParent->GetTransform (); }
csReversibleTransform csMovable::eiMovable::GetFullTransform () const
  { return scfParent->GetFullTransform (); }
void csMovable::eiMovable::MovePosition (const csVector3& v)
  { scfParent->MovePosition (v); }
void csMovable::eiMovable::Transform (const csMatrix3& matrix)
  { scfParent->Transform (matrix); }
void csMovable::eiMovable::AddListener (iMovableListener* listener, void* userdata)
  { scfParent->AddListener (listener, userdata); }
void csMovable::eiMovable::RemoveListener (iMovableListener* listener)
  { scfParent->RemoveListener (listener); }
void csMovable::eiMovable::UpdateMove ()
  { scfParent->UpdateMove (); }
long csMovable::eiMovable::GetUpdateNumber () const
  { return scfParent->GetUpdateNumber (); }
