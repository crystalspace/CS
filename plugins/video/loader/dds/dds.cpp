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

namespace dds
{

struct Color8888
{
  unsigned char r,g,b,a;
};

static inline int ColorComponent (int x, int bitcount, int shift)
{
  int val = ((x >> shift) & ((1 << bitcount) - 1));
  return (val << (8 - bitcount)) + (val >> (2 * bitcount - 8));
}

#define COLOR565_RED(x)	    ColorComponent(x, 5, 11)
#define COLOR565_GREEN(x)   ColorComponent(x, 6, 5)
#define COLOR565_BLUE(x)    ColorComponent(x, 5, 0)

bool Loader::ProbeDXT1Alpha (const uint8* source, int w, int h, int depth, 
			     size_t size)
{
  int          x, y, z;
  unsigned char *Temp;
  uint16       color_0, color_1;

  Temp = (unsigned char*) source;
  for (z = 0; z < depth; z++) 
  {
    for (y = 0; y < w; y += 4) 
    {
      for (x = 0; x < h; x += 4) 
      {
	color_0 = csLittleEndianShort (*((uint16*)Temp)); 
	color_1 = csLittleEndianShort (*((uint16*)Temp+1));
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

void Loader::DecompressDXT1 (csRGBpixel* buffer, const uint8* source, 
			     int Width, int Height, int depth, 
			     size_t planesize)
{
  int          x, y, z, i, j, k, Select;
  unsigned char *Temp;
  uint16       color_0, color_1;
  Color8888    colours[4];
  uint32       bitmask;
  size_t       Offset;

  Temp = (unsigned char*) source;
  for (z = 0; z < depth; z++) 
  {
    for (y = 0; y < Height; y += 4) 
    {
      for (x = 0; x < Width; x += 4) 
      {
	color_0 = csLittleEndianShort (*((uint16*)Temp)); 
	color_1 = csLittleEndianShort (*((uint16*)Temp+1));
	bitmask = csLittleEndianLong (((uint32*)Temp)[1]);
	Temp += 8;

	colours[0].r = COLOR565_RED(color_0); 
	colours[0].g = COLOR565_GREEN(color_0);
	colours[0].b = COLOR565_BLUE(color_0); 
	colours[0].a = 0xFF;

	colours[1].r = COLOR565_RED(color_1); 
	colours[1].g = COLOR565_GREEN(color_1);
	colours[1].b = COLOR565_BLUE(color_1);
	colours[1].a = 0xFF;

	if (color_0 > color_1) 
	{
	  // Four-color block: derive the other two colors.
	  // 00 = color_0, 01 = color_1, 10 = color_2, 11 = color_3
	  // These 2-bit codes correspond to the 2-bit fields
	  // stored in the 64-bit block.
	  colours[2].b = (2 * colours[0].b + colours[1].b + 1) / 3;
	  colours[2].g = (2 * colours[0].g + colours[1].g + 1) / 3;
	  colours[2].r = (2 * colours[0].r + colours[1].r + 1) / 3;
	  colours[2].a = 0xFF;

	  colours[3].b = (colours[0].b + 2 * colours[1].b + 1) / 3;
	  colours[3].g = (colours[0].g + 2 * colours[1].g + 1) / 3;
	  colours[3].r = (colours[0].r + 2 * colours[1].r + 1) / 3;
	  colours[3].a = 0xFF;
	}
	else 
	{
	  // Three-color block: derive the other color.
	  // 00 = color_0,  01 = color_1,  10 = color_2,
	  // 11 = transparent.
	  // These 2-bit codes correspond to the 2-bit fields
	  // stored in the 64-bit block.
	  colours[2].b = (colours[0].b + colours[1].b) / 2;
	  colours[2].g = (colours[0].g + colours[1].g) / 2;
	  colours[2].r = (colours[0].r + colours[1].r) / 2;
	  colours[2].a = 0xFF;

	  colours[3].b = (colours[0].b + 2 * colours[1].b + 1) / 3;
	  colours[3].g = (colours[0].g + 2 * colours[1].g + 1) / 3;
	  colours[3].r = (colours[0].r + 2 * colours[1].r + 1) / 3;
	  colours[3].a = 0x00;
	}

	for (j = 0, k = 0; j < 4; j++) 
	{
	  for (i = 0; i < 4; i++, k++) 
	  {
	    Select = (bitmask & (0x03 << k*2)) >> k*2;
	    const Color8888& col = colours[Select];

	    if (((x + i) < Width) && ((y + j) < Height)) 
	    {
	      Offset = z * planesize + (y + j) * Width + (x + i);
	      buffer[Offset].Set (col.r, col.g, col.b, col.a);
	    }
	  }
	}
      }
    }
  }
}

void Loader::DecompressDXT2(csRGBpixel* buffer, const uint8* source, int width,
			    int height, int depth, size_t planesize)
{
  // Can do color & alpha same as dxt3, but color is pre-multiplied
  // so the result will be wrong unless corrected.
  DecompressDXT3(buffer, source, width, height, depth, planesize);
  CorrectPremult (buffer, width * height * depth);
}

struct DXTAlphaBlockExplicit
{
  uint16 row[4];
};

void Loader::DecompressDXT3(csRGBpixel* buffer, const uint8* source,
			    int Width, int Height, int depth, 
			    size_t planesize)
{
  int           x, y, z, i, j, k, Select;
  unsigned char *Temp;
  uint16        color_0, color_1;
  Color8888     colours[4];
  uint32        bitmask;
  size_t	Offset;
  uint16	word;
  DXTAlphaBlockExplicit alpha;

  Temp = (unsigned char*) source;
  for (z = 0; z < depth; z++) 
  {
    for (y = 0; y < Height; y += 4) 
    {
      for (x = 0; x < Width; x += 4) 
      {
	alpha.row[0] = csLittleEndianShort (Temp[0]);
	alpha.row[1] = csLittleEndianShort (Temp[1]);
	alpha.row[2] = csLittleEndianShort (Temp[2]);
	alpha.row[3] = csLittleEndianShort (Temp[3]);
	Temp += 8;
	color_0 = csLittleEndianShort (*((uint16*)Temp));   
	color_1 = csLittleEndianShort (*((uint16*)Temp+1));
	bitmask = csLittleEndianLong (((uint32*)Temp)[1]);

	colours[0].r = COLOR565_RED(color_0); 
	colours[0].g = COLOR565_GREEN(color_0);
	colours[0].b = COLOR565_BLUE(color_0); 
	colours[0].a = 0xFF;

	colours[1].r = COLOR565_RED(color_1); 
	colours[1].g = COLOR565_GREEN(color_1);
	colours[1].b = COLOR565_BLUE(color_1); 
	colours[1].a = 0xFF;

	// Four-color block: derive the other two colors.
	// 00 = color_0, 01 = color_1, 10 = color_2, 11	= color_3
	// These 2-bit codes correspond to the 2-bit fields
	// stored in the 64-bit block.
	colours[2].b = (2 * colours[0].b + colours[1].b + 1) / 3;
	colours[2].g = (2 * colours[0].g + colours[1].g + 1) / 3;
	colours[2].r = (2 * colours[0].r + colours[1].r + 1) / 3;
	colours[2].a = 0xFF;

	colours[3].b = (colours[0].b + 2 * colours[1].b + 1) / 3;
	colours[3].g = (colours[0].g + 2 * colours[1].g + 1) / 3;
	colours[3].r = (colours[0].r + 2 * colours[1].r + 1) / 3;
	colours[3].a = 0xFF;

	k = 0;
	for (j = 0; j < 4; j++) 
	{
	  for (i = 0; i < 4; i++, k++) 
	  {
	    Select = (bitmask & (0x03 << k*2)) >> k*2;
	    const Color8888& col = colours[Select];

	    if (((x + i) < Width) && ((y + j) < Height)) 
	    {
	      Offset = z * planesize + (y + j) * Width + (x + i);
	      buffer[Offset].Set (col.r, col.g, col.b, col.a);
	    }
	  }
	}

	for (j = 0; j < 4; j++) 
	{
	  word = alpha.row[j];
	  for (i = 0; i < 4; i++) 
	  {
	    if (((x + i) < Width) && ((y + j) < Height)) 
	    {
	      Offset = z * planesize + (y + j) * Width + (x + i);
	      const int a = (word & 0x0F) * 0x11;
	      buffer[Offset].alpha = a;
	    }
	    word >>= 4;
	  }
	}
      }
    }
  }
}

void Loader::DecompressDXT4 (csRGBpixel* buffer, const uint8* source, 
			     int width, int height, int depth, 
			     size_t planesize)
{
  // Can do color & alpha same as dxt5, but color is pre-multiplied
  // so the result will be wrong unless corrected.
  DecompressDXT5 (buffer, source, width, height, depth, planesize);
  CorrectPremult (buffer, width * height * depth);
}

void Loader::DecompressDXT5 (csRGBpixel* buffer, const uint8* source, 
			     int Width, int Height, int depth, 
			     size_t planesize)
{
  int             x, y, z, i, j, k, Select;
  const uint8     *Temp;
  uint16       color_0, color_1;
  Color8888       colours[4];
  uint32          bitmask;
  size_t	  Offset;
  uint8		  alphas[8];
  const uint8*	  alphamask;
  uint32          bits;

  Temp = source;
  for (z = 0; z < depth; z++) 
  {
    for (y = 0; y < Height; y += 4) 
    {
      for (x = 0; x < Width; x += 4) 
      {
	if (y >= Height || x >= Width)
	  break;
	alphas[0] = Temp[0];
	alphas[1] = Temp[1];
	alphamask = Temp + 2;
	Temp += 8;
	color_0 = csLittleEndianShort (*((uint16*)Temp));   
	color_1 = csLittleEndianShort (*((uint16*)Temp+1));
	bitmask = csLittleEndianLong (((uint32*)Temp)[1]);
	Temp += 8;

	colours[0].r = COLOR565_RED(color_0); 
	colours[0].g = COLOR565_GREEN(color_0);
	colours[0].b = COLOR565_BLUE(color_0); 
	colours[0].a = 0xFF;

	colours[1].r = COLOR565_RED(color_1); 
	colours[1].g = COLOR565_GREEN(color_1);
	colours[1].b = COLOR565_BLUE(color_1); 
	colours[1].a = 0xFF;

	// Four-color block: derive the other two colors.
	// 00 = color_0, 01 = color_1, 10 = color_2, 11	= color_3
	// These 2-bit codes correspond to the 2-bit fields
	// stored in the 64-bit block.
	colours[2].b = (2 * colours[0].b + colours[1].b + 1) / 3;
	colours[2].g = (2 * colours[0].g + colours[1].g + 1) / 3;
	colours[2].r = (2 * colours[0].r + colours[1].r + 1) / 3;
	colours[2].a = 0xFF;

	colours[3].b = (colours[0].b + 2 * colours[1].b + 1) / 3;
	colours[3].g = (colours[0].g + 2 * colours[1].g + 1) / 3;
	colours[3].r = (colours[0].r + 2 * colours[1].r + 1) / 3;
	colours[3].a = 0xFF;

	k = 0;
	for (j = 0; j < 4; j++) 
	{
	  for (i = 0; i < 4; i++, k++) 
	  {
	    Select = (bitmask & (0x03 << k*2)) >> k*2;
	    const Color8888& col = colours[Select];

	    // only put pixels out < width or height
	    if (((x + i) < Width) && ((y + j) < Height)) 
	    {
	      Offset = z * planesize + (y + j) * Width + (x + i);
	      buffer[Offset].Set (col.r, col.g, col.b, col.a);
	    }
	  }
	}

	// 8-alpha or 6-alpha block?
	if (alphas[0] > alphas[1]) 
	{
	  // 8-alpha block:  derive the other six alphas.
	  // Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated.
	  alphas[2] = (6 * alphas[0] + 1 * alphas[1] + 3) / 7; // bit code 010
	  alphas[3] = (5 * alphas[0] + 2 * alphas[1] + 3) / 7; // bit code 011
	  alphas[4] = (4 * alphas[0] + 3 * alphas[1] + 3) / 7; // bit code 100
	  alphas[5] = (3 * alphas[0] + 4 * alphas[1] + 3) / 7; // bit code 101
	  alphas[6] = (2 * alphas[0] + 5 * alphas[1] + 3) / 7; // bit code 110
	  alphas[7] = (1 * alphas[0] + 6 * alphas[1] + 3) / 7; // bit code 111
	}
	else 
	{
	  // 6-alpha block.
	  // Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated.
	  alphas[2] = (4 * alphas[0] + 1 * alphas[1] + 2) / 5; // Bit code 010
	  alphas[3] = (3 * alphas[0] + 2 * alphas[1] + 2) / 5; // Bit code 011
	  alphas[4] = (2 * alphas[0] + 3 * alphas[1] + 2) / 5; // Bit code 100
	  alphas[5] = (1 * alphas[0] + 4 * alphas[1] + 2) / 5; // Bit code 101
	  alphas[6] = 0x00;
	  // Bit code 110
	  alphas[7] = 0xFF;
	  // Bit code 111
	}

	// Note: Have to separate the next two loops,
	// it operates on a 6-byte system.

	// First three bytes
	bits = csLittleEndianLong (*((int32*)alphamask));
	for (j = 0; j < 2; j++) 
	{
	  for (i = 0; i < 4; i++) 
	  {
	    // only put pixels out < width or height
	    if (((x + i) < Width) && ((y + j) < Height)) 
	    {
	      Offset = z * planesize + (y + j) * Width + (x + i);
	      buffer[Offset].alpha = alphas[bits & 0x07];
	    }
	    bits >>= 3;
	  }
	}

	// Last three bytes
	bits = csLittleEndianLong (*((int32*)&alphamask[3]));
	for (j = 2; j < 4; j++) 
	{
	  for (i = 0; i < 4; i++) 
	  {
	    // only put pixels out < width or height
	    if (((x + i) < Width) && ((y + j) < Height)) 
	    {
	      Offset = z * planesize + (y + j) * Width + (x + i);
	      buffer[Offset].alpha = alphas[bits & 0x07];
	    }
	    bits >>= 3;
	  }
	}
      }
    }
  }
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
			    int w, int h, int depth, size_t size, 
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
    uint32 px = csGetLittleEndianLong (source) & valMask;
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
			     int w, int h, int depth, size_t size, 
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
    uint32 px = csGetLittleEndianLong (source) & valMask;
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
			    int w, int h, int depth, size_t size, 
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
