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
    Lightmap (int width, int height);

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
    inline void Grow (int w, int h)
    { 
      width = csMax (GetWidth(), w);
      height = csMax (GetHeight(), h);
      lightmapAllocator.Grow (width, height);
    }

    // Set a pixel to given color
    inline void SetAddPixel (size_t u, size_t v, csColor c)
    {
      colorArray[v*GetWidth() + u] += c;
    }

    // Save the lightmap to given file
    void SaveLightmap (const csString& file);

    // Fixup, de-antialise lightmap etc
    void FixupLightmap (const LightmapMask& mask);

    // Data getters
    inline csColor* GetData () const { return colorArray; }

    inline int GetWidth () const { return width; }
    inline int GetHeight () const { return height; }

    inline CS::SubRectanglesCompact& GetAllocator () { return lightmapAllocator; }
    inline const CS::SubRectanglesCompact& GetAllocator () const { return lightmapAllocator; }
    inline void UpdateDimensions ()
    {
      width = lightmapAllocator.GetRectangle().Width();
      height = lightmapAllocator.GetRectangle().Height();
    }

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
      if (colorArray == 0) colorArray = AllocColors ();
      data = colorArray;
      size = GetWidth() * GetHeight() * sizeof (csColor);
      // Set a bogus pointer so accesses to swapped data causes a segfault
      colorArray = BogusPointer ();
    }
    virtual size_t GetSwapSize()
    {
      return GetWidth() * GetHeight() * sizeof (csColor);
    }
    virtual void SwapIn (void* data, size_t size)
    {
      CS_ASSERT (size == GetWidth() * GetHeight() * sizeof (csColor));
      CS_ASSERT (colorArray == BogusPointer ());
      colorArray = (csColor*)data;
    }
  protected:
    // The color data itself
    mutable csColor *colorArray;

    int width, height;

    // Area allocator
    CS::SubRectanglesCompact lightmapAllocator;

    // Filename
    csString filename;

    iTextureWrapper* texture;
    csString GetTextureNameFromFilename (const csString& file);

    inline csColor* BogusPointer () const 
    { return ((csColor*)~0) - (GetWidth() * GetHeight()); }
    inline csColor* AllocColors () const
    { 
      return (csColor*)SwappableHeap::Alloc (GetWidth() * GetHeight() * 
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
