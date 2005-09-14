/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Written by Samuel Humphreys

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

#ifndef __CS_CSGFX_MEMIMAGE_H__
#define __CS_CSGFX_MEMIMAGE_H__

#include "csextern.h"
#include "csutil/leakguard.h"
#include "csutil/scf_implementation.h"

#include "csgfx/imagebase.h"
#include "csgfx/imagetools.h"
#include "csgfx/rgbpixel.h"

/**
 * Memory image.
 */
class CS_CRYSTALSPACE_EXPORT csImageMemory :
  public scfImplementationExt0<csImageMemory, csImageBase>
{
private:
  /// Common code shared by constructors.
  void ConstructCommon();
  /// Used by ctors setting a width/height/format.
  void ConstructWHDF (int width, int height, int depth, int format);
  /// Used by the "copy from iImage" ctor.
  void ConstructSource (iImage* source);
  /// Used by the "init from buffers" ctors.
  void ConstructBuffers (int width, int height, void* buffer, 
    bool destroy, int format, csRGBpixel *palette);
protected:
  /// Width of image.
  int Width;
  /// Height of image.
  int Height;
  /// Depth of image.
  int Depth;
  /**
   * The image data.
   * A value of 0 means the Image contents are "undefined". However,
   * this is an internal state only. No operation should fail due
   * undefined image data (although the data may stay undefined).
   */
  void* Image;
  /// The image palette or 0
  csRGBpixel *Palette;
  /// The alpha map
  uint8 *Alpha;
  /// Image format (see CS_IMGFMT_XXX above)
  int Format;
  /// if it has a keycolour.
  bool has_keycolour;
  /// keycolour value
  csRGBpixel keycolour;
  /// If true when these interfaces are destroyed the image is also.
  bool destroy_image;
  /// Type of the contained images.
  csImageType imageType;
  /// Mip map images.
  /* @@@ This is not csRefArray<iImage> as this does not return csRef<iImage>&
   * from GetExtend() or operator[], which is needed here.
   */
  csArray<csRef<iImage> > mipmaps;

  /**
   * csImageMemory constructor, only set a format, no dimensions.
   * Intended to be used by loaders which later call SetDimensions().
   */
  csImageMemory (int iFormat);
  /**
   * Set the width and height.
   * This will also free the 'image' buffer to hold the pixel data,
   * but it will NOT allocate a new buffer (thus `image' is 0
   * after calling this function). You should pass an appropiate
   * pointer to one of ConvertXXX functions below, define the
   * image itself (or assign something to `Image' manually),
   * or call EnsureImage().
   */
  void SetDimensions (int newWidth, int newHeight);
  void SetDimensions (int newWidth, int newHeight, int newDepth);

  /// Allocate the pixel data buffers.
  void AllocImage();
  /// Allocate the pixel data buffers if they aren't already.
  void EnsureImage();
  /**
   * Free all image data: pixels and palette. Takes care of image data format.
   */
  void FreeImage ();
public:
  CS_LEAKGUARD_DECLARE (csImageMemory);

  /**
   * Create a blank image of these dimensions and the specified
   * format.
   * \param width Width of the image
   * \param height Height of the image
   * \param format Image format. Default: #CS_IMGFMT_TRUECOLOR
   */
  csImageMemory (int width, int height, int format = CS_IMGFMT_TRUECOLOR);
  /**
   * Create a blank image of these dimensions and the specified
   * format.
   * \param width Width of the image
   * \param height Height of the image
   * \param depth Height of the image
   * \param format Image format. 
   */
  csImageMemory (int width, int height, int depth, int format);
  /**
   * Create an instance for this pixel buffer with these dimensions. 
   * If destroy is set to true then the supplied buffer
   * will be destroyed when the instance is.
   * \param width Width of the image
   * \param height Height of the image
   * \param buffer Data containing initial data
   * \param destroy Destroy the buffer when the Image is destroyed
   * \param format Image format. Data in \arg buffer must be in this format.
   * Default: #CS_IMGFMT_TRUECOLOR
   * \param palette Palette for indexed images.
   */
  csImageMemory (int width, int height, void* buffer, bool destroy,
    int format = CS_IMGFMT_TRUECOLOR, csRGBpixel *palette = 0);
  /**
   * Create an instance from a pixel buffer with these dimensions. 
   * A copy of the pixel data is made.
   * \param width Width of the image
   * \param height Height of the image
   * \param buffer Data containing initial data
   * \param format Image format. Data in \arg buffer must be in this format.
   * Default: #CS_IMGFMT_TRUECOLOR
   * \param palette Palette for indexed images.
   */
  csImageMemory (int width, int height, const void* buffer, 
    int format = CS_IMGFMT_TRUECOLOR, const csRGBpixel *palette = 0);
  /**
   * Create an instance that copies the pixel data from another iImage
   * object.
   */
  csImageMemory (iImage* source);
  /**
   * Create an instance that copies the pixel data from another iImage
   * object, and also change the data format.
   */
  csImageMemory (iImage* source, int newFormat);

  virtual ~csImageMemory ();

  /// Get a pointer to the image data that can be changed.
  void* GetImagePtr ();
  /// Get a pointer to the palette data that can be changed.
  csRGBpixel* GetPalettePtr ();
  /// Get a pointer to the alpha data that can be changed.
  uint8* GetAlphaPtr ();

  virtual const void *GetImageData () { return GetImagePtr(); }
  virtual int GetWidth () const { return Width; }
  virtual int GetHeight () const { return Height; }
  virtual int GetDepth () const { return Depth; }

  virtual int GetFormat () const { return Format; }
  virtual const csRGBpixel* GetPalette () { return GetPalettePtr(); }
  virtual const uint8* GetAlpha () { return GetAlphaPtr(); }

  virtual bool HasKeyColor () const { return has_keycolour; }

  virtual void GetKeyColor (int &r, int &g, int &b) const
  { r = keycolour.red; g = keycolour.green; b = keycolour.blue; }

  /// Clears image to colour. Only works for truecolor images.
  void Clear (const csRGBpixel &colour);

  /// Check if all alpha values are "non-transparent" and if so, discard alpha
  void CheckAlpha ();
  /**
   * Convert the image to another format.
   * This method will allocate a respective color component if
   * it was not allocated before. For example, you can use this
   * method to add alpha channel to paletted images, to allocate
   * a image for CS_IMGFMT_NONE alphamaps or vice versa, to remove
   * the image and leave alphamap alone. This routine may be used
   * as well for removing alpha channel.
   */
  void SetFormat (int iFormat);

  /// Set the keycolor
  virtual void SetKeyColor (int r, int g, int b);
  virtual void SetKeycolor (int r, int g, int b) { SetKeyColor(r,g,b); }
  /// Remove the keycolor
  virtual void ClearKeyColor ();
  virtual void ClearKeycolor () { ClearKeyColor(); }

  /**
   * Apply the keycolor, that is, set all pixels which match the
   * keycolor to 0.
   */
  virtual void ApplyKeyColor ();
  virtual void ApplyKeycolor () { ApplyKeyColor(); }

  virtual csImageType GetImageType() const { return imageType; }
  void SetImageType (csImageType type) { imageType = type; }

  virtual uint HasMipmaps () const 
  { 
    size_t num = mipmaps.Length();
    while ((num > 0) && (mipmaps[num-1] == 0)) num--;
    return (uint)num; 
  }
  virtual csRef<iImage> GetMipmap (uint num) 
  { 
    if (num == 0) return this;
    if (num <= mipmaps.Length()) return mipmaps[num-1];
    return 0; 
  }
  /**
   * Set a mipmap of this image.
   * \remark You can set any mipmap except number 0 to your liking, however,
   *  no sanity checking is performed! That is, you can set mipmaps to 0,
   *  leave out mipmaps and provide more mipmaps than the image really needs -
   *  this can be problematic as most clients will expect non-null mipmaps. 
   *  (Note however that trailing 0 mipmaps will not be reported.)
   */
  bool SetMipmap (uint num, iImage* mip)
  {
    if (num == 0) return false;
    mipmaps.GetExtend (num-1) = mip;
    return true;
  }

  /// Copy an image as subpart into this image.
  bool Copy (iImage* srcImage, int x, int y, int width, int height);
  /**
   * Copy an image as subpart into this image with scaling \a sImage to the
   * given size.
   */
  bool CopyScale (iImage* srcImage, int x, int y, int width, int height);
  /**
   * Copy an image as subpart into this image and tile and scale \a sImage 
   * to the given size.
   */
  bool CopyTile (iImage* srcImage, int x, int y, int width, int height);

  /**
   * Used to convert an truecolor RGB image into requested format.
   * If the image loader cannot handle conversion itself, and the image
   * file is in a format that is different from the requested one,
   * load the image in csRGBpixel format and pass the pointer to this
   * function which will handle the RGB -> target format conversion.
   * NOTE: the pointer should be allocated with new csRGBpixel [] and you should
   * not free it after calling this function: the function will free
   * the buffer itself if it is appropriate (or wont if the buffer
   * size/contents are appropriate for target format).
   */
  void ConvertFromRGBA (csRGBpixel* iImage);
  /**
   * Used to convert an 8-bit indexed image into requested format.
   * Pass a pointer to color indices, a pointer to the alpha mask and a 
   * pointer to the palette, and you're done.
   * NOTE: the pointer should be allocated with new uint8 [] and you should
   * not free it after calling this function: the function will free
   * the buffer itself if it is appropriate (or wont if the buffer
   * size/contents are appropriate for target format). Same about palette
   * and alpha.
   */
  void ConvertFromPal8 (uint8* iImage, uint8* alpha, csRGBpixel* iPalette,
    int nPalColors = 256);
  /**
   * Used to convert an 8-bit indexed image into requested format.
   * Pass a pointer to color indices, a pointer to the alpha mask and a 
   * pointer to the palette, and you're done.
   * NOTE: the pointer should be allocated with new uint8 [] and you should
   * not free it after calling this function: the function will free
   * the buffer itself if it is appropriate (or wont if the buffer
   * size/contents are appropriate for target format). Same about palette
   * and alpha.
   */
  void ConvertFromPal8 (uint8* iImage, uint8* alpha, 
    const csRGBcolor* iPalette, int nPalColors = 256);
};

#endif // __CS_CSGFX_MEMIMAGE_H__
