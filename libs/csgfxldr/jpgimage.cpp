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

#include "sysdef.h"
#include "csgfxldr/jpgimage.h"

extern "C"
{
#  if defined (OS_WIN32)	// Avoid defining "boolean" in libjpeg headers
#    define HAVE_BOOLEAN
#  endif
#  include <jpeglib.h>
#  include "csgfxldr/jmemsrc.c"	// include buffer source input code
}

//---------------------------------------------------------------------------

bool RegisterJPG ()
{
  static JPGImageLoader loader;
  return ImageLoader::Register (&loader);
}

AlphaMapFile* JPGImageLoader::LoadAlphaMap(UByte *buf,ULong size)
{
  (void) buf;
  (void) size;
  return NULL;
}

csImageFile* JPGImageLoader::LoadImage (UByte* buf, ULong size)
{
  CHK (ImageJpgFile* i = new ImageJpgFile(buf, size));
  if (i && (i->get_status() & IFE_BadFormat))
  { CHK ( delete i );  i = NULL; }
  return i;    
}

//---------------------------------------------------------------------------

/* ==== Error mgmnt ==== */
static char jpg_err_msg[256];
struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */
  jmp_buf setjmp_buffer;	/* for return to caller */
};
typedef struct my_error_mgr *my_error_ptr;
static void my_error_exit (j_common_ptr cinfo)
{
  char errmsg [256];

  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->format_message) (cinfo,errmsg);
  strcpy (jpg_err_msg, errmsg);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}


/* ==== Constructor ==== */
ImageJpgFile::~ImageJpgFile () {
  /* do nothing */
}

ImageJpgFile::ImageJpgFile (UByte* ptr, long filesize) : csImageFile () {
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;
  JSAMPARRAY buffer;		/* Output row buffer */
  int row_stride;		/* physical row width in output buffer */

  int bufp;
  int i;
  RGBPixel *pixels;

  /* ==== Step 1: allocate and initialize JPEG decompression object */
  /* We set up the normal JPEG error routines, then override error_exit. */
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  if (setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_decompress(&cinfo);
    status = IFE_BadFormat;
    //set_error (IFE_BadFormat,jpg_err_msg);
    return;
  }
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* ==== Step 2: specify data source (memory buffer, in this case) */
  jpeg_memory_src(&cinfo, (char *)ptr, filesize);

  /* ==== Step 3: read file parameters with jpeg_read_header() */
  (void) jpeg_read_header(&cinfo, TRUE);

  /* ==== Step 4: set parameters for decompression */
  /* In this example, we don't need to change any of the defaults set by
   * jpeg_read_header(), so we do nothing here.
   */

  /* ==== Step 5: Start decompressor */

  (void) jpeg_start_decompress(&cinfo);
  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   * In this example, we need to make an output work buffer of the right size.
   */

  set_dimensions (cinfo.output_width, cinfo.output_height);
  pixels = get_buffer ();
  bufp = 0;

  /* JSAMPLEs per row in output buffer */
  row_stride = cinfo.output_width * cinfo.output_components;
  /* Make a one-row-high sample array that will go away when done with image */
  buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

  /* ==== Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

  /* Here we use the library's state variable cinfo.output_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   */
  while (cinfo.output_scanline < cinfo.output_height) {
    /* jpeg_read_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could ask for
     * more than one scanline at a time if that's more convenient.
     */
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);

    /* Assume put_scanline_someplace wants a pointer and sample count. */
    /* put_scanline_someplace(buffer[0], row_stride); */
    if (cinfo.output_components == 1) { /* grey scale */
      for (i=0;i<row_stride;i++) {
         pixels[bufp].red = buffer[0][i];
         pixels[bufp].green = buffer[0][i];
         pixels[bufp].blue = buffer[0][i];
         bufp ++;
      }
    } else { /* rgb triplets */
      for (i = 0; i < (int)cinfo.output_width; i++) {
        pixels[bufp].red = buffer[0][i*3+0];
        pixels[bufp].green = buffer[0][i*3+1];
        pixels[bufp].blue = buffer[0][i*3+2];
        bufp ++;
      }
    }
  }

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
}

