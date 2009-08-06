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

  #define MAX_NODE_SIZE 10

  class OctreeSampleNode
  {
  private:
    size_t ID;
    csArray<size_t> samples;
    OctreeSampleNode* child[8];
    const IrradianceSample** masterArray; 
    bool isLeaf;

    // Bounding box for this octant
    float bboxMin[3];
    float bboxMax[3];
    float bboxMid[3];

  public:
    OctreeSampleNode(const IrradianceSample** &parentArray)
    {
      masterArray = parentArray;
      ID = 0; isLeaf = true;

      bboxMin[0] = bboxMin[1] = bboxMin[2] = FLT_MAX;
      bboxMax[0] = bboxMax[1] = bboxMax[2] = -FLT_MAX;
      bboxMid[0] = bboxMid[1] = bboxMid[2] = 0;

      child[0] = child[1] = child[2] = child[3] = NULL;
      child[4] = child[5] = child[6] = child[7] = NULL;
    }

    ~OctreeSampleNode()
    {
      for(size_t i=0; i<8; i++)
        if(child[i] != NULL) delete child[i];
    }

    bool AddSample(const size_t newNode, bool checkBounds = true)
    {
      // Check if it fits in our bounding box
      if(checkBounds)
      {
        if(masterArray[newNode]->pos[0] < bboxMin[0] ||
           masterArray[newNode]->pos[1] < bboxMin[1] ||
           masterArray[newNode]->pos[2] < bboxMin[2] ||

           masterArray[newNode]->pos[0] >= bboxMax[0] ||
           masterArray[newNode]->pos[1] >= bboxMax[1] ||
           masterArray[newNode]->pos[2] >= bboxMax[2])
        { 
          return false;
        }
      }

      // Is this a leaf node?
      if(isLeaf)
      {
        // Add sample to list and check if split is needed
        samples.Push(newNode);
        if(SplitNeeded())
        {
          SplitNode();
        }
      }

      // Not a leaf so add to child
      else
      {
        // Find child that bounds this new sample
        bool xPlus = (masterArray[newNode]->pos[0] >= bboxMid[0]);
        bool yPlus = (masterArray[newNode]->pos[1] >= bboxMid[1]);
        bool zPlus = (masterArray[newNode]->pos[2] >= bboxMid[2]);

        // Only three tests to find the octant (and no need to re-test
        // bounding box so 2nd param is false).
        if(xPlus)
        {
          if(yPlus)
          {
            if(zPlus)
            {
              child[0]->AddSample(newNode, false);
            }
            else
            {
              child[4]->AddSample(newNode, false);
            }
          }
          else
          {
            if(zPlus)
            {
              child[3]->AddSample(newNode, false);
            }
            else
            {
              child[7]->AddSample(newNode, false);
            }
          }
        }
        else
        {
          if(yPlus)
          {
            if(zPlus)
            {
              child[1]->AddSample(newNode, false);
            }
            else
            {
              child[5]->AddSample(newNode, false);
            }
          }
          else
          {
            if(zPlus)
            {
              child[2]->AddSample(newNode, false);
            }
            else
            {
              child[6]->AddSample(newNode, false);
            }
          }
        }
      }
    }

    bool SplitNeeded()
    {
      if(samples.GetSize() > MAX_NODE_SIZE) return true;
      return false;
    }

    void SplitNode()
    {
      // We only split leaf nodes
      if(!isLeaf) return;

      // Allocate children nodes
      for(size_t i=0; i<8; i++)
      {
        child[i] = new OctreeSampleNode(masterArray);
      }

      // Build child node bounding boxes
      bboxMid[0] = (bboxMin[0] + bboxMax[0])/2.0;
      bboxMid[0] = (bboxMin[1] + bboxMax[1])/2.0;
      bboxMid[0] = (bboxMin[2] + bboxMax[2])/2.0;

      // 0 octant is (+, +, +)
      child[0]->bboxMin[0] = bboxMid[0];
      child[0]->bboxMin[1] = bboxMid[1];
      child[0]->bboxMin[2] = bboxMid[2];

      child[0]->bboxMax[0] = bboxMax[0];
      child[0]->bboxMax[1] = bboxMax[1];
      child[0]->bboxMax[2] = bboxMax[2];

      // 1 octant is (-, +, +)
      child[1]->bboxMin[0] = bboxMin[0];
      child[1]->bboxMin[1] = bboxMid[1];
      child[1]->bboxMin[2] = bboxMid[2];

      child[1]->bboxMax[0] = bboxMid[0];
      child[1]->bboxMax[1] = bboxMax[1];
      child[1]->bboxMax[2] = bboxMax[2];

      // 2 octant is (-, -, +)
      child[2]->bboxMin[0] = bboxMin[0];
      child[2]->bboxMin[1] = bboxMin[1];
      child[2]->bboxMin[2] = bboxMid[2];

      child[2]->bboxMax[0] = bboxMid[0];
      child[2]->bboxMax[1] = bboxMid[1];
      child[2]->bboxMax[2] = bboxMax[2];

      // 3 octant is (+, -, +)
      child[3]->bboxMin[0] = bboxMid[0];
      child[3]->bboxMin[1] = bboxMin[1];
      child[3]->bboxMin[2] = bboxMid[2];

      child[3]->bboxMax[0] = bboxMax[0];
      child[3]->bboxMax[1] = bboxMid[1];
      child[3]->bboxMax[2] = bboxMax[2];

      // 4 octant is (+, +, -)
      child[4]->bboxMin[0] = bboxMid[0];
      child[4]->bboxMin[1] = bboxMid[1];
      child[4]->bboxMin[2] = bboxMin[2];

      child[4]->bboxMax[0] = bboxMax[0];
      child[4]->bboxMax[1] = bboxMax[1];
      child[4]->bboxMax[2] = bboxMid[2];

      // 5 octant is (-, +, -)
      child[5]->bboxMin[0] = bboxMin[0];
      child[5]->bboxMin[1] = bboxMid[1];
      child[5]->bboxMin[2] = bboxMin[2];

      child[5]->bboxMax[0] = bboxMid[0];
      child[5]->bboxMax[1] = bboxMax[1];
      child[5]->bboxMax[2] = bboxMid[2];

      // 6 octant is (-, -, -)
      child[6]->bboxMin[0] = bboxMin[0];
      child[6]->bboxMin[1] = bboxMin[1];
      child[6]->bboxMin[2] = bboxMin[2];

      child[6]->bboxMax[0] = bboxMid[0];
      child[6]->bboxMax[1] = bboxMid[1];
      child[6]->bboxMax[2] = bboxMid[2];

      // 7 octant is (+, -, -)
      child[7]->bboxMin[0] = bboxMid[0];
      child[7]->bboxMin[1] = bboxMin[1];
      child[7]->bboxMin[2] = bboxMin[2];

      child[7]->bboxMax[0] = bboxMax[0];
      child[7]->bboxMax[1] = bboxMid[1];
      child[7]->bboxMax[2] = bboxMid[2];

      // This node is no longer a leaf
      isLeaf = false;

      // Redistribute samples to children by re-adding each one
      for(size_t i=0; i<samples.GetSize(); i++)
      {
        AddSample(samples[i], false);
      }

      // Clear sample list to free up memory
      samples.DeleteAll();
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
