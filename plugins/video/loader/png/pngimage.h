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

#ifndef PNGIMAGE_H
#define PNGIMAGE_H

#include "csgfx/csimage.h"
#include "igraphic/imageio.h"
#include "isys/plugin.h"
#include "iutil/databuff.h"
#include "csutil/csvector.h"

/**
 * The PNG image file format loader.
 */
class csPNGImageIO : public iImageIO
{
 protected:
  csVector formats;

 public:
  SCF_DECLARE_IBASE;

  csPNGImageIO (iBase *pParent);
  virtual ~csPNGImageIO (){}

  virtual const csVector& GetDescription ();
  virtual iImage *Load (UByte* iBuffer, ULong iSize, int iFormat);
  virtual void SetDithering (bool iEnable);
  virtual iDataBuffer *Save (iImage *image, const char *mime = NULL); 
  virtual iDataBuffer *Save (iImage *image, iImageIO::FileFormatDescription *format = NULL);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csPNGImageIO);
    virtual bool Initialize (iObjectRegistry*) { return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
};

/**
 * An csImageFile subclass for reading PNG files.<p>
 * This implementation needs both zlib and pnglib to read .PNG files.
 */
class ImagePngFile : public csImageFile
{
  friend class csPNGImageIO;
private:
  /// Initialize the image object
  ImagePngFile (int iFormat) : csImageFile (iFormat) { };
  /// Try to read the PNG file from the buffer and return success status
  bool Load (UByte* iBuffer, ULong iSize);
};

#endif // PNGIMAGE_H
