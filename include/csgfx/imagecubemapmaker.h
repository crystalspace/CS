/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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

/**\file
 */
 
/**\addtogroup gfx
 * @{ 
 */

#ifndef __CS_CSGFX_IMAGECUBEMAPMAKER_H__
#define __CS_CSGFX_IMAGECUBEMAPMAKER_H__

#include "csextern.h"
#include "csutil/leakguard.h"
#include "imagebase.h"

/**
 * Wrapper class to create a cube map from a number of 2D images as the cube
 * faces. Ensures that all faces are available when requested, if necessary
 * by creating a new image (the famous and popular image-no-found-checkerboard).
 */
class CS_CRYSTALSPACE_EXPORT csImageCubeMapMaker : public csImageBase
{
protected:
  csRef<iImage> cubeImages[6];
  bool manualName;

  void CheckImage (int index);
  void UpdateName ();
public:
  SCF_DECLARE_IBASE;
  CS_LEAKGUARD_DECLARE (csImageCubeMapMaker);

  /// Create a new map without faces set.
  csImageCubeMapMaker();
  /// Create a new map and copy the faces from \a source.
  csImageCubeMapMaker (iImage* source);
  /**
   * Create a new map from separately specified the Positive X, Positive Y ... 
   * images.
   */
  csImageCubeMapMaker (iImage* posX, iImage* negX, iImage* posY, 
    iImage* negY, iImage* posZ, iImage* negZ);

  virtual ~csImageCubeMapMaker();

  virtual const void *GetImageData ();
  virtual int GetWidth () const;
  virtual int GetHeight () const;
  /// Set the name of the image.
  virtual void SetName (const char *iName);
  /**
   * Get the name of the image.
   * \remarks Unless the name was manually overridden with SetName(), the
   *  default name will contain the names of the wrapped subimages, separated
   *  by ':', in the form 
   *  <tt>posx.png:negx.png:posy.png:negy.png:posz.png:negz.png</tt>.
   */
  virtual const char *GetName () const { return fName; }

  virtual int GetFormat () const;
  virtual const csRGBpixel* GetPalette ();
  virtual const uint8* GetAlpha ();

  virtual bool HasKeyColor () const { return false; }
  virtual void GetKeyColor (int &r, int &g, int &b) const { }

  virtual uint HasMipmaps () const;
  virtual csRef<iImage> GetMipmap (uint num);

  virtual const char* GetRawFormat() const;
  virtual csRef<iDataBuffer> GetRawData() const;
  virtual csImageType GetImageType() const { return csimgCube; }
  virtual uint HasSubImages() const { return 5; }
  virtual csRef<iImage> GetSubImage (uint num);
  
  /// Set a specific face.
  void SetSubImage (uint num, iImage* image);
  /**
   * Check whether a face is specified.
   * The difference from GetSubImage(num) is that GetSubImage() will
   * always return an image != 0, while SubImageSet() checks whether
   * the internal face reference is 0 or not.
   */
  bool SubImageSet (uint num) 
  { return ((num >= 0) && (num <= 5) && (cubeImages[num].IsValid())); }
};

/** @} */

#endif // __CS_CSGFX_IMAGECUBEMAPMAKER_H__
