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
#include "plugins/engine/3d/campos.h"
#include "plugins/engine/3d/engine.h"
#include "csgeom/matrix3.h"
#include "csgeom/plane3.h"
#include "csutil/util.h"

SCF_IMPLEMENT_IBASE_EXT(csCameraPosition)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iCameraPosition)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csCameraPosition::CameraPosition)
  SCF_IMPLEMENTS_INTERFACE(iCameraPosition)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csCameraPosition::csCameraPosition (
  const char *name,
  const char *sector,
  const csVector3 &position,
  const csVector3 &forward,
  const csVector3 &upward)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiCameraPosition);
  SetName (name);
  csCameraPosition::sector = csStrNew (sector);
  csCameraPosition::position = position;
  csCameraPosition::forward = forward;
  csCameraPosition::upward = upward;
  far_plane = 0;
}

csCameraPosition::~csCameraPosition ()
{
  delete[] sector;
  delete[] far_plane;
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiCameraPosition);
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
  iSector *room = e->GetSectors ()->FindByName (sector);
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

iObject *csCameraPosition::CameraPosition::QueryObject ()
{
  return scfParent;
}

iCameraPosition *csCameraPosition::CameraPosition::Clone () const
{
  return &((new csCameraPosition (*scfParent))->scfiCameraPosition);
}

const char *csCameraPosition::CameraPosition::GetSector ()
{
  return scfParent->sector;
}

void csCameraPosition::CameraPosition::SetSector (const char *Name)
{
  scfParent->sector = csStrNew (Name);
}

const csVector3 &csCameraPosition::CameraPosition::GetPosition ()
{
  return scfParent->position;
}

void csCameraPosition::CameraPosition::SetPosition (const csVector3 &v)
{
  scfParent->position = v;
}

const csVector3 &csCameraPosition::CameraPosition::GetUpwardVector ()
{
  return scfParent->upward;
}

void csCameraPosition::CameraPosition::SetUpwardVector (const csVector3 &v)
{
  scfParent->upward = v;
}

const csVector3 &csCameraPosition::CameraPosition::GetForwardVector ()
{
  return scfParent->forward;
}

void csCameraPosition::CameraPosition::SetForwardVector (const csVector3 &v)
{
  scfParent->forward = v;
}

void csCameraPosition::CameraPosition::Set (
  const char *sector,
  const csVector3 &pos,
  const csVector3 &forward,
  const csVector3 &upward)
{
  scfParent->Set (sector, pos, forward, upward);
}

bool csCameraPosition::CameraPosition::Load (iCamera *c, iEngine *e)
{
  return scfParent->Load (c, e);
}
