/*
    Crystal Space 3D engine
    Copyright (C) 2000 by Jorrit Tyberghein
  
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

#ifndef __ICOLLIDE_H__
#define __ICOLLIDE_H__

#include "csutil/scf.h"
#include "iplugin.h"

struct iPolygonMesh;
class csTransform;

SCF_VERSION (iCollider, 0, 1, 0);

/**
 * A collider.
 */
struct iCollider : public iBase
{
  /// Return a reference to the polygon mesh for this collider.
  virtual iPolygonMesh* GetGeometry () = 0;
};

SCF_VERSION (iCollideSystem, 0, 0, 1);

/**
 * This is the Collide plug-in. This plugin is a factory for creating
 * iCollider entities. A collider represents an entity in the
 * collision detection world. It uses the geometry data as given by
 * iPolygonMesh.
 */
class iCollideSystem : public iPlugIn
{
public:
  /// Create an iCollider for the given geometry.
  virtual iCollider* CreateCollider (iPolygonMesh* mesh) = 0;

  /**
   * Test collision between two colliders.
   * This is only supported for iCollider objects created by
   * this plugin.
   */
  virtual bool Collide (iCollider* collider1, csTransform* trans1,
  	iCollider* collider2, csTransform* trans2) = 0;
};

#endif // __ICOLLIDE_H__

