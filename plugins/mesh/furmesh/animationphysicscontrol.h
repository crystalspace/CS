/*
  Copyright (C) 2010 Alexandru - Teodor Voicu

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

#include <iutil/comp.h>
#include <csgeom/vector3.h>
#include <imesh/furmesh.h>
#include <csgfx/shadervarcontext.h>
#include <imesh/genmesh.h>

#include "crystalspace.h"

#include "csutil/scf_implementation.h"

struct iObjectRegistry;

CS_PLUGIN_NAMESPACE_BEGIN(FurMesh)
{
  struct csGuideHairAnimation
  {
    csVector3 *controlPoints;
    size_t controlPointsCount;
  };

  class AnimationPhysicsControl : public scfImplementation2 <AnimationPhysicsControl, 
    iFurPhysicsControl, iComponent>
  {
  public:
    CS_LEAKGUARD_DECLARE(AnimationPhysicsControl);

    AnimationPhysicsControl (iBase* parent);
    virtual ~AnimationPhysicsControl ();

    // From iComponent	
    virtual bool Initialize (iObjectRegistry*);

    //-- iFurPhysicsControl
    virtual void SetInitialTransform(csReversibleTransform initialTransform);
    virtual void SetRigidBody (iRigidBody* rigidBody);
    virtual void SetBulletDynamicSystem (iBulletDynamicSystem* bulletDynamicSystem);
    // Initialize the strand with the given ID
    virtual void InitializeStrand (size_t strandID, csVector3* coordinates,
      size_t coordinatesCount);
    // Animate the strand with the given ID
    virtual void AnimateStrand (size_t strandID, csVector3* coordinates, size_t
      coordinatesCount);
    virtual void RemoveStrand (size_t strandID);
    virtual void RemoveAllStrands ();

  private:
    iObjectRegistry* object_reg;
    csHash<csGuideHairAnimation*, size_t> guideRopes;
    csRef<iRigidBody> rigidBody;
    csReversibleTransform initialTransform;
  };
}
CS_PLUGIN_NAMESPACE_END(FurMesh)

#endif // __ANIMATION_PHYSICS_CONTROL_H__
