/*
    Copyright (C) 1998-2003 by Jorrit Tyberghein

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

/* rd_tga.c - read a TrueVision Targa file
**
** Partially based on tgatoppm.c from pbmplus (just a big hack :)
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "cssysdef.h"
#include <math.h>
#include <stdio.h>

#include "tgaimage.h"
#include "csgfx/rgbpixel.h"
#include "csgfx/packrgb.h"
#include "csutil/databuf.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csTGAImageIO)
  SCF_IMPLEMENTS_INTERFACE (iImageIO)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTGAImageIO::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csTGAImageIO)

typedef char ImageIDField[256];

/* Definitions for image types. */
#define TGA_Null	0
#define TGA_Map		1
#define TGA_RGB		2
#define TGA_Mono	3
#define TGA_RLEMap	9
#define TGA_RLERGB     10
#define TGA_RLEMono    11
#define TGA_CompMap    32
#define TGA_CompMap4   33

/* Definitions for interleave flag. */
#define TGA_IL_None   0x00
#define TGA_IL_Two    0x40
#define TGA_IL_Four   0x80
#define TGA_IL_MASK   0xc0

#define TGA_Org_MASK  0x20
#define TGA_Org_BL    0x00
#define TGA_Org_TL    0x20

#define CSTGA_ID "Made with Crystal Space, see http://www.crystalspace3d.org/"
#define TGA_MIME "image/tga"

static iImageIO::FileFormatDescription formatlist[6] =
{
  {TGA_MIME, "Map", CS_IMAGEIO_LOAD},
  {TGA_MIME, "RGB", CS_IMAGEIO_LOAD|CS_IMAGEIO_SAVE},
  {TGA_MIME, "Mono", CS_IMAGEIO_LOAD},
  {TGA_MIME, "RLEMap", CS_IMAGEIO_LOAD|CS_IMAGEIO_SAVE},
  {TGA_MIME, "RLERGB", CS_IMAGEIO_LOAD},
  {TGA_MIME, "RLEMono", CS_IMAGEIO_LOAD}
};

csTGAImageIO::csTGAImageIO (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  formats.Push (&formatlist[0]);
  formats.Push (&formatlist[1]);
  formats.Push (&formatlist[2]);
  formats.Push (&formatlist[3]);
  formats.Push (&formatlist[4]);
  formats.Push (&formatlist[5]);
}

csTGAImageIO::~csTGAImageIO()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

const csImageIOFileFormatDescriptions& csTGAImageIO::GetDescription ()
{
  return formats;
}

csPtr<iImage> csTGAImageIO::Load (iDataBuffer* buf, int iFormat)
{
  ImageTgaFile* i = new ImageTgaFile (object_reg, iFormat);
  if (i && !i->Load (buf))
  {
     delete i ;
    return 0;
  }
  return csPtr<iImage> (i);
}

void csTGAImageIO::SetDithering (bool)
{
}

csPtr<iDataBuffer> csTGAImageIO::Save (iImage *Image, iImageIO::FileFormatDescription *,
  const char* extraoptions)
{
  extraoptions = 0;

  if (!Image || !Image->GetImageData ())
    return 0;

  int format = Image->GetFormat ();
  bool palette = false;

  switch (format & CS_IMGFMT_MASK)
  {
  case CS_IMGFMT_PALETTED8:
    palette = true;
  case CS_IMGFMT_TRUECOLOR:
    break;
  default:
    return 0;
  };

  if (palette && !Image->GetPalette ())
    return 0;

  // fill header
  TGAheader hdr;
  int w = Image->GetWidth ();
  int h = Image->GetHeight ();
  bool has_alpha = Image->GetFormat() & CS_IMGFMT_ALPHA;

  hdr.IDLength  = (uint8)(strlen (CSTGA_ID) + 1);
  hdr.CoMapType = (palette ? 1 : 0);
  hdr.ImgType = (palette ? TGA_Map : TGA_RGB);
  hdr.Index= 0;
  hdr.Length = (palette ? 256 : 0);
  hdr.CoSize = (palette ? 24 : 0);
  hdr.X_org = 0;
  hdr.Y_org = 0;
  hdr.Width = w;
  hdr.Height = h;
  hdr.PixelSize = (palette ? 8 : (has_alpha?32:24));
  hdr.flags = TGA_IL_None | TGA_Org_BL;

  size_t size = w * h * hdr.PixelSize/8 
    + 256 * hdr.CoSize / 8 + sizeof (TGAheader) + hdr.IDLength;

  csDataBuffer *db = new csDataBuffer (size);

  unsigned char *p = (unsigned char*)db->GetData ();

  memcpy (p, &hdr, sizeof(TGAheader));
  p += sizeof(TGAheader);

  memcpy (p, CSTGA_ID, hdr.IDLength);
  p += hdr.IDLength;

  int i, x, y;
  if (palette)
  {
    const csRGBpixel *pal = Image->GetPalette ();
    for (i= 0; i < 256; i++)
    {
      *(p++) = pal[i].blue;
      *(p++) = pal[i].green;
      *(p++) = pal[i].red;
    }

    unsigned char *data = (unsigned char *)Image->GetImageData ();
    for (y=h-1; y >= 0; y--)
      for (x=0; x < w; x++)
	*p++ = data[y*w+x];
  }
  else
  {
    csRGBpixel *pixel = (csRGBpixel *)Image->GetImageData ();
    for (y=h-1; y >= 0; y--)
    {
      csRGBpixel *c = &pixel[y*w];
      for (x=0; x < w; x++)
      {
	*p++ = c->blue;
	*p++ = c->green;
	*p++ = c->red;
	if (has_alpha) *p++ = c->alpha;
	c++;
      }
    }
  }

  return csPtr<iDataBuffer> (db);
}

csPtr<iDataBuffer> csTGAImageIO::Save (iImage *Image, const char *mime,
  const char* extraoptions)
{
  if (!strcasecmp (mime, TGA_MIME))
    return Save (Image, (iImageIO::FileFormatDescription *)0,
      extraoptions);
  return 0;
}

//---------------------------------------------------------------------------

csRef<iImageFileLoader> ImageTgaFile::InitLoader (csRef<iDataBuffer> source)
{
  csRef<TgaLoader> loader;
  loader.AttachNew (new TgaLoader (Format, source));
  if (!loader->InitOk()) return 0;
  return loader;
}

bool ImageTgaFile::TgaLoader::readtga (uint8*& iBuffer, TGAheader* tgaP)
{
  if (!GetBytes (iBuffer, (uint8*)tgaP, sizeof (TGAheader))) return false;

  if (tgaP->IDLength != 0)
    iBuffer += tgaP->IDLength;
  return (iBuffer <= bufferEnd);
}

bool ImageTgaFile::TgaLoader::get_map_entry (uint8*& iBuffer, 
					     csRGBpixel* Value, int Size, 
					     bool alpha)
{
  uint8 bytes[4];
  uint l;

  /* Read appropriate number of bytes, break into rgb & put in map. */
  switch (Size)
  {
    case 8:				/* Grey scale, read and triplicate. */
      if (!GetBytes (iBuffer, bytes, 1)) return false;
      Value->red = Value->green = Value->blue = bytes[0];
      break;

    case 16:				/* 5 bits each of red green and blue. */
    case 15:				/* Watch for byte order. */
      if (!GetBytes (iBuffer, bytes, 2)) return false;
      l = ((unsigned int) bytes[1] << 8) + bytes[0];
      Value->red = (bytes[1] & 0x7C) << 1;
      Value->red |= Value->red >> 5;
      Value->green = ((bytes[1] & 0x03) << 6) | ((bytes[0] & 0xE0) >> 2);
      Value->green |= Value->green >> 5;
      Value->blue = (bytes[0] & 0x1F) << 3;
      Value->blue |= Value->blue >> 5;
      break;

    case 32:
    case 24:				/* 8 bits each of blue green and red. */
      if (!GetBytes (iBuffer, bytes, (Size == 32) ? 4 : 3)) return false;
      Value->blue = bytes[0];
      Value->green = bytes[1];
      Value->red = bytes[2];
      if ((Size == 32) && alpha)
          Value->alpha = bytes[3];	/* Read alpha byte */
      break;

    default:
      return false;
  }
  return true;
}

bool ImageTgaFile::TgaLoader::get_current_pixel (uint8*& iBuffer, int Size, 
						 bool alpha)
{
  /* Check if run length encoded. */
  if (rlencoded)
  {
    if (RLE_count == 0)
    { /* Have to restart run. */
      uint8 i;
      if (!GetBytes (iBuffer, &i, 1)) return false;
      RLE_flag = (i & 0x80);
      if (RLE_flag == 0)
        /* Stream of unencoded pixels. */
        RLE_count = i + 1;
      else
        /* Single pixel replicated. */
        RLE_count = i - 127;
        /* Decrement count & get pixel. */
        --RLE_count;
    }
    else
    { /* Have already read count & (at least) first pixel. */
      --RLE_count;
      if (RLE_flag != 0)
      /* Replicated pixels. */
        return true;
    }
  }

  uint8 bytes[4];
  /* Read appropriate number of bytes, break into RGB. */
  switch (Size)
  {
    case 8:				/* Grey scale, read and triplicate. */
      if (!GetBytes (iBuffer, bytes, 1)) return false;
      currentPixel.Red = currentPixel.Grn = currentPixel.Blu = currentPixel.l = bytes[0];
      currentPixel.Alpha = 0xff;
      break;

    case 16:				/* 5 bits each of red green and blue. */
    case 15:				/* Watch byte order. */
      if (!GetBytes (iBuffer, bytes, 2)) return false;
      currentPixel.l = ((unsigned int) bytes[1] << 8) + bytes[0];
      currentPixel.Red = (bytes[1] & 0x7C) << 1;
      currentPixel.Red |= currentPixel.Red >> 5;
      currentPixel.Grn = ((bytes[1] & 0x03) << 6) | ((bytes[0] & 0xE0) >> 2);
      currentPixel.Grn |= currentPixel.Grn >> 5;
      currentPixel.Blu = (bytes[0] & 0x1F) << 3;
      currentPixel.Blu |= currentPixel.Blu >> 5;
      currentPixel.Alpha = 0xff;
      break;

    case 32:
    case 24:			/* 8 bits each of blue green and red. */
      if (!GetBytes (iBuffer, bytes, (Size == 32) ? 4 : 3)) return false;
      currentPixel.Blu = bytes[0];
      currentPixel.Grn = bytes[1];
      currentPixel.Red = bytes[2];
      currentPixel.Alpha = 0xff;
      if ((Size == 32) && alpha)
        currentPixel.Alpha = bytes[3];	/* Read alpha byte */
      currentPixel.l = 0;
      break;

    default:
      return false;
  }
  return true;
}

bool ImageTgaFile::TgaLoader::get_pixel (uint8*& iBuffer, csRGBpixel* dest, 
					 int Size, bool alpha)
{
  if (!get_current_pixel (iBuffer, Size, alpha)) return false;

  if (mapped)
    *dest = colorMap[currentPixel.l];
  else
  {
    dest->red = currentPixel.Red;
    dest->green = currentPixel.Grn; 
    dest->blue = currentPixel.Blu; 
    dest->alpha = currentPixel.Alpha;
  }
  return true;
}

bool ImageTgaFile::TgaLoader::get_pixel (uint8*& iBuffer, uint8* dest, 
					 int Size, bool alpha)
{
  if (!get_current_pixel (iBuffer, Size, alpha)) return false;

  if (tga_head.PixelSize <= 8)
    *dest = currentPixel.l << indexShift;
  else
    *dest = currentPixel.l >> indexShift;
  return true;
}

ImageTgaFile::TgaLoader::~TgaLoader()
{
  delete[] colorMap;
}

bool ImageTgaFile::TgaLoader::InitOk()
{
  int NewFormat;
  iBuffer = dataSource->GetUint8();
  bufferEnd = iBuffer + dataSource->GetSize();
  if (!readtga (iBuffer, &tga_head)) return false;

  switch (tga_head.ImgType)
  {
    case TGA_Map:
    case TGA_RGB:
    case TGA_Mono:
    case TGA_RLEMap:
    case TGA_RLERGB:
    case TGA_RLEMono:
      break;

    default:
      return false;
  }

  Height = tga_head.Height;
  Width = tga_head.Width;
  colorMapSize = tga_head.Length;

  if (tga_head.ImgType == TGA_Map ||
      tga_head.ImgType == TGA_RLEMap ||
      tga_head.ImgType == TGA_CompMap ||
      tga_head.ImgType == TGA_CompMap4)
  { /* Color-mapped image */
    if (tga_head.CoMapType != 1)
      return false;
    mapped = true;
  }
  else if ((tga_head.ImgType == TGA_Mono)
    || (tga_head.ImgType == TGA_RLEMono))
  {
    // Grayscale image
    colorMapSize = 1 << tga_head.PixelSize;
    mapped = true;
  }
  else
    mapped = false;
  if (mapped && (colorMapSize <= 256))
  {
    dataType = rdtIndexed;
    NewFormat = (Format & ~CS_IMGFMT_MASK) | CS_IMGFMT_PALETTED8;
  }
  else
  {
    dataType = rdtRGBpixel;
    NewFormat = (Format & ~CS_IMGFMT_MASK) | CS_IMGFMT_TRUECOLOR;
  }

  if ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_ANY)
    Format = NewFormat;
  else
    // Copy alpha flag
    Format = (Format & CS_IMGFMT_MASK) | (NewFormat & ~CS_IMGFMT_MASK);

  return true;
}

bool ImageTgaFile::TgaLoader::LoadData ()
{
  if (colorMapSize != 0)
  {
    if (tga_head.PixelSize <= 8)
      indexShift = 8 - tga_head.PixelSize;
    else
      indexShift = tga_head.PixelSize - 8;

    uint colorMapOffs = tga_head.Index;
    colorMap = new csRGBpixel [colorMapOffs + colorMapSize];
    if (tga_head.CoMapType != 0)
    {
      for (uint i = colorMapOffs; i < colorMapOffs + colorMapSize; ++i)
      {
	get_map_entry (iBuffer, colorMap + i, tga_head.CoSize,
	  Format & CS_IMGFMT_ALPHA);
      }
    }
    else
    {
      if (tga_head.PixelSize <= 8)
      {
	for (uint i = 0; i < colorMapSize; i++)
	  colorMap[i].Set (i << indexShift, i << indexShift, i << indexShift);
      }
      else
      {
	for (uint i = 0; i < colorMapSize; i++)
	  colorMap[i].Set (i >> indexShift, i >> indexShift, i >> indexShift);
      }
    }
  }

  /* Check run-length encoding. */
  rlencoded = (tga_head.ImgType == TGA_RLEMap ||
               tga_head.ImgType == TGA_RLERGB ||
               tga_head.ImgType == TGA_RLEMono);

  if (dataType == rdtIndexed)
  {
    indexData = new uint8[Width * Height];
    palette = colorMap;
    paletteCount = colorMapSize;
    colorMap = 0;
  }
  else
    rgbaData = new csRGBpixel [Width * Height];

  int truerow = 0;
  int baserow = 0;
  for (int row = 0; row < Height; ++row)
  {
    int realrow = truerow;
    if ((tga_head.flags & TGA_Org_MASK) == TGA_Org_BL)
      realrow = Height - realrow - 1;

    if (dataType == rdtIndexed)
    {
      for (int col = 0; col < Width; ++col)
      {
	if (!get_pixel (iBuffer, indexData + (realrow * Width + col),
	  tga_head.PixelSize, Format & CS_IMGFMT_ALPHA))
        {
          // Can't read pixel.... file is probably damaged.
          // Use obnoxious pattern for replacement.
          int v = ((realrow >> 1) & 1) ^ ((col >> 1) & 1);
          *(indexData + (realrow * Width + col)) = v * 255;
        }
      }
    }
    else
    {
      for (int col = 0; col < Width; ++col)
      {
	if (!get_pixel (iBuffer, rgbaData + (realrow * Width + col),
	  tga_head.PixelSize, Format & CS_IMGFMT_ALPHA))
        {
          // Can't read pixel.... file is probably damaged.
          // Use obnoxious pattern for replacement.
          int v = ((realrow >> 1) & 1) ^ ((col >> 1) & 1);
          (rgbaData + (realrow * Width + col))->Set (255*v, 255*(1-v), 255);
        }
      }
    }
    if ((tga_head.flags & TGA_IL_MASK) == TGA_IL_Four)
      truerow += 4;
    else if ((tga_head.flags & TGA_IL_MASK) == TGA_IL_Two)
      truerow += 2;
    else
      ++truerow;
    if (truerow >= Height)
      truerow = ++baserow;
  }

  dataSource = 0;
  return true;
}

