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

SCF_IMPLEMENT_FACTORY (csTGAImageIO);

typedef char ImageIDField[256];

/* Definitions for image types. */
#define TGA_Null 0
#define TGA_Map 1
#define TGA_RGB 2
#define TGA_Mono 3
#define TGA_RLEMap 9
#define TGA_RLERGB 10
#define TGA_RLEMono 11
#define TGA_CompMap 32
#define TGA_CompMap4 33

/* Definitions for interleave flag. */
#define TGA_IL_None   0x00
#define TGA_IL_Two    0x40
#define TGA_IL_Four   0x80
#define TGA_IL_MASK   0xc0

#define TGA_Org_MASK  0x20
#define TGA_Org_BL    0x00
#define TGA_Org_TL    0x20

#define MAXCOLORS 16384

#define CSTGA_ID "Made with Crystal Space, see http://www.crystalspace3d.org/"
#define TGA_MIME "image/tga"

//CS_IMPLEMENT_STATIC_VAR (GetColorMap, csRGBpixel, [MAXCOLORS])

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

  hdr.IDLength  = 0;//strlen (CSTGA_ID);
  hdr.CoMapType = (palette ? 1 : 0);
  hdr.ImgType = (palette ? TGA_Map : TGA_RGB);
  hdr.Index_lo = hdr.Index_hi = 0;
  hdr.Length_lo = 0;
  hdr.Length_hi = (palette ? 1 : 0);
  hdr.CoSize = (palette ? 24 : 0);
  hdr.X_org_lo = hdr.X_org_hi = 0;
  hdr.Y_org_lo = hdr.Y_org_hi = 0;
  hdr.Width_lo = (w%256);
  hdr.Width_hi = (w/256);
  hdr.Height_lo = h%256;
  hdr.Height_hi = h/256;
  hdr.PixelSize = (palette ? 8 : (has_alpha?32:24));
  hdr.flags = TGA_IL_None | TGA_Org_BL;


  size_t size = w*h*hdr.PixelSize/8 + MAXCOLORS * hdr.CoSize / 8 + sizeof (TGAheader) + hdr.IDLength;

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

#if 0
bool ImageTgaFile::Load (uint8* iBuffer, size_t iSize)
{
  (void)iSize;
  struct TGAheader tga_head;
  int i;
  unsigned int temp1, temp2;
  int rows, cols, row, col, realrow, truerow, baserow;

  /* If required, read the color map information. */
  if (tga_head.CoMapType != 0)
  {
    temp1 = int (tga_head.Index_lo) + int (tga_head.Index_hi) * 256;
    temp2 = int (tga_head.Length_lo) + int (tga_head.Length_hi) * 256;
    if ((temp1 + temp2 + 1) >= MAXCOLORS)
      return false;
    for (i = temp1; i < int (temp1 + temp2); ++i)
      get_map_entry (iBuffer, &(GetColorMap() [i]), (int) tga_head.CoSize,
        Format & CS_IMGFMT_ALPHA);
  }

  /* Check run-length encoding. */
  rlencoded = (tga_head.ImgType == TGA_RLEMap ||
               tga_head.ImgType == TGA_RLERGB ||
               tga_head.ImgType == TGA_RLEMono);

  /* Read the Targa file body and convert to portable format. */
  SetDimensions (cols, rows);

  // @@todo: avoid converting colormapped images into RGB,
  // instead pass a pointer to convert_pal8
  csRGBpixel *pixels = new csRGBpixel [Width * Height];

  truerow = 0;
  baserow = 0;
  for (row = 0; row < rows; ++row)
  {
    realrow = truerow;
    if ((tga_head.flags & TGA_Org_MASK) == TGA_Org_BL)
      realrow = rows - realrow - 1;

    for (col = 0; col < cols; ++col)
      get_pixel (iBuffer, &(pixels [realrow * cols + col]),
        (int) tga_head.PixelSize, Format & CS_IMGFMT_ALPHA);
    if ((tga_head.flags & TGA_IL_MASK) == TGA_IL_Four)
      truerow += 4;
    else if ((tga_head.flags & TGA_IL_MASK) == TGA_IL_Two)
      truerow += 2;
    else
      ++truerow;
    if (truerow >= rows)
      truerow = ++baserow;
  }

  // Convert image from RGB to requested format
  ConvertFromRGBA (pixels);

  // Check if the alpha channel is valid
  CheckAlpha ();

  return true;
}
#endif

void ImageTgaFile::TgaLoader::readtga (uint8*& iBuffer, TGAheader* tgaP)
{
  tgaP->IDLength = *iBuffer++;
  tgaP->CoMapType = *iBuffer++;
  tgaP->ImgType = *iBuffer++;
  tgaP->Index_lo = *iBuffer++;
  tgaP->Index_hi = *iBuffer++;
  tgaP->Length_lo = *iBuffer++;
  tgaP->Length_hi = *iBuffer++;
  tgaP->CoSize = *iBuffer++;
  tgaP->X_org_lo = *iBuffer++;
  tgaP->X_org_hi = *iBuffer++;
  tgaP->Y_org_lo = *iBuffer++;
  tgaP->Y_org_hi = *iBuffer++;
  tgaP->Width_lo = *iBuffer++;
  tgaP->Width_hi = *iBuffer++;
  tgaP->Height_lo = *iBuffer++;
  tgaP->Height_hi = *iBuffer++;
  tgaP->PixelSize = *iBuffer++;
  tgaP->flags = *iBuffer++;

  if (tgaP->IDLength != 0)
    iBuffer += tgaP->IDLength;
}

void ImageTgaFile::TgaLoader::get_map_entry (uint8*& iBuffer, 
					     csRGBpixel* Value, int Size, 
					     bool alpha)
{
  unsigned char j, k;
  uint l;

  /* Read appropriate number of bytes, break into rgb & put in map. */
  switch (Size)
  {
    case 8:				/* Grey scale, read and triplicate. */
      Value->red = Value->green = Value->blue = *iBuffer++;
      break;

    case 16:				/* 5 bits each of red green and blue. */
    case 15:				/* Watch for byte order. */
      j = *iBuffer++;
      k = *iBuffer++;
      l = ((unsigned int) k << 8)  + j;
      Value->red = (k & 0x7C) << 1;
      Value->red |= Value->red >> 5;
      Value->green = ((k & 0x03) << 6) | ((j & 0xE0) >> 2);
      Value->green |= Value->green >> 5;
      Value->blue = (j & 0x1F) << 3;
      Value->blue |= Value->blue >> 5;
      break;

    case 32:
    case 24:				/* 8 bits each of blue green and red. */
      Value->blue = *iBuffer++;
      Value->green = *iBuffer++;
      Value->red = *iBuffer++;
      if (Size == 32)
        if (alpha)
          Value->alpha = *iBuffer++;	/* Read alpha byte */
        else
          iBuffer++;
      break;

    default:
      return;
  }
}

void ImageTgaFile::TgaLoader::get_pixel (uint8*& iBuffer, csRGBpixel* dest, 
					 int Size, bool alpha)
{
  //int Red, Grn, Blu, Alpha;
  unsigned int l = 0;
  unsigned char j, k;

  /* Check if run length encoded. */
  if (rlencoded)
  {
    if (RLE_count == 0)
    { /* Have to restart run. */
      unsigned char i;
      i = *iBuffer++;
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
        goto PixEncode;
    }
  }

  /* Read appropriate number of bytes, break into RGB. */
  switch (Size)
  {
    case 8:				/* Grey scale, read and triplicate. */
      Red = Grn = Blu = l = *iBuffer++;
      Alpha = 0xff;
      break;

    case 16:				/* 5 bits each of red green and blue. */
    case 15:				/* Watch byte order. */
      j = *iBuffer++;
      k = *iBuffer++;
      l = ((unsigned int) k << 8)  + j;
      Red = (k & 0x7C) << 1;
      Red |= Red >> 5;
      Grn = ((k & 0x03) << 6) | ((j & 0xE0) >> 2);
      Grn |= Grn >> 5;
      Blu = (j & 0x1F) << 3;
      Blu |= Blu >> 5;
      Alpha = 0xff;
      break;

    case 32:
    case 24:			/* 8 bits each of blue green and red. */
      Blu = *iBuffer++;
      Grn = *iBuffer++;
      Red = *iBuffer++;
      Alpha = 0xff;
      if (Size == 32)
        if (alpha)
          Alpha = *iBuffer++;	/* Read alpha byte */
        else
          iBuffer++;
      l = 0;
      break;

    default:
      return;
  }

PixEncode:
  if (mapped)
    *dest = colorMap[l];
  else
  {
    dest->red = Red; dest->green = Grn; dest->blue = Blu; dest->alpha = Alpha;
  }
}

ImageTgaFile::TgaLoader::~TgaLoader()
{
  delete[] colorMap;
}

bool ImageTgaFile::TgaLoader::InitOk()
{
  iBuffer = dataSource->GetUint8();
  readtga (iBuffer, &tga_head);

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

  Height = (int (tga_head.Height_lo)) + (int (tga_head.Height_hi)) * 256;
  Width = (int (tga_head.Width_lo))  + (int (tga_head.Width_hi))  * 256;

  if (tga_head.ImgType == TGA_Map ||
      tga_head.ImgType == TGA_RLEMap ||
      tga_head.ImgType == TGA_CompMap ||
      tga_head.ImgType == TGA_CompMap4)
  { /* Color-mapped image */
    if (tga_head.CoMapType != 1)
      return false;
    mapped = true;
  }
  else
  { /* Not colormap */
    mapped = false;
  }

  /* If required, read the color map information. */
  if (tga_head.CoMapType != 0)
  {
    uint temp1 = int (tga_head.Index_lo) + int (tga_head.Index_hi) * 256;
    uint temp2 = int (tga_head.Length_lo) + int (tga_head.Length_hi) * 256;
    if ((temp1 + temp2 + 1) >= MAXCOLORS)
      return false;
  }

  dataType = rdtRGBpixel;

  return true;
}

bool ImageTgaFile::TgaLoader::LoadData ()
{
  if (tga_head.CoMapType != 0)
  {
    uint temp1 = int (tga_head.Index_lo) + int (tga_head.Index_hi) * 256;
    uint temp2 = int (tga_head.Length_lo) + int (tga_head.Length_hi) * 256;
    colorMap = new csRGBpixel [temp1 + temp2];
    for (int i = temp1; i < int (temp1 + temp2); ++i)
      get_map_entry (iBuffer, colorMap + i, tga_head.CoSize,
        Format & CS_IMGFMT_ALPHA);
  }

  /* Check run-length encoding. */
  rlencoded = (tga_head.ImgType == TGA_RLEMap ||
               tga_head.ImgType == TGA_RLERGB ||
               tga_head.ImgType == TGA_RLEMono);

  // @@todo: avoid converting colormapped images into RGB,
  // instead pass a pointer to convert_pal8
  rgbaData = new csRGBpixel [Width * Height];

  int truerow = 0;
  int baserow = 0;
  for (int row = 0; row < Height; ++row)
  {
    int realrow = truerow;
    if ((tga_head.flags & TGA_Org_MASK) == TGA_Org_BL)
      realrow = Height - realrow - 1;

    for (int col = 0; col < Width; ++col)
      get_pixel (iBuffer, rgbaData + (realrow * Width + col),
        (int) tga_head.PixelSize, Format & CS_IMGFMT_ALPHA);
    if ((tga_head.flags & TGA_IL_MASK) == TGA_IL_Four)
      truerow += 4;
    else if ((tga_head.flags & TGA_IL_MASK) == TGA_IL_Two)
      truerow += 2;
    else
      ++truerow;
    if (truerow >= Height)
      truerow = ++baserow;
  }

  return true;
}

