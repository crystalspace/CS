/*
    Copyright (C) 2000 by Jorrit Tyberghein

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
#include "walktest/wentity.h"
#include "walktest/walktest.h"
#include "csengine/thing.h"
#include "csgeom/matrix3.h"

extern WalkTest* Sys;

IMPLEMENT_CSOBJTYPE (csEntityList, csObject);
IMPLEMENT_CSOBJTYPE (csWalkEntity, csObject);
IMPLEMENT_CSOBJTYPE (csDoor, csWalkEntity);

csDoor::csDoor (csThing* p)
{
  is_open = false;
  transition = 0;
  parent = p;
}

void csDoor::Activate ()
{
printf ("Activate Door!\n");
  is_open = !is_open;
  // We do 1-transition here to make sure that when we
  // activate the door while in mid-transition it will
  // just go back from that point.
  transition = 1-transition;
  // Push ourselves on to the busy list if we're not already there.
  Sys->busy_entities.ObjRelease (this);
  Sys->busy_entities.ObjAdd (this);
}

void csDoor::NextFrame (float elapsed_time)
{
  if (!transition)
  {
    Sys->busy_entities.ObjRelease (this);
printf ("Done opening door.\n");
    return;
  }
  transition -= (float)elapsed_time/1000.;
  if (transition < 0) transition = 0;
  csYRotMatrix3 mat ((M_PI/2.)*transition);
  mat.Invert ();
  parent->SetTransform (mat);
  parent->Transform ();
}

