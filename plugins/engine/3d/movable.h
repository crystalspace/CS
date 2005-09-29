/*
    Copyright (C) 2000 by Jorrit Tyberghein
                  2005 by Marten Svanfeldt

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

#ifndef __CS_MOVABLE_H__
#define __CS_MOVABLE_H__

#include "csgeom/transfrm.h"
#include "csutil/nobjvec.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"
#include "iengine/movable.h"
#include "iengine/sector.h"

class csVector3;
class csMatrix3;
class csMovable;
class csMeshWrapper;
class csLight;
class csCamera;

/// A list of sectors as the movable uses it
class csMovableSectorList : public scfImplementation1<csMovableSectorList,
				iSectorList>,
                            public csRefArrayObject<iSector>
{
private:
  csMovable* movable;

public:

  csMovableSectorList ();
  virtual ~csMovableSectorList ();
  void SetMovable (csMovable* mov) { movable = mov; }

  bool PrepareSector (iSector* item);

  virtual int GetCount () const { return (int)Length (); }
  virtual iSector *Get (int n) const { return (*this)[n]; }
  virtual int Add (iSector *obj);
  virtual bool Remove (iSector *obj);
  virtual bool Remove (int n);
  virtual void RemoveAll ();
  virtual int Find (iSector *obj) const;
  virtual iSector *FindByName (const char *Name) const;
};

/**
 * This class represents an entity that can move in the engine.
 * It has a list of sectors and a position (a list of sectors
 * because an entity can overlap several sectors at the same time
 * through portals). This class itself does not have geometry.
 * It is only responsible for managing movement.
 */
class csMovable : public scfImplementation1<csMovable, iMovable>
{
private:
  /// World to object transformation.
  csReversibleTransform obj;
  /// The following flag is true if the transform is still identity.
  bool is_identity;
  /// List of sectors.
  csMovableSectorList sectors;
  /// List of listeners to this movable.
  csRefArray<iMovableListener> listeners;

  /**
   * Parent (for hierachical transformations).
   * Note that if the parent is not 0 then the list of
   * sectors is ignored for this movable (the parent list is
   * returned) and the 'obj' transformation is relative to
   * the parent one. The pointer is not reference-counted.
   */
  iMovable* parent;

  /**
   * Meshobject on which this movable operates.
   */
  csMeshWrapper* meshobject;

  /**
   * Light on which this movable operates. 
   */
  csLight* lightobject;

  /**
   * Camera on which this movable operates.
   */
  csCamera* cameraobject;

  /// Update number.
  long updatenr;

public:
  /**
   * Create a default movable.
   */
  csMovable ();

  /// Destructor.
  virtual ~csMovable ();

  /// Set meshobject on which this movable operates.
  void SetMeshWrapper (csMeshWrapper* obj)
  { 
    meshobject = obj; 
  }

  /// Get the meshobject on which we operate.
  csMeshWrapper* GetMeshWrapper () const
  { 
    return meshobject; 
  }

  /// Set light on which this movable operates.
  void SetLight (csLight* obj)
  { 
    lightobject = obj; 
  }

  /// Get the light on which we operate.
  csLight* GetLight () const
  { 
    return lightobject; 
  }

  /// Set camera on which this movable operates.
  void SetCamera (csCamera* obj)
  { 
    cameraobject = obj; 
  }

  /// Get the camera on which we operate.
  csCamera* GetCamera () const
  { 
    return cameraobject; 
  }

  /// Set the parent movable.
  void SetParent (iMovable* par)
  { 
    parent = par; 
  }

  /// Get the parent movable.
  iMovable* GetParent () const
  { 
    return parent; 
  }

  /**
   * Initialize the list of sectors to one sector where
   * this thing is. This is a convenience funcion.
   * This function does not do anything if the parent is not 0.
   */
  void SetSector (iSector* sector);

  /**
   * Clear the list of sectors.
   * This function does not do anything if the parent is not 0.
   */
  void ClearSectors ();

  /**
   * Get list of sectors for this entity.
   * This will return the sectors of the parent if there
   * is a parent.
   */
  iSectorList *GetSectors ()
  {
    if (parent) return parent->GetSectors ();
    else return (iSectorList*)&sectors;
  }

  /**
   * Return true if we are placed in a sector.
   */
  bool InSector () const
  {
    return sectors.Length () > 0;
  }

  /**
   * Set the transformation vector and sector to move to
   * some position.
   */
  void SetPosition (iSector* home, const csVector3& v);

  /**
   * Set the transformation vector for this object. Note
   * that the sectors are unchanged.
   */
  void SetPosition (const csVector3& v)
  {
    obj.SetOrigin (v);
  }

  /**
   * Get the current local position.
   */
  const csVector3& GetPosition () const { return obj.GetOrigin (); }

  /**
   * Get the current position using the full transformation (using
   * possible parent transformations).
   * @@@ Currently not very efficient!
   */
  const csVector3 GetFullPosition () const
  {
    return GetFullTransform ().GetOrigin ();
  }

  /**
   * Set the transformation matrix for this entity.
   */
  void SetTransform (const csMatrix3& matrix);

  /**
   * Set the local world to object tranformation.
   */
  void SetTransform (const csReversibleTransform& t) { obj = t; }

  /**
   * Get the local world to object tranformation.
   */
  csReversibleTransform& GetTransform () { return obj; }

  /**
   * Get the local world to object tranformation.
   */
  const csReversibleTransform& GetTransform () const { return obj; }

  /**
   * Construct the full world to object transformation given
   * this transformation and possible parents transformations.
   */
  csReversibleTransform GetFullTransform () const
  {
    if (parent == 0)
      return GetTransform ();
    else if (is_identity)
      return parent->GetFullTransform ();
    else
      return GetTransform () * parent->GetFullTransform ();
  }

  /**
   * Relative move.
   */
  void MovePosition (const csVector3& v);

  /**
   * Relative transform.
   */
  void Transform (const csMatrix3& matrix);

  /**
   * After all movement has been done you need to
   * call UpdateMove() to make the final changes to the entity
   * that is controlled by this movable. This is very important!
   */
  void UpdateMove ();

  /**
   * Add a listener to this movable. This listener will be called whenever
   * the movable changes or right before the movable is destroyed.
   */
  void AddListener (iMovableListener* listener);

  /**
   * Remove a listener from this movable.
   */
  void RemoveListener (iMovableListener* listener);

  /**
   * A number which indicates if the movable has been updated.
   * One can use this number to see if the position of the object
   * has changed since the last time it was checked.
   */
  long GetUpdateNumber () const { return updatenr; }

  bool IsTransformIdentity () const
  {
    return is_identity;
  }
  bool IsFullTransformIdentity () const
  {
    if (!is_identity) return false;
    if (parent != 0)
      return parent->IsFullTransformIdentity ();
    return true;
  }
  void TransformIdentity ()
  {
    obj.Identity ();
  }
};

#endif // __CS_MOVABLE_H__
