/*
    Copyright (C) 2011 by Frank Richter

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

#include "densityfactormap.h"

#include "igraphic/image.h"
#include "iutil/databuff.h"

#include "csgeom/math.h"
#include "csgeom/vector4.h"
#include "csgfx/rgbpixel.h"
#include "csgfx/textureformatstrings.h"
#include "csutil/databuf.h"

CS_PLUGIN_NAMESPACE_BEGIN(Engine)
{
  DensityFactorMap::DensityFactorMap() : mapPtr (nullptr)
  {}
  
  float DensityFactorMap::GetDensity (const csVector3& worldCoord) const
  {
    csVector4 mapCoord (world2map * worldCoord);
    
    int mapX = int (mapCoord.x * width);
    if ((mapX < 0) || (mapX >= width)) return 0;
    int mapY = int (mapCoord.y * height);
    if ((mapY < 0) || (mapY >= height)) return 0;
    uint8 val = mapPtr[mapY * width + mapX];
    return val * (1.0f/255.0f);
  }
  
  void DensityFactorMap::SetImage (iImage* image)
  {
    mapDataKeeper.Invalidate();
    mapPtr = nullptr;
    
    width = image->GetWidth();
    height = image->GetHeight();
    
    /* Check 'native' image format... if it's a one-component 8-bit format,
     * take data as it is */
    CS::StructuredTextureFormat nativeFormat (
      CS::TextureFormatStrings::ConvertStructured (image->GetCookedImageFormat()));
    if (nativeFormat.IsValid()
	&& (nativeFormat.GetComponentCount() == 1)
	&& (nativeFormat.GetComponentSize (0) == 8))
    {
      mapPtr = image->GetCookedImageData()->GetUint8();
      mapDataKeeper = image->GetCookedImageData();
    }
    
    // Next: check if it's a paletted image. If so, take it as well.
    if (!mapPtr
	&& ((image->GetFormat() & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8))
    {
      mapPtr = (uint8*)image->GetImageData();
      mapDataKeeper = image;
    }
    
    // ... so we got an RGB image; extract grayscale values from it.
    if (!mapPtr)
    {
      size_t numPixels = width * height;
      const csRGBpixel* sourceData = (csRGBpixel*)image->GetImageData();
      uint8* convertedData = (uint8*)cs_malloc (numPixels);
      uint8* convertedDataPtr = convertedData;
      
      while (numPixels-- > 0)
      {
	/* @@@ Probably a good idea: support for extracting a specific channel
	 * (would allow reuse of multisplat maps for density) */
	uint8 v = sourceData->Luminance();
	*convertedDataPtr = v;
	sourceData++;
	convertedDataPtr++;
      }
      
      mapPtr = convertedData;
      mapDataKeeper.AttachNew (new CS::DataBuffer<> ((char*)convertedData, width * height));
    }
  }
  
  void DensityFactorMap::SetWorldToMapTransform (const CS::Math::Matrix4& tf)
  {
    world2map = tf;
  }
}
CS_PLUGIN_NAMESPACE_END(Engine)
