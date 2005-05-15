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

bool csDDSSaver::SaveB8G8R8 (csMemFile& out, iImage* image)
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

bool csDDSSaver::SaveB8G8R8A8 (csMemFile& out, iImage* image)
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

bool csDDSSaver::SaveDXT (csMemFile& out, iImage* image, 
			  ImageLib::DXTCMethod method)
{
  ImageLib::Image32* img = new ImageLib::Image32;
  img->SetSize (image->GetWidth(), image->GetHeight());
  ImageLib::Color* p = img->GetPixels();

  const size_t imagePixels = image->GetWidth() * image->GetHeight();
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
  img->DiffuseError((method == ImageLib::DC_DXT3) ? 4 : 8, 5, 6, 5);

  // Build the DXTC texture
  ImageLib::ImageDXTC dxtc;
  dxtc.FromImage32 (img, method);

  out.Write ((char*)dxtc.GetBlocks(), 
    imagePixels / ((method == ImageLib::DC_DXT1) ? 2 : 1));

  delete img;

  return true;
}

csPtr<iDataBuffer> csDDSSaver::Save (csRef<iImage> image, 
  const csImageLoaderOptionsParser& options)
{
  // @@@ What about non-PO2 images?
  if (!csIsPowerOf2 (image->GetWidth()) || !csIsPowerOf2 (image->GetHeight()))
    return 0;
  if (image->GetImageType() != csimg2D) return 0;

  if ((image->GetFormat() & CS_IMGFMT_MASK) != CS_IMGFMT_TRUECOLOR)
  {
    image.AttachNew (new csImageMemory (image,
      (image->GetFormat() & ~CS_IMGFMT_MASK) | CS_IMGFMT_TRUECOLOR));
  }

  csString format (
    (image->GetFormat() & CS_IMGFMT_ALPHA) ? "a8r8g8b8" : "b8g8r8");
  if (options.GetString ("format", format))
  {
    if (format == "dxt")
    {
      if (image->GetFormat() & CS_IMGFMT_ALPHA)
	format = "dxt3"; // "dxt5";
      else
	format = "dxt1";
    }
  }


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

  SaverFunc saverFn = 0;
  if (format == "b8g8r8")
  {
    ddsHead.pixelformat.bitdepth = 24;
    ddsHead.pixelformat.redmask = 0x00ff0000;
    ddsHead.pixelformat.greenmask = 0x0000ff00;
    ddsHead.pixelformat.bluemask = 0x000000ff;
    ddsHead.pixelformat.flags = dds::DDPF_RGB;
    saverFn = &SaveB8G8R8;
  }
  else if (format == "b8g8r8a8")
  {
    ddsHead.pixelformat.bitdepth = 32;
    ddsHead.pixelformat.redmask = 0x00ff0000;
    ddsHead.pixelformat.greenmask = 0x0000ff00;
    ddsHead.pixelformat.bluemask = 0x000000ff;
    ddsHead.pixelformat.alphamask = 0xff000000;
    ddsHead.pixelformat.flags = dds::DDPF_RGB | dds::DDPF_ALPHAPIXEL;
    saverFn = &SaveB8G8R8A8;
  }
  else if (format == "dxt1")
  {
    ddsHead.pixelformat.flags = dds::DDPF_FOURCC;
    ddsHead.pixelformat.fourcc = MakeFourCC ('D','X','T','1');
    saverFn = &SaveDXT1;
  }
  else if (format == "dxt3")
  {
    ddsHead.pixelformat.flags = dds::DDPF_FOURCC;
    ddsHead.pixelformat.fourcc = MakeFourCC ('D','X','T','3');
    saverFn = &SaveDXT3;
  }

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
  if (!saverFn || !saverFn (outFile, image)) return 0;

  csRef<iDataBuffer> fileData (outFile.GetAllData());
  return csPtr<iDataBuffer> (fileData);
}
