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

#include <math.h>
#include <stdio.h>

#include "cssysdef.h"
#include "sgiimage.h"

CS_IMPLEMENT_PLUGIN

IMPLEMENT_IBASE (csSGIImageIO)
  IMPLEMENTS_INTERFACE (iImageIO)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csSGIImageIO);

EXPORT_CLASS_TABLE (cssgiimg)
  EXPORT_CLASS (csSGIImageIO, "crystalspace.graphic.image.io.sgi", "CrystalSpace SGI image format I/O plugin")
EXPORT_CLASS_TABLE_END

static iImageIO::FileFormatDescription formatlist[2] = 
{
  {"image/sgi", "RGB", CS_IMAGEIO_LOAD},
  {"image/sgi", "RLE", CS_IMAGEIO_LOAD}
};

csSGIImageIO::csSGIImageIO (iBase *pParent)
{
  CONSTRUCT_IBASE (pParent);
  formats.Push (&formatlist[0]);
  formats.Push (&formatlist[1]);
}

bool csSGIImageIO::Initialize (iSystem *)
{
  return true;
}

const csVector& csSGIImageIO::GetDescription ()
{
  return formats;
}

iImage *csSGIImageIO::Load (UByte* iBuffer, ULong iSize, int iFormat)
{
  ImageSGIFile* i = new ImageSGIFile (iFormat);
  if (i && !i->Load (iBuffer, iSize))
  {
    delete i;
    return NULL;
  }
  return i;
}

void csSGIImageIO::SetDithering (bool)
{
}

iDataBuffer *csSGIImageIO::Save (iImage *, iImageIO::FileFormatDescription *)
{
  return NULL;
}

iDataBuffer *csSGIImageIO::Save (iImage *, const char *)
{
  return NULL;
}

//---------------------------------------------------------------------------

// some docs: http://www.cica.indiana.edu/graphics/image_specs/rgb.format.txt

static struct SGIHeader
{
  UShort Magic;		// Magic id
  UByte Packed;		// Using rle or straight format
  UByte Bpc;		// Bytes per channel
  UShort Dimension;	// 1=one scanline per channel
			// 2=many lines per channel
			// 3=entire picture for each channel
  UShort Width;		// Width
  UShort Height;	// Height
  UShort Channels;	// Depth (bytes per pixel)
  ULong Pixmin;		// Seems unused
  ULong Pixmax;		// Seems unused
  ULong Dummy;		// Max value for R/G/B components?
  UByte Imagename[80];	// if someone would bother to fill it...
  ULong Colormap;	// who knows... maybe support for paletted images?
} header;

//---------------------------------------------------------------------------

bool ImageSGIFile::Load (UByte* iBuffer, ULong iSize)
{
  (void)iSize;

  UInt planes = readHeader (iBuffer);
  if (planes !=3 && planes != 4)
    return false;

  set_dimensions (header.Width, header.Height);
  csRGBpixel *buffer = new csRGBpixel [Width * Height];

  UByte *line = new UByte [Width];

  ULong *starttable = NULL;
  ULong *lengthtable = NULL;

  // Indicates if it's rle encoded or not
  if (header.Packed)
  {
    starttable = new ULong [Height * header.Channels];
    lengthtable = new ULong [Height * header.Channels];

    loadSGITables (iBuffer + 512, starttable, Height * header.Channels);
    loadSGITables (iBuffer + 512 + (sizeof (ULong) * Height * header.Channels),
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

  convert_rgba (buffer);

  // Check if the alpha channel is valid
  CheckAlpha ();

  return true;
}

static inline ULong getLong (UByte *p)
{
  return ((*(p)<<24)|(*(p+1)<<16)|(*(p+2)<<8)|(*(p+3)));
}

static inline UShort getShort (UByte *p)
{
  return (((*p)<<8)|(*(p+1)));
}

void ImageSGIFile::loadSGITables(UByte *in,ULong *out,int size)
{
  for (int i = 0; i < size; i++)
  {
    out [i] = getLong (in);
    in += 4;
  }
}

int ImageSGIFile::decode_rle (UByte *src, ULong length, UByte *dst)
{
  int size = 0;
  UByte count = 0;

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

UInt ImageSGIFile::readHeader (UByte *iBuffer)
{
  UByte *tmpPtr = iBuffer;

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
