/*
    Copyright (C) 1998-2003 by Jorrit Tyberghein
    Written by Alex Pfaffe.

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
#ifndef __CS_COLLIDER_H__
#define __CS_COLLIDER_H__

#include "csextern.h"

#include "csutil/csobject.h"
#include "csutil/leakguard.h"
#include "csgeom/math3d.h"
#include "csgeom/matrix3.h"
#include "csgeom/vector3.h"

class csReversibleTransform;
struct csIntersectingTriangle;
struct iPolygonMesh;
struct iCollideSystem;
struct iCollider;
struct iObject;
struct iEngine;
struct iRegion;
struct iMeshWrapper;
struct iSector;

SCF_VERSION (csColliderWrapper, 0, 0, 3);

/**
 * This is a convenience object that you can use in your own
 * games to attach an iCollider object (from the CD plugin system)
 * to any other csObject (including CS entities). Use of this object
 * is optional (if you can assign your iCollider's to entities in
 * another manner then this is ok) and the engine will not use
 * this object itself.
 * <p>
 * Important! After creating an instance of this object you should
 * actually DecRef() the pointer because csColliderWrapper will automatically
 * attach itself to the given object. You can use
 * csColliderWrapper::GetCollider() later to get the collider again.
 */
class CS_CRYSTALSPACE_EXPORT csColliderWrapper : public csObject
{
private:
  csRef<iCollideSystem> collide_system;
  csRef<iCollider> collider;

public:
  CS_LEAKGUARD_DECLARE (csColliderWrapper);

  /// Create a collider based on a mesh.
  csColliderWrapper (csObject& parent, iCollideSystem* collide_system,
  	iPolygonMesh* mesh);

  /// Create a collider based on a mesh.
  csColliderWrapper (iObject* parent, iCollideSystem* collide_system,
  	iPolygonMesh* mesh);

  /**
   * Create a collider based on a collider. Note that it is legal to pass
   * in a 0 collider. In that case it indicates that this object has no
   * collider.
   */
  csColliderWrapper (iObject* parent, iCollideSystem* collide_system,
  	iCollider* collider);

  /// Destroy the plugin collider object
  virtual ~csColliderWrapper ();

  /// Get the collider interface for this object.
  iCollider* GetCollider () { return collider; }

  /// Get the collide system.
  iCollideSystem* GetCollideSystem () { return collide_system; }

  /**
   * Check if this collider collides with pOtherCollider.
   * Returns true if collision detected and adds the pair to the collisions
   * hists vector.
   * This collider and pOtherCollider must be of comparable subclasses, if
   * not false is returned.
   */
  bool Collide (csColliderWrapper& pOtherCollider,
                csReversibleTransform* pThisTransform = 0,
                csReversibleTransform* pOtherTransform = 0);
  /**
   * Similar to Collide for csColliderWrapper. Calls GetColliderWrapper for
   * otherCollider.
   */
  bool Collide (csObject& otherObject,
                csReversibleTransform* pThisTransform = 0,
                csReversibleTransform* pOtherTransform = 0);
  /**
   * Similar to Collide for csColliderWrapper. Calls GetColliderWrapper for
   * otherCollider.
   */
  bool Collide (iObject* otherObject,
                csReversibleTransform* pThisTransform = 0,
                csReversibleTransform* pOtherTransform = 0);

  /**
   * If object has a child of type csColliderWrapper it is returned.
   * Otherwise 0 is returned.
   */
  static csColliderWrapper* GetColliderWrapper (csObject& object);

  /**
   * If object has a child of type csColliderWrapper it is returned.
   * Otherwise 0 is returned.
   */
  static csColliderWrapper* GetColliderWrapper (iObject* object);

  SCF_DECLARE_IBASE_EXT (csObject);
};

/**
 * This is a class containing a few static member functions to help
 * work with csColliderWrapper and collision detection in general.
 */
class CS_CRYSTALSPACE_EXPORT csColliderHelper
{
public:
  /**
   * Initialize collision detection for a single object. This function will
   * first check if the parent factory has a collider. If so it will use
   * that for the object too. Otherwise it will create a new collider
   * and attaches it to the object. The new collider will also be attached
   * to the parent factory if it supports iObjectModel.
   * <p>
   * This function will also initialize colliders for the children of the
   * mesh.
   */
  static void InitializeCollisionWrapper (iCollideSystem* colsys,
  	iMeshWrapper* mesh);

  /**
   * Initialize collision detection (i.e. create csColliderWrapper) for
   * all objects in the engine. If the optional region is given only
   * the objects from that region will be initialized.
   */
  static void InitializeCollisionWrappers (iCollideSystem* colsys,
  	iEngine* engine, iRegion* region = 0);

  /**
   * Test collision between one collider and an array of colliders.
   * This function is mainly used by CollidePath() below.
   * \param colsys is the collider system.
   * \param collider is the collider of the object that we are going to move
   *        along the path.
   * \param trans is the transform of that object (see Collide()).
   * \param num_colliders is the number of colliders that we are going to use
   *        to collide with.
   * \param colliders is an array of colliders. Typically you can obtain such a
   *        list by doing iEngine->GetNearbyMeshes() and then getting the
   *        colliders from all meshes you get (possibly using
   *        csColliderWrapper). Note that it is safe to have 'collider' sitting
   *        in this list. This function will ignore that collider.
   * \param transforms is an array of transforms that belong with the array of
   *        colliders.
   */
  static bool CollideArray (
	iCollideSystem* colsys,
	iCollider* collider,
	const csReversibleTransform* trans,
  	int num_colliders,
	iCollider** colliders,
	csReversibleTransform **transforms);

  /**
   * Test if an object can move to a new position. The new position
   * vector will be modified to reflect the maximum new position that the
   * object could move to without colliding with something. This function
   * will return:
   * <ul>
   * <li>-1 if the object could not move at all (i.e. stuck at start position).
   * <li>0 if the object could not move fully to the desired position.
   * <li>1 if the object can move unhindered to the end position.
   * </ul>
   * <p>
   * This function will reset the collision pair array. If there was a
   * collision along the way the array will contain the information for
   * the first collision preventing movement.
   * <p>
   * The given transform should be the transform of the object corresponding
   * with the old position. 'colliders' and 'transforms' should be arrays
   * with 'num_colliders' elements for all the objects that we should
   * test against.
   * \param colsys is the collider system.
   * \param collider is the collider of the object that we are going
   * to move along the path.
   * \param trans is the transform of that object (see Collide()).
   * \param nbrsteps is the number of steps we want to check along the path.
   * Typically the stepsize resulting from this number of steps is set
   * to a step size smaller then the size of the collider bbox.
   * \param newpos is the new position of that object.
   * \param num_colliders is the number of colliders that we are going
   * to use to collide with.
   * \param colliders is an array of colliders. Typically you can obtain
   * such a list by doing iEngine->GetNearbyMeshes() and then getting
   * the colliders from all meshes you get (possibly using csColliderWrapper).
   * Note that it is safe to have 'collider' sitting in this list. This
   * function will ignore that collider.
   * \param transforms is an array of transforms that belong with the
   * array of colliders.
   */
  static int CollidePath (
  	iCollideSystem* colsys,
  	iCollider* collider, const csReversibleTransform* trans,
	float nbrsteps,
	csVector3& newpos,
	int num_colliders,
	iCollider** colliders,
	csReversibleTransform** transforms);


  /**
   * Trace a beam from 'start' to 'end' and return the first hit.
   * This function will use CollideRay() from the collision detection system
   * and is pretty fast. Note that only OPCODE plugin currently supports
   * this! A special note about portals! Portal traversal will NOT be used
   * on portals that have a collider! The idea there is that the portal itself
   * is a surface that cannot be penetrated.
   * \param cdsys is the collider system.
   * \param sector is the starting sector for the beam.
   * \param start is the start of the beam.
   * \param end is the end of the beam.
   * \param traverse_portals set to true in case you want the beam to
   * traverse portals.
   * \param closest_tri will be set to the closest triangle that is hit. The
   * triangle will be specified in world space.
   * \param closest_isect will be set to the closest intersection point (in
   * world space).
   * \param closest_mesh will be set to the closest mesh that is hit.
   * \return the squared distance between 'start' and the closest hit
   * or else a negative number if there was no hit.
   */
  static float TraceBeam (iCollideSystem* cdsys, iSector* sector,
	const csVector3& start, const csVector3& end,
	bool traverse_portals,
	csIntersectingTriangle& closest_tri,
	csVector3& closest_isect,
	iMeshWrapper** closest_mesh = 0);
};

#endif // __CS_COLLIDER_H__
