/*
    DSS image file format support for CrystalSpace 3D library
    Copyright (C) 2004-2005 by Frank Richter

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

#ifndef __CS_DDS_DDSSAVER_H__
#define __CS_DDS_DDSSAVER_H__

#include "csutil/memfile.h"
#include "csutil/ref.h"
#include "csplugincommon/imageloader/optionsparser.h"
#include "iutil/databuff.h"

#include "ImageLib/ImageDXTC.h"

struct iImage;

class csDDSSaver
{
  class Format
  {
  public:
    virtual ~Format() {}
    virtual bool Save (csMemFile& out, iImage* image) = 0;
  };
  class FmtB8G8R8 : public Format
  {
    virtual bool Save (csMemFile& out, iImage* image);
  };
  class FmtB8G8R8A8 : public Format
  {
    virtual bool Save (csMemFile& out, iImage* image);
  };
  class FmtDXT : public Format
  {
  protected:
    ImageLib::DXTCMethod method;
    const csImageLoaderOptionsParser& options;
  public:
    FmtDXT (ImageLib::DXTCMethod method, 
      const csImageLoaderOptionsParser& options) : method(method), 
      options(options) {}
    virtual bool Save (csMemFile& out, iImage* image);
  };

  uint SaveMips (csMemFile& out, iImage* image, Format* format);
public:
  csPtr<iDataBuffer> Save (csRef<iImage> image, 
    const csImageLoaderOptionsParser& options);
};

#endif // __CS_DDS_DDSSAVER_H__
