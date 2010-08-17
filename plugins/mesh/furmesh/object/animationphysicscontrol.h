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

#ifndef __ANIMATION_PHYSICS_CONTROL_H__
#define __ANIMATION_PHYSICS_CONTROL_H__

#include "crystalspace.h"

#include "csutil/scf_implementation.h"

CS_PLUGIN_NAMESPACE_BEGIN(FurMesh)
{
  class AnimationPhysicsControl : public scfImplementation2 
    <AnimationPhysicsControl, CS::Mesh::iFurPhysicsControl, iComponent>
  {
  public:
    CS_LEAKGUARD_DECLARE(AnimationPhysicsControl);

    AnimationPhysicsControl (iBase* parent);
    virtual ~AnimationPhysicsControl ();

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*);

    //-- iFurPhysicsControl
    virtual void SetInitialTransform(csReversibleTransform ){}
    virtual void SetAnimesh (CS::Mesh::iAnimatedMesh* animesh);
    virtual void SetRigidBody (iRigidBody* ){}
    virtual void SetBulletDynamicSystem 
      (CS::Physics::Bullet::iDynamicSystem* ){}
    virtual void InitializeStrand (size_t strandID, csVector3* coordinates,
      size_t coordinatesCount);
    virtual void AnimateStrand (size_t strandID, csVector3* coordinates, 
      size_t coordinatesCount) const;
    virtual void RemoveStrand (size_t strandID);
    virtual void RemoveAllStrands ();

  private:
    struct SphericalCoordinates
    {
      float radius;
      float inclination;
      float azimuth;
    };

    struct Directions
    {
      SphericalCoordinates *sc;
      size_t count;
    };

    struct Anchor
    {
      size_t animeshVertexIndex;
      Directions* direction;
    };

    iObjectRegistry* object_reg;
    csHash<Anchor*, size_t> guideRopes;
    CS::Mesh::iAnimatedMesh* animesh; // use csRef here
  };
}
CS_PLUGIN_NAMESPACE_END(FurMesh)

#endif // __ANIMATION_PHYSICS_CONTROL_H__
