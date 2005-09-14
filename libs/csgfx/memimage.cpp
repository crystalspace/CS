/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Written by Samuel Humphreys

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
#include "csgfx/imagemanipulate.h"
#include "csgfx/quantize.h"
#include "csgfx/memimage.h"
#include "csgfx/rgbpixel.h"
#include "csutil/debug.h"
#include "csutil/util.h"

CS_LEAKGUARD_IMPLEMENT (csImageMemory);

void csImageMemory::ConstructCommon()
{
  DG_ADDI (this, 0);
  DG_TYPE (this, "csImageMemory");

  Image = 0;
  Palette = 0;
  Alpha = 0;
  imageType = csimg2D;
  destroy_image = true;
  has_keycolour = false;
  keycolour.Set (0, 0, 0, 255);
}

void csImageMemory::ConstructWHDF (int width, int height, int depth, int format)
{
  ConstructCommon();
  Width = width;
  Height = height;
  Depth = depth;
  Format = format;
}

void csImageMemory::ConstructSource (iImage* source)
{
  ConstructWHDF (source->GetWidth(), source->GetHeight(), source->GetDepth(),
    source->GetFormat());

  AllocImage();
  memcpy (Image, source->GetImageData(), csImageTools::ComputeDataSize (this));
  if (Alpha)
    memcpy (Alpha, source->GetAlpha(), Width * Height);
  if (Palette)
    memcpy (Palette, source->GetPalette(), sizeof (csRGBpixel) * 256);
}

void csImageMemory::ConstructBuffers (int width, int height, void* buffer,
    bool destroy, int format, csRGBpixel *palette)
{
  ConstructCommon();
  Width = width;
  Height = height;
  Depth = 1;
  Format = format;
  Image = buffer;
  Palette = palette;
  destroy_image = destroy;
}

csImageMemory::csImageMemory (int width, int height, int format) :
  scfImplementationType(this)
{
  ConstructWHDF (width, height, 1, format);
}

csImageMemory::csImageMemory (int width, int height, int depth, int format) :
  scfImplementationType(this)
{
  ConstructWHDF (width, height, depth, format);
  if (depth > 1) imageType = csimg3D;
}

csImageMemory::csImageMemory (int width, int height, void* buffer,
                              bool destroy, int format, csRGBpixel *palette) :
  scfImplementationType(this)
{
  ConstructBuffers (width, height, buffer, destroy, format, palette);
}

csImageMemory::csImageMemory (int width, int height, const void* buffer,
                              int format, const csRGBpixel *palette) :
  scfImplementationType(this)
{
  ConstructWHDF (width, height, 1, format);
  AllocImage();
  memcpy (Image, buffer, csImageTools::ComputeDataSize (this));
  if (Palette)
    memcpy (Palette, palette, sizeof (csRGBpixel) * 256);
}

csImageMemory::csImageMemory (iImage* source) :
  scfImplementationType(this)
{
  ConstructSource (source);
}

csImageMemory::csImageMemory (iImage* source, int newFormat) :
  scfImplementationType(this)
{
  ConstructSource (source);
  SetFormat (newFormat);
}

csImageMemory::csImageMemory (int iFormat) :
  scfImplementationType(this)
{
  ConstructWHDF (0, 0, 1, iFormat);
}

void csImageMemory::AllocImage()
{
  switch (Format & CS_IMGFMT_MASK)
  {
    case CS_IMGFMT_PALETTED8:
      Image = (void*) new uint8[Width*Height*Depth];
      if (Format & CS_IMGFMT_ALPHA)
      {
        Alpha =  new uint8[Width*Height*Depth];
      }
      Palette = new csRGBpixel[256];
      break;
    case CS_IMGFMT_TRUECOLOR:
      Image = (void*) new csRGBpixel[Width*Height*Depth];
      break;
  }
  destroy_image = true;
}

void csImageMemory::EnsureImage()
{
  if ((Image == 0) && (Palette == 0) && (Alpha == 0))
    AllocImage();
}

void csImageMemory::FreeImage ()
{
  if (!destroy_image)
  {
    // Clear the 'Image' and 'Palette' pointers so that they won't be
    // deallocated later.
    Image = 0;
    Palette = 0;
  }
  /*csImageFile::FreeImage();*/
  switch (Format & CS_IMGFMT_MASK)
  {
    case CS_IMGFMT_TRUECOLOR:
      delete [] (csRGBpixel *)Image;
      break;
    case CS_IMGFMT_PALETTED8:
      delete [] (uint8 *)Image;
      break;
  }
  delete [] Palette;
  delete [] Alpha;
  Image = 0; Palette = 0; Alpha = 0;
}

void csImageMemory::SetDimensions (int newWidth, int newHeight)
{
  SetDimensions (newWidth, newHeight, 1);
}

void csImageMemory::SetDimensions (int newWidth, int newHeight, int newDepth)
{
  FreeImage();
  Width = newWidth; Height = newHeight; Depth = newDepth;
}

csImageMemory::~csImageMemory ()
{
  if (!destroy_image)
  {
    // Before calling FreeImage() we first
    // clear the 'Image' and 'Palette' pointers so that
    // it will not try to deallocate.
    Image = 0;
    Palette = 0;
  }
  FreeImage();
  DG_REM (this);
}

void* csImageMemory::GetImagePtr ()
{
  EnsureImage();
  return Image;
}
csRGBpixel* csImageMemory::GetPalettePtr ()
{
  EnsureImage();
  return Palette;
}
uint8* csImageMemory::GetAlphaPtr ()
{
  EnsureImage();
  return Alpha;
}

void csImageMemory::Clear (const csRGBpixel &colour)
{
  if ((Format & CS_IMGFMT_MASK) != CS_IMGFMT_TRUECOLOR) return;

  EnsureImage ();

  uint32 *src = (uint32*) &colour;
  uint32 *dst = (uint32*)Image;

  int i;
  for (i = 0; i < Width*Height*Depth; i++, dst++)
    *dst = *src;
}

void csImageMemory::CheckAlpha ()
{
  if (!(Format & CS_IMGFMT_ALPHA))
    return;

  int i, pixels = Width * Height * Depth;
  bool noalpha = true;
  switch (Format & CS_IMGFMT_MASK)
  {
    case CS_IMGFMT_NONE:
    case CS_IMGFMT_PALETTED8:
      if (Alpha)
        for (i = 0; i < pixels; i++)
          if (Alpha [i] != 255)
          {
            noalpha = false;
            break;
          }
      break;
    case CS_IMGFMT_TRUECOLOR:
      for (i = 0; i < pixels; i++)
        if (((csRGBpixel *)Image) [i].alpha != 255)
        {
          noalpha = false;
          break;
        }
      break;
  }
  if (noalpha)
  {
    if (Alpha)
    {
      delete [] Alpha; Alpha = 0;
    }
    Format &= ~CS_IMGFMT_ALPHA;
  }
}

void csImageMemory::ConvertFromRGBA (csRGBpixel *iImage)
{
  int pixels = Width * Height * Depth;

  if ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_ANY)
    Format = (Format & ~CS_IMGFMT_MASK) | CS_IMGFMT_TRUECOLOR;

  switch (Format & CS_IMGFMT_MASK)
  {
    case CS_IMGFMT_NONE:
    case CS_IMGFMT_PALETTED8:
      if (Format & CS_IMGFMT_ALPHA)
      {
        if (!Alpha)
          Alpha = new uint8 [pixels];
        int i;
        for (i = 0; i < pixels; i++)
          Alpha [i] = iImage [i].alpha;
      }
      if ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8)
      {
        // The most complex case: reduce an RGB image to a paletted image.
        int maxcolors = 256;
        csColorQuantizer quant;
        quant.Begin ();

        quant.Count (iImage, pixels);
        quant.Palette (Palette, maxcolors);
        quant.RemapDither (iImage, pixels, Width, Palette, maxcolors,
          (uint8 *&)Image, has_keycolour ? &keycolour : 0);

        quant.End ();
      }
      delete [] iImage;
      break;
    case CS_IMGFMT_TRUECOLOR:
      if (Image != iImage)
        FreeImage ();
      Image = iImage;
      break;
  }
}

void csImageMemory::ConvertFromPal8 (uint8 *iImage, uint8* alpha, 
				     csRGBpixel *iPalette, int nPalColors)
{
  int pixels = Width * Height * Depth;

  // ensure the palette has at least 256 entries.
  if (nPalColors < 256)
  {
    csRGBpixel *newPal = new csRGBpixel [256];
    memcpy ((void*)newPal, (const void*)iPalette,
      nPalColors * sizeof(csRGBpixel));
    delete[] iPalette;
    iPalette = newPal;
  }


  if ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_ANY)
    Format = (Format & ~CS_IMGFMT_MASK) | CS_IMGFMT_PALETTED8;

  switch (Format & CS_IMGFMT_MASK)
  {
    case CS_IMGFMT_NONE:
      delete[] iImage;
      delete[] iPalette;
      delete[] Alpha;
      break;
    case CS_IMGFMT_PALETTED8:
      Image = iImage;
      Palette = iPalette;
      Alpha = alpha;
      break;
    case CS_IMGFMT_TRUECOLOR:
    {
      uint8 *in = iImage;
      csRGBpixel *out;
      if (Image)
        out = (csRGBpixel *)Image;
      else
        Image = out = new csRGBpixel [pixels];

      if ((Format & CS_IMGFMT_ALPHA) && alpha)
      {
        uint8 *a = alpha;
        while (pixels--)
        {
          *out = iPalette [*in++];
          out->alpha = *a++;
          out++;
        }
      }
      else
        while (pixels--)
          *out++ = iPalette [*in++];
      delete[] alpha;
      delete[] iImage;
      delete[] iPalette;
      break;
    }
  }
  if ((Format & CS_IMGFMT_ALPHA)
   && ((Format & CS_IMGFMT_MASK) != CS_IMGFMT_TRUECOLOR)
   && !Alpha)
    Format &= ~CS_IMGFMT_ALPHA;
}

void csImageMemory::ConvertFromPal8 (uint8 *iImage, uint8* alpha, 
                                     const csRGBcolor *iPalette,
                                     int nPalColors)
{
  csRGBpixel *newpal = new csRGBpixel [256];
  int i;
  for (i = 0; i < nPalColors; i++) // Default csRGBpixel constructor ensures
    newpal [i] = iPalette [i];         // palette past nPalColors is sane.
  ConvertFromPal8 (iImage, alpha, newpal);
}

void csImageMemory::SetFormat (int iFormat)
{
  int pixels = Width * Height * Depth;
  int oldformat = Format;
  void *oldimage = Image;
  uint8* oldalpha = Alpha;
  Alpha = 0;
  Image = 0;
  Format = iFormat;

  if ((oldformat & CS_IMGFMT_MASK) == CS_IMGFMT_TRUECOLOR)
    ConvertFromRGBA ((csRGBpixel *)oldimage);
  else if ((oldformat & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8)
  {
    uint8* Alpha = 0;
    if (iFormat & CS_IMGFMT_ALPHA)
    {
      if (oldalpha)
	Alpha = oldalpha; 
      else
      {
	Alpha = new uint8[Width * Height];
	memset (Alpha, 0xff, Width * Height);
      }
    }
    else
    {
      delete[] oldalpha;
    }
    csRGBpixel* oldPalette = Palette;
    Palette = 0;
    ConvertFromPal8 ((uint8 *)oldimage, Alpha, oldPalette);
  }
  else if ((oldformat & CS_IMGFMT_MASK) == CS_IMGFMT_NONE)
  {
    if ((Format & CS_IMGFMT_ALPHA) && !Alpha)
      Alpha = new uint8 [pixels];
    if ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8)
      Image = new uint8 [pixels];
    else if ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_TRUECOLOR)
      Image = new csRGBpixel [pixels];
  }
}

void csImageMemory::SetKeyColor (int r, int g, int b)
{
  has_keycolour = true;
  keycolour.Set (r, g, b);
}

void csImageMemory::ClearKeyColor ()
{
  has_keycolour = false;
}

void csImageMemory::ApplyKeyColor ()
{
  if (has_keycolour
    && ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8))
  {
    if (Image == 0) return;

    uint8* imageData = (uint8*)Image;
    uint8* imagePtr;
    const int pixcount = Width * Height * Depth;
    int i;

    // Find out what colors in the palette are actually used
    bool usedEntries[256];
    memset (usedEntries, 0, sizeof (usedEntries));
    int remainingEntries = 256;
    imagePtr = imageData;
    for (i = 0; (i < pixcount) && (remainingEntries > 0); i++)
    {
      if (!usedEntries[*imagePtr])
      {
        usedEntries[*imagePtr] = true;
        remainingEntries--;
      }
      imagePtr++;
    }

    // Find original keycolor index
    int keyIndex = -1;
    for (i = 0; i < 256; i++)
    {
      if (Palette[i].eq (keycolour))
      {
        keyIndex = i;
        break;
      }
    }
    if (keyIndex <= 0)
      return; // We're finished already

    // First, find a palette entry that can take the old color at index 0
    int newIndex = -1;
    for (i = 0; i < 256; i++)
    {
      if (!usedEntries[i])
      {
        newIndex = i;
        break;
      }
    }
    if (newIndex == -1)
    {
      newIndex = csImageTools::ClosestPaletteIndex (Palette + 1, Palette[0]);
    }
    else
    {
      Palette[newIndex] = Palette[0];
    }
    Palette[0] = keycolour;

    imagePtr = imageData;
    for (i = 0; i < pixcount; i++)
    {
      if (*imagePtr == 0)
        *imagePtr = newIndex;
      else if (*imagePtr == keyIndex)
        *imagePtr = 0;
      imagePtr++;
    }
  }
}

bool csImageMemory::Copy (iImage* simage, int x, int y,
                          int width, int height )
{
  if (width<0) return false;
  if (height<0) return false;
  if (x+width>GetWidth()) return false;
  if (y+height>GetHeight()) return false;

  if (simage->GetWidth()<width) return false;
  if (simage->GetHeight()<height) return false;

  int i;
  if (Alpha)
  {
    for (i=0; i<height; i++)
      memcpy (Alpha + (i+y)*Width + x, simage->GetAlpha() + i*width, width);
  }

  if (Image)
  {
    switch (Format & CS_IMGFMT_MASK)
    {
      case CS_IMGFMT_NONE:
        break;
      case CS_IMGFMT_PALETTED8:
        for ( i=0; i<height; i++ )
          memcpy ((uint8*)Image + (i+y)*Width + x,
                (uint8*)simage->GetImageData() + i*width,  width);
        break;
      case CS_IMGFMT_TRUECOLOR:
        for ( i=0; i<height; i++ )
          memcpy ((csRGBpixel*)Image + (i+y)*Width + x,
                (csRGBpixel*)simage->GetImageData() + i*width,
                width * sizeof (csRGBpixel));
        break;
    }
  }

  return true;
}

bool csImageMemory::CopyScale (iImage* simage, int x, int y,
                               int width, int height )
{
  if (width<0) return false;
  if (height<0) return false;

  csRef<iImage> psimage = csImageManipulate::Rescale (simage,
    width, height);
  Copy (psimage,x,y,width,height);

  return true;
}

bool csImageMemory::CopyTile (iImage* simage, int x, int y,
                              int width, int height )
{
  if (width<0) return false;
  if (height<0) return false;

  int sWidth = simage->GetWidth();
  int sHeight = simage->GetHeight();

  int wfactor = int ((float)width/(float)sWidth);
  int hfactor = int ((float)height/(float)sHeight);

  if (wfactor<1) wfactor=1;
  if (hfactor<1) hfactor=1;

  csRef<csImageMemory> psimage;
  psimage.AttachNew (new csImageMemory (wfactor*sWidth, hfactor*sHeight,
    Format));

  for (int i=0;i<wfactor;i++)
    for(int j=0;j<hfactor;j++)
      psimage->Copy (simage, i*sWidth, j*sHeight, sWidth, sHeight);

  Copy (csImageManipulate::Rescale (psimage, width, height), x, y,
    width, height);

  return true;
}
