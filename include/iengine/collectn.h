/*
    Crystal Space 3D engine
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

#ifndef __IENGINE_COLLECTN_H__
#define __IENGINE_COLLECTN_H__

#include "csutil/scf.h"

struct iObject;

SCF_VERSION (iCollection, 0, 0, 3);

/**
 * A collection object is for conveniance of the script language.
 */
struct iCollection : public iBase
{
  /// @@@ UGLY
  virtual void* GetPrivateObject () = 0;

  /// Query the iObject for this collection
  virtual iObject *QueryObject() = 0;

  /// Find an object with the given name inside this collection.
  virtual iObject* FindObject (char* name) const = 0;

  /// Get the number of objects in this collection.
  virtual int GetObjectCount () const = 0;

  /// Add an object to the collection.
  virtual void AddObject (iObject* obj) = 0;

  /// Get an object by index (operator version)
  virtual iObject* operator[] (int i) const = 0;

  /// Get an object by index (function version)
  virtual iObject* GetObject (int i) const = 0;
};

SCF_VERSION (iCollectionList, 0, 0, 1);

struct iCollectionList : public iBase
{
  /// Return the number of collections in this list.
  virtual int GetCollectionCount () const = 0;
  /// Return a single collection.
  virtual iCollection *GetCollection (int idx) const = 0;
  /// Create a new collection.
  virtual iCollection* NewCollection (const char* name) = 0;
  /// Remove a collection
  virtual void RemoveCollection (iCollection *collection) = 0;
  /// Find a collection by name
  virtual iCollection *FindByName (const char *name) const = 0;
  /// Find a collection and return its index
  virtual int Find (iCollection *collection) const = 0;
};

#endif // __IENGINE_COLLECTN_H__

