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

#include <math.h>
#include <stdio.h>

#include "cssysdef.h"
#include "tgaimage.h"
#include "csgfx/rgbpixel.h"
#include "csutil/databuf.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csTGAImageIO)
  SCF_IMPLEMENTS_INTERFACE (iImageIO)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTGAImageIO::eiPlugIn)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csTGAImageIO);

SCF_EXPORT_CLASS_TABLE (cstgaimg)
  SCF_EXPORT_CLASS (csTGAImageIO, "crystalspace.graphic.image.io.tga", "CrystalSpace TGA image format I/O plugin")
SCF_EXPORT_CLASS_TABLE_END

/* Header definition. */
struct TGAheader
{
  unsigned char IDLength;		/* length of Identifier String */
  unsigned char CoMapType;		/* 0 = no map */
  unsigned char ImgType;		/* image type (see below for values) */
  unsigned char Index_lo, Index_hi;	/* index of first color map entry */
  unsigned char Length_lo, Length_hi;	/* number of entries in color map */
  unsigned char CoSize;			/* size of color map entry (15,16,24,32) */
  unsigned char X_org_lo, X_org_hi;	/* x origin of image */
  unsigned char Y_org_lo, Y_org_hi;	/* y origin of image */
  unsigned char Width_lo, Width_hi;	/* width of image */
  unsigned char Height_lo, Height_hi;	/* height of image */
  unsigned char PixelSize;		/* pixel size (8,16,24,32) */
  unsigned char AttBits;		/* 4 bits, number of attribute bits per pixel */
  unsigned char Rsrvd;			/* 1 bit, reserved */
  unsigned char OrgBit;			/* 1 bit, origin: 0=lower left, 1=upper left */
  unsigned char IntrLve;		/* 2 bits, interleaving flag */
};

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
#define TGA_IL_None 0
#define TGA_IL_Two 1
#define TGA_IL_Four 2

#define MAXCOLORS 16384

#define CSTGA_ID "Made with CrystalSpace, see http://crystal.linuxgames.com"
#define TGA_MIME "image/tga"

static int mapped, rlencoded;

static csRGBpixel ColorMap[MAXCOLORS];
static int RLE_count = 0, RLE_flag = 0;

static void readtga (UByte*& ptr, struct TGAheader* tgaP);
static void get_map_entry (UByte*& ptr, csRGBpixel* Value, int Size, bool alpha);
static void get_pixel (UByte*& ptr, csRGBpixel* dest, int Size, bool alpha);


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
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
  formats.Push (&formatlist[0]);
  formats.Push (&formatlist[1]);
  formats.Push (&formatlist[2]);
  formats.Push (&formatlist[3]);
  formats.Push (&formatlist[4]);
  formats.Push (&formatlist[5]);
}

const csVector& csTGAImageIO::GetDescription ()
{
  return formats;
}

iImage *csTGAImageIO::Load (UByte* iBuffer, ULong iSize, int iFormat)
{
  ImageTgaFile* i = new ImageTgaFile (iFormat);
  if (i && !i->Load (iBuffer, iSize))
  {
     delete i ;
    return NULL;
  }
  return i;    
}

void csTGAImageIO::SetDithering (bool)
{
}

iDataBuffer *csTGAImageIO::Save (iImage *Image, iImageIO::FileFormatDescription *)
{
  if (!Image || !Image->GetImageData ())
    return NULL;

  int format = Image->GetFormat ();
  bool palette = false;

  switch (format & CS_IMGFMT_MASK)
  {
  case CS_IMGFMT_PALETTED8:
    palette = true;
  case CS_IMGFMT_TRUECOLOR:
    break;
  default:
    return NULL;
  };

  if (palette && !Image->GetPalette ())
    return NULL;

  // fill header
  TGAheader hdr;
  int w = Image->GetWidth ();
  int h = Image->GetHeight ();

  hdr.IDLength  = strlen (CSTGA_ID);
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
  hdr.PixelSize = (palette ? 8 : 24);
  hdr.AttBits = 0;
  hdr.Rsrvd = 0;
  hdr.OrgBit = 0;
  hdr.IntrLve = TGA_IL_None;


  size_t size = w*h*hdr.PixelSize/8 + MAXCOLORS * hdr.CoSize / 8 + sizeof (TGAheader) + hdr.IDLength;

  csDataBuffer *db = new csDataBuffer (size);

  unsigned char *p = (unsigned char*)db->GetData ();

  memcpy (p, &hdr, sizeof(TGAheader));
  p += sizeof(TGAheader);

  memcpy (p, CSTGA_ID, hdr.IDLength);
  p += hdr.IDLength;

  if (palette)
  {
    csRGBpixel *pal = Image->GetPalette ();
    for (int i= 0; i < 256; i++)
    {
      csRGBcolor color = pal[i];
      memcpy (p, &color, sizeof (csRGBcolor));
      p += sizeof (csRGBcolor);
    }

    unsigned char *data = (unsigned char *)Image->GetImageData ();
    for (int y=h-1; y >= 0; y--)
      for (int x=0; x < w; x++)
	*p++ = data[y*w+x];
  }
  else
  {
    csRGBpixel *pixel = (csRGBpixel *)Image->GetImageData ();
    for (int y=h-1; y >= 0; y--)
      for (int x=0; x < w; x++)
      {
	unsigned char *c = (unsigned char *)&pixel[y*w+x];
	*p++ = *(c+2);
	*p++ = *(c+1);
	*p++ = *c;
      }
  }

  return db;
}

iDataBuffer *csTGAImageIO::Save (iImage *Image, const char *mime)
{
  if (!strcasecmp (mime, TGA_MIME))
    return Save (Image, (iImageIO::FileFormatDescription *)NULL);
  return NULL;
}

//---------------------------------------------------------------------------

bool ImageTgaFile::Load (UByte* iBuffer, ULong iSize)
{
  (void)iSize;
  struct TGAheader tga_head;
  int i;
  unsigned int temp1, temp2;
  int rows, cols, row, col, realrow, truerow, baserow;
  int maxval;

  /* @@todo: Add TGA format detection */

  /* Read the Targa file header. */
  readtga (iBuffer, &tga_head);

  rows = (int (tga_head.Height_lo)) + (int (tga_head.Height_hi)) * 256;
  cols = (int (tga_head.Width_lo))  + (int (tga_head.Width_hi))  * 256;

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

  if (tga_head.ImgType == TGA_Map ||
      tga_head.ImgType == TGA_RLEMap ||
      tga_head.ImgType == TGA_CompMap ||
      tga_head.ImgType == TGA_CompMap4) 
  { /* Color-mapped image */
    if (tga_head.CoMapType != 1)
      return false;
    mapped = 1;
    /* Figure maxval from CoSize. */
    switch (tga_head.CoSize) 
    {
      case 8:
      case 24:
      case 32:
        maxval = 255;
        break;

      case 15:
      case 16:
        maxval = 31;
        break;

      default:
	return false;
    }
  }
  else
  { /* Not colormap, so figure maxval from PixelSize. */
    mapped = 0;
    switch (tga_head.PixelSize) 
    {
      case 8:
      case 24:
      case 32:
        maxval = 255;
        break;

      case 15:
      case 16:
        maxval = 31;
        break;

      default:
	return false;
    }
  }

  /* If required, read the color map information. */
  if (tga_head.CoMapType != 0) 
  {
    temp1 = int (tga_head.Index_lo) + int (tga_head.Index_hi) * 256;
    temp2 = int (tga_head.Length_lo) + int (tga_head.Length_hi) * 256;
    if ((temp1 + temp2 + 1) >= MAXCOLORS)
      return false;
    for (i = temp1; i < int (temp1 + temp2); ++i) 
      get_map_entry (iBuffer, &ColorMap [i], (int) tga_head.CoSize,
        Format & CS_IMGFMT_ALPHA);
  }

  /* Check run-length encoding. */
  rlencoded = (tga_head.ImgType == TGA_RLEMap ||
               tga_head.ImgType == TGA_RLERGB ||
               tga_head.ImgType == TGA_RLEMono);

  /* Read the Targa file body and convert to portable format. */
  set_dimensions (cols, rows);

  // @@todo: avoid converting colormapped images into RGB,
  // instead pass a pointer to convert_pal8
  csRGBpixel *pixels = new csRGBpixel [Width * Height];

  truerow = 0;
  baserow = 0;
  for (row = 0; row < rows; ++row) 
  {
    realrow = truerow;
    if (tga_head.OrgBit == 0) 
      realrow = rows - realrow - 1;

    for (col = 0; col < cols; ++col) 
      get_pixel (iBuffer, &(pixels [realrow * cols + col]),
        (int) tga_head.PixelSize, Format & CS_IMGFMT_ALPHA);
    if (tga_head.IntrLve == TGA_IL_Four) 
      truerow += 4;
    else if (tga_head.IntrLve == TGA_IL_Two) 
      truerow += 2;
    else
      ++truerow;
    if (truerow >= rows) 
      truerow = ++baserow;
  }

  // Convert image from RGB to requested format
  convert_rgba (pixels);

  // Check if the alpha channel is valid
  CheckAlpha ();

  return true;
}


static void readtga (UByte*& iBuffer, TGAheader* tgaP)
{
  unsigned char flags;

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
  flags = *iBuffer++;
  tgaP->AttBits = flags & 0xf;
  tgaP->Rsrvd = (flags & 0x10)  >> 4;
  tgaP->OrgBit = (flags & 0x20)  >> 5;
  tgaP->IntrLve = (flags & 0xc0)  >> 6;

  if (tgaP->IDLength != 0) 
    iBuffer += tgaP->IDLength;
}

static void get_map_entry (UByte*& iBuffer, csRGBpixel* Value, int Size, bool alpha)
{
  unsigned char j, k;

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
      Value->red = (k & 0x7C) >> 2;
      Value->green = ((k & 0x03) << 3) + ((j & 0xE0) >> 5);
      Value->blue = j & 0x1F;
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

static void get_pixel (UByte*& iBuffer, csRGBpixel* dest, int Size, bool alpha)
{
  static int Red, Grn, Blu, Alpha;
  static unsigned int l;
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
      Alpha = 0;
      break;

    case 16:				/* 5 bits each of red green and blue. */
    case 15:				/* Watch byte order. */
      j = *iBuffer++;
      k = *iBuffer++;
      l = ((unsigned int) k << 8)  + j;
      Red = (k & 0x7C)  >> 2;
      Grn = ( (k & 0x03)  << 3)  + ( (j & 0xE0)  >> 5);
      Blu = j & 0x1F;
      Alpha = 0;
      break;

    case 32:
    case 24:			/* 8 bits each of blue green and red. */
      Blu = *iBuffer++;
      Grn = *iBuffer++;
      Red = *iBuffer++;
      Alpha = 0;
      if (Size == 32)
        if (alpha)
          Alpha = *iBuffer++;	/* Read alpha byte & throw away. */
        else
          iBuffer++;
      l = 0;
      break;

    default:
      return;
  }

PixEncode:
  if (mapped) 
    *dest = ColorMap [l];
  else
  {
    dest->red = Red; dest->green = Grn; dest->blue = Blu;
  }
}
