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

#ifndef __CS_CSGFX_IMAGEVOLUMEMAKER_H__
#define __CS_CSGFX_IMAGEVOLUMEMAKER_H__

#include "csextern.h"
#include "csutil/leakguard.h"
#include "csutil/refarr.h"
#include "imagebase.h"

/**
 * Wrapper class to create a volume aka 3D texture from a number of 2D images
 * as the volume slices. 
 */
class CS_CSGFX_EXPORT csImageVolumeMaker : public csImageBase
{
protected:
  bool manualName;
  csRefArray<iImage> pendingImages;
  int Width;
  int Height;
  int Depth;
  int Format;

  void* data;
  csRGBpixel* palette;
  uint8* alpha;

  void AppendPending ();
public:
  SCF_DECLARE_IBASE;
  CS_LEAKGUARD_DECLARE (csImageVolumeMaker);

  /**
   * Create a new map without slices set.
   * Format, width, height are, unless specified, taken from the first image
   * added.
   */
  csImageVolumeMaker (int format = -1, int width = -1, int height = -1);
  /**
   * Create a new map and copy slices from \a source.
   */
  csImageVolumeMaker (iImage* source);
  virtual ~csImageVolumeMaker();

  virtual const void *GetImageData ();
  virtual int GetWidth () const { return Width > 0 ? Width : 0; }
  virtual int GetHeight () const { return Height > 0 ? Height : 0; ; }
  virtual int GetDepth () const { return Depth + (int)pendingImages.Length(); }
  /// Set the name of the image.
  virtual void SetName (const char *iName);
  /**
   * Get the name of the image.
   * \remarks Unless the name was manually overridden with SetName(), the
   *  default name will contain the names of the wrapped slices, separated
   *  by ':', in the form <tt>slice1.png:slice2.png:...</tt>.
   */
  virtual const char *GetName () const { return fName; }

  virtual int GetFormat () const;
  virtual const csRGBpixel* GetPalette ();
  virtual const uint8* GetAlpha ();

  virtual const char* GetRawFormat() const { return 0; }
  virtual csRef<iDataBuffer> GetRawData() const { return 0; }
  virtual csImageType GetImageType() const { return csimg3D; }
  
  void AddImage (iImage* source);
};

/** @} */

#endif // __CS_CSGFX_IMAGEVOLUMEMAKER_H__
