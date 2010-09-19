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

iMeshWrapper* LoadMesh(iObjectRegistry* object_reg, const char* name, const char* file);

class Entity :  public scfImplementation2<Entity,iEventHandler, iObject> 
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

public:
  int HP;
  bool frozen;
  bool died;

protected:
  csRef<iVirtualClock> vc;
  /// The queue of events waiting to be handled.
  csRef<iEventQueue> eventQueue;
  /// The event name registry, used to convert event names to IDs and back.
  csRef<iEventNameRegistry> nameRegistry;

  virtual void Behaviour();

  CS_EVENTHANDLER_NAMES ("demo.entity")
  CS_EVENTHANDLER_NIL_CONSTRAINTS

private:
  virtual bool HandleEvent(iEvent& ev);

public:
  Entity(iObjectRegistry*);
  virtual ~Entity();

  virtual void Strafe(float speed);
  virtual void Step(float speed);
  virtual void Jump();
  virtual void Rotate(float speed);
  virtual void RotateCam(float x, float y);
  virtual void InterpolateMovement();

  virtual void Stop();


  virtual void PlayAnimation(const char*, bool) {}
  virtual void StopAnimation() {}
  virtual void Explode() {}
  virtual void ChangeMaterial() {}


  virtual csVector3 GetPosition() = 0;


  //iObject interface
  void SetName (const char *iName) {}
  const char* GetName () const { return "Entity"; }
  uint GetID () const { return 0; }
  void SetObjectParent (iObject *obj) {}
  iObject* GetObjectParent () const { return 0; }
  void ObjAdd (iObject *obj) {}
  void ObjRemove (iObject *obj) {}
  void ObjRemoveAll () {}
  void ObjAddChildren (iObject *Parent) {}
  iObject* GetChild (int iInterfaceID, int iVersion, const char *Name, bool FirstName) const { return 0; }
  iObject* GetChild (const char *Name) const { return 0; }
  csPtr<iObjectIterator> GetIterator () { return 0; }
  void ObjReleaseOld (iObject *obj) {}
  void AddNameChangeListener (iObjectNameChangeListener* listener) {}
  void RemoveNameChangeListener (iObjectNameChangeListener* listener) {}
  iObject* GetChild (int iInterfaceID, int iVersion, const char *Name = 0) const { return 0; }
};

#endif
