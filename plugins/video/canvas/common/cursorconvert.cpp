/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

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

#include "igraphic/imageio.h"
#include "csgfx/memimage.h"
#include "cstool/debugimagewriter.h"

#include "cursorconvert.h"

/*bool csCursorConverter::ConvertTo1bpp (iImage* image, uint8*& bitmap, 
				       uint8*& mask,
				       const csRGBcolor* keycolor)
{
  csRef<iImage> myImage = image->Clone ();
  myImage->SetFormat (CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA);

}*/

//#define DEBUG_WRITE_IMAGES 1

bool csCursorConverter::ConvertTo1bpp (iImage* image, uint8*& bitmap, 
				       uint8*& mask,
				       const csRGBcolor forecolor, 
				       const csRGBcolor backcolor, 
				       const csRGBcolor* keycolor,
				       bool XbitOrder)
{
  csRef<iImage> myImage = image->Clone ();
  myImage->SetFormat (CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA);
  myImage->SetName (image->GetName ());

  csColorQuantizer quantizer;
  quantizer.Begin ();

  csRGBpixel pal[2];
  pal[0] = forecolor;
  pal[1] = backcolor;
  csRGBpixel transp;
  if (keycolor)
    transp = *keycolor;
  else
  {
    transp.Set (255, 0, 255);
    StripAlpha (myImage, transp);
#ifdef DEBUG_WRITE_IMAGES
    csDebugImageWriter::DebugImageWrite (myImage, "cursor-%s-stripped.png",
      image->GetName ());
#endif
  }
  quantizer.Count (pal, 2, &transp);

  bool res = InternalConvertTo1bpp (myImage, quantizer, bitmap, mask, 
    forecolor, backcolor, transp, XbitOrder);

  quantizer.End ();

  return res;
}

static inline int rgb_dist (int tR, int tG, int tB, int sR, int sG, int sB)
{
  register int max = MAX (tR, tG);
  max = MAX (max, tB);

  sR -= tR; sG -= tG; sB -= tB;

  return R_COEF_SQ * sR * sR * (32 - ((max - tR) >> 3)) +
         G_COEF_SQ * sG * sG * (32 - ((max - tG) >> 3)) +
         B_COEF_SQ * sB * sB * (32 - ((max - tB) >> 3));
}

bool csCursorConverter::InternalConvertTo1bpp (iImage* image,
					       csColorQuantizer& quantizer, 
					       uint8*& bitmap, uint8*& mask,
					       const csRGBcolor forecolor, 
					       const csRGBcolor backcolor, 
					       csRGBpixel keycolor,
					       bool XbitOrder)
{
  csRGBpixel* pal = 0;
  int maxcolors = 3; // fg, bg, keycolor
  quantizer.Palette (pal, maxcolors, &keycolor);

  int i;
  int best_dist = 1000000;
  int fgIndex = -1;
  for (i = 1 ; i < maxcolors ; i++)
  {
    int dist = rgb_dist (forecolor.red, forecolor.green, forecolor.blue, 
      pal[i].red, pal[i].green, pal[i].blue);
    if (dist < best_dist)
    {
      best_dist = dist;
      fgIndex = i;
    }
    if (dist == 0) break;
  }
  if (fgIndex == -1)
    return false;

  const int imgW = image->GetWidth ();
  const int imgH = image->GetHeight ();
  uint8* outPixels = new uint8[imgW * imgH];
  csRGBpixel* imageData = (csRGBpixel*)image->GetImageData ();
  quantizer.RemapDither (imageData, imgW * imgH, imgW, pal, maxcolors, 
    outPixels, &keycolor);
#ifdef DEBUG_WRITE_IMAGES
  {
    csRGBpixel newPal[256];
    memcpy (newPal, pal, sizeof (csRGBpixel) * maxcolors);
    newPal[0] = keycolor;
    csRef<iImage> outImage;
    outImage.AttachNew (new csImageMemory (imgW, imgH, outPixels, false,
      CS_IMGFMT_PALETTED8, newPal));
    csDebugImageWriter::DebugImageWrite (outImage, "cursor-%s-remapped.png",
      image->GetName ());
  }
#endif
  delete[] pal;

  const int bytesPerBitmapLine = (imgW + 7) / 8;
  const int bitmapSize = bytesPerBitmapLine * imgH;
  bitmap = new uint8[bitmapSize];
  memset (bitmap, 0, bitmapSize);
  mask = new uint8[bitmapSize];
  memset (mask, 0, bitmapSize);
  uint8* pix = outPixels;
  for (int y = 0; y < imgH; y++)
  {
    for (int x = 0; x < imgW; x++)
    {
      if (*pix != 0)
      {
	int bitValue = (*pix == fgIndex) ? 1 : 0;
	size_t bitIndex = (y * bytesPerBitmapLine * 8) + x;
	int shift = XbitOrder ? (bitIndex % 8) : (7 - (bitIndex % 8));
	bitmap[bitIndex / 8] |= bitValue << shift;
	mask[bitIndex / 8] |= 1 << shift;
      }
      pix++;
    }
  }
  delete[] outPixels;
  return true;
}

void csCursorConverter::StripAlpha (iImage* image, csRGBpixel replaceColor)
{
  csRGBpixel* imageData = (csRGBpixel*)image->GetImageData ();
  int pixcount = image->GetWidth () * image->GetHeight ();
  replaceColor.alpha = 0;
  while (pixcount-- > 0)
  {
    if (imageData->alpha < 128)
    {
      *imageData = replaceColor;
    }
    else
    {
      imageData->alpha = 255;
    }
    imageData++;
  }
}
