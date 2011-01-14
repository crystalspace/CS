/*
    Copyright (C) 2007 by Jelle Hellemans aka sueastside
              (C) 2008 by Frank Richter

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

#include "crystalspace.h"

#include "basemapgen.h"
#include "layers.h"

// The global pointer to basemapgen
extern BaseMapGen *basemapgen;

//-----------------------------------------------------------------------------

AlphaLayers::~AlphaLayers ()
{
  for (size_t i = 0; i < alphaMaps.GetSize(); i++)
    cs_free (alphaMaps[i]);
}
  
bool AlphaLayers::BuildAlphaMapsFromMatMap (iImage* matMap)
{
  w = matMap->GetWidth ();
  h = matMap->GetHeight ();
  size_t numPix = w * h;

  uint8* matmapPtr = (uint8*)(matMap->GetImageData ());
  for (size_t n = 0; n < numPix; n++)
  {
    unsigned int layerNum = *matmapPtr++;

    uint8*& alphaData = alphaMaps.GetExtend (layerNum, 0);
    if (alphaData == 0)
    {
      alphaData = (uint8*)cs_calloc (numPix, 1);
    }
    alphaData[n] = 255;
  }
  return true;
}

bool AlphaLayers::AddAlphaMap (iImage* alphaMap)
{
  int alphamap_w = alphaMap->GetWidth();
  int alphamap_h = alphaMap->GetHeight();

  if ((w != 0) && (h != 0))
  {
    if ((alphamap_w != w) || (alphamap_h != h))
    {
      basemapgen->Report ("Alpha maps don't match in size.");
      return false;
    }
  }
  else
  {
    w = alphamap_w;
    h = alphamap_h;
  }
  
  size_t numPix = w * h;
  uint8* alphaData = (uint8*)cs_malloc (numPix);
  uint8* alphaPtr = alphaData;
      
  int mapFormat = alphaMap->GetFormat();
  if ((mapFormat & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8)
  {
    // Assume grayscale
    uint8* srcData = (uint8*)alphaMap->GetImageData();
    while (numPix-- > 0)
    {
      *alphaPtr++ = *srcData++;
    }
  }
  else if (mapFormat & CS_IMGFMT_ALPHA)
  {
    // Extract data from alpha channel
    csRGBpixel* srcData = (csRGBpixel*)alphaMap->GetImageData();
    while (numPix-- > 0)
    {
      *alphaPtr++ = (srcData++)->alpha;
    }
  }
  else
  {
    // Extract data from RGB intensity
    csRGBpixel* srcData = (csRGBpixel*)alphaMap->GetImageData();
    while (numPix-- > 0)
    {
      *alphaPtr++ = (srcData++)->Intensity();
    }
  }
  alphaMaps.Push (alphaData);
  
  return true;
}
  
void AlphaLayers::AddRemainderAlpha()
{
  size_t numPix = w * h;
  uint8* alphaData = (uint8*)cs_malloc (numPix);
  uint8* alphaPtr = alphaData;
  
  CS_ALLOC_STACK_ARRAY(uint8*, givenAlphaPtr, alphaMaps.GetSize());
  for (size_t i = 0; i < alphaMaps.GetSize(); i++)
    givenAlphaPtr[i] = alphaMaps[i];
      
  while (numPix-- > 0)
  {
    uint a = 0;
    for (size_t i = 0; i < alphaMaps.GetSize(); i++)
      a += *(givenAlphaPtr[i]++);
    *alphaPtr++ = 255 - csMin (a, 255u);
  }
  
  alphaMaps.Push (alphaData);
}

float AlphaLayers::GetAlpha (size_t layer, float coord_x, float coord_y) const
{
  uint8* p = alphaMaps[layer];
  if (p == 0) return 0.0f;

  // Calculate the material coordinates.
  float x_f = (coord_x * w);
  float y_f = (coord_y * h);
  int x = int (x_f);
  int y = int (y_f);
  int nx = (x+1)%w;
  int ny = (y+1)%h;

  // Bilinearly filter from material.
  float p00 ((p[y*w+x]) * (1.0f/255.0f));
  float p01 ((p[ny*w+x]) * (1.0f/255.0f));
  float p11 ((p[ny*w+nx]) * (1.0f/255.0f));
  float p10 ((p[y*w+nx]) * (1.0f/255.0f));

  float f1 = x_f - x;
  float f2 = y_f - y;

  return csLerp (csLerp (p00, p10, f1), 
    csLerp (p01, p11, f1), f2);
}
