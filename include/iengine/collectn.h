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

#ifndef __CS_IENGINE_COLLECTN_H__
#define __CS_IENGINE_COLLECTN_H__

/**\file
 * Collection object
 */

#include "csutil/scf.h"

struct iObject;

/**
 * A collection object is for convenience of the script language.
 * It is simply a collection of other objects.
 * 
 * Main creators of instances implementing this interface:
 * - iCollectionList::NewCollection()
 * 
 * Main ways to get pointers to this interface:
 * - iCollectionList::Get()
 * - iCollectionList::FindByName()
 * 
 * Main users of this interface:
 * - Engine stores them.
 * - Application uses them.
 */
struct iCollection : public virtual iBase
{
  SCF_INTERFACE(iCollection, 2,0,0);
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

/**
 * iCollection list.
 *
 * Main ways to get pointers to this interface:
 *   - iEngine::GetCollections()
 *
 * Main users of this interface:
 *   - Engine stores them.
 *   - Application uses them.
 */
struct iCollectionList : public virtual iBase
{
  SCF_INTERFACE(iCollectionList,2,0,0);
  /// Create a new collection.
  virtual iCollection* NewCollection (const char* name) = 0;

  /// Return the number of collections in this list.
  virtual int GetCount () const = 0;

  /// Return a collection by index.
  virtual iCollection *Get (int n) const = 0;

  /// Add a collection.
  virtual int Add (iCollection *obj) = 0;

  /// Remove a collection.
  virtual bool Remove (iCollection *obj) = 0;

  /// Remove the nth collection.
  virtual bool Remove (int n) = 0;

  /// Remove all collections.
  virtual void RemoveAll () = 0;

  /// Find a collection and return its index.
  virtual int Find (iCollection *obj) const = 0;

  /// Find a collection by name.
  virtual iCollection *FindByName (const char *Name) const = 0;
};

#endif // __CS_IENGINE_COLLECTN_H__

