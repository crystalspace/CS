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

#ifndef __IENGINE_MATERIAL_H__
#define __IENGINE_MATERIAL_H__

#include "csutil/scf.h"

class csMaterialWrapper;
struct iMaterial;
struct iMaterialHandle;
struct iTextureManager;
struct iTextureWrapper;
struct iObject;

SCF_VERSION (iMaterialWrapper, 0, 0, 4);

/**
 * A material wrapper is an engine-level object that wraps around an actual
 * material (iMaterialHandle). Every material in the engine is represented
 * by a material wrapper, which keeps the pointer to the material handle, its
 * name, and possibly the base material object that was registered to create
 * the material handle.
 */
struct iMaterialWrapper : public iBase
{
  /// @@@Ugly.
  virtual csMaterialWrapper* GetPrivateObject () = 0;
  /// Get the iObject for this material.
  virtual iObject *QueryObject() = 0;

  /// Create a clone this material wrapper, using the same material handle
  virtual iMaterialWrapper *Clone () const = 0;

  /**
   * Change the material handle. Note: This will also change the base
   * material to NULL.
   */
  virtual void SetMaterialHandle (iMaterialHandle *mat) = 0;
  /// Get the material handle.
  virtual iMaterialHandle* GetMaterialHandle () = 0;

  /**
   * Change the base material. Note: The changes will not be visible until
   * you re-register the material.
   */
  virtual void SetMaterial (iMaterial* material) = 0;
  /// Get the original material.
  virtual iMaterial* GetMaterial () = 0;

  /// Register the material with the texture manager
  virtual void Register (iTextureManager *txtmng) = 0;

  /**
   * Visit this material. This should be called by the engine right
   * before using the material. It will call Visit() on all textures
   * that are used.
   */
  virtual void Visit () = 0;
};

SCF_VERSION (iMaterialEngine, 0, 0, 1);

/**
 * This interface represents the engine part of the material definition.
 * Using this interface you will be able to access the original texture
 * wrappers that were used to create the material. If you have something
 * that implements iMaterial you can query for iMaterialEngine.
 * So this interface basically augments iMaterial with engine specific
 * features.
 */
struct iMaterialEngine : public iBase
{
  /**
   * Get the base texture from the material.
   */
  virtual iTextureWrapper *GetTextureWrapper () = 0;

  /**
   * Get a texture used by a texture layer.
   */
  virtual iTextureWrapper* GetTextureWrapper (int idx) = 0;
};

SCF_VERSION (iMaterialList, 0, 0, 1);

/**
 * This class represents a list of materials.
 */
struct iMaterialList : public iBase
{
  /// Create a new material.
  virtual iMaterialWrapper* NewMaterial (iMaterial* material) = 0;

  /**
   * Create a engine wrapper for a pre-prepared iTextureHandle
   * The handle will be IncRefed.
   */
  virtual iMaterialWrapper *NewMaterial (iMaterialHandle *ith) = 0;

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

#endif // __IENGINE_MATERIAL_H__
