/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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
#include "csutil/csstring.h"
#include "csgfx/imagemanipulate.h"
#include "csgfx/imagevolumemaker.h"
#include "csgfx/memimage.h"
#include "csgfx/xorpat.h"

CS_LEAKGUARD_IMPLEMENT (csImageVolumeMaker);

SCF_IMPLEMENT_IBASE (csImageVolumeMaker)
  SCF_IMPLEMENTS_INTERFACE (iImage)
SCF_IMPLEMENT_IBASE_END

csImageVolumeMaker::csImageVolumeMaker (int format, int width, int height)
  : csImageBase(), manualName (false), Width (width), Height (height), Depth (0),
  Format (format), data (0), palette (0), alpha (0)
{
}

csImageVolumeMaker::csImageVolumeMaker (iImage* source)
  : csImageBase(), manualName (false)
{
  Format = source->GetFormat();
  Width = source->GetWidth();
  Height = source->GetHeight();
  Depth = source->GetDepth();

  data = 0;
  alpha = 0;
  palette = 0;
  switch (Format & CS_IMGFMT_MASK)
  {
    case CS_IMGFMT_PALETTED8:
      {
	if (Format & CS_IMGFMT_ALPHA)
	{
	  alpha = new uint8[Width * Height * Depth];
	}
	data = new uint8[Width * Height * Depth];
	palette = new csRGBpixel[256];
      }
      break;
    case CS_IMGFMT_TRUECOLOR: 
      {
	data = new csRGBpixel[Width * Height * Depth];
      }
      break;
    default:
      break;
  }
  memcpy (data, source->GetImageData(), csImageTools::ComputeDataSize (source));
  if (alpha != 0)
    memcpy (alpha, source->GetAlpha(), Width * Height * Depth);
  if (palette != 0)
    memcpy (palette, source->GetPalette(), sizeof (csRGBpixel) * 256);
}

csImageVolumeMaker::~csImageVolumeMaker()
{
  switch (Format & CS_IMGFMT_MASK)
  {
    case CS_IMGFMT_PALETTED8: 
      delete[] ((uint8*)data);
      break;
    case CS_IMGFMT_TRUECOLOR: 
      delete[] ((csRGBpixel*)data);
      break;
    default:
      break;
  }
  delete[] palette;
  delete[] alpha;
}

void csImageVolumeMaker::AppendPending ()
{
  if (pendingImages.Length() == 0) return;
  int newDepth = Depth + pendingImages.Length();
  void* newData;
  uint8* newAlpha = 0;
  switch (Format & CS_IMGFMT_MASK)
  {
    case CS_IMGFMT_PALETTED8:
      {
	if (Format & CS_IMGFMT_ALPHA)
	{
	  newAlpha = new uint8[Width * Height * newDepth];
	  if (alpha != 0)
	    memcpy (newAlpha, alpha, Width * Height * Depth);
	}
	newData = new uint8[Width * Height * newDepth];
	if (data != 0)
	  memcpy (newData, data, Width * Height * Depth);
      }
      break;
    case CS_IMGFMT_TRUECOLOR: 
      {
	newData = new csRGBpixel[Width * Height * newDepth];
	if (data != 0)
	  memcpy (newData, data, Width * Height * Depth * sizeof (csRGBpixel));
      }
      break;
    default:
      return;
  }
  csRef<iImage> image;
  csRef<csImageMemory> newImage;
  size_t slicePix = Width * Height;
  size_t sliceSize = slicePix;
  if ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_TRUECOLOR)
    sliceSize *= sizeof (csRGBpixel);
  uint8* curSlice = (uint8*)newData + (Depth * sliceSize);
  uint8* curAlpha = 0;
  if (newAlpha != 0)
    curAlpha = newAlpha + (Depth * slicePix);
  for (size_t i = 0; i < pendingImages.Length(); i++)
  {
    csRef<iImage> image = pendingImages[i];
    if (image->HasKeyColor())
    {
      Format = Format | CS_IMGFMT_ALPHA;
      if (newAlpha == 0)
      {
	newAlpha = new uint8[Width * Height * newDepth];
	memset (newAlpha, 0xff, Width * Height * newDepth);
	curAlpha = newAlpha + (Depth * slicePix);
      }
      int kr, kg, kb;
      image->GetKeyColor (kr, kg, kb);
      csRGBpixel transp (kr, kg, kb);
      csRGBpixel fill (0, 0, 0);
      image = csImageManipulate::RenderKeycolorToAlpha (image,
	transp, fill);
    }
    if ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8)
    {
      if (palette == 0)
      {
	newImage.AttachNew (new csImageMemory (image, Format));
	palette = new csRGBpixel[256];
	memcpy (palette, newImage->GetPalette(), sizeof (csRGBpixel) * 256);
      }
      else
      {
	newImage.AttachNew (new csImageMemory (image, 
	  (Format & ~CS_IMGFMT_MASK) | CS_IMGFMT_TRUECOLOR));
	image = newImage;
	size_t pixNum = image->GetWidth() * image->GetHeight();
	csRGBpixel* px = new csRGBpixel[pixNum];
	memcpy (px, image->GetImageData(), pixNum * sizeof (csRGBpixel));
	newImage.AttachNew (new csImageMemory (image->GetWidth(), 
	  image->GetHeight(), Format));
	newImage->ConvertFromRGBA (px);
      }
      image = newImage;
    }
    else
    {
      if (image->GetFormat() != Format)
      {
	newImage.AttachNew (new csImageMemory (image, Format));
	image = newImage;
      }
    }
    image = csImageManipulate::Rescale (image, Width, Height);
    memcpy (curSlice, image->GetImageData(), sliceSize);
    curSlice += sliceSize;
    if (curAlpha != 0)
    {
      memcpy (curAlpha, image->GetAlpha(), slicePix);
      curAlpha += slicePix;
    }
  }
  switch (Format & CS_IMGFMT_MASK)
  {
    case CS_IMGFMT_PALETTED8: 
      delete[] ((uint8*)data);
      break;
    case CS_IMGFMT_TRUECOLOR: 
      delete[] ((csRGBpixel*)data);
      break;
  }
  data = newData;
  delete[] alpha; alpha = newAlpha;
  Depth = newDepth;
  pendingImages.DeleteAll();
}

const void* csImageVolumeMaker::GetImageData ()
{
  AppendPending();
  return data;
}

void csImageVolumeMaker::SetName (const char *iName)
{
  delete[] fName;
  fName = csStrNew (iName);
  manualName = true;
}

int csImageVolumeMaker::GetFormat () const
{
  return (Format != -1) ? Format : CS_IMGFMT_NONE;
}

const csRGBpixel* csImageVolumeMaker::GetPalette ()
{
  AppendPending();
  return palette;
}

const uint8* csImageVolumeMaker::GetAlpha ()
{
  AppendPending();
  return alpha;
}

void csImageVolumeMaker::AddImage (iImage* source)
{
  if (Width == -1) Width = source->GetWidth();
  if (Height == -1) Height = source->GetHeight();
  if (Format == -1) Format = source->GetFormat();
  if (!manualName)
  {
    if ((Depth + pendingImages.Length()) == 0)
    {
      delete[] fName;
      fName = csStrNew (source->GetName ());
    }
    else
    {
      char* newName = csStrNew (csString().Format ("%s:%s", fName, 
	source->GetName()));
      delete[] fName;
      fName = newName;
    }
  }
  pendingImages.Push (source);
}
