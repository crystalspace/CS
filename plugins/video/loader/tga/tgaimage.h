/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#ifndef __CS_TGAIMAGE_H__
#define __CS_TGAIMAGE_H__

#include "csgfx/memimage.h"
#include "igraphic/imageio.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/databuff.h"
#include "csplugincommon/imageloader/commonimagefile.h"

/**
 * The TGA image file format loader.
 */
class csTGAImageIO : public iImageIO
{
protected:
  csImageIOFileFormatDescriptions formats;
  iObjectRegistry* object_reg;

public:
  SCF_DECLARE_IBASE;

  csTGAImageIO (iBase *pParent);
  virtual ~csTGAImageIO ();

  virtual const csImageIOFileFormatDescriptions& GetDescription ();
  virtual csPtr<iImage> Load (iDataBuffer* buf, int iFormat);
  virtual void SetDithering (bool iEnable);
  virtual csPtr<iDataBuffer> Save (iImage *image, const char *mime = 0,
    const char* extraoptions = 0);
  virtual csPtr<iDataBuffer> Save (iImage *image, iImageIO::FileFormatDescription *format = 0,
    const char* extraoptions = 0);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csTGAImageIO);
    virtual bool Initialize (iObjectRegistry* p) 
    { scfParent->object_reg = p; return true; }
  } scfiComponent;
  friend class eiComponent;
};

/* Header definition. */
struct TGAheader
{
  unsigned char IDLength;		/* length of Identifier String */
  unsigned char CoMapType;		/* 0 = no map */
  unsigned char ImgType;		/* image type (see below for values) */
  unsigned char Index_lo, Index_hi;	/* index of first color map entry */
  unsigned char Length_lo, Length_hi;	/* number of entries in color map */
  unsigned char CoSize;			/* size of color map entry (15,16,24,32) */
  unsigned char X_org_lo, X_org_hi;	/* x origin of image */
  unsigned char Y_org_lo, Y_org_hi;	/* y origin of image */
  unsigned char Width_lo, Width_hi;	/* width of image */
  unsigned char Height_lo, Height_hi;	/* height of image */
  unsigned char PixelSize;		/* pixel size (8,16,24,32) */
  /* 
    bits 7-6, interleaving flag
    bit  5, origin: 0=lower left, 1=upper left
    bit  4, reserved
    bits 3-0, number of attribute bits per pixel 
   */
  unsigned char flags;
};

/**
 * An csCommonImageFile subclass for reading TGA files.
 */
class ImageTgaFile : public csCommonImageFile
{
  friend class csTGAImageIO;

private:
  class TgaLoader : public csCommonImageFileLoader
  {
    csRef<iDataBuffer> dataSource;
    uint8* iBuffer;
    TGAheader tga_head;
    bool mapped, rlencoded;
    int RLE_count, RLE_flag;
    int Red, Grn, Blu, Alpha;
    csRGBpixel* colorMap;

    void readtga (uint8*& ptr, struct TGAheader* tgaP);
    void get_map_entry (uint8*& ptr, csRGBpixel* Value, int Size, bool alpha);
    void get_pixel (uint8*& ptr, csRGBpixel* dest, int Size, bool alpha);
  public:
    TgaLoader (int Format, iDataBuffer* source) 
      : csCommonImageFileLoader (Format), dataSource (source), 
	RLE_count(0), RLE_flag (0), colorMap (0) {}
    virtual ~TgaLoader();
    bool InitOk();
    virtual bool LoadData ();
  };
  friend class TgaLoader;

  /// Initialize the image object
  ImageTgaFile (iObjectRegistry* object_reg, int iFormat) 
    : csCommonImageFile (object_reg, iFormat) { };
  /// Try to read the TGA file from the buffer and return success status
  virtual csRef<iImageFileLoader> InitLoader (csRef<iDataBuffer> source);
};

#endif // __CS_TGAIMAGE_H__
