/*
    DDS image file format support for CrystalSpace 3D library
    Copyright (C) 2003 by Matze Braun <matze@braunis.de>
    Code based on DevIL source by Denton Woods which was based on an nvidia
    example at:
    http://www.nvidia.com/view.asp?IO=dxtc_decompression_code

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
#include "dds.h"

#include "dxt.h"

CS_PLUGIN_NAMESPACE_BEGIN(DDSImageIO)
{
namespace dds
{

bool Loader::ProbeDXT1C (const uint8* source, int w, int h, int depth, 
  size_t /*size*/)
{
  unsigned char *Temp = (unsigned char*) source;

  for (int z = 0; z < depth; z++) 
  {
    for (int y = 0; y < w; y += 4) 
    {
      for (int x = 0; x < h; x += 4) 
      {
        uint16 color_0 = csLittleEndian::Convert (*((uint16*)Temp));
        uint16 color_1 = csLittleEndian::Convert (*((uint16*)Temp+1));
        Temp += 8;

        if (color_0 <= color_1) 
        {
          return false;
        }
      }
    }
  }

  return true;
}

inline static void ComputeMaskParams (uint32 mask, int& shift1, int& mul, int& shift2)
{
  shift1 = 0; mul = 1; shift2 = 0;
  while ((mask & 1) == 0)
  {
    mask >>= 1;
    shift1++;
  }
  uint bc = 0;
  while ((mask & (1 << bc)) != 0) bc++;
  while ((mask * mul) < 255)
    mul = (mul << bc) + 1;
  mask *= mul;
  while ((mask & ~0xff) != 0)
  {
    mask >>= 1;
    shift2++;
  }
}

void Loader::DecompressRGB (csRGBpixel* buffer, const uint8* source, 
			    int w, int h, int depth, size_t /*size*/, 
			    const PixelFormat& pf)
{
  const uint valMask = (1 << pf.bitdepth) - 1;
  const size_t pixSize = (pf.bitdepth + 7) / 8;
  int rShift1, rMul, rShift2;
  ComputeMaskParams (pf.redmask, rShift1, rMul, rShift2);
  int gShift1, gMul, gShift2;
  ComputeMaskParams (pf.greenmask, gShift1, gMul, gShift2);
  int bShift1, bMul, bShift2;
  ComputeMaskParams (pf.bluemask, bShift1, bMul, bShift2);

  uint pixnum = w * h * depth;
  while (pixnum-- > 0)
  {
    uint32 px = 
      csLittleEndian::Convert (csGetFromAddress::UInt32 (source)) & valMask;
    source += pixSize;
    uint32 pxc = px & pf.redmask;
    buffer->red = ((pxc >> rShift1) * rMul) >> rShift2;
    pxc = px & pf.greenmask;
    buffer->green = ((pxc >> gShift1) * gMul) >> gShift2;
    pxc = px & pf.bluemask;
    buffer->blue = ((pxc >> bShift1) * bMul) >> bShift2;
    buffer++;
  }
}

void Loader::DecompressRGBA (csRGBpixel* buffer, const uint8* source, 
			     int w, int h, int depth, size_t /*size*/, 
			     const PixelFormat& pf)
{
  const uint valMask = (pf.bitdepth == 32) ? ~0 : (1 << pf.bitdepth) - 1;
    // Funny x86s, make 1 << 32 == 1
  const size_t pixSize = (pf.bitdepth + 7) / 8;
  int rShift1, rMul, rShift2;
  ComputeMaskParams (pf.redmask, rShift1, rMul, rShift2);
  int gShift1, gMul, gShift2;
  ComputeMaskParams (pf.greenmask, gShift1, gMul, gShift2);
  int bShift1, bMul, bShift2;
  ComputeMaskParams (pf.bluemask, bShift1, bMul, bShift2);
  int aShift1, aMul, aShift2;
  ComputeMaskParams (pf.alphamask, aShift1, aMul, aShift2);

  uint pixnum = w * h * depth;
  while (pixnum-- > 0)
  {
    uint32 px = csLittleEndian::Convert (csGetFromAddress::UInt32 (source)) & valMask;
    source += pixSize;
    uint32 pxc = px & pf.redmask;
    buffer->red = ((pxc >> rShift1) * rMul) >> rShift2;
    pxc = px & pf.greenmask;
    buffer->green = ((pxc >> gShift1) * gMul) >> gShift2;
    pxc = px & pf.bluemask;
    buffer->blue = ((pxc >> bShift1) * bMul) >> bShift2;
    pxc = px & pf.alphamask;
    buffer->alpha = ((pxc >> aShift1) * aMul) >> aShift2;
    buffer++;
  }
}

void Loader::DecompressLum (csRGBpixel* buffer, const uint8* source, 
			    int w, int h, int depth, size_t /*size*/, 
			    const PixelFormat& pf)
{
  int lShift1, lMul, lShift2;
  ComputeMaskParams (pf.redmask, lShift1, lMul, lShift2);

  uint pixnum = w * h * depth;
  while (pixnum-- > 0)
  {
    uint8 px = *(source++);
    buffer->red = buffer->green = buffer->blue = 
      ((px >> lShift1) * lMul) >> lShift2;
    buffer++;
  }
}

void Loader::CorrectPremult (csRGBpixel* buffer, size_t pixnum)
{
  for (uint32 i = 0; i<pixnum; i++)
  {
    const int a = buffer[i].alpha;
    if (!a) continue;
    buffer[i].red = (buffer[i].red << 8) / a;
    buffer[i].green = (buffer[i].green << 8) / a;
    buffer[i].blue = (buffer[i].blue << 8) / a;
  }
}
  
} // end of namespace dds
}
CS_PLUGIN_NAMESPACE_END(DDSImageIO)
