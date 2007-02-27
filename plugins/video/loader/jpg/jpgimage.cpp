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

#include "cssysdef.h"
#include <math.h>
#include <stdio.h>
#include <setjmp.h>
#include <string.h>

#include "csutil/dirtyaccessarray.h"
#include "csutil/memfile.h"
#include "csutil/scf.h"
#include "csplugincommon/imageloader/optionsparser.h"
#include "jpgimage.h"
#include "csgfx/rgbpixel.h"
#include "csgfx/packrgb.h"
#include "csutil/databuf.h"
#include "ivaria/reporter.h"

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(JPGImageIO)
{

CS_LEAKGUARD_IMPLEMENT (ImageJpgFile);

SCF_IMPLEMENT_FACTORY (csJPGImageIO)

#define JPG_MIME "image/jpg"

static iImageIO::FileFormatDescription formatlist[2] =
{
  {JPG_MIME, "Grayscale", CS_IMAGEIO_LOAD},
  {JPG_MIME, "Truecolor", CS_IMAGEIO_LOAD|CS_IMAGEIO_SAVE}
};

/// report something
static void Report (iObjectRegistry *object_reg, int severity, 
                    const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (csQueryRegistry<iReporter> (object_reg));
  if (rep)
    rep->ReportV (severity, "crystalspace.graphic.image.io.jpeg", 
      msg, arg);
  else
  {
    csPrintf ("crystalspace.graphic.image.io.jpeg: ");
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

csJPGImageIO::csJPGImageIO (iBase *pParent) :
  scfImplementationType (this, pParent)
{
  formats.Push (&formatlist[0]);
  formats.Push (&formatlist[1]);
}

csJPGImageIO::~csJPGImageIO()
{
}

const csImageIOFileFormatDescriptions& csJPGImageIO::GetDescription ()
{
  return formats;
}

/* ==== Error mgmnt ==== */

class JpegException : public std::exception
{
public:
  Jpeg::Common& common;
  const JpegError& error;

  JpegException (Jpeg::Common& common, const JpegError& error) : 
    common (common), error (error) {}
};

JpegError::JpegError()
{
  jpeg_std_error (this);
  error_exit = ErrorExit;
}

void JpegError::ErrorExit (j_common_ptr cinfo)
{
  JpegError* this_ = static_cast<JpegError*> (cinfo->err);
  throw JpegException (*(static_cast<Jpeg::Common*> (cinfo)), *this_);
}

csString JpegError::ErrorMessage (Jpeg::Common& common) const
{
  char errmsg [256];
  format_message (&common, errmsg);
  return csString (errmsg);
}

/* ==== Destination mgmnt ==== */

class MyDestinationManager : public Jpeg::DestinationManager
{
  static const size_t buf_len = 4096;
  JOCTET buffer[buf_len];
public:
  csMemFile file;

  void Init (Jpeg::Compress& cinfo)
  {
    next_output_byte = buffer;
    free_in_buffer = buf_len;
  }
  bool EmptyOutputBuffer (Jpeg::Compress& cinfo)
  {
    file.Write ((char*)buffer, buf_len);
    next_output_byte = buffer;
    free_in_buffer = buf_len;
    return true;
  }
  void Term (Jpeg::Compress& cinfo)
  {
    size_t len = buf_len - free_in_buffer;

    if (len > 0)
      file.Write ((char*)buffer, len);
  }
};

csPtr<iImage> csJPGImageIO::Load (iDataBuffer* buf, int iFormat)
{
  ImageJpgFile* i = new ImageJpgFile (object_reg, iFormat);
  if (i && !i->Load (buf))
  {
    delete i;
    return 0;
  }
  return csPtr<iImage> (i);
}

void csJPGImageIO::SetDithering (bool enable)
{
  ImageJpgFile::dither = enable;
}

csPtr<iDataBuffer> csJPGImageIO::Save(iImage *Image, iImageIO::FileFormatDescription*,
  const char* extraoptions)
{
  int format = Image->GetFormat ();
  switch (format & CS_IMGFMT_MASK)
  {
    case CS_IMGFMT_TRUECOLOR:
      break;
    default:
      // unknown format
      return 0;
  } /* endswitch */

  // compression options
  int quality = 80;
  bool progressive = false;

  /*
     parse output options.
     options are a comma-separated list and can be either
     'option' or 'option=value'.

     supported options:
       compress=#   image compression, 0..100 higher values give smaller files
		    but uglier results.
       progressive  progressive encoding.

     examples:
       compress=50
       progressive,compress=30
   */
  csImageLoaderOptionsParser optparser (extraoptions);
  optparser.GetBool ("progressive", progressive);
  if (optparser.GetInt ("compress", quality))
  {
    quality = 100 - quality;
    if (quality < 0) quality = 0;
    if (quality > 100) quality = 100;
  }

  JpegError jerr;
  try
  {
    //struct jpg_datastore ds;
    MyDestinationManager dest;
    Jpeg::Compress cinfo (&jerr);

    cinfo.SetDestination (&dest);

    cinfo.image_width = Image->GetWidth ();
    cinfo.image_height = Image->GetHeight ();
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    cinfo.SetDefaults ();
    cinfo.SetQuality (quality, true);
    if (progressive)
    {
      cinfo.SimpleProgression ();
    }
    cinfo.StartCompress (true);

    JSAMPROW row_pointer[1];
    csDirtyAccessArray<uint8> rgbData;
    {
      const size_t numPixels = Image->GetWidth () * Image->GetHeight ();
      rgbData.SetSize (numPixels * 3);
      csPackRGB::PackRGBpixelToRGB ((csRGBpixel*)Image->GetImageData (),
        rgbData.GetArray(), numPixels);
    }
    JSAMPLE *image = rgbData.GetArray();

    while (cinfo.next_scanline < cinfo.image_height)
    {
      row_pointer[0] = 
        (JSAMPLE*)&image[cinfo.next_scanline * cinfo.image_width * 3];
      cinfo.WriteScanlines (row_pointer, 1);
    }

    cinfo.FinishCompress ();

    return dest.file.GetAllData();
  }
  catch (JpegException& e)
  {
    Report (object_reg, CS_REPORTER_SEVERITY_WARNING,
      "%s", e.error.ErrorMessage (e.common).GetDataSafe());
    return 0;
  }
}

csPtr<iDataBuffer> csJPGImageIO::Save (iImage *Image, const char *mime,
  const char* extraoptions)
{
  if (!strcasecmp (mime, JPG_MIME))
    return Save (Image, (iImageIO::FileFormatDescription *)0,
      extraoptions);
  return 0;
}

//---------------------------------------------------------------------------

/* ==== Constructor ==== */

bool ImageJpgFile::dither = true;

ImageJpgFile::JpegLoader::~JpegLoader()
{
  delete cinfo;
}

bool ImageJpgFile::JpegLoader::InitOk()
{
  // For now we don't support alpha-map images
  // The libjpeg docs are unclear on this subject, it seems that alpha
  // mapped images could be supported by libjpeg (it supports a random
  // number of abstract color channels) but I (A.Z.) just don't have
  // alpha-mapped JPEG images and can't test it.
  Format &= ~CS_IMGFMT_ALPHA;

  try
  {
    /* ==== Step 1: allocate and initialize JPEG decompression object */
    /* We set up the normal JPEG error routines, then override error_exit. */
    cinfo = new Jpeg::Decompress (&jerr);

    /* ==== Step 2: specify data source (memory buffer, in this case) */
    cinfo->SetDataSource (&srcMgr);

    /* ==== Step 3: read file parameters with jpeg_read_header() */
    cinfo->ReadHeader (true);

    /* ==== Step 4: set parameters for decompression */
    // We want max quality, doesnt matter too much it can be a bit slow
    if ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8)
    {
      cinfo->two_pass_quantize = TRUE;
      cinfo->dither_mode = dither ? JDITHER_FS : JDITHER_NONE;
      cinfo->quantize_colors = TRUE;
      cinfo->desired_number_of_colors = 256;
      dataType = rdtIndexed;
    }
    else
      dataType = rdtR8G8B8;
    // We almost always want RGB output (no grayscale, yuv etc)
    if (cinfo->jpeg_color_space != JCS_GRAYSCALE)
      cinfo->out_color_space = JCS_RGB;

    // Recalculate output image dimensions
    cinfo->CalcOutputDimensions ();

    /* ==== Step 5: Start decompressor */

    cinfo->StartDecompress ();
    /* We may need to do some setup of our own at this point before reading
     * the data.  After jpeg_start_decompress() we have the correct scaled
     * output image dimensions available, as well as the output colormap
     * if we asked for color quantization.
     * In this example, we need to make an output work buffer of the right size.
     */
    Width = cinfo->output_width;
    Height = cinfo->output_height;

    if ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_ANY)
      Format = (Format & ~CS_IMGFMT_MASK) |
        (cinfo->quantize_colors ? CS_IMGFMT_PALETTED8 : CS_IMGFMT_TRUECOLOR);
  }
  catch (JpegException& e)
  {
    if (e.error.msg_code != JERR_NO_SOI)
    {
      Report (object_reg, CS_REPORTER_SEVERITY_WARNING,
        "%s", e.error.ErrorMessage (e.common).GetDataSafe());
    }
    return false;
  }

  return true;
}

bool ImageJpgFile::JpegLoader::LoadData ()
{
  try
  {
    size_t row_stride;		/* physical row width in output buffer */

    size_t pixelcount = Width * Height;
    if ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8)
      indexData = new uint8 [pixelcount];
    else
      rawData.AttachNew (new csDataBuffer (pixelcount * 3));

    /* JSAMPLEs per row in output buffer */
    row_stride = cinfo->output_width * cinfo->output_components;
    /* Make a one-row-high sample array that will go away when done with image */
    JSAMPARRAY buffer = (*cinfo->mem->alloc_sarray)
      ((j_common_ptr) cinfo, JPOOL_IMAGE, (JDIMENSION)row_stride, 1);

    /* ==== Step 6: while (scan lines remain to be read) */
    /*           jpeg_read_scanlines(...); */

    /* Here we use the library's state variable cinfo.output_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
     */
    size_t bufp = 0;
    while (cinfo->output_scanline < cinfo->output_height)
    {
      /* jpeg_read_scanlines expects an array of pointers to scanlines.
       * Here the array is only one element long, but you could ask for
       * more than one scanline at a time if that's more convenient.
       */
      cinfo->ReadScanlines (buffer, 1);

      if (cinfo->output_components == 1)
        if (cinfo->quantize_colors)
        {
	  // Safety.
	  if (bufp + row_stride > pixelcount) break;
          /* paletted image */
	  memcpy (indexData + bufp, buffer [0], row_stride);
        }
        else
        {
	  // Safety.
	  if (bufp + (size_t)cinfo->output_width > pixelcount) break;
	  /* Grayscale image */
	  uint8* out = rawData->GetUint8() + bufp*3;
          for (size_t i = 0; i < (size_t)cinfo->output_width; i++)
          {
	    const uint8 v = buffer [0][i];
	    *out++ = v;
	    *out++ = v;
	    *out++ = v;
          }
        }
      else
      {
        // Safety.
        if (bufp + (size_t)cinfo->output_width > pixelcount) break;
        /* rgb triplets */
        uint8* out = rawData->GetUint8() + bufp*3;
        memcpy (out, buffer[0], cinfo->output_width * 3);
      }
      bufp += cinfo->output_width;
    }

    /* Get palette */
    if (cinfo->quantize_colors)
    {
      palette = new csRGBpixel [256];
      int cshift = 8 - cinfo->data_precision;
      for (int i = 0; i < cinfo->actual_number_of_colors; i++)
      {
        palette [i].red   = cinfo->colormap [0] [i] << cshift;
        if (cinfo->jpeg_color_space != JCS_GRAYSCALE)
        {
          palette [i].green = cinfo->colormap [1] [i] << cshift;
          palette [i].blue  = cinfo->colormap [2] [i] << cshift;
        }
        else
          palette [i].green = palette [i].blue = palette [i].red;
      }
    }

    /* ==== Step 7: Finish decompression */

    cinfo->FinishDecompress();
    /* We can ignore the return value since suspension is not possible
     * with the buffer data source.
     */
  }
  catch (JpegException& e)
  {
    if (e.error.msg_code != JERR_NO_SOI)
    {
      Report (object_reg, CS_REPORTER_SEVERITY_WARNING,
        "%s", e.error.ErrorMessage (e.common).GetDataSafe());
    }
    delete cinfo; cinfo = 0;
    return false;
  }

  /* ==== Step 8: Release JPEG decompression object */
  /* This is an important step since it will release a good deal of memory. */
  delete cinfo; cinfo = 0;

  /* At this point you may want to check to see whether any corrupt-data
   * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
   */

  /* And we're done! */
  dataSource = 0;
  return true;
}

csRef<iImageFileLoader> ImageJpgFile::InitLoader (csRef<iDataBuffer> source)
{
  csRef<JpegLoader> loader;
  loader.AttachNew (new JpegLoader (Format, object_reg, source));
  if (!loader->InitOk()) return 0;
  return loader;
}

}
CS_PLUGIN_NAMESPACE_END(JPGImageIO)
