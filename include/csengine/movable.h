/*
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

#ifndef __CS_MOVABLE_H__
#define __CS_MOVABLE_H__

#include "csutil/typedvec.h"
#include "iengine/movable.h"
#include "iengine/sector.h"

class csVector3;
class csMatrix3;
class csMovable;
struct iMeshWrapper;

CS_DECLARE_TYPED_VECTOR_NODELETE (csMovableListenerVector, iMovableListener);
CS_DECLARE_TYPED_VECTOR_NODELETE (csSectorVector, iSector);

/// A list of sectors as the movable uses it
class csMovableSectorList : public csSectorVector
{
private:
  csMovable* movable;

public:
  SCF_DECLARE_IBASE;

  csMovableSectorList ();
  void SetMovable (csMovable* mov) { movable = mov; }
  iSector *FindByName (const char *name) const;
  void AddSector (iSector* sec);
  void RemoveSector (iSector* sec);
  class SectorList : public iSectorList
  {
    SCF_DECLARE_EMBEDDED_IBASE (csMovableSectorList);
    virtual int GetSectorCount () const;
    virtual iSector *GetSector (int idx) const;
    virtual void AddSector (iSector *sec);
    virtual void RemoveSector (iSector *sec);
    virtual iSector *FindByName (const char *name) const;
    virtual int Find (iSector *sec) const;
  } scfiSectorList;
};

/**
 * This class represents an entity that can move in the engine.
 * It has a list of sectors and a position (a list of sectors
 * because an entity can overlap several sectors at the same time
 * through portals). This class itself does not have geometry.
 * It is only responsible for managing movement.
 */
class csMovable : public iBase
{
private:
  /// World to object transformation.
  csReversibleTransform obj;
  /// List of sectors.
  csMovableSectorList sectors;
  /// List of listeners to this movable.
  csMovableListenerVector listeners;
  /// List of user-data for the listeners.
  csVector listener_userdata;

  /**
   * Parent (for hierachical transformations).
   * Note that if the parent is not NULL then the list of
   * sectors is ignored for this movable (the parent list is
   * returned) and the 'obj' transformation is relative to
   * the parent one.
   */
  iMovable* parent;

  /**
   * Mesh on which this movable operates.
   */
  iMeshWrapper* object;

  /// Update number.
  long updatenr;

public:
  /**
   * Create a default movable.
   */
  csMovable ();

  /// Destructor.
  virtual ~csMovable ();

  /// Set mesh on which this movable operates.
  void SetMeshWrapper (iMeshWrapper* obj) { object = obj; }

  /// Get the mesh wrapper on which we operate.
  iMeshWrapper* GetMeshWrapper () { return object; }

  /// Set the parent movable.
  void SetParent (iMovable* parent);

  /// Get the parent movable.
  iMovable* GetParent () const
  {
    return parent;
  }

  /**
   * Initialize the list of sectors to one sector where
   * this thing is. This is a conveniance funcion.
   * This function does not do anything if the parent is not NULL.
   */
  void SetSector (iSector* sector);

  /**
   * Clear the list of sectors.
   * This function does not do anything if the parent is not NULL.
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
    else return &sectors.scfiSectorList;
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
  csReversibleTransform GetFullTransform () const;

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
  void AddListener (iMovableListener* listener, void* userdata);

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

  SCF_DECLARE_IBASE;

  //------------------------- iMovable interface -------------------------------
  struct eiMovable : public iMovable
  {
    SCF_DECLARE_EMBEDDED_IBASE (csMovable);
    virtual iMovable* GetParent () const;
    virtual void SetParent (iMovable* parent)
    {
      scfParent->SetParent (parent);
    }
    virtual void SetSector (iSector* sector);
    virtual void ClearSectors ();
    virtual iSectorList *GetSectors ();
    virtual bool InSector () const;
    virtual void SetPosition (iSector* home, const csVector3& v);
    virtual void SetPosition (const csVector3& v);
    virtual const csVector3& GetPosition () const;
    virtual const csVector3 GetFullPosition () const;
    virtual void SetTransform (const csMatrix3& matrix);
    virtual void SetTransform (const csReversibleTransform& t);
    virtual csReversibleTransform& GetTransform ();
    virtual csReversibleTransform GetFullTransform () const;
    virtual void MovePosition (const csVector3& v);
    virtual void Transform (const csMatrix3& matrix);
    virtual void AddListener (iMovableListener* listener, void* userdata);
    virtual void RemoveListener (iMovableListener* listener);
    virtual void UpdateMove ();
    virtual long GetUpdateNumber () const;
  } scfiMovable;
  friend struct eiMovable;
};

#endif // __CS_MOVABLE_H__
