/*
    Copyright (C) 1998 by Tor Andersson

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

#ifndef __CS_JPGIMAGE_H__
#define __CS_JPGIMAGE_H__

#include "csgfx/memimage.h"
#include "igraphic/imageio.h"
#include "csutil/leakguard.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/databuff.h"
#include "csplugincommon/imageloader/commonimagefile.h"

extern "C"
{
#define jpeg_boolean boolean
#define JDCT_DEFAULT JDCT_FLOAT	// use floating-point for decompression
#define INT32 JPEG_INT32
#include <jpeglib.h>
#include <jerror.h>
#undef INT32
}

/**
 * The JPG image file format loader.
 */
class csJPGImageIO : public iImageIO
{
protected:
  csImageIOFileFormatDescriptions formats;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  csJPGImageIO (iBase *pParent);
  virtual ~csJPGImageIO ();

  virtual const csImageIOFileFormatDescriptions& GetDescription ();
  virtual csPtr<iImage> Load (iDataBuffer* buf, int iFormat);
  virtual void SetDithering (bool iEnable);
  virtual csPtr<iDataBuffer> Save (iImage *image, const char *mime = 0,
    const char* extraoptions = 0);
  virtual csPtr<iDataBuffer> Save (iImage *image,
      	iImageIO::FileFormatDescription *format = 0,
    const char* extraoptions = 0);

  bool Initialize (iObjectRegistry* p) 
  { object_reg = p; return true; }

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csJPGImageIO);
    virtual bool Initialize (iObjectRegistry* p) 
      { return scfParent->Initialize(p); }
  } scfiComponent;
};

/* ==== Error mgmnt ==== */
struct my_error_mgr
{
  struct jpeg_error_mgr pub;	/* "public" fields */
  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr *my_error_ptr;

/**
 * An csImageFile subclass for reading JPG files.<p>
 * This implementation needs libjpeg to read JFIF files.
 */
class ImageJpgFile : public csCommonImageFile
{
  friend class csJPGImageIO;
  static bool dither;

private:

  class JpegLoader : public csCommonImageFileLoader
  {
    iObjectRegistry* object_reg;
    csRef<iDataBuffer> dataSource;

    struct my_error_mgr jerr;
    struct jpeg_decompress_struct cinfo;
    bool decompCreated;
  public:
    JpegLoader (int Format, iObjectRegistry* p, iDataBuffer* source) 
      : csCommonImageFileLoader (Format), dataSource (source),
      decompCreated (false)
    { object_reg = p; };
    virtual ~JpegLoader();
    bool InitOk();
    virtual bool LoadData ();
  };
  friend class JpegLoader;

  /// Initialize the image object
  ImageJpgFile (iObjectRegistry* p, int iFormat) 
    : csCommonImageFile (p, iFormat) { };
  /// Create a JpegLoader for this image
  virtual csRef<iImageFileLoader> InitLoader (csRef<iDataBuffer> source);
public:
  CS_LEAKGUARD_DECLARE (ImageJpgFile);
};

#endif // __CS_JPGIMAGE_H__
