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

#ifndef TEXTURE_H
#define TEXTURE_H

#include "types.h"
#include "csobject/csobj.h"
#include "csgfxldr/boxfilt.h"
#include "igraph2d.h"

struct iTextureHandle;
struct iImageFile;

/**
 * csTextureHandle represents a texture and its link
 * to the iTextureHandle as returned by iTextureManager.
 */
class csTextureHandle : public csObject
{
private:
  /// The corresponding iImageFile.
  iImageFile* ifile;
  /// The handle as returned by iTextureManager.
  iTextureHandle* txt_handle;
  // Transparent color
  int transp_r, transp_g, transp_b;

public:
  /// On texture registration: register for 2D operations?
  bool for_2d;
  /// On texture registration: register for 3D operations?
  bool for_3d;

  /// Construct a texture handle given a image file
  csTextureHandle (iImageFile* image);
  /// Copy contstructor
  csTextureHandle (csTextureHandle &th);
  /// Release texture handle
  virtual ~csTextureHandle ();

  /// Get the texture handle.
  iTextureHandle* GetTextureHandle () { return txt_handle; }

  /// Set the texture handle.
  void SetTextureHandle (iTextureHandle* h);

  /// Get the iImageFile.
  iImageFile* GetImageFile () { return ifile; }

  /// Set the transparent color.
  void SetTransparent (int red, int green, int blue);

  /// Query the transparent color.
  void GetTransparent (int &red, int &green, int &blue)
  { red = transp_r; green = transp_g; blue = transp_b; }

  CSOBJTYPE;
};


/**
 * This class maintains all named textures and their
 * corresponding handles.
 */
class csTextureList
{
private:
  /// List of textures.
  csTextureHandle** textures;
  ///
  int num_textures;
  ///
  int max_textures;

private:
  ///
  int GetTextureIdx (const char* name);

public:
  /// Add a texture
  void AddTexture (csTextureHandle* tm);

public:
  ///
  csTextureList ();
  ///
  virtual ~csTextureList ();

  ///
  void Clear ();

  /// Create a new texture.
  csTextureHandle* NewTexture (iImageFile* image);

  /// Return number of textures
  int GetNumTextures () { return num_textures; }

  /// Return texture by index
  csTextureHandle* GetTextureMM (int idx) { return textures[idx]; }

  /// Find a texture given a name.
  csTextureHandle* GetTextureMM (const char* name);
};


#endif /*TEXTURE_H*/
