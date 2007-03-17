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

#ifndef __LIGHTMAP_H__
#define __LIGHTMAP_H__

#include "common.h"
#include "swappable.h"

namespace lighter
{
  class LightmapMask;

  class Lightmap : public Swappable
  {
  public:
    Lightmap (uint width, uint height);

    ~Lightmap ();

    inline void Initialize ()
    {      
    }

    // Add a general ambient term
    void AddAmbientTerm (const csColor amb);

    // Apply the exposure function
    void ApplyExposureFunction (float expConstant, float expMax);

    // Apply scale and clamp function
    void ApplyScaleClampFunction (float scaleVal, float maxValue);

    // Grow the lightmap    
    inline void Grow (uint w, uint h)
    { 
      width = csMax (width, w);
      height = csMax (height, h);
      lightmapAllocator.Grow (width, height);
    }

    // Set the lightmap size.. this might mess up the allocator
    inline void SetSize (uint w, uint h)
    {
      width = w; height = h;
    }

    // Set the max used uv
    inline void SetMaxUsedUV (uint u, uint v)
    {
      maxUsedU = csMax (maxUsedU, u);
      maxUsedV = csMax (maxUsedV, v);
    }

    // Set a pixel to given color
    inline void SetAddPixel (size_t u, size_t v, csColor c)
    {
      colorArray[v*width + u] += c;
    }

    // Save the lightmap to given file
    void SaveLightmap (const csString& file);

    // Fixup, de-antialise lightmap etc
    void FixupLightmap (const LightmapMask& mask);

    // Data getters
    inline csColor* GetData () const { return colorArray; }

    inline uint GetWidth () const {return width; }
    inline uint GetHeight () const {return height; }

    inline uint GetMaxUsedU () const {return maxUsedU; }
    inline uint GetMaxUsedV () const {return maxUsedV; }

    inline csSubRectangles& GetAllocator () { return lightmapAllocator; }
    inline const csSubRectangles& GetAllocator () const { return lightmapAllocator; }

    inline void SetFilename (const csString& fn) { filename = fn; }
    inline const csString& GetFilename () const { return filename; }
    inline csString GetTextureName () 
    { return GetTextureNameFromFilename (filename); }

    iTextureWrapper* GetTexture();

    bool IsNull ();

    void Lock () const
    {
      if (!IsLocked() && (colorArray == 0))
      {
	colorArray = AllocColors();
      }
      Swappable::Lock();
    }
    
    virtual void GetSwapData (void*& data, size_t& size)
    {
#if 0
      data = colorArray;
      size = colorArray ? width * height * sizeof (csColor) : 0;
      // Set a bogus pointer so accesses to swapped data causes a segfault
      colorArray = BogusPointer ();
#endif
      if (colorArray == 0) colorArray = AllocColors ();
      data = colorArray;
      size = width * height * sizeof (csColor);
      // Set a bogus pointer so accesses to swapped data causes a segfault
      colorArray = BogusPointer ();
    }
    virtual size_t GetSwapSize()
    {
#if 0
      return colorArray ? width * height * sizeof (csColor) : 0;
#endif
      return width * height * sizeof (csColor);
    }
    virtual void SwapIn (void* data, size_t size)
    {
#if 0
      if (data != 0) 
      {
        /* We weren't empty when swapped out: take pointer over. */
        CS_ASSERT (size == width * height * sizeof (csColor));
        colorArray = (csColor*)data;
      }
      else
      {
        /* We were empty when swapped in. We must've been swapped out before,
           so the pointer should be bogus here. */
        CS_ASSERT (colorArray == BogusPointer ());
        /* Lock() with no color array allocated. Being swapped in implies we're
           locked - however, Lock() won't have allocated memory as colorArray
           wasn't 0. So allocate now. */
        colorArray = AllocColors ();
      }
#endif
      CS_ASSERT (size == width * height * sizeof (csColor));
      CS_ASSERT (colorArray == BogusPointer ());
      colorArray = (csColor*)data;
    }
  protected:
    // The color data itself
    mutable csColor *colorArray;

    // Size
    uint width, height;

    // Max used U/V
    uint maxUsedU, maxUsedV;

    // Area allocator
    csSubRectangles lightmapAllocator;

    // Filename
    csString filename;

    iTextureWrapper* texture;
    csString GetTextureNameFromFilename (const csString& file);

    CS_FORCEINLINE csColor* BogusPointer () const 
    { return ((csColor*)~0) - (width * height); }
    CS_FORCEINLINE csColor* AllocColors () const
    { 
      return (csColor*)SwappableHeap::Alloc (width * height * 
        sizeof (csColor));
    }
  };
  typedef csArray<Lightmap> LightmapArray;
  typedef csPDelArray<Lightmap> LightmapPtrDelArray;

  //Used as a mask for lightmap during un-antialiasing
  class LightmapMask
  {
  public:
    LightmapMask (const Lightmap &lm)
      : width (lm.GetWidth ()), height (lm.GetHeight ())
    {
      // Copy over the size from the lightmap
      maskData.SetSize (width*height, 0);
    }
    
    csDirtyAccessArray<float> maskData;
    uint width, height;
  };
  typedef csArray<LightmapMask> LightmapMaskArray;

  class LightmapPostProcess
  {
  public:
    // Add a general ambient term
    static void AddAmbientTerm (csColor* colors, size_t numColors, 
      const csColor amb);

    // Apply the exposure function
    static void ApplyExposureFunction (csColor* colors, size_t numColors, 
      float expConstant, float expMax);

    // Do a static scaling
    static void ApplyScaleClampFunction (csColor* colors, size_t numColors, 
      float scaleValue, float maxValue);
  };
}

#endif
