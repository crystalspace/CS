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

#ifndef __COMMON_H__
#define __COMMON_H__


namespace lighter
{
  // Standard types
  typedef csArray<float> FloatArray;
  typedef csDirtyAccessArray<float> FloatDArray;

  typedef csArray<csVector2> Vector2Array;
  typedef csDirtyAccessArray<csVector2> Vector2DArray;

  typedef csArray<csVector3> Vector3Array;
  typedef csDirtyAccessArray<csVector3> Vector3DArray;

  typedef csArray<csColor> ColorArray;
  typedef csDirtyAccessArray<csColor> ColorDArray;

  typedef csArray<void*> VoidArray;
  typedef csDirtyAccessArray<void*> VoidDArray;

  typedef csArray<int> IntArray;
  typedef csDirtyAccessArray<int> IntDArray;

  // Constants
  static const float LITEPSILON = 1.0e-5f;

  // Settings structure
  struct RadSettings
  {
    // Density in u and v direction. u = uTexelPerUnit*x etc.. 
    float uTexelPerUnit, vTexelPerUnit;
    
    // Element/patch densities
    uint uPatchResolution, vPatchResolution;

    // Max lightmap sizes
    uint maxLightmapU, maxLightmapV;

    // Set default settings
    RadSettings ()
      : uTexelPerUnit (4/(1.0f)), vTexelPerUnit (4/(1.0f)),
      uPatchResolution (4), vPatchResolution (4),
      maxLightmapU (256), maxLightmapV (256)
    {
    }
  };

  //The global settings
  extern RadSettings globalSettings;

}

#endif
