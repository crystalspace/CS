/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

#include "csgeom/transfrm.h"
#include "iengine/engine.h"
#include "iengine/light.h"
#include "iengine/movable.h"
#include "iengine/sector.h"

#include "bot.h"

Bot::Bot (iEngine *Engine, iMeshWrapper* botmesh)
{
  engine = Engine;
  mesh = botmesh;
  do
  {
    d.x = 6*(((float)rand ()) / (float)RAND_MAX)-3;
    d.y = 6*(((float)rand ()) / (float)RAND_MAX)-3;
    d.z = 6*(((float)rand ()) / (float)RAND_MAX)-3;
  }
  while ((d * d) < EPSILON);
  d = d.Unit ();
}

Bot::~Bot ()
{
}

void Bot::set_bot_move (const csVector3& v)
{
  mesh->GetMovable()->SetPosition (v);
  follow = v;
  mesh->GetMovable()->UpdateMove ();
}

void Bot::move (csTicks elapsed_time)
{
  csOrthoTransform old_pos (mesh->GetMovable()->GetTransform ().GetO2T (), follow);
  csVector3 rd = (8.*(float)elapsed_time)/1000. * d;
  follow += rd;
  csVector3 new_pos = follow;
  iSector* s = f_sector;
  bool mirror = false;
  csVector3 v_old_pos = new_pos;
  s = s->FollowSegment (old_pos, new_pos, mirror);
  if (s &&
      ABS (v_old_pos.x-new_pos.x) < SMALL_EPSILON &&
      ABS (v_old_pos.y-new_pos.y) < SMALL_EPSILON &&
      ABS (v_old_pos.z-new_pos.z) < SMALL_EPSILON)
  {
    f_sector = s;
  }
  else
  {
    follow -= rd;
    do
    {
      d.x = 6*(((float)rand ()) / (float)RAND_MAX)-3;
      d.y = 6*(((float)rand ()) / (float)RAND_MAX)-3;
      d.z = 6*(((float)rand ()) / (float)RAND_MAX)-3;
    }
    while ((d * d) < EPSILON);
    d = d.Unit ();
  }

  csVector3 old_p = mesh->GetMovable()->GetPosition ();
  csVector3 dir = follow-old_p;
  dir.Normalize ();
  csVector3 new_p = old_p + ((3.*(float)elapsed_time)/1000.)*dir;
  mesh->GetMovable()->SetPosition (new_p);

  //@@@
  s = mesh->GetMovable()->GetSectors ()->Get (0);
  mirror = false;
  csOrthoTransform old_pos2 (mesh->GetMovable()->GetTransform ().GetO2T (), old_p);
  s = s->FollowSegment (old_pos2, new_p, mirror);
  if (s)
  {
    mesh->GetMovable()->SetSector (s);
    if (light)
    {
      if (s != light->GetSector ())
      {
        light->IncRef ();
        light->GetSector ()->GetLights ()->Remove (light);
        s->GetLights ()->Add (light);
        light->DecRef ();
      }
      light->SetCenter (new_p);
      light->Setup ();
    }
  }
  mesh->GetMovable()->UpdateMove ();
}
