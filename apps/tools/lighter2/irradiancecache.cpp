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

#include "common.h"
#include "irradiancecache.h"

namespace lighter
{
  struct IrradianceSample
  {
    float pos[3];   // sample position
    float norm[3];  // Normal at pos
    float power[3]; // irradiance (uncompressed)

    size_t octant;  // ID of the Octree node that contains this sample
  };

  struct OctreeSampleNode
  {
    size_t ID;
    csArray<size_t> samples;
    OctreeSampleNode* children[8];

    // Bounding box for this octant
    float bboxMin[3];
    float bboxMax[3];

    OctreeSampleNode()
    {
      ID = 0;

      bboxMin[0] = bboxMin[1] = bboxMin[2] = FLT_MAX;
      bboxMax[0] = bboxMax[1] = bboxMax[2] = -FLT_MAX;

      children[0] = NULL;
      children[1] = NULL;
      children[2] = NULL;
      children[3] = NULL;
      children[4] = NULL;
      children[5] = NULL;
      children[6] = NULL;
      children[7] = NULL;
    }
  };

  IrradianceCache::IrradianceCache( const size_t maxSamps )
  {
    storedSamples = 0;
    initialSize = maxSamples = maxSamps;

    samples = (IrradianceSample*)malloc( sizeof( IrradianceSample ) * ( maxSamples+1 ) );
    root = NULL;
  }

  IrradianceCache::~IrradianceCache()
  {
    free( samples );
  }

  size_t IrradianceCache :: GetSampleCount() { return storedSamples; }

  void IrradianceCache :: Store(
                           const float power[3],
                           const float pos[3],
                           const float norm[3])
  {
    // Check for storage and attempt to expand if needed
    if (storedSamples>=maxSamples && !Expand())
      return;

    storedSamples++;
    IrradianceSample *const node = &(samples[storedSamples]);

    for (size_t i=0; i<3; i++) {
      node->pos[i] = pos[i];
      node->power[i] = power[i];
      node->norm[i] = norm[i];
    }
  }

  bool IrradianceCache :: Expand()
  {
    // Increase size by initial amount
    size_t newMax = maxSamples + initialSize;

    // Allocate a new array
    IrradianceSample *newSamples =
      (IrradianceSample*)malloc( sizeof( IrradianceSample ) * ( newMax+1 ) );
    if(newSamples == NULL) return false;

    // Copy over old data
    for(size_t i=0; i<maxSamples; i++)
      newSamples[i] = samples[i];

    // Free old array
    free(samples);

    // Replace old variables
    samples = newSamples;
    maxSamples = newMax;

    // return success
    return true;
  }

};
