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
  struct LEUI16
  {
    uint8 lo, hi;
    operator uint16() const { return hi * 256 + lo; }
    const LEUI16& operator = (uint16 x) 
    { lo = x % 256; hi = x / 256; return *this; }
  };

  uint8 IDLength;		/* length of Identifier String */
  uint8 CoMapType;		/* 0 = no map */
  uint8 ImgType;		/* image type (see below for values) */
  LEUI16 Index;			/* index of first color map entry */
  LEUI16 Length;		/* number of entries in color map */
  uint8 CoSize;			/* size of color map entry (15,16,24,32) */
  LEUI16 X_org;			/* x origin of image */
  LEUI16 Y_org;			/* y origin of image */
  LEUI16 Width;			/* width of image */
  LEUI16 Height;		/* height of image */
  uint8 PixelSize;		/* pixel size (8,16,24,32) */
  /* 
    bits 7-6, interleaving flag
    bit  5, origin: 0=lower left, 1=upper left
    bit  4, reserved
    bits 3-0, number of attribute bits per pixel 
   */
  uint8 flags;
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
    struct TgaPix
    {
      int Red, Grn, Blu, Alpha;
      uint l;
    };

    csRef<iDataBuffer> dataSource;
    uint8* iBuffer;
    TGAheader tga_head;
    bool mapped, rlencoded;
    int RLE_count, RLE_flag;
    TgaPix currentPixel;
    csRGBpixel* colorMap;
    uint colorMapSize;
    uint indexShift;

    void readtga (uint8*& ptr, struct TGAheader* tgaP);
    void get_map_entry (uint8*& ptr, csRGBpixel* Value, int Size, bool alpha);
    void get_current_pixel (uint8*& ptr, int Size, bool alpha);
    void get_pixel (uint8*& ptr, csRGBpixel* dest, int Size, bool alpha);
    void get_pixel (uint8*& ptr, uint8* dest, int Size, bool alpha);
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
