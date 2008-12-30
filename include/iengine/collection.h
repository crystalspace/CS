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

#include "iutil/array.h"
#include "csutil/scf.h"

struct iCameraPosition;
struct iMaterialWrapper;
struct iMeshWrapper;
struct iMeshFactoryWrapper;
struct iObject;
struct iSector;
struct iTextureWrapper;
struct iShader;

/**
 * A collection is used to store related objects in a simple structure
 * to guarentee that they won't be freed by the engine and to help management.
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
  virtual void ReleaseAllObjects(bool debug = false) = 0;

  /**
   * Returns true if this collection is the parent of the object passed.
   */
  virtual bool IsParentOf(iObject* obj) = 0;

  /**
   * Looks to see if this collection contains the sector. If so,
   * it returns the sector.
   */
  virtual iSector* FindSector(const char *name) = 0;
  
  /**
   * Looks to see if this collection contains the sector. If so,
   * it returns the sector.
   */
  virtual iMeshWrapper* FindMeshObject(const char *name) = 0;

  /**
   * Looks to see if this collection contains the mesh factory. If so,
   * it returns the mesh factory.
   */
  virtual iMeshFactoryWrapper* FindMeshFactory (const char *name) = 0;

  /**
   * Looks to see if this collection contains the texture. If so,
   * it returns the texture.
   */
  virtual iTextureWrapper* FindTexture(const char *name) = 0;

  /**
   * Looks to see if this collection contains the material. If so,
   * it returns the material.
   */
  virtual iMaterialWrapper* FindMaterial(const char *name) = 0;

  /**
   * Looks to see if this collection contains the shader. If so,
   * it returns the shader.
   */
  virtual iShader* FindShader(const char *name) = 0;

  /**
   * Looks to see if this collection contains the camera position. If so,
   * it returns the camera position.
   */
  virtual iCameraPosition* FindCameraPosition(const char *name) = 0;
};

/**
 * Used for a readonly array of csRef<iCollection>.
 */
struct iCollectionArray : public iArrayReadOnly<csRef<iCollection> >
{
  SCF_IARRAYREADONLY_INTERFACE (iCollectionArray);
};

#endif // __IENGINE_COLLECTION_H__
