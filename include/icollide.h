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
#include "csgeom/vector3.h"
#include "iplugin.h"

struct iPolygonMesh;
class csTransform;

SCF_VERSION (iCollider, 0, 1, 0);

/**
 * A structure used to return collision pairs.
 */
struct csCollisionPair
{
  csVector3 a1, b1, c1;	// First triangle
  csVector3 a2, b2, c2;	// Second triangle
};

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
struct iCollideSystem : public iPlugIn
{
  /// Create an iCollider for the given geometry.
  virtual iCollider* CreateCollider (iPolygonMesh* mesh) = 0;

  /**
   * Test collision between two colliders.
   * This is only supported for iCollider objects created by
   * this plugin. Returns null if no collision or else a
   * pointer to an array of csCollisionPair's. In the latter
   * case 'num_pairs' will be set to the number of pairs in that
   * array. Note that this array will only be valid upto the next
   * time that Collide is called.
   */
  virtual csCollisionPair* Collide (iCollider* collider1, csTransform* trans1,
  	iCollider* collider2, csTransform* trans2,
	int& num_pairs) = 0;

  /**
   * Indicate if we are interested only in the first hit that is found.
   * This is only valid for CD algorithms that actually allow the
   * detection of multiple CD hit points.
   */
  virtual void SetOneHitOnly (bool o) = 0;

  /**
   * Return true if this CD system will only return the first hit
   * that is found. For CD systems that support multiple hits this
   * will return the value set by the SetOneHitOnly() function.
   * For CD systems that support one hit only this will always return true.
   */
  virtual bool GetOneHitOnly () = 0;
};

#endif // __ICOLLIDE_H__

