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

#include <cssysdef.h>

#include "furmesh.h"
#include "hairphysicscontrol.h"

CS_PLUGIN_NAMESPACE_BEGIN(FurMesh)
{
  /********************
  *  HairPhysicsControl
  ********************/

  SCF_IMPLEMENT_FACTORY (HairPhysicsControl)

  CS_LEAKGUARD_IMPLEMENT(HairPhysicsControl);	

  HairPhysicsControl::HairPhysicsControl (iBase* parent)
    : scfImplementationType (this, parent), object_reg(0), rigidBody(0),
    bulletDynamicSystem(0)
  {
  }

  HairPhysicsControl::~HairPhysicsControl ()
  {
    RemoveAllStrands();
  }

  bool HairPhysicsControl::Initialize (iObjectRegistry* r)
  {
    object_reg = r;
    return true;
  }

  //-- iFurPhysicsControl
  // No initial transformation specified / needed
  void HairPhysicsControl::SetInitialTransform(csReversibleTransform)
  {
  }

  void HairPhysicsControl::SetRigidBody (iRigidBody* rigidBody)
  {
    this->rigidBody = rigidBody;
  }

  void HairPhysicsControl::SetBulletDynamicSystem (CS::Physics::Bullet::iDynamicSystem* 
    bulletDynamicSystem)
  {
    this->bulletDynamicSystem = bulletDynamicSystem;
  }

  // Initialize the strand with the given ID
  void HairPhysicsControl::InitializeStrand (size_t strandID, csVector3* 
    coordinates, size_t coordinatesCount)
  {
    if (!rigidBody || !bulletDynamicSystem)
      return;

    CS::Physics::Bullet::iSoftBody* bulletBody = bulletDynamicSystem->
      CreateRope(coordinates, coordinatesCount);

    // Custom settings for ropes
    bulletBody->SetMass (0.1f);
    bulletBody->SetRigidity (0.99f);
    bulletBody->AnchorVertex (0, rigidBody);

    guideRopes.PutUnique(strandID, bulletBody);
  }

  // Animate the strand with the given ID
  void HairPhysicsControl::AnimateStrand (size_t strandID, csVector3* 
    coordinates, size_t coordinatesCount) const
  {
    csRef<CS::Physics::Bullet::iSoftBody> bulletBody = guideRopes.Get (strandID, 0);

    if(!bulletBody)
      return;

    CS_ASSERT(coordinatesCount == bulletBody->GetVertexCount());

    for ( size_t i = 0 ; i < coordinatesCount ; i ++ )
      coordinates[i] = bulletBody->GetVertexPosition(i);
  }

  void HairPhysicsControl::RemoveStrand (size_t strandID)
  {
    csRef<CS::Physics::Bullet::iSoftBody> bulletBody = guideRopes.Get (strandID, 0);
    if(!bulletBody)
      return;

    guideRopes.Delete(strandID, bulletBody);
    
    bulletDynamicSystem->RemoveSoftBody(bulletBody);
  }

  void HairPhysicsControl::RemoveAllStrands ()
  {
    // Iterate through all ropes
    for (size_t i = 0 ; i < guideRopes.GetSize(); i ++)
    {
      csRef<CS::Physics::Bullet::iSoftBody> bulletBody = guideRopes.Get(i, 0);
      if (bulletBody)
        bulletDynamicSystem->RemoveSoftBody( bulletBody );
    }
    guideRopes.DeleteAll();
  }

}
CS_PLUGIN_NAMESPACE_END(FurMesh)

