/*
    Copyright (C) 2000 by Andrew Zabolotny
    Copyright (C) 2006 by Jorrit Tyberghein

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
#include "csgeom/matrix3.h"
#include "csgeom/plane3.h"
#include "csutil/util.h"

#include "campos.h"
#include "engine.h"


csCameraPosition::csCameraPosition (csCameraPositionList* positions,
                                    const char *name,
                                    const char *sector,
                                    const csVector3 &position,
                                    const csVector3 &forward,
                                    const csVector3 &upward)
  : scfImplementationType (this),
  sector (sector), position (position), forward (forward),
  upward (upward), far_plane (0), positions (positions)
{
  SetName (name);
}

csCameraPosition::csCameraPosition (const csCameraPosition& other)
  : iBase(), scfImplementationType (this),
  sector (other.sector), position (other.position),
  forward (other.forward), upward (other.upward), far_plane (0), 
  positions (other.positions)
{
  SetName (other.GetName ());
}

csCameraPosition::~csCameraPosition ()
{
  delete far_plane;
}

void csCameraPosition::SelfDestruct ()
{
  positions->Remove (static_cast<iCameraPosition*> (this));
}

void csCameraPosition::Set (
  const char *isector,
  const csVector3 &iposition,
  const csVector3 &iforward,
  const csVector3 &iupward)
{
  sector = isector;
  position = iposition;
  forward = iforward;
  upward = iupward;
}

bool csCameraPosition::Load (iCamera *camera, iEngine *e)
{
  // First get the collection for this camera position.
  csRef<iCollection> this_collection;
  if (GetObjectParent () != 0)
  {
    this_collection = scfQueryInterface<iCollection> (GetObjectParent ());
  }

  iSector *room = e->FindSector (sector, this_collection);
  if (!room)
    room = e->FindSector (sector);	// Try globally.
  if (!room) return false;
  camera->SetSector (room);
  camera->GetTransform ().SetOrigin (position);

  upward.Normalize ();
  forward.Normalize ();
  camera->GetTransform ().LookAt (forward, upward);
  camera->SetFarPlane (far_plane);

  return true;
}

void csCameraPosition::Save (iCamera* camera)
{
  sector = camera->GetSector ()->QueryObject ()->GetName ();
  csReversibleTransform& transform = camera->GetTransform ();
  position = transform.GetOrigin ();
  upward.Set (0.0f, 1.0f, 0.0f);
  upward = transform.This2OtherRelative (upward);
  forward.Set (0.0f, 0.0f, 1.0f);
  forward = transform.This2OtherRelative (forward);
  ClearFarPlane ();
  far_plane = camera->GetFarPlane ();
  // Copy the plane if it is valid
  if (far_plane)
    far_plane = new csPlane3 (*far_plane);
}

void csCameraPosition::SetFarPlane (csPlane3 *fp)
{
  ClearFarPlane ();
  if (fp) far_plane = new csPlane3 (*fp);
}

void csCameraPosition::ClearFarPlane ()
{
  delete far_plane;
  far_plane = 0;
}

iObject *csCameraPosition::QueryObject ()
{
  return this;
}

iCameraPosition *csCameraPosition::Clone () const
{
  return new csCameraPosition (*this);
}

const char *csCameraPosition::GetSector ()
{
  return sector;
}

void csCameraPosition::SetSector (const char *Name)
{
  sector = Name;
}

const csVector3 &csCameraPosition::GetPosition ()
{
  return position;
}

void csCameraPosition::SetPosition (const csVector3 &v)
{
  position = v;
}

const csVector3 &csCameraPosition::GetUpwardVector ()
{
  return upward;
}

void csCameraPosition::SetUpwardVector (const csVector3 &v)
{
  upward = v;
}

const csVector3 &csCameraPosition::GetForwardVector ()
{
  return forward;
}

void csCameraPosition::SetForwardVector (const csVector3 &v)
{
  forward = v;
}

