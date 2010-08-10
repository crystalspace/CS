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

#ifndef __FUR_INTERF_H__
#define __FUR_INTERF_H__

#include <csutil/scf.h>

#include "crystalspace.h"

namespace CS
{
  namespace Mesh
  {

    struct iFurPhysicsControl;
    struct iFurStrandGenerator;
    struct iFurMeshFactory;
    struct iFurMeshType;
    struct iFurMesh;

struct iFurPhysicsControl : public virtual iBase
{
  SCF_INTERFACE (CS::Mesh::iFurPhysicsControl, 1, 0, 0);

  virtual void SetInitialTransform(csReversibleTransform initialTransform) = 0;
  virtual void SetRigidBody (iRigidBody* rigidBody) = 0;
  virtual void SetBulletDynamicSystem (CS::Physics::Bullet::iDynamicSystem* 
    bulletDynamicSystem) = 0;

  // Initialize the strand with the given ID
  virtual void InitializeStrand (size_t strandID, csVector3* coordinates,
    size_t coordinatesCount) = 0;

  // Animate the strand with the given ID
  virtual void AnimateStrand (size_t strandID, csVector3* coordinates, size_t
    coordinatesCount) const = 0;

  virtual void RemoveStrand (size_t strandID) = 0;
  virtual void RemoveAllStrands () = 0;
};

struct iFurStrandGenerator : public virtual iBase
{
  SCF_INTERFACE (CS::Mesh::iFurStrandGenerator, 1, 0, 0);

  virtual iMaterial* GetMaterial() const = 0;
  virtual void SetMaterial(iMaterial* material) = 0;
  virtual void Invalidate() = 0;
  virtual void Update() = 0;
};

struct iFurMeshFactory : public virtual iBase
{
  SCF_INTERFACE (CS::Mesh::iFurMeshFactory, 1, 0, 0);

  /// geometry access
  virtual void SetVertexCount (uint n) = 0;
  virtual void SetTriangleCount (uint n) = 0;

  virtual uint GetVertexCount() const = 0;
  virtual uint GetIndexCount() const = 0;
  
  virtual iRenderBuffer* GetIndices () const = 0;
  virtual bool SetIndices (iRenderBuffer* renderBuffer) = 0;
  virtual iRenderBuffer* GetVertices () const = 0;
  virtual bool SetVertices (iRenderBuffer* renderBuffer) = 0;
  virtual iRenderBuffer* GetTexCoords () const = 0;
  virtual bool SetTexCoords (iRenderBuffer* renderBuffer) = 0;
  virtual iRenderBuffer* GetNormals () const = 0;
  virtual bool SetNormals (iRenderBuffer* renderBuffer) = 0;
  virtual iRenderBuffer* GetTangents () const = 0;
  virtual bool SetTangents (iRenderBuffer* renderBuffer) = 0;
  virtual iRenderBuffer* GetBinormals () const = 0;
  virtual bool SetBinormals (iRenderBuffer* renderBuffer) = 0;
};

struct iFurMeshType : public virtual iMeshObjectType
{
  SCF_INTERFACE (CS::Mesh::iFurMeshType, 1, 0, 0);
};

/**
* This is the API for our plugin. It is recommended
* that you use better comments than this one in a
* real situation.
*/
struct iFurMesh : public virtual iBase  
{
  SCF_INTERFACE (CS::Mesh::iFurMesh, 1, 0, 0);

  /// Generate geometry
  virtual void GenerateGeometry (iView* view, iSector* room) = 0;
  virtual void SetGuideLOD(float guideLOD) = 0; 
  virtual void SetStrandLOD(float strandLOD) = 0; 
  virtual void SetLOD(float lod) = 0;

  virtual void SetPhysicsControl (iFurPhysicsControl* physicsControl) = 0;
  virtual void StartPhysicsControl ( ) = 0;
  virtual void StopPhysicsControl ( ) = 0;
 
  virtual void SetFurStrandGenerator( iFurStrandGenerator* furStrandGenerator) = 0;
  virtual iFurStrandGenerator* GetFurStrandGenerator( ) = 0;

  virtual void SetMeshFactory ( CS::Mesh::iAnimatedMeshFactory* meshFactory ) = 0;
  virtual void SetMeshFactorySubMesh ( CS::Mesh::iAnimatedMeshSubMeshFactory* 
    meshFactorySubMesh ) = 0;
  virtual void SetBaseMaterial ( iMaterial* baseMaterial ) = 0;
};

  } // namespace Mesh
} // namespace CS

#endif // __FUR_INTERF_H__
