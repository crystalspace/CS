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
#include "csgfx/rgbpixel.h"
#include "csutil/databuf.h"

extern "C"
{
#define Byte z_Byte     /* Kludge to avoid conflicting typedef in zconf.h */
#include <zlib.h>
#undef Byte
#include <png.h>
}

#include "pngimage.h"

IMPLEMENT_IBASE (csPNGImageIO)
  IMPLEMENTS_INTERFACE (iImageIO)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csPNGImageIO);

EXPORT_CLASS_TABLE (cspngimg)
  EXPORT_CLASS (csPNGImageIO, "crystalspace.graphic.image.io.png", 
		"CrystalSpace PNG image format I/O plugin")
EXPORT_CLASS_TABLE_END

#define PNG_MIME "image/png"

struct datastore{
  unsigned char *data;
  long pos;
  long length;

  datastore () { data = NULL; pos = 0; length = 0; }
  ~datastore () { free (data); }
};

static void png_write (png_structp png, png_bytep data, png_size_t length)
{
  datastore *ds = (datastore *)png->io_ptr;
  if (ds->pos + (long)length > ds->length)
  {
    ds->data = (unsigned char*)realloc (ds->data, ds->pos + (long)length);
    if (!ds->data)
      png_error (png, "memory allocation error");
    else
      ds->length = ds->pos + length;
  }
  memcpy (ds->data + ds->pos, data, length);
  ds->pos += length;
}

void png_flush (png_structp)
{
}

iImageIO::FileFormatDescription formatlist[5] = 
{
  {PNG_MIME, "Gray", CS_IMAGEIO_LOAD},
  {PNG_MIME, "GrayAlpha", CS_IMAGEIO_LOAD},
  {PNG_MIME, "Palette", CS_IMAGEIO_LOAD},
  {PNG_MIME, "RGB", CS_IMAGEIO_LOAD|CS_IMAGEIO_SAVE},
  {PNG_MIME, "RGBA", CS_IMAGEIO_LOAD|CS_IMAGEIO_SAVE}
};

csPNGImageIO::csPNGImageIO (iBase *pParent)
{
  CONSTRUCT_IBASE (pParent);
  formats.Push (&formatlist[0]);
  formats.Push (&formatlist[1]);
  formats.Push (&formatlist[2]);
  formats.Push (&formatlist[3]);
  formats.Push (&formatlist[4]);
}

bool csPNGImageIO::Initialize (iSystem *)
{
  return true;
}

const csVector& csPNGImageIO::GetDescription ()
{
  return formats;
}

iImage *csPNGImageIO::Load (UByte* iBuffer, ULong iSize, int iFormat)
{
  ImagePngFile* i = new ImagePngFile (iFormat);
  if (i && !i->Load (iBuffer, iSize))
  {
    delete i;
    return NULL;
  }
  return i;    
}

void csPNGImageIO::SetDithering (bool)
{
}

iDataBuffer *csPNGImageIO::Save (iImage *Image, iImageIO::FileFormatDescription *)
{
  if (!Image)
    return NULL;

  datastore ds;

  png_structp png = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png)
  {
error1:
    return NULL;
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
  png_set_write_fn (png, (png_voidp)&ds, png_write, png_flush);

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

  /* make the iDataBuffer to return */
  csDataBuffer *db = new csDataBuffer (ds.pos);
  memcpy (db->GetData (), ds.data, ds.pos);

  /* that's it */
  return db;
}

iDataBuffer *csPNGImageIO::Save (iImage *Image, const char *mime)
{
  if (!strcasecmp (mime, PNG_MIME))
    return Save (Image, (iImageIO::FileFormatDescription *)NULL);
  return NULL;
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
