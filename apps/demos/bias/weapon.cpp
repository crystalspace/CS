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

#include "weapon.h"

#include "entity.h"
#include "monster.h"

Weapon::Weapon(iObjectRegistry* obj_reg) : scfImplementationType(this), object_reg(obj_reg)
{
  vc = csQueryRegistry<iVirtualClock> (object_reg);
  eventQueue = csQueryRegistry<iEventQueue> (object_reg);
  nameRegistry = csEventNameRegistry::GetRegistry(object_reg);

  registered = false;
  reloading = false;

  reloadtime = 1300;
  reloadingtime = 0;

  attackAnimation = "attack";
}

void Weapon::Register()
{
  if (!registered)
  {
    //Register for the Frame event, for Handle().
    eventQueue->RegisterListener (this, nameRegistry->GetID("crystalspace.frame"));
    registered = true;
  }
}

void Weapon::UnRegister()
{
  if (registered)
  {
    eventQueue->RemoveListener(this);
    registered = false;
  }
}

Weapon::~Weapon()
{
  UnRegister();
}

bool Weapon::HandleEvent(iEvent& ev)
{
  if (reloading)
  {
    csTicks elapsed_ticks = vc->GetElapsedTicks();
    reloadingtime += elapsed_ticks;
    if (reloadingtime > reloadtime)
    {
      reloading = false;
      reloadingtime = 0;
      UnRegister();
    }
  }

  return false;
}

bool Weapon::IsReady()
{
  return !reloading;
}

bool Weapon::Fire()
{
  if (reloading) return false;

  // Play the fire animation
  if (fsmNode)
    fsmNode->SwitchToState (fsmNodeFactory->FindState (attackAnimation.GetData ()));

  /*
  if (arrowsound)
  {
    arrowsound.sndstream->ResetPosition();
    arrowsound.sndstream->Unpause ();
  }
  */

  reloading = true;
  Register();

  return true;
}

void Weapon::ApplyDamage(Entity* entity)
{
  if (entity->died) return;

  int type = 1;
  int maxHP = 50;
  int points = 50;

  Monster* m = dynamic_cast<Monster*>(entity);
  if (m)
  { // Make monsters 'look around' when they're hit
    // Don't just stand there and take it!
    m->awareRadius *= 2;
  }

  if (type == 0) // melee damage
  {
    entity->HP -= points;
    entity->PlayAnimation("hit", false);
  }
  else if (type == 1) // ice damage
  {
    entity->HP -= points;
    if (points > maxHP/2)
    {
      entity->Step(0);

      entity->ChangeMaterial ();
      entity->StopAnimation();
      if (entity->HP <= 0 && !entity->frozen) // Don't explode immediatly.
        entity->HP = 1;
      entity->frozen = true;
    }
    else
      entity->PlayAnimation("hit", false);
  }
  else
    return;

  /*
  if (frozen)
  {
    entity->frozensound.sndstream->ResetPosition();
    entity->frozensound.sndstream->Unpause ();
  }
  else
  {
    entity->hitsound.sndstream->ResetPosition();
    entity->hitsound.sndstream->Unpause ();
  }
  */

  if (entity->HP <= 0)
  {
    entity->Step(0);
    if (entity->frozen)
    {
      entity->Explode();
    }
    else
    {
      entity->PlayAnimation("die", true);
    }
    entity->died = true;
  }
}
