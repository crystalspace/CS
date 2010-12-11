/*
  Copyright (C) 2007 by Marten Svanfeldt

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

#ifndef __CS_TERRAIN_FEEDERHELPER__
#define __CS_TERRAIN_FEEDERHELPER__

#include "csutil/csstring.h"
#include "imap/loader.h"
#include "iutil/vfs.h"

struct iTerrainCell;
struct iObjectRegistry;


CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{

  enum FeederHeightSourceType
  {
    HEIGHT_SOURCE_IMAGE = 0,
    HEIGHT_SOURCE_RAW8,
    HEIGHT_SOURCE_RAW16LE,
    HEIGHT_SOURCE_RAW16BE,
    HEIGHT_SOURCE_RAW32LE,
    HEIGHT_SOURCE_RAW32BE,
    HEIGHT_SOURCE_RAWFLOATLE,
    HEIGHT_SOURCE_RAWFLOATBE
  };

  class HeightFeederParser
  {
  public:
    HeightFeederParser (const csString& mapSource, const csString& format, 
      iLoader* imageLoader, iObjectRegistry* objReg);

    bool Load (float* outputBuffer, size_t outputWidth, size_t outputHeight,
      size_t outputPitch, float heightScale, float offset);

    bool LoadFromImage (float* outputBuffer, size_t outputWidth,
	size_t outputHeight, size_t outputPitch, float heightScale,
	float offset);

  private:
    csString sourceLocation;
    FeederHeightSourceType sourceFormat;

    csRef<iLoader> imageLoader;
    csRef<iVFS> vfs;
    iObjectRegistry* objReg;
  };

  void SmoothHeightmap (float* heightBuffer, size_t width, size_t height, 
    size_t pitch);

  class NormalFeederParser
  {
  public:
    NormalFeederParser (const csString& mapSource, iLoader* imageLoader, iObjectRegistry* objReg);
    bool Load (csVector3* outputBuffer, size_t outputWidth, size_t outputHeight, size_t outputPitch);

  private:
    csString sourceLocation;

    csRef<iLoader> imageLoader;
    iObjectRegistry* objReg;
  };
  
}
CS_PLUGIN_NAMESPACE_END(Terrain2)


#endif
