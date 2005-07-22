/*
    Copyright (C) 1998 by Jorrit Tyberghein

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
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "gifimage.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csGIFImageIO)
  SCF_IMPLEMENTS_INTERFACE (iImageIO)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGIFImageIO::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csGIFImageIO)


static iImageIO::FileFormatDescription formatlist[2] =
{
  {"image/gif", "GIF87a", CS_IMAGEIO_LOAD},
  {"image/gif", "GIF89a", CS_IMAGEIO_LOAD}
};

csGIFImageIO::csGIFImageIO (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  formats.Push (&formatlist[0]);
  formats.Push (&formatlist[1]);
}

csGIFImageIO::~csGIFImageIO()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

const csImageIOFileFormatDescriptions& csGIFImageIO::GetDescription ()
{
  return formats;
}

csPtr<iImage> csGIFImageIO::Load (iDataBuffer* buf, int iFormat)
{
  ImageGifFile* i = new ImageGifFile (iFormat);
  if (i && !i->Load (buf->GetUint8(), buf->GetSize()))
  {
    delete i;
    return 0;
  }
  return csPtr<iImage> (i);
}

void csGIFImageIO::SetDithering (bool)
{
}

csPtr<iDataBuffer> csGIFImageIO::Save (iImage* image, iImageIO::FileFormatDescription* format,
  const char* extraoptions)
{
  image = 0;
  format = 0;
  extraoptions = 0;

  return 0;
}

csPtr<iDataBuffer> csGIFImageIO::Save (iImage* image, const char* mime,
  const char* extraoptions)
{
  image = 0;
  mime = 0;
  extraoptions = 0;

  return 0;
}

//---------------------------------------------------------------------------

#define IMAGESEP		0x2c
#define GRAPHIC_EXT		0xf9
#define PLAINTEXT_EXT		0x01
#define APPLICATION_EXT		0xff
#define COMMENT_EXT		0xfe
#define START_EXTENSION		0x21
#define INTERLACEMASK		0x40
#define COLORMAPMASK		0x80

#define GIF_BadExtension	1
#define GIF_OutCount		2
#define GIF_CodeSize		3
#define GIF_BadFormat		4

//---------------------------------------------------------------------------

class GIFStream
{
private:
  uint8 *buf, *ptr, *bmark;
  size_t size, remaining;
  uint8 bitoffs;
  bool EOFcode;

public:
  GIFStream(uint8* b, size_t fsize, size_t offs=0) :
   buf(b), ptr(b+offs), bmark(0), size(fsize), remaining(fsize-offs),
   bitoffs(0), EOFcode( fsize <= offs ) {}

  uint8 operator* () const
  { if (EOFcode) return 0;  else return *ptr; }
  uint8 operator[] (size_t i) const
  { if (i >= remaining) return 0;  else return ptr[i]; }
  GIFStream& operator++ ()
  { ptr++;  remaining--;  EOFcode = remaining<=0;  return *this; }
  GIFStream operator++ (int)
  { GIFStream t = *this;  ++(*this);  return t; }
  GIFStream& operator+= (int i)
  { ptr += i;  remaining -= i;  EOFcode = remaining<=0;  return *this; }
  bool isEOF() const { return EOFcode; }

  uint8 nextbyte() { return *(*this)++; }
  int nextword() { int r = nextbyte();  return ( nextbyte() << 8 ) + r; }
  int nextcode(uint8 codesize);
private:
  int getunblock();
};

int GIFStream::getunblock()
{
  if (!bmark) bmark = ptr;
  while ( (bmark <= ptr) && (bmark < buf+size) )
  {
   ++(*this);
   bmark += *bmark+1;
  }

  int r = *(*this);
  if (bmark > ptr+1) r += (*this)[1] << 8;
    else r += (*this)[2] << 8;
  if (bmark > ptr+2) r += (*this)[2] << 16;
    else r += (*this)[3] << 16;
  return r;
}

int GIFStream::nextcode(uint8 codesize)
{
  int code = getunblock();
  code = (code >> bitoffs) & ( (1<<codesize) - 1 );
  bitoffs += codesize;
  (*this) += bitoffs >> 3;
  bitoffs &= 7;
  return code;
}

//---------------------------------------------------------------------------

class GIFOutput
{
private:
  uint8 *img;
  int w, h, x, y;
  bool interlaced;
  int pass;

public:

  GIFOutput (int width, int height, bool ilace = false) : w (width), h (height),
    x (0), y (0), interlaced (ilace), pass (0)
  {
    img = new uint8 [width * height];
  }

  uint8& operator* () const
  { return *(img + y * w + x); }

  GIFOutput& operator++ ()
  {
    if (++x == w)
    {
      x = 0;
      if (!interlaced)
        y++;
      else
      {
        if (pass > 0 && pass < 4)
          y += 16 >> pass;
        else if (pass == 0)
          y += 8;
        if (y >= h && pass < 3)
          y = 1 << (2 - pass++);
      }
      if (y >= h)
        y = 0;
    } /* if (++x == w) */
    return *this;
  }

  uint8 *get_image () { return img; }
};

//---------------------------------------------------------------------------

class GIFPalette
{
private:
  csRGBcolor palette[256];
  int pal_size;
  int bitmask;

public:
  GIFPalette () : pal_size (0), bitmask (0) {}

  void Load (GIFStream& gptr, int size)
  {
    pal_size = (size < 0) ? 0 : (size > 256) ? 256 : size;
    bitmask = pal_size - 1;
	int i;
    for (i = 0; i < pal_size; i++)
    {
      palette[i].red = gptr.nextbyte ();
      palette[i].green = gptr.nextbyte ();
      palette[i].blue = gptr.nextbyte ();
    }
  }

  csRGBcolor operator () (int i)
  { return palette [i & bitmask]; }

  int get_size () const { return pal_size; }
  int get_mask () const { return bitmask; }

  csRGBcolor *get_palette () { return palette; }
};

//---------------------------------------------------------------------------

int ImageGifFile::decode_gif (uint8* iBuffer, size_t iSize, int* Prefix,
  int* Suffix, int* OutCode)
{
  GIFStream gptr (iBuffer,iSize);
  GIFPalette palette;
  uint8 ch;
  int is_transparent = 0;
  uint8 transp_index = 0;

  if (strncmp( (char*)iBuffer, "GIF87a", 6) && strncmp( (char*)iBuffer, "GIF89a", 6))
    return GIF_BadFormat;

  gptr += 6;

  // Get variables from the GIF screen descriptor

  gptr += 4;  // skip screen width, screen height (unused)

  ch = gptr.nextbyte();

  bool has_cmap = ch & COLORMAPMASK;
  int cmap_size = 1 << ( (ch & 7) + 1 );

  gptr += 2;  // skip 2 bytes : background color and '0'

  if (has_cmap) palette.Load (gptr, cmap_size);

  // look for image separator
  for (uint8 i = gptr.nextbyte() ; i != IMAGESEP ; i = gptr.nextbyte())
  {
    if (i != START_EXTENSION)
      return GIF_BadFormat;

    // handle image extensions
    switch (ch = gptr.nextbyte())
    {
      case GRAPHIC_EXT:
	ch = gptr.nextbyte();
	if (*gptr & 0x1)
	{
	  // image is transparent
	  is_transparent = 1;
	  // get transparent color index - (ch==4), so the 3'th byte
	  transp_index = gptr[3];
	  csRGBcolor tcol = palette(transp_index);
	  has_keycolour = true;
	  keycolour = tcol;
	}
	gptr += ch;
	break;
      case PLAINTEXT_EXT:
	break;
      case APPLICATION_EXT:
	break;
      case COMMENT_EXT:
	break;
      default:
        return GIF_BadExtension;
    }

    while ((ch = gptr.nextbyte()) != 0) gptr += ch;
  }

  // Now read in values from the image descriptor

  gptr += 4;  // skipping Left Offset, Top Offset

  int width  = gptr.nextword();
  int height = gptr.nextword();
  SetDimensions (width, height);

  GIFOutput optr(width, height, (gptr.nextbyte() & INTERLACEMASK));

  // Note that I ignore the possible existence of a local color map.
  // I'm told there aren't many files around that use them, and the spec
  // says it's defined for future use.  This could lead to an error
  // reading some files.

  // Start reading the raster data. First we get the intial code size
  // and compute decompressor constant values, based on this code size.
  // The GIF spec has it that the code size is the code size used to
  // compute the above values is the code size given in the file, but the
  // code size used in compression/decompression is the code size given in
  // the file plus one. (thus the ++).

  int code_size = gptr.nextbyte();       // bits per code
  int val_clear = 1 << code_size;        // code representing 'clear'
  int val_EOF = val_clear + 1;          // code representing 'EOF'
  int code_free = val_clear + 2;        //
  int first_free = code_free;            // initial value of code_free
  int first_size = ++code_size;          // initial value of code_size
  int code_max = val_clear << 1;         // upper bound for each code
  int OutCount = 0;

  if (code_size > 12)
    return GIF_CodeSize;

  // Decompress the file, continuing until you see the GIF EOF code.
  // One obvious enhancement is to add checking for corrupt files here.

  int code = gptr.nextcode(code_size);
  int code_write = 0, code_old = 0;

  while (code != val_EOF && !gptr.isEOF ())
  {
    if (code == val_clear)
    {
      // The Clear code sets everything back to its initial value,
      // then reads the subsequent code as uncompressed data.

      code_size = first_size;
      code_max = 1 << code_size;
      code_free = first_free;
      code_write = code_old = code = gptr.nextcode(code_size);
      *optr = code_write;
      ++optr;
    }
    else
    {
      // If it's not a Clear, then it must be data.
      int code_in = code;

      // If it's >= code_free, then it's not in the hash table yet;
      // repeat the last character decoded.

      if (OutCount > 1024)
        return GIF_OutCount;

      if (code >= code_free)
      {
        code = code_old;
        OutCode[OutCount++] = code_write;
      }

      // Unless this code is raw data, pursue the chain pointed to by
      // code_cur through the hash table to its end;  each code in the
      // chain puts its associated output code on the output queue.

      while (code > palette.get_mask())
      {
        if (OutCount > 1024)
          return GIF_OutCount;

        OutCode[OutCount++] = Suffix[code];
        code = Prefix[code];
      }

      // The last code in the chain is treated as raw data.

      if (OutCount > 1024)
        return GIF_OutCount;

      OutCode[OutCount++] = code;

      // Now we put the data out to the image buffer.
      // It's been stacked LIFO, so deal with it that way...

      int j;
      for (j = OutCount - 1; j >= 0; j--)
      {
        *optr = OutCode [j];
        ++optr;
      }
      OutCount = 0;

      // Build the hash table on-the-fly. No table is stored in the file.

      Prefix[code_free] = code_old;
      Suffix[code_free] = code_write = code;
      code_old = code_in;

      // Point to the next slot in the table.  If we exceed the current
      // max code value, increment the code size unless it's already 12.
      // If the size is already 12, do nothing:  the next code better be
      // CLEAR.

      if (++code_free >= code_max)
      {
        if (code_size < 12) { code_size++;  code_max <<= 1; }
      }
    }
    code = gptr.nextcode(code_size);
  }

  Format &= ~CS_IMGFMT_ALPHA;
  ConvertFromPal8 (optr.get_image (), 0, palette.get_palette ());

  return 0;
}

bool ImageGifFile::Load (uint8* iBuffer, size_t iSize)
{
  int* Prefix  = new int [4096]; // Hash table used by decompressor.
  int* Suffix  = new int [4096]; // Hash table used by decompressor.
  int* OutCode = new int [1025]; // Output array used by decompressor.

  int rc = decode_gif (iBuffer, iSize, Prefix, Suffix, OutCode);

  delete [] Prefix;
  delete [] Suffix;
  delete [] OutCode;

  return (rc == 0);
}
