/*
    JNG image file format support for CrystalSpace 3D library
    Copyright (C) 2002 by Jorrit Tyberghein
    Copyright (C) 2002 by Frank Richter <resqu@gmx.ch>

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

#define MNG_USE_DLL
#include "libmng.h"

#include "cssysdef.h"
#include "csutil/scf.h"
#include "cssys/sysfunc.h"
#include "csgfx/rgbpixel.h"
#include "csutil/databuf.h"
#include "ivaria/reporter.h"

#include "jng.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csJNGImageIO)
  SCF_IMPLEMENTS_INTERFACE (iImageIO)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csJNGImageIO::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csJNGImageIO);

SCF_EXPORT_CLASS_TABLE (csjngimg)
  SCF_EXPORT_CLASS (csJNGImageIO, "crystalspace.graphic.image.io.jng",
		"CrystalSpace JNG image format I/O plugin")
SCF_EXPORT_CLASS_TABLE_END

#define JNG_MIME "image/jng" // @@@ officially image/x-jng
#define MNG_MIME "image/mng"

static iImageIO::FileFormatDescription formatlist[5] =
{
  {JNG_MIME, "RGBA", CS_IMAGEIO_LOAD|CS_IMAGEIO_SAVE},
  {MNG_MIME, "RGBA", CS_IMAGEIO_LOAD|CS_IMAGEIO_SAVE}
};

csJNGImageIO::csJNGImageIO (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  formats.Push (&formatlist[0]);
  formats.Push (&formatlist[1]);
}

const csVector& csJNGImageIO::GetDescription ()
{
  return formats;
}

iImage *csJNGImageIO::Load (uint8* iBuffer, uint32 iSize, int iFormat)
{
  ImageJngFile* i = new ImageJngFile (iFormat, object_reg);
  if (i && !i->Load (iBuffer, iSize))
  {
    delete i;
    return NULL;
  }
  return i;
}

void csJNGImageIO::SetDithering (bool)
{
}

iDataBuffer *csJNGImageIO::Save (iImage *Image, iImageIO::FileFormatDescription *,
  const char* extraoptions)
{
  return NULL;
}

iDataBuffer *csJNGImageIO::Save (iImage *Image, const char *mime,
  const char* extraoptions)
{
  return NULL;
}

//---------------------------------------------------------------------------

void ImageJngFile::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  iReporter* rep = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (rep)
  {
    rep->ReportV (severity, "crystalspace.graphic.image.io.jng", 
      msg, arg);
    rep->DecRef ();
  }
  else
  {
    csPrintf ("crystalspace.graphic.image.io.jng: ");
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

void ImageJngFile::ReportLibmngError (mng_handle hMNG, char* msg)
{
  mng_int8     severity;
  mng_chunkid  chunkname;
  mng_uint32   chunkseq;
  mng_int32    extra1;
  mng_int32    extra2;
  mng_pchar    errortext;

  mng_getlasterror (hMNG, &severity, &chunkname, &chunkseq, &extra1, 
    &extra2, &errortext);

  Report (CS_REPORTER_SEVERITY_WARNING,
    "%s: %s (severity %d, chunkname %.8x, chunkseq %d, %.8x, %.8x)",
    msg, errortext, severity, chunkname, chunkseq, extra1, extra2);
}

/*
 * A small wor don how everything works:
 * Basically, libmng is made to read & display animated stuff and
 * requires to have some timing stuff set up, thus the getticks and
 * settimer callbacks, although they seem to be a bit weird when loading
 * still images. 
 *
 * We just want one frame. To achieve this, mng_display() is called once. 
 * With the images we are most interested in here (JNGs) this is probably 
 * successful and libmng fills our buffer. However this does _not_ work as 
 * well with MNGs. It actually depends on the file; with some the first frame 
 * is displayed right, with others just partially. Download the MNGsuite
 * (http://www.libmng.com/MNGsuite) and see for yourself.
 */

mng_ptr ImageJngFile::mng_alloc (mng_size_t iLen) 
{
  mng_ptr res = (mng_ptr)new uint8[iLen];
  if (res)                 
    memset (res, 0, iLen);
  return res;
}

void ImageJngFile::mng_free (mng_ptr iPtr, mng_size_t iLen)
{
  delete iPtr;
  (void)iLen;     // Kill compiler warning
}

mng_bool ImageJngFile::mng_openstream (mng_handle hMNG)
{
  (void)hMNG;
  return MNG_TRUE;                     
}

mng_bool ImageJngFile::mng_closestream (mng_handle hMNG)
{
  (void)hMNG;
  return MNG_TRUE;                     
}

mng_bool ImageJngFile::mng_readdata (mng_handle hHandle, mng_ptr pBuf,
					      mng_uint32 iBuflen, mng_uint32 *pRead)
{
  ImageJngFile *this_;

  this_ = (ImageJngFile *)mng_get_userdata (hHandle);

  // determine amount of data to read; 0 if EOF
  *pRead = MIN (iBuflen, this_->bufferSize - (this_->bufptr - this_->buffer));
  if (*pRead > 0)
  {
    memcpy (pBuf, this_->bufptr, *pRead);
    this_->bufptr += *pRead;
  }
  else
    *pRead = 0;

  return MNG_TRUE;
}

mng_bool ImageJngFile::mng_processheader (mng_handle hHandle, 
					  mng_uint32 iWidth, mng_uint32 iHeight)
{
  ImageJngFile *this_;
  
  this_ = (ImageJngFile *)mng_get_userdata (hHandle);

  if (mng_set_canvasstyle (hHandle, MNG_CANVAS_RGBA8))
  {
    this_->ReportLibmngError (hHandle, "failed to set canvas style");
    return MNG_FALSE;
  }
   
  this_->Width = iWidth;
  this_->Height = iHeight;
  this_->NewImage = new csRGBpixel [iWidth * iHeight];

  return MNG_TRUE;
}

mng_ptr ImageJngFile::mng_getcanvasline (mng_handle hHandle, mng_uint32 iLineNr)
{
  ImageJngFile *this_;

  this_ = (ImageJngFile *)mng_get_userdata (hHandle);

  return this_->NewImage + (iLineNr * this_->Width);
}

mng_bool ImageJngFile::mng_imagerefresh (mng_handle hHandle,
					 mng_uint32 iX, mng_uint32 iY, 
					 mng_uint32 iWidth, mng_uint32 iHeight)
{
  return MNG_TRUE;
}

mng_uint32 ImageJngFile::mng_gettickcount (mng_handle hHandle)
{
  return csGetTicks();      
}

mng_bool ImageJngFile::mng_settimer (mng_handle hHandle, mng_uint32 iMsecs)
{
  return MNG_TRUE;
}

bool ImageJngFile::Load (uint8 *iBuffer, uint32 iSize)
{
  NewImage = NULL;
  mng_retcode retcode;

  mng_handle handle = mng_initialize ( mng_ptr(this), mng_alloc, 
                                      mng_free, MNG_NULL);
  if (!handle)
  {
    Report (CS_REPORTER_SEVERITY_WARNING,
      "failed to initialize libmng");
    return false;
  }

  buffer = iBuffer;
  bufptr = buffer;
  bufferSize = iSize;

  if ((mng_setcb_openstream (handle, mng_openstream) != MNG_NOERROR) ||
      (mng_setcb_closestream (handle, mng_closestream) != MNG_NOERROR) ||
      (mng_setcb_readdata (handle, mng_readdata) != MNG_NOERROR) ||
      (mng_setcb_processheader(handle, mng_processheader) != MNG_NOERROR) ||
      (mng_setcb_getcanvasline(handle, mng_getcanvasline) != MNG_NOERROR) ||
      (mng_setcb_refresh(handle, mng_imagerefresh) != MNG_NOERROR) ||
      (mng_setcb_gettickcount(handle, mng_gettickcount) != MNG_NOERROR) ||
      (mng_setcb_settimer(handle, mng_settimer) != MNG_NOERROR))
  {
    ReportLibmngError (handle, "failed to set libmng callbacks");
    mng_cleanup (&handle);
    return false;
  }

  retcode = mng_read (handle);
  if (retcode != MNG_NOERROR)
  {
    if (retcode != MNG_INVALIDSIG) // maybe its just not an jng/mng...
      ReportLibmngError (handle, "failed to read data");
    mng_cleanup (&handle);
    return false;
  }

  // Don't read PNGs 
  if (mng_get_sigtype (handle) == mng_it_png)
  {
    if (NewImage) delete[] NewImage;
    mng_cleanup (&handle);
    return false;
  }

  mng_display (handle);

  if (NewImage)
    convert_rgba (NewImage);

  mng_cleanup (&handle);

  return NewImage;
}
