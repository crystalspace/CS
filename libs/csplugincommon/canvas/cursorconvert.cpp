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

#include "csplugincommon/canvas/cursorconvert.h"

#define DEBUG_WRITE_IMAGES

static inline int rgb_dist (int tR, int tG, int tB, int sR, int sG, int sB)
{
  register int max = MAX (tR, tG);
  max = MAX (max, tB);

  sR -= tR; sG -= tG; sB -= tB;

  return R_COEF_SQ * sR * sR * (32 - ((max - tR) >> 3)) +
         G_COEF_SQ * sG * sG * (32 - ((max - tG) >> 3)) +
         B_COEF_SQ * sB * sB * (32 - ((max - tB) >> 3));
}

bool csCursorConverter::ConvertTo1bpp (iImage* image, uint8*& bitmap, 
				       uint8*& mask,
				       const csRGBcolor forecolor, 
				       const csRGBcolor backcolor, 
				       const csRGBcolor* keycolor,
				       bool XbitOrder)
{
  csRef<csImageMemory> myImage;
  myImage.AttachNew (new csImageMemory (image, CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA));
  myImage->SetName (image->GetName ());

  csColorQuantizer quantizer;
  quantizer.Begin ();

  csRGBpixel pal[3];
  pal[0] = forecolor;
  pal[1] = backcolor;
  csRGBpixel transp;
  if (keycolor)
    transp = *keycolor;
  else
  {
    transp.Set (255, 0, 255);
    StripAlphaFromRGBA (myImage, transp);
#ifdef DEBUG_WRITE_IMAGES
    csDebugImageWriter::DebugImageWrite (myImage, "cursor-%s-stripped.png",
      image->GetName ());
#endif
  }
  quantizer.Count (pal, 2, &transp);
  csRGBpixel* palPtr = 0;
  int maxcolors = 3; // fg, bg, keycolor
  quantizer.Palette (palPtr, maxcolors, &transp);
  int i;
  int best_dist = 1000000;
  int fgIndex = -1;
  for (i = 1 ; i < maxcolors ; i++)
  {
    int dist = rgb_dist (forecolor.red, forecolor.green, forecolor.blue, 
      palPtr[i].red, palPtr[i].green, palPtr[i].blue);
    if (dist < best_dist)
    {
      best_dist = dist;
      fgIndex = i;
    }
    if (dist == 0) break;
  }
  if (fgIndex == -1)
    return false;

  bool res = InternalConvertTo1bpp (myImage, quantizer, bitmap, mask, 
    fgIndex, transp, palPtr, maxcolors, XbitOrder);

  quantizer.End ();
  delete[] palPtr;

  return res;
}

bool csCursorConverter::ConvertTo1bppAutoColor (iImage* image, uint8*& bitmap, 
				                uint8*& mask,
				                csRGBcolor& forecolor, 
				                csRGBcolor& backcolor, 
				                const csRGBcolor* keycolor,
				                bool XbitOrder)
{
  csRef<csImageMemory> myImage;
  myImage.AttachNew (new csImageMemory (image, CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA));
  myImage->SetName (image->GetName ());

  csColorQuantizer quantizer;
  quantizer.Begin ();

  csRGBpixel transp;
  if (keycolor)
    transp = *keycolor;
  else
  {
    transp.Set (255, 0, 255);
    StripAlphaFromRGBA (myImage, transp);
#ifdef DEBUG_WRITE_IMAGES
    csDebugImageWriter::DebugImageWrite (myImage, "cursor-%s-stripped.png",
      image->GetName ());
#endif
  }
  quantizer.Count ((csRGBpixel*)myImage->GetImageData(), 
    myImage->GetWidth() * myImage->GetHeight(), &transp);
  csRGBpixel* pal = 0;
  int maxcolors = 3; // fg, bg, keycolor
  quantizer.Palette (pal, maxcolors, &transp);
  int fgIndex = (pal[0] != transp) ? 0 : 1;

  bool res = InternalConvertTo1bpp (myImage, quantizer, bitmap, mask, 
    fgIndex, transp, pal, maxcolors, XbitOrder);

  quantizer.End ();
  delete[] pal;

  return res;
}

bool csCursorConverter::InternalConvertTo1bpp (iImage* image,
					       csColorQuantizer& quantizer, 
					       uint8*& bitmap, uint8*& mask,
					       int fgIndex,
					       csRGBpixel keycolor,
					       csRGBpixel* pal, 
					       int maxcolors, 
					       bool XbitOrder)
{
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
	int bitIndex = (y * bytesPerBitmapLine * 8) + x;
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

bool csCursorConverter::ConvertTo8bpp (iImage* image, uint8*& pixels, 
				       csRGBpixel*& palette, 
				       const csRGBcolor* keycolor)
{
  const int imgW = image->GetWidth ();
  const int imgH = image->GetHeight ();
  csRef<csImageMemory> myImage;
  myImage.AttachNew (new csImageMemory (imgW, imgH, image->GetFormat ()));
  myImage->SetName (image->GetName ());

  size_t dataSize = imgW * imgH;
  if ((image->GetFormat () & CS_IMGFMT_MASK) == CS_IMGFMT_TRUECOLOR)
    dataSize *= sizeof (csRGBpixel);

  memcpy (myImage->GetImagePtr(), image->GetImageData(),
    dataSize);

  csRGBcolor transp;
  if (keycolor)
    transp = *keycolor;
  else
  {
    int tr = 255, tg = 0, tb = 255;
    if (image->HasKeyColor ())
      image->GetKeyColor (tr, tg, tb);
    transp.Set (tr, tg, tb);
  }
  myImage->SetKeyColor (transp.red, transp.green, transp.blue);

  myImage->SetFormat (CS_IMGFMT_PALETTED8 
    | (image->GetFormat () & ~CS_IMGFMT_MASK));
  if ((image->GetFormat() & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8)
  {
    // copy over the original palette
    memcpy (myImage->GetPalettePtr(), image->GetPalette(), 
      sizeof (csRGBpixel) * 256);
    myImage->ApplyKeyColor ();
  }
#ifdef DEBUG_WRITE_IMAGES
  csDebugImageWriter::DebugImageWrite (myImage, "cursor-%s-8bpp.png",
    image->GetName ());
#endif

  if (myImage->GetFormat () & CS_IMGFMT_ALPHA)
  {
    StripAlphaFromPal8 (myImage);
#ifdef DEBUG_WRITE_IMAGES
    csDebugImageWriter::DebugImageWrite (myImage, "cursor-%s-stripped.png",
      image->GetName ());
#endif
  }

  pixels = new uint8[imgW * imgH];
  memcpy (pixels, myImage->GetImageData (), imgW * imgH);
  palette = new csRGBpixel[256];
  memcpy (palette, myImage->GetPalette (), 256 * sizeof (csRGBpixel));

  return true;
}

#define ALPHA_DITHERING

void csCursorConverter::StripAlphaFromRGBA (iImage* image, 
					    csRGBpixel replaceColor)
{
  CS_ASSERT_MSG (
    "csCursorConverter::StripAlphaFromRGBA called with image of wrong format",
    ((image->GetFormat () & CS_IMGFMT_MASK) == CS_IMGFMT_TRUECOLOR)
    && (image->GetFormat () & CS_IMGFMT_ALPHA));;
#ifdef ALPHA_DITHERING
  // Fancy: dither alpha to opaque/transparent
  csRGBpixel* imageData = (csRGBpixel*)image->GetImageData ();
  int pixcount = image->GetWidth () * image->GetHeight ();
  csRGBpixel* alphaData = new csRGBpixel[pixcount];
  csRGBpixel* imagePtr = imageData;
  csRGBpixel* alphaPtr = alphaData;
  int p;
  for (p = 0; p < pixcount; p++)
  {
    int a = (imagePtr++)->alpha;
    (alphaPtr++)->Set (a, a, a);
  }

  csColorQuantizer quantizer;
  quantizer.Begin ();

  csRGBpixel* pal = 0;
  int maxcolors = 2;

  quantizer.Count (alphaData, pixcount);
  quantizer.Palette (pal, maxcolors);

  uint8* alphaDithered = 0;
  quantizer.RemapDither (alphaData, pixcount, image->GetWidth (), pal, 
    maxcolors, alphaDithered);

  imagePtr = imageData;
  uint8* ditherPtr = alphaDithered;
  for (p = 0; p < pixcount; p++)
  {
    if (pal[*ditherPtr++].red < 128)
    {
      *imagePtr = replaceColor;
    }
    else
    {
      imagePtr->alpha = 255;
    }
    imagePtr++;
  }

  delete[] alphaData;
  delete[] pal;
  delete[] alphaDithered;
#else
  // Plain: cut off all alphas below 0.5
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
#endif
}

void csCursorConverter::StripAlphaFromPal8 (csImageMemory* image)
{
  CS_ASSERT_MSG (
    "csCursorConverter::StripAlphaFromPal8 called with image of wrong format",
    ((image->GetFormat () & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8)
    && (image->GetFormat () & CS_IMGFMT_ALPHA));;
#ifdef ALPHA_DITHERING
  // Fancy: dither alpha to opaque/transparent
  uint8* imageData = image->GetAlphaPtr ();
  int pixcount = image->GetWidth () * image->GetHeight ();
  csRGBpixel* alphaData = new csRGBpixel[pixcount];
  uint8* imagePtr = imageData;
  csRGBpixel* alphaPtr = alphaData;
  int p;
  for (p = 0; p < pixcount; p++)
  {
    int a = *imagePtr++;
    (alphaPtr++)->Set (a, a, a);
  }

  csColorQuantizer quantizer;
  quantizer.Begin ();

  csRGBpixel* pal = 0;
  int maxcolors = 2;

  quantizer.Count (alphaData, pixcount);
  quantizer.Palette (pal, maxcolors);

  uint8* alphaDithered = 0;
  quantizer.RemapDither (alphaData, pixcount, image->GetWidth (), pal, 
    maxcolors, alphaDithered);

  imagePtr = (uint8*)image->GetImagePtr ();
  uint8* ditherPtr = alphaDithered;
  for (p = 0; p < pixcount; p++)
  {
    if (pal[*ditherPtr++].red < 128)
    {
      *imagePtr = 0;
    }
    imagePtr++;
  }

  delete[] alphaData;
  delete[] pal;
  delete[] alphaDithered;
#else
  // Plain: cut off all alphas below 0.5
  uint8* imageData = (uint8*)image->GetImageData ();
  uint8* alphaData = (uint8*)image->GetAlpha ();
  int pixcount = image->GetWidth () * image->GetHeight ();
  while (pixcount-- > 0)
  {
    if (*alphaData < 128)
    {
      *imageData = 0;
    }
    imageData++;
    alphaData++;
  }
#endif
}
