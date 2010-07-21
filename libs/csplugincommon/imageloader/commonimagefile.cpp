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
#include "csgfx/packrgb.h"
#include "csplugincommon/imageloader/commonimagefile.h"
#include "csutil/scopedlock.h"
#include "csutil/threadjobqueue.h"
#include "iutil/objreg.h"


csCommonImageFileLoader::csCommonImageFileLoader (int format)
  : scfImplementationType (this), 
  Format(format), dataType (rdtInvalid), rgbaData (0), indexData (0),
  palette (0), paletteCount (0), alpha (0), hasKeycolor (false),
  Width (0), Height (0)
{
}

csCommonImageFileLoader::~csCommonImageFileLoader()
{
  delete[] indexData;
  delete[] palette;
  delete[] rgbaData;
}

void csCommonImageFileLoader::ApplyTo (csImageMemory* image)
{
  //image->SetFormat (Format);
  CS_ASSERT (dataType != rdtInvalid);
  if (dataType == rdtIndexed)
  {
    image->ConvertFromPal8 (indexData, alpha, palette, (int)paletteCount);
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

csCommonImageFile::LoaderJob::LoaderJob (csCommonImageFile* fileToLoad) 
  : scfImplementationType (this), fileToLoad (fileToLoad)
{
}

csCommonImageFile::LoaderJob::~LoaderJob()
{
}

void csCommonImageFile::LoaderJob::Run()
{
  csRef<iImageFileLoader> currentLoader;
  {
    csRef<csCommonImageFile> fileToLoad;
    {
      CS::Threading::MutexScopedLock lock (fileToLoadLock);
      if (!this->fileToLoad) return;
      /* Catch possibility that Run() is executed just while fileToLoad is
         destructed */
      if (this->fileToLoad->GetRefCount() == 0) return;
      fileToLoad = this->fileToLoad;
    }
    currentLoader = fileToLoad->currentLoader;
    if (!currentLoader.IsValid()) return;
  }
  loadResult = currentLoader->LoadData ();
}

void csCommonImageFile::LoaderJob::ClearFileToLoad ()
{
  CS::Threading::MutexScopedLock lock (fileToLoadLock);
  fileToLoad = nullptr;
}

//---------------------------------------------------------------------------

#include "csutil/custom_new_disable.h"
csCommonImageFile::csCommonImageFile (iObjectRegistry* object_reg, int format) 
  : scfImplementationType (this, format), object_reg (object_reg) 
{
#ifdef CSCOMMONIMAGEFILE_THREADED_LOADING
  static const char queueTag[] = "crystalspace.jobqueue.imageload";
  jobQueue = csQueryRegistryTagInterface<iJobQueue> (object_reg, queueTag);
  if (!jobQueue.IsValid())
  {
    jobQueue.AttachNew (new CS::Threading::ThreadedJobQueue (1,
      CS::Threading::THREAD_PRIO_NORMAL, "image load"));
    object_reg->Register (jobQueue, queueTag);
  }
#endif
}
#include "csutil/custom_new_enable.h"

csCommonImageFile::~csCommonImageFile()
{
#ifdef CSCOMMONIMAGEFILE_THREADED_LOADING
  if (loadJob.IsValid())
  {
    loadJob->ClearFileToLoad ();
    jobQueue->Dequeue (loadJob, true);
  }
#endif
}

#include "csutil/custom_new_disable.h"
bool csCommonImageFile::Load (csRef<iDataBuffer> source)
{
  currentLoader = InitLoader (source);
  if (!currentLoader.IsValid()) return false;
  const uint format = currentLoader->GetFormat();
  CS_ASSERT ((format & CS_IMGFMT_MASK) != CS_IMGFMT_ANY);
  Format = format;
  const int w = currentLoader->GetWidth();
  const int h = currentLoader->GetHeight();
  CS_ASSERT ((w != 0) && (h != 0));
  SetDimensions (w, h);
#ifdef CSCOMMONIMAGEFILE_THREADED_LOADING
  loadJob.AttachNew (new LoaderJob (this));
  jobQueue->Enqueue (loadJob);
  return true;
#else
  return currentLoader->LoadData();
#endif
}
#include "csutil/custom_new_enable.h"

void csCommonImageFile::WaitForJob() const
{
#ifdef CSCOMMONIMAGEFILE_THREADED_LOADING
  jobQueue->PullAndRun (loadJob);
#endif
}

void csCommonImageFile::MakeImageData() const
{
#ifdef CSCOMMONIMAGEFILE_THREADED_LOADING
  if (loadJob)
  {
    WaitForJob();
    // Ugly ugly ugly so we can call ApplyTo()...
    csImageMemory* thisNonConst = const_cast<csCommonImageFile*> (this);
    currentLoader->ApplyTo (thisNonConst);
    currentLoader = 0;
    loadJob = 0;
    jobQueue = 0;
  }
#else
  if (currentLoader)
  {
    // Ugly ugly ugly so we can call ApplyTo()...
    csImageMemory* thisNonConst = const_cast<csCommonImageFile*> (this);
    currentLoader->ApplyTo (thisNonConst);
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
      return "b8g8r8";
    default:
      // Let the underlying image handle the other raw data types.
      return 0;
  }
}

const char* csCommonImageFile::GetRawFormat() const
{
#ifdef CSCOMMONIMAGEFILE_THREADED_LOADING
  if (!loadJob) return 0;
  //WaitForJob();
#endif
  if (currentLoader.IsValid())
  {
    const char* rawFormat = DataTypeString (currentLoader->GetDataType());
    if (rawFormat != 0) return rawFormat;
  }
  MakeImageData();
  return csImageMemory::GetRawFormat ();
}

csRef<iDataBuffer> csCommonImageFile::GetRawData() const
{
#ifdef CSCOMMONIMAGEFILE_THREADED_LOADING
  if (!loadJob) return csRef<iDataBuffer> ();
  WaitForJob();
#endif
  if (currentLoader.IsValid()
    && (DataTypeString (currentLoader->GetDataType()) != 0))
    return currentLoader->GetRawData();
  MakeImageData();
  return csImageMemory::GetRawData ();
}
