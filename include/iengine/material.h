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

#ifndef __CS_IENGINE_MATERIAL_H__
#define __CS_IENGINE_MATERIAL_H__

#include "csutil/scf.h"
#include "iutil/strset.h"

/**\file
 * Material interfaces
 */
/**
 * \addtogroup engine3d_textures
 * @{ */


struct iMaterial;
struct iTextureManager;
struct iTextureWrapper;
struct iObject;

SCF_VERSION (iMaterialWrapper, 0, 0, 5);

/**
 * A material wrapper is an engine-level object that wraps around an actual
 * material (iMaterial). Every material in the engine is represented by a 
 * material wrapper, which keeps the pointer to the material and its name, and 
 * possibly the base material object.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iEngine::CreateMaterial()
 *   <li>iMaterialList::NewMaterial()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>iEngine::FindMaterial()
 *   <li>iMaterialList::Get()
 *   <li>iMaterialList::FindByName()
 *   <li>iLoaderContext::FindMaterial()
 *   <li>iLoaderContext::FindNamedMaterial()
 *   <li>iMeshObject::GetMaterialWrapper()
 *   <li>Various state interfaces for mesh objects.
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEngine
 *   <li>Mesh objects
 *   </ul>
 */
struct iMaterialWrapper : public iBase
{
  /// Get the iObject for this material.
  virtual iObject *QueryObject() = 0;

  /// Create a clone this material wrapper, using the same material handle
  virtual iMaterialWrapper *Clone () const = 0;

  /**
   * Change the base material. Note: The changes will not be visible until
   * you re-register the material.
   */
  virtual void SetMaterial (iMaterial* material) = 0;
  /// Get the original material.
  virtual iMaterial* GetMaterial () = 0;

  /**
   * Visit this material. This should be called by the engine right
   * before using the material. It will call Visit() on all textures
   * that are used.
   */
  virtual void Visit () = 0;

  /**
   * Return true if it is needed to call Visit().
   */
  virtual bool IsVisitRequired () const = 0;
};

SCF_VERSION (iMaterialEngine, 0, 0, 2);

/**
 * This interface represents the engine part of the material definition.
 * Using this interface you will be able to access the original texture
 * wrappers that were used to create the material. If you have something
 * that implements iMaterial you can query for iMaterialEngine.
 * So this interface basically augments iMaterial with engine specific
 * features.
 * <p>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>SCF_QUERY_INTERFACE() from iMaterial
 *   </ul>
 */
struct iMaterialEngine : public iBase
{
  /**
   * Get the base texture from the material.
   */
  virtual iTextureWrapper *GetTextureWrapper () = 0;

  /**
   * Get a texture by name.
   */
    virtual iTextureWrapper* GetTextureWrapper (csStringID name) = 0;

  /**
   * Visit all textures.
   */
  virtual void Visit () = 0;

  /**
   * Return true if it is needed to call Visit().
   */
  virtual bool IsVisitRequired () const = 0;
};

SCF_VERSION (iMaterialList, 0, 0, 1);

/**
 * This class represents a list of materials.
 * <p>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>iEngine::GetMaterialList()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEngine
 *   </ul>
 */
struct iMaterialList : public iBase
{
  /// Create a new material.
  virtual iMaterialWrapper* NewMaterial (iMaterial* material,
  	const char* name) = 0;

  /// Return the number of materials in this list
  virtual int GetCount () const = 0;

  /// Return a material by index
  virtual iMaterialWrapper *Get (int n) const = 0;

  /// Add a material
  virtual int Add (iMaterialWrapper *obj) = 0;

  /// Remove a material
  virtual bool Remove (iMaterialWrapper *obj) = 0;

  /// Remove the nth material
  virtual bool Remove (int n) = 0;

  /// Remove all materials
  virtual void RemoveAll () = 0;

  /// Find a material and return its index
  virtual int Find (iMaterialWrapper *obj) const = 0;

  /// Find a material by name
  virtual iMaterialWrapper *FindByName (const char *Name) const = 0;
};

/** @} */

#endif // __CS_IENGINE_MATERIAL_H__
