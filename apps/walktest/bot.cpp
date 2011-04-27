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
#include "iengine/mesh.h"
#include "imesh/sprite3d.h"
#include "imesh/object.h"

#include "bot.h"
#include "walktest.h"
#include "missile.h"
#include "lights.h"

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

void Bot::SetBotMove (const csVector3& v)
{
  mesh->GetMovable()->SetPosition (v);
  follow = v;
  mesh->GetMovable()->UpdateMove ();
}

void Bot::Move (csTicks elapsed_time)
{
  csOrthoTransform old_pos (mesh->GetMovable()->GetTransform ().GetO2T (), follow);
  csVector3 rd = (8.*(float)elapsed_time)/1000. * d;
  if (fabs (rd.x) < .00001 && fabs (rd.y) < .00001 && fabs (rd.z) < .00001)
    return;
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
  csVector3 d = ((3.*(float)elapsed_time)/1000.)*dir;
  if (fabs (d.x) < .00001 && fabs (d.y) < .00001 && fabs (d.z) < .00001)
    return;
  csVector3 new_p = old_p + d;
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
      iSector* currSector = light->GetMovable()->GetSectors()->Get(0);
      if (s != currSector)
      {
        light->IncRef ();
        currSector->GetLights ()->Remove (light);
        s->GetLights ()->Add (light);
        light->DecRef ();
      }
      light->GetMovable()->SetPosition (new_p);
      light->GetMovable()->UpdateMove();
    }
  }
  mesh->GetMovable()->UpdateMove ();
}

BotManager::BotManager (WalkTest* walktest) : walktest (walktest)
{
}

Bot* BotManager::CreateBot (iSector* where, const csVector3& pos, float dyn_radius, bool manual)
{
  csRef<iLight> dyn;
  if (dyn_radius)
  {
    dyn = walktest->lights->CreateRandomLight (pos, where, dyn_radius);
  }
  iMeshFactoryWrapper* tmpl = walktest->Engine->GetMeshFactories ()
  	->FindByName ("bot");
  if (!tmpl) return 0;
  csRef<iMeshObject> botmesh (tmpl->GetMeshObjectFactory ()->NewInstance ());
  csRef<iMeshWrapper> botWrapper = walktest->Engine->CreateMeshWrapper (botmesh, "bot",
    where);

  botWrapper->GetMovable ()->UpdateMove ();
  csRef<iSprite3DState> state (scfQueryInterface<iSprite3DState> (botmesh));
  state->SetAction ("default");
  
  Bot* bot = new Bot (walktest->Engine, botWrapper);
  bot->SetBotMove (pos);
  bot->SetBotSector (where);
  bot->light = dyn;
  if (manual)
    manual_bots.Push (bot);
  else
    bots.Push (bot);
  return bot;
}

void BotManager::DeleteOldestBot (bool manual)
{
  if (manual)
  {
    if (manual_bots.GetSize () > 0)
      manual_bots.DeleteIndex (0);
  }
  else
  {
    if (bots.GetSize () > 0)
      bots.DeleteIndex (0);
  }
}

void BotManager::MoveBots (csTicks elapsed_time)
{
  size_t i;
  for (i = 0; i < bots.GetSize (); i++)
  {
    bots[i]->Move (elapsed_time);
  }

}

