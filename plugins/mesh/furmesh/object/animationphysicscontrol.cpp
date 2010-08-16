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
#include "animationphysicscontrol.h"

CS_PLUGIN_NAMESPACE_BEGIN(FurMesh)
{
  /*************************
  *  AnimationPhysicsControl
  **************************/

  SCF_IMPLEMENT_FACTORY (AnimationPhysicsControl)

  CS_LEAKGUARD_IMPLEMENT(AnimationPhysicsControl);	

  AnimationPhysicsControl::AnimationPhysicsControl (iBase* parent)
    : scfImplementationType (this, parent), object_reg(0), animesh(0)
  {
  }

  AnimationPhysicsControl::~AnimationPhysicsControl ()
  {
    RemoveAllStrands();
  }

  bool AnimationPhysicsControl::Initialize (iObjectRegistry* r)
  {
    // Default initial transformation
    object_reg = r;
    return true;
  }

  void AnimationPhysicsControl::SetAnimesh (CS::Mesh::iAnimatedMesh* animesh)
  {
    this->animesh = animesh;
  }

  // Initialize the strand with the given ID
  void AnimationPhysicsControl::InitializeStrand (size_t strandID, 
    csVector3* coordinates, size_t coordinatesCount)
  {
    csFurData* guideHairAnimation = new csFurData;

    guideHairAnimation->controlPointsCount = coordinatesCount;
    guideHairAnimation->controlPoints = new csVector3[coordinatesCount];

    for (size_t i = 0 ; i < coordinatesCount ; i ++)
      guideHairAnimation->controlPoints[i] = coordinates[i];

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
    anchor->furData = guideHairAnimation;
    anchor->animeshVertexIndex = closestVertex;

    guideRopes.PutUnique(strandID, anchor);
  }

  // Animate the strand with the given ID
  void AnimationPhysicsControl::AnimateStrand (size_t strandID, 
    csVector3* coordinates, size_t coordinatesCount) const
  {
    if (!animesh)
      return;

    Anchor* anchor = guideRopes.Get(strandID, 0);

    if (!anchor || anchor->animeshVertexIndex == (size_t) ~0)
      return;

    // Create a walker for the position buffer of the animesh
    csRenderBufferHolder holder;
    animesh->GetRenderBufferAccessor ()->PreGetBuffer (&holder, CS_BUFFER_POSITION);
    csRenderBufferLock<csVector3> positions (holder.GetRenderBuffer (CS_BUFFER_POSITION));

    // Compute the new position of the anchor
    csRef<iMeshObject> mesh = scfQueryInterface<iMeshObject> (animesh);
    csVector3 newPosition = mesh->GetMeshWrapper ()->GetMovable ()->
      GetTransform ().This2Other (positions[anchor->animeshVertexIndex]);

    csFurData *guideHairAnimation = anchor->furData;

    if (!guideHairAnimation || !guideHairAnimation->controlPointsCount)
      return;

    csVector3 direction = newPosition - guideHairAnimation->controlPoints[0];
    float distance = csVector3::Norm(direction);
    direction.Normalize();

    for ( size_t i = 0 ; i < coordinatesCount ; i ++ )
      coordinates[i] = guideHairAnimation->controlPoints[i] + 
        distance * direction;
  }

  void AnimationPhysicsControl::RemoveStrand (size_t strandID)
  {
    Anchor* anchor = guideRopes.Get(strandID, 0);

    if (!anchor)
      return;

    csFurData* guideHairAnimation = anchor->furData;

    guideRopes.Delete(strandID, anchor);

    if (guideHairAnimation->controlPoints && 
        guideHairAnimation->controlPointsCount)
      delete guideHairAnimation->controlPoints;

    delete guideHairAnimation;

    delete anchor;
  }

  void AnimationPhysicsControl::RemoveAllStrands ()
  {
    // Iterate through all ropes
    for (size_t i = 0 ; i < guideRopes.GetSize(); i ++)
    {
      Anchor* anchor = guideRopes.Get(i, 0);

      if (anchor)
      {
        csFurData* guideHairAnimation = anchor->furData;

        if (guideHairAnimation->controlPoints && 
            guideHairAnimation->controlPointsCount)
          delete guideHairAnimation->controlPoints;
      
        delete guideHairAnimation;

        delete anchor;
      }
    }

    guideRopes.DeleteAll();
  }

}
CS_PLUGIN_NAMESPACE_END(FurMesh)

