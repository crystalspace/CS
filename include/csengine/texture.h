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

#ifndef __CS_TEXTURE_H__
#define __CS_TEXTURE_H__

#include "cstypes.h"
#include "csobject/csobject.h"
#include "csobject/pobject.h"
#include "csobject/nobjvec.h"
#include "igraph2d.h"
#include "itexture.h"

struct iTextureManager;
struct iTextureHandle;
struct iImage;

/**
 * csTextureWrapper represents a texture and its link
 * to the iTextureHandle as returned by iTextureManager.
 */
class csTextureWrapper : public csPObject
{
private:
  /// The corresponding iImage.
  iImage* image;
  /// The handle as returned by iTextureManager.
  iTextureHandle* handle;
  // key color
  int key_col_r, key_col_g, key_col_b;

public:
  /// Texture registration flags
  int flags;

  /// Construct a texture handle given a image file
  csTextureWrapper (iImage* Image);

  /**
   * Construct a csTextureWrapper from a pre-registered AND prepared texture 
   * handle. The engine takes over responsibility for destroying the texture
   * handle. To prevent this IncRef () the texture handle.
   */
  csTextureWrapper (iTextureHandle *ith);

  /// Copy constructor
  csTextureWrapper (csTextureWrapper &th);
  /// Release texture handle
  virtual ~csTextureWrapper ();

  /// Get the texture handle.
  iTextureHandle* GetTextureHandle () { return handle; }

  /// Change the base iImage
  void SetImageFile (iImage *Image);
  /// Get the iImage.
  iImage* GetImageFile () { return image; }

  /// Set the transparent color.
  void SetKeyColor (int red, int green, int blue);

  /// Query the transparent color.
  void GetKeyColor (int &red, int &green, int &blue)
  { red = key_col_r; green = key_col_g; blue = key_col_b; }

  /// Register the texture with the texture manager
  void Register (iTextureManager *txtmng);

  CSOBJTYPE;
  DECLARE_IBASE;

  //-------------------- iTextureWrapper implementation -----------------------
  struct TextureWrapper : public iTextureWrapper
  {
    DECLARE_EMBEDDED_IBASE (csTextureWrapper);
  } scfiTextureWrapper;
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
  csTextureWrapper *NewTexture (iImage *image);

  /**
   * Create a engine wrapper for a pre-prepared iTextureHandle
   * The handle will be IncRefed
   */
  csTextureWrapper *NewTexture (iTextureHandle *ith);

  /// Return texture by index
  csTextureWrapper *Get (int idx)
  { return (csTextureWrapper *)csNamedObjVector::Get (idx); }

  /// Find a texture by name
  csTextureWrapper *FindByName (const char* iName)
  { return (csTextureWrapper *)csNamedObjVector::FindByName (iName); }
};

#endif // __CS_TEXTURE_H__
