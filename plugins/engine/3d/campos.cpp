/*
    Copyright (C) 2000 by Andrew Zabolotny

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
#include "plugins/engine/3d/campos.h"
#include "plugins/engine/3d/engine.h"


csCameraPosition::csCameraPosition (
  const char *name,
  const char *sector,
  const csVector3 &position,
  const csVector3 &forward,
  const csVector3 &upward)
  : scfImplementationType (this),
  sector (csStrNew (sector)), position (position), forward (forward),
  upward (upward), far_plane (0)
{
  SetName (name);
}

csCameraPosition::csCameraPosition (const csCameraPosition& other)
  : iBase(), scfImplementationType (this),
  sector (csStrNew (other.sector)), position (other.position), forward (other.forward),
  upward (other.upward), far_plane (0)
{
  SetName (other.GetName ());
}

csCameraPosition::~csCameraPosition ()
{
  delete[] sector;
  delete[] far_plane;
}

void csCameraPosition::Set (
  const char *isector,
  const csVector3 &iposition,
  const csVector3 &iforward,
  const csVector3 &iupward)
{
  delete[] sector;
  sector = csStrNew (isector);
  position = iposition;
  forward = iforward;
  upward = iupward;
}

bool csCameraPosition::Load (iCamera *camera, iEngine *e)
{
  // First get the region for this camera position.
  csRef<iRegion> this_region;
  if (GetObjectParent () != 0)
  {
    this_region = SCF_QUERY_INTERFACE (GetObjectParent (), iRegion);
  }

  iSector *room = e->FindSector (sector, this_region);
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
  sector = csStrNew (Name);
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

