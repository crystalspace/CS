/*
    DSS image file format support for CrystalSpace 3D library
    Copyright (C) 2003 by Matze Braun <matze@braunis.de>

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
#ifndef __DDS_DDSLOADER_H__
#define __DDS_DDSLOADER_H__

#include "csutil/scf.h"
#include "csgfx/csimage.h"

#include "iutil/comp.h"
#include "igraphic/imageio.h"

#include "csutil/refarr.h"
#include "dds.h"

struct iObjectRegistry;

class csDDSImageIO : public iImageIO, public iComponent
{
public:
  SCF_DECLARE_IBASE;

  csDDSImageIO (iBase* parent);
  virtual ~csDDSImageIO ();

  virtual const csImageIOFileFormatDescriptions& GetDescription ();
  virtual csPtr<iImage> Load (uint8* buffer, size_t size, int format);
  virtual void SetDithering (bool dithering);
  virtual csPtr<iDataBuffer> Save (iImage* image, const char* mime,
				   const char* options);
  virtual csPtr<iDataBuffer> Save (iImage* image,
      iImageIO::FileFormatDescription* format, const char* options);
  
  virtual bool Initialize (iObjectRegistry* objreg);

private:
  csImageIOFileFormatDescriptions formats;
  csRef<iObjectRegistry> object_reg;
};

class csDDSImageFile : public csImageFile
{
  friend class csDDSImageIO;
public:
  virtual ~csDDSImageFile ();

  virtual csPtr<iImage> MipMap (int step, csRGBpixel* transp);
  
  virtual int HasMipmaps ();  
private:
  csDDSImageFile (iObjectRegistry* object_reg, int format);

  bool Load (dds::Loader* loader);

  csRefArray<iImage> mipmaps;
  int mipmapcount;
  csRef<iObjectRegistry> object_reg;

  void Report (int severity, const char* msg, ...);
};

#endif

