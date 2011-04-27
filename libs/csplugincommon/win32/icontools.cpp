/*
    Copyright (C) 2009 by Frank Richter

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

#include "csplugincommon/win32/icontools.h"

#include "csgeom/math.h"
#include "csgfx/imagememory.h"
#include "csutil/win32/cachedll.h"
#include "csutil/win32/wintools.h"
#include "csplugincommon/canvas/cursorconvert.h"

#include <windows.h>
#include "csutil/win32/psdk-compat.h"

/* HACK: The IDI_ values are macros defined to use MAKEINTRESOURCE.
   MAKEINTRESOURCE is a cast to a string pointer; these can't be used as case
   values in switches.
   In order to nevertheless be able to use the IDI_ values in case labels
   define MAKEINTRESOURCE to be a no-op. */
#undef MAKEINTRESOURCE
#define MAKEINTRESOURCE(X)      X

namespace CS
{
  namespace Platform
  {
    namespace Win32
    {
      // Helper to get pointer to resource data
      static void* GetResourceData (HMODULE module, LPCTSTR name, LPCTSTR type, DWORD* size = 0)
      {
	HRSRC resInfo = FindResource (module, name, type);
	if (resInfo)
	{
	  if (size) *size = SizeofResource (module, resInfo);
	  HGLOBAL res = LoadResource (module, resInfo);
	  if (res)
	  {
	    return LockResource (res);
	  }
	}
	return 0;
      }

      HICON IconTools::LoadStockIconSize (uintptr_t id, int desiredSize)
      {
	CS::Platform::Win32::CacheDLL hUser32 ("user32.dll");
	WORD resID;
	/* @@@ Rather ugly, but there is no way to get this information -
	   Vista has GetIconInfoEx but that returns a useless resource ID for stock
	   icons. Using LoadIcon() + CopyImage() to resize uses a single-size icon
	   (typically 32x32) and resizes uglily. */
	switch (id)
	{
	  case IDI_APPLICATION:  resID = 100; break;
	  case IDI_WARNING:      resID = 101; break;
	  case IDI_QUESTION:     resID = 102; break;
	  case IDI_ERROR:        resID = 103; break;
	  case IDI_INFORMATION:  resID = 104; break;
	  case IDI_WINLOGO:      resID = 105; break;
	  case IDI_SHIELD:       resID = 106; break;
	  default: return 0;
	}

	HICON loadedIcon = 0;

	void* groupData = GetResourceData (hUser32, (TCHAR*)(uintptr_t)(resID),
	  (TCHAR*)RT_GROUP_ICON);
	if (groupData)
	{
	  int iconID = LookupIconIdFromDirectoryEx ((PBYTE)groupData, true, desiredSize, desiredSize, 0);
	  if (iconID != 0)
	  {
	    DWORD iconDataSize;
	    void* iconData = GetResourceData (hUser32, (const TCHAR*)(iconID),
	      (TCHAR*)RT_ICON, &iconDataSize);
	    if (iconData)
	    {
	      return CreateIconFromResourceEx ((PBYTE)iconData, iconDataSize, true,
		0x00030000, 0, 0, 0);
	    }
	  }
	}

	return loadedIcon;
      }

      csPtr<iImage> IconTools::IconToImage (HICON icon)
      {
	csRef<iImage> img;

	ICONINFO iconinfo;
	if (GetIconInfo (icon, &iconinfo))
	{
	  HDC screenDC = GetDC (0);
	  BITMAPINFO bmi;
          
	  memset (&bmi, 0, sizeof (bmi));
	  bmi.bmiHeader.biSize = sizeof (bmi.bmiHeader);
	  if (GetDIBits (screenDC, iconinfo.hbmColor, 0, 0, 0, &bmi, DIB_RGB_COLORS))
	  {
	    int iconW = ABS (bmi.bmiHeader.biWidth);
	    int iconH = ABS (bmi.bmiHeader.biHeight);
	    csImageMemory* newImg = new csImageMemory (iconW, iconH,
	      CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA);
	    bmi.bmiHeader.biHeight = -iconH;
	    bmi.bmiHeader.biCompression = BI_RGB;
	    int orgBits = bmi.bmiHeader.biBitCount;
	    bmi.bmiHeader.biBitCount = 32;
	    if (GetDIBits (screenDC, iconinfo.hbmColor,
		0, iconH,
		newImg->GetImagePtr(), &bmi, DIB_RGB_COLORS))
	    {
	      {
		// Convert BGR to RGB
		csRGBpixel* dstPtr = (csRGBpixel*)newImg->GetImageData();
		for (int y = 0; y < iconH; y++)
		{
		  for (int x = 0; x < iconW; x++)
		  {
		    CS::Swap (dstPtr->red, dstPtr->blue);
		    dstPtr++;
		  }
		}
	      }
	      img.AttachNew (newImg);
	      if (orgBits < 32)
	      {
		/* When the icon image has less than 32 bits the alpha channel is
		   unset. Copy the alpha data from the mask bitmap */
		BITMAPINFO* bwbmi = (BITMAPINFO*)cs_malloc (sizeof (BITMAPINFO) + sizeof (RGBQUAD));
		memset (bwbmi, 0, sizeof (bwbmi));
		bwbmi->bmiHeader.biSize = sizeof (bwbmi->bmiHeader);
		bwbmi->bmiHeader.biCompression = BI_RGB;
		bwbmi->bmiHeader.biWidth = bmi.bmiHeader.biWidth;
		bwbmi->bmiHeader.biHeight = bmi.bmiHeader.biHeight;
		bwbmi->bmiHeader.biBitCount = 1;
		bwbmi->bmiHeader.biPlanes = 1;
		bwbmi->bmiColors[0].rgbRed = 0;
		bwbmi->bmiColors[0].rgbGreen = 0;
		bwbmi->bmiColors[0].rgbBlue = 0;
		bwbmi->bmiColors[0].rgbReserved = 0;
		bwbmi->bmiColors[1].rgbRed = 255;
		bwbmi->bmiColors[1].rgbGreen = 255;
		bwbmi->bmiColors[1].rgbBlue = 255;
		bwbmi->bmiColors[1].rgbReserved = 0;

		// Scan lines are DWORD-aligned
		size_t scanLineSize = ((iconW + 31) & ~31) / 8;
		size_t maskSize = scanLineSize * iconH;
		uint8* maskData = (uint8*)cs_malloc (maskSize);
		if (!GetDIBits (screenDC, iconinfo.hbmMask,
		    0, ABS (bmi.bmiHeader.biHeight),
		    maskData, bwbmi, DIB_RGB_COLORS))
		  memset (maskData, 0, maskSize);
		csRGBpixel* dstPtr = (csRGBpixel*)newImg->GetImageData();
		for (int y = 0; y < iconH; y++)
		{
		  uint8* linePtr = maskData + scanLineSize * y;
		  for (int x = 0; x < iconW; x++)
		  {
		    // The mask is an AND mask, ie 0 means opaque, 1 means transparent
		    dstPtr->alpha = ((~(*linePtr) >> (7-(x & 7))) & 1) * 255;
		    dstPtr++;
		    if ((x & 7) == 7) linePtr++;
		  }
		}

		cs_free (maskData);
		cs_free (bwbmi);
	      }
	    }
	    else
	      delete newImg;
	  }

	  ReleaseDC (0, screenDC);

	  DeleteObject (iconinfo.hbmColor);
	  DeleteObject (iconinfo.hbmMask);
	}
	return csPtr<iImage> (img);
      }

      static HBITMAP CreateCursorBitmapXP (HDC hDC, const BITMAPINFO* bitmapInfo,
					    DWORD dibFlags, const void* data)
      {
	return CreateDIBitmap (hDC, &bitmapInfo->bmiHeader, dibFlags, data, 
	  bitmapInfo, DIB_RGB_COLORS);
      }

      static HBITMAP CreateCursorBitmapOther (HDC hDC, const BITMAPINFO* bitmapInfo,
					       DWORD /*dibFlags*/, const void* data)
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

      HICON IconTools::IconFromImage (iImage* image, const ICONINFO* iconTemplate)
      {
	cswinWindowsVersion ver;
	cswinIsWinNT (&ver);
	HDC DC = GetDC (0);
	int colorDepth = GetDeviceCaps (DC, BITSPIXEL);
	ReleaseDC (0, DC);
	// Use alpha cursor when we can (Win2K+), the image actually possesses
	// alpha data, and if we have more than 24bpp color depth
	bool doAlpha = (ver >= cswinWin2K)
	  && (image->GetFormat() & CS_IMGFMT_ALPHA)
	  && (colorDepth >= 24);
	// Only use a paletted cursor when we're on NT4.0 or the source image
	// has a palette, but we won't use alpha for it.
	bool doPaletted = (ver == cswinWinNT) 
	  || (((image->GetFormat() & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8)
	  && !doAlpha);
	CreateCursorBitmapFN CreateCursorBitmap = 
	  (ver >= cswinWinXP) ? &CreateCursorBitmapXP : &CreateCursorBitmapOther;

	const int imgW = image->GetWidth();
	const int imgH = image->GetHeight();
        
	uint8* pixels = 0;
	csRGBpixel* palette = 0;
	uint8* pixelsRGB = 0;
	csRef<csImageMemory> imageRGB;
	csRGBpixel transp;
	if (doPaletted)
	{
	  if (!csCursorConverter::ConvertTo8bpp (image, pixels, palette, 0))
	    return 0;
	}
	else
	{
      #include "csutil/custom_new_disable.h"
	  imageRGB.AttachNew (new csImageMemory (image));
      #include "csutil/custom_new_enable.h"
	  if (!doAlpha)
	  {
	    imageRGB->SetFormat (CS_IMGFMT_TRUECOLOR 
	      | (image->GetFormat () & ~CS_IMGFMT_MASK));

	    int tr = 255, tg = 0, tb = 255;
	    if (image->HasKeyColor ())
	      image->GetKeyColor (tr, tg, tb);
	    transp.Set (tr, tg, tb);
	    if (image->GetFormat () & CS_IMGFMT_ALPHA)
	      csCursorConverter::StripAlphaFromRGBA (imageRGB, transp);
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
	if (iconTemplate != 0)
	  iconInfo = *iconTemplate;
	else
	{
	  iconInfo.fIcon          = true;
	  iconInfo.xHotspot	  = 0;
	  iconInfo.yHotspot	  = 0;
	}
	iconInfo.hbmMask	  = ANDbitmap;
	iconInfo.hbmColor	  = XORbitmap;

	HICON icon = CreateIconIndirect (&iconInfo);

	ReleaseDC (0, hClientDC);
	if (ANDbitmap) DeleteObject (ANDbitmap);
	if (XORbitmap) DeleteObject (XORbitmap);
	free (bmpInfoMem);
	delete[] pixels;
	delete[] pixelsRGB;

	return icon;
      }

    } // namespace Win32
  } // namespace Platform
} // namespace CS
