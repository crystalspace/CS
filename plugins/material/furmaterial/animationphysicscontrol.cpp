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
#include <cssysdef.h>
#include <iutil/objreg.h>
#include <iutil/plugin.h>

#include "furmaterial.h"
#include "animationphysicscontrol.h"

CS_PLUGIN_NAMESPACE_BEGIN(FurMesh)
{
  /*************************
  *  AnimationPhysicsControl
  **************************/
  SCF_IMPLEMENT_FACTORY (AnimationPhysicsControl)

    CS_LEAKGUARD_IMPLEMENT(AnimationPhysicsControl);	

  AnimationPhysicsControl::AnimationPhysicsControl (iBase* parent)
    : scfImplementationType (this, parent), object_reg(0)
  {
  }

  AnimationPhysicsControl::~AnimationPhysicsControl ()
  {
    RemoveAllStrands();
  }

  // From iComponent
  bool AnimationPhysicsControl::Initialize (iObjectRegistry* r)
  {
    initialTransform.Identity();
    object_reg = r;
    return true;
  }

  //-- iFurPhysicsControl

  void AnimationPhysicsControl::SetInitialTransform(csReversibleTransform initialTransform)
  {
    this->initialTransform = initialTransform;
  }

  void AnimationPhysicsControl::SetRigidBody (iRigidBody* rigidBody)
  {
    this->rigidBody = rigidBody;
  }

  void AnimationPhysicsControl::SetBulletDynamicSystem (iBulletDynamicSystem* 
    bulletDynamicSystem)
  {
  }

  // Initialize the strand with the given ID
  void AnimationPhysicsControl::InitializeStrand (size_t strandID, csVector3* 
    coordinates, size_t coordinatesCount)
  {
    csGuideHairAnimation* guideHairAnimation;

    guideHairAnimation = new csGuideHairAnimation;

    guideHairAnimation->controlPointsCount = coordinatesCount;
    guideHairAnimation->controlPoints = new csVector3[coordinatesCount];

    for (size_t i = 0 ; i < coordinatesCount ; i ++)
      guideHairAnimation->controlPoints[i] = coordinates[i];

    guideRopes.PutUnique(strandID, guideHairAnimation);
  }

  // Animate the strand with the given ID
  void AnimationPhysicsControl::AnimateStrand (size_t strandID, csVector3* 
    coordinates, size_t coordinatesCount)
  {
    csReversibleTransform currentTransform = 
      initialTransform * rigidBody->GetTransform();

    csGuideHairAnimation *guideHairAnimation = guideRopes.Get(strandID, 0);

    if (guideHairAnimation)
    {
      for ( size_t i = 0 ; i < coordinatesCount ; i ++ )
        coordinates[i] = guideHairAnimation->controlPoints[i] * currentTransform.GetInverse();
    }
  }

  void AnimationPhysicsControl::RemoveStrand (size_t strandID)
  {
    csGuideHairAnimation* guideHairAnimation = guideRopes.Get(strandID, 0);

    if (guideHairAnimation)
    {
      guideRopes.Delete(strandID, guideHairAnimation);

      if (guideHairAnimation->controlPoints && guideHairAnimation->controlPointsCount)
        delete guideHairAnimation->controlPoints;

      delete guideHairAnimation;
    }
  }

  void AnimationPhysicsControl::RemoveAllStrands ()
  {
    for (size_t i = 0 ; i < guideRopes.GetSize(); i ++)
    {
      csGuideHairAnimation* guideHairAnimation = guideRopes.Get(i, 0);

      if (guideHairAnimation)
      {
        if (guideHairAnimation->controlPoints && guideHairAnimation->controlPointsCount)
          delete guideHairAnimation->controlPoints;
      
        delete guideHairAnimation;
      }
    }

    guideRopes.DeleteAll();
  }

}
CS_PLUGIN_NAMESPACE_END(FurMesh)

