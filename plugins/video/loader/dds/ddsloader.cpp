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

#include "igraphic/dxtcompress.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"

#include "dds.h"
#include "ddsloader.h"
#include "ddsutil.h"
#include "ddssaver.h"

#define DDS_MIME "image/dds"

CS_PLUGIN_NAMESPACE_BEGIN(DDSImageIO)
{

SCF_IMPLEMENT_FACTORY(csDDSImageIO)

static iImageIO::FileFormatDescription formatlist[] =
{
  {DDS_MIME, "RGBA", CS_IMAGEIO_LOAD | CS_IMAGEIO_SAVE}
};

csDDSImageIO::csDDSImageIO (iBase* parent) : scfImplementationType (this, parent)
{
  int const formatcount =
    sizeof(formatlist)/sizeof(iImageIO::FileFormatDescription);
  for (int i=0; i < formatcount; i++)
    formats.Push (&formatlist[i]);
}

csDDSImageIO::~csDDSImageIO ()
{
}

bool csDDSImageIO::Initialize (iObjectRegistry* objreg)
{
  object_reg = objreg;
  return true;
}

CS::Graphics::iDXTDecompressor* csDDSImageIO::GetDXTDecompressor ()
{
  if (!dxt_decompress)
    dxt_decompress = csQueryRegistryOrLoad<CS::Graphics::iDXTDecompressor> (object_reg,
                                                                            CS_DXTDECOMPRESSOR_DEFAULT);

  return dxt_decompress;
}

const csImageIOFileFormatDescriptions& csDDSImageIO::GetDescription ()
{
  return formats;
}

csDDSRawDataType csDDSImageIO::IdentifyPixelFormat (const dds::PixelFormat& pf, 
                                                    uint32 dxgiFormat, bool isDX10,
                                                    uint& bpp)
{
  csDDSRawDataType type = csrawUnsupported;
  if(isDX10)
  {
    switch(dxgiFormat)
    {
    case 71:
      {
        if(pf.flags & dds::DDPF_ALPHAPIXEL)
        {
          type = csrawDXT1Alpha;
        }
        else
        {
          type = csrawDXT1;
        }
        bpp = 4;
        break;
      }
    case 74:
      {
        type = csrawDXT3;
        bpp = 8;
        break;
      }
    case 77:
      {
        type = csrawDXT5;
        bpp = 8; 
        break;
      }
    }
  }
  else if (pf.flags & dds::DDPF_FOURCC)
  {
    switch (pf.fourcc)
    {
    case MakeFourCC ('D','X','T','1'):
      type = csrawDXT1Alpha;
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
          type = csrawR8G8B8;
      }
    }
    else if (pf.bitdepth == 32)
    {
      if ((pf.redmask == 0x00ff0000) && (pf.greenmask == 0x0000ff00) 
        && (pf.bluemask == 0x00000ff))
      {
        if ((pf.flags & dds::DDPF_ALPHAPIXEL) && (pf.alphamask == 0xff000000))
          type = csrawA8R8G8B8;
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

  // Check for DX10 format.
  bool isDX10 = false;
  dds::Header10 head10;
  if (head.pixelformat.fourcc == MakeFourCC('D', 'X', '1', '0'))
  {
    isDX10 = true;
    CopyLEUI32s (&head10, buf->GetData()+sizeof (head), sizeof (head10) / sizeof (uint32));
  }

  const uint32 minimumflags = dds::DDSD_CAPS | dds::DDSD_HEIGHT | 
    dds::DDSD_WIDTH | dds::DDSD_PIXELFORMAT;
  if ((head.flags & minimumflags) != minimumflags)
    return 0;

  uint bpp = 0;
  csDDSRawDataType dataType = IdentifyPixelFormat (head.pixelformat,
  head10.dxgiFormat, isDX10, bpp);
  if (dataType == csrawUnsupported)
    return 0;

  bool cubeMap = false;
  bool volMap = false;
  if (head.capabilities.caps2 & dds::DDSCAPS2_CUBEMAP)
    cubeMap = (head.capabilities.caps2 & (dds::DDSCAPS2_CUBEMAP_POSITIVEX
      | dds::DDSCAPS2_CUBEMAP_NEGATIVEX | dds::DDSCAPS2_CUBEMAP_POSITIVEY
      | dds::DDSCAPS2_CUBEMAP_NEGATIVEY | dds::DDSCAPS2_CUBEMAP_POSITIVEZ
      | dds::DDSCAPS2_CUBEMAP_NEGATIVEZ)) != 0;
  else if (head.capabilities.caps2 & dds::DDSCAPS2_VOLUME)
    volMap = true;

  uint Depth = (head.flags & dds::DDSD_DEPTH) ? head.depth : 1;
  size_t imgOffset = sizeof (dds::Header);
  size_t dataSize;

  if (!isDX10 && (dataType == csrawDXT1Alpha) && 
    (dds::Loader::ProbeDXT1C (buf->GetUint8() + imgOffset,
    head.width, head.height, Depth, 
    DataSize (dataType, bpp, head.width, head.height, Depth))))
    dataType = csrawDXT1;

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
    Image.AttachNew (new csDDSImageFile (this, format, imgBuf,
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
	mip.AttachNew (new csDDSImageFile (this, format, imgBuf,
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

csDDSImageFile::csDDSImageFile (csDDSImageIO* iio,
                                int format,
                                iDataBuffer* sourceData, 
                                csDDSRawDataType rawType, 
                                const dds::PixelFormat& pixelFmt)
  : csImageMemory (format), mipmaps(0), iio (iio), imageType (csimg2D)
{
  rawInfo = new RawInfo;
  rawInfo->rawData = sourceData;
  rawInfo->rawType = rawType;
  rawInfo->pixelFmt = pixelFmt;
}

csDDSImageFile::~csDDSImageFile ()
{
  delete rawInfo;
}

namespace
{
  template<typename OutputType>
  struct OddDimensionsDecompressor
  {
    typedef const void* (CS::Graphics::iDXTDecompressor::*DecompressorFunction) (const void* inBlockPtr,
                                                                                 size_t blockDistance,
                                                                                 size_t numBlocks,
                                                                                 OutputType* outDataPtr,
                                                                                 const CS::Graphics::UncompressedDXTDataLayout& outDataLayout);

    static void Decompress (const uint8* source, size_t sourceBlockDist,
                            void* dest_, size_t destPixelDistance, int width, int height,
                            CS::Graphics::iDXTDecompressor* decompressor, DecompressorFunction func)
    {
      uint8* dest (reinterpret_cast<uint8*> (dest_));
      CS::Graphics::UncompressedDXTDataLayout layout;
      layout.bytesToNextPixel = destPixelDistance;
      layout.bytesToNextRow = width * destPixelDistance;
      layout.bytesToNextBlock = 4*destPixelDistance;
      layout.blocksPerRow = (width+3) / 4;

      int fullBlocksX = width / 4;
      int fullBlocksY = height / 4;
      const void* nextBlock;
      if ((width % 4) == 0)
      {
        nextBlock = (decompressor->*func) (source, sourceBlockDist, fullBlocksX * fullBlocksY,
                                           reinterpret_cast<OutputType*> (dest), layout);
        dest += fullBlocksY * 4 * width * destPixelDistance;
      }
      else
      {
        nextBlock = source;
        /* Width is not divisible by 4, need to decompress each row of blocks
         * separately and copy only the leftmost pixels of the right block */
        for (int blockY = 0; blockY < fullBlocksY; blockY++)
        {
          nextBlock = (decompressor->*func) (nextBlock, sourceBlockDist, fullBlocksX,
                                             reinterpret_cast<OutputType*> (dest), layout);
          // Handle 'incomplete' rightmost block
          OutputType block[4 * 4];
          nextBlock = (decompressor->*func) (nextBlock, sourceBlockDist, 1,
                                             block, CS::Graphics::UncompressedDXTDataLayout ());
          int numCols = width - fullBlocksX*4;
          uint8* destBlockStart = dest + fullBlocksX * 4 * destPixelDistance;
          for (int y = 0; y < 4; y++)
          {
            uint8* destPixel (destBlockStart + y * width * destPixelDistance);
            for (int x = 0; x < numCols; x++)
            {
              *(reinterpret_cast<OutputType*> (destPixel)) = block[y*4 + x];
              destPixel += destPixelDistance;
            }
          }
          dest += 4 * width * destPixelDistance;
        }
      }
      if ((fullBlocksY * 4) != height)
      {
        /* Height is not divisible by 4, need to decompress each block in the last row
         * separately and copy only the top pixels */
        int remainingRows = height % 4;
        uint8* firstRow = dest;
        // Handle incomplete lower blocks
        OutputType block[4 * 4];
        int blocksX ((width+3) / 4);
        for (int x = 0; x < blocksX; x++)
        {
          nextBlock = (decompressor->*func) (nextBlock, sourceBlockDist, 1,
                                             block, CS::Graphics::UncompressedDXTDataLayout ());
          int numCols = csMin (width - x*4, 4);

          for (int y = 0; y < remainingRows; y++)
          {
            uint8* destPixel (firstRow + y * width * destPixelDistance);
            for (int x = 0; x < numCols; x++)
            {
              *(reinterpret_cast<OutputType*> (destPixel)) = block[y*4 + x];
              destPixel += destPixelDistance;
            }
          }
          firstRow += 4 * destPixelDistance;
        }
      }
    }
  };
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
        OddDimensionsDecompressor<csRGBpixel>::Decompress (
          source, 8, buf, sizeof (csRGBpixel), Width, Height,
          iio->GetDXTDecompressor(), &CS::Graphics::iDXTDecompressor::DecompressDXT1RGBA);
        break;
      }
      case csrawDXT2:
      case csrawDXT3:
      {
        OddDimensionsDecompressor<csRGBpixel>::Decompress (
          source + 8, 16, buf, sizeof (csRGBpixel), Width, Height,
          iio->GetDXTDecompressor(), &CS::Graphics::iDXTDecompressor::DecompressDXT1RGB);
        OddDimensionsDecompressor<uint8>::Decompress (
          source, 16, &(buf->alpha), sizeof (csRGBpixel), Width, Height,
          iio->GetDXTDecompressor(), &CS::Graphics::iDXTDecompressor::DecompressDXT3Alpha);

        if (rawInfo->rawType == csrawDXT2)
          dds::Loader::CorrectPremult (buf, Width * Height);
        break;
      }
      case csrawDXT4:
      case csrawDXT5:
      {
        OddDimensionsDecompressor<csRGBpixel>::Decompress (
          source + 8, 16, buf, sizeof (csRGBpixel), Width, Height,
          iio->GetDXTDecompressor(), &CS::Graphics::iDXTDecompressor::DecompressDXT1RGB);
        OddDimensionsDecompressor<uint8>::Decompress (
          source, 16, &(buf->alpha), sizeof (csRGBpixel), Width, Height,
          iio->GetDXTDecompressor(), &CS::Graphics::iDXTDecompressor::DecompressDXTUNormToUI8);

        if (rawInfo->rawType == csrawDXT4)
          dds::Loader::CorrectPremult (buf, Width * Height);
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
  return (uint)mipmaps.GetSize ();
}

csRef<iImage> csDDSImageFile::GetMipmap (uint num)
{
  if (num == 0)
    return this;
  if (num > mipmaps.GetSize ())
    return 0;

  return csRef<iImage> (mipmaps[num-1]);
}

uint csDDSImageFile::HasSubImages() const 
{ 
  return (uint)subImages.GetSize ();
}

csRef<iImage> csDDSImageFile::GetSubImage (uint num) 
{
  if (num == 0)
    return this;
  if (num > subImages.GetSize ())
    return 0;

  return csRef<iImage> (subImages[num-1]);
}

static const char* RawTypeString (csDDSRawDataType type)
{
  switch (type)
  {
    case csrawDXT1:   
      return "*dxt1";
    case csrawDXT1Alpha:
      return "*dxt1a";
    case csrawDXT2:
      return "*dxt2";
    case csrawDXT3:
      return "*dxt3";
    case csrawDXT4:
      return "*dxt4";
    case csrawDXT5:
      return "*dxt5";
    case csrawR8G8B8:
      return "r8g8b8";
    case csrawR5G6B5:
      return "r5g6b5";
    case csrawA8R8G8B8:
      return "a8r8g8b8";
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
  return rawInfo ? (rawInfo->rawData) : csRef<iDataBuffer> (0);
}

void csDDSImageFile::Report (int severity, const char* msg, ...)
{
  va_list argv;
  va_start (argv, msg);
  csReportV (iio->GetObjectReg(), severity, "crystalspace.graphic.image.io.dds", msg,
    argv);
  va_end (argv);
}

}
CS_PLUGIN_NAMESPACE_END(DDSImageIO)
