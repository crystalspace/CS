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

#ifndef BMPIMAGE_H
#define BMPIMAGE_H

#include "csgfx/csimage.h"
#include "igraphic/imageio.h"
#include "isys/plugin.h"
#include "iutil/databuff.h"
#include "csutil/csvector.h"

/**
 * The BMP image file format loader.
 */
class csBMPImageIO : public iImageIO
{
 protected:
  csVector formats;

 public:
  SCF_DECLARE_IBASE;

  csBMPImageIO (iBase *pParent);
  virtual ~csBMPImageIO () {}

  virtual const csVector& GetDescription ();
  virtual iImage *Load (UByte* iBuffer, ULong iSize, int iFormat);
  virtual void SetDithering (bool iEnable);
  virtual iDataBuffer *Save (iImage *image, const char *mime = NULL); 
  virtual iDataBuffer *Save (iImage *image, iImageIO::FileFormatDescription *format = NULL);

  struct eiPlugIn : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csBMPImageIO);
    virtual bool Initialize (iSystem*) { return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
};

/**
 * An csImageFile subclass for reading BMP files.
 *<p>
 * Current Limitations:
 * Only able to read 24 bits RGB encoded with no palette files and
 * 8 bits RGB encoded files.
 */
class ImageBMPFile : public csImageFile
{
  friend class csBMPImageIO;
  // Load a Windows-format BMP file
  bool LoadWindowsBitmap (UByte* iBuffer, ULong iSize);

private:
  /// Initialize the image object
  ImageBMPFile (int iFormat) : csImageFile (iFormat) { };
  /// Try to read the BMP file from the buffer and return success status
  bool Load (UByte* iBuffer, ULong iSize);
};

#endif
