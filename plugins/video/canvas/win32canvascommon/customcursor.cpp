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

csWin32CustomCursors::~csWin32CustomCursors ()
{
  csHash<HCURSOR, csStrKey, csConstCharHashKeyHandler>::GlobalIterator it =
    cachedCursors.GetIterator ();

  while (it.HasNext ())
  {
    HCURSOR cur = it.Next ();
    DestroyCursor (cur);
  }
}

HCURSOR csWin32CustomCursors::GetMouseCursor (iImage* image, 
                                              const csRGBcolor* keycolor, 
                                              int hotspot_x, int hotspot_y, 
                                              csRGBcolor fg, csRGBcolor bg)
{
  HCURSOR cursor;
  const char* cacheName = image->GetName ();
  if (cacheName != 0)
  {
    cursor = cachedCursors.Get (cacheName, 0);
    if (cursor != 0)
      return cursor;
  }

  if (keycolor)
    cursor = CreateCursor(image, keycolor, hotspot_x, hotspot_y);
  else
  {
    csRGBcolor keycolor;
    cursor = CreateCursor(image, &keycolor, hotspot_x, hotspot_y);
  }

  if (cacheName != 0)
  {
    cachedCursors.Put (cacheName, cursor);
  }
  return cursor;
}

HCURSOR csWin32CustomCursors::CreateCursor(iImage* image,
	const csRGBcolor* keycolor, int hotspot_x, int hotspot_y)
{
  bool paletted8 = image->GetFormat()==CS_IMGFMT_PALETTED8;

  BITMAPINFO* bmpInfoMem;

  if (paletted8)
    bmpInfoMem = (BITMAPINFO *) malloc(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256);
  else
    bmpInfoMem = (BITMAPINFO *) malloc(sizeof(BITMAPINFO));

  bmpInfoMem->bmiHeader.biSize               = sizeof(BITMAPINFOHEADER);
  bmpInfoMem->bmiHeader.biWidth          = image->GetWidth( );
  bmpInfoMem->bmiHeader.biHeight          = -(image->GetHeight( ));
  bmpInfoMem->bmiHeader.biPlanes          = 1;
  bmpInfoMem->bmiHeader.biBitCount          = (paletted8 ? 8:24);
  bmpInfoMem->bmiHeader.biCompression     = BI_RGB;

  if (paletted8)
  {
    csRGBpixel* palette = image->GetPalette();
    for(int i = 0;i<256;i++)
    {
      bmpInfoMem->bmiColors[i].rgbRed = palette[i].red;
      bmpInfoMem->bmiColors[i].rgbGreen = palette[i].green;
      bmpInfoMem->bmiColors[i].rgbBlue = palette[i].blue;
    }
  }

  HDC hClientDC          = GetDC( NULL );

  HDC hDCCreate          = CreateCompatibleDC( hClientDC );
  HDC hDCImage          = CreateCompatibleDC( hClientDC );
  HDC hDCAnd               = CreateCompatibleDC( hClientDC );
  HDC hDCXOr               = CreateCompatibleDC( hClientDC );

  HBITMAP hDDB          = CreateCompatibleBitmap( hClientDC, image->GetWidth( ), image->GetHeight( ) );
  HBITMAP hXOrBitmap     = CreateCompatibleBitmap( hClientDC, image->GetWidth( ), image->GetHeight( ) );
  HBITMAP hAndBitmap     = CreateBitmap( image->GetWidth( ), image->GetHeight( ), 1, 1, NULL );

  csRGBpixel* imageData = (csRGBpixel*) image->GetImageData();

  BYTE* bytes = (BYTE*) malloc(3*image->GetWidth()*image->GetHeight());
  BYTE* startBytes = bytes;

  // Changes format to BGR as requested by windows
  for(int y=0;y<(image->GetWidth()*image->GetHeight());y++)
  {
    if (imageData[y].alpha == 0)
    {
      *bytes++ = keycolor->blue;
      *bytes++ = keycolor->green;
      *bytes++ = keycolor->red;
      continue;
    }

    *bytes++ = imageData[y].blue;
    *bytes++ = imageData[y].green;
    *bytes++ = imageData[y].red;
  }


  if (SetDIBits( hDCCreate,
    hDDB,
    0,
    image->GetHeight( ),
    startBytes,
    bmpInfoMem, 
    DIB_RGB_COLORS ) != image->GetHeight())
  {
    DeleteDC( hDCCreate );
    DeleteDC( hDCXOr );
    DeleteDC( hDCImage );
    DeleteDC( hDCAnd );
    DeleteObject( hDDB );
    DeleteObject( hAndBitmap );
    DeleteObject( hXOrBitmap );
    return 0;
  }

  DeleteDC( hDCCreate );

  // Create the AND Mask bitmap
  HBITMAP hOldAndBitmap     = (HBITMAP)SelectObject( hDCAnd, hAndBitmap );
  HBITMAP hOldImageBitmap     = (HBITMAP)SelectObject( hDCImage, hDDB );
  HBITMAP hOldXorBitmap     = (HBITMAP)SelectObject( hDCXOr, hXOrBitmap );
  // Create the monochrome mask bitmap from the source image using the background color

  COLORREF mainBitPixel;
  for(int x=0;x<image->GetWidth();++x)
  {
    for(int y=0;y<image->GetHeight();++y)
    {
      mainBitPixel = GetPixel(hDCImage,x,y);
      if(mainBitPixel == RGB(keycolor->red,keycolor->green,keycolor->blue))
      {
        SetPixel(hDCAnd,x,y,RGB(255,255,255));
        SetPixel(hDCXOr,x,y,RGB(0,0,0));
      }
      else
      {
        SetPixel(hDCAnd,x,y,RGB(0,0,0));
        SetPixel(hDCXOr,x,y,mainBitPixel);
      }
    }
  }

  SelectObject( hDCImage, hOldImageBitmap );
  SelectObject( hDCAnd, hOldAndBitmap );
  SelectObject( hDCXOr, hOldXorBitmap );

  DeleteDC( hDCXOr );
  DeleteDC( hDCImage );
  DeleteDC( hDCAnd );

  ReleaseDC( NULL, hClientDC );

  ICONINFO iconInfo;
  iconInfo.fIcon          = false;
  iconInfo.xHotspot     = hotspot_x;
  iconInfo.yHotspot     = hotspot_y;
  iconInfo.hbmMask     = hAndBitmap;
  iconInfo.hbmColor     = hXOrBitmap;

  HCURSOR hCursor = CreateIconIndirect( &iconInfo );

  DeleteObject( hDDB );
  DeleteObject( hAndBitmap );
  DeleteObject( hXOrBitmap );

  free(bmpInfoMem);
  free(startBytes);

  return hCursor;
}
