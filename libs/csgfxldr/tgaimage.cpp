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

#include "sysdef.h"
#include "csgfxldr/tgaimage.h"

//---------------------------------------------------------------------------

bool RegisterTGA ()
{
  static TGAImageLoader loader;
  return ImageLoader::Register (&loader);
}

AlphaMapFile* TGAImageLoader::LoadAlphaMap(UByte *buf,ULong size)
{
  (void) buf;
  (void) size;
  return NULL;
}

ImageFile* TGAImageLoader::LoadImage (UByte* buf, ULong size)
{
  CHK (ImageTgaFile* i = new ImageTgaFile(buf, size));
  if (i && (i->get_status() & IFE_BadFormat))
  { CHK ( delete i );  i = NULL; }
  return i;    
}

//---------------------------------------------------------------------------

#define MAXCOLORS 16384

static int mapped, rlencoded;

static RGBPixel ColorMap[MAXCOLORS];
static int RLE_count = 0, RLE_flag = 0;

static void readtga ( UByte*& ptr, struct ImageHeader* tgaP );
static void get_map_entry ( UByte*& ptr, RGBPixel* Value, int Size );
static void get_pixel ( UByte*& ptr, RGBPixel* dest, int Size );
static unsigned char getbyte ( UByte*& ptr );

ImageTgaFile::~ImageTgaFile ()
{
}

ImageTgaFile::ImageTgaFile (UByte* ptr, long filesize) : ImageFile ()
{
  (void)filesize;
  struct ImageHeader tga_head;
  int i;
  unsigned int temp1, temp2;
  int rows, cols, row, col, realrow, truerow, baserow;
  int maxval;
  RGBPixel *pixels;

  /* @@@ to do: Add TGA format detection */

  /* Read the Targa file header. */
  readtga (ptr, &tga_head);

  rows = ( (int) tga_head.Height_lo ) + ( (int) tga_head.Height_hi ) * 256;
  cols = ( (int) tga_head.Width_lo ) + ( (int) tga_head.Width_hi ) * 256;

  switch ( tga_head.ImgType )
  {
    case TGA_Map:
    case TGA_RGB:
    case TGA_Mono:
    case TGA_RLEMap:
    case TGA_RLERGB:
    case TGA_RLEMono:
    break;

    default:
      status = IFE_BadFormat;
      //set_error (IFE_BadFormat, "Unknown Targa image type");
      return;
  }

  if ( tga_head.ImgType == TGA_Map ||
       tga_head.ImgType == TGA_RLEMap ||
       tga_head.ImgType == TGA_CompMap ||
       tga_head.ImgType == TGA_CompMap4 )
  { /* Color-mapped image */
    if ( tga_head.CoMapType != 1 )
    {
      status = IFE_Corrupt;
      //set_error (IFE_BadFormat, "Mapped image with bad color map type");
      return;
    }
    mapped = 1;
    /* Figure maxval from CoSize. */
    switch ( tga_head.CoSize )
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
        status = IFE_Corrupt;
        //set_error (IFE_BadFormat, "Unknown colormap pixel size");
	return;
    }
  }
  else
  { /* Not colormap, so figure maxval from PixelSize. */
    mapped = 0;
    switch ( tga_head.PixelSize )
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
        status = IFE_Corrupt;
        //set_error (IFE_BadFormat, "Unknown pixel size");
	return;
    }
  }

  /* If required, read the color map information. */
  if ( tga_head.CoMapType != 0 )
  {
	temp1 = tga_head.Index_lo + tga_head.Index_hi * 256;
	temp2 = tga_head.Length_lo + tga_head.Length_hi * 256;
	if ( ( temp1 + temp2 + 1 ) >= MAXCOLORS )
	{
          status = IFE_Corrupt;
	  //set_error (IFE_BadFormat, "Too many colors in colormap");
	  return;
	}
	for ( i = temp1; i < (int)( temp1 + temp2 ); ++i )
	    get_map_entry( ptr, &ColorMap[i], (int) tga_head.CoSize );
	}

    /* Check run-length encoding. */
    if ( tga_head.ImgType == TGA_RLEMap ||
	 tga_head.ImgType == TGA_RLERGB ||
	 tga_head.ImgType == TGA_RLEMono )
	rlencoded = 1;
    else
	rlencoded = 0;

    /* Read the Targa file body and convert to portable format. */
    set_dimensions (cols, rows);
    pixels = get_buffer ();

    truerow = 0;
    baserow = 0;
    for ( row = 0; row < rows; ++row )
    {
	realrow = truerow;
	if ( tga_head.OrgBit == 0 )
	    realrow = rows - realrow - 1;

	for ( col = 0; col < cols; ++col )
	    get_pixel( ptr, &(pixels[realrow*cols+col]), (int) tga_head.PixelSize );
	if ( tga_head.IntrLve == TGA_IL_Four )
	    truerow += 4;
	else if ( tga_head.IntrLve == TGA_IL_Two )
	    truerow += 2;
	else
	    ++truerow;
	if ( truerow >= rows )
	    truerow = ++baserow;
    }
}


static void readtga (UByte*& ptr, ImageHeader* tgaP)
{
    unsigned char flags;

    tgaP->IDLength = getbyte( ptr );
    tgaP->CoMapType = getbyte( ptr );
    tgaP->ImgType = getbyte( ptr );
    tgaP->Index_lo = getbyte( ptr );
    tgaP->Index_hi = getbyte( ptr );
    tgaP->Length_lo = getbyte( ptr );
    tgaP->Length_hi = getbyte( ptr );
    tgaP->CoSize = getbyte( ptr );
    tgaP->X_org_lo = getbyte( ptr );
    tgaP->X_org_hi = getbyte( ptr );
    tgaP->Y_org_lo = getbyte( ptr );
    tgaP->Y_org_hi = getbyte( ptr );
    tgaP->Width_lo = getbyte( ptr );
    tgaP->Width_hi = getbyte( ptr );
    tgaP->Height_lo = getbyte( ptr );
    tgaP->Height_hi = getbyte( ptr );
    tgaP->PixelSize = getbyte( ptr );
    flags = getbyte( ptr );
    tgaP->AttBits = flags & 0xf;
    tgaP->Rsrvd = ( flags & 0x10 ) >> 4;
    tgaP->OrgBit = ( flags & 0x20 ) >> 5;
    tgaP->IntrLve = ( flags & 0xc0 ) >> 6;

    if ( tgaP->IDLength != 0 )
      ptr += tgaP->IDLength;
}

static void get_map_entry (UByte*& ptr, RGBPixel* Value, int Size)
{
    unsigned char j, k, r, g, b;
    r=g=b=0; /* get rid of stupid 'might be used uninited' warning */

    /* Read appropriate number of bytes, break into rgb & put in map. */
    switch ( Size )
	{
	case 8:				/* Grey scale, read and triplicate. */
	r = g = b = getbyte( ptr );
	break;

	case 16:			/* 5 bits each of red green and blue. */
	case 15:			/* Watch for byte order. */
	j = getbyte( ptr );
	k = getbyte( ptr );
	r = ( k & 0x7C ) >> 2;
	g = ( ( k & 0x03 ) << 3 ) + ( ( j & 0xE0 ) >> 5 );
	b = j & 0x1F;
	break;

	case 32:
	case 24:			/* 8 bits each of blue green and red. */
	b = getbyte( ptr );
	g = getbyte( ptr );
	r = getbyte( ptr );
	if ( Size == 32 )
	    (void) getbyte( ptr );	/* Read alpha byte & throw away. */
	break;

	default:
	  //set_error (IFE_BadFormat, "Unknown colormap pixel size");
	  return;
	}
    Value->red=r; Value->green=g; Value->blue=b;
}

static void get_pixel (UByte*& ptr, RGBPixel* dest, int Size)
{
    static int Red, Grn, Blu;
    unsigned char j, k;
    static unsigned int l;

    /* Check if run length encoded. */
    if ( rlencoded )
	{
	if ( RLE_count == 0 )
	    { /* Have to restart run. */
	    unsigned char i;
	    i = getbyte( ptr );
	    RLE_flag = ( i & 0x80 );
	    if ( RLE_flag == 0 )
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
	    if ( RLE_flag != 0 )
		/* Replicated pixels. */
		goto PixEncode;
	    }
	}
    /* Read appropriate number of bytes, break into RGB. */
    switch ( Size )
	{
	case 8:				/* Grey scale, read and triplicate. */
	Red = Grn = Blu = l = getbyte( ptr );
	break;

	case 16:			/* 5 bits each of red green and blue. */
	case 15:			/* Watch byte order. */
	j = getbyte( ptr );
	k = getbyte( ptr );
	l = ( (unsigned int) k << 8 ) + j;
	Red = ( k & 0x7C ) >> 2;
	Grn = ( ( k & 0x03 ) << 3 ) + ( ( j & 0xE0 ) >> 5 );
	Blu = j & 0x1F;
	break;

	case 32:
	case 24:			/* 8 bits each of blue green and red. */
	Blu = getbyte( ptr );
	Grn = getbyte( ptr );
	Red = getbyte( ptr );
	if ( Size == 32 )
	    (void) getbyte( ptr );	/* Read alpha byte & throw away. */
	l = 0;
	break;

	default:
	  //set_error (IFE_BadFormat, "Unknown pixel size");
	  return;
	}

PixEncode:
    if ( mapped )
	*dest = ColorMap[l];
    else
    {
	dest->red=Red;dest->green=Grn;dest->blue=Blu;
    }
}

static unsigned char getbyte (UByte*& ptr)
{
  unsigned char c = *ptr++;
  return c;
}
