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

//-----------------------------------------------------------------------------
// The underlying KD-tree structure and the functions that balance it are
// from the photonmap.cc code from Henrick Wann Jensen's book 'Realistic Image
// Synthesis Using Photon Mapping'
//-----------------------------------------------------------------------------

#ifndef __IRRADIANCECACHE_H__
#define __IRRADIANCECACHE_H__

#include "statistics.h"

namespace lighter
{
  struct IrradianceSample;

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
     **/
    void Store(
      const float power[3],
      const float pos[3]);

    size_t GetSampleCount();

    /**
     * Balance
     *    Restructure the photon map heap into a balanced kd-tree.
     * You must call this function before using the map with the
     * function sample.
     **/
    void Balance(Statistics::ProgressState &prog);

    /**
     * SaveToFile
     *    Write the contents of the irradiance cache to a file for external
     * use. The file will have the following format:
     *
     *    <size_t:sampleCount>
     *
     *    <float:PositionXCoordinate>*sampleCount
     *    <float:PositionYCoordinate>*sampleCount
     *    <float:PositionZCoordinate>*sampleCount
     *
     *    <float:IrradianceRedComponent>*sampleCount
     *    <float:IrradianceGreenComponent>*sampleCount
     *    <float:IrradianceBlueComponent>*sampleCount
     *
     *    <short int:KDTreeSplittingPlane>*photonCount
     *
     * Note: whitespace and newlines above are not part of the file format.
     * Endiness is the machine native format.
     *
     * /param filename - name of file to store irradiance cache data
     **/
    void SaveToFile(
      const char* filename);

  private:

    bool Expand();

    void BalanceSegment(
      IrradianceSample **pbal,
      IrradianceSample **porg,
      const size_t index,

      const size_t start,
      const size_t end,
      Statistics::ProgressState &prog);

    void MedianSplit(
      IrradianceSample **p,
      const size_t start,
      const size_t end,
      const size_t median,
      const int axis );

    IrradianceSample *samples;          ///< Internal array of photons (kd-tree heap after call to Balance())

    size_t initialSize;       ///< The initial requested array size (used for array expansion)
    size_t storedSamples;     ///< Number of photons stored in the internal array
    size_t halfStoredSamples; ///< Half the photons stored in the internal array
    size_t maxSamples;        ///< Allocated size of internal array

    float bboxMin[3];         ///< Minimum value in each dimension for bounding box computation
    float bboxMax[3];         ///< Maximum value in each dimension for bounding box computation
  };
};

#endif // __IRRADIANCECACHE_H__