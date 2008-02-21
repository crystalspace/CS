/*
Copyright (C) 2008 by Jorrit Tyberghein and Michael Gist

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

#ifndef __COLLECTION_H__
#define __COLLECTION_H__

#include "csutil/csobject.h"
#include "csutil/scf_implementation.h"
#include "iengine/collection.h"

class csEngine;

/**
 * A collection is used to store related objects in a simple structure
 * to guarentee that they won't be freed by the engine. The engine has
 * a default collection where all iObjects are placed unless explicitly
 * placed in another collectino.
 */

class csCollection : public scfImplementationExt1<csCollection,
                                                  csObject,
                                                  iCollection>
{
public:
  /**
   * Initialize an empty collection.
   */
  csCollection() : scfImplementationType (this) {}
  /**
   * Delete this collection, releasing all references held.
   */
  ~csCollection() { ReleaseAllObjects(); }

  /**
  * Get the iObject for this collection.
  */
  iObject *QueryObject() { return this; }

  /**
   * Add an object to this collection.
   */
  inline void Add(iObject *obj) { ObjAdd(obj); }

  /**
   * Remove an object from this collection.
   */
  inline void Remove(iObject *obj) { ObjRemove(obj); }

  /**
   * Release all references to objects held by this collection.
   */
  inline void ReleaseAllObjects() { ObjRemoveAll(); }

  /**
   * Returns a pointer to the object if inside the collection.
   */
  template<class Type>
  inline csPtr<Type> Contains(const char* name)
  {
      return CS::GetNamedChildObject<Type>(this, name);
  }
};

#endif // __COLLECTION_H__
