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

#include "csgfx/memimage.h"
#include "igraphic/image.h"

#include "csplugincommon/win32/customcursor.h"
#include "csplugincommon/canvas/cursorconvert.h"
#include <windows.h>

#include "csutil/win32/wintools.h"

csWin32CustomCursors::~csWin32CustomCursors ()
{
  csHash<CachedCursor, csStrKey, csConstCharHashKeyHandler>::GlobalIterator it =
    cachedCursors.GetIterator();

  while (it.HasNext())
  {
    CachedCursor cur = it.Next();
    if (cur.destroyAsIcon)
      DestroyIcon (cur.cursor);
    else
      DestroyCursor (cur.cursor);
  }
  for (size_t i = 0; i < blindCursors.Length(); i++)
  {
    CachedCursor& cur = blindCursors[i];
    if (cur.destroyAsIcon)
      DestroyIcon (cur.cursor);
    else
      DestroyCursor (cur.cursor);
  }
}

csWin32CustomCursors::CachedCursor csWin32CustomCursors::CreateMonoCursor (
  iImage* image, const csRGBcolor* keycolor, int hotspot_x, int hotspot_y)
{
  HCURSOR cursor;

  uint8* ANDmask;
  uint8* XORmask;
  if (!csCursorConverter::ConvertTo1bpp (image, XORmask, ANDmask, 
    csRGBcolor (255, 255, 255), csRGBcolor (0, 0, 0), keycolor)) 
    // @@@ Force color to black & white for now
    return CachedCursor ();

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

  return CachedCursor (cursor, false);
}

HCURSOR csWin32CustomCursors::GetMouseCursor (iImage* image, 
					      const csRGBcolor* keycolor, 
					      int hotspot_x, int hotspot_y, 
					      csRGBcolor fg, csRGBcolor bg)
{
  CachedCursor cursor;
  const char* cacheName = image->GetName();
  if (cacheName != 0)
  {
    cursor = cachedCursors.Get (cacheName, CachedCursor ());
    if (cursor.cursor != 0)
      return cursor.cursor;
  }

  cursor = CreateCursor (image, keycolor, hotspot_x, hotspot_y);
  //cursor = CreateMonoCursor (image, keycolor, hotspot_x, hotspot_y);

  if (cursor.cursor != 0)
  {
    if (cacheName != 0)
      cachedCursors.Put (cacheName, cursor);
    else
      blindCursors.Push (cursor);
  }
  return cursor.cursor;
}

static HBITMAP CreateCursorBitmapXP (HDC hDC, const BITMAPINFO* bitmapInfo,
                                      DWORD dibFlags, const void* data)
{
  return CreateDIBitmap (hDC, &bitmapInfo->bmiHeader, dibFlags, data, 
    bitmapInfo, DIB_RGB_COLORS);
}

static HBITMAP CreateCursorBitmapOther (HDC hDC, const BITMAPINFO* bitmapInfo,
                                         DWORD dibFlags, const void* data)
{
  HDC memDC = CreateCompatibleDC (hDC);
  HBITMAP bm = CreateCompatibleBitmap (hDC, bitmapInfo->bmiHeader.biWidth,
    -bitmapInfo->bmiHeader.biHeight);
  
  SetDIBits (memDC, bm, 0, -bitmapInfo->bmiHeader.biHeight, data, bitmapInfo,
    DIB_RGB_COLORS);
  DeleteDC (memDC);
  
  return bm;
}

typedef HBITMAP (*CreateCursorBitmapFN)(HDC hDC, const BITMAPINFO* bitmapInfo,
  DWORD dibFlags, const void* data);

csWin32CustomCursors::CachedCursor csWin32CustomCursors::CreateCursor(
  iImage* image, const csRGBcolor* keycolor,  int hotspot_x, int hotspot_y)
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
  CreateCursorBitmapFN CreateCursorBitmap = 
    (ver >= cswinWinXP) ? &CreateCursorBitmapXP : &CreateCursorBitmapOther;

  const int imgW = image->GetWidth();
  const int imgH = image->GetHeight();
  
  if ((ver == cswinWin9x) 
    && ((imgW != GetSystemMetrics (SM_CXCURSOR)) 
      || (imgH != GetSystemMetrics (SM_CYCURSOR))))
    // @@@ Support smaller cursors.
    return CachedCursor ();

  uint8* pixels = 0;
  csRGBpixel* palette = 0;
  uint8* pixelsRGB = 0;
  csRef<csImageMemory> imageRGB;
  csRGBpixel transp;
  if (doPaletted)
  {
    if (!csCursorConverter::ConvertTo8bpp (image, pixels, palette, keycolor))
      return CachedCursor ();
  }
  else
  {
    imageRGB.AttachNew (new csImageMemory (image));
    if (!doAlpha)
    {
      imageRGB->SetFormat (CS_IMGFMT_TRUECOLOR 
	| (image->GetFormat () & ~CS_IMGFMT_MASK));

      if (keycolor)
	transp = *keycolor;
      else
      {
	transp.Set (255, 0, 255);
	if (image->GetFormat () & CS_IMGFMT_ALPHA)
	  csCursorConverter::StripAlphaFromRGBA (imageRGB, transp);
      }
    }
    else
      imageRGB->SetFormat (CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA);

    int scanlineSize = doAlpha ? imgW * 4 : ((imgW * 3) + 3) & ~3;
    csRGBpixel* imageData = (csRGBpixel*)imageRGB->GetImageData();
    pixelsRGB = new uint8[scanlineSize * imgH];
    for (int y = 0; y < imgH; y++)
    {
      uint8* pixPtr = pixelsRGB + (y * scanlineSize);

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
	  }
	  else
	  {
	    *pixPtr++ = imageData->blue;
	    *pixPtr++ = imageData->green;
	    *pixPtr++ = imageData->red;
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
    delete[] palette;
  }

  HDC hClientDC	= GetDC (0);

  HBITMAP XORbitmap = CreateCursorBitmap (hClientDC, bmpInfoMem, CBM_INIT, 
    doPaletted ? pixels : pixelsRGB);

  HBITMAP ANDbitmap;
  if (!doAlpha)
  {
    // Create the monochrome AND mask
    int destScanlineSize = (((imgW + 7) / 8) + 1) & ~1;
    if (doPaletted)
    {
      int srcScanlineSize = (imgW + 1) & ~1;
      CS_ALLOC_STACK_ARRAY(uint8, destLine, destScanlineSize + 1);
      
      for (int y = 0; y < imgH; y++)
      {
        uint8* dest = destLine;
        memset (dest, 0, destScanlineSize);
        uint8* src = pixels + y * srcScanlineSize;
        
        for (int x = 0; x < imgW; x++)
        {
          if (*src++ == 0)
            *dest |= 1;
          
          if (((x + 1) % 8) == 0)
            dest++;
          else
            *dest <<= 1;
        }
        *dest <<= 7 - (imgW % 8);
        memcpy (pixels + y * destScanlineSize, destLine, destScanlineSize);
      }
      ANDbitmap = CreateBitmap (imgW, imgH, 1, 1, pixels);
    }
    else
    {
      csRGBpixel* imageData = (csRGBpixel*)imageRGB->GetImageData();
      uint8* pixels = new uint8[imgH * destScanlineSize];
      memset (pixels, 0, imgH * destScanlineSize);
      
      for (int y = 0; y < imgH; y++)
      {
        uint8* dest = pixels + y * destScanlineSize;
        
        for (int x = 0; x < imgW; x++)
        {
          if ((imageData++)->eq (transp))
            *dest |= 1;
          
          if (((x + 1) % 8) == 0)
            dest++;
          else
            *dest <<= 1;
        }
	if ((imgW % 8) != 0)
	  *dest <<= 7 - (imgW % 8);
      }
      ANDbitmap = CreateBitmap (imgW, imgH, 1, 1, pixels);
      delete[] pixels;
    }
  }
  else
  {
    ANDbitmap = CreateBitmap (imgW, imgH, 1, 1, 0);
      /* Apparently, for alpha cursors, the AND mask 
         contents don't matter, but you need _some_
         bitmap */
  }

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
  delete[] pixelsRGB;

  return CachedCursor (hCursor, true);
}
