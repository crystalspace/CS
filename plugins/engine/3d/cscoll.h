/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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
#include "csutil/refarr.h"
#include "csutil/csobject.h"
#include "iengine/collectn.h"

class csSector;
class csEngine;

SCF_VERSION (csCollection, 0, 0, 1);

/**
 * A collection object is for convenience of the script language.
 * It simply groups objects which are related in some way.
 */
class csCollection : public csObject
{
private:
  /// The list of objects contained in this csCollection.
  csRefArray<iObject> objects;

  /// Handle to the engine plug-in.
  csEngine* engine;

private:
  ///
  virtual ~csCollection ();

public:
  /**
   * Create a new csCollection with the given name.
   */
  csCollection (csEngine* engine);

  /**
   * Find an object with the given name inside this collection.
   */
  iObject* FindObject (char* name);

  /**
   * Get the number of objects in this collection.
   */
  int GetObjectCount () const { return (int)objects.Length(); }

  /// Add an object to the collection.
  void AddObject (iObject* obj) { objects.Push (obj); }

  ///
  iObject* operator[] (int i) { return objects[i]; }

  SCF_DECLARE_IBASE;

  //------------------------- iCollection interface --------------------------
  struct Collection : public iCollection
  {
    SCF_DECLARE_EMBEDDED_IBASE (csCollection);
    virtual iObject *QueryObject()
      { return scfParent; }
    virtual iObject* FindObject (char* name) const
      { return scfParent->FindObject(name); }
    virtual int GetObjectCount () const
      { return scfParent->GetObjectCount(); }
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
