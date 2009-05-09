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

    } // namespace CS
  } // namespace Platform
} // namespace Win32
