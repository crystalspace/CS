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

  class Lightmap
  {
  public:
    Lightmap (uint width, uint height)
      : width (0), height (0), maxUsedU (0), maxUsedV (0), 
      lightmapAllocator (csRect (0,0,1,1))
    {
      Grow (width, height);
    }

    inline void Initialize ()
    {
      data.DeleteAll ();
      data.SetSize (width*height, csColor (0.0f,0.0f,0.0f));
      // Init with gray shades ;)
      for (uint i = 0; i < data.GetSize (); i++)
      {
        csColor &c = data[i];
        c.red = c.green = c.blue = ((float)i/(float)(data.GetSize ()));
      }
    }

    // Add a general ambient term
    void AddAmbientTerm (const csColor amb);

    // Apply the exposure function
    void ApplyExposureFunction (float expConstant, float expMax);

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

    // Save the lightmap to given file
    void SaveLightmap (const csString& file);

    // Data getters
    inline ColorArray& GetData () { return data; }
    inline const ColorArray& GetData () const { return data; }

    inline uint GetWidth () const {return width; }
    inline uint GetHeight () const {return height; }

    inline uint GetMaxUsedU () const {return maxUsedU; }
    inline uint GetMaxUsedV () const {return maxUsedV; }

    inline csSubRectangles& GetAllocator () { return lightmapAllocator; }
    inline const csSubRectangles& GetAllocator () const { return lightmapAllocator; }

    inline const csString& GetFilename () { return filename; }

  protected:
    // The color data itself
    ColorArray data;

    // Size
    uint width, height;

    // Max used U/V
    uint maxUsedU, maxUsedV;

    // Area allocator
    csSubRectangles lightmapAllocator;

    // Filename
    csString filename;
  };
  typedef csArray<Lightmap> LightmapArray;
  typedef csPDelArray<Lightmap> LightmapPtrDelArray;
}

#endif
