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
  delete[] header;
}

void Loader::SetSource (void* buffer, size_t bufferlen)
{
  source = (uint8*) buffer;
  sourcelen = bufferlen;
  header = 0;
}

bool Loader::IsDDS ()
{
  uint32 magic = little_endian_long( *((uint32*) source));
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

uint8* Loader::LoadImage ()
{
  uint8* buffer = new uint8[GetWidth() * GetHeight() * GetDepth() * bpp];

  // Data starts after header (header size is minus magic)
  Decompress (buffer, positions[0], GetWidth(), GetHeight(), 
	      GetWidth() * GetHeight() * bpp);

  return buffer;
}

uint8* Loader::LoadMipmap (int number)
{
  if (number >= GetMipmapCount())
    return 0;

  int width = GetWidth() >> (number+1);
  int height = GetHeight() >> (number+1);

  uint8* buffer = new uint8[width * height * bpp];
  Decompress (buffer, positions[number+1], width, height,
	      width * height * bpp);

  return buffer;
}

uint32 Loader::GetUInt32()
{
  uint32 res = little_endian_long (*((uint32*) readpos));
  readpos += sizeof(uint32);

  return res;
}

bool Loader::ReadHeader ()
{
  if (sourcelen < sizeof(Header))
    return false;

  int i;

  readpos = source;

  delete[] header;
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

  // Adjust linearsize
  if (header->flags & FLAG_PITCH)
    header->linearsize *= header->height;

  // Calculate Positions of the image Data
  delete[] positions;
  positions = new uint8* [GetMipmapCount() + 1];
  positions[0] = readpos;
  uint32 add = header->linearsize;
  for (i=1;i<GetMipmapCount()+1;i++)
  {
    positions[i] = positions[i-1]+add;
    if (format == FORMAT_RGBA || format == FORMAT_RGB)
      add /= 4;
    else if (format == FORMAT_DXT1)
      add = MAX(add/4, 8);
    else
      add = MAX(add/4, 16);
  }
  if ((size_t)(positions[GetMipmapCount()] - positions[0] + add) > sourcelen)
  {
    printf ("DDS Image too small Needs:%u Has: %lu.\n",
	(unsigned int)(positions[GetMipmapCount()] - positions[0] + add),
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
      blocksize = GetWidth() * GetHeight() * header->pixelformat.bitdepth * 4;
      format = FORMAT_RGBA;
    }
    else
    {
      blocksize = GetWidth() * GetHeight() * header->pixelformat.bitdepth * 3;
      format = FORMAT_RGB;
      bpp = 3;
    }
  }

  depth = header->depth ? header->depth : 1;
}

void Loader::Decompress (uint8* buffer, uint8* source, int width, int height,
			 uint32 planesize)
{
  switch (format)
  {
    case FORMAT_RGB:
    case FORMAT_RGBA:
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
}

struct Color8888
{
  unsigned char r,g,b,a;
};

struct Color888
{
  unsigned char r,g,b;
};

struct Color565
{
  unsigned nBlue : 5;
  unsigned nGreen : 6;
  unsigned nRed : 5;
};

void Loader::DecompressRGBA (uint8* buffer, uint8* source, int, int, 
			     uint32 size)
{
  memcpy (buffer, source, (size_t) size);
}

void Loader::DecompressDXT1 (uint8* buffer, uint8* source, 
			     int Width, int Height, uint32 planesize)
{
  int          x, y, z, i, j, k, Select;
  unsigned char *Temp;
  Color565     *color_0, *color_1;
  Color8888    colours[4], *col;
  uint32       bitmask, Offset;

  int bps = Width * bpp;

  Temp = (unsigned char*) source;
  for (z = 0; z < depth; z++) {
    for (y = 0; y < Height; y += 4) {
      for (x = 0; x < Width; x += 4) {

	color_0 = ((Color565*)Temp);
	color_1 = ((Color565*)(Temp+2));
	bitmask = ((uint32*)Temp)[1];
	Temp += 8;

	colours[0].r = color_0->nRed << 3;
	colours[0].g = color_0->nGreen << 2;
	colours[0].b = color_0->nBlue << 3;
	colours[0].a = 0xFF;

	colours[1].r = color_1->nRed << 3;
	colours[1].g = color_1->nGreen << 2;
	colours[1].b = color_1->nBlue << 3;
	colours[1].a = 0xFF;


	if (*((uint16*)color_0) > *((uint16*)color_1)) {
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
	else {
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

	for (j = 0, k = 0; j < 4; j++) {
	  for (i = 0; i < 4; i++, k++) {

	    Select = (bitmask & (0x03 << k*2)) >> k*2;
	    col = &colours[Select];

	    if (((x + i) < Width) && ((y + j) < Height)) {
	      Offset = z * planesize + (y + j) * bps + (x + i) * bpp;
	      buffer[Offset + 0] = col->r;
	      buffer[Offset + 1] = col->g;
	      buffer[Offset + 2] = col->b;
	      buffer[Offset + 3] = col->a;
	    }
	  }
	}
      }
    }
  }

  return;
}

void Loader::DecompressDXT2(uint8* buffer, uint8* source, int width,
			    int height, uint32 planesize)
{
  // Can do color & alpha same as dxt3, but color is pre-multiplied
  //   so the result will be wrong unless corrected.
  DecompressDXT3(buffer, source, width, height, planesize);
  CorrectPremult (buffer, planesize);
}

struct DXTAlphaBlockExplicit
{
  uint16 row[4];
};

void Loader::DecompressDXT3(uint8* buffer, uint8* source,
			    int Width, int Height, uint32 planesize)
{
  int           x, y, z, i, j, k, Select;
  unsigned char *Temp;
  Color565      *color_0, *color_1;
  Color8888     colours[4], *col;
  uint32        bitmask, Offset;
  uint16	word;
  DXTAlphaBlockExplicit *alpha;

  int bps = Width * bpp;

  Temp = (unsigned char*) source;
  for (z = 0; z < depth; z++) {
    for (y = 0; y < Height; y += 4) {
      for (x = 0; x < Width; x += 4) {
	alpha = (DXTAlphaBlockExplicit*)Temp;
	Temp += 8;
	color_0 = ((Color565*)Temp);
	color_1 = ((Color565*)(Temp+2));
	bitmask = ((uint32*)Temp)[1];
	Temp += 8;

	colours[0].r = color_0->nRed << 3;
	colours[0].g = color_0->nGreen << 2;
	colours[0].b = color_0->nBlue << 3;
	colours[0].a = 0xFF;

	colours[1].r = color_1->nRed << 3;
	colours[1].g = color_1->nGreen << 2;
	colours[1].b = color_1->nBlue << 3;
	colours[1].a = 0xFF;

	// Four-color block: derive the other two colors.
	// 00 = color_0, 01 = color_1, 10 = color_2, 11	= color_3
	  // These 2-bit codes correspond to the 2-bit fields
	  // stored in the 64-bit block.
	  colours[2].b = (2 * colours[0].b + colours[1].b
	      + 1) / 3;
	colours[2].g = (2 * colours[0].g + colours[1].g
	    + 1) / 3;
	colours[2].r = (2 * colours[0].r + colours[1].r
	    + 1) / 3;
	colours[2].a = 0xFF;

	colours[3].b = (colours[0].b + 2 * colours[1].b
	    + 1) / 3;
	colours[3].g = (colours[0].g + 2 * colours[1].g
	    + 1) / 3;
	colours[3].r = (colours[0].r + 2 * colours[1].r
	    + 1) / 3;
	colours[3].a = 0xFF;

	k = 0;
	for (j = 0; j < 4; j++) {
	  for (i = 0; i < 4; i++, k++) {

	    Select = (bitmask & (0x03 << k*2)) >> k*2;
	    col = &colours[Select];

	    if (((x + i) < Width) && ((y + j) < Height)) {
	      Offset = z * planesize + (y + j) * bps + (x + i) * bpp;
	      buffer[Offset + 0] = col->r;
	      buffer[Offset + 1] = col->g;
	      buffer[Offset + 2] = col->b;
	    }
	  }
	}

	for (j = 0; j < 4; j++) {
	  word = alpha->row[j];
	  for (i = 0; i < 4; i++) {
	    if (((x + i) < Width) && ((y + j) < Height)) {
	      Offset = z * planesize + (y + j) * bps + (x + i) * bpp + 3;
	      buffer[Offset] = word & 0x0F;
	      buffer[Offset] |= (buffer[Offset] << 4);
	    }
	    word >>= 4;
	  }
	}

      }
    }
  }

  return;
}

void Loader::DecompressDXT4 (uint8* buffer, uint8* source, 
			     int width, int height, uint32 planesize)
{
  // Can do color & alpha same as dxt5, but color is pre-multiplied
  //   so the result will be wrong unless corrected.
  DecompressDXT5 (buffer, source, width, height, planesize);
  CorrectPremult (buffer, planesize);
}

void Loader::DecompressDXT5 (uint8* buffer, uint8* source, 
			     int Width, int Height, uint32 planesize)
{
  int             x, y, z, i, j, k, Select;
  uint8         *Temp;
  Color565        *color_0, *color_1;
  Color8888       colours[4], *col;
  uint32          bitmask, Offset;
  uint8         alphas[8], *alphamask;
  uint32          bits;

  int bps = Width * bpp;                                    

  Temp = source;
  for (z = 0; z < depth; z++) {
    for (y = 0; y < Height; y += 4) {
      for (x = 0; x < Width; x += 4) {
	if (y >= Height || x >= Width)
	  break;
	alphas[0] = Temp[0];
	alphas[1] = Temp[1];
	alphamask = Temp + 2;
	Temp += 8;
	color_0 = ((Color565*)Temp);
	color_1 = ((Color565*)(Temp+2));
	bitmask = ((uint32*)Temp)[1];
	Temp += 8;

	colours[0].r = color_0->nRed << 3;
	colours[0].g = color_0->nGreen << 2;
	colours[0].b = color_0->nBlue << 3;
	colours[0].a = 0xFF;

	colours[1].r = color_1->nRed << 3;
	colours[1].g = color_1->nGreen << 2;
	colours[1].b = color_1->nBlue << 3;
	colours[1].a = 0xFF;
	// Four-color block: derive the other two colors.
	// 00 = color_0, 01 = color_1, 10 = color_2, 11	= color_3
	  // These 2-bit codes correspond to the 2-bit fields
	  // stored in the 64-bit block.
	colours[2].b = (2 * colours[0].b + colours[1].b + 1) / 3;
	colours[2].g = (2 * colours[0].g + colours[1].g + 1) / 3;
	colours[2].r = (2 * colours[0].r + colours[1].r + 1) / 3;
	colours[2].a = 0xFF;

	colours[3].b = (colours[0].b + 2 * colours[1].b
	    + 1) / 3;
	colours[3].g = (colours[0].g + 2 * colours[1].g
	    + 1) / 3;
	colours[3].r = (colours[0].r + 2 * colours[1].r
	    + 1) / 3;
	colours[3].a = 0xFF;

	k = 0;
	for (j = 0; j < 4; j++) {
	  for (i = 0; i < 4; i++, k++) {

	    Select = (bitmask & (0x03 << k*2)) >> k*2;
	    col = &colours[Select];

	    // only put pixels out < width or height
	    if (((x + i) < Width) && ((y + j) < Height)) {
	      Offset = z * planesize + (y + j) * bps + (x + i) * bpp;
	      buffer[Offset + 0] = col->r;
	      buffer[Offset + 1] = col->g;
	      buffer[Offset + 2] = col->b;
	    }
	  }
	}

	// 8-alpha or 6-alpha block?
	if (alphas[0] > alphas[1]) {
	  // 8-alpha block:  derive the other six alphas.
	  // Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated.
	  alphas[2] = (6 * alphas[0] + 1 * alphas[1] + 3) / 7;    // bit code 010
	  alphas[3] = (5 * alphas[0] + 2 * alphas[1] + 3) / 7;    // bit code 011
	  alphas[4] = (4 * alphas[0] + 3 * alphas[1] + 3) / 7;    // bit code 100
	  alphas[5] = (3 * alphas[0] + 4 * alphas[1] + 3) / 7;    // bit code 101
	  alphas[6] = (2 * alphas[0] + 5 * alphas[1] + 3) / 7;    // bit code 110
	  alphas[7] = (1 * alphas[0] + 6 * alphas[1] + 3) / 7;    // bit code 111
	}
	else {
	  // 6-alpha block.
	  // Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated.
	  alphas[2] = (4 * alphas[0] + 1 * alphas[1] + 2) / 5;    // Bit code 010
	  alphas[3] = (3 * alphas[0] + 2 * alphas[1] + 2) / 5;    // Bit code 011
	  alphas[4] = (2 * alphas[0] + 3 * alphas[1] + 2) / 5;    // Bit code 100
	  alphas[5] = (1 * alphas[0] + 4 * alphas[1] + 2) / 5;    // Bit code 101
	  alphas[6] = 0x00;
	  // Bit code 110
	  alphas[7] = 0xFF;
	  // Bit code 111
	}

	// Note: Have to separate the next two loops,
	//      it operates on a 6-byte system.

	// First three bytes
	bits = *((int32*)alphamask);
	for (j = 0; j < 2; j++) {
	  for (i = 0; i < 4; i++) {
	    // only put pixels out < width or height
	    if (((x + i) < Width) && ((y + j) < Height)) {
	      Offset = z * planesize + (y + j) * bps + (x + i) * bpp + 3;
	      buffer[Offset] = alphas[bits & 0x07];
	    }
	    bits >>= 3;
	  }
	}

	// Last three bytes
	bits = *((int32*)&alphamask[3]);
	for (j = 2; j < 4; j++) {
	  for (i = 0; i < 4; i++) {
	    // only put pixels out < width or height
	    if (((x + i) < Width) && ((y + j) < Height)) {
	      Offset = z * planesize + (y + j) * bps + (x + i) * bpp + 3;
	      buffer[Offset] = alphas[bits & 0x07];
	    }
	    bits >>= 3;
	  }
	}
      }
    }
  }
}

void Loader::CorrectPremult (uint8* buffer, uint32 planesize)
{
  uint32 size = planesize * GetDepth();
  for (uint32 i=0;i<size;i+=4)
  {
    if (!buffer[i+3]) continue;
    buffer[i] = (uint8) (((uint32) buffer[i] << 8) / buffer[i+3]);
    buffer[i+1] = (uint8) (((uint32) buffer[i+1] << 8) / buffer[i+3]);
    buffer[i+2] = (uint8) (((uint32) buffer[i+2] << 8) / buffer[i+3]);
  }
}
  
} // end of namespace dds

