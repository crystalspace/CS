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

#ifndef JNG_H
#define JNG_H

#include "csgfx/csimage.h"
#include "igraphic/imageio.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/databuff.h"
#include "csutil/csvector.h"
#include "csutil/memfile.h"

/**
 * The JNG image file format loader.
 */
class csJNGImageIO : public iImageIO
{
 protected:
  csVector formats;
  iObjectRegistry* object_reg;

 private:
  csMemFile *outfile;
  iImage *imgRGBA;

  /// write something to our output stream
  static mng_bool MNG_DECL cb_writedata (mng_handle hHandle, mng_ptr pBuf,
                                         mng_uint32 iBuflen, mng_uint32p pWritten);
  /// libmng wants a line in the buffer
  static mng_ptr MNG_DECL cb_getcanvasline(mng_handle hHandle, mng_uint32 iLinenr);

 public:
  SCF_DECLARE_IBASE;

  csJNGImageIO (iBase *pParent);
  virtual ~csJNGImageIO (){}

  virtual const csVector& GetDescription ();
  virtual csPtr<iImage> Load (uint8* iBuffer, uint32 iSize, int iFormat);
  virtual void SetDithering (bool iEnable);
  virtual iDataBuffer *Save (iImage *image, const char *mime = NULL,
    const char* extraoptions = NULL);
  virtual iDataBuffer *Save (iImage *image, iImageIO::FileFormatDescription *format = NULL,
    const char* extraoptions = NULL);

  virtual bool Initialize (iObjectRegistry* p) 
    { object_reg = p; return true; }

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csJNGImageIO);
    virtual bool Initialize (iObjectRegistry* p) 
      { return scfParent->Initialize(p); }
  } scfiComponent;
};

/**
 * An csImageFile subclass for reading JNG files.<p>
 * This implementation needs libmng to read .JNG files.
 */
class ImageJngFile : public csImageFile
{
  friend class csJNGImageIO;
private:
  uint8  *buffer, *bufptr;
  uint32 bufferSize;
  
  csRGBpixel *NewImage;
  iObjectRegistry* object_reg;

  /// stream read callback for libmng
  static mng_bool MNG_DECL cb_readdata(mng_handle hHandle, mng_ptr pBuf,
			               mng_uint32 iBuflen, mng_uint32 *pRead);
  /// libmng tells us width/height through this
  static mng_bool MNG_DECL cb_processheader(mng_handle hHandle, 
					    mng_uint32 iWidth, mng_uint32 iHeight);
  /// libmng wants a line in the buffer
  static mng_ptr MNG_DECL cb_getcanvasline(mng_handle hHandle, mng_uint32 iLinenr);
  /// libmng tells us that an area of the image has updated
  static mng_bool MNG_DECL cb_imagerefresh(mng_handle hHandle, mng_uint32 iX, 
					   mng_uint32 iY, mng_uint32 iWidth, 
					   mng_uint32 iHeight);
  /// libmng wants to know the time...
  static mng_uint32 MNG_DECL cb_gettickcount (mng_handle hHandle);
  /// libmng wants us to set up a timer
  static mng_bool MNG_DECL cb_settimer (mng_handle hHandle, mng_uint32 iMsecs);

  /// Initialize the image object
  ImageJngFile (int iFormat, iObjectRegistry* p) : csImageFile (iFormat) 
    { object_reg = p; };
  /// Try to read the JNG file from the buffer and return success status
  bool Load (uint8* iBuffer, uint32 iSize);
};

#endif // JNG_H
