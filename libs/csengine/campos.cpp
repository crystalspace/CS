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
#include "csengine/campos.h"
#include "csengine/engine.h"
#include "csgeom/matrix3.h"
#include "csutil/util.h"

SCF_IMPLEMENT_IBASE_EXT (csCameraPosition)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iCameraPosition)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csCameraPosition::CameraPosition)
  SCF_IMPLEMENTS_INTERFACE (iCameraPosition)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csCameraPosition::csCameraPosition (const char *iName, const char *iSector,
  const csVector3 &iPosition, const csVector3 &iForward,
  const csVector3 &iUpward)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiCameraPosition);
  SetName (iName);
  Sector = csStrNew (iSector);
  Position = iPosition;
  Forward = iForward;
  Upward = iUpward;
  csEngine::current_engine->AddToCurrentRegion (this);
}

csCameraPosition::~csCameraPosition ()
{
  delete [] Sector;
}

void csCameraPosition::Set (const char *iSector, const csVector3 &iPosition,
  const csVector3 &iForward, const csVector3 &iUpward)
{
  delete [] Sector;
  Sector = csStrNew (iSector);
  Position = iPosition;
  Forward = iForward;
  Upward = iUpward;
}

bool csCameraPosition::Load (iCamera* oCamera, iEngine *e)
{
  iSector *room = e->GetSectors ()->FindByName (Sector);
  if (!room) return false;
  oCamera->SetSector (room);
  oCamera->GetTransform ().SetOrigin (Position);
  csVector3 Right = Upward % Forward;
  oCamera->GetTransform ().SetT2O (
    csMatrix3 (Right.x, Upward.x, Forward.x,
               Right.y, Upward.y, Forward.y,
               Right.z, Upward.z, Forward.z));
  return true;
}


iObject *csCameraPosition::CameraPosition::QueryObject()
  { return scfParent; }
iCameraPosition *csCameraPosition::CameraPosition::Clone () const
  { return &(new csCameraPosition (*scfParent))->scfiCameraPosition; }
const char *csCameraPosition::CameraPosition::GetSector()
  { return scfParent->Sector; }
void csCameraPosition::CameraPosition::SetSector(const char *Name)
  { scfParent->Sector = csStrNew (Name); }
const csVector3 &csCameraPosition::CameraPosition::GetPosition()
  { return scfParent->Position; }
void csCameraPosition::CameraPosition::SetPosition (const csVector3 &v)
  { scfParent->Position = v; }
const csVector3 &csCameraPosition::CameraPosition::GetUpwardVector()
  { return scfParent->Upward; }
void csCameraPosition::CameraPosition::SetUpwardVector (const csVector3 &v)
  { scfParent->Upward = v; }
const csVector3 &csCameraPosition::CameraPosition::GetForwardVector()
  { return scfParent->Forward; }
void csCameraPosition::CameraPosition::SetForwardVector (const csVector3 &v)
  { scfParent->Forward = v; }
void csCameraPosition::CameraPosition::Set (const char *sector, const csVector3 &pos,
      const csVector3 &forward, const csVector3 &upward)
  { scfParent->Set (sector, pos, forward, upward); }
bool csCameraPosition::CameraPosition::Load (iCamera *c, iEngine *e)
  { return scfParent->Load (c, e); }
