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

Weapon::Weapon(iObjectRegistry* obj_reg) : scfImplementationType(this), object_reg(obj_reg)
{
  vc = csQueryRegistry<iVirtualClock> (object_reg);
  eventQueue = csQueryRegistry<iEventQueue> (object_reg);
  nameRegistry = csEventNameRegistry::GetRegistry(object_reg);

  registered = false;
  reloading = false;

  reloadtime = 1300;
  reloadingtime = 0;
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

  if (mesh)
  {
    csRef<iGeneralMeshState> spstate (scfQueryInterface<iGeneralMeshState> (mesh->GetMeshObject ()));
    csRef<iGenMeshSkeletonControlState> animcontrol (scfQueryInterface<iGenMeshSkeletonControlState> (spstate->GetAnimationControl ()));
    iSkeleton* skeleton = animcontrol->GetSkeleton ();

    skeleton->StopAll();
    skeleton->Execute("shoot");
    skeleton->Append("reload");
  }

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

void Weapon::ApplyDamage()
{
}
