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
IMPLEMENT_CSOBJTYPE (csRotatingObject, csWalkEntity);
IMPLEMENT_CSOBJTYPE (csLightObject, csWalkEntity);

//--------------------------------------------------------------------------

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
  transition -= elapsed_time/1000.;
  if (transition < 0) transition = 0;
  csYRotMatrix3 mat ((M_PI/2.)*transition);
  mat.Invert ();
  parent->GetMovable ().SetTransform (mat);
  parent->GetMovable ().UpdateMove ();
}

//--------------------------------------------------------------------------


csRotatingObject::csRotatingObject (csThing* p)
{
  always = true;
  parent = p;
  angles.Set (90, 0, 0);
  remaining = 0;
}

void csRotatingObject::Activate ()
{
  if (always) return;
  // Push ourselves on to the busy list if we're not already there.
  Sys->busy_entities.ObjRelease (this);
  Sys->busy_entities.ObjAdd (this);
  remaining = 10000;
}

void csRotatingObject::NextFrame (float elapsed_time)
{
  if (remaining)
  {
    remaining -= elapsed_time;
    if (remaining <= 0)
    {
      remaining = 0;
      Sys->busy_entities.ObjRelease (this);
    }
  }
  else if (!always) return;

  float trans = (2. * M_PI / 360.) * (elapsed_time / 1000.);
  csXRotMatrix3 matx (angles.x * trans);
  csYRotMatrix3 maty (angles.y * trans);
  csZRotMatrix3 matz (angles.z * trans);
  csMatrix3 mat = matz * maty * matx;
  parent->GetMovable ().Transform (mat);
  parent->GetMovable ().UpdateMove ();
}

//--------------------------------------------------------------------------

csLightObject::csLightObject (csLight* p)
{
  light = p;
  act_time = 1000;
  cur_time = 0;
}

void csLightObject::Activate ()
{
  // Push ourselves on to the busy list if we're not already there.
  Sys->busy_entities.ObjRelease (this);
  Sys->busy_entities.ObjAdd (this);
  cur_time = act_time;
}

void csLightObject::NextFrame (float elapsed_time)
{
  if (cur_time <= 0) return;
  cur_time -= elapsed_time;
  if (cur_time <= 0)
  {
    cur_time = 0;
    Sys->busy_entities.ObjRelease (this);
  }

  csColor s_color (start_color);
  csColor e_color (end_color);
  s_color *= cur_time/act_time;
  e_color *= (act_time-cur_time)/act_time;
  light->SetColor (s_color+e_color);
}

//--------------------------------------------------------------------------

