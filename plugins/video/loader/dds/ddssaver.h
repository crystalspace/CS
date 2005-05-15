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
  typedef bool (*SaverFunc) (csMemFile& out, iImage* image);

  static bool SaveB8G8R8 (csMemFile& out, iImage* image);
  static bool SaveB8G8R8A8 (csMemFile& out, iImage* image);
  static bool SaveDXT (csMemFile& out, iImage* image, 
    ImageLib::DXTCMethod method);
  static bool SaveDXT1 (csMemFile& out, iImage* image)
  { return SaveDXT (out, image, ImageLib::DC_DXT1); }
  static bool SaveDXT3 (csMemFile& out, iImage* image)
  { return SaveDXT (out, image, ImageLib::DC_DXT3); }
public:
  csPtr<iDataBuffer> Save (csRef<iImage> image, 
    const csImageLoaderOptionsParser& options);
};

#endif // __CS_DDS_DDSSAVER_H__
