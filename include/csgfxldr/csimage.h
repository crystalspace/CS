/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Contributions made by Ivan Avramovic <ivan@avramovic.com>

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

#ifndef IMAGE_H
#define IMAGE_H

#include <stdio.h>
#include "csutil/scf.h"
#include "csgfxldr/rgbpixel.h"
#include "types.h"
#include "iimage.h"
#include "ialphmap.h"

/// Status flag indicating that the image has loaded without problem.
#define IFE_OK 0
/// Status flag indicating that the image data is in the wrong format.
#define IFE_BadFormat 1
/// Status flag indicating that the image data is corrupt.
#define IFE_Corrupt 2

struct Filter3x3;
struct Filter5x5;

class AlphaMapFile
{
// @@@ FIXME: None of this is implemented anywhere.
#if 0
private:
  int width;
  int height;
  UByte *alphamap;
protected:
  AlphaMapFile();
  int status;
  void set_dimensions(int w,int h);
  UByte *get_buffer(){return alphamap;}
public:
  virtual ~AlphaMapFile();
  int get_width() const{return width;}
  int get_height() const{return height;}
  ULong get_size() const{return width*height;}
  const UByte* get_image() const {return alphamap;}
  int get_status() const {return status;}
  virtual const char* get_status_mesg() const;
#endif
};

/**
 * An abstract class implementing an image loader. For every image
 * type supported, a subclass should be created for loading that image
 * type.
 */

class csImageFile : public iImageFile
{
private:
  /// Width of image.
  int width;
  /// Height of image.
  int height;
  /// The image data.
  RGBPixel* image;
  /// Image file name
  char *fname;

protected:
  /**
   * csImageFile constructor.
   * This object can only be created by an appropriate loader, which is why
   * the constructor is protected.
   */
  csImageFile ();

  /**
   * Status of the loaded image.
   * (status == IFE_OK) if the image loaded correctly.
   * (status & IFE_BadFormat) indicates that the image is in the wrong format.
   * (status & IFE_Corrupt) indicates that the image is in the correct format,
   * but the data is corrupt and unreadable.
   */
  int status;

  /**
   * Set the width and height.
   * This will also allocate the 'image' buffer to hold the bitmap.
   */
  void set_dimensions (int w, int h);

  /// Get the buffer in which to write image data.
  RGBPixel* get_buffer() { return image; }

public:
  DECLARE_IBASE;

  ///
  virtual ~csImageFile ();

  /// Returns the error status of the loaded image.
  int get_status() const { return status; }
  /// Returns a text message explaining the image status.
  virtual const char* get_status_mesg() const;

  /**
   * Create a new csImageFile which is a mipmapped version of this one.
   * 'steps' indicates how much the mipmap should be scaled down. Only
   * steps 1, 2, and 3 are supported.
   * If 'steps' is 1 then the 3x3 filter is used. Otherwise the 5x5 filter
   * is used. If the filters are NULL then the pixels are just
   * averaged.
   */
  csImageFile* mipmap (int steps, Filter3x3* filt1, Filter5x5* filt2);

  /**
   * Create a new csImageFile which is a mipmapped version of this one.
   * 'steps' indicates how much the mipmap should be scaled down. Only
   * steps 1, 2, and 3 are supported.
   * This version is required for transparent images. It preserves color
   * 0 (transparent).
   */
  csImageFile* mipmap (int steps);

  /**
   * Create a new csImageFile which is a blended version of this one.
   */
  csImageFile* blend (Filter3x3* filt1);

  /***************************** iImage interface *****************************/
  ///
  virtual RGBPixel *GetImageData ();
  ///
  virtual int GetWidth ();
  ///
  virtual int GetHeight ();
  ///
  virtual int GetSize ();

  /// Resize the image to the given size
  virtual iImageFile* Resize(int newwidth, int newheight);

  ///
  virtual iImageFile *MipMap (int steps, Filter3x3* filt1, Filter5x5* filt2);
  ///
  virtual iImageFile *MipMap (int steps);
  ///
  virtual iImageFile *Blend (Filter3x3* filter);
  /// Set image file name
  virtual void SetName (const char *iName);
  /// Get image file name
  virtual const char *GetName ();
};

class csVector;

/**
 * Extend this class to support a particular type of image loading.
 */
class csImageLoader
{
protected:
  /**
   * Load an image from the given buffer.
   * Attempts to read an image from the buffer 'buf' of length 'size'.
   * If successful, returns a pointer to the resulting csImageFile.  Otherwise
   * returns NULL.
   */
  virtual csImageFile* LoadImage (UByte* buf, ULong size) = 0;
  virtual AlphaMapFile* LoadAlphaMap(UByte* buf,ULong size) =0;

  ///
  virtual ~csImageLoader() {}

public:
  /**
   * Register a loader for a given image type.
   * Adds 'loader' to the list of image formats to be checked during an
   * csImageLoader::load(...) call.
   */
  static bool Register (csImageLoader* loader);

  /// Return the name of the image type supported by this loader.
  virtual const char* GetName() const = 0;

  /// Return a descriptive line about this image format.
  virtual const char* GetDescription() const = 0;

  /**
   * Load an image from a buffer.
   * This routine will read from the buffer buf of length size, try to
   * recognize the type of image contained within, and return an csImageFile
   * of the appropriate type.  Returns a pointer to the csImageFile on
   * success, or NULL on failure.
   */
  static csImageFile* load (UByte* buf, ULong size);

  /**
  * Load an alpha-map from an 8-bit image
  */
  static AlphaMapFile *load_alpha(UByte *buf,ULong size);

  /// A pointer to next image loader (loaders are chained in a list)
  csImageLoader *Next;
};

#endif
