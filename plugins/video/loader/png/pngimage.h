/*
    Copyright (C) 1998,2000 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CS_PNGIMAGE_H__
#define __CS_PNGIMAGE_H__

#include "csgfx/memimage.h"
#include "igraphic/imageio.h"
#include "csutil/leakguard.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/databuff.h"
#include "csplugincommon/imageloader/commonimagefile.h"

/**
 * The PNG image file format loader.
 */
class csPNGImageIO : public iImageIO
{
protected:
  csImageIOFileFormatDescriptions formats;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  csPNGImageIO (iBase *pParent);
  virtual ~csPNGImageIO ();

  virtual const csImageIOFileFormatDescriptions& GetDescription ();
  virtual csPtr<iImage> Load (iDataBuffer* buf, int iFormat);
  virtual void SetDithering (bool iEnable);
  virtual csPtr<iDataBuffer> Save (iImage *image, const char *mime = 0,
    const char* extraoptions = 0);
  virtual csPtr<iDataBuffer> Save (iImage *image,
      iImageIO::FileFormatDescription *format = 0,
    const char* extraoptions = 0);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csPNGImageIO);
    virtual bool Initialize (iObjectRegistry* p) 
    { scfParent->object_reg = p; return true; }
  } scfiComponent;
  friend class eiComponent;
};

/**
 * An csImageFile subclass for reading PNG files.<p>
 * This implementation needs both zlib and pnglib to read .PNG files.
 */
class ImagePngFile : public csCommonImageFile
{
  friend class csPNGImageIO;
private:
  class PngLoader : public csCommonImageFileLoader
  {
    struct ImagePngRawData
    {
      // The buffer to "read" from
      uint8 *r_data;
      // The buffer size
      size_t r_size;
    };

    png_structp png;
    png_infop info;
    int bit_depth, color_type;
    enum { imgRGB, imgPAL, imgGrayAlpha } ImageType;
    int keycolor_index;
    csRef<iDataBuffer> dataSource;
    ImagePngRawData raw;

    static void ImagePngRead (png_structp png, png_bytep data, 
      png_size_t size);
  public:
    PngLoader (int Format, iDataBuffer* source) 
      : csCommonImageFileLoader (Format), png (0), info (0), 
      keycolor_index (-1), dataSource (source) {}
    virtual ~PngLoader();
    bool InitOk();
    /// Try to read the PNG file from the buffer and return success status
    virtual bool LoadData ();
  };
  friend class PngLoader;

  /// Initialize the image object
  ImagePngFile (iObjectRegistry* object_reg, int iFormat) 
    : csCommonImageFile (object_reg, iFormat) { };
  virtual csRef<iImageFileLoader> InitLoader (csRef<iDataBuffer> source);
public:
  CS_LEAKGUARD_DECLARE (ImagePngFile);
};

#endif // __CS_PNGIMAGE_H__
