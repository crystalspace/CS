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

#include "igraphic/image.h"

#include "customcursor.h"
#include "plugins/video/canvas/common/cursorconvert.h"
#include <windows.h>

#include "csutil/win32/wintools.h"

csWin32CustomCursors::~csWin32CustomCursors ()
{
  csHash<HCURSOR, csStrKey, csConstCharHashKeyHandler>::GlobalIterator it =
    cachedCursors.GetIterator();

  while (it.HasNext())
  {
    HCURSOR cur = it.Next();
    DestroyCursor (cur);
  }
}

HCURSOR csWin32CustomCursors::CreateMonoCursor (iImage* image, 
						const csRGBcolor* keycolor,
						int hotspot_x, int hotspot_y)
{
  HCURSOR cursor;

  uint8* ANDmask;
  uint8* XORmask;
  if (!csCursorConverter::ConvertTo1bpp (image, XORmask, ANDmask, csRGBcolor (255, 255, 255),
    csRGBcolor (0, 0, 0), keycolor)) // @@@ Force color to black & white for now
    return false;

  // Need to invert AND mask
  {
    uint8* ANDptr = ANDmask;
    int byteNum = ((image->GetWidth() + 7) / 8) * image->GetHeight();
    while (byteNum-- > 0)
    {
      *ANDptr++ ^= 0xff;
    }
  }

  cursor = ::CreateCursor (0, hotspot_x, hotspot_y, image->GetWidth(), 
    image->GetHeight(), ANDmask, XORmask);
  delete[] ANDmask;
  delete[] XORmask;

  return cursor;
}

HCURSOR csWin32CustomCursors::GetMouseCursor (iImage* image, 
					      const csRGBcolor* keycolor, 
					      int hotspot_x, int hotspot_y, 
					      csRGBcolor fg, csRGBcolor bg)
{
  HCURSOR cursor;
  const char* cacheName = image->GetName();
  if (cacheName != 0)
  {
    cursor = cachedCursors.Get (cacheName, 0);
    if (cursor != 0)
      return cursor;
  }

  cursor = CreateCursor (image, keycolor, hotspot_x, hotspot_y);
  //cursor = CreateMonoCursor (image, keycolor, hotspot_x, hotspot_y);

  if ((cursor != 0) && (cacheName != 0))
  {
    cachedCursors.Put (cacheName, cursor);
  }
  return cursor;
}

HCURSOR csWin32CustomCursors::CreateCursor(iImage* image,
					   const csRGBcolor* keycolor, 
					   int hotspot_x, int hotspot_y)
{
  cswinWindowsVersion ver;
  cswinIsWinNT (&ver);
  // Use alpha cursor when we can (Win2K+) and the image actually possesses
  // alpha data
  bool doAlpha = (ver >= cswinWin2K)
    && (image->GetFormat() & CS_IMGFMT_ALPHA);
  // Only use a paletted cursor when we're on NT4.0 or the source image
  // has a palette, but we won't use alpha for it.
  bool doPaletted = (ver == cswinWinNT) 
    || (((image->GetFormat() & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8)
    && !doAlpha);

  const int imgW = image->GetWidth();
  const int imgH = image->GetHeight();

  uint8* pixels = 0;
  csRGBpixel* palette = 0;
  uint8* pixelsRGB = 0;
  uint8* maskRGB = 0;
  if (doPaletted)
  {
    if (!csCursorConverter::ConvertTo8bpp (image, pixels, palette, keycolor))
      return 0;
  }
  else
  {
    csRef<iImage> myImage = image->Clone();
    csRGBpixel transp;
    if (!doAlpha)
    {
      myImage->SetFormat (CS_IMGFMT_TRUECOLOR 
	| (image->GetFormat () & ~CS_IMGFMT_MASK));

      if (keycolor)
	transp = *keycolor;
      else
      {
	transp.Set (255, 0, 255);
	if (image->GetFormat () & CS_IMGFMT_ALPHA)
	  csCursorConverter::StripAlphaFromRGBA (myImage, transp);
      }
    }
    else
      myImage->SetFormat (CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA);

    int scanlineSize = doAlpha ? imgW * 4 : ((imgW * 3) + 3) & ~3;
    csRGBpixel* imageData = (csRGBpixel*)myImage->GetImageData();
    pixelsRGB = new uint8[scanlineSize * imgH];
    if (!doAlpha)
      maskRGB = new uint8[scanlineSize * imgH];
    for (int y = 0; y < imgH; y++)
    {
      uint8* pixPtr = pixelsRGB + (y * scanlineSize);
      uint8* maskPtr = maskRGB + (y * scanlineSize);

      for (int x = 0; x < imgW; x++)
      {
	if (doAlpha)
	{
	  *pixPtr++ = imageData->blue;
	  *pixPtr++ = imageData->green;
	  *pixPtr++ = imageData->red;
	  *pixPtr++ = imageData->alpha;
	}
	else
	{
	  if (imageData->eq (transp))
	  {
	    *pixPtr++ = 0;
	    *pixPtr++ = 0;
	    *pixPtr++ = 0;
	    *maskPtr++ = 255;
	    *maskPtr++ = 255;
	    *maskPtr++ = 255;
	  }
	  else
	  {
	    *pixPtr++ = imageData->blue;
	    *pixPtr++ = imageData->green;
	    *pixPtr++ = imageData->red;
	    *maskPtr++ = 0;
	    *maskPtr++ = 0;
	    *maskPtr++ = 0;
	  }
	}
	imageData++;
      }
    }
  }
 
  BITMAPINFO* bmpInfoMem;
  bmpInfoMem = (BITMAPINFO *)calloc (sizeof (BITMAPINFOHEADER) + 
    (doPaletted ? sizeof (RGBQUAD) * 256 : 0), 1);

  bmpInfoMem->bmiHeader.biSize           = sizeof (BITMAPINFOHEADER);
  bmpInfoMem->bmiHeader.biWidth          = imgW;
  bmpInfoMem->bmiHeader.biHeight         = -imgH;
  bmpInfoMem->bmiHeader.biPlanes         = 1;
  bmpInfoMem->bmiHeader.biBitCount       = doPaletted ? 8 : (doAlpha ? 32 : 24);
  bmpInfoMem->bmiHeader.biCompression    = BI_RGB;

  int i;
  if (doPaletted)
  {
    for(i = 0; i < 256; i++)
    {
      bmpInfoMem->bmiColors[i].rgbRed = palette[i].red;
      bmpInfoMem->bmiColors[i].rgbGreen = palette[i].green;
      bmpInfoMem->bmiColors[i].rgbBlue = palette[i].blue;
    }
    bmpInfoMem->bmiColors[0].rgbRed = 0;
    bmpInfoMem->bmiColors[0].rgbGreen = 0;
    bmpInfoMem->bmiColors[0].rgbBlue = 0;
 
    if (imgW & 1)
    {
      // Pixel data needs WORD alignment
      size_t scanlineSize = imgW + 1;
      uint8* alignedPixels = new uint8[scanlineSize * imgH];
      for (int y = 0; y < imgH; y++)
      {
	memcpy (alignedPixels + (y * scanlineSize), pixels + (y * imgW), imgW);
      }
      delete[] pixels; pixels = alignedPixels;
    }
  }

  HDC hClientDC		    = GetDC (0);

  HBITMAP XORbitmap = CreateDIBitmap (hClientDC, &bmpInfoMem->bmiHeader, CBM_INIT, 
    doPaletted ? pixels : pixelsRGB, bmpInfoMem, DIB_RGB_COLORS);

  if (doPaletted)
  {
    for(i = 1; i < 256; i++)
    {
      bmpInfoMem->bmiColors[i].rgbRed = 0;
      bmpInfoMem->bmiColors[i].rgbGreen = 0;
      bmpInfoMem->bmiColors[i].rgbBlue = 0;
    }
    bmpInfoMem->bmiColors[0].rgbRed = 255;
    bmpInfoMem->bmiColors[0].rgbGreen = 255;
    bmpInfoMem->bmiColors[0].rgbBlue = 255;
  }

  HBITMAP ANDbitmap = CreateDIBitmap (hClientDC, &bmpInfoMem->bmiHeader, 
    doAlpha ? 0 : CBM_INIT, /* Apparently, for alpha cursors, the AND mask 
			       contents don't matter, but you need _some_
			       bitmap */
    doPaletted ? pixels : maskRGB, bmpInfoMem, DIB_RGB_COLORS);

  ICONINFO iconInfo;
  iconInfo.fIcon          = false;
  iconInfo.xHotspot	  = hotspot_x;
  iconInfo.yHotspot	  = hotspot_y;
  iconInfo.hbmMask	  = ANDbitmap;
  iconInfo.hbmColor	  = XORbitmap;

  HCURSOR hCursor = CreateIconIndirect (&iconInfo);

  ReleaseDC (0, hClientDC);
  if (ANDbitmap) DeleteObject (ANDbitmap);
  if (XORbitmap) DeleteObject (XORbitmap);
  free (bmpInfoMem);
  delete[] pixels;
  delete[] palette;
  delete[] pixelsRGB;
  delete[] maskRGB;

  return hCursor;
}
