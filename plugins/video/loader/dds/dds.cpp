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

#define MakeFourCC(c1, c2, c3, c4) \
  ((uint32) c1 | ((uint32) c2<<8) | ((uint32) c3<<16) | ((uint32) c4<<24))

Loader::Loader ()
  : source(0), sourcelen(0), header(0), lasterror(0)
{
  positions = 0;
}

Loader::~Loader ()
{
  delete[] positions;
  delete header;
}

void Loader::SetSource (void* buffer, size_t bufferlen)
{
  source = (uint8*) buffer;
  sourcelen = bufferlen;
  header = 0;
}

bool Loader::IsDDS ()
{
  uint32 magic = csLittleEndianLong( *((uint32*) source));
  if (magic != MakeFourCC ('D','D','S',' '))
    return false;

  if (!ReadHeader ())
    return false;

  if (header->size != 124)
    return false;

  // minimum flags we need
  uint32 minimumflags = FLAG_CAPS | FLAG_HEIGHT | FLAG_WIDTH |
		     FLAG_PIXELFORMAT;
  if ((header->flags & minimumflags) != minimumflags)
    return false;

  return true;
}

int Loader::GetWidth()
{
  return header->width;
}

int Loader::GetHeight ()
{
  return header->height;
}

int Loader::GetBytesPerPixel ()
{
  return bpp;
}

int Loader::GetDepth ()
{
  return depth;
}

int Loader::GetMipmapCount ()
{
  if (!(header->flags & FLAG_MIPMAPCOUNT))
    return 0;

  return header->mipmapcount;
}

csRGBpixel* Loader::LoadImage ()
{
  csRGBpixel* buffer = new csRGBpixel[GetWidth() * GetHeight() * GetDepth()];

  // Data starts after header (header size is minus magic)
  if (!Decompress (buffer, positions[0], GetWidth(), GetHeight(), 
	      GetWidth() * GetHeight() * bpp))
  {
    delete[] buffer;
    return 0;
  }

  return buffer;
}

csRGBpixel* Loader::LoadMipmap (int number)
{
  if (number >= GetMipmapCount())
    return 0;

  int width = GetWidth() >> (number+1);
  int height = GetHeight() >> (number+1);

  csRGBpixel* buffer = new csRGBpixel[width * height];
  Decompress (buffer, positions[number+1], width, height,
	      width * height * bpp);

  return buffer;
}

uint32 Loader::GetUInt32()
{
  uint32 res = csLittleEndianLong (*((uint32*) readpos));
  readpos += sizeof(uint32);

  return res;
}

bool Loader::ReadHeader ()
{
  if (sourcelen < sizeof(Header))
    return false;

  int i;

  readpos = source;

  delete header;
  header = new Header;
  header->magic = GetUInt32();
  header->size = GetUInt32();
  header->flags = GetUInt32 ();
  header->height = GetUInt32 ();
  header->width = GetUInt32 ();
  header->linearsize = GetUInt32 ();
  header->depth = GetUInt32 ();
  header->mipmapcount = GetUInt32 ();
  header->alphabitdepth = GetUInt32 ();
  for (i=0;i<10;i++)
    header->reserved[i] = GetUInt32 ();
  
  header->pixelformat.size = GetUInt32 ();
  header->pixelformat.flags = GetUInt32 ();
  header->pixelformat.fourcc = GetUInt32 ();
  header->pixelformat.bitdepth = GetUInt32 ();
  header->pixelformat.redmask = GetUInt32 ();
  header->pixelformat.bluemask = GetUInt32 ();
  header->pixelformat.greenmask = GetUInt32 ();
  header->pixelformat.alphamask = GetUInt32 ();

  header->capabilities.caps1 = GetUInt32 ();
  header->capabilities.caps2 = GetUInt32 ();
  header->capabilities.caps3 = GetUInt32 ();
  header->capabilities.caps4 = GetUInt32 ();
  header->capabilities.texturestage = GetUInt32 ();

  if (header->size != 124)
    return false;

  // minimum flags we need                                   
  uint32 minimumflags = FLAG_CAPS | FLAG_HEIGHT | FLAG_WIDTH |
		     FLAG_PIXELFORMAT;
  if ((header->flags & minimumflags) != minimumflags)
    return false;

  CheckFormat ();

  if (format == FORMAT_UNKNOWN)
  {
    printf ("Unknown compression format in dds file.\n");
    return false;
  }
  if (header->capabilities.caps1 & FLAG_COMPLEX
      && header->capabilities.caps1 & FLAG_CUBEMAP)
  {
    printf ("Cubemaps not supported yet!\n");  
    return false;
  }

  // Calculate Positions of the image Data
  int nmips = GetMipmapCount();
  if (nmips == 0) nmips = 1;
  delete[] positions;
  positions = new uint8* [nmips];
  positions[0] = readpos;
  uint32 add;
  if (header->flags & FLAG_LINEARSIZE)
    add = header->linearsize;
  else if (header->flags & FLAG_PITCH)
    add = header->linearsize *= header->height;
  else
  {
    if (format == FORMAT_RGBA || format == FORMAT_RGB)
      add = header->width * header->height * bpp;
    else if (format == FORMAT_DXT1)
      add = header->width * header->height / 2;
    else
      add = header->width * header->height;
  }
  for (i = 1; i < nmips; i++)
  {
    positions[i] = positions[i - 1] + add;
    if (format == FORMAT_RGBA || format == FORMAT_RGB)
      add /= 4;
    else if (format == FORMAT_DXT1)
      add = MAX(add/4, 8);
    else
      add = MAX(add/4, 16);
  }
  if ((size_t)(positions[nmips - 1] - positions[0] + add) > sourcelen)
  {
    printf ("DDS Image too small: needs %u bytes; contains only %lu.\n",
	(unsigned int)(positions[nmips] - positions[0] + add),
	(unsigned long)sourcelen);
    return false;
  }
    
  return true;
}

void Loader::CheckFormat ()
{
  blocksize = 0;
  bpp = 4;

  if (header->pixelformat.flags & FLAG_FOURCC) // compressed image
  {
    blocksize = ((GetWidth()+3)/4) * ((GetHeight()+3)/4) *
      ((header->pixelformat.bitdepth +3)/4);
    switch (header->pixelformat.fourcc)
    {
      case MakeFourCC ('D','X','T','1'):
	blocksize *= 8;
	format = FORMAT_DXT1;
	break;
      case MakeFourCC ('D','X','T','2'):
	blocksize *= 16;
	format = FORMAT_DXT2;
	break;
      case MakeFourCC ('D','X','T','3'):
	blocksize *= 16;
	format = FORMAT_DXT3;
	break;
      case MakeFourCC ('D','X','T','4'):
	blocksize *= 16;
	format = FORMAT_DXT4;
	break;
      case MakeFourCC ('D','X','T','5'):
	blocksize *= 16;
	format = FORMAT_DXT5;
	break;

      default:
	format = FORMAT_UNKNOWN;
	lasterror = "Unknown compression format";
	return;
    }
  }
  else // not compressed
  {
    if (header->pixelformat.flags & FLAG_ALPHAPIXEL)
    {
      //blocksize = GetWidth()*GetHeight() * header->pixelformat.bitdepth * 4;
      format = FORMAT_RGBA;
    }
    else
    {
      //blocksize = GetWidth()*GetHeight() * header->pixelformat.bitdepth * 3;
      format = FORMAT_RGB;
      //bpp = 3;
    }
    bpp = header->pixelformat.bitdepth / 8;
    blocksize = GetWidth() * GetHeight() * bpp;
  }

  depth = header->depth ? header->depth : 1;
}

bool Loader::Decompress (csRGBpixel* buffer, uint8* source,
			 int width, int height, uint32 planesize)
{
  switch (format)
  {
    case FORMAT_RGB:
    case FORMAT_RGBA:
      if (bpp < 3) return false;
      DecompressRGBA (buffer, source, width, height, planesize);
      break;
    case FORMAT_DXT1:
      DecompressDXT1 (buffer, source, width, height, planesize);
      break;
    case FORMAT_DXT2:
      DecompressDXT2 (buffer, source, width, height, planesize);
      break;
    case FORMAT_DXT3:
      DecompressDXT3 (buffer, source, width, height, planesize);
      break;
    case FORMAT_DXT4:
      DecompressDXT4 (buffer, source, width, height, planesize);
      break;
    case FORMAT_DXT5:
      DecompressDXT5 (buffer, source, width, height, planesize);
      break;
    default:
      break;
  }
  return true;
}

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

void Loader::DecompressRGBA (csRGBpixel* buffer, uint8* source, int, int, 
			     uint32 size)
{
  if (format == FORMAT_RGB)
  {
    while (size > 0)
    {
      buffer->red = *source++;
      buffer->green = *source++;
      buffer->blue = *source++;
      buffer->alpha = 0xff;
      size -= 3;
      buffer++;
    }
  }
  else
  {
    while (size > 0)
    {
      buffer->red = *source++;
      buffer->green = *source++;
      buffer->blue = *source++;
      buffer->alpha = *source++;
      size -= 4;
      buffer++;
    }
  }
}

void Loader::DecompressDXT1 (csRGBpixel* buffer, uint8* source, 
			     int Width, int Height, uint32 planesize)
{
  int          x, y, z, i, j, k, Select;
  unsigned char *Temp;
  uint16       color_0, color_1;
  Color8888    colours[4];
  uint32       bitmask, Offset;

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

void Loader::DecompressDXT2(csRGBpixel* buffer, uint8* source, int width,
			    int height, uint32 planesize)
{
  // Can do color & alpha same as dxt3, but color is pre-multiplied
  // so the result will be wrong unless corrected.
  DecompressDXT3(buffer, source, width, height, planesize);
  CorrectPremult (buffer, planesize);
}

struct DXTAlphaBlockExplicit
{
  uint16 row[4];
};

void Loader::DecompressDXT3(csRGBpixel* buffer, uint8* source,
			    int Width, int Height, uint32 planesize)
{
  int           x, y, z, i, j, k, Select;
  unsigned char *Temp;
  uint16       color_0, color_1;
  Color8888     colours[4];
  uint32        bitmask, Offset;
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

void Loader::DecompressDXT4 (csRGBpixel* buffer, uint8* source, 
			     int width, int height, uint32 planesize)
{
  // Can do color & alpha same as dxt5, but color is pre-multiplied
  // so the result will be wrong unless corrected.
  DecompressDXT5 (buffer, source, width, height, planesize);
  CorrectPremult (buffer, planesize);
}

void Loader::DecompressDXT5 (csRGBpixel* buffer, uint8* source, 
			     int Width, int Height, uint32 planesize)
{
  int             x, y, z, i, j, k, Select;
  uint8         *Temp;
  uint16       color_0, color_1;
  Color8888       colours[4];
  uint32          bitmask, Offset;
  uint8         alphas[8], *alphamask;
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

void Loader::CorrectPremult (csRGBpixel* buffer, uint32 planesize)
{
  uint32 size = (planesize * GetDepth()) / bpp;
  for (uint32 i = 0; i<size; i++)
  {
    const int a = buffer[i].alpha;
    if (!a) continue;
    buffer[i].red = (buffer[i].red << 8) / a;
    buffer[i].green = (buffer[i].green << 8) / a;
    buffer[i].blue = (buffer[i].blue << 8) / a;
  }
}
  
} // end of namespace dds
