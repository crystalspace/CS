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
    if(!animesh || !coordinatesCount)
      return;

    float closestDistance = 100000.0f;
    size_t closestVertex = (size_t) ~0;

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

    Directions* dir = new Directions;
    dir->count = coordinatesCount - 1;
    SphericalCoordinates* sc = new SphericalCoordinates[coordinatesCount];

    // Get spherical coordinates
    if (closestVertex != (size_t) ~0)
    {
      csRenderBufferHolder holder;
      animesh->GetRenderBufferAccessor ()->PreGetBuffer (&holder, CS_BUFFER_NORMAL);
      csRenderBufferLock<csVector3> normals (holder.GetRenderBuffer (CS_BUFFER_NORMAL));
      animesh->GetRenderBufferAccessor ()->PreGetBuffer (&holder, CS_BUFFER_TANGENT);
      csRenderBufferLock<csVector3> tangents (holder.GetRenderBuffer (CS_BUFFER_TANGENT));
      animesh->GetRenderBufferAccessor ()->PreGetBuffer (&holder, CS_BUFFER_BINORMAL);
      csRenderBufferLock<csVector3> binormals (holder.GetRenderBuffer (CS_BUFFER_BINORMAL));

      // Compute the new position of the anchor
      csRef<iMeshObject> mesh = scfQueryInterface<iMeshObject> (animesh);

      csVector3 normal = mesh->GetMeshWrapper ()->GetMovable ()->
        GetTransform ().This2Other (normals[closestVertex]);
      normal.Normalize();
      csVector3 tangent = mesh->GetMeshWrapper ()->GetMovable ()->
        GetTransform ().This2Other (tangents[closestVertex]);
      tangent.Normalize();
      csVector3 binormal = mesh->GetMeshWrapper ()->GetMovable ()->
        GetTransform ().This2Other (binormals[closestVertex]);
      binormal.Normalize();

      for (size_t i = 0 ; i < coordinatesCount - 1; i ++)
      {
        csVector3 direction = coordinates[i + 1] - coordinates[i];
        float length = direction.Norm();
        direction.Normalize();

        sc[i].radius = length;

        if ( fabs( normal * direction - 1 ) < EPSILON )
          sc[i].inclination = 0;
        else
          sc[i].inclination = acos( normal * direction );

        csVector3 projection = direction * sin ( sc[i].inclination );
        projection.Normalize();

        float sgn = acos( binormal * projection ) > PI / 2 ? -1 : 1;

        if ( fabs( tangent * projection - 1 ) < EPSILON )
          sc[i].azimuth = 0;
        else
          sc[i].azimuth = acos ( tangent * projection ) * sgn;

//       csPrintf("%f %f \n", sc->inclination, sc->azimuth);
//       csPrintf("%f %f %f %f %f\n", acos((float)(tangent * projection)), tangent * projection,
//         projection.x, projection.y, projection.z);
      }

      dir->sc = sc;
    }

    Anchor *anchor = new Anchor;
    anchor->animeshVertexIndex = closestVertex;
    anchor->direction = dir;

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

    CS_ASSERT(coordinatesCount - 1 == anchor->direction->count);

    // Create a walker for the position buffer of the animesh
    csRenderBufferHolder holder;
    animesh->GetRenderBufferAccessor ()->PreGetBuffer (&holder, CS_BUFFER_POSITION);
    csRenderBufferLock<csVector3> positions (holder.GetRenderBuffer (CS_BUFFER_POSITION));
    animesh->GetRenderBufferAccessor ()->PreGetBuffer (&holder, CS_BUFFER_NORMAL);
    csRenderBufferLock<csVector3> normals (holder.GetRenderBuffer (CS_BUFFER_NORMAL));
    animesh->GetRenderBufferAccessor ()->PreGetBuffer (&holder, CS_BUFFER_TANGENT);
    csRenderBufferLock<csVector3> tangents (holder.GetRenderBuffer (CS_BUFFER_TANGENT));
    animesh->GetRenderBufferAccessor ()->PreGetBuffer (&holder, CS_BUFFER_BINORMAL);
    csRenderBufferLock<csVector3> binormals (holder.GetRenderBuffer (CS_BUFFER_BINORMAL));

    // Compute the new position of the anchor
    csRef<iMeshObject> mesh = scfQueryInterface<iMeshObject> (animesh);

    csVector3 position = mesh->GetMeshWrapper ()->GetMovable ()->
      GetTransform ().This2Other (positions[anchor->animeshVertexIndex]);
    csVector3 normal = mesh->GetMeshWrapper ()->GetMovable ()->
      GetTransform ().This2Other (normals[anchor->animeshVertexIndex]);
    normal.Normalize();
    csVector3 tangent = mesh->GetMeshWrapper ()->GetMovable ()->
      GetTransform ().This2Other (tangents[anchor->animeshVertexIndex]);
    tangent.Normalize();
    csVector3 binormal = mesh->GetMeshWrapper ()->GetMovable ()->
      GetTransform ().This2Other (binormals[anchor->animeshVertexIndex]);
    binormal.Normalize();

    coordinates[0] = position;

    for (size_t i = 0 ; i < coordinatesCount - 1 ; i ++)
    {
      csVector3 direction = normal * cos (anchor->direction->sc[i].inclination) +
        binormal * sin (anchor->direction->sc[i].inclination) *
          sin (anchor->direction->sc[i].azimuth) + 
        tangent * sin (anchor->direction->sc[i].inclination) * 
          cos (anchor->direction->sc[i].azimuth);
      direction.Normalize();

      coordinates[i + 1] = coordinates[i] + 
        direction * anchor->direction->sc[i].radius;
    }
  }

  void AnimationPhysicsControl::RemoveStrand (size_t strandID)
  {
    Anchor* anchor = guideRopes.Get(strandID, 0);

    if (!anchor)
      return;

    guideRopes.Delete(strandID, anchor);
  
    Directions* dir = anchor->direction;

    delete dir->sc;
    
    delete dir;
    
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
        Directions* dir = anchor->direction;

        delete dir->sc;

        delete dir;

        delete anchor;
      }
    }

    guideRopes.DeleteAll();
  }

}
CS_PLUGIN_NAMESPACE_END(FurMesh)

