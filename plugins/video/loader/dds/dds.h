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
#ifndef __DDS_DDS_H__
#define __DDS_DDS_H__

#include "csgfx/rgbpixel.h"

namespace dds
{

enum
{
  // Flags for Header
  FLAG_CAPS	   = 0x00000001,
  FLAG_HEIGHT	   = 0x00000002,
  FLAG_WIDTH	   = 0x00000004,
  FLAG_PITCH	   = 0x00000010,
  FLAG_PIXELFORMAT = 0x00001000,
  FLAG_MIPMAPCOUNT = 0x00020000,
  FLAG_LINEARSIZE  = 0x00080000,
  FLAG_DEPTH	   = 0x00800000,

  // Flags for Pixelformats
  FLAG_ALPHAPIXEL  = 0x00000001,
  FLAG_FOURCC	   = 0x00000004,
  FLAG_RGB	   = 0x00000040,
  
  // Flags for complex caps
  FLAG_COMPLEX	   = 0x00000008,
  FLAG_TEXTURE	   = 0x00001000,
  FLAG_MIPMAP	   = 0x00400000,

  // Flags for cubemaps
  FLAG_CUBEMAP	   = 0x00000200,
  FLAG_CUBEMAP_POSITIVEX = 0x00000400,
  FLAG_CUBEMAP_NEGATIVEX = 0x00000800,
  FLAG_CUBEMAP_POSITIVEY = 0x00001000,
  FLAG_CUBEMAP_NEGATIVEY = 0x00002000,
  FLAG_CUBEMAP_POSITIVEZ = 0x00004000,
  FLAG_CUBEMAP_NEGATIVEZ = 0x00008000,
  FLAG_VOLUME		 = 0x00200000
};

enum
{
  FORMAT_UNKNOWN = 0,
  FORMAT_DXT1,
  FORMAT_DXT2,
  FORMAT_DXT3,
  FORMAT_DXT4,
  FORMAT_DXT5,
  FORMAT_RGB,
  FORMAT_RGBA
};

struct PixelFormat
{
  uint32 size;      // size of this structure (should be 32)

  uint32 flags;     // flags which values are present here
  uint32 fourcc;    // character code for image format
  uint32 bitdepth;  // number of bits per pixel (usually 16,24 or 32)
  
  uint32 redmask;   // mask for the red pixels (usually 0x00ff0000)
  uint32 bluemask;  // mask for the green pixels (usually 0x0000ff00)
  uint32 greenmask; // mask for the blue pixels (usually 0x000000ff)
  
  uint32 alphamask; // mask for the alpha value in a pixel
			      // (usually 0xff000000)
};

struct Capabilities
{
  uint32 caps1;
  uint32 caps2;
  uint32 caps3;
  uint32 caps4;
  uint32 texturestage;
};

struct Header
{
  uint32 magic;     // Magic Number (has to be "DDS "
  
  uint32 size;	    // Size of the descriptor structure (should be 124)

  uint32 flags;     // flags field
  uint32 height;
  uint32 width;

  uint32 linearsize;

  uint32 depth;	    // for volume textures: depth the FLAG_DEPTH flag
		    // should be set in this case
  uint32 mipmapcount; // number of mipmap levels included. flags
		      // should include FLAG_MIPMAPCOUNT in this case
  uint32 alphabitdepth;	// depth of alpha buffer
  
  uint32 reserved[10];
  
  PixelFormat pixelformat;
  Capabilities capabilities;

  uint32 reserved001;
};

class Loader
{
public:
  Loader ();
  ~Loader ();
  
  void SetSource (void* buffer, size_t bufferlen);

  bool IsDDS ();	  // Readsin the Header and tests if it is a dds file
  int GetWidth ();	  // returns width of the image
  int GetHeight ();	  // returns height of the image
  int GetBytesPerPixel (); // return bits per pixel
  int GetDepth ();	  // returns depth of the image (for volume/3d images)
  int GetMipmapCount ();  // returns number of contained mipmap levels
  
  // Loads the main Image and returns a buffer which has to be deleted with
  // delete[] later.
  // Note that you MUST call IsDDS before calling this function
  csRGBpixel* LoadImage ();

  // Loads mipmap number n. Returns a buffer which has to be deleted with
  // delete[] later
  csRGBpixel* LoadMipmap (int n);

  int GetFormat () const { return format; }
private:
  bool ReadHeader ();
  
  void CheckFormat ();

  uint32 GetUInt32 ();

  bool Decompress (csRGBpixel* buffer, uint8* source, int w, int h, 
    uint32 size);
  void DecompressRGBA (csRGBpixel* buffer, uint8* source, int w, int h, 
    uint32 size);
  void DecompressDXT1 (csRGBpixel* buffer, uint8* source, int w, int h, 
    uint32 size);
  void DecompressDXT2 (csRGBpixel* buffer, uint8* source, int w, int h, 
    uint32 size);
  void DecompressDXT3 (csRGBpixel* buffer, uint8* source, int w, int h, 
    uint32 size);
  void DecompressDXT4 (csRGBpixel* buffer, uint8* source, int w, int h, 
    uint32 size);
  void DecompressDXT5 (csRGBpixel* buffer, uint8* source, int w, int h, 
    uint32 size);

  void CorrectPremult (csRGBpixel* buffer, uint32 planesize);

  int format, blocksize;
  int depth; // depth of volume textures
  int bpp; // bytes per pixel

  uint8* source;
  uint8* readpos;
  uint8** positions;
  size_t sourcelen;
  Header* header;
  
  const char* lasterror;
};

} // end of namespace dds

#endif

