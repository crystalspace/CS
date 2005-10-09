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
#include "csgfx/memimage.h"
#include "csutil/ref.h"
#include "csutil/scf_interface.h"
#include "csutil/scf_implementation.h"
#include "iutil/databuff.h"
#include "iutil/job.h"

struct iObjectRegistry;

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

/**
 * An image file loader.
 * Handles the decoding of an image.
 */
struct iImageFileLoader : public virtual iBase
{
  SCF_INTERFACE(iImageFileLoader, 2,0,0);
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
  /// Query whether a keycolor is set
  virtual bool HasKeyColor() const = 0;
  /// Query keycolor
  virtual void GetKeyColor (int &r, int &g, int &b) const = 0;
};

/**
 * Base image loader implementation.
 */
class CS_CRYSTALSPACE_EXPORT csCommonImageFileLoader : 
  public scfImplementation1<csCommonImageFileLoader, iImageFileLoader>
{
protected:
  /// Format of the image
  int Format;
  /// Buffer with raw data.
  csRef<iDataBuffer> rawData;
  /// The type of image data this loader provides.
  csLoaderDataType dataType;
  /// Pointer to RGBA data (if dataType == rdtRGBpixel)
  csRGBpixel* rgbaData;
  /// Pointer to indexed data (if dataType == rdtIndexed)
  uint8* indexData;
  /// Palette for indexed colors.
  csRGBpixel* palette;
  /// Number of entries in the palette
  size_t paletteCount;
  /// Alpha data for indexed images.
  uint8* alpha;
  /// Whether the image has a keycolor.
  bool hasKeycolor;
  /// Keycolor.
  csRGBcolor keycolor;
  /// Image dimensions.
  int Width, Height;
public:
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
  virtual bool HasKeyColor() const { return hasKeycolor; }
  virtual void GetKeyColor (int &r, int &g, int &b) const
  { 
    r = keycolor.red; g = keycolor.green; b = keycolor.blue; 
  }
};

#define THREADED_LOADING

/**
 * A base class for image loader plugin iImage implementations.
 */
class CS_CRYSTALSPACE_EXPORT csCommonImageFile : 
  public scfImplementationExt0<csCommonImageFile, csImageMemory>
{
protected:
  friend class csCommonImageFileLoader;
  
  class CS_CRYSTALSPACE_EXPORT LoaderJob : 
    public scfImplementation1<LoaderJob, iJob>
  {
  public:
    /// The actual image loader.
    csRef<iImageFileLoader> currentLoader;
    /// Result of the iImageFileLoader->LoadData() call.
    bool loadResult;
    /// Create new instance with a given image loader.
    LoaderJob (iImageFileLoader* loader);
    virtual ~LoaderJob();

    virtual void Run();
  };

#ifdef THREADED_LOADING
  /// Reference to the job for loading this image.
  csRef<LoaderJob> loadJob;
  /// Reference to job queue.
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
   * format, data type and keycolor flag (note not the actual color)
   * already set.
   */
  virtual csRef<iImageFileLoader> InitLoader (csRef<iDataBuffer> source) = 0;

  /// Wait for the current image loading job to finish.
  void WaitForJob() const;
  /// Convert data from the loader to actual image data.
  void MakeImageData();

  virtual const void *GetImageData ();
  virtual const csRGBpixel* GetPalette ();
  virtual const uint8* GetAlpha ();

  virtual bool HasKeyColor () const 
  { 
#ifdef THREADED_LOADING
    if (loadJob)
    {
      return loadJob->currentLoader->HasKeyColor();
    }
#endif
    return has_keycolour; 
  }

  virtual void GetKeyColor (int &r, int &g, int &b) const
  { 
#ifdef THREADED_LOADING
    if (loadJob)
    {
      // Keycolor may only be available after loading...
      WaitForJob();
      loadJob->currentLoader->GetKeyColor (r, g, b);
      return;
    }
#endif
    r = keycolour.red; g = keycolour.green; b = keycolour.blue; 
  }

  /**
   * Convert an image loader data type into a raw data description (if 
   * supported).
   */
  static const char* DataTypeString (csLoaderDataType dataType);
  virtual const char* GetRawFormat() const;
  virtual csRef<iDataBuffer> GetRawData() const;
};

/** @} */

#endif // __CS_CSPLUGINCOMMON_IMAGELOADER_COMMONIMAGEFILE_H__
