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

#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "crystalspace.h"

#include "weapon.h"

class Entity :  public scfImplementation1<Entity,iEventHandler> 
{
protected:
  iObjectRegistry* object_reg;

protected:
  csColliderActor collider_actor;

  float cfg_walk_accelerate;
  float cfg_walk_maxspeed_multreal;
  float cfg_walk_maxspeed;

  float cfg_rotate_accelerate;
  float cfg_rotate_maxspeed;

  float cfg_jumpspeed;

  float speed;
  csVector3 desired_velocity;
  csVector3 velocity;
  csVector3 desired_angle_velocity;
  csVector3 angle_velocity;

  csRef<Weapon> weapon;

protected:
  csRef<iVirtualClock> vc;
  /// The queue of events waiting to be handled.
  csRef<iEventQueue> eventQueue;
  /// The event name registry, used to convert event names to IDs and back.
  csRef<iEventNameRegistry> nameRegistry;

  virtual bool HandleEvent(iEvent& ev);

  virtual void Behaviour();

  CS_EVENTHANDLER_NAMES ("demo.entity")
  CS_EVENTHANDLER_NIL_CONSTRAINTS

public:
  Entity(iObjectRegistry* or);
  ~Entity();

  iMeshWrapper* LoadMesh(const char* name, const char* file);

  virtual void Fire(int x, int y);

  virtual void Strafe(float speed);
  virtual void Step(float speed);
  virtual void Jump();
  virtual void Rotate(float speed);
  virtual void RotateCam(float x, float y);
  virtual void InterpolateMovement();
};

#endif
