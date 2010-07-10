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
#include "hairphysicscontrol.h"

CS_PLUGIN_NAMESPACE_BEGIN(FurMaterial)
{
  /********************
  *  HairPhysicsControl
  ********************/
  SCF_IMPLEMENT_FACTORY (HairPhysicsControl)

    CS_LEAKGUARD_IMPLEMENT(HairPhysicsControl);	

  HairPhysicsControl::HairPhysicsControl (iBase* parent)
    : scfImplementationType (this, parent), object_reg(0)
  {
  }

  HairPhysicsControl::~HairPhysicsControl ()
  {
    guideRopes.DeleteAll();
  }

  // From iComponent
  bool HairPhysicsControl::Initialize (iObjectRegistry* r)
  {
    object_reg = r;
    return true;
  }

  //-- iFurPhysicsControl

  void HairPhysicsControl::SetRigidBody (iRigidBody* rigidBody)
  {
    this->rigidBody = rigidBody;
  }

  void HairPhysicsControl::SetBulletDynamicSystem (iBulletDynamicSystem* 
    bulletDynamicSystem)
  {
    this->bulletDynamicSystem = bulletDynamicSystem;
  }

  // Initialize the strand with the given ID
  void HairPhysicsControl::InitializeStrand (size_t strandID, const csVector3* 
    coordinates, size_t coordinatesCount)
  {
    csVector3 first = coordinates[0];
    csVector3 last = coordinates[coordinatesCount - 1];

    iBulletSoftBody* bulletBody = bulletDynamicSystem->
      CreateRope(first, first + csVector3(0, 1, 0) * (last - first).Norm() , 
      coordinatesCount - 1);	//	replace with -1
    bulletBody->SetMass (0.1f);
    bulletBody->SetRigidity (0.99f);
    bulletBody->AnchorVertex (0, rigidBody);

    guideRopes.PutUnique(strandID, bulletBody);
  }

  // Animate the strand with the given ID
  void HairPhysicsControl::AnimateStrand (size_t strandID, csVector3* 
    coordinates, size_t coordinatesCount)
  {
    csRef<iBulletSoftBody> bulletBody = guideRopes.Get (strandID, 0);

    if(!bulletBody)
      return;

    CS_ASSERT(coordinatesCount == bulletBody->GetVertexCount());
    //printf("%d\t%d\n", coordinatesCount, bulletBody->GetVertexCount());

    for ( size_t i = 0 ; i < coordinatesCount ; i ++ )
      coordinates[i] = bulletBody->GetVertexPosition(i);
    /*
    if (strandID % 3 == 0)
    for ( size_t i = 0 ; i < coordinatesCount ; i ++ )
    coordinates[i] = bulletBody->GetVertexPosition(0)+ i*0.05 *
    csVector3(1,0,0);
    else if (strandID % 3 == 1)
    for ( size_t i = 0 ; i < coordinatesCount ; i ++ )
    coordinates[i] = bulletBody->GetVertexPosition(0)+ i*0.05 *
    csVector3(0,1,0);
    else
    for ( size_t i = 0 ; i < coordinatesCount ; i ++ )
    coordinates[i] = bulletBody->GetVertexPosition(0)+ i*0.05 *
    csVector3(0,0,1);
    */
  }

  void HairPhysicsControl::RemoveStrand (size_t strandID)
  {
    csRef<iBulletSoftBody> bulletBody = guideRopes.Get (strandID, 0);
    if(!bulletBody)
      return;

    guideRopes.Delete(strandID, bulletBody);
    
    bulletDynamicSystem->RemoveSoftBody(bulletBody);
  }

  void HairPhysicsControl::RemoveAllStrands ()
  {
    guideRopes.DeleteAll();
  }

}
CS_PLUGIN_NAMESPACE_END(FurMaterial)

