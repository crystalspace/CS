/*
  Copyright (C) 2010 Jelle Hellemans

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "player.h"

Player::Player(iObjectRegistry* obj_reg) : Entity(obj_reg)
{
  csRef<iView> view (csQueryRegistry<iView> (object_reg));
  csRef<iConfigManager> cfg (csQueryRegistry<iConfigManager> (object_reg));


  float cfg_body_height = cfg->GetFloat ("Walktest.CollDet.BodyHeight", 1.4f);
  float cfg_body_width = cfg->GetFloat ("Walktest.CollDet.BodyWidth", 0.5f);
  float cfg_body_depth = cfg->GetFloat ("Walktest.CollDet.BodyDepth", 0.5f);
  float cfg_eye_offset = cfg->GetFloat ("Walktest.CollDet.EyeOffset", -0.7f);
  float cfg_legs_width = cfg->GetFloat ("Walktest.CollDet.LegsWidth", 0.4f);
  float cfg_legs_depth = cfg->GetFloat ("Walktest.CollDet.LegsDepth", 0.4f);
  float cfg_legs_offset = cfg->GetFloat ("Walktest.CollDet.LegsOffset", -1.1f);

  csVector3 legs (cfg_legs_width, cfg_eye_offset-cfg_legs_offset, cfg_legs_depth);
  csVector3 body (cfg_body_width, cfg_body_height, cfg_body_depth);
  csVector3 shift (0, cfg_legs_offset, 0);
  collider_actor.InitializeColliders (view->GetCamera (), legs, body, shift);
  collider_actor.SetCamera (view->GetCamera (), true);


  weapon->mesh = LoadMesh("gencrossbow", "/data/bias/models/crossbow/crossbow");
}

Player::~Player()
{
}

bool Player::HandleEvent(iEvent& ev)
{
  //Update weapon transform
  csRef<iView> view (csQueryRegistry<iView> (object_reg));
  iCamera* cam = view->GetCamera();
  csOrthoTransform trans = cam->GetTransform();
  iSector* sector = cam->GetSector();

  float yrot = 0;
  csVector3 pos(0,0,0);
  pos.z += 0.50f;
  pos.y -= 0.20f;
  pos.x += 0.20f;
  yrot  += 0.07f;

  pos =trans.This2OtherRelative(pos);
  trans.Translate(pos);

  weapon->mesh->GetMovable()->SetTransform(trans);
  weapon->mesh->GetMovable()->Transform(csYRotMatrix3(yrot));
  weapon->mesh->GetMovable()->SetSector(sector);
  weapon->mesh->GetMovable()->UpdateMove();

  //Update movement
  csTicks elapsed_time = vc->GetElapsedTicks ();

  float delta = float (elapsed_time) / 1000.0f;
  collider_actor.Move (delta, speed, velocity, angle_velocity);

  InterpolateMovement ();

  return false;
}

void Player::Fire(int x, int y)
{
  if (weapon->IsReady())
  {
    csRef<iView> view (csQueryRegistry<iView> (object_reg));
    csScreenTargetResult result = csEngineTools::FindScreenTarget (
      csVector2 (x, y), 1000.0f, view->GetCamera ());

    if (weapon->Fire() && result.mesh)
    {
      printf("PLAYER HIT\n");
      weapon->ApplyDamage();
    }
  }
}
