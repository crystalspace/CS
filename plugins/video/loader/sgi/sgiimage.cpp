/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Robert Bergkvist (FragDance)
    Major fixes by Andrew Zabolotny

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
#include <stdio.h>

#include "sgiimage.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csSGIImageIO)
  SCF_IMPLEMENTS_INTERFACE (iImageIO)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSGIImageIO::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csSGIImageIO)


static iImageIO::FileFormatDescription formatlist[2] =
{
  {"image/sgi", "RGB", CS_IMAGEIO_LOAD},
  {"image/sgi", "RLE", CS_IMAGEIO_LOAD}
};

csSGIImageIO::csSGIImageIO (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  formats.Push (&formatlist[0]);
  formats.Push (&formatlist[1]);
}

csSGIImageIO::~csSGIImageIO()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

const csImageIOFileFormatDescriptions& csSGIImageIO::GetDescription ()
{
  return formats;
}

csPtr<iImage> csSGIImageIO::Load (iDataBuffer* buf, int iFormat)
{
  ImageSGIFile* i = new ImageSGIFile (iFormat);
  if (i && !i->Load (buf->GetUint8(), buf->GetSize()))
  {
    delete i;
    return 0;
  }
  return csPtr<iImage> (i);
}

void csSGIImageIO::SetDithering (bool)
{
}

csPtr<iDataBuffer> csSGIImageIO::Save (iImage* image,
  iImageIO::FileFormatDescription* format, const char* extraoptions)
{
  // Following block to kill compiler warnings ONLY
  image = 0;
  format = 0;
  extraoptions = 0;

  return 0;
}

csPtr<iDataBuffer> csSGIImageIO::Save (iImage* image, const char* mime,
  const char* extraoptions)
{
  // Following block to kill compiler warnings ONLY
  image = 0;
  mime = 0;
  extraoptions = 0;

  return 0;
}

//---------------------------------------------------------------------------

// some docs: http://www.cica.indiana.edu/graphics/image_specs/rgb.format.txt

static struct SGIHeader
{
  uint16 Magic;		// Magic id
  uint8 Packed;		// Using rle or straight format
  uint8 Bpc;		// Bytes per channel
  uint16 Dimension;	// 1=one scanline per channel
			// 2=many lines per channel
			// 3=entire picture for each channel
  uint16 Width;		// Width
  uint16 Height;	// Height
  uint16 Channels;	// Depth (bytes per pixel)
  uint32 Pixmin;		// Seems unused
  uint32 Pixmax;		// Seems unused
  uint32 Dummy;		// Max value for R/G/B components?
  uint8 Imagename[80];	// if someone would bother to fill it...
  uint32 Colormap;	// who knows... maybe support for paletted images?
} header;

//---------------------------------------------------------------------------

bool ImageSGIFile::Load (uint8* iBuffer, size_t iSize)
{
  (void)iSize;

  uint planes = readHeader (iBuffer);
  if (planes !=3 && planes != 4)
    return false;

  SetDimensions (header.Width, header.Height);
  csRGBpixel *buffer = new csRGBpixel [Width * Height];

  uint8 *line = new uint8 [Width];

  uint32 *starttable = 0;
  uint32 *lengthtable = 0;

  // Indicates if it's rle encoded or not
  if (header.Packed)
  {
    starttable = new uint32 [Height * header.Channels];
    lengthtable = new uint32 [Height * header.Channels];

    loadSGITables (iBuffer + 512, starttable, Height * header.Channels);
    loadSGITables (iBuffer + 512 + (sizeof (uint32) * Height * header.Channels),
      lengthtable, Height * header.Channels);
  }
  else
    iBuffer += 512;

  int i, j, maxi = Height * header.Channels;
  for (i = 0; i < maxi; i++)
  {
    if (header.Packed)
    {
      if (decode_rle (iBuffer + starttable [i], lengthtable [i], line) != Width)
        break;
    }
    else
    {
      memcpy (line, iBuffer, header.Width);
      iBuffer += header.Width;
    }

    csRGBpixel *d;
    int comp;
    if (header.Dimension == 3)
    {
      comp = i / Height;
      d = buffer + (Height - 1 - (i % Height)) * Width;
    }
    else
    {
      comp = i % header.Channels;
      d = buffer + (Height - 1 - (i / header.Channels)) * Width;
    }

    switch (comp)
    {
      case 0:
        for (j = 0; j < Width; j++, d++)
          d->red = line [j];
        break;
      case 1:
        for (j = 0; j < Width; j++, d++)
          d->green = line [j];
        break;
      case 2:
        for (j = 0; j < Width; j++, d++)
          d->blue = line [j];
        break;
    } /* endswitch */
  }

  delete [] starttable;
  delete [] lengthtable;

  delete [] line;

  if (i < maxi)
  {
    delete [] buffer;
    return false;
  }

  ConvertFromRGBA (buffer);

  // Check if the alpha channel is valid
  CheckAlpha ();

  return true;
}

static inline uint32 getLong (uint8 *p)
{
  return ((*(p)<<24)|(*(p+1)<<16)|(*(p+2)<<8)|(*(p+3)));
}

static inline uint16 getShort (uint8 *p)
{
  return (((*p)<<8)|(*(p+1)));
}

void ImageSGIFile::loadSGITables(uint8 *in,uint32 *out,int size)
{
  int i;
  for (i = 0; i < size; i++)
  {
    out [i] = getLong (in);
    in += 4;
  }
}

int ImageSGIFile::decode_rle (uint8 *src, uint32 length, uint8 *dst)
{
  int size = 0;
  uint8 count = 0;

  while (length-- && (count = *src++))
  {
    if (count & 0x80)
    {
      // uncompressed data follows
      count &= 0x7f;
      memcpy (dst, src, count);
      src += count;
      if (length >= count)
        length -= count;
      else
        break;
    }
    else
    {
      // repeat next byte count times
      memset (dst, *src++, count);
      length--;
    }
    dst += count;
    size += count;
  }
  return size;
}

uint ImageSGIFile::readHeader (uint8 *iBuffer)
{
  uint8 *tmpPtr = iBuffer;

  // This could propably be done better
  header.Magic = getShort (tmpPtr); tmpPtr += 2;
  if (header.Magic != 474) return false;
  header.Packed = (*(tmpPtr)); tmpPtr++;
  header.Bpc = (*(tmpPtr)); tmpPtr++;
  header.Dimension = getShort (tmpPtr); tmpPtr += 2;
  header.Width = getShort (tmpPtr); tmpPtr += 2;
  header.Height = getShort (tmpPtr); tmpPtr += 2;
  header.Channels = getShort (tmpPtr); tmpPtr += 2;
  return (header.Channels);
}
