/*
    BMPImage class
    Copyright (C) 1998 by Olivier Langlois <olanglois@sympatico.ca>

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

#ifndef __CS_BMPIMAGE_H__
#define __CS_BMPIMAGE_H__

#include "csgfx/memimage.h"
#include "igraphic/imageio.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/databuff.h"

/**
 * The BMP image file format loader.
 */
class csBMPImageIO : public iImageIO
{
 protected:
  csImageIOFileFormatDescriptions formats;

 public:
  SCF_DECLARE_IBASE;

  csBMPImageIO (iBase *pParent);
  virtual ~csBMPImageIO ();

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
    SCF_DECLARE_EMBEDDED_IBASE(csBMPImageIO);
    virtual bool Initialize (iObjectRegistry*) { return true; }
  } scfiComponent;
};

/**
 * An csImageFile subclass for reading BMP files.
 *<p>
 * Current Limitations:
 * Only able to read 24 bits RGB encoded with no palette files and
 * 8 bits RGB encoded files.
 */
class ImageBMPFile : public csImageMemory
{
  friend class csBMPImageIO;
  // Load a Windows-format BMP file
  bool LoadWindowsBitmap (uint8* iBuffer, size_t iSize);

private:
  /// Initialize the image object
  ImageBMPFile (int iFormat) : csImageMemory (iFormat) { };
  /// Try to read the BMP file from the buffer and return success status
  bool Load (uint8* iBuffer, size_t iSize);
};

#endif // __CS_BMPIMAGE_H__
