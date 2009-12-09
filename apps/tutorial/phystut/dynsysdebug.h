/*
  Copyright (C) 2009 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __DYNSYSDEBUG_H__
#define __DYNSYSDEBUG_H__

#include <iengine/sector.h>
#include "csutil/ref.h"

#include <cstool/proctex.h>
#include <csutil/cscolor.h>

struct iObjectRegistry;
struct iMaterialWrapper;
struct iDynamicSystem;
struct iRigidBody;

//------------------------ csDynamicSystemDebugger ----------------------

// TODO: move this in a cstool file?
/**
 * This class let visualize the colliders of a dynamic simulation.
 */
class csDynamicSystemDebugger
{
 public:
  csDynamicSystemDebugger (iObjectRegistry* object_reg);

  void SetDynamicSystem (iDynamicSystem* system);
  void SetDebugSector (iSector* sector);
  void SetDebugDisplayMode (bool debugMode);

 private:

  struct MeshData
  {
    csRef<iMeshWrapper> mesh;
    //csOrthoTransform transform;
  };

  iObjectRegistry* object_reg;
  csRef<iDynamicSystem> system;
  csRef<iSector> sector;
  csRef<iMaterialWrapper> material;
  bool debugMode;
  csHash<MeshData, csPtrKey<iRigidBody> > storedMeshes;
};

//------------------------ ColoredTexture ----------------------

// TODO: move this in plugins/proctex/standard?
/**
 * A procedural texture to create a material from a color.
 */
class ColoredTexture : public csProcTexture
{
public:
  ColoredTexture (csColor color);
  virtual ~ColoredTexture () { }
  virtual bool PrepareAnim ();
  virtual void Animate (csTicks current_time);

  static iMaterialWrapper* CreateColoredMaterial(const char* materialName,
				 csColor color, iObjectRegistry* object_reg);

 private:
  csColor color;
};

//------------------------ DebugShape ----------------------

/**
 * Some utility methods to create primitive meshes.
 */
class DebugShape
{
 public:
  static csRef<iMeshWrapper> CreateBoxMesh (csBox3 box,
				iMaterialWrapper* material, csOrthoTransform transform,
				iSector* sector, iObjectRegistry* object_reg);

  static csRef<iMeshWrapper> CreateSphereMesh (csSphere sphere, iMaterialWrapper* material,
				iSector* sector, iObjectRegistry* object_reg);

  static csRef<iMeshWrapper> CreateCylinderMesh (float length, float radius,
				iMaterialWrapper* material, csOrthoTransform transform,
				iSector* sector, iObjectRegistry* object_reg);

  static csRef<iMeshWrapper> CreateCapsuleMesh (float length, float radius,
				iMaterialWrapper* material, csOrthoTransform transform,
				iSector* sector, iObjectRegistry* object_reg);

};

#endif // __DYNSYSDEBUG_H__
