/*
    Copyright (C) 1998,2000 by Jorrit Tyberghein
  
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

#ifndef __CS_CSCOLL_H__
#define __CS_CSCOLL_H__

#include "csgeom/matrix3.h"
#include "csobject/csobject.h"
#include "csobject/pobject.h"
#include "csutil/csvector.h"
#include "csengine/movable.h"
#include "iengine/collectn.h"

class csSector;
class csEngine;

/**
 * A collection object is for conveniance of the script language.
 * Objects are (currently) not really hierarchical in Crystal Space.
 * A collection simulates a hierarchy. The script can then perform
 * operations like 'move' and 'transform' on all the objects in
 * the collection together.
 */
class csCollection : public csPObject
{
  friend class csMovable;

private:
  /// The list of objects contained in this csCollection.
  csVector objects;

  /// Position in the world.
  csMovable movable;

  /// Handle to the engine plug-in.
  csEngine* engine;

protected:
  /// Move this collection to the specified sector. Can be called multiple times.
  virtual void MoveToSector (csSector* s);

  /// Remove this collection from all sectors it is in (but not from the engine).
  virtual void RemoveFromSectors ();

  /**
   * Update transformations after the collection has moved
   * (through updating the movable instance).
   * This MUST be done after you change the movable otherwise
   * some of the internal data structures will not be updated
   * correctly. This function is called by movable.UpdateMove().
   */
  virtual void UpdateMove ();

public:
  /**
   * Create a new csCollection with the given name.
   */
  csCollection (csEngine* engine);

  ///
  virtual ~csCollection ();

  /**
   * Get the movable instance for this collection.
   * It is very important to call GetMovable().UpdateMove()
   * after doing any kind of modification to this movable
   * to make sure that internal data structures are
   * correctly updated.
   */
  csMovable& GetMovable () { return movable; }

  /**
   * Find an object with the given name inside this collection.
   */
  iObject* FindObject (char* name);

  /**
   * Get the number of objects in this collection.
   */
  int GetNumObjects () const { return objects.Length(); }

  /// Add an object to the collection.
  void AddObject (iObject* obj) { objects.Push((csSome)obj); }

  ///
  iObject* operator[] (int i) { return (iObject*) (objects[i]); }
  
  CSOBJTYPE;
  DECLARE_IBASE;

  //------------------------- iCollection interface --------------------------
  struct Collection : public iCollection
  {
    DECLARE_EMBEDDED_IBASE (csCollection);
    virtual iObject *QueryObject()
      { return scfParent; }
    virtual iMovable* GetMovable () const
      { return &(scfParent->GetMovable().scfiMovable); }
    virtual iObject* FindObject (char* name) const
      { return scfParent->FindObject(name); }
    virtual int GetNumObjects () const
      { return scfParent->GetNumObjects(); }
    virtual void AddObject (iObject* obj)
      { scfParent->AddObject(obj); }
    virtual iObject* operator[] (int i) const
      { return (*scfParent)[i]; }
    virtual iObject* GetObject (int i) const
      { return (*scfParent)[i]; }
  } scfiCollection;
  friend struct Collection;
};

#endif // __CS_CSCOLL_H__
