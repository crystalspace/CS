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

#include "csutil/scf.h"
#include "csgeom/vector3.h"
#include "csutil/array.h"

struct iPolygonMesh;
struct iMeshObject;
class csReversibleTransform;

/**
 * A structure used to return collision pairs.
 * These coordinates are in object space.
 */
struct csCollisionPair
{
  csVector3 a1, b1, c1;	// First triangle
  csVector3 a2, b2, c2;	// Second triangle
};

/**
 * An intersection triangle for CollideRay.
 */
struct csIntersectingTriangle
{
  csVector3 a, b, c;
};

SCF_VERSION (iCollider, 0, 2, 0);

/**
 * A mesh collider.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iCollideSystem::CreateCollider()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>csColliderWrapper::GetCollider()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>csColliderWrapper
 *   </ul>
 */
struct iCollider : public iBase
{
};

SCF_VERSION (iCollideSystem, 0, 0, 4);

/**
 * This is the Collide plug-in. This plugin is a factory for creating
 * iCollider entities. A collider represents an entity in the
 * collision detection world. It uses the geometry data as given by
 * iPolygonMesh.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>OPCODE plugin (crystalspace.collisiondetection.opcode)
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>CS_QUERY_REGISTRY()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>csColliderWrapper
 *   <li>csColliderHelper
 *   </ul>
 */
struct iCollideSystem : public iBase
{
  /**
   * Create a iCollider for the given mesh geometry.
   * \param mesh is a structure describing the geometry from which the
   * collider will be made. You can get such a mesh either by making your
   * own subclass of iPolygonMesh, by getting a mesh from
   * iMeshObject->GetObjectModel()->GetPolygonMeshColldet(), or else
   * by using csPolygonMesh, or csPolygonMeshBox.
   * \return a reference to a collider that you have to store.
   */
  virtual csPtr<iCollider> CreateCollider (iPolygonMesh* mesh) = 0;
  
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
   * Collide a collider with a world space ray.
   * \param collider is the collider to test with.
   * \param trans is the transform for the object represented by the
   * collider. If the collider belongs to a mesh object then you can get
   * the transform by calling mesh->GetMovable()->GetFullTransform().
   * \param start is the start of the ray.
   * \param end is the end of the ray.
   * \return true if there was a collision. The array with intersecting
   * triangles will be updated (see GetIntersectingTriangles()).
   */
  virtual bool CollideRay (
  	iCollider* collider, const csReversibleTransform* trans,
	const csVector3& start, const csVector3& end) = 0;

  /**
   * Collide a collider with a world space segment. This will not
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

