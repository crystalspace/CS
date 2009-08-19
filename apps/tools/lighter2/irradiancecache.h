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
  class OctreeSampleNode;

  struct IrradianceSample
  {
    float pos[3];   // sample position
    float norm[3];  // Normal at pos
    float power[3]; // irradiance
    float mean;     // Harmonic mean distance to visible surfaces
  };

  class IrradianceCache
  {
  public:
    IrradianceCache( const float bboxMin[3], const float bboxMax[3],
      size_t maxSamples, double maxError = 0.1 );
    ~IrradianceCache();

    /**
     * Store
     *    Add a sample to the irradiance cache structure.
     * /param pos - The world position of this sample
     * /param norm - The surface normal at this position
     * /param power - The irradiance estimate at this position
     * /param mean - The harmonic mean of ray lengths used to calc this sample
     **/
    void Store(
      const float pos[3],
      const float norm[3],
      const float power[3],
      const float mean);

    /**
     * EstimateIrradiance
     *    Try to estimate the irradiance from the cache.  This function
     * will estimate the irradiance gradiant using the method proposed
     * by Ward et al. and determine if it can be averged from values
     * already in the cache.  If so, it will place the estimate in
     * 'power' and return true.  If not, it will return false.
     * /param pos - The world position at which to compute irradiance
     * /param norm - The surface normal at pos
     * /param power - On return, holds the irradiance at pos
     **/
    bool EstimateIrradiance(
      const float pos[3],
      const float norm[3],
      float* &power);

    size_t GetSampleCount();

  private:

    bool Expand();

    IrradianceSample *samples;  ///< Internal array of samples
    OctreeSampleNode *root;     ///< The root of the octree
    double alpha;               ///< The maximum error allowed when estimating irradiance

    size_t initialSize;     ///< Initial allocated sample array size
    size_t storedSamples;   ///< Number of samples stored in the internal array
    size_t maxSamples;      ///< Actual allocated size of sample array
  };
};

#endif // __IRRADIANCECACHE_H__