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

#ifndef __IENGINE_COLLECTION_H__
#define __IENGINE_COLLECTION_H__

#include "csutil/scf.h"

struct iObject;

/**
 * A collection is used to store related objects in a simple structure
 * to guarentee that they won't be freed by the engine. The engine has
 * a default collection where all iObjects are placed unless explicitly
 * placed in another collectino.
 */

struct iCollection : public virtual iBase
{
  SCF_INTERFACE(iCollection, 1,0,0);

 /**
  * Get the iObject for this collection.
  */
  virtual iObject *QueryObject() = 0;

  /**
   * Add an object to this collection.
   */
  virtual void Add(iObject *obj) = 0;

  /**
   * Remove an object from this collection.
   */
  virtual void Remove(iObject *obj) = 0;

  /**
   * Release all references to objects held by this collection.
   */
  virtual void ReleaseAllObjects() = 0;
};

#endif // __IENGINE_COLLECTION_H__
