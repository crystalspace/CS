/*
    PNG image file format support for CrystalSpace 3D library
    Copyright (C) 1998,2000 by Andrew Zabolotny <bit@eltech.ru>

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

extern "C"
{
#define Byte z_Byte     /* Kludge to avoid conflicting typedef in zconf.h */
#include <zlib.h>
#undef Byte
#include <png.h>
}

#include "pngimage.h"

//---------------------------------------------------------------------------

csImageLoader *csCreateImageLoader_PNG ()
{
  return new csPNGImageLoader();
}

csImageFile* csPNGImageLoader::LoadImage (UByte* iBuffer, ULong iSize,
  int iFormat)
{
  ImagePngFile* i = new ImagePngFile (iFormat);
  if (i && !i->Load (iBuffer, iSize))
  {
    delete i;
    return NULL;
  }
  return i;    
}

//---------------------------------------------------------------------------

struct ImagePngRawData
{
  // The buffer to "read" from
  UByte *r_data;
  // The buffer size
  size_t r_size;
};

void ImagePngRead (png_structp png, png_bytep data, png_size_t size)
{
  ImagePngRawData *self = (ImagePngRawData *) png->io_ptr;

  if (self->r_size < size)
    png_error (png, "Read Error");
  else
  {
    memcpy (data, self->r_data, size);
    self->r_size -= size;
    self->r_data += size;
  } /* endif */
}

bool ImagePngFile::Load (UByte *iBuffer, ULong iSize)
{
  size_t rowbytes, exp_rowbytes;
  png_infop info;

  if (!png_check_sig (iBuffer, iSize))
    return false;

  png_structp png =
    png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png)
  {
nomem:
    FreeImage ();
    return false;
  }
  info = png_create_info_struct (png);
  if (!info)
  {
nomem2:
    png_destroy_read_struct (&png, (png_infopp) NULL, (png_infopp) NULL);
    goto nomem;
  }

  if (setjmp (png->jmpbuf))
    // If we get here, we had a problem reading the file
    goto nomem2;

  ImagePngRawData raw = { iBuffer, iSize };
  png_set_read_fn (png, &raw, ImagePngRead);

  png_read_info (png, info);

  // Get picture info
  png_uint_32 Width, Height;
  int bit_depth, color_type;

  png_get_IHDR (png, info, &Width, &Height, &bit_depth, &color_type,
    NULL, NULL, NULL);

  if (bit_depth > 8)
    // tell libpng to strip 16 bit/color files down to 8 bits/color
    png_set_strip_16 (png);
  else if (bit_depth < 8)
    // Expand pictures with less than 8bpp to 8bpp
    png_set_packing (png);

  volatile enum { imgRGB, imgPAL, imgPALALPHA } ImageType;
  switch (color_type)
  {
    case PNG_COLOR_TYPE_GRAY:
    case PNG_COLOR_TYPE_GRAY_ALPHA:
    case PNG_COLOR_TYPE_PALETTE:
      ImageType = imgPAL;
      // If we need alpha, take it. If we don't, strip it.
      if (Format & CS_IMGFMT_ALPHA)
      {
        if (color_type & PNG_COLOR_MASK_ALPHA)
          ImageType = imgPALALPHA;
        else
          Format &= ~CS_IMGFMT_ALPHA;
      }
      else if (color_type & PNG_COLOR_MASK_ALPHA)
        png_set_strip_alpha (png);
      break;
    case PNG_COLOR_TYPE_RGB:
    case PNG_COLOR_TYPE_RGB_ALPHA:
      ImageType = imgRGB;
      // If there is no alpha information, fill with 0xff
      if (!(color_type & PNG_COLOR_MASK_ALPHA))
      {
        // Expand paletted or RGB images with transparency to full alpha
	// channels so the data will be available as RGBA quartets.
        if (png_get_valid (png, info, PNG_INFO_tRNS))
          png_set_expand (png);
        else
        {
          png_set_filler (png, 0xff, PNG_FILLER_AFTER);
          Format &= ~CS_IMGFMT_ALPHA;
        }
      }
      break;
    default:
      goto nomem2;
  }

  // Update structure with the above settings
  png_read_update_info (png, info);

  // Allocate the memory to hold the image
  set_dimensions (Width, Height);
  if (ImageType == imgRGB)
    exp_rowbytes = Width * sizeof (csRGBpixel);
  else if (ImageType == imgPALALPHA)
    exp_rowbytes = Width * 2;
  else
    exp_rowbytes = Width;

  rowbytes = png_get_rowbytes (png, info);
  if (rowbytes != exp_rowbytes)
    goto nomem2;                        // Yuck! Something went wrong!

  png_bytep * const row_pointers = new png_bytep[Height];

  if (setjmp (png->jmpbuf))             // Set a new exception handler
  {
    delete [] row_pointers;
    goto nomem2;
  }

  void *NewImage = NULL;
  if (ImageType == imgRGB)
    NewImage = new csRGBpixel [Width * Height];
  else if (ImageType == imgPALALPHA)
    NewImage = new UByte [Width * Height * 2];
  else
    NewImage = new UByte [Width * Height];
  if (!NewImage)
    goto nomem2;

  for (png_uint_32 row = 0; row < Height; row++)
    row_pointers [row] = ((png_bytep)NewImage) + row * rowbytes;

  // Read image data
  png_read_image (png, row_pointers);

  // read rest of file, and get additional chunks in info_ptr
  png_read_end (png, (png_infop)NULL);

  if (ImageType == imgRGB)
    convert_rgba ((csRGBpixel *)NewImage);
  else if (ImageType == imgPAL)
  {
    csRGBcolor graypal [256];
    csRGBcolor *palette = NULL;
    int colors;
    if (!png_get_PLTE (png, info, (png_colorp *)&palette, &colors))
    {
      // This is a grayscale image, build a grayscale palette
      palette = graypal;
      int entries = (1 << bit_depth) - 1;
      colors = entries + 1;
      for (int i = 0; i <= entries; i++)
        palette [i].red = palette [i].green = palette [i].blue =
          (i * 255) / entries;
    }
    convert_pal8 ((UByte *)NewImage, palette, colors);
  }
  else // grayscale + alpha
  {
    // This is a grayscale image, build a grayscale palette
    csRGBpixel *palette = new csRGBpixel [256];
    int i, entries = (1 << bit_depth) - 1;
    for (i = 0; i <= entries; i++)
      palette [i].red = palette [i].green = palette [i].blue =
        (i * 255) / entries;

    int pixels = Width * Height;
    UByte *image = new UByte [pixels];
    Alpha = new UByte [pixels];
    UByte *src = (UByte *)NewImage;
    for (i = 0; i < pixels; i++)
    {
      image [i] = *src++;
      Alpha [i] = *src++;
    }
    delete [] (UByte *)NewImage;
    convert_pal8 (image, palette);
  }

  // clean up after the read, and free any memory allocated
  png_destroy_read_struct (&png, &info, (png_infopp) NULL);

  // Free the row pointers array that is not needed anymore
  delete [] row_pointers;

  // Check if the alpha channel is valid
  CheckAlpha ();

  return true;
}
