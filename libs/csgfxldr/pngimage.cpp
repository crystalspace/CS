/*
    PNG image file format support for CrystalSpace 3D library
    Copyright (C) 1998 by Jorrit Tyberghein
    Contributed by Andrew Zabolotny <bit@eltech.ru>

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

extern "C" {
  #define Byte z_Byte     /* Kludge to avoid conflicting typedef in zconf.h */
  #include <zlib.h>
  #undef Byte
  #include <png.h>
}

#include "sysdef.h"
#include "csgfxldr/pngimage.h"

//---------------------------------------------------------------------------

bool RegisterPNG ()
{
  static csPNGImageLoader loader;
  return csImageLoader::Register (&loader);
}

AlphaMapFile* csPNGImageLoader::LoadAlphaMap (UByte* buf, ULong size)
{
  (void) buf;
  (void) size;
  return NULL;
}

csImageFile* csPNGImageLoader::LoadImage (UByte* buf, ULong size)
{
  CHK (ImagePngFile* i = new ImagePngFile(buf, size));
  if (i && (i->get_status() & IFE_BadFormat)) 
  { CHK ( delete i );  i = NULL; }
  return i;    
}

//---------------------------------------------------------------------------

ImagePngFile::ImagePngFile (UByte * ptr, size_t filesize):csImageFile ()
{
  RGBPixel *Image;
  size_t rowbytes;
  png_infop end_info;
  png_infop info;

  if (!png_check_sig (ptr, filesize))
  {
    status = IFE_BadFormat;
    return;
  }
  r_data = ptr;
  r_size = filesize;

  png_structp png = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png)
  {
nomem:
    status = IFE_Corrupt;
    r_data = NULL;
    r_size = 0;
    return;
  }
  info = png_create_info_struct (png);
  if (!info)
  {
nomem2:
    png_destroy_read_struct (&png, (png_infopp) NULL, (png_infopp) NULL);
    goto nomem;
  }
  end_info = png_create_info_struct (png);
  if (!end_info)
    goto nomem2;

  if (setjmp (png->jmpbuf))
    goto nomem2;                        // If we get here, we had a problem reading the
                                        // file

  png_set_read_fn (png, (void *) this, PNG_read);

  png_read_info (png, info);

  // Get picture info
  png_uint_32 Width, Height;
  int bit_depth, color_type, interlace_type;

  png_get_IHDR (png, info, &Width, &Height, &bit_depth, &color_type,
    &interlace_type, NULL, NULL);
  // tell libpng to strip 16 bit/color files down to 8 bits/color
  png_set_strip_16 (png);
  // Expand paletted images to RGB triplets
  if (color_type & PNG_COLOR_MASK_PALETTE)
    png_set_expand (png);
  // Expand gray-scaled images to RGB triplets
  if (!(color_type & PNG_COLOR_MASK_COLOR))
    png_set_gray_to_rgb (png);

  // If there is no alpha information, fill with zeros
  if (!(color_type & PNG_COLOR_MASK_ALPHA))
    png_set_filler (png, 0, PNG_FILLER_AFTER);

  // Expand pictures with less than 8bpp to 8bpp
  if (bit_depth < 8)
    png_set_packing (png);

  // Update structure with the above settings
  png_read_update_info (png, info);

  // Allocate the memory to hold the image
  set_dimensions (Width, Height);
  Image = get_buffer ();
  if (!Image)
    goto nomem2;

  CHK (png_bytep *row_pointers = new png_bytep[Height]);
  if (setjmp (png->jmpbuf))             // Set a new exception handler
  {
    CHK (delete[] row_pointers);
    goto nomem2;
  }
  rowbytes = png_get_rowbytes (png, info);
  if (rowbytes != Width * sizeof (RGBPixel))
    goto nomem2;                        // Yuck! Something went wrong!

  for (png_uint_32 row = 0; row < Height; row++)
    row_pointers[row] = (png_bytep) & Image[row * Width];

  // Read image data
  png_read_image (png, row_pointers);

  // read rest of file, and get additional chunks in info_ptr
  png_read_end (png, info);

  // clean up after the read, and free any memory allocated
  png_destroy_read_struct (&png, &info, (png_infopp) NULL);

  CHK (delete[] row_pointers);
}

ImagePngFile::~ImagePngFile ()
{
}

void ImagePngFile::PNG_read (png_structp png, png_bytep data, png_size_t size)
{
  ImagePngFile *self = (ImagePngFile *) png->io_ptr;

  if (self->r_size < size)
    png_error (png, "Read Error");
  else
  {
    memcpy (data, self->r_data, size);
    self->r_size -= size;
    self->r_data += size;
  } /* endif */
}
