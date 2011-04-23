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
   * Find the sector with the given name in this collection. Returns
   * nullptr if the sector is not found.
   */
  virtual iSector* FindSector(const char *name) = 0;
  
  /**
   * Find the mesh with the given name in this collection. Returns
   * nullptr if the mesh is not found.
   */
  virtual iMeshWrapper* FindMeshObject(const char *name) = 0;

  /**
   * Find the mesh factory with the given name in this collection. Returns
   * nullptr if the mesh factory is not found.
   */
  virtual iMeshFactoryWrapper* FindMeshFactory (const char *name) = 0;

  /**
   * Find the texture with the given name in this collection. Returns
   * nullptr if the texture is not found.
   */
  virtual iTextureWrapper* FindTexture(const char *name) = 0;

  /**
   * Find the material with the given name in this collection. Returns
   * nullptr if the material is not found.
   */
  virtual iMaterialWrapper* FindMaterial(const char *name) = 0;

  /**
   * Find the shader with the given name in this collection. Returns
   * nullptr if the shader is not found.
   */
  virtual iShader* FindShader(const char *name) = 0;

  /**
   * Find the camera position with the given name in this collection. Returns
   * nullptr if the camera position is not found.
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
