/*
    PNG image file format support for CrystalSpace 3D library
    Copyright (C) 2000 by Andrew Zabolotny <bit@eltech.ru>

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

#include "csgfx/pngsave.h"
#include "csgfx/rgbpixel.h"

static void png_write (png_structp png, png_bytep data, png_size_t length)
{
  png_uint_32 check = fwrite (data, 1, length, (FILE *)(png->io_ptr));
  if (check != length)
    png_error (png, "Write Error");
}

void png_flush (png_structp png)
{
  if (png->io_ptr)
    fflush ((FILE *)(png->io_ptr));
}

bool csSavePNG (const char *FileName, iImage *Image)
{
  if (!Image)
    return false;

  /* open the file */
  FILE *fp = fopen (FileName, "wb");
  if (fp == NULL)
    return false;

  png_structp png = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png)
  {
error1:
    fclose (fp);
    return false;
  }

  /* Allocate/initialize the image information data. */
  png_infop info = png_create_info_struct (png);
  if (info == NULL)
  {
error2:
    png_destroy_write_struct (&png, (png_infopp)NULL);
    goto error1;
  }

  /* Catch processing errors */
  if (setjmp(png->jmpbuf))
    /* If we get here, we had a problem reading the file */
    goto error2;

  /* Set up the output function. We could use standard file output
   * routines but for some (strange) reason if we write the file inside
   * the shared PNG library this causes problems at least on OS/2...
   */
#if 1
  png_set_write_fn (png, (png_voidp)fp, png_write, png_flush);
#else
  png_init_io (png, fp);
#endif

  /* Set the image information here.  Width and height are up to 2^31,
   * bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
   * the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
   * PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
   * or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
   * PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
   * currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
   */
  int format = Image->GetFormat ();
  int colortype, rowlen;
  int width = Image->GetWidth (), height = Image->GetHeight ();
  switch (format & CS_IMGFMT_MASK)
  {
    case CS_IMGFMT_NONE:
      // plain alphamaps not supported
      goto error2;
    case CS_IMGFMT_PALETTED8:
      colortype = PNG_COLOR_TYPE_PALETTE;
      rowlen = Image->GetWidth ();
      break;
    case CS_IMGFMT_TRUECOLOR:
      colortype = (format & CS_IMGFMT_ALPHA) ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB;
      rowlen = Image->GetWidth () * sizeof (csRGBpixel);
      break;
    default:
      // unknown format
      goto error2;
  } /* endswitch */
  png_set_IHDR (png, info, width, height, 8, colortype,
    PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  /* set the palette if there is one. */
  if (colortype & PNG_COLOR_MASK_PALETTE)
  {
    csRGBpixel *pal = Image->GetPalette ();
    png_colorp palette = (png_colorp)malloc (256 * sizeof (png_color));
    for (int i = 0; i < 256; i++)
    {
      palette [i].red   = pal [i].red;
      palette [i].green = pal [i].green;
      palette [i].blue  = pal [i].blue;
    } /* endfor */
    png_set_PLTE (png, info, palette, 256);
  } /* endif */

  /* otherwise, if we are dealing with a color image then */
  png_color_8 sig_bit;
  memset (&sig_bit, 0, sizeof (sig_bit));
  sig_bit.red = 8;
  sig_bit.green = 8;
  sig_bit.blue = 8;
  /* if the image has an alpha channel then */
  if (format & CS_IMGFMT_ALPHA)
    sig_bit.alpha = 8;
  png_set_sBIT (png, info, &sig_bit);

  /* Write the file header information. */
  png_write_info (png, info);

  /* Get rid of filler (OR ALPHA) bytes, pack XRGB/RGBX/ARGB/RGBA into
   * RGB (4 channels -> 3 channels). The second parameter is not used.
   */
  if (((format & CS_IMGFMT_MASK) == CS_IMGFMT_TRUECOLOR)
   && !(format & CS_IMGFMT_ALPHA))
    png_set_filler (png, 0xff, PNG_FILLER_AFTER);

  /* The easiest way to write the image (you may have a different memory
   * layout, however, so choose what fits your needs best).  You need to
   * use the first method if you aren't handling interlacing yourself.
   */
  png_bytep *row_pointers = new png_bytep [height];
  UByte *ImageData = (UByte *)Image->GetImageData ();
  for (int i = 0; i < height; i++)
    row_pointers [i] = ImageData + i * rowlen;

  /* One of the following output methods is REQUIRED */
  png_write_image (png, row_pointers);

  /* It is REQUIRED to call this to finish writing the rest of the file */
  png_write_end (png, info);

  /* if you malloced the palette, free it here */
  if (info->palette)
    free (info->palette);

  /* clean up after the write, and free any memory allocated */
  png_destroy_write_struct (&png, (png_infopp)NULL);

  /* Free the row pointers */
  delete [] row_pointers;

  /* close the file */
  fclose (fp);

  /* that's it */
  return true;
}
