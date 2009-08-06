/*
  Copyright (C) 2009 by Seth Berrier

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

#ifndef __IRRADIANCECACHE_H__
#define __IRRADIANCECACHE_H__

#include "statistics.h"

namespace lighter
{
  struct IrradianceSample;
  class OctreeSampleNode;

  class IrradianceCache
  {
  public:
    IrradianceCache( size_t maxSamples );
    ~IrradianceCache();

    /**
     * Store
     *    Add a sample to the irradiance cache structure.
     * /param power - The irradiance estimate at this position
     * /param pos - The world position of this sample
     * /param norm - The surface normal at this position
     **/
    void Store(
      const float power[3],
      const float pos[3],
      const float norm[3]);

    size_t GetSampleCount();

  private:

    bool Expand();

    IrradianceSample *samples;  ///< Internal array of samples
    OctreeSampleNode *root;     ///< The root of the octree

    size_t initialSize;       ///< The initial requested array size (used for array expansion)
    size_t storedSamples;     ///< Number of photons stored in the internal array
    size_t halfStoredSamples; ///< Half the photons stored in the internal array
    size_t maxSamples;        ///< Allocated size of internal array
  };
};

#endif // __IRRADIANCECACHE_H__