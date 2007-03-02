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

namespace lighter
{
  class LightmapMask;

  struct LightmapData
  {
    LightmapData ()
      : colorArray (0), colorArraySize (0)
    {}

    ~LightmapData ()
    {
      delete[] colorArray;
    }

    csColor *colorArray;
    size_t colorArraySize;
  };

  class Lightmap
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
      data->colorArray[v*width + u] += c;
    }

    // Save the lightmap to given file
    void SaveLightmap (const csString& file);

    // Fixup, de-antialise lightmap etc
    void FixupLightmap (const LightmapMask& mask);

    // Data getters
    inline LightmapData* GetData () const { return data; }
    inline void SetData (LightmapData* d) { data = d; }

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
  protected:
    // The color data itself
    LightmapData* data;

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

  // File-backed cache for LM data
  class LightmapCache
  {
  public:
    LightmapCache (size_t maxSize);
    ~LightmapCache ();

    // Initialize, load config, preallocate etc
    static void Initialize ();

    // Remove etc
    static void CleanUp ();

    // Lock a light map in cache, make sure it have data.
    // Post condition: lightmap have a sufficiently large memory buffer
    void LockLM (Lightmap* lm);

    // Unlock a light map in cache, potentially it can be swapped out after this
    void UnlockLM (Lightmap* lm);

    // Unlock all light maps in cache
    void UnlockAll ();

    // Remove any trace of lm
    void RemoveLM (Lightmap* lm);
  private:
    // One in-memory entry
    struct LMEntry
    {
      LMEntry ()
        : lm (0), data (0), lastUnlockTime (0)
      {
      }

      Lightmap* lm;
      LightmapData* data;
      size_t lastUnlockTime;
    };

    // Swap out one LM-entry to disk
    bool SwapOut (LMEntry* e);

    // Swap in one LM-entry from disk
    bool SwapIn (LMEntry* e);

    // Free memory, around size bytes
    void FreeMemory (size_t desiredAmount);

    // Allocate
    LightmapData* AllocateData (size_t size);

    // Given a lightmap, get a temporary filename for the cache
    csString GetLMFileName (Lightmap* lm);

    // Compare two LM entries
    static int LMEntryAgeCompare (LMEntry* const & e1, LMEntry* const& e2)
    {
      return (int)e1->lastUnlockTime - (int)e2->lastUnlockTime;
    }
    
    //All current LM cache entries
    typedef csHash<LMEntry*, csPtrKey<Lightmap> > LMCacheType;
    LMCacheType lmCache;

    //Currently unlocked LM cache entires (potential to be swapped out)
    typedef csSet<csPtrKey<LMEntry> > UnlockedEntriesType;
    UnlockedEntriesType unlockedCacheEntries;

    //Statistics for house-keeping
    size_t maxCacheSize, currentCacheSize;
    size_t currentUnlockTime;
  };
  extern LightmapCache* globalLMCache;

  // Small helper to lock LMs
  class LightmapCacheLock
  {
  public:
    LightmapCacheLock (Lightmap* lm)
      : lightmap (lm)
    {
      globalLMCache->LockLM (lightmap);
    }


    ~LightmapCacheLock ()
    {
      globalLMCache->UnlockLM (lightmap);
    }

  private:
    Lightmap* lightmap;
  };
}

#endif
