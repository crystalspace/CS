/*
    Copyright (C) 2003 by Jorrit Tyberghein

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

#ifndef __CS_IENGINE_OBJWATCH_H__
#define __CS_IENGINE_OBJWATCH_H__

/**\file
 * Object change watcher
 */
/**
 * \addtogroup engine3d
 * @{ */
 
#include "csutil/scf.h"

struct iMovable;
struct iLight;
struct iSector;
struct iMeshWrapper;

/** \name Operations
 * Operations indicate what has changed in one of the objects that
 * is being watched.
 * @{ */
/// No operation recorded yet.
#define CS_WATCH_NONE 0
/// Light has been destroyed.
#define CS_WATCH_LIGHT_DESTROY 1
/// Light has moved.
#define CS_WATCH_LIGHT_MOVE 2
/// Light has changed radius.
#define CS_WATCH_LIGHT_RADIUS 3
/// Light has changed color.
#define CS_WATCH_LIGHT_COLOR 4
/// Light has changed sector.
#define CS_WATCH_LIGHT_SECTOR 5
/// Movable has been destroyed.
#define CS_WATCH_MOVABLE_DESTROY 6
/// Movable has changed otherwise.
#define CS_WATCH_MOVABLE_CHANGED 7
/// Light has changed attenuation.
#define CS_WATCH_LIGHT_ATTENUATION 8
/// Sector has a new mesh.
#define CS_WATCH_SECTOR_NEWMESH 9
/// Sector has a removed mesh.
#define CS_WATCH_SECTOR_REMOVEMESH 10
/** @} */


/**
 * Implement this class if you're interested in hearing about
 * object watcher events.
 */
struct iObjectWatcherListener : public virtual iBase
{
  SCF_INTERFACE (iObjectWatcherListener, 0, 0, 1);

  /**
   * A change has occured. You can use the operation to examine what
   * kind of change it is.
   */
  virtual void ObjectChanged (int op, iMovable* movable) = 0;

  /**
   * A change has occured. You can use the operation to examine what
   * kind of change it is.
   */
  virtual void ObjectChanged (int op, iLight* light) = 0;

  /**
   * A change has occured. You can use the operation to examine what
   * kind of change it is.
   */
  virtual void ObjectChanged (int op, iSector* sector, iMeshWrapper* mesh) = 0;
};


/**
 * This is a generic object watcher. Currently it can watch on light
 * and movable changes. You can query if something has changed by
 * examining the 'number' or else you can register a listener and
 * get notified when one of the objects changes. This object will
 * not keep real references to the objects it is watching but it will
 * clean up the watcher for some object if that object is removed.
 */
struct iObjectWatcher : public virtual iBase
{
  SCF_INTERFACE(iObjectWatcher, 2,0,0);
  /// Add a light to watch.
  virtual void WatchLight (iLight* light) = 0;
  /// Remove a light to watch.
  virtual void RemoveLight (iLight* light) = 0;
  /// Get the number of watched lights.
  virtual int GetWatchedLightCount () const = 0;
  /// Get the specified watched light.
  virtual iLight* GetLight (int idx) = 0;

  /// Add a movable to watch.
  virtual void WatchMovable (iMovable* movable) = 0;
  /// Remove a movable to watch.
  virtual void RemoveMovable (iMovable* movable) = 0;
  /// Get the number of watched movables.
  virtual int GetWatchedMovableCount () const = 0;
  /// Get the specified watched movable.
  virtual iMovable* GetMovable (int idx) = 0;

  /// Add a sector to watch for meshes.
  virtual void WatchSector (iSector* sector) = 0;
  /// Remove a sector to watch.
  virtual void RemoveSector (iSector* sector) = 0;
  /// Get the number of watched sectors.
  virtual int GetWatchedSectorCount () const = 0;
  /// Get the specified watched sector.
  virtual iSector* GetSector (int idx) = 0;

  /// Reset. Remove all watched objects from this watcher.
  virtual void Reset () = 0;

  /**
   * Get the current number for his watcher. This number will increase
   * as soon as some of the watched objects change. When this happens you
   * can query the last change (only the last change!) by calling
   * GetLastOperation() and/or GetLastLight() or GetLastMovable().
   * Note that if the operation indicates that something is destroyed
   * then you should no longer use the pointer returned by GetLastLight()
   * or GetLastMovable() as the object will already be gone by then.
   * You can only use the returned pointer to clean up from internal
   * data structures.
   */
  virtual uint32 GetWatchNumber () const = 0;

  /**
   * Get the last operation that occured. This will be one of:
   * <ul>
   * <li>#CS_WATCH_NONE: nothing happened yet.
   * <li>#CS_WATCH_LIGHT_DESTROY: light is destroyed.
   * <li>#CS_WATCH_LIGHT_MOVE: light has moved.
   * <li>#CS_WATCH_LIGHT_COLOR: light has changed color.
   * <li>#CS_WATCH_LIGHT_SECTOR: light has changed sector.
   * <li>#CS_WATCH_LIGHT_RADIUS: light has changed radius.
   * <li>#CS_WATCH_LIGHT_ATTENUATION: light has changed radius.
   * <li>#CS_WATCH_MOVABLE_DESTROY: movable is destroyed.
   * <li>#CS_WATCH_MOVABLE_CHANGED: movable is changed.
   * <li>#CS_WATCH_SECTOR_NEWMESH: sector has a new mesh.
   * <li>#CS_WATCH_SECTOR_REMOVEMESH: a mesh got removed from the sector.
   * </ul>
   */
  virtual int GetLastOperation () const = 0;

  /**
   * Get the last light. Only valid if the last operation (GetLastOperation())
   * is one of CS_WATCH_LIGHT_....
   */
  virtual iLight* GetLastLight () const = 0;

  /**
   * Get the last movable. Only valid if the last operation (GetLastOperation())
   * is one of CS_WATCH_MOVABLE_....
   */
  virtual iMovable* GetLastMovable () const = 0;

  /**
   * Get the last sector. Only valid if the last operation (GetLastOperation())
   * is one of CS_WATCH_SECTOR_....
   */
  virtual iSector* GetLastSector () const = 0;

  /**
   * Get the last mesh. Only valid if the last operation (GetLastOperation())
   * is one of CS_WATCH_SECTOR_....
   */
  virtual iMeshWrapper* GetLastMeshWrapper () const = 0;

  /**
   * Add a listener to this object watcher. This will call IncRef() on
   * the listener So make sure you call DecRef() to release your own reference.
   */
  virtual void AddListener (iObjectWatcherListener* cb) = 0;

  /**
   * Remove a listener.
   */
  virtual void RemoveListener (iObjectWatcherListener* cb) = 0;
};

/** @} */

#endif // __CS_IENGINE_OBJWATCH_H__

