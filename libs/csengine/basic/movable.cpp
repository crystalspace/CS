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
}

iSector *csMovableSectorList::FindByName (const char *name) const
{
  if (!name) return NULL;

  int i;
  for (i=0; i<Length (); i++)
  {
    iSector *sec = Get(i);
    if (sec->QueryObject ()->GetName ())
      if (!strcmp (sec->QueryObject ()->GetName (), name))
        return sec;
  }
  return NULL;
}

int csMovableSectorList::SectorList::GetSectorCount () const
{ return scfParent->Length (); }
iSector *csMovableSectorList::SectorList::GetSector (int idx) const
{ return scfParent->Get (idx); }
void csMovableSectorList::SectorList::AddSector (iSector *)
{ }
void csMovableSectorList::SectorList::RemoveSector (iSector *)
{ }
iSector *csMovableSectorList::SectorList::FindByName (const char *name) const
{ return scfParent->FindByName (name); }
int csMovableSectorList::SectorList::Find (iSector *sec) const
  { return scfParent->Find (sec); }

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
  updatenr = 0;
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
  //@@@
  // The following DecRef() is not possible because we
  // actually have a circular reference here (parent -> this and
  // this -> parent).
  //if (iparent) iparent->DecRef ();
}

void csMovable::SetParent (iMovable* p)
{
  //@@@ (see comment above)
  //iMovable* ipar = SCF_QUERY_INTERFACE_SAFE (parent, iMovable);
  //if (iparent) iparent->DecRef ();
  //iparent = ipar;
  parent = p;
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
  AddSector (sector);
}

void csMovable::ClearSectors ()
{
  if (parent == NULL)
  {
    object->GetPrivateObject ()->RemoveFromSectors ();
    sectors.SetLength (0);
  }
}

void csMovable::AddSector (iSector* sector)
{
  if (sector == NULL) return;
  if (parent == NULL)
  {
    sectors.Push (sector);
    object->GetPrivateObject ()->MoveToSector (sector->GetPrivateObject ());
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
  object->GetPrivateObject ()->UpdateMove ();

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
void csMovable::eiMovable::AddSector (iSector* sector)
  { scfParent->AddSector (sector); }
const iSectorList *csMovable::eiMovable::GetSectors () const
  { return scfParent->GetSectors (); }
iSector* csMovable::eiMovable::GetSector (int idx) const
  { return scfParent->GetSectors ()->GetSector (idx); }
bool csMovable::eiMovable::InSector () const
  { return scfParent->InSector (); }
int csMovable::eiMovable::GetSectorCount () const
  { return scfParent->GetSectorCount (); }
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
