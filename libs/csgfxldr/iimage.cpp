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

#include "sysdef.h"
#include "csgfxldr/csimage.h"
#include "iimage.h"

IMPLEMENT_COMPOSITE_UNKNOWN (ImageFile, ImageFile)

STDMETHODIMP IImageFile::GetImageData (RGBPixel** ppResult)
{
  METHOD_PROLOGUE (ImageFile, ImageFile)
  *ppResult = pThis->image;
  return S_OK;
}

IMPLEMENT_GET_PROPERTY (GetWidth, get_width (), int, ImageFile, ImageFile)
IMPLEMENT_GET_PROPERTY (GetHeight, get_height (), int, ImageFile, ImageFile)
IMPLEMENT_GET_PROPERTY (GetSize, get_size (), int, ImageFile, ImageFile)

STDMETHODIMP IImageFile::MipMap (int steps, IImageFile** nimage)
{
  METHOD_PROLOGUE (ImageFile, ImageFile)
  ImageFile* ifile = pThis->mipmap (steps);
  *nimage = GetIImageFileFromImageFile (ifile);
  (*nimage)->AddRef ();
  return S_OK;
}

STDMETHODIMP IImageFile::MipMap (int steps, Filter3x3* filt1, Filter5x5* filt2, IImageFile** nimage)
{
  METHOD_PROLOGUE (ImageFile, ImageFile)
  ImageFile* ifile = pThis->mipmap (steps, filt1, filt2);
  *nimage = GetIImageFileFromImageFile (ifile);
  (*nimage)->AddRef ();
  return S_OK;
}

STDMETHODIMP IImageFile::Blend (Filter3x3* filter, IImageFile** nimage)
{
  METHOD_PROLOGUE (ImageFile, ImageFile)
  ImageFile* ifile = pThis->blend (filter);
  *nimage = GetIImageFileFromImageFile (ifile);
  (*nimage)->AddRef ();
  return S_OK;
}
