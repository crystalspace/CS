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
struct iTextureHandle;
struct iTextureManager;
struct iTextureWrapper;

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
  /// @@@temporary: return the private csTextureWrapper object
  virtual csTextureWrapper *GetPrivateObject() const = 0;

  /// Get the texture handle
  virtual iTextureHandle *GetTextureHandle() const = 0;

  /// Set the transparent color.
  virtual void SetKeyColor (int red, int green, int blue) = 0;
  /// Query the transparent color.
  virtual void GetKeyColor (int &red, int &green, int &blue) const = 0;

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
  virtual csTextureCallback* GetUseCallback () const = 0;
  /**
   * Get the data for the use callback.
   */
  virtual void* GetUseData () const = 0;
};

#endif // __IENGINE_TEXTURE_H__
