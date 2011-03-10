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
    Lightmap (const Lightmap& other);

    ~Lightmap ();

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
    void SaveLightmap (const csString& file, bool gray = false);

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

    bool IsNull (float threshold, bool gray = false);
    bool IsOneColor (float threshold, csColor& color);

    void Lock () const
    {
      if (!IsLocked() && (colorArray == 0))
      {
	colorArray = AllocColors();
      }           
      Swappable::Lock();

      CS_ASSERT (colorArray);
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
      CS_ASSERT (data);

      colorArray = (csColor*)data;      
    }
    const char* Describe() const
    { return "Lightmap"; }

    static csString GetTextureNameFromFilename (const csString& file);
  protected:
    // The color data itself
    mutable csColor *colorArray;

    int width, height;

    // Area allocator
    CS::SubRectanglesCompact lightmapAllocator;

    // Filename
    csString filename;

    iTextureWrapper* texture;

    inline csColor* BogusPointer () const 
    { return ((csColor*)~0) - (GetWidth() * GetHeight()); }
    inline csColor* AllocColors () const
    { 
      return (csColor*)SwappableHeap::Alloc (GetWidth() * GetHeight() * 
        sizeof (csColor));
    }
  };
  typedef csPDelArray<Lightmap> LightmapPtrDelArray;

  //Used as a mask for lightmap during un-antialiasing
  class LightmapMask : public Swappable
  {
    mutable float* maskData;
    uint width, height;
    
    inline float* BogusPointer () const 
    { return ((float*)~0) - (width * height); }
    inline float* AllocMask () const
    { 
      return (float*)SwappableHeap::Alloc (width * height * 
        sizeof (float));
    }
  public:
    LightmapMask (const Lightmap &lm) : maskData (0),
      width (lm.GetWidth ()), height (lm.GetHeight ())
    {
    }
    ~LightmapMask ()
    {
      Lock();
      SwappableHeap::Free (maskData);
    }
    
    float* GetMaskData() const { return maskData; }
    inline uint GetWidth () const { return width; }
    inline uint GetHeight () const { return height; }

    void Lock () const
    {
      if (!IsLocked() && (maskData == 0))
      {
	maskData = AllocMask();
      }
      Swappable::Lock();
    }
    
    virtual void GetSwapData (void*& data, size_t& size)
    {
      if (maskData == 0) maskData = AllocMask ();
      data = maskData;
      size = width * height * sizeof (float);
      // Set a bogus pointer so accesses to swapped data causes a segfault
      maskData = BogusPointer ();
    }
    virtual size_t GetSwapSize()
    {
      return width * height * sizeof (float);
    }
    virtual void SwapIn (void* data, size_t size)
    {
      CS_ASSERT (size == width * height * sizeof (float));
      CS_ASSERT (maskData == BogusPointer ());
      maskData = (float*)data;
    }
    const char* Describe() const
    { return "LightmapMask"; }
  };
  typedef csPDelArray<LightmapMask> LightmapMaskPtrDelArray;

  /// Records the influences of a light on a primitive group
  class LightInfluences : public Swappable
  {
  protected:
    struct LightInfluence
    {
      csVector3 direction;
      float intensity;
    };
    mutable LightInfluence* mapData;
    uint width, height;
    uint xOffs, yOffs;
    
    inline LightInfluence* BogusPointer () const 
    { return ((LightInfluence*)~0) - (width * height); }
    inline LightInfluence* AllocMap () const
    { 
      return (LightInfluence*)SwappableHeap::Alloc (width * height * 
        sizeof (LightInfluence));
    }
  public:
    LightInfluences (uint w, uint h, uint xOffs, uint yOffs) : mapData (0),
      width (w), height (h), xOffs (xOffs), yOffs (yOffs)
    {
    }
    ~LightInfluences ()
    {
      Lock();
      SwappableHeap::Free (mapData);
    }
    
    inline uint GetWidth () const { return width; }
    inline uint GetHeight () const { return height; }
    inline uint GetXOffset () const { return xOffs; }
    inline uint GetYOffset () const { return yOffs; }

    inline void AddDirection (size_t u, size_t v,
      const csVector3& d, float intensity)
    {
      LightInfluence& i = mapData[(u-xOffs)+(v-yOffs)*width];
      i.direction += d*intensity;
      i.intensity += intensity;
    }
    
    inline const csVector3& GetDirectionForLocalCoord (size_t u, size_t v) const
    {
      return mapData[u + width * v].direction;
    }
    inline float GetIntensityForLocalCoord (size_t u, size_t v) const
    {
      return mapData[u + width * v].intensity;
    }
    inline float GetTotalIntensity() const
    {
      float sum = 0;
      for (uint i = 0; i < width*height; i++)
        sum += mapData[i].intensity;
      return sum;
    }
    
    void Lock () const
    {
      if (!IsLocked() && (mapData == 0))
      {
	mapData = AllocMap();
      }
      Swappable::Lock();
    }
    
    virtual void GetSwapData (void*& data, size_t& size)
    {
      if (mapData == 0) mapData = AllocMap ();
      data = mapData;
      size = width * height * sizeof (LightInfluence);
      // Set a bogus pointer so accesses to swapped data causes a segfault
      mapData = BogusPointer ();
    }
    virtual size_t GetSwapSize()
    {
      return width * height * sizeof (LightInfluence);
    }
    virtual void SwapIn (void* data, size_t size)
    {
      CS_ASSERT (size == width * height * sizeof (LightInfluence));
      CS_ASSERT (mapData == BogusPointer ());
      mapData = (LightInfluence*)data;
    }
    const char* Describe() const
    { return "LightInfluences"; }
  };
  

  class DirectionMap : public Swappable
  {
  protected:
    mutable csVector3* mapData;
    uint width, height;
    
    inline csVector3* BogusPointer () const 
    { return ((csVector3*)~0) - (width * height); }
    inline csVector3* AllocMap () const
    { 
      return (csVector3*)SwappableHeap::Alloc (width * height * 
        sizeof (csVector3));
    }
  public:
    DirectionMap (const Lightmap &lm) : mapData (0),
      width (lm.GetWidth ()), height (lm.GetHeight ())
    {
    }
    ~DirectionMap ()
    {
      Lock();
      SwappableHeap::Free (mapData);
    }
    
    inline uint GetWidth () const { return width; }
    inline uint GetHeight () const { return height; }

    inline const csVector3* GetDirections () const
    {     
      return mapData;
    }
    
    void Normalize ();
    
    void AddFromLightInfluences (const LightInfluences& influences);

    void Lock () const
    {
      if (!IsLocked() && (mapData == 0))
      {
	mapData = AllocMap();
      }
      Swappable::Lock();
    }
    
    virtual void GetSwapData (void*& data, size_t& size)
    {
      if (mapData == 0) mapData = AllocMap ();
      data = mapData;
      size = width * height * sizeof (csVector3);
      // Set a bogus pointer so accesses to swapped data causes a segfault
      mapData = BogusPointer ();
    }
    virtual size_t GetSwapSize()
    {
      return width * height * sizeof (csVector3);
    }
    virtual void SwapIn (void* data, size_t size)
    {
      CS_ASSERT (size == width * height * sizeof (csVector3));
      CS_ASSERT (mapData == BogusPointer ());
      mapData = (csVector3*)data;
    }
    const char* Describe() const
    { return "DirectionMap"; }
  };
  typedef csPDelArray<DirectionMap> DirectionMapPtrDelArray;
  
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
