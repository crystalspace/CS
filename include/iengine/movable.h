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

#ifndef __IENGINE_MOVABLE_H__
#define __IENGINE_MOVABLE_H__

#include "csutil/scf.h"
#include "csutil/csvector.h"
#include "csgeom/transfrm.h"

class csVector3;
class csMatrix3;
class csThing;
struct iSector;
struct iMovable;

SCF_VERSION (iMovableListener, 0, 0, 1);

/**
 * Implement this class if you're interested in hearing about
 * movable changes.
 */
struct iMovableListener : public iBase
{
  /// The movable has changed.
  virtual void MovableChanged (iMovable* movable, void* userdata) = 0;
  /// The movable is about to be destroyed.
  virtual void MovableDestroyed (iMovable* movable, void* userdata) = 0;
};

SCF_VERSION (iMovable, 0, 0, 8);

/**
 * This interface describes a movable entity. It is usually
 * attached to some other geometrical object like a thing or
 * mesh object.
 */
struct iMovable : public iBase
{
  /// Get the parent movable.
  virtual iMovable* GetParent () = 0;

  /**
   * Initialize the list of sectors to one sector where
   * this thing is. This is a conveniance funcion.
   * This function does not update the corresponding list in
   * the sector. It only does a local update here.
   * This function does not do anything if the parent is not NULL.
   */
  virtual void SetSector (iSector* sector) = 0;

  /**
   * Clear the list of sectors.
   * This function does not do anything if the parent is not NULL.
   */
  virtual void ClearSectors () = 0;

  /**
   * Add a sector to the list of sectors.
   * This function does not do anything if the parent is not NULL.
   */
  virtual void AddSector (iSector* sector) = 0;

  /**
   * Get list of sectors for this entity.
   * This will return the sectors of the parent if there
   * is a parent.
   */
  virtual csVector& GetSectors () = 0;

  /**
   * Get the specified sector where this entity lives.
   * (conveniance function).
   */
  virtual iSector* GetSector (int idx) = 0;

  /**
   * Return true if we are placed in a sector.
   */
  virtual bool InSector () = 0;

  /**
   * Set the transformation vector and sector to move to
   * some position.
   */
  virtual void SetPosition (iSector* home, const csVector3& v) = 0;

  /**
   * Set the transformation vector for this object. Note
   * that the sectors are unchanged.
   */
  virtual void SetPosition (const csVector3& v) = 0;

  /**
   * Get the current position.
   */
  virtual const csVector3& GetPosition () = 0;

  /**
   * Get the current position but keep track of hierarchical
   * transformations.
   */
  virtual const csVector3 GetFullPosition () = 0;

  /**
   * Set the transformation matrix for this entity.
   */
  virtual void SetTransform (const csMatrix3& matrix) = 0;

  /**
   * Set the world to object tranformation.
   */
  virtual void SetTransform (const csReversibleTransform& t) = 0;

  /**
   * Get the world to object tranformation.
   */
  virtual csReversibleTransform& GetTransform () = 0;

  /**
   * Construct the full world to object transformation given
   * this transformation and possible parents transformations.
   */
  virtual csReversibleTransform GetFullTransform () = 0;

  /**
   * Relative move.
   */
  virtual void MovePosition (const csVector3& v) = 0;

  /**
   * Relative transform.
   */
  virtual void Transform (csMatrix3& matrix) = 0;

  /**
   * Add a listener to this movable. This listener will be called whenever
   * the movable changes or right before the movable is destroyed.
   */
  virtual void AddListener (iMovableListener* listener, void* userdata) = 0;

  /**
   * Remove a listener from this movable.
   */
  virtual void RemoveListener (iMovableListener* listener) = 0;

  /**
   * After all movement has been done you need to
   * call UpdateMove() to make the final changes to the entity
   * that is controlled by this movable. This is very important!
   * This function is responsible for calling all movement listeners.
   */
  virtual void UpdateMove () = 0;

  /**
   * A number which indicates if the movable has been updated.
   * One can use this number to see if the position of the object
   * has changed since the last time it was checked.
   */
  virtual long GetUpdateNumber () = 0;
};

#endif
