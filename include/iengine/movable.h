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
  /**
   * The movable has changed. This is called whenever something does
   * UpdateMove() on the movable.
   */
  virtual void MovableChanged (iMovable* movable) = 0;
  /// The movable is about to be destroyed.
  virtual void MovableDestroyed (iMovable* movable) = 0;
};

SCF_VERSION (iMovable, 0, 1, 2);

/**
 * This interface represents the position and orientation of an object
 * relative to its parent (this is the transformation between local object
 * space of the model and world space (i.e. where it is in the world)).
 * Movables are attached to objects (like meshes). For example, use
 * iMeshWrapper->GetMovable()) to get the movable belonging to some mesh.
 * <p>
 * The parent of an object can be null in which case it is positioned
 * relative to world space coordinates. The parent of an object can also
 * be another object in which case the transformation is relative to that
 * other object (and possible other objects from which that parent is itself
 * a child).
 * <p>
 * Usually models are defined with the local origin (0,0,0) somewhere in
 * the model. Sometimes in the center or the center-bottom depending on what
 * is easiest. It is important to realize that all position setting routines
 * below operate relative to the local origin of the object.
 * <p>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>iMeshWrapper::GetMovable()
 *   <li>iLight::GetMovable()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEngine
 *   </ul>
 */
struct iMovable : public iBase
{
  /**
   * Get the parent movable. This is relevant in case the object belonging
   * to this movable is part of a hierarchical transformation.
   */
  virtual iMovable* GetParent () const = 0;
  /**
   * Set the parent movable. Usually you don't need to call this function
   * yourselves as it is called automatically whenever you add some object
   * to another parent (using iMeshWrapper->GetChildren ()->Add() for meshes).
   */
  virtual void SetParent (iMovable* parent) = 0;

  /**
   * Initialize the list of sectors to one sector where
   * this object is. This is a convenience funcion. Calling this function
   * makes the object visible in this sector. Use GetSectors() if you want
   * to have more control over where the object really is.
   * This function does not do anything if the parent is not 0.
   * You have to call UpdateMove() after changing the sector
   * information.
   */
  virtual void SetSector (iSector* sector) = 0;

  /**
   * Clear the list of sectors. This basically makes the object invisible
   * as it will not be present in any sector. The object will still be present
   * in the engine though.
   * This function does not do anything if the parent is not 0.
   * You have to call UpdateMove() after changing the sector
   * information.
   */
  virtual void ClearSectors () = 0;

  /**
   * Get the list of sectors for this object. Using this list you can get
   * and set all sectors that this object is in. Note that if an object
   * crosses a portal then you should in theory add every touched sector
   * to this list of sectors. If objects are small then you can get away
   * by not doing this. But it is possible that you will get render/clipping
   * errors. There is a convenience function (iMeshWrapper->PlaceMesh())
   * which will attempt to find all sectors a mesh is in and update the
   * movable.
   * <p>
   * This will return the sectors of the parent if there is a parent.
   */
  virtual iSectorList* GetSectors () = 0;

  /**
   * Return true if we are placed in a sector (i.e. visible).
   */
  virtual bool InSector () const = 0;

  /**
   * Set the transformation vector and sector to move to
   * some position. This call doesn't change the orientation of the model.
   * You have to call UpdateMove() after changing the position
   * of an object.
   * <p>
   * This function ignores the hierarchical transformation this movable
   * may be part off. If part of a hierarchical transformation this function
   * will set the position relative to the parent.
   */
  virtual void SetPosition (iSector* home, const csVector3& v) = 0;

  /**
   * Set the transformation vector for this object. Note
   * that the sectors and orientation are unchanged.
   * You have to call UpdateMove() after changing the position
   * of an object.
   * <p>
   * This function ignores the hierarchical transformation this movable
   * may be part off. If part of a hierarchical transformation this function
   * will set the position relative to the parent.
   */
  virtual void SetPosition (const csVector3& v) = 0;

  /**
   * Get the current position. Remember that positions are relative
   * to some sector.
   * <p>
   * This function ignores the hierarchical transformation this movable
   * may be part off. If part of a hierarchical transformation this function
   * returns the position relative to the parent.
   */
  virtual const csVector3& GetPosition () const = 0;

  /**
   * Get the current position but keep track of hierarchical
   * transformations. So the returned vector will be the world space coordinate
   * where this object really is.
   */
  virtual const csVector3 GetFullPosition () const = 0;

  /**
   * Set the transformation matrix for this entity.
   * You have to call UpdateMove() after changing the transform
   * of an object.
   * <p>
   * <b>WARNING:</b> Do not scale objects using the transform in the movable!
   * Several subsystems in Crystal Space (like collision detection and
   * visibility culling) don't work properly if you do that. Instead use
   * iMeshWrapper or iMeshFactoryWrapper->HardTransform() to scale.
   * <p>
   * This function ignores the hierarchical transformation this movable
   * may be part off. If part of a hierarchical transformation this function
   * will set the transformation relative to the parent.
   */
  virtual void SetTransform (const csMatrix3& matrix) = 0;

  /**
   * Set the world to object tranformation. This==object and Other==world
   * for the transform.
   * <p>
   * <b>WARNING:</b> Do not scale objects using the transform in the movable!
   * Several subsystems in Crystal Space (like collision detection and
   * visibility culling) don't work properly if you do that. Instead use
   * iMeshWrapper or iMeshFactoryWrapper->HardTransform() to scale.
   * <p>
   * This function ignores the hierarchical transformation this movable
   * may be part off. If part of a hierarchical transformation this function
   * will set the transformation relative to the parent.
   */
  virtual void SetTransform (const csReversibleTransform& t) = 0;

  /**
   * Get the world to object tranformation. This==object
   * and Other==world so This2Other()
   * transforms from object to world space. If you modify this
   * transform you have to call UpdateMove() later.
   * <p>
   * This function ignores the hierarchical transformation this movable
   * may be part off. If part of a hierarchical transformation this function
   * will get the transformation relative to the parent.
   */
  virtual csReversibleTransform& GetTransform () = 0;

  /**
   * Construct the full world to object transformation given
   * this transformation and possible parents transformations.
   */
  virtual csReversibleTransform GetFullTransform () const = 0;

  /**
   * Relative move.
   * <p>
   * Note that this function will not check for collision detection or
   * portals.
   * <p>
   * This function ignores the hierarchical transformation this movable
   * may be part off. If part of a hierarchical transformation this function
   * will move relative to the parent.
   */
  virtual void MovePosition (const csVector3& v) = 0;

  /**
   * Relative transform.
   * <p>
   * This function ignores the hierarchical transformation this movable
   * may be part off. If part of a hierarchical transformation this function
   * will move relative to the parent.
   */
  virtual void Transform (const csMatrix3& matrix) = 0;

  /**
   * Add a listener to this movable. This listener will be called whenever
   * the movable changes (when UpdateMove() is called) or right before the
   * movable is destroyed.
   */
  virtual void AddListener (iMovableListener* listener) = 0;

  /**
   * Remove a listener from this movable.
   */
  virtual void RemoveListener (iMovableListener* listener) = 0;

  /**
   * After all movement has been done you need to
   * call UpdateMove() to make the final changes to the object
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
   * <p>
   * This function ignores the hierarchical transformation this movable
   * may be part off. If part of a hierarchical transformation this function
   * will only test the local transform for identity.
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
   * <p>
   * This function ignores the hierarchical transformation this movable
   * may be part off. If part of a hierarchical transformation this function
   * will only set the local transform to identity.
   */
  virtual void TransformIdentity () = 0;
};

/** @} */

#endif // __CS_IENGINE_MOVABLE_H__
