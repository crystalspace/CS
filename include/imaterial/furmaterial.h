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

#include <ivideo/material.h>
#include <ivaria/view.h>
#include <iengine/sector.h>

#include "crystalspace.h"

struct iFurMaterial;

class csVector3;
class csColor4;

struct iFurPhysicsControl : public virtual iBase
{
  SCF_INTERFACE (iFurPhysicsControl, 1, 0, 0);

  virtual void SetRigidBody (iRigidBody* rigidBody) = 0;
  virtual void SetBulletDynamicSystem (iBulletDynamicSystem* 
    bulletDynamicSystem) = 0;

  // Initialize the strand with the given ID
  virtual void InitializeStrand (size_t strandID, const csVector3* coordinates,
    size_t coordinatesCount) = 0;

  // Animate the strand with the given ID
  virtual void AnimateStrand (size_t strandID, csVector3* coordinates, size_t
    coordinatesCount) = 0;

  virtual void RemoveStrand (size_t strandID) = 0;
  virtual void RemoveAllStrands () = 0;
};

struct iFurStrandGenerator : public virtual iBase
{
  SCF_INTERFACE (iFurStrandGenerator, 1, 0, 0);

  virtual iMaterial* GetMaterial() = 0;
  virtual void SetMaterial(iMaterial* material) = 0;
  virtual void Invalidate() = 0;
  virtual void Update() = 0;
};

struct iFurMaterialType : public virtual iBase
{
  SCF_INTERFACE (iFurMaterialType, 1, 0, 0);

  virtual void ClearFurMaterials () = 0;
  virtual void RemoveFurMaterial (const char* name,iFurMaterial* furMaterial) = 0;
  virtual iFurMaterial* CreateFurMaterial (const char* name) = 0;
  virtual iFurMaterial* FindFurMaterial (const char* name) const = 0;
};

/**
* This is the API for our plugin. It is recommended
* that you use better comments than this one in a
* real situation.
*/
struct iFurMaterial : public virtual iMaterial 
{
  SCF_INTERFACE (iFurMaterial, 1, 0, 0);
  /// Generate geometry
  virtual void GenerateGeometry (iView* view, iSector* room) = 0;

  virtual void SetPhysicsControl (iFurPhysicsControl* physicsControl) = 0;

  virtual void SetFurStrandGenerator( iFurStrandGenerator* furStrandMaterial) = 0;
  virtual iFurStrandGenerator* GetFurStrandGenerator( ) = 0;

  virtual void SetMeshFactory ( iAnimatedMeshFactory* meshFactory ) = 0;
  virtual void SetMeshFactorySubMesh ( iAnimatedMeshFactorySubMesh* 
    meshFactorySubMesh ) = 0;
  virtual void SetMaterial ( iMaterial* material ) = 0;
};

#endif // __FUR_INTERF_H__
