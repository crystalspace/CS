/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "types.h"
#include "csobject/csobject.h"
#include "csobject/nobjvec.h"
#include "igraph2d.h"

struct iTextureManager;
struct iTextureHandle;
struct iImage;

/**
 * csTextureHandle represents a texture and its link
 * to the iTextureHandle as returned by iTextureManager.
 */
class csTextureHandle : public csObject
{
private:
  /// The corresponding iImage.
  iImage* image;
  /// The handle as returned by iTextureManager.
  iTextureHandle* handle;
  // Transparent color
  int transp_r, transp_g, transp_b;

public:
  /// Texture registration flags
  int flags;

  /// Construct a texture handle given a image file
  csTextureHandle (iImage* Image);
  /// Copy contstructor
  csTextureHandle (csTextureHandle &th);
  /// Release texture handle
  virtual ~csTextureHandle ();

  /// Get the texture handle.
  iTextureHandle* GetTextureHandle () { return handle; }

  /// Get the iImage.
  iImage* GetImageFile () { return image; }

  /// Set the transparent color.
  void SetTransparent (int red, int green, int blue);

  /// Query the transparent color.
  void GetTransparent (int &red, int &green, int &blue)
  { red = transp_r; green = transp_g; blue = transp_b; }

  /// Register the texture with the texture manager
  void Register (iTextureManager *txtmng);

  CSOBJTYPE;
};

/**
 * This class is used to hold a list of textures.
 */
class csTextureList : public csNamedObjVector
{
public:
  /// Initialize the array
  csTextureList () : csNamedObjVector (16, 16)
  { }
  /// Destroy every texture in the list
  virtual ~csTextureList ();

  /// Create a new texture.
  csTextureHandle *NewTexture (iImage *image);

  /// Return texture by index
  csTextureHandle *Get (int idx)
  { return (csTextureHandle *)csNamedObjVector::Get (idx); }

  /// Find a texture by name
  csTextureHandle *FindByName (const char* iName)
  { return (csTextureHandle *)csNamedObjVector::FindByName (iName); }
};

#endif // __TEXTURE_H__
