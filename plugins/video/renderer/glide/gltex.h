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

#ifndef TXTMGR_GLIDE_H
#define TXTMGR_GLIDE_H

#include "csutil/scf.h"
#include "video/renderer/common/txtmgr.h"
#include "itexture.h"
#include "iimage.h"

class csGraphics3DGlide;
class csGlideProcedural;
/**
 * csTextureMMGlide represents a texture and all its mipmapped
 * variants.
 */
class csTextureMMGlide : public csTextureMM
{
private:
  csGraphics3DGlide *g3d;
  csGlideProcedural *dyn;

public:
  /// Create a mipmapped texture object
  csTextureMMGlide (csGraphics3DGlide *g3d, iImage* image, int flags);
  virtual ~csTextureMMGlide ();
  /// Create a new csTextureGlide object
  virtual csTexture *NewTexture (iImage *Image);
  /// Compute the mean color
  virtual void ComputeMeanColor ();
  /// Encode 24 bit data into 16 bit ( 565 RGB scheme )
  virtual void remap_mm ();
  virtual iGraphics3D *GetProcTextureInterface();
};

/**
* The Glide version of csTexture
*/
class csTextureGlide : public csTexture
{
  friend class csTextureMMGlide;
  /// the original image
  iImage *image;
  /// 16 bit encoded raw image data
  UShort *raw;
public:
  /// Create a new texture object
  csTextureGlide (csTextureMM *Parent, iImage *Image);
  /// Destroy the texture object
  virtual ~csTextureGlide ();
  /// Get the raw bitmap data
  void *get_bitmap () 
  { return (void *) raw; }
  /// 
  RGBPixel *get_image_data ()
  { return (RGBPixel*)image->GetImageData (); }
  iImage *get_image ()
  { return image; }
};

/**
 * Glide version of the texture manager. This
 */
class csTextureManagerGlide : public csTextureManager
{

 protected:
  csGraphics3DGlide *g3d;

public:
  ///
  csTextureManagerGlide (iSystem* iSys, iGraphics2D* iG2D, csGraphics3DGlide* iG3D, csIniFile *config);
  ///
  virtual ~csTextureManagerGlide ();

  ///
  virtual void PrepareTextures ();
  ///
  virtual iTextureHandle *RegisterTexture (iImage* image, int flags);
  ///
  virtual void PrepareTexture (iTextureHandle *handle);
  ///
  virtual void UnregisterTexture (iTextureHandle* handle);

};

#endif 
