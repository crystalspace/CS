/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#include "sysdef.h"
#include "walktest/bot.h"
#include "csengine/sector.h"
#include "csengine/light/light.h"
#include "csobject/nameobj.h"

CSOBJTYPE_IMPL(Bot,csSprite3D);

Bot::Bot (csSpriteTemplate* tmpl) : csSprite3D ()
{
  SetTemplate (tmpl);
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
  csSprite3D::SetMove (v);
  follow = v;
}

void Bot::move (long elapsed_time)
{
  csOrthoTransform old_pos (GetW2T (), follow);
  csVector3 rd = (8.*(float)elapsed_time)/1000. * d;
  follow += rd;
  csVector3 new_pos = follow;
  csSector* s = f_sector;
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

  csVector3 old_p = GetW2TTranslation ();
  csVector3 dir = (follow-old_p).Unit ();
  csVector3 new_p = old_p + ((3.*(float)elapsed_time)/1000.)*dir;
  csSprite3D::SetMove (new_p);

  s = (csSector*)sectors[0];
  mirror = false;
  csOrthoTransform old_pos2 (GetW2T (), old_p);
  s = s->FollowSegment (old_pos2, new_p, mirror);
  if (s)
  {
    MoveToSector (s);
    if (light)
    {
      light->Move (s, new_p.x, new_p.y, new_p.z);
      light->Setup ();
    }
  }
}

