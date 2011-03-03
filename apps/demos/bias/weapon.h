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

#ifndef __WEAPON_H__
#define __WEAPON_H__

#include "crystalspace.h"

class Entity;

class Weapon :  public scfImplementation1<Weapon,iEventHandler> 
{
private:
  iObjectRegistry* object_reg;

private:
  csRef<iVirtualClock> vc;
  /// The queue of events waiting to be handled.
  csRef<iEventQueue> eventQueue;
  /// The event name registry, used to convert event names to IDs and back.
  csRef<iEventNameRegistry> nameRegistry;

  bool HandleEvent(iEvent& ev);

  CS_EVENTHANDLER_NAMES ("demo.weapon")
  CS_EVENTHANDLER_NIL_CONSTRAINTS

private:
  bool registered;
  void Register();
  void UnRegister();

  bool reloading;
  int reloadtime;
  int reloadingtime;

public:
  csRef<iMeshWrapper> mesh;
  csRef<CS::Animation::iSkeletonFSMNode> fsmNode;
  csRef<CS::Animation::iSkeletonFSMNodeFactory> fsmNodeFactory;
  csString attackAnimation;

public:
  Weapon(iObjectRegistry*);
  ~Weapon();

  bool IsReady();
  bool Fire();

  void ApplyDamage(Entity* entity);
};

#endif
