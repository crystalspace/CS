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

#ifndef __CS_CSPLUGINCOMMON_IMAGELOADER_COMMONIMAGEFILE_H__
#define __CS_CSPLUGINCOMMON_IMAGELOADER_COMMONIMAGEFILE_H__

/**\file
 * Base classes for image loaders.
 */

#include "csextern.h"
#include "iutil/databuff.h"
#include "iutil/job.h"
#include "csutil/ref.h"
#include "csgfx/memimage.h"

/**
 * \addtogroup plugincommon
 * @{ */

/**
 * The data type a loader provides. The data is automatically converted
 * to the needed image format.
 */
enum csLoaderDataType
{
  /// Invalid data type - the loader didn't set one
  rdtInvalid,
  /// 24bpp pixel data
  rdtR8G8B8,
  /// Array of csRGBpixel
  rdtRGBpixel,
  /// 8-bit paletted data
  rdtIndexed
};

class csCommonImageFile;

SCF_VERSION (iImageFileLoader, 0, 0, 1);

/**
 * An image file loader.
 * Handles the decoding of an image.
 */
struct iImageFileLoader : public iBase
{
  /// Do the loading.
  virtual bool LoadData () = 0;
  /// Return "raw data" (if supported)
  virtual csRef<iDataBuffer> GetRawData() = 0;
  /// Return type of raw data
  virtual csLoaderDataType GetDataType() = 0;
  /// Query width.
  virtual int GetWidth() = 0;
  /// Query height.
  virtual int GetHeight() = 0;
  /// Query format.
  virtual int GetFormat() = 0;
  /// Copy the image data into an image object.
  virtual void ApplyTo (csImageMemory* image) = 0;
};

/**
 * Base image loader implementation.
 */
class CS_CSPLUGINCOMMON_EXPORT csCommonImageFileLoader : public iImageFileLoader
{
protected:
  int Format;
  csRef<iDataBuffer> rawData;
  csLoaderDataType dataType;
  csRGBpixel* rgbaData;
  uint8* indexData;
  csRGBpixel* palette;
  size_t paletteCount;
  uint8* alpha;
  bool hasKeycolor;
  csRGBcolor keycolor;
  int Width, Height;
public:
  SCF_DECLARE_IBASE;

  csCommonImageFileLoader (int format);
  virtual ~csCommonImageFileLoader();

  virtual csRef<iDataBuffer> GetRawData() 
  { return rawData; }
  virtual csLoaderDataType GetDataType() 
  { return dataType; }
  virtual int GetWidth() { return Width; }
  virtual int GetHeight() { return Height; }
  virtual int GetFormat() { return Format; }
  virtual void ApplyTo (csImageMemory* image);
};

#define THREADED_LOADING

/**
 * A base class for image loader plugin iImage implementations.
 */
class CS_CSPLUGINCOMMON_EXPORT csCommonImageFile : public csImageMemory
{
protected:
  friend class csCommonImageFileLoader;
  
  class CS_CSPLUGINCOMMON_EXPORT LoaderJob : public iJob
  {
  public:
    csRef<iImageFileLoader> currentLoader;
    bool loadResult;
    SCF_DECLARE_IBASE;

    LoaderJob (iImageFileLoader* loader);
    virtual ~LoaderJob();

    virtual void Run();
  };

#ifdef THREADED_LOADING
  csRef<LoaderJob> loadJob;
  csRef<iJobQueue> jobQueue;
#else
  csRef<iImageFileLoader> currentLoader;
#endif
  iObjectRegistry* object_reg;

  csCommonImageFile (iObjectRegistry* object_reg, int format);
  virtual ~csCommonImageFile();

  /// Load an image from a data buffer.
  virtual bool Load (csRef<iDataBuffer> source);
  /**
   * Create a loader object, which will handle the actual loading.
   * Note: the returned loader should have a proper width, height,
   * format and data type already set.
   */
  virtual csRef<iImageFileLoader> InitLoader (csRef<iDataBuffer> source) = 0;

  /// Wait for the current image loading job to finish.
  void WaitForJob() const;
  /// Convert data from the loader ro actual image data.
  void MakeImageData();

  virtual const void *GetImageData ();
  virtual const csRGBpixel* GetPalette ();
  virtual const uint8* GetAlpha ();

  static const char* DataTypeString (csLoaderDataType dataType);
  virtual const char* GetRawFormat() const;
  virtual csRef<iDataBuffer> GetRawData() const;
};

/** @} */

#endif // __CS_CSPLUGINCOMMON_IMAGELOADER_COMMONIMAGEFILE_H__
