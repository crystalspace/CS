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

#ifndef __IENGINE_TEXTURE_H__
#define __IENGINE_TEXTURE_H__

#include "csutil/scf.h"
#include "cstypes.h"

class csTextureWrapper;
struct iImage;
struct iTextureHandle;
struct iTextureManager;
struct iTextureWrapper;
struct iObject;

/// A callback function for when a csTextureWrapper is used.
typedef void (csTextureCallback) (iTextureWrapper* wrap, void* data);

SCF_VERSION (iTextureWrapper, 0, 0, 3);

/**
 * This class represents a texture wrapper which holds
 * the mapping between a texture in the engine and a texture
 * in the 3D rasterizer.
 */
struct iTextureWrapper : public iBase
{
  /// @@@ return the private object
  virtual csTextureWrapper *GetPrivateObject() = 0;
  /// Get the iObject for this texture
  virtual iObject *QueryObject() = 0;

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
   * the new texture and the iImage to NULL.
   */
  virtual void SetTextureHandle (iTextureHandle *tex) = 0;
  /// Get the texture handle.
  virtual iTextureHandle* GetTextureHandle () = 0;

  /// Set the transparent color.
  virtual void SetKeyColor (int red, int green, int blue) = 0;
  /// Query the transparent color.
  virtual void GetKeyColor (int &red, int &green, int &blue) = 0;

  /// Set the flags which are used to register the texture
  virtual void SetFlags (int flags) = 0;
  /// Return the flags which are used to register the texture
  virtual int GetFlags () = 0;

  /// Register the texture with the texture manager
  virtual void Register (iTextureManager *txtmng) = 0;

  /**
   * Set a callback which is called just before the texture is used.
   * This is mainly useful for procedural textures which can then
   * choose to update their image.
   */
  virtual void SetUseCallback (csTextureCallback* callback, void* data) = 0;

  /**
   * Get the use callback. If there are multiple use callbacks you can
   * use this function to chain.
   */
  virtual csTextureCallback* GetUseCallback () = 0;

  /**
   * Get the use data.
   */
  virtual void* GetUseData () = 0;

  /**
   * Visit this texture. This should be called by the engine right
   * before using the texture. It is responsible for calling the use
   * callback if there is one.
   */
  virtual void Visit () = 0;
};


SCF_VERSION (iTextureList, 0, 0, 1);

/**
 * This class represents a list of texture wrappers.
 */
struct iTextureList : public iBase
{
  /// Create a new texture.
  virtual iTextureWrapper *NewTexture (iImage *image) = 0;
  /**
   * Create a engine wrapper for a pre-prepared iTextureHandle
   * The handle will be IncRefed
   */
  virtual iTextureWrapper *NewTexture (iTextureHandle *ith) = 0;
  /// Return the number of textures in the list
  virtual long GetNumTextures () const = 0;
  /// Return texture by index
  virtual iTextureWrapper *Get (int idx) const = 0;
  /// Find a texture by name
  virtual iTextureWrapper *FindByName (const char* iName) const = 0;
};

#endif // __IENGINE_TEXTURE_H__
