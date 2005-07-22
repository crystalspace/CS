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
#include "csgfx/imagetools.h"
#include "csgfx/rgbpixel.h"
#include "csgfx/packrgb.h"
#include "csutil/csendian.h"
#include "csutil/csstring.h"
#include "csutil/databuf.h"
#include "csplugincommon/imageloader/optionsparser.h"
#include <math.h>

extern "C"
{
#define Byte z_Byte     /* Kludge to avoid conflicting typedef in zconf.h */
#include <zlib.h>
#undef Byte
#include <png.h>
}

#include "pngimage.h"

CS_LEAKGUARD_IMPLEMENT (ImagePngFile);

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csPNGImageIO)
  SCF_IMPLEMENTS_INTERFACE (iImageIO)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csPNGImageIO::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csPNGImageIO)


#define PNG_MIME "image/png"

struct datastore{
  unsigned char *data;
  size_t pos;
  size_t length;

  datastore () { data = 0; pos = 0; length = 0; }
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

static iImageIO::FileFormatDescription formatlist[5] =
{
  {PNG_MIME, "Gray", CS_IMAGEIO_LOAD},
  {PNG_MIME, "GrayAlpha", CS_IMAGEIO_LOAD},
  {PNG_MIME, "Palette", CS_IMAGEIO_LOAD|CS_IMAGEIO_SAVE},
  {PNG_MIME, "RGB", CS_IMAGEIO_LOAD|CS_IMAGEIO_SAVE},
  {PNG_MIME, "RGBA", CS_IMAGEIO_LOAD|CS_IMAGEIO_SAVE}
};

csPNGImageIO::csPNGImageIO (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  formats.Push (&formatlist[0]);
  formats.Push (&formatlist[1]);
  formats.Push (&formatlist[2]);
  formats.Push (&formatlist[3]);
  formats.Push (&formatlist[4]);
}

csPNGImageIO::~csPNGImageIO()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

const csImageIOFileFormatDescriptions& csPNGImageIO::GetDescription ()
{
  return formats;
}

csPtr<iImage> csPNGImageIO::Load (iDataBuffer* buf, int iFormat)
{
  ImagePngFile* i = new ImagePngFile (object_reg, iFormat);
  if (i && !i->Load (buf))
  {
    delete i;
    return 0;
  }
  return csPtr<iImage> (i);
}

void csPNGImageIO::SetDithering (bool)
{
}

csPtr<iDataBuffer> csPNGImageIO::Save (iImage *Image, iImageIO::FileFormatDescription *,
  const char* extraoptions)
{
  if (!Image)
    return 0;

  int compress = 6;
  bool interlace = false;
  /*
     parse output options.
     options are a comma-separated list and can be either
     'option' or 'option=value'.

     supported options:
       compress=#   image compression, 0..100 higher values give smaller files,
		    but take longer to encode.
       progressive  interlaced output.
   
     examples:
       compress=50
       progressive,compress=30
   */
  csImageLoaderOptionsParser optparser (extraoptions);
  optparser.GetBool ("progressive", interlace);
  if (optparser.GetInt ("compress", compress))
  {
    compress /= 10;
    if (compress < 0) compress = 0;
    if (compress > 9) compress = 9;
  }

  datastore ds;

  png_structp png = png_create_write_struct (PNG_LIBPNG_VER_STRING, 0, 0, 0);

  if (!png)
  {
error1:
    return 0;
  }

  png_set_compression_level (png, compress);

  /* Allocate/initialize the image information data. */
  png_infop info = png_create_info_struct (png);
  if (info == 0)
  {
    png_destroy_write_struct (&png, (png_infopp)0);
error2:
    goto error1;
  }

  /* Catch processing errors */
  if (setjmp(png->jmpbuf))
  {
    /* If we get here, we had a problem reading the file */
    png_destroy_write_struct (&png, &info);
    goto error2;
  }

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
      rowlen = Image->GetWidth () * 4;
      break;
    default:
      // unknown format
      goto error2;
  } /* endswitch */
  png_set_IHDR (png, info, width, height, 8, colortype,
    interlace?PNG_INTERLACE_ADAM7:PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, 
    PNG_FILTER_TYPE_BASE);

  png_colorp palette = 0;
  /* set the palette if there is one. */
  if (colortype & PNG_COLOR_MASK_PALETTE)
  {
    const csRGBpixel *pal = Image->GetPalette ();
    
    palette = (png_colorp)malloc (256 * sizeof (png_color));
    int i;
    for (i = 0; i < 256; i++)
    {
      palette [i].red   = pal [i].red;
      palette [i].green = pal [i].green;
      palette [i].blue  = pal [i].blue;
    } /* endfor */
    int max_color = 0;
    // seek maximum color index used in the image
    int n = Image->GetWidth() * Image->GetHeight();
    uint8 *imagedata = (uint8*)Image->GetImageData();
    while (n > 0)
    {
      max_color = MAX(max_color, *imagedata);
      imagedata++;
      n--;
    }
    png_set_PLTE (png, info, palette, max_color+1);

    if (Image->HasKeyColor())
    {
      // Get the keycolor palette index and write the appropriate
      // tRNS chunk.
      int key_r, key_g, key_b;
      Image->GetKeyColor (key_r, key_g, key_b);
      csRGBpixel key (key_r, key_g, key_b);
      int key_index = csImageTools::ClosestPaletteIndex (
	Image->GetPalette(), key);
      png_bytep trans = new png_byte[key_index + 1];
      memset (trans, 0xff, key_index);
      trans[key_index] = 0;
      png_set_tRNS (png, info, trans, key_index+1, 0);
      delete[] trans;
    }
  }
  else
  {
    //Write tRNS chunk with keycolor.
    if (Image->HasKeyColor())
    {
      int key_r, key_g, key_b;
      Image->GetKeyColor (key_r, key_g, key_b);
      png_color_16 trans;
      memset (&trans, 0, sizeof(trans));
      trans.red = csBigEndianShort (key_r << 8);
      trans.green = csBigEndianShort (key_g << 8);
      trans.blue = csBigEndianShort (key_b << 8);
      png_set_tRNS (png, info, 0, 0, &trans);
    }
  }

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

  /* Writing the image. Interlacing is handled automagically. */
  png_bytep *row_pointers = new png_bytep [height];
  png_bytep packedData;
  if ((format & CS_IMGFMT_MASK) == CS_IMGFMT_TRUECOLOR)
  {
    packedData = (png_bytep)csPackRGBA::PackRGBpixelToRGBA 
      ((csRGBpixel *)Image->GetImageData (), width * height);
  }
  else
  {
    packedData = (png_bytep)Image->GetImageData ();
  }
  int i;
  for (i = 0; i < height; i++)
    row_pointers [i] = packedData + i * rowlen;
  png_write_image (png, row_pointers);
  if ((format & CS_IMGFMT_MASK) == CS_IMGFMT_TRUECOLOR)
  {
    csPackRGBA::DiscardPackedRGBA (packedData);
  }

  /* It is REQUIRED to call this to finish writing the rest of the file */
  png_write_end (png, info);

  /* clean up after the write, and free any memory allocated */
  png_destroy_write_struct (&png, &info);
  if (palette)
    free(palette);

  /* Free the row pointers */
  delete [] row_pointers;

  /* make the iDataBuffer to return */
  csDataBuffer *db = new csDataBuffer (ds.pos);
  memcpy (db->GetData (), ds.data, ds.pos);

  /* that's it */
  return csPtr<iDataBuffer> (db);
}

csPtr<iDataBuffer> csPNGImageIO::Save (iImage *Image, const char *mime,
  const char* extraoptions)
{
  // The only supported mime format is 'image/png'.
  // Default to that if no format is supplied.
  if (!mime || !strcasecmp (mime, PNG_MIME))
    return Save (Image, (iImageIO::FileFormatDescription *)0,
      extraoptions);
  return 0;
}

//---------------------------------------------------------------------------

void ImagePngFile::PngLoader::ImagePngRead (png_structp png, png_bytep data, 
					    png_size_t size)
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

ImagePngFile::PngLoader::~PngLoader()
{
  if (png != 0)
    png_destroy_read_struct (&png, &info, (png_infopp) 0);
}

bool ImagePngFile::PngLoader::InitOk ()
{
  const png_bytep iBuffer = dataSource->GetUint8();
  const size_t iSize = dataSource->GetSize();

  if (!png_check_sig (iBuffer, (int)iSize))
    return false;
  png = png_create_read_struct (PNG_LIBPNG_VER_STRING, 0, 0, 0);
  if (!png)
  {
    return false;
  }

  info = png_create_info_struct (png);
  if (!info)
  {
    png_destroy_read_struct (&png, (png_infopp) 0, (png_infopp) 0);
    png = 0;
    return false;
  }

  if (setjmp (png->jmpbuf))
  {
nomem2:
    // If we get here, we had a problem reading the file
    png_destroy_read_struct (&png, &info, (png_infopp) 0);
    png = 0; info = 0;
    return false;
  }

  raw.r_data = iBuffer;
  raw.r_size = iSize;
  png_set_read_fn (png, &raw, ImagePngRead);

  png_read_info (png, info);

  // Get picture info
  png_uint_32 W, H;

  png_get_IHDR (png, info, &W, &H, &bit_depth, &color_type,
    0, 0, 0);
  Width = W;
  Height = H;

  int NewFormat;

  switch (color_type)
  {
    case PNG_COLOR_TYPE_GRAY:
    case PNG_COLOR_TYPE_GRAY_ALPHA:
      dataType = rdtIndexed;
      NewFormat = (Format & ~CS_IMGFMT_MASK) | CS_IMGFMT_PALETTED8;
      if ((Format & CS_IMGFMT_ALPHA) && (color_type & PNG_COLOR_MASK_ALPHA))
      {
	ImageType = imgGrayAlpha;
      }
      else
      {
        // Check if we have keycolor transparency.
	ImageType = imgPAL;
	png_set_strip_alpha (png);
	if (png_get_valid (png, info, PNG_INFO_tRNS))
	{
	  png_color_16p trans_values;
	  png_get_tRNS (png, info, 0, 0, &trans_values);
	  hasKeycolor = true;
	  keycolor.red = csConvertEndian (trans_values->gray) & 0xff;
	  keycolor.green = csConvertEndian (trans_values->gray) & 0xff;
	  keycolor.blue = csConvertEndian (trans_values->gray) & 0xff;
	}
      }
      break;
    case PNG_COLOR_TYPE_PALETTE:
      NewFormat = (Format & ~CS_IMGFMT_MASK) | CS_IMGFMT_PALETTED8;
      ImageType = imgPAL;
      dataType = rdtIndexed;
      // If we need alpha, take it. If we don't, strip it.
      if (Format & CS_IMGFMT_ALPHA)
      {
	if (png_get_valid (png, info, PNG_INFO_tRNS))
	{
	  // tRNS chunk. Every palette entry gets its own alpha value.
	  png_bytep trans;
	  int num_trans;
	  png_get_tRNS (png, info, &trans, &num_trans, 0);
	  
	  // see if there is a single entry w/ alpha==0 and all other 255.
	  // if yes use keycolor transparency.
	  bool only_binary_trans = true;
	  for (int i = 0; (i < num_trans)&& only_binary_trans; i++)
	  {
	    if (trans[i] != 0xff)
	    {
	      only_binary_trans = only_binary_trans && (trans[i] == 0) 
		&& (keycolor_index == -1);
	      keycolor_index = i;
	    }
	  }
	  if (!only_binary_trans)
	  {
	    keycolor_index = -1;
	    png_set_palette_to_rgb (png);
	    png_set_tRNS_to_alpha (png);
	    ImageType = imgRGB;
	  }
	  else
	    NewFormat &= ~CS_IMGFMT_ALPHA; // We're basically keycolored
	}
	else
	  NewFormat &= ~CS_IMGFMT_ALPHA; // Nope, not transparent
      }
      break;
    case PNG_COLOR_TYPE_RGB:
    case PNG_COLOR_TYPE_RGB_ALPHA:
      ImageType = imgRGB;
      dataType = rdtRGBpixel;
      NewFormat = (Format & ~CS_IMGFMT_MASK) | CS_IMGFMT_TRUECOLOR;
      // If there is no alpha information, fill with 0xff
      if (!(color_type & PNG_COLOR_MASK_ALPHA))
      {
        NewFormat &= ~CS_IMGFMT_ALPHA;
        // Check if we have keycolor transparency.
        if (png_get_valid (png, info, PNG_INFO_tRNS))
	{
	  png_color_16p trans_values;
	  png_get_tRNS (png, info, 0, 0, &trans_values);
	  hasKeycolor = true;
	  keycolor.red = csConvertEndian (trans_values->red) & 0xff;
	  keycolor.green = csConvertEndian (trans_values->green) & 0xff;
	  keycolor.blue = csConvertEndian (trans_values->blue) & 0xff;
	}
        png_set_filler (png, 0xff, PNG_FILLER_AFTER);
      }
      break;
    default:
      goto nomem2;
  }

  if ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_ANY)
    Format = NewFormat;
  else
    // Copy alpha flag
    Format = (Format & CS_IMGFMT_MASK) | (NewFormat & ~CS_IMGFMT_MASK);

  return true;
}

bool ImagePngFile::PngLoader::LoadData ()
{
  size_t rowbytes, exp_rowbytes;

  if (setjmp (png->jmpbuf))
  {
nomem2:
    // If we get here, we had a problem reading the file
    png_destroy_read_struct (&png, &info, (png_infopp) 0);
    png = 0; info = 0;
    return false;
  }

  if (bit_depth > 8)
  {
    // tell libpng to strip 16 bit/color files down to 8 bits/color
    png_set_strip_16 (png);
    bit_depth = 8;
  }
  else if (bit_depth < 8)
    // Expand pictures with less than 8bpp to 8bpp
    png_set_packing (png);

  // Update structure with the above settings
  png_read_update_info (png, info);

  // Allocate the memory to hold the image
  if (ImageType == imgRGB)
    exp_rowbytes = Width * 4;	  // RGBA
  else if (ImageType == imgGrayAlpha)
    exp_rowbytes = Width * 2;	  // GrayA
  else
    exp_rowbytes = Width;	  // Index

  rowbytes = png_get_rowbytes (png, info);
  if (rowbytes != exp_rowbytes)
    goto nomem2;                        // Yuck! Something went wrong!

  png_bytep * const row_pointers = new png_bytep[Height];

  if (setjmp (png->jmpbuf))             // Set a new exception handler
  {
    delete [] row_pointers;
    goto nomem2;
  }

  uint8 *NewImage = 0;
  if (ImageType == imgRGB)
    NewImage = new uint8 [Width * Height * 4];
  else if (ImageType == imgGrayAlpha)
    NewImage = new uint8 [Width * Height * 2];
  else
    NewImage = new uint8 [Width * Height];
  if (!NewImage)
    goto nomem2;

  for (int row = 0; row < Height; row++)
    row_pointers [row] = ((png_bytep)NewImage) + row * rowbytes;

  // Read image data
  png_read_image (png, row_pointers);

  // read rest of file, and get additional chunks in info_ptr
  png_read_end (png, (png_infop)0);

  if (ImageType == imgRGB)
  {
    if (csPackRGBA::IsRGBpixelSane())
      rgbaData = (csRGBpixel*)NewImage;
    else
    {
      rgbaData = csPackRGBA::CopyUnpackRGBAtoRGBpixel
	(NewImage, Width * Height);
      delete[] NewImage;
    }
  }
  else if (ImageType == imgPAL)
  {
    png_color graypal [256];
    png_color *png_palette = 0;
    int colors, i;
    if (!png_get_PLTE (png, info, &png_palette, &colors))
    {
      // This is a grayscale image, build a grayscale palette
      png_palette = graypal;
      int entries = (1 << bit_depth) - 1;
      colors = entries + 1;
      for (i = 0; i <= entries; i++)
        png_palette [i].red = png_palette [i].green = png_palette [i].blue =
          (i * 255) / entries;
    }
    // copy PNG palette.
    palette = new csRGBpixel[colors];
    paletteCount = colors;
    for (i = 0; i < colors; i++)
    {
      palette[i].red = png_palette[i].red;
      palette[i].green = png_palette[i].green;
      palette[i].blue = png_palette[i].blue;
    }
    if (keycolor_index != -1)
    {
      hasKeycolor = true;
      keycolor.red = palette[keycolor_index].red;
      keycolor.green = palette[keycolor_index].green;
      keycolor.blue = palette[keycolor_index].blue;
    }
    indexData = NewImage;
  }
  else // grayscale + alpha
  {
    // This is a grayscale image, build a grayscale palette
    paletteCount = 256;
    palette = new csRGBpixel [256];
    int i, entries = (1 << bit_depth) - 1;
    for (i = 0; i <= entries; i++)
      palette [i].red = palette [i].green = palette [i].blue =
        (i * 255) / entries;

    int pixels = Width * Height;
    indexData = new uint8 [pixels];
    alpha = new uint8 [pixels];
    uint8 *src = (uint8 *)NewImage;
    for (i = 0; i < pixels; i++)
    {
      indexData [i] = *src++;
      alpha [i] = *src++;
    }
    delete [] (uint8 *)NewImage;
  }

  // clean up after the read, and free any memory allocated
  png_destroy_read_struct (&png, &info, (png_infopp) 0);
  png = 0; info = 0;

  // Free the row pointers array that is not needed anymore
  delete [] row_pointers;

  dataSource = 0;
  return true;
}

csRef<iImageFileLoader> ImagePngFile::InitLoader (csRef<iDataBuffer> source)
{
  csRef<PngLoader> loader;
  loader.AttachNew (new PngLoader (Format, source));
  if (!loader->InitOk()) return 0;
  return loader;
}
