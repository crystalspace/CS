/*
    Copyright (C) 1998-2004 by Jorrit Tyberghein
	      (C) 2003 by Philip Aumayr
	      (C) 2004 by Frank Richter

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

#ifndef __CS_GL_TXTMGR_IMAGETEX_H__
#define __CS_GL_TXTMGR_IMAGETEX_H__

#include "csgfx/rgbpixel.h"

#include "gl_txtmgr_basictex.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{

class csGLTextureHandle : public csGLBasicTextureHandle
{
private:
  /// Used until Prepare() is called
  csRef<iImage> image;

  /// Stores the names of the images
  char* origName;

  /// The transparent color
  csRGBpixel transp_color;

  /// Prepare a single image (rescale to po2 and make transparency).
  csRef<iImage> PrepareIntImage (int actual_width, int actual_height,
      int actual_depth, iImage* srcimage, csAlphaMode::AlphaType newAlphaType);

  bool MakeUploadData (bool allowCompressed, 
    GLenum targetFormat, iImage* Image, int mipNum, int imageNum);

  void PrepareKeycolor (csRef<iImage>& image, const csRGBpixel& transp_color,
    csAlphaMode::AlphaType& alphaType);
  void CheckAlpha (int w, int h, int d, csRGBpixel *src, 
    const csRGBpixel* transp_color, csAlphaMode::AlphaType& alphaType);
public:
  csGLTextureHandle (iImage* image, int flags, csGLGraphics3D *iG3D);

  virtual ~csGLTextureHandle ();

  csRef<iImage>& GetImage () { return image; }
  /// Merge this texture into current palette, compute mipmaps and so on.
  virtual void PrepareInt ();

  /// Release the original image (iImage) as given by the engine.
  void FreeImage ();
  /**
   * Return the original dimensions of the image used to create this texture.
   * This is most often equal to GetMipMapDimensions (0, mw, mh) but in
   * some cases the texture will have been resized in order to accomodate
   * hardware restrictions (like power of two and maximum texture size).
   * This function returns the uncorrected coordinates.
   */
  virtual void GetOriginalDimensions (int& mw, int& mh);

  void CreateMipMaps();

  /**
   * Return the original dimensions of the image used to create this texture.
   * This is most often equal to GetMipMapDimensions (0, mw, mh, md) but in
   * some cases the texture will have been resized in order to accomodate
   * hardware restrictions (like power of two and maximum texture size).
   * This function returns the uncorrected coordinates.
   */
  virtual void GetOriginalDimensions (int& mw, int& mh, int &md);

  /// Get the original image name
  virtual const char* GetImageName () const;

  /// Enable key color
  virtual void SetKeyColor (bool Enable);

  /// Set the key color.
  virtual void SetKeyColor (uint8 red, uint8 green, uint8 blue);

  /// Get the key color status (false if disabled, true if enabled).
  virtual bool GetKeyColor () const;
  /// Get the key color
  virtual void GetKeyColor (uint8 &red, uint8 &green, uint8 &blue) const;

};

}
CS_PLUGIN_NAMESPACE_END(gl3d)

#endif // __CS_GL_TXTMGR_IMAGETEX_H__

