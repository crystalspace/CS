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
#include "ircacheoctreenode.h"

namespace lighter
{
  IrradianceCache::IrradianceCache(const float bboxMin[3], const float bboxMax[3],
                      const double maxError )
  {
    // Initialize all values
    storedSamples = 0;

    // Create initial root node
    root = new OctreeSampleNode(maxError);
    root->SetBoundingBox(bboxMin, bboxMax);
  }

  IrradianceCache::~IrradianceCache()
  {
    delete root;
  }

  size_t IrradianceCache :: GetSampleCount() { return storedSamples; }

  void IrradianceCache :: Store(
                           const float pos[3],
                           const float norm[3],
                           const float power[3],
                           const float mean)
  {
    IrradianceSample *const node = new (samplePool) IrradianceSample;
    storedSamples++;

    for (size_t i=0; i<3; i++)
    {
      node->pos[i] = pos[i];
      node->power[i] = power[i];
      node->norm[i] = norm[i];
    }
    node->mean = mean;

    root->AddSample(node);
  }

  bool IrradianceCache :: EstimateIrradiance(
      const float pos[3], const float norm[3], float* power)
  {
    // Put in sample struct
    IrradianceSample samp;
    samp.pos[0] = pos[0];
    samp.pos[1] = pos[1];
    samp.pos[2] = pos[2];
    samp.norm[0] = norm[0];
    samp.norm[1] = norm[1];
    samp.norm[2] = norm[2];

    // Build nearest struct
    NearestSamples* nearest = new NearestSamples();
    nearest->weights = new float[storedSamples];
    nearest->samples = new IrradianceSample const*[storedSamples];
    nearest->count = 0;

    // Search Octree
    root->FindSamples(&samp, nearest);

    bool res;
    // Check results
    if(nearest->count > 0)
    {
      // Compute irradiance estimate
      power[0] = power[1] = power[2] = 0.0;
      float weightSum = 0.0;

      for(size_t i=0; i<nearest->count; i++)
      {
        // Get local copies of data
        const IrradianceSample *Pi = nearest->samples[i];
        float Wi = nearest->weights[i];

        // Add weighted power contribution
        power[0] += Pi->power[0]*Wi;
        power[1] += Pi->power[1]*Wi;
        power[2] += Pi->power[2]*Wi;

        // Sum weights
        weightSum += Wi;
      }

      // Normalize estimate
      power[0] /= weightSum;
      power[1] /= weightSum;
      power[2] /= weightSum;

      // Signal success
      res = true;
    }
    
    // No valid samples found
    res = false;
    
    delete[] nearest->weights;
    delete[] nearest->samples;
    delete nearest;
    
    return res;
  }

};
