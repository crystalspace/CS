/*
  Copyright (C) 2010 Alexandru - Teodor Voicu
      Faculty of Automatic Control and Computer Science of the "Politehnica"
      University of Bucharest
      http://csite.cs.pub.ro/index.php/en/

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __HAIR_PHYSICS_CONTROL_H__
#define __HAIR_PHYSICS_CONTROL_H__

#include "crystalspace.h"

#include "csutil/scf_implementation.h"

CS_PLUGIN_NAMESPACE_BEGIN(FurMesh)
{
  class HairPhysicsControl : public scfImplementation2 
    <HairPhysicsControl, scfFakeInterface<CS::Animation::iFurAnimationControl>, 
    CS::Animation::iFurPhysicsControl>
  {
  public:
    CS_LEAKGUARD_DECLARE(HairPhysicsControl);

    HairPhysicsControl (iBase *parent);
    virtual ~HairPhysicsControl ();

    //-- iFurPhysicsControl
    virtual void SetAnimatedMesh (CS::Mesh::iAnimatedMesh* animesh);
    virtual void SetRigidBody (iRigidBody* rigidBody);
    virtual void SetBulletDynamicSystem 
      (CS::Physics::Bullet::iDynamicSystem* bulletDynamicSystem);
    virtual void InitializeStrand (size_t strandID, csVector3* coordinates,
      size_t coordinatesCount);
    virtual void AnimateStrand (size_t strandID, csVector3* coordinates, 
      size_t coordinatesCount) const;
    virtual void RemoveStrand (size_t strandID);
    virtual void RemoveAllStrands ();

  private:
    struct Anchor
    {
      size_t animeshVertexIndex;
      csRef<CS::Physics::Bullet::iSoftBody> softBody;
    };

    csHash<Anchor*, size_t > guideRopes;
    csRef<iRigidBody> rigidBody;
    csRef<CS::Physics::Bullet::iDynamicSystem> bulletDynamicSystem;
    CS::Mesh::iAnimatedMesh* animesh;
    size_t maxRange;
  };
}
CS_PLUGIN_NAMESPACE_END(FurMesh)

#endif // __HAIR_PHYSICS_CONTROL_H__
