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

IMPLEMENT_IBASE_EXT (csCameraPosition)
  IMPLEMENTS_EMBEDDED_INTERFACE (iCameraPosition)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csCameraPosition::CameraPosition)
  IMPLEMENTS_INTERFACE (iCameraPosition)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_CSOBJTYPE (csCameraPosition,csPObject);

csCameraPosition::csCameraPosition (const char *iName, const char *iSector,
  const csVector3 &iPosition, const csVector3 &iForward,
  const csVector3 &iUpward)
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiCameraPosition);
  SetName (iName);
  Sector = strnew (iSector);
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
  Sector = strdup (iSector);
  Position = iPosition;
  Forward = iForward;
  Upward = iUpward;
}

bool csCameraPosition::Load (csCamera &oCamera, csEngine *e)
{
  csSector *room = (csSector *)e->sectors.FindByName (Sector);
  if (!room) return false;
  oCamera.SetSector (room);
  oCamera.SetPosition (Position);
  csVector3 Right = Upward % Forward;
  oCamera.SetC2W (
    csMatrix3 (Right.x, Upward.x, Forward.x,
               Right.y, Upward.y, Forward.y,
               Right.z, Upward.z, Forward.z));
  return true;
}
