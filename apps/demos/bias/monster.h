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

#ifndef __MONSTER_H__
#define __MONSTER_H__

#include "crystalspace.h"

#include "entity.h"

class Monster : public Entity 
{
private:
  void Behaviour();

  csString factoryName;
  csRef<iMeshWrapper> mesh;
  csRef<iMeshWrapper> sword;

  csRef<CS::Animation::iSkeletonAnimNode> rootNode;
  csRef<CS::Animation::iSkeletonFSMNode> fsmNode;
  csRef<CS::Animation::iSkeletonFSMNodeFactory> fsmNodeFactory;
  csRef<CS::Animation::iSkeletonLookAtNode> lookAtNode;

public:
  float awareRadius, curAwareRadius;

public:
  Monster(iObjectRegistry*);
  ~Monster();

  virtual bool Initialize (const char* name, iSector* sector, csTransform& transform);

  virtual void PlayAnimation(const char*, bool);
  virtual void StopAnimation();

  virtual csVector3 GetPosition();

  virtual void Explode();
  virtual void ChangeMaterial();
};

#endif
