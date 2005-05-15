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

#include "cssysdef.h"
#include "csutil/csendian.h"
#include "csgfx/imagemanipulate.h"
#include "ivaria/reporter.h"
#include "dds.h"
#include "ddsloader.h"
#include "ddsutil.h"
#include "ddssaver.h"

#define DDS_MIME "image/dds"

static iImageIO::FileFormatDescription formatlist[1] =
{
  {DDS_MIME, "RGBA", CS_IMAGEIO_LOAD | CS_IMAGEIO_SAVE}
};

csDDSImageIO::csDDSImageIO (iBase* parent)
{
  SCF_CONSTRUCT_IBASE(parent);
  int const formatcount =
    sizeof(formatlist)/sizeof(iImageIO::FileFormatDescription);
  for (int i=0; i < formatcount; i++)
    formats.Push (&formatlist[i]);
}

csDDSImageIO::~csDDSImageIO ()
{
  SCF_DESTRUCT_IBASE();
}

bool csDDSImageIO::Initialize (iObjectRegistry* objreg)
{
  object_reg = objreg;
  return true;
}

const csImageIOFileFormatDescriptions& csDDSImageIO::GetDescription ()
{
  return formats;
}

void csDDSImageIO::SetDithering (bool)
{
}

csDDSRawDataType csDDSImageIO::IdentifyPixelFormat (const dds::PixelFormat& pf, 
						    uint& bpp)
{
  csDDSRawDataType type = csrawUnsupported;
  if (pf.flags & dds::DDPF_FOURCC)
  {
    switch (pf.fourcc)
    {
      case MakeFourCC ('D','X','T','1'):
	type = csrawDXT1;
	bpp = 4; 
	break;
      case MakeFourCC ('D','X','T','2'):
	type = csrawDXT2;
	bpp = 8; 
	break;
      case MakeFourCC ('D','X','T','3'):
	type = csrawDXT3;
	bpp = 8; 
	break;
      case MakeFourCC ('D','X','T','4'):
	type = csrawDXT4;
	bpp = 8; 
	break;
      case MakeFourCC ('D','X','T','5'):
	type = csrawDXT5;
	bpp = 8; 
	break;
    }
  }
  else
  {
    type = (pf.flags & dds::DDPF_ALPHAPIXEL) ? csrawUnknownAlpha : csrawUnknown;
    bpp = pf.bitdepth;
    if (pf.bitdepth == 8)
    {
      if (pf.redmask == 0xff)
      {
	if ((pf.flags & dds::DDPF_LUMINANCE) 
	  && !(pf.flags & dds::DDPF_ALPHAPIXEL))
	  type = csrawLum8;
      }
    }
    else if (pf.bitdepth == 16)
    {
      if ((pf.redmask == 0xf800) && (pf.greenmask == 0x07e0) 
	&& (pf.bluemask == 0x001f))
      {
	if (!(pf.flags & dds::DDPF_ALPHAPIXEL))
	  type = csrawR5G6B5;
      }
    }
    else if (pf.bitdepth == 24)
    {
      if ((pf.redmask == 0x00ff0000) && (pf.greenmask == 0x0000ff00) 
	&& (pf.bluemask == 0x00000ff))
      {
	if (!(pf.flags & dds::DDPF_ALPHAPIXEL))
	  type = csrawB8G8R8;
      }
    }
    else if (pf.bitdepth == 32)
    {
      if ((pf.redmask == 0x00ff0000) && (pf.greenmask == 0x0000ff00) 
	&& (pf.bluemask == 0x00000ff))
      {
	if ((pf.flags & dds::DDPF_ALPHAPIXEL) && (pf.alphamask == 0xff000000))
	  type = csrawB8G8R8A8;
      }
    }
  }

  return type;
}

static size_t DataSize (csDDSRawDataType dataType, int bpp, int w, int h, 
			int d)
{
  switch (dataType)
  {
    case csrawDXT1:
    case csrawDXT1Alpha:
    case csrawDXT2:
    case csrawDXT3:
    case csrawDXT4:
    case csrawDXT5:
      {
	int minW = ((w + 3) / 4) * 4;
	int minH = ((h + 3) / 4) * 4;
	return (minW * minH * bpp) / 8;
      }
    default:
      return (w * h * d * bpp) / 8;
  }
}

csPtr<iImage> csDDSImageIO::Load (iDataBuffer* buf, int format)
{
  if (buf->GetSize() < sizeof (dds::Header))
    return 0;

  dds::Header head;
  CopyLEUI32s (&head, buf->GetData(), sizeof (head) / sizeof (uint32));
  if (head.magic != dds::Magic)
    return 0;

  const uint32 minimumflags = dds::DDSD_CAPS | dds::DDSD_HEIGHT | 
    dds::DDSD_WIDTH | dds::DDSD_PIXELFORMAT;
  if ((head.flags & minimumflags) != minimumflags)
    return 0;

  uint bpp = 0;
  csDDSRawDataType dataType = IdentifyPixelFormat (head.pixelformat,
    bpp);
  if (dataType == csrawUnsupported)
    return 0;

  bool cubeMap = false;
  bool volMap = false;
  if (head.capabilities.caps1 & dds::DDSCAPS_COMPLEX)
  {
    if (head.capabilities.caps2 & dds::DDSCAPS2_CUBEMAP)
      cubeMap = (head.capabilities.caps2 & (dds::DDSCAPS2_CUBEMAP_POSITIVEX
	| dds::DDSCAPS2_CUBEMAP_NEGATIVEX | dds::DDSCAPS2_CUBEMAP_POSITIVEY
	| dds::DDSCAPS2_CUBEMAP_NEGATIVEY | dds::DDSCAPS2_CUBEMAP_POSITIVEZ
	| dds::DDSCAPS2_CUBEMAP_NEGATIVEZ));
    else if (head.capabilities.caps2 & dds::DDSCAPS2_VOLUME)
      volMap = true;
  }

  uint Depth = (head.flags & dds::DDSD_DEPTH) ? head.depth : 1;
  size_t imgOffset = sizeof (dds::Header);
  size_t dataSize;

  if ((dataType == csrawDXT1) && 
    (dds::Loader::ProbeDXT1Alpha (buf->GetUint8() + imgOffset,
    head.width, head.height, Depth, 
    DataSize (dataType, bpp, head.width, head.height, Depth))))
    dataType = csrawDXT1Alpha;

  if ((dataType < csrawAlphaFirst) || (dataType > csrawAlphaLast))
    format &= ~CS_IMGFMT_ALPHA;
  else
    format |= CS_IMGFMT_ALPHA;
  if ((format & CS_IMGFMT_MASK) == CS_IMGFMT_ANY)
    format = (format & ~CS_IMGFMT_MASK) | CS_IMGFMT_TRUECOLOR;

  csRef<csDDSImageFile> mainImage;
  for (int f = cubeMap?6:1; f > 0; f--)
  {
    csRef<iDataBuffer> imgBuf;
    dataSize = DataSize (dataType, bpp, head.width, head.height, Depth);
    imgBuf.AttachNew (new csParasiticDataBuffer (buf, imgOffset, dataSize));
    imgOffset += dataSize;
    csRef<csDDSImageFile> Image;
    Image.AttachNew (new csDDSImageFile (object_reg, format, imgBuf,
      dataType, head.pixelformat));
    Image->SetDimensions (head.width, head.height, Depth);
    if (!mainImage.IsValid())
    {
      mainImage = Image;
      if (cubeMap) Image->imageType = csimgCube;
      else if (volMap) Image->imageType = csimg3D;
    }
    else
      mainImage->subImages.Push (Image);

    if ((head.flags & dds::DDSD_MIPMAPCOUNT) 
      && (head.capabilities.caps1 & dds::DDSCAPS_MIPMAP)
      && (head.mipmapcount != 0))
    {
      csRef<csDDSImageFile> mip;
      uint mipCount = head.mipmapcount - 1;
      uint w = head.width; uint h = head.height; uint d = Depth;
      while (mipCount-- > 0)
      {
	w = MAX (1, w >> 1); h = MAX (1, h >> 1); d = MAX (1, d >> 1);
	dataSize = DataSize (dataType, bpp, w, h, d);
	imgBuf.AttachNew (new csParasiticDataBuffer (buf, imgOffset, dataSize));
	imgOffset += dataSize;
	mip.AttachNew (new csDDSImageFile (object_reg, format, imgBuf,
	  dataType, head.pixelformat));
	mip->SetDimensions (w, h, d);
	if (volMap) mip->imageType = csimg3D;
	Image->mipmaps.Push (mip);
      }
    }
  }

  return csPtr<iImage> (mainImage);
}

csPtr<iDataBuffer> csDDSImageIO::Save (iImage* image,
    iImageIO::FileFormatDescription* format, const char* options)
{
  return Save (image, format->mime, options);
}

csPtr<iDataBuffer> csDDSImageIO::Save (iImage* image, const char* mime,
				       const char* options)
{
  if (strcmp (mime, DDS_MIME) != 0) return 0;
  csImageLoaderOptionsParser optparser (options);
  csDDSSaver saver;
  return saver.Save (image, optparser);
}

//---------------------------------------------------------------------------

csDDSImageFile::csDDSImageFile (iObjectRegistry* object_reg, int format, 
				iDataBuffer* sourceData, 
				csDDSRawDataType rawType, 
				const dds::PixelFormat& pixelFmt)
  : csImageMemory (format), mipmaps(0), imageType (csimg2D)
{
  csDDSImageFile::object_reg = object_reg;
  rawInfo = new RawInfo;
  rawInfo->rawData = sourceData;
  rawInfo->rawType = rawType;
  rawInfo->pixelFmt = pixelFmt;
}

csDDSImageFile::~csDDSImageFile ()
{
  delete rawInfo;
}

void csDDSImageFile::MakeImageData ()
{
  if (rawInfo != 0)
  {
    const uint8* source = rawInfo->rawData->GetUint8();
    const size_t dataSize = rawInfo->rawData->GetSize();
    csRGBpixel* buf = new csRGBpixel[Width * Height * Depth];
    switch (rawInfo->rawType)
    {
      case csrawDXT1:
      case csrawDXT1Alpha:
	{
	  dds::Loader::DecompressDXT1 (buf, source, Width, Height, 1,
	    dataSize);
	  break;
	}
      case csrawDXT2:
	{
	  dds::Loader::DecompressDXT2 (buf, source, Width, Height, 1,
	    dataSize);
	  break;
	}
      case csrawDXT3:
	{
	  dds::Loader::DecompressDXT3 (buf, source, Width, Height, 1,
	    dataSize);
	  break;
	}
      case csrawDXT4:
	{
	  dds::Loader::DecompressDXT4 (buf, source, Width, Height, 1,
	    dataSize);
	  break;
	}
      case csrawDXT5:
	{
	  dds::Loader::DecompressDXT5 (buf, source, Width, Height, 1,
	    dataSize);
	  break;
	}
      case csrawLum8:
	{
	  dds::Loader::DecompressLum (buf, source, Width, Height, Depth, 
	    dataSize, rawInfo->pixelFmt);
	  break;
	}
      default:
	{
	  if (rawInfo->pixelFmt.flags & dds::DDPF_ALPHAPIXEL)
	    dds::Loader::DecompressRGBA (buf, source, Width, Height, Depth,
	      dataSize, rawInfo->pixelFmt);
	  else
	    dds::Loader::DecompressRGB (buf, source, Width, Height, Depth,
	      dataSize, rawInfo->pixelFmt);
	}
    }

    ConvertFromRGBA (buf);
    if (Format & CS_IMGFMT_ALPHA) CheckAlpha();
    delete rawInfo; rawInfo = 0;
  }
}

const void* csDDSImageFile::GetImageData ()
{
  MakeImageData();
  return csImageMemory::GetImageData();
}

const csRGBpixel* csDDSImageFile::GetPalette ()
{
  if (!(Format & CS_IMGFMT_PALETTED8)) return 0;
  MakeImageData();
  return csImageMemory::GetPalette();
}

const uint8* csDDSImageFile::GetAlpha ()
{
  if ((Format & (CS_IMGFMT_PALETTED8 | CS_IMGFMT_ALPHA))
    != (CS_IMGFMT_PALETTED8 | CS_IMGFMT_ALPHA)) return 0;
  MakeImageData();
  return csImageMemory::GetAlpha();
}

uint csDDSImageFile::HasMipmaps () const
{
  return (uint)mipmaps.Length();
}

csRef<iImage> csDDSImageFile::GetMipmap (uint num)
{
  if (num == 0)
    return this;
  if (num > mipmaps.Length())
    return 0;

  return mipmaps[num-1];
}

uint csDDSImageFile::HasSubImages() const 
{ 
  return (uint)subImages.Length();
}

csRef<iImage> csDDSImageFile::GetSubImage (uint num) 
{
  if (num == 0)
    return this;
  if (num > subImages.Length())
    return 0;

  return subImages[num-1];
}

static const char* RawTypeString (csDDSRawDataType type)
{
  switch (type)
  {
    case csrawDXT1:   
    case csrawDXT1Alpha:
      return "dxt1";
    case csrawDXT2:
      return "dxt2";
    case csrawDXT3:
      return "dxt3";
    case csrawDXT4:
      return "dxt4";
    case csrawDXT5:
      return "dxt5";
    case csrawB8G8R8:
      return "b8g8r8";
    case csrawR5G6B5:
      return "r5g6b5";
    case csrawB8G8R8A8:
      return "b8g8r8a8";
    case csrawLum8:
      return "l8";
    default:
      return 0;
  }
}

const char* csDDSImageFile::GetRawFormat() const
{
  return rawInfo ? (RawTypeString (rawInfo->rawType)) : 0;
}

csRef<iDataBuffer> csDDSImageFile::GetRawData() const
{
  return rawInfo ? (rawInfo->rawData) : 0;
}

void csDDSImageFile::Report (int severity, const char* msg, ...)
{
  va_list argv;
  va_start (argv, msg);
  csReportV (object_reg, severity, "crystalspace.graphic.image.io.dds", msg, 
    argv);
  va_end (argv);
}

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csDDSImageIO)
  SCF_IMPLEMENTS_INTERFACE (iImageIO)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY(csDDSImageIO);
