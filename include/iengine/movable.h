/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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

#ifndef __CS_IENGINE_MOVABLE_H__
#define __CS_IENGINE_MOVABLE_H__

/**\file
 */
/**
 * \addtogroup engine3d
 * @{ */
 
#include "csutil/scf.h"
#include "csutil/csvector.h"
#include "csgeom/transfrm.h"

class csVector3;
class csMatrix3;
struct iSector;
struct iMovable;
struct iSectorList;

SCF_VERSION (iMovableListener, 0, 0, 1);

/**
 * Implement this class if you're interested in hearing about
 * movable changes.
 */
struct iMovableListener : public iBase
{
  /// The movable has changed.
  virtual void MovableChanged (iMovable* movable) = 0;
  /// The movable is about to be destroyed.
  virtual void MovableDestroyed (iMovable* movable) = 0;
};

SCF_VERSION (iMovable, 0, 1, 2);

/**
 * This interface describes a movable entity. It is usually
 * attached to some other geometrical object like a thing or
 * mesh object.
 */
struct iMovable : public iBase
{
  /// Get the parent movable.
  virtual iMovable* GetParent () const = 0;
  /// Set the parent movable.
  virtual void SetParent (iMovable* parent) = 0;

  /**
   * Initialize the list of sectors to one sector where
   * this thing is. This is a conveniance funcion.
   * This function does not update the corresponding list in
   * the sector. It only does a local update here.
   * This function does not do anything if the parent is not NULL.
   * You have to call UpdateMove() after changing the sector
   * information.
   */
  virtual void SetSector (iSector* sector) = 0;

  /**
   * Clear the list of sectors.
   * This function does not do anything if the parent is not NULL.
   * You have to call UpdateMove() after changing the sector
   * information.
   */
  virtual void ClearSectors () = 0;

  /**
   * Get list of sectors for this entity.
   * This will return the sectors of the parent if there
   * is a parent.
   */
  virtual iSectorList* GetSectors () = 0;

  /**
   * Return true if we are placed in a sector.
   */
  virtual bool InSector () const = 0;

  /**
   * Set the transformation vector and sector to move to
   * some position.
   * You have to call UpdateMove() after changing the position
   * of an object.
   */
  virtual void SetPosition (iSector* home, const csVector3& v) = 0;

  /**
   * Set the transformation vector for this object. Note
   * that the sectors are unchanged.
   * You have to call UpdateMove() after changing the position
   * of an object.
   */
  virtual void SetPosition (const csVector3& v) = 0;

  /**
   * Get the current position.
   */
  virtual const csVector3& GetPosition () const = 0;

  /**
   * Get the current position but keep track of hierarchical
   * transformations.
   */
  virtual const csVector3 GetFullPosition () const = 0;

  /**
   * Set the transformation matrix for this entity.
   * You have to call UpdateMove() after changing the transform
   * of an object.
   */
  virtual void SetTransform (const csMatrix3& matrix) = 0;

  /**
   * Set the world to object tranformation. Note that it is
   * recommended not to scale objects using SetTransform()
   * as some parts of CS don't work properly in that case.
   * It is better to scale your object using
   * iMeshWrapper or iMeshFactoryWrapper->HardTransform().
   * You have to call UpdateMove() after changing the transform
   * of an object.
   */
  virtual void SetTransform (const csReversibleTransform& t) = 0;

  /**
   * Get the world to object tranformation. This==object
   * and Other==world so This2Other()
   * transforms from object to world space. If you modify this
   * transform you have to call UpdateMove() later.
   */
  virtual csReversibleTransform& GetTransform () = 0;

  /**
   * Construct the full world to object transformation given
   * this transformation and possible parents transformations.
   */
  virtual csReversibleTransform GetFullTransform () const = 0;

  /**
   * Relative move.
   */
  virtual void MovePosition (const csVector3& v) = 0;

  /**
   * Relative transform.
   */
  virtual void Transform (const csMatrix3& matrix) = 0;

  /**
   * Add a listener to this movable. This listener will be called whenever
   * the movable changes or right before the movable is destroyed.
   */
  virtual void AddListener (iMovableListener* listener) = 0;

  /**
   * Remove a listener from this movable.
   */
  virtual void RemoveListener (iMovableListener* listener) = 0;

  /**
   * After all movement has been done you need to
   * call UpdateMove() to make the final changes to the entity
   * that is controlled by this movable. This is very important!
   * This function is responsible for calling all movement listeners.
   * If you do not call this function then the visibility cullers
   * will not work right (among other things).
   * <p>
   * UpdateMove() will also check for the transform being the identity
   * transform and in that case it will set the identity flag to true.
   */
  virtual void UpdateMove () = 0;

  /**
   * A number which indicates if the movable has been updated.
   * One can use this number to see if the position of the object
   * has changed since the last time it was checked.
   */
  virtual long GetUpdateNumber () const = 0;

  /**
   * This function returns true if the movable transformation is an
   * identity transformation. As soon as the object is moved this function
   * will return false. You can use 'TransformIdentity()' to go back to the
   * identity transform which will again let this flag return true.
   * Note that this flag is only relevant for the transform of this
   * movable and doesn't look at the transforms of the parents. Use
   * IsFullTransformIdentity() for that.
   * <p>
   * The engine and visibility cullers can use this information to
   * optimize stuff a bit.
   */
  virtual bool IsTransformIdentity () const = 0;

  /**
   * Return true if the movable transformation is an identity transformation
   * and the (optional) parent of this movable also is has identity
   * transformation. Only in this case can you know that the object has
   * the same object space coordinates as world space coordinates.
   * Basically this function will return true if GetFullTransform() would
   * return the identity transform.
   */
  virtual bool IsFullTransformIdentity () const = 0;

  /**
   * Set the transform of this movable to the identity transform (i.e.
   * not moving at all). You have to call UpdateMove() after calling
   * this.
   */
  virtual void TransformIdentity () = 0;
};

/** @} */

#endif // __CS_IENGINE_MOVABLE_H__
