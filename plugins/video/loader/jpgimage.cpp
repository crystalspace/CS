/*
    Copyright (C) 1998 by Tor Andersson and Jorrit Tyberghein

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

#include <math.h>
#include <stdio.h>
#include <setjmp.h>
#include <string.h>

#include "cssysdef.h"
#include "jpgimage.h"

extern "C"
{
#if defined (OS_WIN32)
#if !defined (COMP_GCC) // Avoid defining "boolean" in libjpeg headers
#  define HAVE_BOOLEAN
#endif
#endif
#define JDCT_DEFAULT JDCT_FLOAT	// use floating-point for decompression
#include <jpeglib.h>
#include <jerror.h>
}

//---------------------------------------------------------------------------
bool RegisterJPG ()
{
  return csImageLoader::Register (new csJPGImageLoader());
}

csImageFile* csJPGImageLoader::LoadImage (UByte* iBuffer, ULong iSize, int iFormat)
{
  ImageJpgFile* i = new ImageJpgFile (iFormat);
  if (i && !i->Load (iBuffer, iSize))
  {
    delete i;
    return NULL;
  }
  return i;    
}

//---------------------------------------------------------------------------

/* ==== Error mgmnt ==== */

struct my_error_mgr
{
  struct jpeg_error_mgr pub;	/* "public" fields */
  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr *my_error_ptr;

static void my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

/* Expanded data source object for memory buffer input */
typedef struct
{
  struct jpeg_source_mgr pub;	/* public fields */
  FILE *infile;			/* source stream */
  JOCTET *buffer;		/* start of buffer */
  boolean start_of_file;	/* have we gotten any data yet? */
} my_source_mgr;

typedef my_source_mgr *my_src_ptr;

/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */
static void init_source (j_decompress_ptr cinfo)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;

  /* We reset the empty-input-file flag for each image,
   * but we don't clear the input buffer.
   * This is correct behavior for reading a series of images from one source.
   */
  src->start_of_file = TRUE;
}

/*
 * Fill the input buffer --- called whenever buffer is emptied.
 * should never happen :)
 */
static boolean fill_input_buffer (j_decompress_ptr cinfo)
{
  /* no-op */ (void)cinfo;
  return TRUE;
}

/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 */
static void skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;

  if (num_bytes > 0)
  {
    src->pub.next_input_byte += (size_t) num_bytes;
    src->pub.bytes_in_buffer -= (size_t) num_bytes;
  }
}

/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */
static void term_source (j_decompress_ptr cinfo)
{
  /* no work necessary here */ (void)cinfo;
}

/*
 * Prepare for input from mem buffer.
 * Leaves buffer untouched.
 */
static void jpeg_memory_src (j_decompress_ptr cinfo, char *inbfr, int len)
{
  my_src_ptr src;

  /* The source object and input buffer are made permanent so that a series
   * of JPEG images can be read from the same file by calling jpeg_stdio_src
   * only before the first one.  (If we discarded the buffer at the end of
   * one image, we'd likely lose the start of the next one.)
   * This makes it unsafe to use this manager and a different source
   * manager serially with the same JPEG object.  Caveat programmer.
   */
  if (cinfo->src == NULL)
  {
    /* first time for this JPEG object? */
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  (size_t)sizeof(my_source_mgr));
    src = (my_src_ptr) cinfo->src;
    src->buffer = (JOCTET *) inbfr;
  }

  src = (my_src_ptr) cinfo->src;
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->pub.term_source = term_source;
  src->infile = 0L;
  src->pub.bytes_in_buffer = len;		/* sets to entire file len */
  src->pub.next_input_byte = (JOCTET *)inbfr;	/* at start of buffer */
}

/* ==== Constructor ==== */

bool ImageJpgFile::Load (UByte* iBuffer, ULong iSize)
{
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;
  int row_stride;		/* physical row width in output buffer */

  // For now we don't support alpha-map images
  // The libjpeg docs are unclear on this subject, it seems that alpha
  // mapped images could be supported by libjpeg (it supports a random
  // number of abstract color channels) but I (A.Z.) just don't have
  // alpha-mapped JPEG images and can't test it.
  Format &= ~CS_IMGFMT_ALPHA;

  int i;

  /* ==== Step 1: allocate and initialize JPEG decompression object */
  /* We set up the normal JPEG error routines, then override error_exit. */
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  if (setjmp (jerr.setjmp_buffer))
  {
#if 0
    // We dont actually need this as we want to know
    // just whenever a error occured or not
    char errmsg [256];
    cinfo.err->format_message ((jpeg_common_struct *)&cinfo, errmsg);
    fprintf (stderr, "%s\n", errmsg);
#endif
    jpeg_destroy_decompress (&cinfo);
    return false;
  }

  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress (&cinfo);

  /* ==== Step 2: specify data source (memory buffer, in this case) */
  jpeg_memory_src (&cinfo, (char *)iBuffer, iSize);

  /* ==== Step 3: read file parameters with jpeg_read_header() */
  (void) jpeg_read_header(&cinfo, TRUE);

  /* ==== Step 4: set parameters for decompression */
  // We want max quality, doesnt matter too much it can be a bit slow
  if ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8)
  {
    extern bool csImage_dither;
    cinfo.two_pass_quantize = TRUE;
    cinfo.dither_mode = csImage_dither ? JDITHER_FS : JDITHER_NONE;
    cinfo.quantize_colors = TRUE;
    cinfo.desired_number_of_colors = 256;
  }
  // We almost always want RGB output (no grayscale, yuv etc)
  if (cinfo.jpeg_color_space != JCS_GRAYSCALE)
    cinfo.out_color_space = JCS_RGB;

  // Recalculate output image dimensions
  jpeg_calc_output_dimensions (&cinfo);

  /* ==== Step 5: Start decompressor */

  (void) jpeg_start_decompress (&cinfo);
  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   * In this example, we need to make an output work buffer of the right size.
   */

  set_dimensions (cinfo.output_width, cinfo.output_height);

  int pixelcount = Width * Height;
  if ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8)
    Image = new UByte [pixelcount];
  else
    Image = new csRGBpixel [pixelcount];

  /* JSAMPLEs per row in output buffer */
  row_stride = cinfo.output_width * cinfo.output_components;
  /* Make a one-row-high sample array that will go away when done with image */
  JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)
    ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

  /* ==== Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

  /* Here we use the library's state variable cinfo.output_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   */
  int bufp = 0;
  while (cinfo.output_scanline < cinfo.output_height)
  {
    /* jpeg_read_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could ask for
     * more than one scanline at a time if that's more convenient.
     */
    (void) jpeg_read_scanlines (&cinfo, buffer, 1);

    if (cinfo.output_components == 1)
      if (cinfo.quantize_colors)
        /* paletted image */
        memcpy (((UByte *)Image) + bufp, buffer [0], row_stride);
      else
      { /* Grayscale image */
        csRGBpixel *out = ((csRGBpixel *)Image) + bufp;
        for (i = 0; i < (int)cinfo.output_width; i++)
        {
          out->red = out->green = out->blue = buffer [0][i];
          out++;
        }
      }
    else
    { /* rgb triplets */
      csRGBpixel *out = ((csRGBpixel *)Image) + bufp;
      for (i = 0; i < (int)cinfo.output_width; i++)
        memcpy (out++, buffer [0] + i * 3, 3);
    }
    bufp += cinfo.output_width;
  }

  /* Get palette */
  if (cinfo.quantize_colors)
  {
    Palette = new csRGBpixel [256];
    int cshift = 8 - cinfo.data_precision;
    for (i = 0; i < cinfo.actual_number_of_colors; i++)
    {
      Palette [i].red   = cinfo.colormap [0] [i] << cshift;
      if (cinfo.jpeg_color_space != JCS_GRAYSCALE)
      {
        Palette [i].green = cinfo.colormap [1] [i] << cshift;
        Palette [i].blue  = cinfo.colormap [2] [i] << cshift;
      }
      else
        Palette [i].green = Palette [i].blue = Palette [i].red;
    }
  }

  if ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_ANY)
    Format = (Format & ~CS_IMGFMT_MASK) |
      (cinfo.quantize_colors ? CS_IMGFMT_PALETTED8 : CS_IMGFMT_TRUECOLOR);

  /* ==== Step 7: Finish decompression */

  (void) jpeg_finish_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the buffer data source.
   */

  /* ==== Step 8: Release JPEG decompression object */
  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_decompress(&cinfo);


  /* At this point you may want to check to see whether any corrupt-data
   * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
   */

  /* And we're done! */
  return true;
}
