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

  CS_LEAKGUARD_IMPLEMENT(HairPhysicsControl);	

  HairPhysicsControl::HairPhysicsControl (iBase *parent)
    : scfImplementationType (this), rigidBody(0), bulletDynamicSystem(0), 
    animesh(0), maxRange(0)
  {
  }

  HairPhysicsControl::~HairPhysicsControl ()
  {
    RemoveAllStrands();
  }

  //-- iFurPhysicsControl

  void HairPhysicsControl::SetAnimatedMesh (CS::Mesh::iAnimatedMesh* animesh)
  {
    this->animesh = animesh;
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
    if (!rigidBody || !bulletDynamicSystem || !coordinatesCount)
      return;

    CS::Physics::Bullet::iSoftBody* bulletBody = bulletDynamicSystem->
      CreateRope(coordinates, coordinatesCount);

    // Custom settings for ropes
    bulletBody->SetMass (0.1f);
    bulletBody->SetRigidity (0.99f);
    bulletBody->AnchorVertex (0, rigidBody);

    float closestDistance = 100000.0f;
    size_t closestVertex = (size_t) ~0;

    if (animesh)
    {
      // Find the closest vertex of the animesh if asked for
      csRef<iMeshObject> mesh = scfQueryInterface<iMeshObject> (animesh);
      csReversibleTransform& animeshTransform =
        mesh->GetMeshWrapper ()->GetMovable ()->GetTransform ();

      // Create a walker for the position buffer of the animesh
      csRenderBufferHolder holder;
      animesh->GetRenderBufferAccessor ()->PreGetBuffer (&holder, CS_BUFFER_POSITION);
      iRenderBuffer* positions = holder.GetRenderBuffer (CS_BUFFER_POSITION);
      csVertexListWalker<float, csVector3> positionWalker (positions);

      // Iterate on all vertices
      for (size_t i = 0; i < positionWalker.GetSize (); i++)
      {
        float distance = (coordinates[0]
          - animeshTransform.This2Other ((*positionWalker))).Norm ();
        
        if (distance < closestDistance)
        {
          closestDistance = distance;
          closestVertex = i;
        }

        ++positionWalker;
      }
    }

    Anchor *anchor = new Anchor;
    anchor->softBody = bulletBody;
    anchor->animeshVertexIndex = closestVertex;

    guideRopes.PutUnique(strandID, anchor);

    if (maxRange < strandID)
      maxRange = strandID;
  }

  // Animate the strand with the given ID
  void HairPhysicsControl::AnimateStrand (size_t strandID, csVector3* 
    coordinates, size_t coordinatesCount) const
  {
    Anchor* anchor = guideRopes.Get (strandID, 0);

    if(!anchor)
      return;

    csRef<CS::Physics::Bullet::iSoftBody> bulletBody = anchor->softBody;

    CS_ASSERT(coordinatesCount == bulletBody->GetVertexCount());

    if (animesh && anchor->animeshVertexIndex != (size_t) ~0)
    {
      // Create a walker for the position buffer of the animesh
      csRenderBufferHolder holder;
      animesh->GetRenderBufferAccessor ()->PreGetBuffer (&holder, CS_BUFFER_POSITION);
      csRenderBufferLock<csVector3> positions (holder.GetRenderBuffer (CS_BUFFER_POSITION));

      // Compute the new position of the anchor
      csRef<iMeshObject> mesh = scfQueryInterface<iMeshObject> (animesh);
      csVector3 newPosition = mesh->GetMeshWrapper ()->GetMovable ()->
        GetTransform ().This2Other (positions[anchor->animeshVertexIndex]);

      bulletBody->UpdateAnchor (0, newPosition);
    }

    for ( size_t i = 0 ; i < coordinatesCount ; i ++ )
      coordinates[i] = bulletBody->GetVertexPosition(i);
  }

  void HairPhysicsControl::RemoveStrand (size_t strandID)
  {
    Anchor* anchor = guideRopes.Get (strandID, 0);

    if(!anchor)
      return;

    csRef<CS::Physics::Bullet::iSoftBody> bulletBody = anchor->softBody;

    guideRopes.Delete(strandID, anchor);
    
    bulletDynamicSystem->RemoveSoftBody(bulletBody);

    delete anchor;
  }

  void HairPhysicsControl::RemoveAllStrands ()
  {
    // Iterate through all ropes
    for (size_t i = 0 ; i < maxRange; i ++)
    {
      Anchor* anchor = guideRopes.Get (i, 0);
      
      if (anchor)
      {
        csRef<CS::Physics::Bullet::iSoftBody> bulletBody = anchor->softBody;
        bulletDynamicSystem->RemoveSoftBody( bulletBody );

        delete anchor;
      }
    }

    guideRopes.DeleteAll();
  }

}
CS_PLUGIN_NAMESPACE_END(FurMesh)

