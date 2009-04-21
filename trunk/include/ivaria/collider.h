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

#ifndef __CS_IVARIA_COLLIDER_H__
#define __CS_IVARIA_COLLIDER_H__

/**\file
 * Mesh collider interfaces
 */

#include "csutil/scf_interface.h"
#include "csgeom/vector3.h"
#include "csutil/array.h"
#include "csutil/ref.h"
#include "iutil/strset.h"

struct iTriangleMesh;
struct iTerraFormer;
struct iMeshObject;
class csReversibleTransform;
struct iTerrainSystem;

/**
 * A structure used to return collision pairs.
 * These coordinates are in object space.
 */
struct csCollisionPair
{
  //@{
  /// First triangle
  csVector3 a1, b1, c1;	
  //@}
  //@{
  // Second triangle
  csVector3 a2, b2, c2;	
  //@}

  /// A comparison operator in order for it to fit into iArray
  bool operator==(const csCollisionPair& p) const
  {
    return (a1 == p.a1 && b1 == p.b1 && c1 == p.c1 &&
            a2 == p.a2 && b2 == p.b2 && c2 == p.c2);
  }
};

/**
 * An intersection triangle for CollideRay.
 */
struct csIntersectingTriangle
{
  csVector3 a, b, c;
};



enum csColliderType
{
  CS_MESH_COLLIDER = 0,
  CS_TERRAFORMER_COLLIDER,
  CS_TERRAIN_COLLIDER
};

/**
 * A mesh collider.
 *
 * Main creators of instances implementing this interface:
 * - iCollideSystem::CreateCollider()
 * 
 * Main ways to get pointers to this interface:
 * - csColliderWrapper::GetCollider()
 * 
 * Main users of this interface:
 * - csColliderWrapper
 */
struct iCollider : public virtual iBase
{
  SCF_INTERFACE (iCollider, 0, 2, 0);
  virtual csColliderType GetColliderType () = 0;
};

/**
 * This is the Collide plug-in. This plugin is a factory for creating
 * iCollider entities. A collider represents an entity in the
 * collision detection world. It uses the geometry data as given by
 * iTriangleMesh.
 *
 * Main creators of instances implementing this interface:
 * - OPCODE plugin (crystalspace.collisiondetection.opcode)
 * 
 * Main ways to get pointers to this interface:
 * - csQueryRegistry()
 * 
 * Main users of this interface:
 * - csColliderWrapper
 * - csColliderHelper
 */
struct iCollideSystem : public virtual iBase
{
  SCF_INTERFACE (iCollideSystem, 2, 2, 1);

  /**
   * Get the ID that the collision detection system prefers for getting
   * triangle data from iObjectModel. This corresponds with the ID you
   * would get from doing strings->Request ("colldet") where 'strings'
   * is a reference to the standard string set.
   */
  virtual csStringID GetTriangleDataID () = 0;

  /**
   * Get the ID that for the base triangle mesh model from
   * iObjectModel. This corresponds with the ID you
   * would get from doing strings->Request ("base") where 'strings'
   * is a reference to the standard string set.
   */
  virtual csStringID GetBaseDataID () = 0;

  /**
   * Create a iCollider for the given mesh geometry.
   * \param mesh is a structure describing the geometry from which the
   * collider will be made. You can get such a mesh either by making your
   * own subclass of iTriangleMesh, by getting a mesh from
   * iMeshObject->GetObjectModel()->GetTriangleData(), or else
   * by using csTriangleMesh, or csTriangleMeshBox. Note that the
   * collision detection system usually uses triangle meshes with
   * the id equal to 'colldet'.
   * \return a reference to a collider that you have to store.
   */
  virtual csPtr<iCollider> CreateCollider (iTriangleMesh* mesh) = 0;

  /**
   * Create a Collider from a terrain. This should be used instead
   * of the iTriangleMesh version in case you have a landscape because
   * this is a more optimal way to do.
   */
  virtual csPtr<iCollider> CreateCollider (iTerraFormer* mesh) = 0;
  
  /**
   * Create a Collider from a terrain.
   */
  virtual csPtr<iCollider> CreateCollider (iTerrainSystem* mesh) = 0;

  /**
   * Test collision between two colliders.
   * This is only supported for iCollider objects created by
   * this plugin. Returns false if no collision or else true.
   * The collisions will be added to the collision pair array
   * that you can query with GetCollisionPairs and reset/clear
   * with ResetCollisionPairs (very important! Do not forget this).
   * Every call to Collide will add to that array.
   *
   * \param collider1 is the first collider as created by this same
   * collide system (never pass in a collider created by another
   * collide system).
   * \param trans1 is the transform for the object represented by
   * the first collider. If the collider belongs to a mesh object
   * then you can get the transform by calling
   * mesh->GetMovable ()->GetFullTransform().
   * \param collider2 is the second collider.
   * \param trans2 is the second transform.
   * \return true if there are triangles that intersect. The
   * array with collision pairs will be updated.
   */
  virtual bool Collide (
  	iCollider* collider1, const csReversibleTransform* trans1,
  	iCollider* collider2, const csReversibleTransform* trans2) = 0;

  /**
   * Get pointer to current array of collision pairs.
   * This array will grow with every call to Collide until you clear
   * it using 'ResetCollisionPairs'. Note that the triangles are
   * in object space and not world space!
   * \return an array of collision pairs for all Collide() calls
   * that occured between now and the call to ResetCollisionPairs().
   */
  virtual csCollisionPair* GetCollisionPairs () = 0;

  /**
   * Get number of collision pairs in array.
   * \return the number of collision pairs.
   */
  virtual size_t GetCollisionPairCount () = 0;

  /**
   * Reset the array with collision pairs. It is very important to call
   * this before collision detection. Otherwise the internal table of
   * collision pairs will grow forever.
   */
  virtual void ResetCollisionPairs () = 0;

  /**
   * Collide a collider with a (infinite) world space ray.
   * \param collider is the collider to test with.
   * \param trans is the transform for the object represented by the
   * collider. If the collider belongs to a mesh object then you can get
   * the transform by calling mesh->GetMovable()->GetFullTransform().
   * \param start is the start of the ray.
   * \param pointOnRay A point on the ray other than \a start, used to
   *   compute the ray's direction.
   * \return true if there was a collision. The array with intersecting
   * triangles will be updated (see GetIntersectingTriangles()).
   */
  virtual bool CollideRay (
  	iCollider* collider, const csReversibleTransform* trans,
	const csVector3& start, const csVector3& pointOnRay) = 0;

  /**
   * Collide a collider with a (finite) world space segment. This will not
   * return collisions with triangles behind the end of the segment.
   * \param collider is the collider to test with.
   * \param trans is the transform for the object represented by the
   * collider. If the collider belongs to a mesh object then you can get
   * the transform by calling mesh->GetMovable()->GetFullTransform().
   * \param start is the start of the ray.
   * \param end is the end of the ray.
   * \return true if there was a collision. The array with intersecting
   * triangles will be updated (see GetIntersectingTriangles()).
   */
  virtual bool CollideSegment (
  	iCollider* collider, const csReversibleTransform* trans,
	const csVector3& start, const csVector3& end) = 0;

  /**
   * Get the array of intersection points as returned by CollideRay().
   * Note that the coordinates in the array of triangles is in object
   * space of the collider object and not world space!
   */
  virtual const csArray<csIntersectingTriangle>& GetIntersectingTriangles ()
  	const = 0;

  /**
   * Indicate if we are interested only in the first hit that is found.
   * This is only valid for CD algorithms that actually allow the
   * detection of multiple CD hit points.
   * \param o is true if you are only interested in one colliding
   * triangle per call to Collide. By default this is 'false' unless
   * the CD system only supports single hits.
   */
  virtual void SetOneHitOnly (bool o) = 0;

  /**
   * Return true if this CD system will only return the first hit
   * that is found. For CD systems that support multiple hits this
   * will return the value set by the SetOneHitOnly() function.
   * For CD systems that support one hit only this will always return true.
   * \return true if there is only one hit recorder for every
   * call to Collide().
   */
  virtual bool GetOneHitOnly () = 0;
};

#endif // __CS_IVARIA_COLLIDER_H__

