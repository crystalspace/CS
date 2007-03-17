/*
  Copyright (C) 2005 by Marten Svanfeldt

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

#include "common.h"

#include "lightmap.h"
#include "lighter.h"

namespace lighter
{
  Lightmap::Lightmap (uint width, uint height)
    : colorArray (0), width (0), height (0), maxUsedU (0), maxUsedV (0), 
    lightmapAllocator (csRect (0,0,1,1)), texture (0)
  {
    Grow (width, height);
  }

  Lightmap::~Lightmap ()
  {
    if (colorArray) SwappableHeap::Free (colorArray);
  }

  void Lightmap::AddAmbientTerm (const csColor amb)
  {
    ScopedSwapLock<Lightmap> l (*this);
    LightmapPostProcess::AddAmbientTerm (colorArray,
      width * height, amb);
  }

  void Lightmap::ApplyExposureFunction (float expConstant, float expMax)
  {
    ScopedSwapLock<Lightmap> l (*this);
    LightmapPostProcess::ApplyExposureFunction (colorArray,
      width * height,  expConstant, expMax);
  }

  void Lightmap::ApplyScaleClampFunction (float scaleVal, float maxValue)
  {
    ScopedSwapLock<Lightmap> l (*this);
    LightmapPostProcess::ApplyScaleClampFunction (colorArray,
      width * height,  scaleVal, maxValue);
  }

  void Lightmap::SaveLightmap (const csString& fname)
  {
    ScopedSwapLock<Lightmap> l (*this);

    filename = fname;
    //write it out

    // first we downsample to LDR csRGBpixel RGBA
    const size_t colorArraySize = width*height;
    csRGBpixel *pixelData = new csRGBpixel[colorArraySize];
    for (uint i = 0; i < colorArraySize; i++)
    {
      csColor &c = colorArray[i];
      c.Clamp (1.0f,1.0f,1.0f); //just make sure we don't oversaturate below
      pixelData[i].red = (uint) (c.red * 255.0f);
      pixelData[i].green = (uint) (c.green * 255.0f);
      pixelData[i].blue = (uint) (c.blue * 255.0f);
    }

    // make an image
    csRef<iImage> img;
    img.AttachNew (new csImageMemory (width, height, pixelData));
    csRef<iDataBuffer> imgData = globalLighter->imageIO->Save (img, "image/png");
    csRef<iFile> file = globalLighter->vfs->Open (fname, VFS_FILE_WRITE);
    if (file)
    {
      file->Write (imgData->GetData (), imgData->GetSize ());
      file->Flush ();
    }
    delete[] pixelData;
  }

  void Lightmap::FixupLightmap (const LightmapMask& mask)
  {
    ScopedSwapLock<Lightmap> l (*this);
    csColor* lmData = colorArray;
    const float* mmData = mask.maskData.GetArray ();

    const size_t size = width*height;

    for (uint j = 0; j < size; j++, lmData++, mmData++)
    {
      if (*mmData < FLT_EPSILON || *mmData >= 1.0f) continue;

      *lmData *= (1.0f / *mmData);
    }

    // Reset
    lmData = colorArray;
    mmData = mask.maskData.GetArray ();

    for (uint v = 0; v < height; v++)
    {
      // now scan over the row
      for (uint u = 0; u < width; u++)
      {
        const uint idx = v*width+u;

        // Only try to fix non-masked
        if (mmData[idx]>0) continue;

        uint count = 0;
        csColor newColor (0.0f,0.0f,0.0f);

        // We have a row above to use
        if (v > 0)
        {
          // We have a column to the left
          if (u > 0 && mmData[(v-1)*width+(u-1)] > FLT_EPSILON) newColor += lmData[(v-1)*width+(u-1)], count++;
          if (mmData[(v-1)*width+(u)] > FLT_EPSILON) newColor += lmData[(v-1)*width+(u)], count++;
          if (u < width-1 && mmData[(v-1)*width+(u+1)] > FLT_EPSILON) newColor += lmData[(v-1)*width+(u+1)], count++;
        }

        //current row
        if (u > 0 && mmData[v*width+(u-1)] > FLT_EPSILON) newColor += lmData[v*width+(u-1)], count++;
        if (u < width-1 && mmData[v*width+(u+1)] > FLT_EPSILON) newColor += lmData[v*width+(u+1)], count++;

        // We have a row below
        if (v < (height-1))
        {
          if (u > 0 && mmData[(v+1)*width+(u-1)] > FLT_EPSILON) newColor += lmData[(v+1)*width+(u-1)], count++;
          if (mmData[(v+1)*width+(u)] > FLT_EPSILON) newColor += lmData[(v+1)*width+(u)], count++;
          if (u < width-1 && mmData[(v+1)*width+(u+1)] > FLT_EPSILON) newColor += lmData[(v+1)*width+(u+1)], count++;
        }

        if (count > 0) 
        {
          newColor *= (1.0f/count);
          lmData[idx] = newColor;
        }
      }
    }
  }

  iTextureWrapper* Lightmap::GetTexture()
  {
    if (texture == 0)
    {
      texture = globalLighter->engine->CreateBlackTexture (
        GetTextureName(), 1, 1, 0, CS_TEXTURE_3D);
    }
    return texture;
  }

  bool Lightmap::IsNull ()
  {
    ScopedSwapLock<Lightmap> l (*this);

    for (uint i = 0; i < width * height; i++)
    {
      const csColor &c = colorArray[i];
      if (!c.IsBlack ())
        return false;
    }
    return true;
  }

  csString Lightmap::GetTextureNameFromFilename (const csString& file)
  {
    csString out (file);
    out.ReplaceAll ("\\", "_"); //replace bad characters
    out.ReplaceAll ("/", "_"); 
    out.ReplaceAll (" ", "_"); 
    out.ReplaceAll (".", "_"); 
    return out;
  }

  //-------------------------------------------------------------------------

  void LightmapPostProcess::AddAmbientTerm (csColor* colors, 
                                            size_t numColors, 
                                            const csColor amb)
  {
    for (uint i = 0; i < numColors; i++)
    {
      colors[i] += amb;
    }
  }

  void LightmapPostProcess::ApplyExposureFunction (csColor* colors, 
                                                   size_t numColors, 
                                                   float expConstant, 
                                                   float expMax)
  {
    for (uint i = 0; i < numColors; i++)
    {
      csColor &c = colors[i];
      c.red = expMax * (1 - expf (-c.red * expConstant));
      c.green = expMax * (1 - expf (-c.green * expConstant));
      c.blue = expMax * (1 - expf (-c.blue * expConstant));
    }
  }

  void LightmapPostProcess::ApplyScaleClampFunction (csColor* colors, size_t numColors, 
    float scaleValue, float maxValue)
  {
    for (uint i = 0; i < numColors; i++)
    {
      csColor &c = colors[i];
      c.red = csClamp (c.red * scaleValue, maxValue, 0.0f);
      c.green = csClamp (c.green * scaleValue, maxValue, 0.0f);
      c.blue = csClamp (c.blue * scaleValue, maxValue, 0.0f);
    }
  }

}
