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

#ifndef __ITERRAIN_OBJECT_H__
#define __ITERRAIN_OBJECT_H__

#include "csutil/scf.h"
#include "isys/plugin.h"

class csColor;
class csTransform;
class csVector3;
struct iMaterialWrapper;
struct iRenderView;
struct iTerrainObject;
struct iTerrainObjectFactory;

SCF_VERSION (iTerrainObject, 0, 0, 2);

/**
 * This object represents a terrain mesh.
 */
struct iTerrainObject : public iBase
{
  /// Set a directional light.
  virtual void SetDirLight (const csVector3& dirl, const csColor& dirc) = 0;
  /// Get the directional light position.
  virtual csVector3 GetDirLightPosition () const = 0;
  /// Get the directional light color.
  virtual csColor GetDirLightColor () const = 0;
  /// Disable directional light.
  virtual void DisableDirLight () = 0;
  /// Return true if directional light is enabled.
  virtual bool IsDirLightEnabled () const = 0;

  /**
   * Draw this terrain given a view and transformation.
   */
  virtual void Draw (iRenderView *rview, bool use_z_buf = true) = 0;

  /// Set a material for this surface.
  virtual void SetMaterial (int i, iMaterialWrapper *material) = 0;
  /// Get the number of materials required/supported.
  virtual int GetMaterialCount () = 0;
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

#endif // __ITERRAIN_OBJECT_H__
