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
  Lightmap::Lightmap (int w, int h)
    : colorArray (0), width (0), height (0),
    lightmapAllocator (csRect (0, 0, w, h)), texture (0)
  {
    lightmapAllocator.SetGrowPO2 (true);
  }

  Lightmap::Lightmap (const Lightmap& other) : colorArray (0), 
    width (other.width), height (other.height),
    lightmapAllocator (other.lightmapAllocator.GetRectangle()), texture (0)
  {
    lightmapAllocator.SetGrowPO2 (other.lightmapAllocator.GetGrowPO2());
  }

  Lightmap::~Lightmap ()
  {
    Lock();
    SwappableHeap::Free (colorArray);
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

  void Lightmap::SaveLightmap (const csString& fname, bool gray)
  {
    ScopedSwapLock<Lightmap> l (*this);

    filename = fname;
    //write it out

    csRef<iImage> img;
    void* data;
    const size_t pixelArraySize = width*height;

    if (gray)
    {
      // first we downsample to LDR gray values
      data = cs_malloc (pixelArraySize);
      uint8* pixelData = (uint8*)data;
      for (uint i = 0; i < pixelArraySize; i++)
      {
        csColor &c = colorArray[i];
        // make sure we don't oversaturate below
        float f = csClamp (c.Luminance(), 1.0f, 0.0f);
        pixelData[i] = (uint) (f * 255.0f);
      }
      // make an image
      csRGBpixel* gray = new csRGBpixel[256];
      for (int v = 0; v < 256; v++) gray[v].Set (v, v, v);
      img.AttachNew (new csImageMemory (width, height, pixelData, false,
        CS_IMGFMT_PALETTED8, gray));
    }
    else
    {
      // first we downsample to LDR csRGBpixel RGBA
      data = cs_malloc (pixelArraySize * sizeof (csRGBpixel));
      csRGBpixel *pixelData = (csRGBpixel*)data;
      for (uint i = 0; i < pixelArraySize; i++)
      {
        csColor &c = colorArray[i];
        c.Clamp (1.0f,1.0f,1.0f); //just make sure we don't oversaturate below
        pixelData[i].red = (uint) (c.red * 255.0f);
        pixelData[i].green = (uint) (c.green * 255.0f);
        pixelData[i].blue = (uint) (c.blue * 255.0f);
      }
      // make an image
      img.AttachNew (new csImageMemory (width, height, pixelData, false));
    }

    csRef<iDataBuffer> imgData = globalLighter->imageIO->Save (img, "image/png");
    csRef<iFile> file = globalLighter->vfs->Open (fname, VFS_FILE_WRITE);
    if (file)
    {
      file->Write (imgData->GetData (), imgData->GetSize ());
      file->Flush ();
    }
    cs_free (data);
  }

  void Lightmap::FixupLightmap (const LightmapMask& mask)
  {
    ScopedSwapLock<Lightmap> l (*this);
    ScopedSwapLock<LightmapMask> m (mask);
    csColor* lmData = colorArray;
    const float* mmData = mask.GetMaskData();

    const size_t size = width*height;

    for (uint j = 0; j < size; j++, lmData++, mmData++)
    {
      if (*mmData == 0 /*|| *mmData >= 1.0f*/)
        continue;

      *lmData *= (1.0f / *mmData);
    }

    // Reset
    
    lmData = colorArray;
    mmData = mask.GetMaskData();

    for (int v = 0; v < height; v++)
    {
      // now scan over the row
      for (int u = 0; u < width; u++)
      {
        const int idx = v*width+u;

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

  bool Lightmap::IsNull (float threshold, bool gray)
  {
    ScopedSwapLock<Lightmap> l (*this);

    if (gray)
    {
      for (int i = 0; i < width * height; i++)
      {
        float f = colorArray[i].Luminance();
        if (f >= threshold)
          return false;
      }
    }
    else
    {
      for (int i = 0; i < width * height; i++)
      {
        const csColor &c = colorArray[i];
        if (!c.IsBlack (threshold))
          return false;
      }
    }
    return true;
  }

  bool Lightmap::IsOneColor (float threshold, csColor& color)
  {
    ScopedSwapLock<Lightmap> l (*this);

    if ((width == 1) && (height == 1))
    {
      color = colorArray[0];
      return true;
    }
    // rest of code assumes number of pixels is even

    // The threshold check is done every 2*N pixels, N this amount.
    static const int checkThresholdFrequence = 1024;
    csColor minColor (colorArray[0]);
    csColor maxColor (colorArray[1]);
    int n = width * height - 2;
    csColor* p = colorArray + 2;
    int check = checkThresholdFrequence;
    do
    {
      if (p[0].red < p[1].red)
      {
        if (p[0].red < minColor.red) minColor.red = p[0].red;
        if (p[1].red > maxColor.red) maxColor.red = p[1].red;
      }
      else
      {
        if (p[1].red < minColor.red) minColor.red = p[1].red;
        if (p[0].red > maxColor.red) maxColor.red = p[0].red;
      }
      if (p[0].green < p[1].green)
      {
        if (p[0].green < minColor.green) minColor.green = p[0].green;
        if (p[1].green > maxColor.green) maxColor.green = p[1].green;
      }
      else
      {
        if (p[1].green < minColor.green) minColor.green = p[1].green;
        if (p[0].green > maxColor.green) maxColor.green = p[0].green;
      }
      if (p[0].blue < p[1].blue)
      {
        if (p[0].blue < minColor.blue) minColor.blue = p[0].blue;
        if (p[1].blue > maxColor.blue) maxColor.blue = p[1].blue;
      }
      else
      {
        if (p[1].blue < minColor.blue) minColor.blue = p[1].blue;
        if (p[0].blue > maxColor.blue) maxColor.blue = p[0].blue;
      }
      p += 2;
      n -= 2;
      if (--check == 0)
      {
        if (((maxColor.red - minColor.red) > threshold)
          || ((maxColor.green - minColor.green) > threshold)
          || ((maxColor.blue - minColor.blue) > threshold))
        {
          return false;
        }
        if (n > 0) check = checkThresholdFrequence;
      }
    }
    while (n > 0);

    if (check > 0)
    {
      if (((maxColor.red - minColor.red) > threshold)
        || ((maxColor.green - minColor.green) > threshold)
        || ((maxColor.blue - minColor.blue) > threshold))
      {
        return false;
      }
    }

    color = (minColor + maxColor) * 0.5f;
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
  
  void DirectionMap::Normalize()
  {
    ScopedSwapLock<DirectionMap> l (*this);
    
    const size_t directionsArraySize = width*height;
    
    for (size_t i = 0; i < directionsArraySize; i++)
    {
      csVector3& v = mapData[i];
      if (!v.IsZero()) v.Normalize();
    }
  }
  
  void DirectionMap::AddFromLightInfluences (const LightInfluences& influences)
  {
    ScopedSwapLock<DirectionMap> l (*this);
    ScopedSwapLock<LightInfluences> l2 (influences);
    
    uint inflW = csMin (influences.GetWidth(), width-influences.GetXOffset());
    uint inflH = csMin (influences.GetHeight(), height-influences.GetYOffset());
    for (uint y = 0; y < inflH; y++)
    {
      for (uint x = 0; x < inflW; x++)
      {
        mapData[(x+influences.GetXOffset())+width*(y+influences.GetYOffset())]
          += influences.GetDirectionForLocalCoord (x, y);
      }
    }
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
