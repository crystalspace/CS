/*
    Copyright (C) 1998, 2000 by Jorrit Tyberghein

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

#ifndef __CS_IENGINE_TEXTURE_H__
#define __CS_IENGINE_TEXTURE_H__

/**\file
 * Texture interfaces
 */
/**
 * \addtogroup engine3d_textures
 * @{ */
 
#include "csutil/scf.h"

struct iImage;
struct iObject;
struct iTextureHandle;
struct iTextureManager;
struct iTextureWrapper;

/**
 * A callback for when a iTextureWrapper is used.
 *
 * This callback is used by:
 * - iTextureWrapper
 */
struct iTextureCallback : public virtual iBase
{
  SCF_INTERFACE(iTextureCallback, 2,0,0);
  /// Get height.
  virtual void UseTexture (iTextureWrapper* wrap) = 0;
};

/**
 * A texture wrapper is an engine-level object that wraps around an actual
 * texture (iTextureHandle). Every texture in the engine is represented
 * by a texture wrapper, which keeps the pointer to the texture handle, its
 * name, and possibly the base image object.
 *
 * Main creators of instances implementing this interface:
 * - iEngine::CreateTexture()
 * - iEngine::CreateBlackTexture()
 * - iTextureList::NewTexture()
 * - iLoader::LoadTexture()
 *
 * Main ways to get pointers to this interface:
 * - iEngine::FindTexture()
 * - iTextureList::Get()
 * - iTextureList::FindByName()
 * - iLoaderContext::FindTexture()
 * - iLoaderContext::FindNamedTexture()
 * - iMaterialEngine::GetTextureWrapper()
 *
 * Main users of this interface:
 * - iEngine
 */
struct iTextureWrapper : public virtual iBase
{
  SCF_INTERFACE(iTextureWrapper, 2,0,0);
  /// Get the iObject for this texture
  virtual iObject *QueryObject() = 0;

  /// Create a clone this texture wrapper, using the same texture handle
  virtual iTextureWrapper *Clone () const = 0;

  /**
   * Change the base iImage. The changes will not be visible until the
   * texture is registered again.
   */
  virtual void SetImageFile (iImage *Image) = 0;
  /// Get the iImage.
  virtual iImage* GetImageFile () = 0;

  /**
   * Change the texture handle. The changes will immediatly be visible. This
   * will also change the key color and registration flags to those of
   * the new texture and the iImage to 0.
   */
  virtual void SetTextureHandle (iTextureHandle *tex) = 0;
  /**
   * Get the texture handle.
   * A texture handle will be available after one of Register() or
   * iEngine::Prepare() was called or a handle set manually with 
   * SetTextureHandle().
   */
  virtual iTextureHandle* GetTextureHandle () = 0;

  /// Set the transparent color.
  virtual void SetKeyColor (int red, int green, int blue) = 0;
  /// Query the transparent color.
  virtual void GetKeyColor (int &red, int &green, int &blue) const = 0;

  /// Set the flags which are used to register the texture
  virtual void SetFlags (int flags) = 0;
  /// Return the flags which are used to register the texture
  virtual int GetFlags () const = 0;

  /**
   * Register the texture with the texture manager, making a texture handle
   * available.
   */
  virtual void Register (iTextureManager *txtmng) = 0;

  /**
   * Set a callback which is called just before the texture is used.
   * This is mainly useful for procedural textures which can then
   * choose to update their image.
   */
  virtual void SetUseCallback (iTextureCallback* callback) = 0;

  /**
   * Get the use callback. If there are multiple use callbacks you can
   * use this function to chain.
   */
  virtual iTextureCallback* GetUseCallback () const = 0;

  /**
   * Visit this texture. This should be called by the engine right
   * before using the texture. It is responsible for calling the use
   * callback if there is one.
   */
  virtual void Visit () = 0;

  /**
   * Return true if it is needed to call Visit().
   */
  virtual bool IsVisitRequired () const = 0;

  /**
   * Set the keep image flag. See KeepImage() function for explanation.
   */
  virtual void SetKeepImage (bool k) = 0;

  /**
   * If this flag is true then the image will be kept even after
   * calling Register. If this flag is false then Register() will
   * remove the image pointer from this texture wrapper. False by default.
   */
  virtual bool KeepImage () const = 0;

  /**
   * Set the "class" of this texture.
   * For more information, see iTextureHandle::SetTextureClass.
   */
  virtual void SetTextureClass (const char* className) = 0;
  /**
   * Get the "class" of this texture.
   * For more information, see iTextureHandle::GetTextureClass.
   */
  virtual const char* GetTextureClass () = 0;
};


/**
 * This class represents a list of texture wrappers.
 *
 * Main ways to get pointers to this interface:
 *   - iEngine::GetTextureList()
 */
struct iTextureList : public virtual iBase
{
  SCF_INTERFACE (iTextureList, 2, 0, 0);

  /**
   * Create a new texture.
   * Remember that the texture needs to be <em>registered</em> before it can
   * be used.
   */
  virtual iTextureWrapper *NewTexture (iImage *image) = 0;

  /**
   * Create a new texture but don't add it to the engine list.
   * Remember that the texture needs to be <em>registered</em> before it can
   * be used.
   */
  virtual csPtr<iTextureWrapper> CreateTexture (iImage *image) = 0;

  /**
   * Create a engine wrapper for a pre-prepared iTextureHandle
   * The handle will be IncRefed.
   */
  virtual iTextureWrapper *NewTexture (iTextureHandle *ith) = 0;

  /**
   * Create a engine wrapper for a pre-prepared iTextureHandle
   * The handle will be IncRefed. The texture won't be added to
   * the engine list.
   */
  virtual csPtr<iTextureWrapper> CreateTexture (iTextureHandle *ith) = 0;

  /// Return the number of textures in this list.
  virtual int GetCount () const = 0;

  /// Return a texture by index.
  virtual iTextureWrapper *Get (int n) const = 0;

  /// Add a texture.
  virtual int Add (iTextureWrapper *obj) = 0;

  /// Remove a texture.
  virtual bool Remove (iTextureWrapper *obj) = 0;

  /// Remove the nth texture.
  virtual bool Remove (int n) = 0;

  /// Remove all textures.
  virtual void RemoveAll () = 0;

  /// Find a texture and return its index.
  virtual int Find (iTextureWrapper *obj) const = 0;

  /// Find a texture by name.
  virtual iTextureWrapper *FindByName (const char *Name) const = 0;
};

/** @} */

#endif // __CS_IENGINE_TEXTURE_H__
