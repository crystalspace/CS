/*
    Copyright (C) 2005 by Jorrit Tyberghein
		  2005 by Frank Richter

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
#include "csutil/threadjobqueue.h"
#include "csgfx/packrgb.h"
#include "iutil/objreg.h"
#include "csplugincommon/imageloader/commonimagefile.h"

SCF_IMPLEMENT_IBASE(csCommonImageFileLoader)
  SCF_IMPLEMENTS_INTERFACE(iImageFileLoader)
SCF_IMPLEMENT_IBASE_END

csCommonImageFileLoader::csCommonImageFileLoader (int format)
  : Format(format), dataType (rdtInvalid), rgbaData (0), indexData (0),
  palette (0), paletteCount (0), alpha (0), hasKeycolor (false),
  Width (0), Height (0)
{
  SCF_CONSTRUCT_IBASE(0);
}

csCommonImageFileLoader::~csCommonImageFileLoader()
{
  delete[] indexData;
  delete[] palette;
  delete[] rgbaData;
  SCF_DESTRUCT_IBASE();
}

void csCommonImageFileLoader::ApplyTo (csImageMemory* image)
{
  //image->SetFormat (Format);
  CS_ASSERT (dataType != rdtInvalid);
  if (dataType == rdtIndexed)
  {
    image->ConvertFromPal8 (indexData, alpha, palette, paletteCount);
    palette = 0; indexData = 0; alpha = 0;
  }
  else if (dataType == rdtRGBpixel)
  {
    image->ConvertFromRGBA (rgbaData);
    rgbaData = 0;
  }
  else
  {
    const size_t numPix = rawData->GetSize() / 3;
    if ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_TRUECOLOR)
    {
      csPackRGB::UnpackRGBtoRGBpixelBuffer (
	(csRGBpixel*)image->GetImagePtr(), rawData->GetUint8(), numPix);
    }
    else
    {
      csRGBpixel* rgbaData = new csRGBpixel[numPix];
      csPackRGB::UnpackRGBtoRGBpixelBuffer (rgbaData, 
	rawData->GetUint8(), numPix);
      image->ConvertFromRGBA (rgbaData);
    }
    rawData = 0;
  }
  if (hasKeycolor)
    image->SetKeyColor (keycolor.red, keycolor.green, keycolor.blue);
  image->CheckAlpha();
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csCommonImageFile::LoaderJob)
  SCF_IMPLEMENTS_INTERFACE(iJob)
SCF_IMPLEMENT_IBASE_END

csCommonImageFile::LoaderJob::LoaderJob (iImageFileLoader* loader) 
  : currentLoader (loader)
{
  SCF_CONSTRUCT_IBASE(0);
}

csCommonImageFile::LoaderJob::~LoaderJob()
{
  SCF_DESTRUCT_IBASE();
}

void csCommonImageFile::LoaderJob::Run()
{
  loadResult = currentLoader->LoadData ();
}

//---------------------------------------------------------------------------

csCommonImageFile::csCommonImageFile (iObjectRegistry* object_reg, int format) 
  : csImageMemory (format), object_reg (object_reg) 
{
#ifdef THREADED_LOADING
  static const char queueTag[] = "crystalspace.jobqueue.imageload";
  jobQueue = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg, queueTag, iJobQueue);
  if (!jobQueue.IsValid())
  {
    jobQueue.AttachNew (new csThreadJobQueue ());
    object_reg->Register (jobQueue, queueTag);
  }
#endif
}

csCommonImageFile::~csCommonImageFile()
{
#ifdef THREADED_LOADING
  if (loadJob.IsValid())
    jobQueue->Unqueue (loadJob);
#endif
}

bool csCommonImageFile::Load (csRef<iDataBuffer> source)
{
#ifdef THREADED_LOADING
  csRef<iImageFileLoader> currentLoader;
#endif
  currentLoader = InitLoader (source);
  if (!currentLoader.IsValid()) return false;
  const uint format = currentLoader->GetFormat();
  CS_ASSERT ((format & CS_IMGFMT_MASK) != CS_IMGFMT_ANY);
  Format = format;
  const int w = currentLoader->GetWidth();
  const int h = currentLoader->GetHeight();
  CS_ASSERT ((w != 0) && (h != 0));
  SetDimensions (w, h);
#ifdef THREADED_LOADING
  loadJob.AttachNew (new LoaderJob (currentLoader));
  jobQueue->Enqueue (loadJob);
  return true;
#else
  return currentLoader->LoadData();
#endif
}

void csCommonImageFile::WaitForJob() const
{
#ifdef THREADED_LOADING
  jobQueue->PullAndRun (loadJob);
#endif
}

void csCommonImageFile::MakeImageData()
{
#ifdef THREADED_LOADING
  if (loadJob)
  {
    WaitForJob();
    loadJob->currentLoader->ApplyTo (this);
    loadJob = 0;
    jobQueue = 0;
  }
#else
  if (currentLoader)
  {
    currentLoader->ApplyTo (this);
    currentLoader = 0;
  }
#endif
}

const void* csCommonImageFile::GetImageData()
{
  MakeImageData();
  return csImageMemory::GetImageData();
}

const csRGBpixel* csCommonImageFile::GetPalette()
{
  if (!(Format & CS_IMGFMT_PALETTED8)) return 0;
  MakeImageData();
  return csImageMemory::GetPalette();
}

const uint8* csCommonImageFile::GetAlpha ()
{
  if ((Format & (CS_IMGFMT_PALETTED8 | CS_IMGFMT_ALPHA))
    != (CS_IMGFMT_PALETTED8 | CS_IMGFMT_ALPHA)) return 0;
  MakeImageData();
  return csImageMemory::GetAlpha();
}

const char* csCommonImageFile::DataTypeString (csLoaderDataType dataType)
{
  switch (dataType)
  {
    case rdtR8G8B8:
      return "r8g8b8";
    default:
      return 0;
  }
}

const char* csCommonImageFile::GetRawFormat() const
{
#ifdef THREADED_LOADING
  if (!loadJob) return 0;
  //WaitForJob();
  csRef<iImageFileLoader> currentLoader = 
    loadJob->currentLoader;
#endif
  return (currentLoader.IsValid()) ? 
    DataTypeString (currentLoader->GetDataType()) : 0;
}

csRef<iDataBuffer> csCommonImageFile::GetRawData() const
{
#ifdef THREADED_LOADING
  if (!loadJob) return 0;
  WaitForJob();
  csRef<iImageFileLoader> currentLoader = 
    loadJob->currentLoader;
#endif
  return (currentLoader.IsValid()) ? currentLoader->GetRawData() : 0;
}
