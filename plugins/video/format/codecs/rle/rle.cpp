/*
    Copyright (C) 2001 by Norman Krämer
  
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
#include "rle.h"

CS_IMPLEMENT_PLUGIN

#define Mono          1  // Monochrome bitmap
#define _16Color      4  // 16 color bitmap
#define _256Color     8  // 256 color bitmap
#define HIGHCOLOR    16  // 16bit (high color) bitmap
#define TRUECOLOR24  24  // 24bit (true color) bitmap
#define TRUECOLOR32  32  // 32bit (true color) bitmap

// Compression Types
#ifndef BI_RGB
#define BI_RGB        0  // none
#define BI_RLE8       1  // RLE 8-bit / pixel
#define BI_RLE4       2  // RLE 4-bit / pixel
#define BI_BITFIELDS  3  // Bitfields
#endif

#define BICOMP(x)    *(ULong*)((x)+16)
#define BITCOUNT(x)  *(UShort*)((x) + 14)
#define BICLRUSED(x) *(ULong*)((x) + 32)
#define BIPALETTE(x) (UByte*)((x)+40)

SCF_IMPLEMENT_IBASE (csRLECodec)
  SCF_IMPLEMENTS_INTERFACE (iAVICodec)
  SCF_IMPLEMENTS_INTERFACE (iBase)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csRLECodec)
SCF_EXPORT_CLASS_TABLE (rlecodec)
  SCF_EXPORT_CLASS (csRLECodec, "crystalspace.video.codec.avi.rle", "CrystalSpace RLE codec")
  SCF_EXPORT_CLASS (csRLECodec, "crystalspace.video.codec.avi.RLE", "CrystalSpace RLE codec")
SCF_EXPORT_CLASS_TABLE_END

csRLECodec::csRLECodec (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  pixel = NULL;
}

csRLECodec::~csRLECodec ()
{
  delete [] pixel;
}

static void decode_idx (UByte *dst, UByte *src, ULong, csRGBcolor *pMap, 
			int w, int h)
{
  int i,j;
  UByte *ps;
  src = src + (h-1)*w;

  for (i=0; i<h; i++)
  {
    ps = src;
    for (j=0; j<w; j++)
    {
      memcpy (dst, &pMap[*ps++], 3);
      dst+=4;
    }
    src -= w;
  }
}

static void decode_rgb24 (UByte *dst, UByte *src, ULong, csRGBcolor *, 
			  int w, int h)
{
  int i,j;
  UByte *ps;
  int stride = w*3;

  src = src + (h-1)*stride;
  for (i=0; i<h; i++)
  {
    ps = src;
    for (j=0; i<w; j++)
    {
      // BGR order
      *(dst+2) = *ps++;
      *(dst+1) = *ps++;
      *(dst)   = *ps++;
      dst+=4;
    }
    src -= stride;
  }
}

static void decode_rle8 (UByte *dst, UByte *src, ULong inlen, csRGBcolor *pMap, 
			 int w, int h)
{
  // Decompress pixel data
  UByte rl, rl1, i;			// runlength
  UByte clridx, clridx1;		// colorindex
  int buffer_x = 0;
  UByte *ps, *end, *buffer_y;
  ps = src;
  end = ps + inlen;
  int dststride = w*4;
  bool blip = false;

  buffer_y = dst + dststride*(h-1);

  while (ps < end && buffer_y >= dst)
  {
    rl = rl1 = *ps++;
    clridx = clridx1 = *ps++;
    if (rl == 0)
      if (clridx == 0)
      {
	// new scanline
	if (!blip)
	{
	  // if we didnt already jumped to the new line, do it now
	  buffer_x  = 0;
	  buffer_y -= dststride;
	}
	continue;
      }
      else if (clridx == 1)
	// end of bitmap
	break;
      else if (clridx == 2)
      {
	// next 2 bytes mean column- and scanline- offset
	buffer_x += (4* (*ps++));
	buffer_y -= (dststride * (*ps++));
	continue;
      }
      else if (clridx > 2)
	rl1 = clridx;

    for ( i = 0; i < rl1; i++ )
    {
      if (!rl) clridx1 = *ps++;
      memcpy (buffer_y + buffer_x, &pMap[clridx1], 3);
      buffer_x += 4;
      if (buffer_x >= dststride)
      {
	buffer_x  = 0;
	buffer_y -= dststride;
	blip = true;
      }
      else
	blip = false;
    }
    // pad in case rl == 0 and clridx in [3..255]
    if (rl == 0 && (clridx & 0x01)) ps++;
  }
}

bool csRLECodec::Initialize (csStreamDescription *desc, UByte *, ULong, 
			     UByte *pFormatEx, ULong nFormatEx)
{
  csVideoStreamDescription *vd = (csVideoStreamDescription *)desc;
  w = vd->width;
  h = vd->height;
  bOK = false;

  pixel = new csRGBpixel [w*h];

  if (vd->colordepth <= 8)
  {
    if (nFormatEx != 0)
    {
      int i, nMaxColor = 256; //MIN (256, (nFormatEx-40) / 4);
      UByte *pPal = BIPALETTE(pFormatEx);
      memset (cmap, 0, 256 * sizeof(csRGBcolor));
      // read colormap
      for (i=0; i < nMaxColor; i++)
      {
	cmap[i].blue = *pPal++;
	cmap[i].green = *pPal++;
	cmap[i].red = *pPal++;
	pPal++;
      }
    }
    else
      return false;
  }

  // setup the decoding functions
  if (BICOMP(pFormatEx) == BI_RGB)
    decode = decode_idx;
  else
  if (BICOMP(pFormatEx) == BI_RLE8)
    decode = decode_rle8;
  else
    //  if (BICOMP(pFormatEx) == BI_RLE4)
    //    decode = decode_rle4;
    //  else
  if (!BICLRUSED(pFormatEx) && BITCOUNT(pFormatEx) == TRUECOLOR24)
    decode = decode_rgb24;
  else
    return false;

  bOK = true;
  return true;
}

void csRLECodec::GetCodecDescription (csCodecDescription &desc)
{
  desc.bEncode = false; // not implemented yet
  desc.bDecode = true;
  desc.decodeoutput = CS_CODECFORMAT_RGBA_INTERLEAVED;
  desc.encodeinput = CS_CODECFORMAT_RGBA_INTERLEAVED;
}

bool csRLECodec::Decode (char *indata, ULong inlength, void *&outdata)
{
  if (bOK)
  {
    decode ((UByte*)pixel, (UByte*)indata, inlength, &cmap[0], w, h);
    outdata = (void*)pixel;
    return true;
  }
  return false;
}

bool csRLECodec::Encode (void *, char *, ULong &)
{
  return false;
}
