/*
    Copyright (C) 2001 by Jorrit Tyberghein
    Plug-In Written by Richard D Shank

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

#ifndef __ITERROBJ_H__
#define __ITERROBJ_H__

#include "csutil/scf.h"
#include "iplugin.h"

class csColor;
class csTransform;
class csVector3;
class csTerrainWrapper;
struct iLight;
struct iMaterialWrapper;
struct iRenderView;
struct iTerrainObject;
struct iTerrainObjectFactory;
struct iTerrainWrapper;

SCF_VERSION (iTerrainObject, 0, 0, 1);

struct iTerrainObject : public iBase
{
  /// Set a directional light.
  virtual void SetDirLight ( csVector3& dirl, csColor& dirc) = 0;
  /// Disable directional light.
  virtual void DisableDirLight () = 0;

  /**
   * Draw this terrain given a view and transformation.
   */
  virtual void Draw (iRenderView *rview, bool use_z_buf = true) = 0;

  /// Set a material for this surface.
  virtual void SetMaterial (int i, iMaterialWrapper *material) = 0;
  /// Get the number of materials required/supported.
  virtual int GetNumMaterials () = 0;
  /// Set the amount of triangles
  virtual void SetLOD (unsigned int detail) = 0;

  /**
   * If current transformation puts us below the terrain at the given
   * x,z location then we have hit the terrain.  We adjust position to
   * be at level of terrain and return 1.  If we are above the terrain we
   * return 0.
   */
  virtual int CollisionDetect (csTransform *p) = 0;

};

SCF_VERSION (iTerrainObjectFactory, 0, 0, 1);

/**
 * This object is a factory which can generate a terrain.
 */
struct iTerrainObjectFactory : public iBase
{
  /// Create an instance of iMeshObject.
  virtual iTerrainObject* NewInstance () = 0;
};

SCF_VERSION (iTerrainObjectType, 0, 0, 1);

/**
 * This plugin describes a specific type of terrain. 
 */
struct iTerrainObjectType : public iPlugIn
{
  /// Create an instance of iMeshObjectFactory.
  virtual iTerrainObjectFactory* NewFactory () = 0;
};

SCF_VERSION (iTerrainWrapper, 0, 0, 1);

/**
 * This interface corresponds to the object in the engine
 * that holds reference to the real iTerrainObject.
 */
struct iTerrainWrapper : public iBase
{
  /// Ugly.
  virtual csTerrainWrapper* GetPrivateObject () = 0;

  /// Get the iTerrainObject.
  virtual iTerrainObject* GetTerrainObject () = 0;

  /**
   * Light object according to the given array of lights (i.e.
   * fill the vertex color array).
   * No shadow calculation will be done. This is assumed to have
   * been done earlier. This is a primitive lighting process
   * based on the lights which hit one point of the sprite (usually
   * the center). More elaborate lighting systems are possible
   * but this will do for now.
   */
  virtual void UpdateLighting (iLight** lights, int num_lights) = 0;

};

SCF_VERSION (iTerrainFactoryWrapper, 0, 0, 1);

/**
 * This interface corresponds to the object in the engine
 * that holds reference to the real iTerrainFactory.
 */
struct iTerrainFactoryWrapper : public iBase
{
  /// Get the iTerrainFactory.
  virtual iTerrainObjectFactory* GetTerrainObjectFactory () = 0;
};

#endif // __ITERROBJ_H__

