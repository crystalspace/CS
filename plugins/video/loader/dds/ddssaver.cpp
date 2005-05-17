/*
    DSS image file format support for CrystalSpace 3D library
    Copyright (C) 2004-2005 by Frank Richter

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
#include "csgfx/memimage.h"
#include "csutil/util.h"

#include "ddssaver.h"
#include "dds.h"
#include "ddsutil.h"

#include "ImageLib/Image.h"

bool csDDSSaver::FmtB8G8R8::Save (csMemFile& out, iImage* image)
{
  size_t pixNum = image->GetWidth() * image->GetHeight() * image->GetDepth();
  csRGBpixel* pix = (csRGBpixel*)image->GetImageData();
  while (pixNum-- > 0)
  {
    uint8 b = pix->blue;
    out.Write ((char*)&b, 1);
    b = pix->green;
    out.Write ((char*)&b, 1);
    b = pix->red;
    out.Write ((char*)&b, 1);
    pix++;
  }
  return true;
}

bool csDDSSaver::FmtB8G8R8A8::Save (csMemFile& out, iImage* image)
{
  size_t pixNum = image->GetWidth() * image->GetHeight() * image->GetDepth();
  csRGBpixel* pix = (csRGBpixel*)image->GetImageData();
  while (pixNum-- > 0)
  {
    uint8 b = pix->blue;
    out.Write ((char*)&b, 1);
    b = pix->green;
    out.Write ((char*)&b, 1);
    b = pix->red;
    out.Write ((char*)&b, 1);
    b = pix->alpha;
    out.Write ((char*)&b, 1);
    pix++;
  }
  return true;
}

bool csDDSSaver::FmtDXT::Save (csMemFile& out, iImage* image)
{
  const int imgW = image->GetWidth();
  if ((imgW > 4) && ((imgW & 3) != 0)) return 0;
  const int imgH = image->GetHeight();
  if ((imgH > 4) && ((imgH & 3) != 0)) return 0;

  ImageLib::Image32* img = new ImageLib::Image32;
  img->SetSize (imgW, imgH);
  ImageLib::Color* p = img->GetPixels();

  const size_t imagePixels = imgW * imgH;
  size_t pixNum = imagePixels;
  csRGBpixel* pix = (csRGBpixel*)image->GetImageData();
  while (pixNum-- > 0)
  {
    p->r = pix->red;
    p->g = pix->green;
    p->b = pix->blue;
    // ImageLib also considers the alpha when compressing for DXT1,
    // so pull it up to get a solid image.
    p->a = (method == ImageLib::DC_DXT1) ? 255 : pix->alpha;
    p++; pix++;
  }

  // Floyd/Steinberg (modified) error diffusion
  bool dither = false;
  if (options.GetBool ("dither", dither))
  {
    img->DiffuseError((method == ImageLib::DC_DXT3) ? 4 : 8, 5, 6, 5);
  }

  // Build the DXTC texture
  ImageLib::ImageDXTC dxtc;
  dxtc.FromImage32 (img, method);

  uint16* dxtBlocks = (uint16*)dxtc.GetBlocks();
  size_t wordCount = (MAX(imgW, 4)*MAX(imgH, 4)) / ((method == ImageLib::DC_DXT1) ? 4 : 2);
  while (wordCount-- > 0)
  {
    uint16 ui16 = csLittleEndianShort (*dxtBlocks++);
    out.Write ((char*)&ui16, sizeof (ui16));
  }

  delete img;

  return true;
}

uint csDDSSaver::SaveMips (csMemFile& out, iImage* image, Format* format)
{
  uint m;
  for (m = 0; m <= image->HasMipmaps(); m++)
  {
    csRef<iImage> mip = image->GetMipmap (m);
    if (!format->Save (out, mip)) return 0;
  }
  return m;
}

csPtr<iDataBuffer> csDDSSaver::Save (csRef<iImage> image, 
  const csImageLoaderOptionsParser& options)
{
  if ((image->GetFormat() & CS_IMGFMT_MASK) != CS_IMGFMT_TRUECOLOR)
  {
    image.AttachNew (new csImageMemory (image,
      (image->GetFormat() & ~CS_IMGFMT_MASK) | CS_IMGFMT_TRUECOLOR));
  }

  csString format (
    (image->GetFormat() & CS_IMGFMT_ALPHA) ? "b8g8r8a8" : "b8g8r8");
  if (options.GetString ("format", format))
  {
    if (format == "dxt")
    {
      if (image->GetFormat() & CS_IMGFMT_ALPHA)
	format = "dxt5";
      else
	format = "dxt1";
    }
  }
  bool noMipMaps = false;
  options.GetBool ("nomipmaps", noMipMaps);

  dds::Header ddsHead;
  memset (&ddsHead, 0, sizeof (ddsHead));

  ddsHead.magic = dds::Magic;
  ddsHead.size = sizeof(ddsHead) - sizeof(uint32);
  ddsHead.flags = dds::DDSD_WIDTH | dds::DDSD_HEIGHT | dds::DDSD_CAPS 
    | dds::DDSD_PIXELFORMAT;
  ddsHead.width = image->GetWidth();
  ddsHead.height = image->GetHeight();
  ddsHead.pixelformat.size = sizeof(ddsHead.pixelformat);
  ddsHead.capabilities.caps1 = dds::DDSCAPS_TEXTURE;
  if (image->HasMipmaps() && !noMipMaps)
  {
    ddsHead.mipmapcount = image->HasMipmaps()+1;
    ddsHead.flags |= dds::DDSD_MIPMAPCOUNT;
    ddsHead.capabilities.caps1 |= dds::DDSCAPS_MIPMAP | dds::DDSCAPS_COMPLEX;
  }
  if (image->GetImageType() == csimg3D)
  {
    ddsHead.depth = image->GetDepth();
    ddsHead.flags |= dds::DDSD_DEPTH;
    ddsHead.capabilities.caps1 |= dds::DDSCAPS_COMPLEX;
    ddsHead.capabilities.caps2 |= dds::DDSCAPS2_VOLUME;
  }
  else if ((image->GetImageType() == csimgCube) 
    && (image->HasSubImages() > 5))
  {
    ddsHead.capabilities.caps1 |= dds::DDSCAPS_COMPLEX;
    ddsHead.capabilities.caps2 |= dds::DDSCAPS2_CUBEMAP
      | dds::DDSCAPS2_CUBEMAP_NEGATIVEX | dds::DDSCAPS2_CUBEMAP_POSITIVEX
      | dds::DDSCAPS2_CUBEMAP_NEGATIVEY | dds::DDSCAPS2_CUBEMAP_POSITIVEY
      | dds::DDSCAPS2_CUBEMAP_NEGATIVEZ | dds::DDSCAPS2_CUBEMAP_POSITIVEZ;
  }

  Format* saver = 0;
  if (format == "b8g8r8")
  {
    ddsHead.pixelformat.bitdepth = 24;
    ddsHead.pixelformat.redmask = 0x00ff0000;
    ddsHead.pixelformat.greenmask = 0x0000ff00;
    ddsHead.pixelformat.bluemask = 0x000000ff;
    ddsHead.pixelformat.flags = dds::DDPF_RGB;
    saver = new FmtB8G8R8;
  }
  else if (format == "b8g8r8a8")
  {
    ddsHead.pixelformat.bitdepth = 32;
    ddsHead.pixelformat.redmask = 0x00ff0000;
    ddsHead.pixelformat.greenmask = 0x0000ff00;
    ddsHead.pixelformat.bluemask = 0x000000ff;
    ddsHead.pixelformat.alphamask = 0xff000000;
    ddsHead.pixelformat.flags = dds::DDPF_RGB | dds::DDPF_ALPHAPIXEL;
    saver = new FmtB8G8R8A8;
  }
  else if (format == "dxt1")
  {
    if (image->GetImageType() != csimg2D) return 0;
    ddsHead.pixelformat.flags = dds::DDPF_FOURCC;
    ddsHead.pixelformat.fourcc = MakeFourCC ('D','X','T','1');
    saver = new FmtDXT (ImageLib::DC_DXT1, options);
  }
  else if (format == "dxt3")
  {
    if (image->GetImageType() != csimg2D) return 0;
    ddsHead.pixelformat.flags = dds::DDPF_FOURCC;
    ddsHead.pixelformat.fourcc = MakeFourCC ('D','X','T','3');
    saver = new FmtDXT (ImageLib::DC_DXT3, options);
  }
  else if (format == "dxt5")
  {
    if (image->GetImageType() != csimg2D) return 0;
    ddsHead.pixelformat.flags = dds::DDPF_FOURCC;
    ddsHead.pixelformat.fourcc = MakeFourCC ('D','X','T','5');
    saver = new FmtDXT (ImageLib::DC_DXT5, options);
  }
  if (!saver) return 0;

  csMemFile outFile;
  {
    size_t n = sizeof (ddsHead) / sizeof(uint32);
    uint32* ptr = (uint32*)&ddsHead;
    while (n-- > 0)
    {
      uint32 x = csGetLittleEndianLong (ptr++);
      outFile.Write ((char*)&x, sizeof (x));
    }
  }
  if (image->GetImageType() == csimgCube)
  {
    const uint mipcount = ddsHead.mipmapcount ? ddsHead.mipmapcount : 1;
    for (int i = 0; i < 6; i++)
    {
      if (noMipMaps)
      {
	if (!saver->Save (outFile, image->GetSubImage (i))) 
	{
	  delete saver;
	  return 0;
	}
      }
      else
      {
	if (SaveMips (outFile, image->GetSubImage (i), saver) 
	  != mipcount)
	{
	  delete saver;
	  return 0;
	}
      }
    }
  }
  else
  {
    if (noMipMaps)
    {
      if (!saver->Save (outFile, image)) 
      {
	delete saver;
	return 0;
      }
    }
    else
    {
      if (!SaveMips (outFile, image, saver)) 
      {
	delete saver;
	return 0;
      }
    }
  }
  delete saver;

  csRef<iDataBuffer> fileData (outFile.GetAllData());
  return csPtr<iDataBuffer> (fileData);
}
