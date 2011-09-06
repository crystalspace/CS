/*
    DSS image file format support for CrystalSpace 3D library
    Copyright (C) 2003 by Matze Braun <matze@braunis.de>

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

#ifndef __DDS_DDSLOADER_H__
#define __DDS_DDSLOADER_H__

#include "csutil/scf.h"
#include "csgfx/imagememory.h"
#include "csutil/parasiticdatabuffer.h"
#include "csutil/refarr.h"
#include "iutil/comp.h"
#include "igraphic/imageio.h"

#include "dds.h"

struct iObjectRegistry;

namespace CS
{
  namespace Graphics
  {
    struct iDXTDecompressor;
  }
}

CS_PLUGIN_NAMESPACE_BEGIN(DDSImageIO)
{

enum csDDSRawDataType
{
  csrawUnknown,
  csrawDXT1,
  csrawR8G8B8,
  csrawR5G6B5,
  csrawLum8,

  csrawUnknownAlpha,
  csrawDXT1Alpha,
  csrawDXT2,
  csrawDXT3,
  csrawDXT4,
  csrawDXT5,
  csrawA8R8G8B8,

  csrawUnsupported,

  csrawAlphaFirst = csrawUnknownAlpha,
  csrawAlphaLast = csrawA8R8G8B8
};

class csDDSImageIO : 
  public scfImplementation2<csDDSImageIO,
                            iImageIO,
                            iComponent>
{
public:
  csDDSImageIO (iBase* parent);
  virtual ~csDDSImageIO ();

  virtual const csImageIOFileFormatDescriptions& GetDescription ();
  virtual csPtr<iImage> Load (iDataBuffer* buf, int format);
  virtual csPtr<iDataBuffer> Save (iImage* image, const char* mime,
				   const char* options);
  virtual csPtr<iDataBuffer> Save (iImage* image,
      iImageIO::FileFormatDescription* format, const char* options);
  
  virtual bool Initialize (iObjectRegistry* objreg);

  iObjectRegistry* GetObjectReg () const { return object_reg; }
  CS::Graphics::iDXTDecompressor* GetDXTDecompressor ();
private:
  csImageIOFileFormatDescriptions formats;
  iObjectRegistry* object_reg;
  csRef<CS::Graphics::iDXTDecompressor> dxt_decompress;

  csDDSRawDataType IdentifyPixelFormat (const dds::PixelFormat& pf, 
    uint32 dxgiFormat, bool isDX10, uint& bpp);
};

class csDDSImageFile : public csImageMemory
{
  friend class csDDSImageIO;
public:
  virtual ~csDDSImageFile ();

  virtual const void *GetImageData ();
  virtual const csRGBpixel* GetPalette ();
  virtual const uint8* GetAlpha ();

  virtual uint HasMipmaps () const;  
  virtual csRef<iImage> GetMipmap (uint num);

  virtual uint HasSubImages() const;
  virtual csRef<iImage> GetSubImage (uint num);

  virtual const char* GetRawFormat() const;
  virtual csRef<iDataBuffer> GetRawData() const;
  virtual csImageType GetImageType() const { return imageType; }
private:
  csDDSImageFile (csDDSImageIO* iio,
    int format, iDataBuffer* sourceData, csDDSRawDataType rawType, 
    const dds::PixelFormat& pixelFmt);

  struct RawInfo
  {
    csRef<iDataBuffer> rawData;
    csDDSRawDataType rawType;
    dds::PixelFormat pixelFmt;
  };
  RawInfo* rawInfo;
  void MakeImageData ();

  csRefArray<iImage> mipmaps;
  csRefArray<iImage> subImages;
  csRef<csDDSImageIO> iio;
  csImageType imageType;

  void Report (int severity, const char* msg, ...);
};

}
CS_PLUGIN_NAMESPACE_END(DDSImageIO)

#endif // __DDS_DDSLOADER_H__

