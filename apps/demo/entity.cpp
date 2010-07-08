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

#include "entity.h"

#include <string>

iMeshWrapper* Entity::LoadMesh(const char* name, const char* file)
{
  csRef<iVFS> vfs (csQueryRegistry<iVFS> (object_reg));
  csRef<iLoader> loader (csQueryRegistry<iLoader> (object_reg));
  csRef<iEngine> engine (csQueryRegistry<iEngine> (object_reg));

  std::string s(file);
  size_t pos = s.find_last_of("/");
  s = s.substr(0, pos+1);

  printf(s.c_str());

  vfs->ChDir (s.c_str());
  bool l = loader->LoadLibraryFile(file);
  if (!l) printf("LoadMesh failed!\n");

  iMeshFactoryWrapper* mf = engine->FindMeshFactory(name);

  csRef<iMeshWrapper> m = engine->CreateMeshWrapper(mf, name);

  return m;
}

Entity::Entity(iObjectRegistry* or) : scfImplementationType(this), object_reg(or)
{
  vc = csQueryRegistry<iVirtualClock> (object_reg);
  eventQueue = csQueryRegistry<iEventQueue> (object_reg);
  nameRegistry = csEventNameRegistry::GetRegistry(object_reg);

  eventQueue->RegisterListener (this, nameRegistry->GetID("crystalspace.frame"));

  cfg_walk_accelerate = 0.01f;
  cfg_walk_maxspeed = 0.1f;
  cfg_walk_maxspeed_multreal = 1.0f;

  cfg_rotate_accelerate = 0.005f;
  cfg_rotate_maxspeed = 0.03f;

  cfg_jumpspeed = 0.08f;

  speed = 1.0f;
  desired_velocity.Set(0.0f);
  velocity.Set(0.0f);
  desired_angle_velocity.Set(0.0f);
  angle_velocity.Set(0.0f);

  csRef<iCollideSystem> collide_system (csQueryRegistry<iCollideSystem> (object_reg));
  csRef<iEngine> engine (csQueryRegistry<iEngine> (object_reg));

  collider_actor.SetCollideSystem (collide_system);
  collider_actor.SetEngine (engine);
  collider_actor.SetCD (true);

  weapon.AttachNew(new Weapon(object_reg));
}

Entity::~Entity()
{
}

bool Entity::HandleEvent(iEvent& ev)
{
  Behaviour();

  //Update movement
  csTicks elapsed_time = vc->GetElapsedTicks ();

  float delta = float (elapsed_time) / 1000.0f;
  collider_actor.Move (delta, speed, velocity, angle_velocity);

  InterpolateMovement ();

  return false;
}

void Entity::Behaviour()
{
}

void Entity::Fire(int x, int y)
{
  if (weapon->IsReady())
  {
    csRef<iView> view (csQueryRegistry<iView> (object_reg));
    csScreenTargetResult result = csEngineTools::FindScreenTarget (
      csVector2 (x, y), 1000.0f, view->GetCamera ());

    if (weapon->Fire() && result.mesh)
    {
      printf("ENTITY HIT\n");
      weapon->ApplyDamage();
    }
  }
}

void Entity::Strafe (float speed)
{
  speed *= cfg_walk_maxspeed_multreal;
  desired_velocity.x = 140.0f * speed * cfg_walk_maxspeed
  	* cfg_walk_maxspeed_multreal;
}

void Entity::Step (float speed)
{
  speed *= cfg_walk_maxspeed_multreal;
  desired_velocity.z = 140.0f * speed * cfg_walk_maxspeed
  	* cfg_walk_maxspeed_multreal;
}

void Entity::Jump ()
{
  velocity.y = 110.0f * cfg_jumpspeed;
  desired_velocity.y = 0.0f;
}

void Entity::Rotate (float speed)
{
  desired_angle_velocity.y = 100.0f * speed * cfg_rotate_maxspeed
  	* cfg_walk_maxspeed_multreal;
}

void Entity::RotateCam (float x, float y)
{
  csVector3 rot = collider_actor.GetRotation ();
  rot.x += x;
  rot.y += y;
  collider_actor.SetRotation (rot);
}

void Entity::InterpolateMovement ()
{
  float elapsed = vc->GetElapsedTicks () / 1000.0f;
  elapsed *= 1700.0f;

  for (size_t i = 0; i < 3; i++)
  {
    if (velocity[i] < desired_velocity[i])
    {
      velocity[i] += cfg_walk_accelerate * elapsed;
      if (velocity[i] > desired_velocity[i])
        velocity[i] = desired_velocity[i];
    }
    else
    {
      velocity[i] -= cfg_walk_accelerate * elapsed;
      if (velocity[i] < desired_velocity[i])
        velocity[i] = desired_velocity[i];
    }
  }

  for (size_t i = 0; i < 3; i++)
  {
    if (angle_velocity[i] < desired_angle_velocity[i])
    {
      angle_velocity[i] += cfg_rotate_accelerate * elapsed;
      if (angle_velocity[i] > desired_angle_velocity[i])
        angle_velocity[i] = desired_angle_velocity[i];
    }
    else
    {
      angle_velocity[i] -= cfg_rotate_accelerate * elapsed;
      if (angle_velocity[i] < desired_angle_velocity[i])
        angle_velocity[i] = desired_angle_velocity[i];
    }
  }
}